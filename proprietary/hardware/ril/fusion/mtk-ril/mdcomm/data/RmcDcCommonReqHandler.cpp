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
#include <linux/sockios.h>
#include <sys/socket.h>
#include <memory>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "NetAgentService.h"

#include "RmcDataDefs.h"
#include "RmcDcUtility.h"
#include "RfxStringData.h"
#include "RfxStringsData.h"
#include "RfxVoidData.h"
#include "RfxIntsData.h"
#include "RfxDataCallResponseData.h"
#include "RfxPcoData.h"
#include "RfxPcoIaData.h"
#include "RfxMessageId.h"
#include "RmcDcCommonReqHandler.h"
#include "RfxRilUtils.h"
#include "tinyxml.h"
#include <libmtkrilutils.h>
#include "RfxKeepaliveStatusData.h"

#define RFX_LOG_TAG "RmcDcCommon"
#define MAX_AT_CMD_LENGTH 512
#define CMD_INDEX_OFFSET 100

/*****************************************************************************
 * Class RmcDcCommonReqHandler
 *****************************************************************************/
RFX_REGISTER_DATA_TO_URC_ID(RfxPcoData, RFX_MSG_UNSOL_PCO_DATA);
RFX_REGISTER_DATA_TO_URC_ID(RfxPcoIaData, RFX_MSG_UNSOL_PCO_DATA_AFTER_ATTACHED);
RFX_REGISTER_DATA_TO_URC_ID(RfxIntsData, RFX_MSG_URC_QUALIFIED_NETWORK_TYPES_CHANGED);
RFX_REGISTER_DATA_TO_EVENT_ID(RfxIntsData, RFX_MSG_EVENT_QUERY_PCO_WITH_AID);
RFX_REGISTER_DATA_TO_EVENT_ID(RfxVoidData, RFX_MSG_EVENT_REQ_DATA_CONTEXT_IDS);
RFX_REGISTER_DATA_TO_EVENT_ID(RfxIntsData, RFX_MSG_EVENT_SEND_QUALIFIED_NETWORK_TYPES_CHANGED);
RFX_REGISTER_DATA_TO_EVENT_ID(RfxVoidData, RFX_MSG_EVENT_QUERY_SELF_IA);
RFX_REGISTER_DATA_TO_URC_ID(RfxIntsData, RFX_MSG_URC_MOBILE_DATA_USAGE);
RFX_REGISTER_DATA_TO_URC_ID(RfxKeepaliveStatusData, RFX_MSG_URC_KEEPALIVE_STATUS);
RFX_REGISTER_DATA_TO_URC_ID(RfxIntsData, RFX_MSG_URC_NW_LIMIT);

RIL_MtkDataProfileInfo* RmcDcCommonReqHandler::s_LastApnTable[MAX_SIM_COUNT] = {NULL};
int RmcDcCommonReqHandler::s_nLastReqNum[MAX_SIM_COUNT] = {0};
int* RmcDcCommonReqHandler::s_ApnCmdIndex[MAX_SIM_COUNT] = {NULL};

int RmcDcCommonReqHandler::s_dataSetting[MAX_SIM_COUNT][DATA_SETTING_NUMBERS] = {{0}};
int RmcDcCommonReqHandler::s_dataSetting_resend[MAX_SIM_COUNT][DATA_SETTING_NUMBERS] = {{SKIP_DATA_SETTINGS}};

RmcDcCommonReqHandler::RmcDcCommonReqHandler(int slot_id, int channel_id, RmcDcPdnManager* pdnManager)
: RfxBaseHandler(slot_id, channel_id), m_pPdnManager(pdnManager), m_nGprsFailureCause(PDP_FAIL_ERROR_UNSPECIFIED) {
}

RmcDcCommonReqHandler::~RmcDcCommonReqHandler() {
}

int RmcDcCommonReqHandler::activatePdn(const char *reqApn, const char *profileType,
        Vector<int> *vAidList, int isEmergency, MTK_RIL_Data_Call_Response_v11* response, int cmdIndex) {
    String8 cmd("");
    int err = 0;
    sp<RfxAtResponse> p_response;
    char *out = NULL;
    RfxAtLine *p_cur = NULL;
    int rid = m_slot_id;
    int cmeError = CME_ERROR_NON_CME;
    char apn[MAX_APN_NAME_LENGTH] = {0};

    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] isEmergency: %d",
            rid, __FUNCTION__, isEmergency);

    if (response == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] response is null!",
                rid, __FUNCTION__);
        return PDN_FAILED;
    }

    // AT+EAPNACT=<state>,<apn_name>,<apn_type>;
    // <state>: 0-deactivate; 1-activate
    if (reqApn != NULL && strlen(reqApn) > 0) {
        strncpy(apn, reqApn, MAX_APN_NAME_LENGTH-1);
        addEscapeSequence(apn);
    }
    cmd.append(String8::format("AT+EAPNACT=1,\"%s\",\"%s\",%d", apn, profileType, cmdIndex));

    // +CGEV: ME PDN ACT <aid>[,<reason>[,<aid_other>]]
    p_response = atSendCommandMultiline(cmd, "+CGEV: ME PDN ACT ");

    if (p_response == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get p_response!",
                rid, __FUNCTION__);
        goto error;
    }

    err = p_response->getError();
    if (err == AT_ERROR_CHANNEL_CLOSED) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] Error when channel closed", rid, __FUNCTION__);
        goto error_channel_closed;
    }

    for (p_cur = p_response->getIntermediates(); p_cur != NULL; p_cur = p_cur->getNext()) {
        p_cur->atTokStart(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] Error when token start",
                    rid, __FUNCTION__);
            goto error;
        }

        int aid = INVALID_AID;
        int reason = NO_CAUSE;
        int otherAid = INVALID_AID;
        char *tempParam = NULL;

        tempParam = p_cur->atTokNextstr(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] Error when parsing aid",
                    rid, __FUNCTION__);
            goto error;
        }
        char *firstParam = tempParam + strlen(ME_PDN_ACT);
        aid = RmcDcUtility::stringToBinaryBase(firstParam, 10, &err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when converting aid to binary"
                    ", err = %d", rid, __FUNCTION__, err);
            goto error;
        }

        setIsDedicateBearer(aid, 0);
        updatePdnActiveStatus(aid, DATA_STATE_LINKDOWN);
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] default bearer activated [aid=%d]",
                rid, __FUNCTION__, aid);

        if (p_cur->atTokHasmore()) {
            reason = p_cur->atTokNextint(&err);
            if (err < 0) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] Error when parsing reason for aid%d",
                        rid, __FUNCTION__, aid);
                goto error;
            }

            RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] default bearer activated [aid=%d, reason=%d]",
                    rid, __FUNCTION__, aid, reason);

            if (p_cur->atTokHasmore()) {
                otherAid = p_cur->atTokNextint(&err);
                if (err < 0) {
                    RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] Error when parsing otherAid for aid%d",
                            rid, __FUNCTION__, aid);
                    goto error;
                }

                RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] default bearer activated [aid=%d, reason=%d,"
                        " otherAid=%d]", rid, __FUNCTION__, aid, reason, otherAid);
            }
        }

        setAidAndPrimaryAid(aid, aid, aid);
        setReason(aid, reason);
        setIsEmergency(aid, isEmergency);
        vAidList->push_back(aid);
    }

    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] %zu PDN activated",
            rid, __FUNCTION__, vAidList->size());

    if (p_response->isAtResponseFail() || (vAidList->size() == 0)) {
        if (p_response->getSuccess() > 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] weird case, can not get fail cause due to final"
                    " response indicates success", rid, __FUNCTION__);
            goto error;
        }
        cmeError = p_response->atGetCmeError();
        if (cmeError == CME_UNKNOWN || cmeError == CME_ERROR_NON_CME) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] AT+EAPNACT CME UNKNOWN(100) or NON CME ERROR(-1): %d",
                    rid, __FUNCTION__, cmeError);
            m_nGprsFailureCause = PDP_FAIL_ERROR_UNSPECIFIED;
            if (cmeError == CME_ERROR_NON_CME) {
                goto error;
            }
        } else {
            updateLastFailCause(cmeError);
            if (m_nGprsFailureCause == PDP_FAIL_NONE) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] NO CAUSE(0) but AT+EAPNACT failed",
                        rid, __FUNCTION__);
                m_nGprsFailureCause = PDP_FAIL_ERROR_UNSPECIFIED;
            } else {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] AT+EAPNACT CAUSE: %X",
                        rid, __FUNCTION__, m_nGprsFailureCause);
            }
        }
        goto cme_error;
    }

    RFX_LOG_V(RFX_LOG_TAG, "[%d][%s] finished X", rid, __FUNCTION__);
    return PDN_SUCCESS;

error:
    response->status = m_nGprsFailureCause = PDP_FAIL_ERROR_UNSPECIFIED;
    return PDN_FAILED;

cme_error:
    response->status = m_nGprsFailureCause;
    response->suggestedRetryTime = getModemSuggestedRetryTime(apn);
    return PDN_FAILED;

error_channel_closed:
    response->status = m_nGprsFailureCause = PDP_FAIL_ERROR_UNSPECIFIED;
    return CHANNEL_CLOSED;
}

int RmcDcCommonReqHandler::getModemSuggestedRetryTime(const char *apnName) {
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> response;
    RfxAtLine *line = NULL;
    int err = 0;
    String8 cmd("");
    int suggestType = RETRY_TYPE_NO_SUGGEST;
    int suggestTime = -1;
    int rid = m_slot_id;

    if (m_nGprsFailureCause == PDP_FAIL_DATA_NOT_ALLOW) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] m_nGprsFailureCause is PDP_FAIL_DATA_NOT_ALLOW",
                rid, __FUNCTION__);
        return DATA_NOT_ALLOW_RETRY_TIME;
    }

    // AT+EDRETRY=<mode><APN name>
    // <mode>:indicate it's query mode or reset mode. 0:query mode, 1:reset mode
    // <APN name>: APN (string type) which is use to query or reset bar timer and retry count.
    cmd = String8::format("AT+EDRETRY=0,\"%s\"", apnName);
    p_response = atSendCommandSingleline(cmd, "+EDRETRY:");

    if (p_response == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get p_response!",
                rid, __FUNCTION__);
        return -1;
    }

    if (p_response->isATCmdRspErr()) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] AT+EDRETRY query command returns ERROR", rid, __FUNCTION__);
        return -1; //NO suggested retry time
    } else {
        err = p_response->getError();
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get error!",
                    rid, __FUNCTION__);
            return -1;
        }

        line = p_response->getIntermediates();

        if (line == NULL) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get intermediate results!",
                    rid, __FUNCTION__);
            return -1;
        }

        line->atTokStart(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when token start", rid, __FUNCTION__);
            return -1;
        }
        suggestType = line->atTokNextint(&err);
        if (err < 0) return -1;

        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] Modem suggest type:[%d]", rid, __FUNCTION__, suggestType);
        if (suggestType == RETRY_TYPE_NO_SUGGEST) {
            return -1;
        } else if (suggestType == RETRY_TYPE_NO_RETRY) {
            return 0x7fffffff;
        } else if (suggestType == RETRY_TYPE_WITH_SUGGEST) {
            if (line->atTokHasmore()) {
                suggestTime = line->atTokNextint(&err);
                if (err < 0) return -1;
                RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] Modem suggest time:[%d]", rid, __FUNCTION__, suggestTime);
                if (suggestTime >= 0 && suggestTime != 0x7fffffff) {
                    // The unit of suggested retry time from modem is second.
                    // We need to return the value in msec.
                    return suggestTime*1000;
                } else {
                    return -1;
                }
            } else {
                return -1;
            }
        }
    }
    return suggestTime;
}

int RmcDcCommonReqHandler::updatePdnInformation(const int activatedAid, int protocol,
        NETAGENT_IFST_STATUS ifst) {
    int rid = m_slot_id;
    int err = 0;
    int tempAid = INVALID_AID;
    int transIntfId = INVALID_TRANS_INTF_ID;
    char *cmdFormat = NULL;
    int cause = 0;
    int rat = 1;  // 1: cellular
    int mtu = 0;
    int pdnType = INVALID_PDN_TYPE;
    char *addr1 = NULL;
    char *addr2 = NULL;
    String8 cmd("");
    RfxAtLine *line = NULL;
    sp<RfxAtResponse> p_response;
    int protocolClassBitmap = RmcDcUtility::getProtocolClassBitmap(protocol);

    if (protocolClassBitmap == NETAGENT_ADDR_TYPE_UNKNOWN) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] invalid protocolClassBitmap", rid, __FUNCTION__);
        return PDN_FAILED;
    }

    if (ifst != NETAGENT_IFST_NONE && ifst != NETAGENT_IFST_UP) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] invalid ifst status %d", rid, __FUNCTION__, ifst);
        return PDN_FAILED;
    }

    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] activated aid%d", rid, __FUNCTION__, activatedAid);

    // AT+EPDN=<aid>, "ifst", <ifst_status + protocol_class_bitmap>
    // <ifst_status> :
    //       0     -> update interface status without waiting
    //       16    -> wait for interface up
    // <protocol_class_bitmap> :
    //       0     -> unknown
    //       1     -> wait for ipv4 address
    //       2     -> wait for ipv6 address
    //       3     -> wait for ipv4 and ipv6 address
    //       4     -> wait for any address
    //
    // Response:
    //     +EPDN:<aid>,"new",<rat type>,<interface id>,<mtu>,<address type>,<address1>[,<address2>]
    //     +EPDN:<aid>,"update",<interface id>,<address type>,<address1>[,<address2>]
    //     +EPDN:<aid>,"err",<err>
    cmd.append(String8::format("AT+EPDN=%d,\"ifst\",%d", activatedAid, ifst + protocolClassBitmap));
    p_response = atSendCommandSingleline(cmd, "+EPDN:");

    if (p_response == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get p_response!",
                rid, __FUNCTION__);
        return PDN_FAILED;
    }

    if (p_response->isAtResponseFail()) {
        if (p_response->getSuccess() > 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] weird case, can not get fail cause due to final"
                    " response indicates success", rid, __FUNCTION__);
        } else {
            cause = p_response->atGetCmeError();
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] AT+EPDN(ifst) for activatedAid%d returns cme cause : %d",
                    rid, __FUNCTION__, activatedAid, cause);
        }
        return PDN_FAILED;
    }

    line = p_response->getIntermediates();

    if (line == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get intermediate results!",
                rid, __FUNCTION__);
        return PDN_FAILED;
    }

    line->atTokStart(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when token start",
                rid, __FUNCTION__);
        return PDN_FAILED;
    }

    tempAid = line->atTokNextint(&err);
    if (err < 0 || (tempAid != activatedAid)) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get aid or it's different to MD assigned",
                rid, __FUNCTION__);
        return PDN_FAILED;
    }

    cmdFormat = line->atTokNextstr(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing cmd",
                rid, __FUNCTION__);
        return PDN_FAILED;
    }

    if (strncmp(DC_ERROR, cmdFormat, strlen(DC_ERROR)) == 0) {
        if (line->atTokHasmore()) {
            cause = line->atTokNextint(&err);  // Not use currently.
            if (err < 0) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing err cause",
                        rid, __FUNCTION__);
            } else {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] AT+EPDN(ifst) for activatedAid%d returns err cause : %d",
                        rid, __FUNCTION__, activatedAid, cause);
            }
        }
        return PDN_FAILED;
    } else if (strncmp(DC_NEW, cmdFormat, strlen(DC_NEW)) == 0) {
        rat = line->atTokNextint(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing rat type",
                    rid, __FUNCTION__);
            return PDN_FAILED;
        }
        updateRat(activatedAid, rat);

        transIntfId = line->atTokNextint(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing transIntfId",
                    rid, __FUNCTION__);
            return PDN_FAILED;
        }
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] push transIntfId %d to pdn table [%d]",
                rid, __FUNCTION__, transIntfId, activatedAid);
        updateTransIntfId(activatedAid, transIntfId);

        mtu = line->atTokNextint(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing mtu",
                    rid, __FUNCTION__);
            return PDN_FAILED;
        }
        updateMtu(activatedAid, mtu);

        pdnType = line->atTokNextint(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing addrType",
                    rid, __FUNCTION__);
            return PDN_FAILED;
        }

        addr1 = line->atTokNextstr(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing addr1",
                    rid, __FUNCTION__);
            return PDN_FAILED;
        }

        if (line->atTokHasmore()) {
            addr2 = line->atTokNextstr(&err);
            if (err < 0) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing addr2",
                        rid, __FUNCTION__);
                return PDN_FAILED;
            }
        }

        if (PDN_SUCCESS != updateIpAddress(pdnType, activatedAid, addr1, addr2)) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] updateIpAddress failed for activatedAid%d",
                    rid, __FUNCTION__, activatedAid);
            return PDN_FAILED;
        }
    } else if (strncmp(DC_UPDATE, cmdFormat, strlen(DC_UPDATE)) == 0) {
        transIntfId = line->atTokNextint(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing transIntfId",
                    rid, __FUNCTION__);
            return PDN_FAILED;
        }

        if (getTransIntfId(activatedAid) != transIntfId) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] transIntfId%d is different to MD assigned %d",
                    rid, __FUNCTION__, getTransIntfId(activatedAid), transIntfId);
            return PDN_FAILED;
        }

        pdnType = line->atTokNextint(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing addrType",
                    rid, __FUNCTION__);
            return PDN_FAILED;
        }

        addr1 = line->atTokNextstr(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing addr1", rid, __FUNCTION__);
            return PDN_FAILED;
        }

        if (line->atTokHasmore()) {
            addr2 = line->atTokNextstr(&err);
            if (err < 0) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing addr2",
                        rid, __FUNCTION__);
                return PDN_FAILED;
            }
        }

        if (PDN_SUCCESS != updateIpAddress(pdnType, activatedAid, addr1, addr2)) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] updateIpAddress failed for activatedAid%d",
                    rid, __FUNCTION__, activatedAid);
            return PDN_FAILED;
        }
    } else {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] no matched command format (%s)",
                rid, __FUNCTION__, cmdFormat);
        return PDN_FAILED;
    }

    return PDN_SUCCESS;
}

int RmcDcCommonReqHandler::updateIpAddress(int pdnType, int aid, char* addr1, char* addr2) {
    char addressV6[MAX_IPV6_ADDRESS_LENGTH] = {0};
    switch (pdnType) {
        case NETAGENT_ADDR_TYPE_IPV4: {
            updateIpv4Address(aid, addr1);
            break;
        }
        case NETAGENT_ADDR_TYPE_IPV6: {
            convertIpv6Address(addressV6, addr1, 0);
            updateIpv6Address(aid, addressV6);
            break;
        }
        case NETAGENT_ADDR_TYPE_IPV4V6: {
            updateIpv4Address(aid, addr1);
            convertIpv6Address(addressV6, addr2, 0);
            updateIpv6Address(aid, addressV6);
            break;
        }
        default:
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] no matched pdn type (%d)",
                    m_slot_id, __FUNCTION__, pdnType);
            return PDN_FAILED;
    }
    return PDN_SUCCESS;
}

void RmcDcCommonReqHandler::requestSetupDataCall(const sp<RfxMclMessage>& msg) {
    const char **pReqData = (const char **)msg->getData()->getData();
    sp<RfxAtResponse> p_response;
    const int nPdnInfoSize = getPdnTableSize();
    int isEmergency = 0;
    int rid = m_slot_id;
    int transIntfId = INVALID_TRANS_INTF_ID;
    Vector<int> vAidList;
    Vector<int>::iterator it;
    MTK_RIL_Data_Call_Response_v11* response = NULL;
    sp<RfxMclMessage> responseMsg;
    int num = 1;
    const char *pReqApn = pReqData[2] == NULL ? "" : pReqData[2];
    int profileId = atoi(pReqData[1]);
    int reason = ACT_REASON_AOSP_NORMAL;
    if (pReqData[15] == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] pReqData[15] = NULL, suggest to check why ril version"
                " is lower than V1.2", rid, __FUNCTION__);
    } else {
        reason = atoi(pReqData[15]);
    }
    RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] profileId=%d, reason=%d",
            rid, __FUNCTION__, profileId, reason);
    const char *pProfileType = RmcDcUtility::getProfileType(profileId);
    int protocol = RmcDcUtility::getProtocolType(pReqData[6]);

    ApnTableReq apnTableReq;
    int cmdIndex = 0;
    apnTableReq.apn = const_cast<char *>(pReqApn);
    apnTableReq.username = const_cast<char *>(pReqData[3]);
    apnTableReq.password = const_cast<char *>(pReqData[4]);
    apnTableReq.apnTypeId = RmcDcUtility::getApnTypeId(profileId);
    apnTableReq.protocol = const_cast<char *>(pReqData[6]);
    apnTableReq.authtype = atoi(pReqData[5]);
    cmdIndex = RmcDcCommonReqHandler::getCmdIndexFromApnTable(rid, &apnTableReq);

    RFX_LOG_V(RFX_LOG_TAG, "[%d][%s] apn=%s, profile=%s, protocol=%d",
            rid, __FUNCTION__, pReqApn, pProfileType, protocol);

    // Initialize data call response.
    response = (MTK_RIL_Data_Call_Response_v11*)calloc(1, sizeof(MTK_RIL_Data_Call_Response_v11));
    RFX_ASSERT(response != NULL);
    initDataCallResponse(response, num);

    if (!isDataAllowed(pReqApn)) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] isDataAllowed() return false!",
                rid, __FUNCTION__);
        response->status = m_nGprsFailureCause = PDP_FAIL_DATA_NOT_ALLOW;
        goto error;
    }

    if (isSscMode3Request(reason, pReqApn, &transIntfId)) {
        createDataResponse(transIntfId, IPV4V6, response);
        RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] isSscMode3Request: %s", rid, __FUNCTION__,
                responsesToString(response, num).string());
        goto finish;
    }

    if (isHandoverRequest(profileId, &transIntfId)) {
        createDataResponse(transIntfId, IPV4V6, response);
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] %s", rid, __FUNCTION__,
                responsesToString(response, num).string());
        goto finish;
    }

    // Reserve size of AID list.
    vAidList.reserve(nPdnInfoSize);

    /*Fix timing issue of IMS/EIMS PDN*/
    //IMS/EIMS PDN was already deactivated in MD,
    //while setup data call for IMS/EIMS PDN is still ongoing in AP fwk/ril.
    //Refer to ALPS04066280 and ALPS03861864 for detail scenario.
    if ((strncmp(pProfileType, "ims", 3) == 0) &&
        (getMclStatusManager(rid)->getIntValue(RFX_STATUS_KEY_IMS_PDN_STATE) == IMS_OR_EIMS_PDN_DEACTIVATED)) {
        RFX_LOG_I(RFX_LOG_TAG, "[%d] %s pdn is already deactivated at MD, shouldn't be activated", rid, pProfileType);
        goto error;
    } else if ((strncmp(pProfileType, "emergency", 9) == 0) &&
        (getMclStatusManager(rid)->getIntValue(RFX_STATUS_KEY_EIMS_PDN_STATE) == IMS_OR_EIMS_PDN_DEACTIVATED)) {
        RFX_LOG_I(RFX_LOG_TAG, "[%d] %s pdn is already deactivated at MD, shouldn't be activated", rid, pProfileType);
        goto error;
    }
    /*Fix timing issue of IMS/EIMS PDN*/

    // Activate PDN.
    if (PDN_SUCCESS != activatePdn(pReqApn, pProfileType, &vAidList, isEmergency, response, cmdIndex)) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] activatePdn failed for apn [name=\"%s\",type=\"%s\",cmdIndex=%d]",
                rid, __FUNCTION__, pReqApn, pProfileType, cmdIndex);
        goto error;
    }

    for (it = vAidList.begin(); it != vAidList.end(); it++) {
        int activatedAid = *it;
        // Wait for MD binding NW interface id and updating rat, interfaceId, mtu and address if needed.
        if (PDN_SUCCESS != updatePdnInformation(activatedAid, IPV4V6, NETAGENT_IFST_UP)) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] updatePdnInformation failed for activatedAid%d",
                    rid, __FUNCTION__, activatedAid);
            response->status = m_nGprsFailureCause = PDP_FAIL_ERROR_UNSPECIFIED;
            goto error;
        } else if (transIntfId == INVALID_TRANS_INTF_ID) {
            transIntfId = getTransIntfId(activatedAid);
        } else if (transIntfId != getTransIntfId(activatedAid)) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] Error occurs when same apn has two different transIntfIds!",
                    rid, __FUNCTION__);
            response->status = m_nGprsFailureCause = PDP_FAIL_ERROR_UNSPECIFIED;
            goto error;
        }
        setProfileId(activatedAid, profileId);
        updateApnName(activatedAid, pReqApn);

        if (PDN_SUCCESS != updateDefaultBearerInfo(activatedAid)) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] Error occurs when updateDefaultBearerInfo aid = %d!",
                    rid, __FUNCTION__, activatedAid);
            response->status = m_nGprsFailureCause = PDP_FAIL_ERROR_UNSPECIFIED;
            goto error;
        }
    }

    if (!isAllAidActive(&vAidList)) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] Error occurs when isAllAidActive!",
                rid, __FUNCTION__);
        response->status = m_nGprsFailureCause = PDP_FAIL_LOST_CONNECTION;
        goto error;
    }

    createDataResponse(transIntfId, IPV4V6, response);
    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] %s", rid, __FUNCTION__, responsesToString(response, num).string());

    if (RmcDcUtility::isOp12Support()) {
        sendEvent(RFX_MSG_EVENT_QUERY_PCO_WITH_AID, RfxIntsData(vAidList.begin(), sizeof(int)),
                RIL_CMD_PROXY_5, m_slot_id, -1, -1, 100e6);
    }
    sendEvent(RFX_MSG_EVENT_REQ_DATA_CONTEXT_IDS, RfxVoidData(),
            RIL_CMD_PROXY_5, m_slot_id, -1, -1, 100e6);
    setPdnInfoProperty();

    goto finish;

error:
    for (it = vAidList.begin(); it != vAidList.end(); it++) {
        int activatedAid = *it;
        deactivateDataCall(activatedAid);
        clearPdnInfo(activatedAid);
    }

finish:
    responseMsg = RfxMclMessage::obtainResponse(RIL_E_SUCCESS,
            RfxDataCallResponseData(response, num), msg);
    responseToTelCore(responseMsg);

    if (response != NULL) {
        if (profileId == RIL_DATA_PROFILE_VENDOR_MMS && response->status == PDP_FAIL_NONE) {
            int data[2];
            data[0] = RmcDcUtility::getApnTypeId(profileId);
            data[1] = response->rat;
            sendEvent(RFX_MSG_EVENT_SEND_QUALIFIED_NETWORK_TYPES_CHANGED, RfxIntsData(data, 2),
                    RIL_CMD_PROXY_5, m_slot_id, -1, -1, 500e6);
        }
        freeDataResponse(response);
        FREEIF(response);
    }
    RFX_LOG_V(RFX_LOG_TAG, "[%d][%s] finish", rid, __FUNCTION__);
}

// Default Bearer
void RmcDcCommonReqHandler::onNwPdnAct(const sp<RfxMclMessage>& msg) {
    // +CGEV: NW PDN ACT <aid>
    char *urc = (char*)msg->getData()->getData();
    int rid = m_slot_id;
    int err = 0;
    int aid = INVALID_AID;
    RfxAtLine *pLine = NULL;
    char *tempParam = NULL;
    sp<RfxMclMessage> urcMsg;

    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] urc = %s", rid, __FUNCTION__, urc);

    pLine = new RfxAtLine(urc, NULL);

    if (pLine == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to new pLine!",
                rid, __FUNCTION__);
        return;
    }

    pLine->atTokStart(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when token start",
                rid, __FUNCTION__);
        AT_LINE_FREE(pLine);
        return;
    }

    tempParam = pLine->atTokNextstr(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing aid",
                rid, __FUNCTION__);
        AT_LINE_FREE(pLine);
        return;
    }

    tempParam += strlen(NW_PDN_ACT);
    aid = RmcDcUtility::stringToBinaryBase(tempParam, 10, &err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when converting aid to binary, err = %d",
                rid, __FUNCTION__, err);
        AT_LINE_FREE(pLine);
        return;
    }

    // For the current design, we don't expect to use any default bearers which are established by NW.
    // However, if operator's requirements appear in the future, then the design should be rethinking.
    if (confirmPdnUsage(aid, false) != CME_SUCCESS) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] confirmPdnUsage failed for aid%d",
                rid, __FUNCTION__, aid);
    }

    AT_LINE_FREE(pLine);
}

// Default Bearer
void RmcDcCommonReqHandler::onNwPdnDeact(const sp<RfxMclMessage>& msg) {
    // +CGEV: NW PDN DEACT <aid>
    char *urc = (char*)msg->getData()->getData();
    int rid = m_slot_id;
    int err = 0;
    int aid = INVALID_AID;
    RfxAtLine *pLine = NULL;
    char *tempParam = NULL;
    const int nPdnInfoSize = getPdnTableSize();

    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] urc = %s", rid, __FUNCTION__, urc);

    pLine = new RfxAtLine(urc, NULL);

    if (pLine == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to new pLine!",
                rid, __FUNCTION__);
        return;
    }

    pLine->atTokStart(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when token start",
                rid, __FUNCTION__);
        AT_LINE_FREE(pLine);
        return;
    }

    tempParam = pLine->atTokNextstr(&err);
    if (err < 0){
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing aid",
                rid, __FUNCTION__);
        AT_LINE_FREE(pLine);
        return;
    }

    tempParam += strlen(NW_PDN_DEACT);
    aid = RmcDcUtility::stringToBinaryBase(tempParam, 10, &err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when converting aid to binary, err = %d",
                rid, __FUNCTION__, err);
        AT_LINE_FREE(pLine);
        return;
    }

    if (PDN_SUCCESS != updatePdnDeactInfo(aid)) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] updatePdnDeactInfo failed for aid%d",
                rid, __FUNCTION__, aid);
    }

    // Just telling DDM that AP will not use this aid to let DDM release it.
    if (confirmPdnUsage(aid, false) != CME_SUCCESS) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] confirmPdnUsage failed for aid%d",
                rid, __FUNCTION__, aid);
    }

    for (int i = 0; i < nPdnInfoSize; i++) {
        if (isDedicateBearer(i) && getPrimaryAid(i) == aid) {
            RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] clean dedicate pdn aid%d info "
                    "due to default pdn aid%d deactivated", rid, __FUNCTION__, getAid(i), aid);
            clearPdnInfo(i);
        }
    }

    if (notifyDeactReasonIfNeeded(aid)) {
        sendDataCallListResponse(msg, aid);
        clearPdnInfo(aid);
        AT_LINE_FREE(pLine);
        return;
    }

    clearPdnInfo(aid);
    sendDataCallListResponse(msg);
    requestGetDataContextIds(msg);
    setPdnInfoProperty();

    AT_LINE_FREE(pLine);
}

void RmcDcCommonReqHandler::onNwModify(const sp<RfxMclMessage>& msg) {
    // +CGEV: NW MODIFY <aid>, <change_reason>, <event_type>
    char *urc = (char*)msg->getData()->getData();
    int rid = m_slot_id;
    int err = 0;
    int aid = INVALID_AID;
    Vector<int> vAid;
    int defaultAid = INVALID_AID;
    int reason = 0;
    RfxAtLine *pLine = NULL;
    char *tempParam = NULL;

    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] urc = %s", rid, __FUNCTION__, urc);

    // Reserve size of AID.
    vAid.reserve(1);

    pLine = new RfxAtLine(urc, NULL);

    if (pLine == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to new pLine!",
                rid, __FUNCTION__);
        goto error;
    }

    pLine->atTokStart(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when token start",
                rid, __FUNCTION__);
        goto error;
    }

    tempParam = pLine->atTokNextstr(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing aid",
                rid, __FUNCTION__);
        goto error;
    }
    tempParam += strlen(NW_MODIFY);
    aid = RmcDcUtility::stringToBinaryBase(tempParam, 10, &err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when converting aid to binary"
                ", err = %d", rid, __FUNCTION__, err);
        goto error;
    }
    vAid.push_back(aid);

    reason = pLine->atTokNextint(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing reason",
                rid, __FUNCTION__);
        goto error;
    }

    setAid(aid, aid);
    defaultAid = getPrimaryAid(aid);

    if (defaultAid == aid) {
        setIsDedicateBearer(aid, 0);
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] not to update active status [aid=%d, active=%d]",
                rid, __FUNCTION__, aid, getPdnActiveStatus(aid));
    } else if (INVALID_AID != defaultAid) {
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] a dedicate bearer is modified [aid=%d]",
                rid, __FUNCTION__, aid);
        setIsDedicateBearer(aid, 1);
        updateTransIntfId(aid, getTransIntfId(defaultAid));
        updatePdnActiveStatus(aid, getPdnActiveStatus(defaultAid));
    }

    if (DATA_STATE_INACTIVE == isAllAidActive(&vAid)) {
        goto error;
    }

    if ((INVALID_AID != defaultAid) &&
            (DATA_STATE_ACTIVE == getPdnActiveStatus(defaultAid))) {
        if (defaultAid == aid) {
            requestOrSendDataCallList(msg, aid);
            RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] udpate default bearer %d thru DataCallListChanged",
                    rid, __FUNCTION__, aid);
        } else {
            RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] not to send URC of dedicate bearer %d activation",
                    rid, __FUNCTION__, aid);
        }
    } else {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] default bearer %d is not activated, "
                "not to send URC of dedicate bearer activation", rid, __FUNCTION__, defaultAid);
    }

    requestQueryPco(aid, 0, NULL, NULL);

error:
    AT_LINE_FREE(pLine);
}

void RmcDcCommonReqHandler::onNwReact(const sp<RfxMclMessage>& msg) {
    // +CGEV: NW REACT <PDP_type>, <PDP_addr>, [<aid>]
    int err = 0;
    int aid = INVALID_AID;
    char *out = NULL;
    char *urc = (char*)msg->getData()->getData();
    int rid = m_slot_id;
    const int nPdnInfoSize = getPdnTableSize();
    RfxAtLine *pLine = NULL;
    char *tempParam = NULL;
    bool isDedicatedBearer = false;

    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] urc = %s", rid, __FUNCTION__, urc);

    pLine = new RfxAtLine(urc, NULL);

    if (pLine == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to new pLine!",
                rid, __FUNCTION__);
        goto error;
    }

    pLine->atTokStart(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when token start",
                rid, __FUNCTION__);
        goto error;
    }

    tempParam = pLine->atTokNextstr(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing defaultAid",
                rid, __FUNCTION__);
        goto error;
    }

    if (tempParam != NULL && strlen(tempParam) > 0) {
        // Skip PDP type

        if (pLine->atTokHasmore()) {
            // Skip PDP address
            out = pLine->atTokNextstr(&err);
            if (err < 0) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing pdp address",
                        rid, __FUNCTION__);
                goto error;
            }
        }

        if (pLine->atTokHasmore()) {
            aid = pLine->atTokNextint(&err);
            if (err < 0) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing aid",
                        rid, __FUNCTION__);
                goto error;
            }

            if (aid < 0) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs due to invalid aid=%d",
                        rid, __FUNCTION__, aid);
                goto error;
            }

            for (int i = 0; i < nPdnInfoSize; i++) {
                if (isDedicateBearer(i) && getPrimaryAid(i) == aid) {
                    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] clean dedicate pdn aid%d info "
                            "due to default pdn aid%d deactivated", rid, __FUNCTION__, i, aid);
                    clearPdnInfo(i);
                }
            }

            isDedicatedBearer = isDedicateBearer(aid);
            clearPdnInfo(aid);
            if (isDedicatedBearer) {
                RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] aid%d is a dedicate bearer",
                        rid, __FUNCTION__, aid);
            } else {
                sendDataCallListResponse(msg);
            }
        }
    } else {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs on null/empty token string",
                rid, __FUNCTION__);
    }

error:
    AT_LINE_FREE(pLine);
}

// Default Bearer
void RmcDcCommonReqHandler::onMePdnAct(const sp<RfxMclMessage>& msg) {
    // +CGEV: ME PDN ACT <aid>[,<reason>[,<aid_other>[,<WLAN_Offload>[,<SSC>,<old_aid>]]]]
    char *urc = (char*)msg->getData()->getData();
    int rid = m_slot_id;
    int activatedAid = INVALID_AID;
    int reason = NO_CAUSE;
    int otherAid = INVALID_AID;
    int wlanOffload = INVALID_VALUE;
    int sscMode = SSC_UNKNOWN;
    int oldAid = INVALID_AID;
    int olderAid = INVALID_AID;
    int newerAid = INVALID_AID;
    int err = 0;
    char *tempParam = NULL;
    RfxAtLine *pLine = NULL;
    RfxAtLine *p_cur = NULL;
    sp<RfxAtResponse> p_response;

    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] urc = %s", rid, __FUNCTION__, urc);

    pLine = new RfxAtLine(urc, NULL);

    if (pLine == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to new pLine!",
                rid, __FUNCTION__);
        goto error;
    }

    pLine->atTokStart(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when token start",
                rid, __FUNCTION__);
        goto error;
    }

    // aid
    tempParam = pLine->atTokNextstr(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing aid",
                rid, __FUNCTION__);
        goto error;
    }
    tempParam += strlen(ME_PDN_ACT);
    activatedAid = RmcDcUtility::stringToBinaryBase(tempParam, 10, &err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when converting activatedAid to binary"
                ", err = %d", rid, __FUNCTION__, err);
        goto error;
    }

    // reason
    if (pLine->atTokHasmore()) {
        reason = pLine->atTokNextint(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing reason for aid%d",
                    rid, __FUNCTION__, activatedAid);
            reason = NO_CAUSE;
        }
    }

    // other aid
    if (pLine->atTokHasmore()) {
        otherAid = pLine->atTokNextint(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing otherAid for aid%d",
                    rid, __FUNCTION__, activatedAid);
            otherAid = INVALID_AID;
        }
    }

    // WLAN offload
    if (pLine->atTokHasmore()) {
        wlanOffload = pLine->atTokNextint(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing wlanOffload for aid%d",
                    rid, __FUNCTION__, activatedAid);
            wlanOffload = INVALID_VALUE;
        }
    }

    // SSC mode
    if (pLine->atTokHasmore()) {
        sscMode = pLine->atTokNextint(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing sscType for aid%d",
                    rid, __FUNCTION__, activatedAid);
            sscMode = SSC_UNKNOWN;
        }
    }

    // old aid
    if (pLine->atTokHasmore()) {
        oldAid = pLine->atTokNextint(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing oldAid for aid%d",
                    rid, __FUNCTION__, activatedAid);
            oldAid = INVALID_AID;
        }
    }

    RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] aid%d is activated and current state is %d",
            rid, __FUNCTION__, activatedAid, getPdnActiveStatus(activatedAid));

    if (sscMode == SSC_MODE2) {
        // Currently, SSC mode 2 is already supported by AOSP, we don't need to handle it.
        // The flow is as follows:
        // 1. Receive network pdn deact urc.
        // 2. Unsolicited data call list change.
        // 3. Android framework retry data connection.
        goto error;
    } else if (sscMode == SSC_MODE3) {
        olderAid = oldAid;
        newerAid = activatedAid;
    } else {
        olderAid = activatedAid;
        newerAid = otherAid;
    }

    if (olderAid != INVALID_AID && getPdnActiveStatus(olderAid) == DATA_STATE_INACTIVE) {
        updatePdnActiveStatus(olderAid, DATA_STATE_LINKDOWN);  // Update with link down state.
        setAidAndPrimaryAid(olderAid, olderAid, olderAid);
        setReason(olderAid, reason);
    }

    // For IPv4v6 fallback, ex. +CGEV: ME PDN ACT 1, 2, 4
    // For SSC mode3 new PDU, ex. +CGEV: ME PDN ACT 4,,,,2,1
    if (newerAid != INVALID_AID) {
        if ((getPdnActiveStatus(olderAid) != DATA_STATE_ACTIVE) ||
                (sscMode == SSC_MODE3 && !RmcDcUtility::isSupportSscMode(3))) {
            // confirm with DDM that AP will not use this otherAid.
            if (confirmPdnUsage(newerAid, false, sscMode) != CME_SUCCESS) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] confirmPdnUsage failed for newerAid%d",
                        rid, __FUNCTION__, newerAid);
            }
        } else {
            // confirm with DDM that AP will use this otherAid.
            if (confirmPdnUsage(newerAid, true, sscMode) != CME_SUCCESS) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] confirmPdnUsage failed for newerAid%d",
                        rid, __FUNCTION__, newerAid);
            }

            // update the TransIntfId of otherAid
            if (otherAid != INVALID_AID && newerAid == otherAid) {
                setAidAndPrimaryAid(newerAid, newerAid, newerAid);
                updateTransIntfId(newerAid, getTransIntfId(olderAid));
            }

            // Update rat, interfaceId, mtu and address if needed.
            if (PDN_SUCCESS != updatePdnInformation(newerAid, IPV4V6, NETAGENT_IFST_NONE)) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] updatePdnInformation failed for newerAid%d",
                        rid, __FUNCTION__, newerAid);
                goto error;
            }

            if (PDN_SUCCESS != updateDefaultBearerInfo(newerAid)) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] updateDefaultBearerInfo failed for newerAid%d",
                        rid, __FUNCTION__, newerAid);
                goto error;
            }

            updateActiveStatus();
            if (DATA_STATE_INACTIVE == getPdnActiveStatus(newerAid)) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] newerAid%d is inactive",
                        rid, __FUNCTION__, newerAid);
                goto error;
            }

            if (sscMode == SSC_MODE3) {
                setSscMode(newerAid, sscMode);
                setProfileId(newerAid, getProfileId(olderAid));
                updateApnName(newerAid, getApnName(olderAid));
                updatePdnActiveStatus(newerAid, DATA_STATE_LINKDOWN);
                sendDataCallListResponse(msg, INVALID_AID, sscMode);
            } else {
                sendDataCallListResponse(msg);
            }

            requestQueryPco(olderAid, 0, NULL, NULL);
        }
    }

error:
    AT_LINE_FREE(pLine);
}

// Default Bearer
void RmcDcCommonReqHandler::onMePdnDeact(const sp<RfxMclMessage>& msg) {
    // R12: +CGEV: ME PDN DEACT <aid>
    int err = 0;
    int aid = INVALID_AID;
    char *urc = (char*)msg->getData()->getData();
    int rid = m_slot_id;
    const int nPdnInfoSize = getPdnTableSize();
    RfxAtLine *pLine = NULL;
    char *tempParam = NULL;

    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] urc = %s", rid, __FUNCTION__, urc);

    pLine = new RfxAtLine(urc, NULL);

    if (pLine == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to new pLine!",
                rid, __FUNCTION__);
        return;
    }

    pLine->atTokStart(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when token start",
                rid, __FUNCTION__);
        AT_LINE_FREE(pLine);
        return;
    }

    tempParam = pLine->atTokNextstr(&err);
    if (err < 0){
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing defaultAid",
                rid, __FUNCTION__);
        AT_LINE_FREE(pLine);
        return;
    }

    tempParam += strlen(ME_PDN_DEACT);
    aid = RmcDcUtility::stringToBinaryBase(tempParam, 10, &err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when converting aid "
                "to binary, err = %d", rid, __FUNCTION__, err);
        AT_LINE_FREE(pLine);
        return;
    }

    // Just telling DDM that AP will not use this aid to let DDM release it.
    if (confirmPdnUsage(aid, false) != CME_SUCCESS) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] confirmPdnUsage failed for aid%d",
                rid, __FUNCTION__, aid);
    }

    for (int i = 0; i < nPdnInfoSize; i++) {
        if (isDedicateBearer(i) && getPrimaryAid(i) == aid) {
            RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] clean dedicate pdn aid%d info "
                    "due to default pdn aid%d deactivated", rid, __FUNCTION__, getAid(i), aid);
            clearPdnInfo(i);
        }
    }

    clearPdnInfo(aid);
    sendDataCallListResponse(msg);
    requestGetDataContextIds(msg);
    setPdnInfoProperty();

    AT_LINE_FREE(pLine);
}

void RmcDcCommonReqHandler::onPdnChange(const sp<RfxMclMessage>& msg) {
    //+EPDN: <aid>,"dcchg",<event>
    char *urc = (char*)msg->getData()->getData();
    int rid = m_slot_id;
    int err = 0;
    int aid = INVALID_AID;
    char *cmdFormat = NULL;
    int event = DC_EVENT_UNKNOWN;
    RfxAtLine *pLine = NULL;
    bool needCheckmore = false;
    sp<RfxMclMessage> urcMsg;

    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] urc = %s", rid, __FUNCTION__, urc);

    pLine = new RfxAtLine(urc, NULL);

    if (pLine == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to new pLine!",
                rid, __FUNCTION__);
        goto error;
    }

    pLine->atTokStart(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when token start",
                rid, __FUNCTION__);
        goto error;
    }

    aid = pLine->atTokNextint(&err);
    if (err < 0) {
        if (pLine->atTokHasmore()) {
            RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] Has command data, need parsing more",
                    rid, __FUNCTION__);
            // For 93 data retry, if aid is null and has command data
            // We need to check if it is "+EPDN:,"dcchg",129"
            needCheckmore = true;
        } else {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing aid",
                    rid, __FUNCTION__);
            goto error;
        }
    }

    if (!needCheckmore && getTransIntfId(aid) == INVALID_TRANS_INTF_ID) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs due to original"
                " transIntfId removed", rid, __FUNCTION__);
        goto error;
    }

    cmdFormat = pLine->atTokNextstr(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing cmd",
                rid, __FUNCTION__);
        goto error;
    }

    if (strncmp(DC_CHANGE, cmdFormat, strlen(DC_CHANGE)) == 0) {
        event = pLine->atTokNextint(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing event",
                    rid, __FUNCTION__);
        }

        if(needCheckmore && event != DC_EVENT_DATA_RESET_NOTIFY) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] Have checked more but event is NOT 129",
                rid, __FUNCTION__);
            goto error;
        }

        switch (event) {
            case DC_EVENT_NO_RA:
                /* Clear IPv6 addresse by aid due to RA fail */
                for (int i = 0; i < MAX_NUM_IPV6_ADDRESS_NUMBER; i++) {
                    updateIpv6Address(aid, i, "");
                }
                sendDataCallListResponse(msg);
                break;
            case DC_EVENT_DATA_RESET_NOTIFY:
                // M: Data Retry, MD notify data count reset
                // +EPDN:,"dcchg",129
                urcMsg = RfxMclMessage::obtainUrc(RFX_MSG_URC_MD_DATA_RETRY_COUNT_RESET, m_slot_id,
                        RfxVoidData());
                responseToTelCore(urcMsg);
                break;
            case DC_EVENT_IP_NORMAL_CHANGE:
                onMePdnPropertyChange(msg, aid);
                break;
            case DC_EVENT_HANDOVER_NORMAL_CHANGE:
                onMePdnPropertyChange(msg, aid);
                sendQualifiedNetworkTypesChanged(aid);
                break;
            default:
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] no matched event (%d)",
                        rid, __FUNCTION__, event);
                break;
        }
    }

error:
    AT_LINE_FREE(pLine);
}

void RmcDcCommonReqHandler::onMePdnPropertyChange(const sp<RfxMclMessage>& msg, const int aid) {
    // Handle any PDN link property changing.
    // First use AT+EPDN to confirm that new IP address (if IP changed) already been configured,
    // then query PDN information and sync data call list.
    int rid = m_slot_id;

    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] aid%d", rid, __FUNCTION__, aid);

    // Because this event was sent from URC channel to Data channel asynchronously, need to
    // confirm active status before do other things.
    updateActiveStatus();
    if (DATA_STATE_INACTIVE == getPdnActiveStatus(aid)) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] aid%d is inactive", rid, __FUNCTION__, aid);
        return;
    }

    if (PDN_SUCCESS != updatePdnAddress(aid)) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] updatePdnAddress failed for aid%d",
                rid, __FUNCTION__, aid);
        return;
    }

    if (PDN_SUCCESS != updateDefaultBearerInfo(aid)) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] updateDefaultBearerInfo failed for aid%d",
                rid, __FUNCTION__, aid);
        return;
    }

    sendDataCallListResponse(msg);
}

// Support IWLAN AP-ASSISTED mode
void RmcDcCommonReqHandler::onQualifiedNetworkTypeChanged(const sp<RfxMclMessage>& msg) {
    //Upeate Inter Wireless LAN Priority List
    //+EIWLPL:<cmd>,<type>,<priority for setup>,<priority for cellular>,<priority for wifi>,<description>
    //  <cmd> : 'notify'
    //  <type>: 'ims' or 'emergency' or 'mms
    //  <priority for setup> : 1 ~ 5
    //  <priority for cellular> : 1 ~ 5
    //  <priority for wifi> : 1 ~ 5
    //  <descrption> : just for log print, e.g. wifi > cellular

    //  value of priority
    //       0: start
    //       1: null
    //       2: cellular
    //       3: wifi
    //       4: cellular > wifi
    //       5: wifi > cellular
    //       6: end
    char *urc = (char*)msg->getData()->getData();
    int rid = m_slot_id;
    int err = 0;
    char *cmdFormat = NULL;
    int event = DC_EVENT_UNKNOWN;
    bool needCheckmore = false;
    sp<RfxMclMessage> urcMsg;

    bool needToNotify = true;
    int mPdnTableSize = getPdnTableSize();
    PdnInfo pdnInfo;

    std::unique_ptr<RfxAtLine> pLine(new RfxAtLine(urc, NULL));
    RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] urc=%s", rid, __FUNCTION__, urc);
    pLine->atTokStart(&err);
    if (err < 0) return;

    cmdFormat = pLine->atTokNextstr(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing cmd",
                rid, __FUNCTION__);
        return;
    }

    if (strncmp("notify", cmdFormat, strlen("notify")) == 0) {
        int priorityListForSetup = 0;
        int priorityListForCellular = 0;
        int priorityListForWifi = 0;
        char *type = NULL;
        char *descrption = NULL;

        int matchApnType = -1;
        int supportNetworkTypes = 0;
        char tempApnTypeStr[MAX_AT_CMD_LENGTH] = {0};

        type = pLine->atTokNextstr(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing type",
                    rid, __FUNCTION__);
            return;
        }
        priorityListForSetup = pLine->atTokNextint(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing priorityListForSetup",
                    rid, __FUNCTION__);
            return;
        }
        priorityListForCellular = pLine->atTokNextint(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing priorityListForCellular",
                    rid, __FUNCTION__);
            return;
        }
        priorityListForWifi = pLine->atTokNextint(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing priorityListForWifi",
                    rid, __FUNCTION__);
            return;
        }
        descrption = pLine->atTokNextstr(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing descrption",
                    rid, __FUNCTION__);
            return;
        }

        // get str of type to campare
        RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] type=%s", rid, __FUNCTION__, type);
        for (int i = 0; i < MAX_COUNT_QUALIFIED_NETWORK_UPDATE_APNS; i++) {
            memset(tempApnTypeStr, 0, MAX_AT_CMD_LENGTH);
            strncpy(tempApnTypeStr,
                    RmcDcUtility::getApnType(SUPPORT_QUALIFIED_NETWORK_UPDATE_APNS[i]),
                    MAX_AT_CMD_LENGTH - 1);
            if (strncmp(tempApnTypeStr, type, strlen(tempApnTypeStr)) == 0){
                matchApnType = SUPPORT_QUALIFIED_NETWORK_UPDATE_APNS[i];
                RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] matchApnType=%d", rid, __FUNCTION__, matchApnType);
                break;
            }
        }

        for (int i = 0; i < mPdnTableSize; i++) {
            int currentAid = getAid(i);
            if (currentAid != INVALID_AID) {
                pdnInfo = getPdnInfo(currentAid);
                RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] pdnInfo.profileId=%d, pdn_info.active=%d",
                        rid, __FUNCTION__, pdnInfo.profileId, pdnInfo.active);
                if ((matchApnType == RmcDcUtility::getApnTypeId(pdnInfo.profileId))
                        && (pdnInfo.active == DATA_STATE_ACTIVE)) {
                    needToNotify = false;
                    break;
                }
            }
        }
        if (needToNotify) {
            if (matchApnType > 0) {
                bool urcWith2Types = true;
                int urc1Type[3] = {1, matchApnType | RIL_APN_TYPE_MMS | RIL_APN_TYPE_XCAP, ACCESS_NW_TYPE_UNKNOWN};
                int urc2Types[4] = {1, matchApnType | RIL_APN_TYPE_MMS | RIL_APN_TYPE_XCAP, ACCESS_NW_TYPE_UNKNOWN,
                        ACCESS_NW_TYPE_UNKNOWN};
                RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] priorityListForSetup=%d", rid, __FUNCTION__,
                        priorityListForSetup);
                switch(priorityListForSetup) {
                    case PRIORITY_CELLULAR_ONLY:
                        urc1Type[2] = ACCESS_NW_TYPE_CELLULAR;
                        break;

                    case PRIORITY_WIFI_ONLY:
                        urc1Type[2] = ACCESS_NW_TYPE_IWLAN;
                        break;

                    case PRIORITY_CELLULAR_PREFER:
                    case PRIORITY_WIFI_PREFER:
                        supportNetworkTypes = getSupportNetworkTypes(matchApnType);
                        RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] supportNetworkTypes=%d",
                                rid, __FUNCTION__, supportNetworkTypes);
                        if (supportNetworkTypes == SUPPORT_TRANSPORT_MODE_CELLULAR_WIFI) {
                            if (priorityListForSetup == PRIORITY_CELLULAR_PREFER) {
                                urc2Types[2] = ACCESS_NW_TYPE_CELLULAR;
                                urc2Types[3] = ACCESS_NW_TYPE_IWLAN;
                            } else {
                                urc2Types[2] = ACCESS_NW_TYPE_IWLAN;
                                urc2Types[3] = ACCESS_NW_TYPE_CELLULAR;
                            }
                            urcWith2Types = false;
                        } else if (supportNetworkTypes > SUPPORT_TRANSPORT_MODE_UNKNOW) {
                            if (supportNetworkTypes == SUPPORT_TRANSPORT_MODE_CELLULAR_ONLY) {
                                urc1Type[2] = ACCESS_NW_TYPE_CELLULAR;
                            } else {
                                urc1Type[2] = ACCESS_NW_TYPE_IWLAN;
                            }
                        } else {
                             RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] supportNetworkTypes unknown: %d",
                                     rid, __FUNCTION__, supportNetworkTypes);
                        }
                        break;

                    default:
                        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] priority list for setup unknown: %d",
                                    rid, __FUNCTION__, priorityListForSetup);
                        return;
                }

                if (urcWith2Types) {
                    RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] urcWith2Types: %d",
                                rid, __FUNCTION__, urc1Type[2]);
                    urcMsg = RfxMclMessage::obtainUrc(RFX_MSG_URC_QUALIFIED_NETWORK_TYPES_CHANGED,
                            m_slot_id, RfxIntsData(urc1Type, 3));
                    responseToTelCore(urcMsg);
                } else {
                    RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] urcWith3Types: %d",
                                rid, __FUNCTION__, urc2Types[2]);
                    urcMsg = RfxMclMessage::obtainUrc(RFX_MSG_URC_QUALIFIED_NETWORK_TYPES_CHANGED,
                            m_slot_id, RfxIntsData(urc2Types, 4));
                    responseToTelCore(urcMsg);
                }

            } else {
                    RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] apn=%s not support qualified network update!",
                            rid, __FUNCTION__, type);
            }
        } else {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] apn state is active skip to notify",
                    rid, __FUNCTION__);
        }
    } else {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] unknown <cmd> for QualifiedNetworkTypeChanged!",
                rid, __FUNCTION__);
    }
    return;
}

void RmcDcCommonReqHandler::requestDeactivateDataCall(const sp<RfxMclMessage>& msg) {
    const char **pReqData = (const char **)msg->getData()->getData();
    int transIntfId = atoi(pReqData[0]);
    int reason = atoi(pReqData[1]);
    int rid = m_slot_id;
    int err = 0;
    const int nPdnInfoSize = getPdnTableSize();
    sp<RfxMclMessage> responseMsg;

    RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] transIntfId=%d, reason: %d",
            rid, __FUNCTION__, transIntfId, reason);

    if (reason != DEACT_REASON_AOSP_HANDOVER && transIntfId > INVALID_TRANS_INTF_ID) {
        for (int i = 0; i < nPdnInfoSize; i++) {
            if ((getTransIntfId(i) == transIntfId) && !isDedicateBearer(i)) {
                // Error handling if needed.
                err = deactivateDataCall(getAid(i), reason);

                // Reset pdn info with current aid.
                clearPdnInfo(getAid(i));
            }
        }
    }

    sendEvent(RFX_MSG_EVENT_REQ_DATA_CONTEXT_IDS, RfxVoidData(),
            RIL_CMD_PROXY_5, m_slot_id, -1, -1, 100e6);
    setPdnInfoProperty();

    responseMsg = RfxMclMessage::obtainResponse(RIL_E_SUCCESS, RfxVoidData(), msg);
    responseToTelCore(responseMsg);
}

int RmcDcCommonReqHandler::deactivateDataCall(const int aid) {
    return deactivateDataCall(aid, DEACT_REASON_UNKNOWN);
}

int RmcDcCommonReqHandler::deactivateDataCall(const int aid, const int reason) {
    int rid = m_slot_id;
    int err = 0;
    int respAid = INVALID_AID;
    char *tmpStr = NULL;
    char *tmpAid = NULL;
    String8 cmd("");
    RfxAtLine *pLine = NULL;
    sp<RfxAtResponse> p_response;

    // Check if AID is out of bound.
    if (!validateAid(aid)) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] aid=%d is out of bound",
                rid, __FUNCTION__, aid);
        return CME_ERROR_NON_CME;
    }

    // AT+EAPNACT=<state>,<aid>[,<reason>];
    // <state>: 0-deactivate; 1-activate
    // <reason>: see PDN_DEACT_REASON in RmcDataDefs.h for more details
    cmd.append(String8::format("AT+EAPNACT=0,%d,%d", aid, convertDeactCauseToMdStandard(reason)));
    p_response = atSendCommandSingleline(cmd, "+CGEV: ME PDN DEACT ");

    if (p_response == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get p_response!",
                rid, __FUNCTION__);
        return CME_ERROR_NON_CME;
    }

    if (p_response->isAtResponseFail()) {
        if (p_response->getSuccess() == 0) {
            int cause = p_response->atGetCmeError();
            RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] CME CAUSE : %d",
                    rid, __FUNCTION__, cause);
            return cause;
        } else {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] CME CAUSE : CME_UNKNOWN",
                    rid, __FUNCTION__);
            return CME_UNKNOWN;
        }
    }

    pLine = p_response->getIntermediates();

    if (pLine == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get intermediate results!",
                rid, __FUNCTION__);
        return CME_ERROR_NON_CME;
    }

    pLine->atTokStart(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when token start",
                rid, __FUNCTION__);
        return CME_ERROR_NON_CME;
    }

    tmpStr = pLine->atTokNextstr(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing aid",
                rid, __FUNCTION__);
        return CME_ERROR_NON_CME;
    }
    tmpAid = tmpStr + strlen(ME_PDN_DEACT);
    respAid = RmcDcUtility::stringToBinaryBase(tmpAid, 10, &err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when converting aid to binary"
                ", err = %d", rid, __FUNCTION__, err);
        return CME_ERROR_NON_CME;
    }

    if (respAid != aid) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] aid%d is different to MD assigned aid%d",
                rid, __FUNCTION__, aid, respAid);
        return CME_ERROR_NON_CME;
    }

    return CME_SUCCESS;
}

void RmcDcCommonReqHandler::requestSyncApnTable(const sp<RfxMclMessage>& msg) {
    int rid = m_slot_id;
    sp<RfxMclMessage> response;
    RIL_MtkDataProfileInfo **oldDataProfilePtrs =
                (RIL_MtkDataProfileInfo **)msg->getData()->getData();
    int nOldNum = msg->getData()->getDataLength() / sizeof(RIL_MtkDataProfileInfo*);

    if (nOldNum <= 0) {
        response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
                RfxVoidData(), msg, false);
        responseToTelCore(response);
        return;
    }

    RIL_MtkDataProfileInfo **dataProfilePtrs = (RIL_MtkDataProfileInfo**) calloc(
            nOldNum + MAX_COUNT_EXTRA_APN, sizeof(RIL_MtkDataProfileInfo*));
    RFX_ASSERT(dataProfilePtrs != NULL);
    int nReqNum = createDataProfiles(oldDataProfilePtrs, dataProfilePtrs, nOldNum);

    sp<RfxAtResponse> p_response;
    char *line = NULL, *token = NULL;
    char strParam[MAX_AT_CMD_LENGTH] = {0};
    char tmpStrParam[MAX_AT_CMD_LENGTH] = {0};
    String8 cmd("");

    // DataProfile info
    int profileId = 0;
    int authType = 0;
    int type = 0;
    int maxConnsTime = 0;
    int maxConns = 0;
    int waitTime = 0;
    int enabled = 0;
    int supportedTypesBitmask = 0;
    long int bearerBitmask = 0;
    int mtu = 0;
    int inactiveTimer = 0;

    // local ver.
    int cmdIndex = 1;
    char ApnTypeStr[MAX_AT_CMD_LENGTH] = {0};
    char tempApnTypeStr[MAX_AT_CMD_LENGTH] = {0};
    int countOfApnType = 0;
    char apn[MAX_APN_NAME_LENGTH] = {0};
    char user[MAX_APN_NAME_LENGTH] = {0};
    char password[MAX_APN_NAME_LENGTH] = {0};

    // Default IMS apn setting
    int hasImsApn = -1;

    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] num of pararmeters = %d", rid, __FUNCTION__, nReqNum);

    // avoid syncing the same apn table, e.g. sub id ready then record loaded event
    if (s_LastApnTable[m_slot_id] != NULL) {
        bool bSameApnTable = true;
        if (s_nLastReqNum[m_slot_id] == nReqNum) {
            for (int i = 0; i < s_nLastReqNum[m_slot_id]; i++) {
                RIL_MtkDataProfileInfo *preProfile = &s_LastApnTable[m_slot_id][i];
                RIL_MtkDataProfileInfo *curProfile = dataProfilePtrs[i];
                if (preProfile->profileId != curProfile->profileId
                        || 0 != strcmp(preProfile->apn, curProfile->apn)
                        || 0 != strcmp(preProfile->protocol, curProfile->protocol)
                        || 0 != strcmp(preProfile->roamingProtocol, curProfile->roamingProtocol)
                        || preProfile->authType != curProfile->authType
                        || 0 != strcmp(preProfile->user, curProfile->user)
                        || 0 != strcmp(preProfile->password, curProfile->password)
                        || preProfile->maxConnsTime != curProfile->maxConnsTime
                        || preProfile->maxConns != curProfile->maxConns
                        || preProfile->waitTime != curProfile->waitTime
                        || preProfile->enabled != curProfile->enabled
                        || preProfile->supportedTypesBitmask != curProfile->supportedTypesBitmask
                        || preProfile->inactiveTimer != curProfile->inactiveTimer) {
                    bSameApnTable = false;
                    break;
                }
            }
        } else {
            bSameApnTable = false;
        }
        if (bSameApnTable) {
            RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] ignore same apn table sync",
                    rid, __FUNCTION__);
            response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
                    RfxVoidData(), msg, false);

            // Make IA the same as used to be.
            // Once the Set_Data_Profile triggered, send the EIAAPN even if EAPNSET is ignored.
            RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] Sending event: RFX_MSG_EVENT_QUERY_SELF_IA",
                    rid, __FUNCTION__);
            sendEvent(RFX_MSG_EVENT_QUERY_SELF_IA, RfxVoidData(),
                    RIL_CMD_PROXY_5, m_slot_id);
            responseToTelCore(response);

            deleteDataProfiles(dataProfilePtrs, nOldNum + MAX_COUNT_EXTRA_APN);
            return;
        } else {
            // reset Last APN table
            for (int i = 0; i < s_nLastReqNum[m_slot_id]; i++) {
                FREEIF(s_LastApnTable[m_slot_id][i].apn);
                FREEIF(s_LastApnTable[m_slot_id][i].protocol);
                FREEIF(s_LastApnTable[m_slot_id][i].roamingProtocol);
                FREEIF(s_LastApnTable[m_slot_id][i].user);
                FREEIF(s_LastApnTable[m_slot_id][i].password);
                FREEIF(s_LastApnTable[m_slot_id][i].mvnoType);
                FREEIF(s_LastApnTable[m_slot_id][i].mvnoMatchData);
            }
            FREEIF(s_ApnCmdIndex[m_slot_id]);
            FREEIF(s_LastApnTable[m_slot_id]);
        }
    }

    // init s_nLastReqNum, s_ApnCmdIndex, and s_LastApnTable
    s_nLastReqNum[m_slot_id] = nReqNum;
    s_ApnCmdIndex[m_slot_id] = (int*)calloc(s_nLastReqNum[m_slot_id], sizeof(int));
    s_LastApnTable[m_slot_id] = (RIL_MtkDataProfileInfo*)calloc(s_nLastReqNum[m_slot_id],
            sizeof(RIL_MtkDataProfileInfo));

    // lock the apn table
    p_response = atSendCommand("AT+EAPNLOCK=1");

    if (p_response == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] 1. fail to get p_response!",
                rid, __FUNCTION__);
        goto error;
    }

    if (p_response->isAtResponseFail()) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] AT+EAPNLOCK=1 returns ERROR",
                rid, __FUNCTION__);
        goto error;
    }

    // clear the apn table
    p_response = atSendCommand("AT+EAPNSET");

    if (p_response == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] 2. fail to get p_response!",
                rid, __FUNCTION__);
        goto error;
    }

    if (p_response->isAtResponseFail()) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] AT+EAPNSET clear apn table ERROR",
                rid, __FUNCTION__);
        goto error;
    }

    for (int i = 0; i < nReqNum; i++) {
        profileId = dataProfilePtrs[i]->profileId;
        authType = RmcDcUtility::getAuthType(dataProfilePtrs[i]->authType);
        type = dataProfilePtrs[i]->type;
        maxConnsTime = dataProfilePtrs[i]->maxConnsTime;
        maxConns = dataProfilePtrs[i]->maxConns;
        waitTime = dataProfilePtrs[i]->waitTime;
        enabled = dataProfilePtrs[i]->enabled;
        supportedTypesBitmask = dataProfilePtrs[i]->supportedTypesBitmask;
        bearerBitmask = dataProfilePtrs[i]->bearerBitmask;
        mtu = dataProfilePtrs[i]->mtu;
        inactiveTimer = dataProfilePtrs[i]->inactiveTimer;

        // update Last APN table
        s_LastApnTable[m_slot_id][i].profileId = profileId;
        asprintf(&s_LastApnTable[m_slot_id][i].apn, "%s", dataProfilePtrs[i]->apn);
        asprintf(&s_LastApnTable[m_slot_id][i].protocol, "%s", dataProfilePtrs[i]->protocol);
        asprintf(&s_LastApnTable[m_slot_id][i].roamingProtocol, "%s",
                dataProfilePtrs[i]->roamingProtocol);
        s_LastApnTable[m_slot_id][i].authType = dataProfilePtrs[i]->authType;  //backup raw data
        asprintf(&s_LastApnTable[m_slot_id][i].user, "%s", dataProfilePtrs[i]->user);
        asprintf(&s_LastApnTable[m_slot_id][i].password, "%s", dataProfilePtrs[i]->password);
        s_LastApnTable[m_slot_id][i].type = type;
        s_LastApnTable[m_slot_id][i].maxConnsTime = maxConnsTime;
        s_LastApnTable[m_slot_id][i].maxConns = maxConns;
        s_LastApnTable[m_slot_id][i].waitTime = waitTime;
        s_LastApnTable[m_slot_id][i].enabled = enabled;
        s_LastApnTable[m_slot_id][i].supportedTypesBitmask = supportedTypesBitmask;
        s_LastApnTable[m_slot_id][i].bearerBitmask = bearerBitmask;
        s_LastApnTable[m_slot_id][i].mtu = mtu;
        asprintf(&s_LastApnTable[m_slot_id][i].mvnoType, "%s", dataProfilePtrs[i]->mvnoType);
        asprintf(&s_LastApnTable[m_slot_id][i].mvnoMatchData, "%s",
                dataProfilePtrs[i]->mvnoMatchData);
        s_LastApnTable[m_slot_id][i].inactiveTimer = inactiveTimer;

        cmdIndex = 1;

        for (int j = 0; j < i; j++) {
            if (0 == strcasecmp(dataProfilePtrs[i]->apn, dataProfilePtrs[j]->apn)) {
                cmdIndex += CMD_INDEX_OFFSET;
            }
        }
        s_ApnCmdIndex[m_slot_id][i] = cmdIndex;

        // To sync IMS/Eemergency PDN default mtu config to MD -- START
        if ((supportedTypesBitmask & RIL_APN_TYPE_IMS) == RIL_APN_TYPE_IMS) {
            cmd.clear();
            cmd.append(String8::format("AT+EPDNCFG=\"ims_mtu\",%d",mtu));
            p_response = atSendCommand(cmd);
        }

        if ((supportedTypesBitmask & RIL_APN_TYPE_EMERGENCY) == RIL_APN_TYPE_EMERGENCY) {
            cmd.clear();
            cmd.append(String8::format("AT+EPDNCFG=\"eims_mtu\",%d",mtu));
            p_response = atSendCommand(cmd);
        }
        // To sync IMS/Eemergency PDN default mtu config to MD -- END

        // update the user name and password
        // AT+EAPNSET = "apn",cmdIndex,"user","pwd"
        strncpy(apn, dataProfilePtrs[i]->apn, MAX_APN_NAME_LENGTH-1);
        addEscapeSequence(apn);
        strncpy(user, dataProfilePtrs[i]->user, MAX_APN_NAME_LENGTH-1);
        addEscapeSequence(user);
        strncpy(password, dataProfilePtrs[i]->password, MAX_APN_NAME_LENGTH-1);
        addEscapeSequence(password);
        cmd.clear();
        cmd.append(String8::format("AT+EAPNSET=\"%s\",%d,\"%s\",\"%s\"",
                apn, cmdIndex, user, password));
        p_response = atSendCommand(cmd);

        if (p_response == NULL) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] 3. fail to get p_response!",
                    rid, __FUNCTION__);
            goto error;
        }

        if (p_response->isAtResponseFail()) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] AT+EAPNSET update user name and password ERROR",
                    rid, __FUNCTION__);
            goto error;
        }

        // conver supportedTypesBitmask to apn type stting.
        // the format of apn type string is "type, type, type, ..."
        memset(ApnTypeStr, 0, MAX_AT_CMD_LENGTH);
        countOfApnType = 0;
        if (supportedTypesBitmask == RIL_APN_TYPE_ALL ||
                supportedTypesBitmask == RIL_APN_TYPE_MTKALL) {
            RFX_LOG_W(RFX_LOG_TAG, "[%d][%s] Unexpected to configure apn to support all types",
                    rid, __FUNCTION__);
            strncpy(ApnTypeStr, RmcDcUtility::getApnType(supportedTypesBitmask),
                    MAX_AT_CMD_LENGTH-1);
        } else {
            for (int j = 0; j < RIL_APN_TYPE_COUNT; j++) {
                int typeBitmask = 1 << j;
                if (supportedTypesBitmask & typeBitmask) {
                    memset(tempApnTypeStr, 0, MAX_AT_CMD_LENGTH);
                    strncpy(tempApnTypeStr, RmcDcUtility::getApnType(typeBitmask),
                            MAX_AT_CMD_LENGTH - 1);
                    if (countOfApnType > 0) {
                        strncat(ApnTypeStr + strlen(ApnTypeStr), ",",
                                MAX_AT_CMD_LENGTH - strlen(ApnTypeStr) - 1);
                    }
                    strncat(ApnTypeStr + strlen(ApnTypeStr), tempApnTypeStr,
                            MAX_AT_CMD_LENGTH - strlen(ApnTypeStr) - 1);
                    countOfApnType++;

                    if (typeBitmask == RIL_APN_TYPE_IMS) {
                        hasImsApn = 1;
                    }
                }
            }
        }

        // update other parameters
        // AT+EAPNSET = "apn",cmdIndex+1,"param1=value1;param2=value2;param3=value3;..."
        memset(strParam, 0, MAX_AT_CMD_LENGTH);
        if (strlen(ApnTypeStr) > 0) {
            snprintf(strParam, MAX_AT_CMD_LENGTH, "type=%s;", ApnTypeStr);
        }
        memset(tmpStrParam, 0, MAX_AT_CMD_LENGTH);
        snprintf(tmpStrParam, MAX_AT_CMD_LENGTH, "protocol=%s;roaming_protocol=%s;"
                "authtype=%d;carrier_enabled=%d;max_conns=%d;max_conns_time=%d;wait_time=%d;"
                "bearer_bitmask=%lu;inactive_timer=%d",
                strlen(dataProfilePtrs[i]->protocol) == 0 ? "IP" : dataProfilePtrs[i]->protocol,
                strlen(dataProfilePtrs[i]->roamingProtocol) == 0 ? "IP" : dataProfilePtrs[i]->roamingProtocol,
                authType, enabled, maxConns, maxConnsTime, waitTime,
                0 == bearerBitmask ? ALL_BEARER_EXCLUDE_IWLAN_BITMASK : bearerBitmask,
                inactiveTimer);
        strncat(strParam + strlen(strParam), tmpStrParam, MAX_AT_CMD_LENGTH-strlen(strParam)-1);

        cmd.clear();
        cmd.append(String8::format("AT+EAPNSET=\"%s\",%d,\"%s\"", apn, cmdIndex+1, strParam));
        p_response = atSendCommandSingleline(cmd, "+EAPNSET:");

        if (p_response == NULL) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] 4. fail to get p_response!",
                    rid, __FUNCTION__);
            goto error;
        }

        if (p_response->getIntermediates() != NULL) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] AT+EAPNSET update other parameters ERROR",
                    rid, __FUNCTION__);
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] %s",
                    rid, __FUNCTION__, p_response->getIntermediates()->getLine());
            goto error;
        }
    }

    // Customization requirement, update protocol&roaming protocol of modem's default emergency apn.
    if (RmcDcUtility::isSupportOverrideModemDefaultApn()) {
        updateMdDefaultEmergencyApn(dataProfilePtrs, nReqNum);
    }

    //Default IMS apn setting
    //if IMS apn settings does not exists, add one and sync to MD
    if(RmcDcUtility::isSupportDefaultImsApnSettings()) {
        RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] hasImsApn = %d, no IMS apn, add default IMS apn settings ",
                rid, __FUNCTION__, hasImsApn);
        if(hasImsApn == -1) {
        // update the user name and password
        // AT+EAPNSET = "apn",cmdIndex,"user","pwd"
            cmd.clear();
            cmd.append(String8::format("AT+EAPNSET=\"ims\",1,\"\",\"\""));
            p_response = atSendCommand(cmd);

            if (p_response == NULL) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] 1. Set default IMS apn fail to get p_response!",
                        rid, __FUNCTION__);
                goto error;
            }

            if (p_response->isAtResponseFail()) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] AT+EAPNSET update user name and password ERROR",
                        rid, __FUNCTION__);
                goto error;
            }

            // update other parameters
            // AT+EAPNSET = "apn",cmdIndex+1,"param1=value1;param2=value2;param3=value3;..."
            cmd.clear();
            cmd.append(String8::format("AT+EAPNSET=\"ims\",2,\"type=ims;protocol=IPV4V6;" \
                    "roaming_protocol=IPV4V6;authtype=0;carrier_enabled=1;max_conns=0;" \
                    "max_conns_time=0;wait_time=0;bearer_bitmask=%u;inactive_timer=0\"",
                    ALL_BEARER_BITMASK));
            p_response = atSendCommandSingleline(cmd, "+EAPNSET:");

            if (p_response == NULL) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] 2. Set default IMS apn fail to get p_response!",
                        rid, __FUNCTION__);
                goto error;
            }

            if (p_response->getIntermediates() != NULL) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] AT+EAPNSET update other parameters ERROR",
                        rid, __FUNCTION__);
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] %s",
                        rid, __FUNCTION__, p_response->getIntermediates()->getLine());
                goto error;
            }
        }
    }

    // unlock the apn table
    p_response = atSendCommand("AT+EAPNLOCK=0");

    if (p_response == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] 5. fail to get p_response!",
                rid, __FUNCTION__);
        goto error;
    }

    if (p_response->isAtResponseFail()) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] AT+EAPNLOCK=0 returns ERROR",
                rid, __FUNCTION__);
        response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
                RfxVoidData(), msg, false);
    } else {
        response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
                RfxVoidData(), msg, false);
    }

    RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] Sending event: RFX_MSG_EVENT_QUERY_SELF_IA",
            rid, __FUNCTION__);
    sendEvent(RFX_MSG_EVENT_QUERY_SELF_IA, RfxVoidData(),
            RIL_CMD_PROXY_5, m_slot_id);

    responseToTelCore(response);

    deleteDataProfiles(dataProfilePtrs, nOldNum + MAX_COUNT_EXTRA_APN);
    return;

error:
    // reset Last APN table
    if (s_LastApnTable[m_slot_id] != NULL) {
        for (int i = 0; i < s_nLastReqNum[m_slot_id]; i++) {
            FREEIF(s_LastApnTable[m_slot_id][i].apn);
            FREEIF(s_LastApnTable[m_slot_id][i].protocol);
            FREEIF(s_LastApnTable[m_slot_id][i].roamingProtocol);
            FREEIF(s_LastApnTable[m_slot_id][i].user);
            FREEIF(s_LastApnTable[m_slot_id][i].password);
            FREEIF(s_LastApnTable[m_slot_id][i].mvnoType);
            FREEIF(s_LastApnTable[m_slot_id][i].mvnoMatchData);
        }
        FREEIF(s_ApnCmdIndex[m_slot_id]);
        FREEIF(s_LastApnTable[m_slot_id]);
    }
    s_nLastReqNum[m_slot_id] = 0;

    // unlock the apn table
    p_response = atSendCommand("AT+EAPNLOCK=0");

    if (p_response == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] 6. fail to get p_response!",
                rid, __FUNCTION__);
    } else if (p_response->isAtResponseFail()) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] AT+EAPNLOCK=0 returns ERROR",
                rid, __FUNCTION__);
    }
    response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
            RfxVoidData(), msg, false);
    responseToTelCore(response);

    deleteDataProfiles(dataProfilePtrs, nOldNum + MAX_COUNT_EXTRA_APN);
}

void RmcDcCommonReqHandler::requestSyncDataSettingsToMd(const sp<RfxMclMessage>& msg) {
    int rid = m_slot_id;
    int *pReqData = (int *)msg->getData()->getData();
    int reqDataNum = msg->getData()->getDataLength() / sizeof(int);
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> response;
    RIL_Errno ret = RIL_E_SUCCESS;

    // s_dataSetting[m_slot_id][0 (MOBILE_DATA)]:      data setting on/off.
    // s_dataSetting[m_slot_id][1 (ROAMING_DATA)]:     data roaming setting on/off.
    // s_dataSetting[m_slot_id][2 (DEFAULT_DATA_SIM)]: default data SIM setting.
    // s_dataSetting[m_slot_id][3 (DOMESTIC_ROAMING_DATA)]: domestic roaming setting.
    // s_dataSetting[m_slot_id][4 (INTERNATIONAL_ROAMING_DATA)]: international roaming setting.
    for (int i = 0; i < reqDataNum; i++) {
        s_dataSetting[m_slot_id][i] = pReqData[i];
        if (pReqData[i] != SKIP_DATA_SETTINGS) {
            s_dataSetting_resend[m_slot_id][i] = pReqData[i];
        }
    }

    // 0: clear
    // 1: slot 0
    // 2: slot 1
    // Modem only receive the slot ID which start from 1.
    // And 0 is treated as "UNSET" to reset.
    int setDefaultSim = s_dataSetting[m_slot_id][DEFAULT_DATA_SIM] + 1;

    RFX_LOG_D(RFX_LOG_TAG,
            "[%d][%s] Start, dataOn:%d, dataRoaming:%d, defaultDataSim: %d (original: %d),"
            "domestic roaming: %d, international raoming: %d",
            rid,
            __FUNCTION__,
            s_dataSetting[m_slot_id][MOBILE_DATA],
            s_dataSetting[m_slot_id][ROAMING_DATA],
            setDefaultSim,
            s_dataSetting[m_slot_id][DEFAULT_DATA_SIM],
            s_dataSetting[m_slot_id][DOMESTIC_ROAMING_DATA],
            s_dataSetting[m_slot_id][INTERNATIONAL_ROAMING_DATA]);

    if (s_dataSetting[m_slot_id][DEFAULT_DATA_SIM] != SKIP_DATA_SETTINGS
        && (s_dataSetting[m_slot_id][DEFAULT_DATA_SIM] <= MAX_RFX_SLOT_ID
            || s_dataSetting[m_slot_id][DEFAULT_DATA_SIM] >= RFX_SLOT_ID_0)) {

        p_response = atSendCommand(String8::format("AT+EDALLOW=2,%d",
            setDefaultSim));

        if (p_response == NULL) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] 1. fail to get p_response!",
                    rid, __FUNCTION__);
            ret = RIL_E_GENERIC_FAILURE;
        } else if (p_response->isATCmdRspErr()) {
            // If sync default data SIM to MD meet error, then keep sync data settings to MD.
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] AT+EDALLOW command returns ERROR",
                    rid, __FUNCTION__);
            ret = RIL_E_GENERIC_FAILURE;
        }
    }

    if (s_dataSetting[m_slot_id][MOBILE_DATA] == SKIP_DATA_SETTINGS) {
        response = RfxMclMessage::obtainResponse(msg->getId(), ret, RfxVoidData(), msg, false);
        responseToTelCore(response);
        return;
    }

    /**
     * To get IMS test mode setting and sync to modem for OP12
     *    Enable : Set Attach PDN to VZWINTERNET
     *    Disable: Set Attach PDN to VZWIMS (Default)
     */
    char value[MTK_PROPERTY_VALUE_MAX] = {0};
    rfx_property_get("persist.vendor.radio.imstestmode", value, "0");
    int imsTestMode = atoi(value);

    // AT+ECNCFG=<mobile_data>[,[<data_roaming>],[<volte>],<ims_test_mode>
    // [,<domestic_data_roaming>,<international_data_roaming>]]
    if (RmcDcUtility::isOp20Support() == 1) {
        if (s_dataSetting[m_slot_id][DOMESTIC_ROAMING_DATA] == SKIP_DATA_SETTINGS &&
            s_dataSetting[m_slot_id][INTERNATIONAL_ROAMING_DATA] == SKIP_DATA_SETTINGS) {
            p_response = atSendCommand(String8::format("AT+ECNCFG=%d,,,%d",
                    s_dataSetting[m_slot_id][MOBILE_DATA],
                    imsTestMode));
        } else if (s_dataSetting[m_slot_id][DOMESTIC_ROAMING_DATA] != SKIP_DATA_SETTINGS &&
                s_dataSetting[m_slot_id][INTERNATIONAL_ROAMING_DATA] != SKIP_DATA_SETTINGS) {
            p_response = atSendCommand(String8::format("AT+ECNCFG=%d,,,%d,%d,%d",
                    s_dataSetting[m_slot_id][MOBILE_DATA],
                    imsTestMode,
                    s_dataSetting[m_slot_id][DOMESTIC_ROAMING_DATA],
                    s_dataSetting[m_slot_id][INTERNATIONAL_ROAMING_DATA]));
        } else {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] invalid domestic/international roaming value",
                    rid, __FUNCTION__);
            response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
                    RfxVoidData(), msg, false);
            responseToTelCore(response);
            return;
        }
    } else {
        if (s_dataSetting[m_slot_id][ROAMING_DATA] == SKIP_DATA_SETTINGS) {
            p_response = atSendCommand(String8::format("AT+ECNCFG=%d,,,%d",
                    s_dataSetting[m_slot_id][MOBILE_DATA],
                    imsTestMode));
        } else {
            p_response = atSendCommand(String8::format("AT+ECNCFG=%d,%d,,%d",
                    s_dataSetting[m_slot_id][MOBILE_DATA],
                    s_dataSetting[m_slot_id][ROAMING_DATA],
                    imsTestMode));
        }
    }

    if ((p_response == NULL) || (p_response != NULL && p_response->isATCmdRspErr())) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] AT+ECNCFG command returns ERROR or p_response is null",
                rid, __FUNCTION__);
        response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
                RfxVoidData(), msg, false);
    } else {
        response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
                RfxVoidData(), msg, false);
    }

    responseToTelCore(response);
}

void RmcDcCommonReqHandler::requestResendSyncDataSettingsToMd(const sp<RfxMclMessage>& msg) {
    int rid = m_slot_id;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> response;

    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] Start, dataOn:%d",
            rid,
            __FUNCTION__,
            s_dataSetting_resend[m_slot_id][MOBILE_DATA]);

    if (s_dataSetting_resend[m_slot_id][MOBILE_DATA] == SKIP_DATA_SETTINGS) {
        response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
                RfxVoidData(), msg, false);
        responseToTelCore(response);
        return;
    }

    char value[MTK_PROPERTY_VALUE_MAX] = {0};
    rfx_property_get("persist.vendor.radio.imstestmode", value, "0");
    int imsTestMode = atoi(value);

    if (RmcDcUtility::isOp20Support() == 1) {
        if (s_dataSetting_resend[m_slot_id][DOMESTIC_ROAMING_DATA] == SKIP_DATA_SETTINGS &&
                s_dataSetting_resend[m_slot_id][INTERNATIONAL_ROAMING_DATA] == SKIP_DATA_SETTINGS) {
            p_response = atSendCommand(String8::format("AT+ECNCFG=%d,,,%d",
                    s_dataSetting_resend[m_slot_id][MOBILE_DATA],
                    imsTestMode));
        } else if (s_dataSetting_resend[m_slot_id][DOMESTIC_ROAMING_DATA] != SKIP_DATA_SETTINGS &&
                s_dataSetting_resend[m_slot_id][INTERNATIONAL_ROAMING_DATA] != SKIP_DATA_SETTINGS) {
            p_response = atSendCommand(String8::format("AT+ECNCFG=%d,,,%d,%d,%d",
                    s_dataSetting_resend[m_slot_id][MOBILE_DATA],
                    imsTestMode,
                    s_dataSetting_resend[m_slot_id][DOMESTIC_ROAMING_DATA],
                    s_dataSetting_resend[m_slot_id][INTERNATIONAL_ROAMING_DATA]));
        } else {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] invalid domestic/international roaming value",
                    rid, __FUNCTION__);
            response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
                    RfxVoidData(), msg, false);
            responseToTelCore(response);
            return;
        }
    } else {
        if (s_dataSetting_resend[m_slot_id][ROAMING_DATA] == SKIP_DATA_SETTINGS) {
            p_response = atSendCommand(String8::format("AT+ECNCFG=%d,,,%d",
                    s_dataSetting[m_slot_id][MOBILE_DATA],
                    imsTestMode));
        } else {
            p_response = atSendCommand(String8::format("AT+ECNCFG=%d,%d,,%d",
                    s_dataSetting_resend[m_slot_id][MOBILE_DATA],
                    s_dataSetting_resend[m_slot_id][ROAMING_DATA],
                    imsTestMode));
        }
    }

    if ((p_response == NULL) || (p_response != NULL && p_response->isATCmdRspErr())) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] AT+ECNCFG command returns ERROR or p_response is null",
                rid, __FUNCTION__);
        response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
                RfxVoidData(), msg, false);
    } else {
        response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
                RfxVoidData(), msg, false);
    }

    responseToTelCore(response);
}

void RmcDcCommonReqHandler::requestResetMdDataRetryCount(const sp<RfxMclMessage>& msg) {
    int rid = m_slot_id;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> response;
    const char *apnName = (const char *)msg->getData()->getData();
    char apn[MAX_APN_NAME_LENGTH] = {0};

    // Debug only
    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] Reset MD data count for APN: %s", rid, __FUNCTION__, apnName);

    // AT+EDRETRY=<mode><APN name>
    // <mode>:indicate it's query mode or reset mode. 0:query mode, 1:reset mode
    // <APN name>: APN (string type) which is use to query or reset bar timer and retry count.
    if (apnName != NULL && strlen(apnName) > 0) {
        strncpy(apn, apnName, MAX_APN_NAME_LENGTH-1);
        addEscapeSequence(apn);
    }
    p_response = atSendCommand(String8::format("AT+EDRETRY=1,\"%s\"", apn));

    if ((p_response == NULL) || (p_response != NULL && p_response->isATCmdRspErr())) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] AT+EDRETRY command returns ERROR or p_response is null",
                rid, __FUNCTION__);
        response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
                RfxVoidData(), msg, false);
    } else {
        response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
                RfxVoidData(), msg, false);
    }

    responseToTelCore(response);
}

RIL_DataCallFailCause RmcDcCommonReqHandler::convertFailCauseToRilStandard(int cause) {
    int rid = m_slot_id;

    if (cause > SM_CAUSE_BASE && cause < SM_CAUSE_END) {
        cause -= SM_CAUSE_BASE;
    } else if (cause > ESM_CAUSE_BASE && cause < ESM_CAUSE_END) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ESM CAUSE: %X",
                rid, __FUNCTION__, cause);
        cause -= ESM_CAUSE_BASE;
    } else if (cause > PAM_CAUSE_BASE && cause < PAM_CAUSE_END) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] PAM CAUSE: %X",
                rid, __FUNCTION__, cause);
        cause -= PAM_CAUSE_BASE;
    } else if (cause > CME_CAUSE_BASE_V14 && cause < CME_CAUSE_END_V14) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] CME CAUSE: %X",
                rid, __FUNCTION__, cause);
        if (cause == CME_PDP_AUTHENTICATION_FAILED) {
            return PDP_FAIL_USER_AUTHENTICATION;
        } else if (cause == CME_OPERATOR_DETERMINED_BARRING) {
            return PDP_FAIL_OPERATOR_BARRED;
        } else if (cause == CME_MAXIMUM_NMUBER_PDP_CONTEXTS_REACHED) {
            return PDP_FAIL_MAX_ACTIVE_PDP_CONTEXT_REACHED;
        } else if (cause == CME_REQUESTED_APN_NOT_SUPPORTED_IN_RAT_AND_PLMN) {
            return PDP_FAIL_UNSUPPORTED_APN_IN_CURRENT_PLMN;
        } else {
            cause -= CME_CAUSE_BASE_V14;
        }
    }

    switch (cause) {
        case 0: return PDP_FAIL_NONE;
        case 0x08: return PDP_FAIL_OPERATOR_BARRED;
        case 0x0E: return PDP_FAIL_NAS_SIGNALLING;
        case 0x18: return PDP_FAIL_MBMS_CAPABILITIES_INSUFFICIENT;
        case 0x19: return PDP_FAIL_LLC_SNDCP;
        case 0x1A: return PDP_FAIL_INSUFFICIENT_RESOURCES;
        case 0x1B: return PDP_FAIL_MISSING_UKNOWN_APN;
        case 0x1C: return PDP_FAIL_UNKNOWN_PDP_ADDRESS_TYPE;
        case 0x1D: return PDP_FAIL_USER_AUTHENTICATION;
        case 0x1E: return PDP_FAIL_ACTIVATION_REJECT_GGSN;
        case 0x1F: return PDP_FAIL_ACTIVATION_REJECT_UNSPECIFIED;
        case 0x20: return PDP_FAIL_SERVICE_OPTION_NOT_SUPPORTED;
        case 0x21: return PDP_FAIL_SERVICE_OPTION_NOT_SUBSCRIBED;
        case 0x22: return PDP_FAIL_SERVICE_OPTION_OUT_OF_ORDER;
        case 0x23: return PDP_FAIL_NSAPI_IN_USE;
        case 0x24: return PDP_FAIL_REGULAR_DEACTIVATION;
        case 0x25: return PDP_FAIL_QOS_NOT_ACCEPTED;
        case 0x26: return PDP_FAIL_NETWORK_FAILURE;
        case 0x27: return PDP_FAIL_UMTS_REACTIVATION_REQ;
        case 0x28: return PDP_FAIL_FEATURE_NOT_SUPP;
        case 0x29: return PDP_FAIL_TFT_SEMANTIC_ERROR;
        case 0x2A: return PDP_FAIL_TFT_SYTAX_ERROR;
        case 0x2B: return PDP_FAIL_UNKNOWN_PDP_CONTEXT;
        case 0x2C: return PDP_FAIL_FILTER_SEMANTIC_ERROR;
        case 0x2D: return PDP_FAIL_FILTER_SYTAX_ERROR;
        case 0x2E: return PDP_FAIL_PDP_WITHOUT_ACTIVE_TFT;
        case 0x2F: return PDP_FAIL_MULTICAST_GROUP_MEMBERSHIP_TIMEOUT;
        case 0x30: return PDP_FAIL_BCM_VIOLATION;
        case 0x31: return PDP_FAIL_LAST_PDN_DISC_NOT_ALLOWED;
        case 0x32: return PDP_FAIL_ONLY_IPV4_ALLOWED;
        case 0x33: return PDP_FAIL_ONLY_IPV6_ALLOWED;
        case 0x34: return PDP_FAIL_ONLY_SINGLE_BEARER_ALLOWED;
        case 0x35: return PDP_FAIL_ESM_INFO_NOT_RECEIVED;
        case 0x36: return PDP_FAIL_PDN_CONN_DOES_NOT_EXIST;
        case 0x37: return PDP_FAIL_MULTI_CONN_TO_SAME_PDN_NOT_ALLOWED;
        case 0x38: return PDP_FAIL_COLLISION_WITH_NW_INITIATED_REQUEST;
        case 0x3B: return PDP_FAIL_ESM_UNSUPPORTED_QCI_VALUE;
        case 0x3C: return PDP_FAIL_BEARER_HANDLING_NOT_SUPPORT;
        case 0x41: return PDP_FAIL_MAX_ACTIVE_PDP_CONTEXT_REACHED;
        case 0x42: return PDP_FAIL_UNSUPPORTED_APN_IN_CURRENT_PLMN;
        case 0x51: return PDP_FAIL_INVALID_TRANSACTION_ID;
        case 0x5F: return PDP_FAIL_MESSAGE_INCORRECT_SEMANTIC;
        case 0x60: return PDP_FAIL_INVALID_MANDATORY_INFO;
        case 0x61: return PDP_FAIL_MESSAGE_TYPE_UNSUPPORTED;
        case 0x62: return PDP_FAIL_MSG_TYPE_NONCOMPATIBLE_STATE;
        case 0x63: return PDP_FAIL_UNKNOWN_INFO_ELEMENT;
        case 0x64: return PDP_FAIL_CONDITIONAL_IE_ERROR;
        case 0x65: return PDP_FAIL_MSG_AND_PROTOCOL_STATE_UNCOMPATIBLE;
        case 0x6F: return PDP_FAIL_PROTOCOL_ERRORS;
        case 0x70: return PDP_FAIL_APN_TYPE_CONFLICT;
        case 0x71: return PDP_FAIL_INVALID_PCSCF_ADDR;
        case 0x72: return PDP_FAIL_INTERNAL_CALL_PREEMPT_BY_HIGH_PRIO_APN;
        case 0x73: return PDP_FAIL_EMM_ACCESS_BARRED;
        case 0x74: return PDP_FAIL_EMERGENCY_IFACE_ONLY;
        case 0x75: return PDP_FAIL_IFACE_MISMATCH;
        case 0x76: return PDP_FAIL_COMPANION_IFACE_IN_USE;
        case 0x77: return PDP_FAIL_IP_ADDRESS_MISMATCH;
        case 0x78: return PDP_FAIL_IFACE_AND_POL_FAMILY_MISMATCH;
        case 0x79: return PDP_FAIL_EMM_ACCESS_BARRED_INFINITE_RETRY;
        case 0x7A: return PDP_FAIL_AUTH_FAILURE_ON_EMERGENCY_CALL;
        case 0x0E0F: return PDP_FAIL_LOCAL_REJECT_ACT_REQ_DUE_TO_REACH_RETRY_COUNTER;
        case 0x0F47: return PDP_FAIL_TCM_ESM_TIMER_TIMEOUT;
        case 0x1402: return PDP_FAIL_PAM_ATT_PDN_ACCESS_REJECT_IMS_PDN_BLOCK_TEMP;
        case 0x1671: return PDP_FAIL_DATA_NOT_ALLOW;
        case -1: return PDP_FAIL_VOICE_REGISTRATION_FAIL;
        case -2: return PDP_FAIL_DATA_REGISTRATION_FAIL;
        case -3: return PDP_FAIL_SIGNAL_LOST;
        case -4: return PDP_FAIL_PREF_RADIO_TECH_CHANGED;
        case -5: return PDP_FAIL_RADIO_POWER_OFF;
        case -6: return PDP_FAIL_TETHERED_CALL_ACTIVE;
        default: return PDP_FAIL_ERROR_UNSPECIFIED;
    }
}

int RmcDcCommonReqHandler::convertDeactCauseToMdStandard(int reason) {
    switch (reason) {
        case DEACT_REASON_AOSP_SHUTDOWN: return ATCMD_DEACT_CAUSE_EGACT_CAUSE_AOSP_SHUTDOWN;
        case DEACT_REASON_AOSP_HANDOVER: return ATCMD_DEACT_CAUSE_EGACT_CAUSE_AOSP_HANDOVER;
        case DEACT_REASON_APN_CHANGED: return ATCMD_DEACT_CAUSE_EGACT_CAUSE_APN_CHANGED;
        default: return ATCMD_DEACT_CAUSE_EGACT_CAUSE_DEACT_NORMAL;
    }
}

void RmcDcCommonReqHandler::updateLastFailCause(int cause) {
    m_nGprsFailureCause = convertFailCauseToRilStandard(cause);
}

int RmcDcCommonReqHandler::getLastFailCause() {
    RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] CAUSE: %X", m_slot_id, __FUNCTION__, m_nGprsFailureCause);
    return m_nGprsFailureCause;
}

int RmcDcCommonReqHandler::isAllAidActive(Vector<int> *vAidList) {
    int rid = m_slot_id;

    updateActiveStatus();

    for (Vector<int>::iterator it = vAidList->begin(); it != vAidList->end(); it++) {
        int aid = *it;
        RFX_LOG_V(RFX_LOG_TAG, "[%d][%s] pdn_info[%d], state: %d",
                rid, __FUNCTION__, aid, getPdnActiveStatus(aid));
        if (DATA_STATE_INACTIVE == getPdnActiveStatus(aid)) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] aid%d is inactive", rid, __FUNCTION__, aid);
            return 0;
        }
    }

    return 1;
}

void RmcDcCommonReqHandler::requestOrSendDataCallList(const sp<RfxMclMessage>& msg) {
    int rid = m_slot_id;
    MTK_RIL_Data_Call_Response_v11* responsesOfActive = NULL;
    Vector<int> vTransIntfIdList;
    Vector<int>::iterator it;
    sp<RfxMclMessage> responseMsg;
    const int nPdnInfoSize = getPdnTableSize();

    RFX_LOG_V(RFX_LOG_TAG, "[%d][%s] PdnTableSize=%d", rid, __FUNCTION__, nPdnInfoSize);

    // Reserve size of TransIntfId list.
    vTransIntfIdList.reserve(nPdnInfoSize);

    // Update all PDN active status.
    updateActiveStatus();

    // Update all PDN ip address.
    updatePdnAddress();

    // Update all PDN default bearer information.
    if (PDN_SUCCESS != updateDefaultBearerInfo()) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] updateDefaultBearerInfo failed", rid, __FUNCTION__);
        goto error;
    }

    for (int i = 0; i < nPdnInfoSize; i++) {
        if ((getPdnActiveStatus(i) == DATA_STATE_ACTIVE) &&
                (getTransIntfId(i) != INVALID_TRANS_INTF_ID) &&
                (std::find(vTransIntfIdList.begin(), vTransIntfIdList.end(),
                getTransIntfId(i)) == vTransIntfIdList.end())) {
                vTransIntfIdList.push_back(getTransIntfId(i));
        }
    }

    if (vTransIntfIdList.size() > 0) {
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] active trans-interface number is %zu",
                rid, __FUNCTION__, vTransIntfIdList.size());

        responsesOfActive = (MTK_RIL_Data_Call_Response_v11*)
                calloc(1, vTransIntfIdList.size() * sizeof(MTK_RIL_Data_Call_Response_v11));

        if (responsesOfActive == NULL) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to allocate responsesOfActive!",
                    rid, __FUNCTION__);
            RFX_ASSERT(false);
        }

        initDataCallResponse(responsesOfActive, vTransIntfIdList.size());
        int index = 0;
        for (it = vTransIntfIdList.begin(); it != vTransIntfIdList.end(); it++) {
            int transIntfId = *it;
            createDataResponse(transIntfId, IPV4V6, &responsesOfActive[index]);
            ++index;
        }
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] %s", rid, __FUNCTION__,
                responsesToString(responsesOfActive, vTransIntfIdList.size()).string());

        if (msg->getType() == RFX_MESSAGE_TYPE::REQUEST) {
            responseMsg = RfxMclMessage::obtainResponse(RIL_E_SUCCESS,
                    RfxDataCallResponseData(responsesOfActive, vTransIntfIdList.size()), msg);
        } else {
            responseMsg = RfxMclMessage::obtainUrc(RFX_MSG_URC_DATA_CALL_LIST_CHANGED, rid,
                    RfxDataCallResponseData(responsesOfActive, vTransIntfIdList.size()));
        }
    } else {
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] no active response", rid, __FUNCTION__);

        if (msg->getType() == RFX_MESSAGE_TYPE::REQUEST) {
            responseMsg = RfxMclMessage::obtainResponse(RIL_E_SUCCESS, RfxVoidData(), msg);
        } else {
            responseMsg = RfxMclMessage::obtainUrc(RFX_MSG_URC_DATA_CALL_LIST_CHANGED, rid,
                    RfxVoidData());
        }
    }

    responseToTelCore(responseMsg);

    if (responsesOfActive != NULL) {
        int size = vTransIntfIdList.size();
        for (int i = 0; i < size; i++) {
            freeDataResponse(&responsesOfActive[i]);
        }
        FREEIF(responsesOfActive);
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] free responsesOfActive", rid, __FUNCTION__);
    }

    return;

error:
    if (msg->getType() == RFX_MESSAGE_TYPE::REQUEST) {
        responseMsg = RfxMclMessage::obtainResponse(RIL_E_GENERIC_FAILURE, RfxVoidData(), msg);
    } else {
        responseMsg = RfxMclMessage::obtainUrc(RFX_MSG_URC_DATA_CALL_LIST_CHANGED, rid, RfxVoidData());
    }
    responseToTelCore(responseMsg);
}

void RmcDcCommonReqHandler::requestOrSendDataCallList(const sp<RfxMclMessage>& msg, Vector<int> *vAidList) {
    int rid = m_slot_id;
    MTK_RIL_Data_Call_Response_v11* responsesOfActive = NULL;
    Vector<int> vTransIntfIdList;
    Vector<int>::iterator it;
    sp<RfxMclMessage> responseMsg;
    const int nPdnInfoSize = getPdnTableSize();

    RFX_LOG_V(RFX_LOG_TAG, "[%d][%s] PdnTableSize=%d", rid, __FUNCTION__, nPdnInfoSize);

    // Reserve size of TransIntfId list.
    vTransIntfIdList.reserve(nPdnInfoSize);

    updateActiveStatus();

    for (it = vAidList->begin(); it != vAidList->end(); it++) {
        int aid = *it;
        if (PDN_SUCCESS != updatePdnAddress(aid)) {
            // Not to check error since we expect to query all PDN connections even if error occurred
            RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] updatePdnAddress failed for aid%d",
                    rid, __FUNCTION__, aid);
        }

        if (PDN_SUCCESS != updateDefaultBearerInfo(aid)) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] updateDefaultBearerInfo failed for aid%d",
                    rid, __FUNCTION__, aid);
            goto error;
        }
    }

    for (int i = 0; i < nPdnInfoSize; i++) {
        if ((getPdnActiveStatus(i) == DATA_STATE_ACTIVE) &&
                (getTransIntfId(i) != INVALID_TRANS_INTF_ID) &&
                (std::find(vTransIntfIdList.begin(), vTransIntfIdList.end(),
                getTransIntfId(i)) == vTransIntfIdList.end())) {
                vTransIntfIdList.push_back(getTransIntfId(i));
        }
    }

    if (vTransIntfIdList.size() > 0) {
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] active trans-interface number is %zu",
                rid, __FUNCTION__, vTransIntfIdList.size());

        responsesOfActive = (MTK_RIL_Data_Call_Response_v11*)
                calloc(1, vTransIntfIdList.size() * sizeof(MTK_RIL_Data_Call_Response_v11));

        if (responsesOfActive == NULL) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to allocate responsesOfActive!",
                    rid, __FUNCTION__);
            RFX_ASSERT(false);
        }

        initDataCallResponse(responsesOfActive, vTransIntfIdList.size());
        int index = 0;
        for (it = vTransIntfIdList.begin(); it != vTransIntfIdList.end(); it++) {
            int transIntfId = *it;
            createDataResponse(transIntfId, IPV4V6, &responsesOfActive[index]);
            ++index;
        }
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] %s", rid, __FUNCTION__,
                responsesToString(responsesOfActive, vTransIntfIdList.size()).string());

        if (msg->getType() == RFX_MESSAGE_TYPE::REQUEST) {
            responseMsg = RfxMclMessage::obtainResponse(RIL_E_SUCCESS,
                    RfxDataCallResponseData(responsesOfActive, vTransIntfIdList.size()), msg);
        } else {
            responseMsg = RfxMclMessage::obtainUrc(RFX_MSG_URC_DATA_CALL_LIST_CHANGED, rid,
                    RfxDataCallResponseData(responsesOfActive, vTransIntfIdList.size()));
        }
    } else {
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] no active response", rid, __FUNCTION__);

        if (msg->getType() == RFX_MESSAGE_TYPE::REQUEST) {
            responseMsg = RfxMclMessage::obtainResponse(RIL_E_SUCCESS, RfxVoidData(), msg);
        } else {
            responseMsg = RfxMclMessage::obtainUrc(RFX_MSG_URC_DATA_CALL_LIST_CHANGED, rid,
                    RfxVoidData());
        }
    }

    responseToTelCore(responseMsg);

    if (responsesOfActive != NULL) {
        int size = vTransIntfIdList.size();
        for (int i = 0; i < size; i++) {
            freeDataResponse(&responsesOfActive[i]);
        }
        FREEIF(responsesOfActive);
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] free responsesOfActive", rid, __FUNCTION__);
    }

    return;

error:
    if (msg->getType() == RFX_MESSAGE_TYPE::REQUEST) {
        responseMsg = RfxMclMessage::obtainResponse(RIL_E_GENERIC_FAILURE, RfxVoidData(), msg);
    } else {
        responseMsg = RfxMclMessage::obtainUrc(RFX_MSG_URC_DATA_CALL_LIST_CHANGED, rid, RfxVoidData());
    }
    responseToTelCore(responseMsg);
}

void RmcDcCommonReqHandler::requestOrSendDataCallList(const sp<RfxMclMessage>& msg, int aid) {
    int rid = m_slot_id;
    MTK_RIL_Data_Call_Response_v11* responsesOfActive = NULL;
    Vector<int> vTransIntfIdList;
    Vector<int>::iterator it;
    sp<RfxMclMessage> responseMsg;
    const int nPdnInfoSize = getPdnTableSize();

    RFX_LOG_V(RFX_LOG_TAG, "[%d][%s] PdnTableSize=%d", rid, __FUNCTION__, nPdnInfoSize);

    if (PDN_SUCCESS != updatePdnAddress(aid)) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] updatePdnAddress failed for aid%d",
                rid, __FUNCTION__, aid);
        return;
    }

    if (PDN_SUCCESS != updateDefaultBearerInfo(aid)) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] updateDefaultBearerInfo failed for aid%d",
                rid, __FUNCTION__, aid);
        return;
    }

    updateActiveStatus();

    // Reserve size of TransIntfId list.
    vTransIntfIdList.reserve(nPdnInfoSize);

    for (int i = 0; i < nPdnInfoSize; i++) {
        if ((getPdnActiveStatus(i) == DATA_STATE_ACTIVE) &&
                (getTransIntfId(i) != INVALID_TRANS_INTF_ID) &&
                (std::find(vTransIntfIdList.begin(), vTransIntfIdList.end(),
                getTransIntfId(i)) == vTransIntfIdList.end())) {
                vTransIntfIdList.push_back(getTransIntfId(i));
        }
    }

    if (vTransIntfIdList.size() > 0) {
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] active trans-interface number is %zu",
                rid, __FUNCTION__, vTransIntfIdList.size());

        responsesOfActive = (MTK_RIL_Data_Call_Response_v11*)
                calloc(1, vTransIntfIdList.size() * sizeof(MTK_RIL_Data_Call_Response_v11));

        if (responsesOfActive == NULL) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to allocate responsesOfActive!",
                    rid, __FUNCTION__);
            RFX_ASSERT(false);
        }

        initDataCallResponse(responsesOfActive, vTransIntfIdList.size());
        int index = 0;
        for (it = vTransIntfIdList.begin(); it != vTransIntfIdList.end(); it++) {
            int transIntfId = *it;
            createDataResponse(transIntfId, IPV4V6, &responsesOfActive[index]);
            ++index;
        }
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] %s", rid, __FUNCTION__,
                responsesToString(responsesOfActive, vTransIntfIdList.size()).string());

        if (msg->getType() == RFX_MESSAGE_TYPE::REQUEST) {
            responseMsg = RfxMclMessage::obtainResponse(RIL_E_SUCCESS,
                    RfxDataCallResponseData(responsesOfActive, vTransIntfIdList.size()), msg);
        } else {
            responseMsg = RfxMclMessage::obtainUrc(RFX_MSG_URC_DATA_CALL_LIST_CHANGED, rid,
                    RfxDataCallResponseData(responsesOfActive, vTransIntfIdList.size()));
        }
    } else {
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] no active response", rid, __FUNCTION__);

        if (msg->getType() == RFX_MESSAGE_TYPE::REQUEST) {
            responseMsg = RfxMclMessage::obtainResponse(RIL_E_SUCCESS, RfxVoidData(), msg);
        } else {
            responseMsg = RfxMclMessage::obtainUrc(RFX_MSG_URC_DATA_CALL_LIST_CHANGED, rid,
                    RfxVoidData());
        }
    }

    responseToTelCore(responseMsg);

    if (responsesOfActive != NULL) {
        int size = vTransIntfIdList.size();
        for (int i = 0; i < size; i++) {
            freeDataResponse(&responsesOfActive[i]);
        }
        FREEIF(responsesOfActive);
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] free responsesOfActive", rid, __FUNCTION__);
    }
}

void RmcDcCommonReqHandler::sendDataCallListResponse(const sp<RfxMclMessage>& msg, int deactivatedAid,
        int sscMode) {
    int rid = m_slot_id;
    MTK_RIL_Data_Call_Response_v11* responsesOfActive = NULL;
    Vector<int> vTransIntfIdList;
    Vector<int>::iterator it;
    sp<RfxMclMessage> responseMsg;
    const int nPdnInfoSize = getPdnTableSize();
    int firstAid = INVALID_AID;
    int secondAid = INVALID_AID;
    int lifetime = INVALID_LIFE_TIME;

    RFX_LOG_V(RFX_LOG_TAG, "[%d][%s] PdnTableSize=%d", rid, __FUNCTION__, nPdnInfoSize);

    // Reserve size of TransIntfId list.
    vTransIntfIdList.reserve(nPdnInfoSize);

    for (int i = 0; i < nPdnInfoSize; i++) {
        if ((getPdnActiveStatus(i) == DATA_STATE_ACTIVE) &&
                (getTransIntfId(i) != INVALID_TRANS_INTF_ID) &&
                (std::find(vTransIntfIdList.begin(), vTransIntfIdList.end(),
                getTransIntfId(i)) == vTransIntfIdList.end())) {
            vTransIntfIdList.push_back(getTransIntfId(i));
        }

        if ((sscMode == SSC_MODE3) && (getSscMode(i) == SSC_MODE3) &&
                (getPdnActiveStatus(i) == DATA_STATE_LINKDOWN) &&
                (std::find(vTransIntfIdList.begin(), vTransIntfIdList.end(),
                getTransIntfId(i)) == vTransIntfIdList.end())) {
            secondAid = i;
            vTransIntfIdList.push_back(getTransIntfId(i));
        }
    }

    if (secondAid != INVALID_AID) {
        for (int i = 0; i < nPdnInfoSize; i++) {
            if ((i != secondAid) && (getPdnActiveStatus(i) == DATA_STATE_ACTIVE) &&
                    (strncasecmp(getApnName(i), getApnName(secondAid), MAX_APN_NAME_LENGTH-1) == 0)) {
                firstAid = i;
                break;
            }
        }
        if (firstAid != INVALID_AID) {
            if (PDN_SUCCESS != getAddressLifeTime(firstAid, &lifetime)) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] getAddressLifeTime failed for aid%d, lifetime=%d",
                        rid, __FUNCTION__, firstAid, lifetime);
            }
        }
    }

    if (vTransIntfIdList.size() > 0) {
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] active trans-interface number is %zu",
                rid, __FUNCTION__, vTransIntfIdList.size());

        responsesOfActive = (MTK_RIL_Data_Call_Response_v11*)
                calloc(1, vTransIntfIdList.size() * sizeof(MTK_RIL_Data_Call_Response_v11));

        if (responsesOfActive == NULL) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to allocate responsesOfActive!",
                    rid, __FUNCTION__);
            RFX_ASSERT(false);
        }

        initDataCallResponse(responsesOfActive, vTransIntfIdList.size());
        int index = 0;
        for (it = vTransIntfIdList.begin(); it != vTransIntfIdList.end(); it++) {
            int transIntfId = *it;
            createDataResponse(transIntfId, IPV4V6, &responsesOfActive[index]);

            if (deactivatedAid != INVALID_AID &&
                    notifyDeactReasonIfNeeded(deactivatedAid) &&
                    transIntfId == getTransIntfId(deactivatedAid)) {
                responsesOfActive[index].active = DATA_STATE_INACTIVE;
                responsesOfActive[index].status = getDeactReason(deactivatedAid);
            }

            if (lifetime != INVALID_LIFE_TIME &&
                    secondAid != INVALID_AID &&
                    getTransIntfId(secondAid) == transIntfId) {
                RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] ssc3 pdu info [type/aid/transIntfId/lifetime(s)],"
                        " first pdu [%s/%d/%d/%d], second pdu [%s/%d/%d]", rid, __FUNCTION__,
                        RmcDcUtility::getProfileType(getProfileId(firstAid)), firstAid,
                        getTransIntfId(firstAid), lifetime,
                        RmcDcUtility::getProfileType(getProfileId(secondAid)), secondAid,
                        getTransIntfId(secondAid));
                responsesOfActive[index].cid = getTransIntfId(firstAid);
                responsesOfActive[index].suggestedRetryTime = lifetime;
                responsesOfActive[index].status = MTK_PDU_SSC_MODE_3;
            }

            ++index;
        }
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] %s", rid, __FUNCTION__,
                responsesToString(responsesOfActive, vTransIntfIdList.size()).string());

        if (msg->getType() == RFX_MESSAGE_TYPE::REQUEST) {
            responseMsg = RfxMclMessage::obtainResponse(RIL_E_SUCCESS,
                    RfxDataCallResponseData(responsesOfActive, vTransIntfIdList.size()), msg);
        } else {
            responseMsg = RfxMclMessage::obtainUrc(RFX_MSG_URC_DATA_CALL_LIST_CHANGED, rid,
                    RfxDataCallResponseData(responsesOfActive, vTransIntfIdList.size()));
        }
    } else {
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] no active response", rid, __FUNCTION__);

        if (msg->getType() == RFX_MESSAGE_TYPE::REQUEST) {
            responseMsg = RfxMclMessage::obtainResponse(RIL_E_SUCCESS, RfxVoidData(), msg);
        } else {
            responseMsg = RfxMclMessage::obtainUrc(RFX_MSG_URC_DATA_CALL_LIST_CHANGED, rid,
                    RfxVoidData());
        }
    }

    responseToTelCore(responseMsg);

    if (responsesOfActive != NULL) {
        int size = vTransIntfIdList.size();
        for (int i = 0; i < size; i++) {
            freeDataResponse(&responsesOfActive[i]);
        }
        FREEIF(responsesOfActive);
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] free responsesOfActive", rid, __FUNCTION__);
    }
}

void RmcDcCommonReqHandler::updateActiveStatus() {
    sp<RfxAtResponse> p_response;
    RfxAtLine *p_cur;
    int maxAidSupported = 0;
    int rid = m_slot_id;
    int err = 0;
    std::stringstream activeAidString;
    std::stringstream inactiveAidString;

    p_response = atSendCommandMultiline("AT+CGACT?", "+CGACT:");

    if (p_response == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get p_response!",
                rid, __FUNCTION__);
        return;
    }

    if (p_response->isAtResponseFail()) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] AT+CGACT? response ERROR", rid, __FUNCTION__);
        return;
    }

    for (p_cur = p_response->getIntermediates(); p_cur != NULL; p_cur = p_cur->getNext()) {
        maxAidSupported++;
    }

    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] PDP capability [%d], PdnTableSize [%d]",
            rid, __FUNCTION__, maxAidSupported, getPdnTableSize());

    for (p_cur = p_response->getIntermediates(); p_cur != NULL; p_cur = p_cur->getNext()) {
        int responseAid = 0;
        int active = 0;

        p_cur->atTokStart(&err);
        if (err < 0)
            break;

        responseAid = p_cur->atTokNextint(&err);
        if (err < 0)
            break;

        active = p_cur->atTokNextint(&err);
        if (err < 0) break;

        if (active > 0) {
            // 0=inactive, 1=active/physical link down, 2=active/physical link up
            if (getTransIntfId(responseAid) == INVALID_TRANS_INTF_ID) {
                updatePdnActiveStatus(responseAid, DATA_STATE_LINKDOWN);
                RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] aid%d is linkdown", rid, __FUNCTION__, responseAid);
            } else {
                updatePdnActiveStatus(responseAid, DATA_STATE_ACTIVE);
                RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] aid%d is active", rid, __FUNCTION__, responseAid);
            }

            if (getAid(responseAid) == INVALID_AID) {
                setAidAndPrimaryAid(responseAid, responseAid, responseAid);
            }
            activeAidString << responseAid << ":" << getPrimaryAid(responseAid) << ", ";
        } else {
            updatePdnActiveStatus(responseAid, DATA_STATE_INACTIVE);
            inactiveAidString << responseAid << ":" << getPrimaryAid(responseAid) << ", ";
        }
    }
    if (activeAidString.str().length() > 0) {
        RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] active list [aid:primaryAid] = [%s]",
                rid, __FUNCTION__, activeAidString.str().c_str());
    }
    if (inactiveAidString.str().length() > 0) {
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] inactive list [aid:primaryAid] = [%s]",
                rid, __FUNCTION__, inactiveAidString.str().c_str());
    }
}

int RmcDcCommonReqHandler::updateDefaultBearerInfo() {
    //Be aware that this method is used only for retrieving P-CSCF address and signaling flag
    int nRet = PDN_FAILED;
    sp<RfxAtResponse> p_response;
    RfxAtLine *p_cur;
    int count = 0;
    int *v4Count = NULL;
    int *v6Count = NULL;
    int *pcscfCount = NULL;
    int rid = m_slot_id;
    PdnInfo pdn_info;
    memset(&pdn_info, 0, sizeof(pdn_info));
    int err = 0;
    char* out = NULL;
    const int nPdnInfoSize = getPdnTableSize();
    String8 cmd("");

    for (int i = 0; i < nPdnInfoSize; i++) {
        //clear default bearer information in AID table since we would query it again
        pdn_info = getPdnInfo(i);
        memset(pdn_info.pcscf, 0, sizeof(pdn_info.pcscf));
        memset(pdn_info.dnsV4, 0, sizeof(pdn_info.dnsV4));
        memset(pdn_info.dnsV6, 0, sizeof(pdn_info.dnsV6));
        pdn_info.signalingFlag = 0;
        pdn_info.bearerId = INVALID_AID;
        pdn_info.mtu = 0;
        setPdnInfo(i, &pdn_info);
    }

    //+CGCONTRDP response a list of [+CGCONTRDP: <aid>,<bearer_id>,<apn>[,<local_addr and subnet_mask>
    //[,<gw_addr>[,<DNS_prim_addr>[,<DNS_sec_addr>[,<P-CSCF_prim_addr>[,<P-CSCF_sec_addr>
    //[,<IM_CN_Signalling_Flag>[,<LIPA_indication>[,<IPv4_MTU>[,<WLAN_Offload>[,<Local_Addr_Ind>
    //[,<Non-IP_MTU>]]]]]]]]]]]]]
    p_response = atSendCommandMultiline("AT+CGCONTRDP", "+CGCONTRDP:");

    if (p_response == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get p_response!",
                rid, __FUNCTION__);
        return PDN_FAILED;
    }

    if (p_response->isAtResponseFail()) {
        if (p_response->getSuccess() > 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] weird case, can not get fail cause due to final"
                    " response indicates success", rid, __FUNCTION__);
        } else {
            int cause = p_response->atGetCmeError();
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] AT+CGCONTRDP returns cme cause : %d",
                    rid, __FUNCTION__, cause);
        }
        return PDN_FAILED;
    }

    for (p_cur = p_response->getIntermediates(); p_cur != NULL; p_cur = p_cur->getNext()) {
        ++count;
    }

    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] response count [%d]", rid, __FUNCTION__, count);

    if (count == 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] response success but no intermediate", rid, __FUNCTION__);
        return PDN_SUCCESS;
    }

    v4Count = (int*)calloc(1, nPdnInfoSize * sizeof(int));
    RFX_ASSERT(v4Count != NULL);
    v6Count = (int*)calloc(1, nPdnInfoSize * sizeof(int));
    RFX_ASSERT(v6Count != NULL);
    pcscfCount = (int*)calloc(1, nPdnInfoSize * sizeof(int));
    RFX_ASSERT(pcscfCount != NULL);

    memset(v4Count, 0, nPdnInfoSize * sizeof(int));
    memset(v6Count, 0, nPdnInfoSize * sizeof(int));
    memset(pcscfCount, 0, nPdnInfoSize * sizeof(int));

    for (p_cur = p_response->getIntermediates(); p_cur != NULL; p_cur = p_cur->getNext()) {
        cmd.clear();
        p_cur->atTokStart(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when token start",
                    rid, __FUNCTION__);
            goto error;
        }

        int responseAid = 0;
        int bearerId = 0;
        char* apn = NULL;

        responseAid = p_cur->atTokNextint(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing aid",
                    rid, __FUNCTION__);
            goto error;
        }

        bearerId = p_cur->atTokNextint(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing bearer id",
                    rid, __FUNCTION__);
            goto error;
        }
        updateBearerId(responseAid, bearerId);

        apn = p_cur->atTokNextstr(&err); //apn
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing apn",
                    rid, __FUNCTION__);
            goto error;
        }

        if (p_cur->atTokHasmore()) {
            out = p_cur->atTokNextstr(&err); //local_addr and subnet_mask
            if (err < 0) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing local addr and subnet mask",
                        rid, __FUNCTION__);
                goto error;
            }
            out = NULL;
        }

        if (p_cur->atTokHasmore()) {
            out = p_cur->atTokNextstr(&err); //gw_addr
            if (err < 0) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing gw addr",
                        rid, __FUNCTION__);
                goto error;
            }
            out = NULL;
        }

        if (p_cur->atTokHasmore()) {
            out = p_cur->atTokNextstr(&err); //DNS_prim_addr
            if (err < 0) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing DNS prim addr",
                        rid, __FUNCTION__);
                goto error;
            }

            if (out != NULL && strlen(out) > 0 && strcmp(NULL_IPV4_ADDRESS, out) != 0) {
                int index = 0;
                if (RmcDcUtility::getAddressType(out) == IPV6) {
                    char dnsV6[MAX_IPV6_ADDRESS_LENGTH] = {0};
                    convertIpv6Address(dnsV6, out, 0);
                    index = v6Count[responseAid]++;
                    updateIpv6Dns(responseAid, index, dnsV6);
                    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] DNS%d V6 for aid%d is %s",
                            rid, __FUNCTION__, v6Count[responseAid], responseAid,
                            getIpv6Dns(responseAid, index));
                } else if (RmcDcUtility::getAddressType(out) == IPV4) {
                    index = v4Count[responseAid]++;
                    updateIpv4Dns(responseAid, index, out);
                    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] DNS%d V4 for aid%d is %s",
                            rid, __FUNCTION__, v4Count[responseAid], responseAid,
                            getIpv4Dns(responseAid, index));
                } else {
                   RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] DNS for aid%d is NOT V4 or V6 type",
                           rid, __FUNCTION__, responseAid);
                   goto error;
                }
            }
            out = NULL;
        }

        if (p_cur->atTokHasmore()) {
            out = p_cur->atTokNextstr(&err); //DNS_sec_addr
            if (err < 0) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing DNS sec addr",
                        rid, __FUNCTION__);
                goto error;
            }

            if (out != NULL && strlen(out) > 0 && strcmp(NULL_IPV4_ADDRESS, out) != 0) {
                int index = 0;
                if (RmcDcUtility::getAddressType(out) == IPV6) {
                    char dnsV6[MAX_IPV6_ADDRESS_LENGTH] = {0};
                    convertIpv6Address(dnsV6, out, 0);
                    index = v6Count[responseAid]++;
                    updateIpv6Dns(responseAid, index, dnsV6);
                    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] DNS%d V6 for aid%d is %s",
                            rid, __FUNCTION__, v6Count[responseAid], responseAid,
                            getIpv6Dns(responseAid, index));
                } else if (RmcDcUtility::getAddressType(out) == IPV4) {
                    index = v4Count[responseAid]++;
                    updateIpv4Dns(responseAid, index, out);
                    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] DNS%d V4 for aid%d is %s",
                            rid, __FUNCTION__, v4Count[responseAid], responseAid,
                            getIpv4Dns(responseAid, index));
                } else {
                   RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] DNS for aid%d is NOT V4 or V6 type",
                           rid, __FUNCTION__, responseAid);
                   goto error;
                }
            }
            out = NULL;
        }

        if (p_cur->atTokHasmore()) {
            out = p_cur->atTokNextstr(&err); //P-CSCF_prim_addr
            if (err < 0) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing P-CSCF prim addr",
                        rid, __FUNCTION__);
                goto error;
            }
            if (out != NULL && strlen(out) > 0 && strcmp(NULL_IPV4_ADDRESS, out) != 0) {
                int index = 0;
                if (RmcDcUtility::getAddressType(out) == IPV6) {
                    char pcscfV6[MAX_IPV6_ADDRESS_LENGTH] = {0};
                    convertIpv6Address(pcscfV6, out, 0);
                    updatePcscfAddress(responseAid, pcscfCount[responseAid]++, pcscfV6);
                    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] P-CSCF is %s",
                            rid, __FUNCTION__, pcscfV6);
                } else if (RmcDcUtility::getAddressType(out) == IPV4) {
                    updatePcscfAddress(responseAid, pcscfCount[responseAid]++, out);
                    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] P-CSCF is %s",
                            rid, __FUNCTION__, out);
                } else {
                   RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] P-CSCF for aid%d is NOT V4 or V6 type",
                           rid, __FUNCTION__, responseAid);
                   goto error;
                }
            }
            out = NULL;
        }

        if (p_cur->atTokHasmore()) {
            out = p_cur->atTokNextstr(&err); //P-CSCF_sec_addr
            if (err < 0) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing P-CSCF sec addr",
                        rid, __FUNCTION__);
                goto error;
            }
            if (out != NULL && strlen(out) > 0 && strcmp(NULL_IPV4_ADDRESS, out) != 0) {
                int index = 0;
                if (RmcDcUtility::getAddressType(out) == IPV6) {
                    char pcscfV6[MAX_IPV6_ADDRESS_LENGTH] = {0};
                    convertIpv6Address(pcscfV6, out, 0);
                    updatePcscfAddress(responseAid, pcscfCount[responseAid]++, pcscfV6);
                    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] P-CSCF is %s",
                            rid, __FUNCTION__, pcscfV6);
                } else if (RmcDcUtility::getAddressType(out) == IPV4) {
                    updatePcscfAddress(responseAid, pcscfCount[responseAid]++, out);
                    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] P-CSCF is %s",
                            rid, __FUNCTION__, out);
                } else {
                   RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] P-CSCF for aid%d is NOT V4 or V6 type",
                           rid, __FUNCTION__, responseAid);
                   goto error;
                }
            }
            out = NULL;
        }

        if (p_cur->atTokHasmore()) {
            int signalingFlag = p_cur->atTokNextint(&err); //IM_CN_Signalling_Flag
            if (err < 0) {
                setSignalingFlag(responseAid, 0);
                cmd.append(String8::format(" invalid signalingFlag"));
            } else {
                setSignalingFlag(responseAid, signalingFlag);
                RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] signalingFlag = %d for aid%d",
                        rid, __FUNCTION__, getSignalingFlag(responseAid), responseAid);
            }
        }

        if (p_cur->atTokHasmore()) {
            int lipaIndication = p_cur->atTokNextint(&err); //LIPA_indication
            if (err < 0) {
                cmd.append(String8::format(" invalid LIPA_indication"));
            } else {
                RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] LIPA_indication = %d for aid%d",
                        rid, __FUNCTION__, lipaIndication, responseAid);
            }
        }

        if (p_cur->atTokHasmore()) {
            int mtu = p_cur->atTokNextint(&err); //IPv4_MTU
            if (err < 0) {
                updateMtu(responseAid, 0);
                cmd.append(String8::format(" invalid mtu"));
            } else {
                updateMtu(responseAid, mtu);
                RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] mtu = %d for aid%d",
                        rid, __FUNCTION__, getMtu(responseAid), responseAid);
            }
        }

        if (!cmd.isEmpty()) {
            RFX_LOG_D(RFX_LOG_TAG, "[%d][%s]%s for aid%d",
                    rid, __FUNCTION__, cmd.string(), responseAid);
        }
    }

    nRet = PDN_SUCCESS;
    goto finish;

error:
    nRet = PDN_FAILED;

finish:
    FREEIF(pcscfCount);
    FREEIF(v4Count);
    FREEIF(v6Count);
    return nRet;
}

int RmcDcCommonReqHandler::updateDefaultBearerInfo(int aid) {
    //Be aware that this method is used only for retrieving P-CSCF address and signaling flag
    int nRet = PDN_FAILED;
    sp<RfxAtResponse> p_response;
    RfxAtLine *p_cur = NULL;
    int v4Count = 0;
    int v6Count = 0;
    int pcscfCount = 0;
    int rid = m_slot_id;
    int err = 0;
    int responseAid = 0;
    int bearerId = 0;
    char* apn = NULL;
    char* out = NULL;
    String8 cmd("");

    //clear default bearer information in AID table since we would query it again
    PdnInfo pdn_info = getPdnInfo(aid);
    memset(pdn_info.pcscf, 0, sizeof(pdn_info.pcscf));
    memset(pdn_info.dnsV4, 0, sizeof(pdn_info.dnsV4));
    memset(pdn_info.dnsV6, 0, sizeof(pdn_info.dnsV6));
    pdn_info.signalingFlag = 0;
    pdn_info.bearerId = INVALID_AID;
    pdn_info.mtu = 0;
    setPdnInfo(aid, &pdn_info);

    //+CGCONTRDP response a list of [+CGCONTRDP: <aid>,<bearer_id>,<apn>[,<local_addr and subnet_mask>
    //[,<gw_addr>[,<DNS_prim_addr>[,<DNS_sec_addr>[,<P-CSCF_prim_addr>[,<P-CSCF_sec_addr>
    //[,<IM_CN_Signalling_Flag>[,<LIPA_indication>[,<IPv4_MTU>[,<WLAN_Offload>[,<Local_Addr_Ind>
    //[,<Non-IP_MTU>]]]]]]]]]]]]]
    p_response = atSendCommandMultiline(String8::format("AT+CGCONTRDP=%d", aid), "+CGCONTRDP:");

    if (p_response == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get p_response!",
                rid, __FUNCTION__);
        return PDN_FAILED;
    }

    if (p_response->isAtResponseFail()) {
        if (p_response->getSuccess() > 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] weird case, can not get fail cause due to final"
                    " response indicates success", rid, __FUNCTION__);
        } else {
            int cause = p_response->atGetCmeError();
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] AT+CGCONTRDP for activatedAid%d returns cme cause : %d",
                    rid, __FUNCTION__, aid, cause);
        }
        return PDN_FAILED;
    }

    for (p_cur = p_response->getIntermediates(); p_cur != NULL; p_cur = p_cur->getNext()) {
        cmd.clear();
        p_cur->atTokStart(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when token start",
                    rid, __FUNCTION__);
            return PDN_FAILED;
        }

        responseAid = p_cur->atTokNextint(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing aid",
                    rid, __FUNCTION__);
            return PDN_FAILED;
        }

        if (responseAid != aid) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] aid%d is different to MD assigned aid%d",
                    rid, __FUNCTION__, aid, responseAid);
             return PDN_FAILED;
        }

        bearerId = p_cur->atTokNextint(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing bearer id",
                    rid, __FUNCTION__);
            return PDN_FAILED;
        }
        updateBearerId(aid, bearerId);

        apn = p_cur->atTokNextstr(&err); //apn
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing apn",
                    rid, __FUNCTION__);
            return PDN_FAILED;
        }

        if (p_cur->atTokHasmore()) {
            out = p_cur->atTokNextstr(&err); //local_addr and subnet_mask
            if (err < 0) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing local addr and subnet mask",
                        rid, __FUNCTION__);
                return PDN_FAILED;
            }
            out = NULL;
        }

        if (p_cur->atTokHasmore()) {
            out = p_cur->atTokNextstr(&err); //gw_addr
            if (err < 0) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing gw addr",
                        rid, __FUNCTION__);
                return PDN_FAILED;
            }
            out = NULL;
        }

        if (p_cur->atTokHasmore()) {
            out = p_cur->atTokNextstr(&err); //DNS_prim_addr
            if (err < 0) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing DNS prim addr",
                        rid, __FUNCTION__);
                return PDN_FAILED;
            }

            if (out != NULL && strlen(out) > 0 && strcmp(NULL_IPV4_ADDRESS, out) != 0) {
                int index = 0;
                if (RmcDcUtility::getAddressType(out) == IPV6) {
                    char dnsV6[MAX_IPV6_ADDRESS_LENGTH] = {0};
                    convertIpv6Address(dnsV6, out, 0);
                    index = v6Count++;
                    updateIpv6Dns(aid, index, dnsV6);
                    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] DNS%d V6 for aid%d is %s",
                            rid, __FUNCTION__, v6Count, aid, getIpv6Dns(aid, index));
                } else if (RmcDcUtility::getAddressType(out) == IPV4) {
                    index = v4Count++;
                    updateIpv4Dns(aid, index, out);
                    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] DNS%d V4 for aid%d is %s",
                            rid, __FUNCTION__, v4Count, aid, getIpv4Dns(aid, index));
                } else {
                   RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] DNS for aid%d is NOT V4 or V6 type",
                           rid, __FUNCTION__, aid);
                   return PDN_FAILED;
                }
            }
            out = NULL;
        }

        if (p_cur->atTokHasmore()) {
            out = p_cur->atTokNextstr(&err); //DNS_sec_addr
            if (err < 0) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing DNS sec addr",
                        rid, __FUNCTION__);
                return PDN_FAILED;
            }

            if (out != NULL && strlen(out) > 0 && strcmp(NULL_IPV4_ADDRESS, out) != 0) {
                int index = 0;
                if (RmcDcUtility::getAddressType(out) == IPV6) {
                    char dnsV6[MAX_IPV6_ADDRESS_LENGTH] = {0};
                    convertIpv6Address(dnsV6, out, 0);
                    index = v6Count++;
                    updateIpv6Dns(aid, index, dnsV6);
                    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] DNS%d V6 for aid%d is %s",
                            rid, __FUNCTION__, v6Count, aid, getIpv6Dns(aid, index));
                } else if (RmcDcUtility::getAddressType(out) == IPV4) {
                    index = v4Count++;
                    updateIpv4Dns(aid, index, out);
                    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] DNS%d V4 for aid%d is %s",
                            rid, __FUNCTION__, v4Count, aid, getIpv4Dns(aid, index));
                } else {
                   RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] DNS for aid%d is NOT V4 or V6 type",
                           rid, __FUNCTION__, aid);
                   return PDN_FAILED;
                }
            }
            out = NULL;
        }

        if (p_cur->atTokHasmore()) {
            out = p_cur->atTokNextstr(&err); //P-CSCF_prim_addr
            if (err < 0) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing P-CSCF prim addr",
                        rid, __FUNCTION__);
                return PDN_FAILED;
            }
            if (out != NULL && strlen(out) > 0 && strcmp(NULL_IPV4_ADDRESS, out) != 0) {
                int index = 0;
                if (RmcDcUtility::getAddressType(out) == IPV6) {
                    char pcscfV6[MAX_IPV6_ADDRESS_LENGTH] = {0};
                    convertIpv6Address(pcscfV6, out, 0);
                    updatePcscfAddress(aid, pcscfCount++, pcscfV6);
                    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] P-CSCF is %s",
                            rid, __FUNCTION__, pcscfV6);
                } else if (RmcDcUtility::getAddressType(out) == IPV4) {
                    updatePcscfAddress(aid, pcscfCount++, out);
                    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] P-CSCF is %s",
                            rid, __FUNCTION__, out);
                } else {
                   RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] P-CSCF for aid%d is NOT V4 or V6 type",
                           rid, __FUNCTION__, responseAid);
                   return PDN_FAILED;
                }
            }
            out = NULL;
        }

        if (p_cur->atTokHasmore()) {
            out = p_cur->atTokNextstr(&err); //P-CSCF_sec_addr
            if (err < 0) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing P-CSCF sec addr",
                        rid, __FUNCTION__);
                return PDN_FAILED;
            }
            if (out != NULL && strlen(out) > 0 && strcmp(NULL_IPV4_ADDRESS, out) != 0) {
                int index = 0;
                if (RmcDcUtility::getAddressType(out) == IPV6) {
                    char pcscfV6[MAX_IPV6_ADDRESS_LENGTH] = {0};
                    convertIpv6Address(pcscfV6, out, 0);
                    updatePcscfAddress(aid, pcscfCount++, pcscfV6);
                    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] P-CSCF is %s",
                            rid, __FUNCTION__, pcscfV6);
                } else if (RmcDcUtility::getAddressType(out) == IPV4) {
                    updatePcscfAddress(aid, pcscfCount++, out);
                    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] P-CSCF is %s",
                            rid, __FUNCTION__, out);
                } else {
                   RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] P-CSCF for aid%d is NOT V4 or V6 type",
                           rid, __FUNCTION__, aid);
                   return PDN_FAILED;
                }
            }
            out = NULL;
        }

        if (p_cur->atTokHasmore()) {
            int signalingFlag = p_cur->atTokNextint(&err); //IM_CN_Signalling_Flag
            if (err < 0) {
                setSignalingFlag(aid, 0);
                cmd.append(String8::format(" invalid signalingFlag"));
            } else {
                setSignalingFlag(aid, signalingFlag);
                RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] signalingFlag = %d for aid%d",
                        rid, __FUNCTION__, getSignalingFlag(aid), aid);
            }
        }

        if (p_cur->atTokHasmore()) {
            int lipaIndication = p_cur->atTokNextint(&err); //LIPA_indication
            if (err < 0) {
                cmd.append(String8::format(" invalid LIPA_indication"));
            } else {
                RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] LIPA_indication = %d for aid%d",
                        rid, __FUNCTION__, lipaIndication, responseAid);
            }
        }

        if (p_cur->atTokHasmore()) {
            int mtu = p_cur->atTokNextint(&err); //IPv4_MTU
            if (err < 0) {
                updateMtu(aid, 0);
                cmd.append(String8::format(" invalid mtu"));
            } else {
                updateMtu(aid, mtu);
                RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] mtu = %d for aid%d",
                        rid, __FUNCTION__, getMtu(aid), aid);
            }
        }

        if (!cmd.isEmpty()) {
            RFX_LOG_D(RFX_LOG_TAG, "[%d][%s]%s for aid%d",
                    rid, __FUNCTION__, cmd.string(), responseAid);
        }
    }

    return PDN_SUCCESS;
}

void RmcDcCommonReqHandler::updatePdnAddress() {
    const int pdnInfoSize = getPdnTableSize();
    int rid = m_slot_id;

    for (int i = 0; i < pdnInfoSize; i++) {
        if (DATA_STATE_INACTIVE != getPdnActiveStatus(i)) {
            if (PDN_SUCCESS != updatePdnAddress(i)) {
                RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] Fail to updatePdnAddress for aid%d",
                        rid, __FUNCTION__, i);
            }
        }
    }
}

int RmcDcCommonReqHandler::updatePdnAddress(int aid) {
    String8 cmd("");
    RfxAtLine *p_cur = NULL;
    sp<RfxAtResponse> p_response;
    int err = -1;
    int rid = m_slot_id;
    int cause = 0;
    int row = 1;
    int ipv6Index = 0;

    if (isDedicateBearer(aid)) {
        RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] NO need to update dedicate bearer address [aid%d]",
                rid, __FUNCTION__, aid);
        return PDN_SUCCESS;
    }

    /* Firstly clear all ip addresses with aid before update it */
    updateIpv4Address(aid, "");
    for (int i = 0; i < MAX_NUM_IPV6_ADDRESS_NUMBER; i++) {
        updateIpv6Address(aid, i, "");
    }

    /* Use AT+EPDN=<aid>,"addr" to query the ip address assigned to this PDP context indicated via this aid */
    cmd.append(String8::format("AT+EPDN=%d,\"addr\"", aid));

    // Response:
    //    +EPDN:<aid>,"addr",<address type>[,<address>]
    //    +EPDN:<aid>,"err",<err>
    p_response = atSendCommandMultiline(cmd, "+EPDN:");

    if (p_response == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get p_response for aid%d",
                rid, __FUNCTION__, aid);
        return PDN_FAILED;
    }

    if (p_response->isAtResponseFail()) {
        if (p_response->getSuccess() > 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] weird case, can not get fail cause for aid% "
                    "due to final response indicates success", rid, __FUNCTION__, aid);
        }
        return PDN_FAILED;
    }

    for (p_cur = p_response->getIntermediates(); p_cur != NULL; p_cur = p_cur->getNext()) {
        ADDRESS_TYPE addrType = ADDRESS_NULL;
        int responseAid = INVALID_AID;
        char *cmdFormat = NULL;
        char *address = NULL;

        p_cur->atTokStart(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when token start with aid%d for %d's row",
                    rid, __FUNCTION__, aid, row);
            return PDN_FAILED;
        }

        /* Get 1st parameter: AID */
        responseAid = p_cur->atTokNextint(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing aid%d for %d's row",
                    rid, __FUNCTION__, aid, row);
            return PDN_FAILED;
        }

        if (responseAid != aid) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR aid%d is different to MD assigned aid%d for %d's row",
                    rid, __FUNCTION__, aid, responseAid, row);
            return PDN_FAILED;
        }

        /* Get 2nd parameter: Cmd */
        cmdFormat = p_cur->atTokNextstr(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR parsing cmd with aid%d for %d's row",
                    rid, __FUNCTION__, aid, row);
            return PDN_FAILED;
        }

        if (strncmp(DC_ERROR, cmdFormat, strlen(DC_ERROR)) == 0) {
            /* Get 3rd paramter: Error*/
            if (p_cur->atTokHasmore()) {
                cause = p_cur->atTokNextint(&err);  // Not use currently.
                if (err < 0) {
                    RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing err cause for aid%d for %d's row",
                            rid, __FUNCTION__, aid, row);
                } else {
                    RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] AT+EPDN(addr) for aid%d returns err cause : %d for %d's row",
                            rid, __FUNCTION__, aid, cause, row);
                }
            }
            return PDN_FAILED;
        } else if (strncmp(DC_ADDRESS, cmdFormat, strlen(DC_ADDRESS)) == 0) {
            /* Get 3rd paramter: Address type*/
            if (p_cur->atTokHasmore()) {
                addrType = (ADDRESS_TYPE)p_cur->atTokNextint(&err);
                if (err < 0) {
                    RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR parsing address type with aid%d for %d's row",
                            rid, __FUNCTION__, aid, row);
                    return PDN_FAILED;
                }
            }

            /* Get 4th paramter: Address*/
            if (p_cur->atTokHasmore()) {
                address = p_cur->atTokNextstr(&err);
                if (err < 0) {
                    RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR parsing address with aid%d for %d's row",
                            rid, __FUNCTION__, aid, row);
                    return PDN_FAILED;
                }

                if (addrType == ADDRESS_IPV4) {
                    updateIpv4Address(aid, address);
                } else if (addrType == ADDRESS_IPV6_GLOBAL) {
                    if (!RmcDcUtility::isOp16Support()) {
                        char addressV6[MAX_IPV6_ADDRESS_LENGTH] = {0};
                        convertIpv6Address(addressV6, address, 0);
                        updateIpv6Address(aid, ipv6Index, addressV6);
                        ipv6Index++;
                    }
                } else {
                    RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] no matched address type (%s) with aid%d for %d's row",
                            rid, __FUNCTION__, RmcDcUtility::addrTypeToString(addrType), aid, row);
                }
            }
        } else {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] no matched command format (%s) with aid%d for %d's row",
                    rid, __FUNCTION__, cmdFormat, aid, row);
            return PDN_FAILED;
        }
        row++;
    }

    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] Response IP address [aid%d, IPv4Addr=%s, IPv6Addr=%s]",
            rid, __FUNCTION__, aid, getIpv4Address(aid), ipv6AddressToString(aid).string());

    return PDN_SUCCESS;
}

void RmcDcCommonReqHandler::initDataCallResponse(MTK_RIL_Data_Call_Response_v11* responses, int length) {
    for (int i = 0; i < length; i++) {
        memset(&responses[i], 0, sizeof(MTK_RIL_Data_Call_Response_v11));
        responses[i].status = PDP_FAIL_ERROR_UNSPECIFIED;
        responses[i].active = DATA_STATE_INACTIVE;
        responses[i].cid = INVALID_AID;
        responses[i].suggestedRetryTime = -1;
        responses[i].rat = 1; // 1: cellular
    }
}

void RmcDcCommonReqHandler::initAidList(int* list) {
    const int nPdnInfoSize = getPdnTableSize();
    for (int i = 0; i < nPdnInfoSize; i++) {
        list[i] = INVALID_AID;
    }
}

void RmcDcCommonReqHandler::createDataResponse(int transIntfId, int protocol,
        MTK_RIL_Data_Call_Response_v11* response) {
    int rid = m_slot_id;
    char addressV4[MAX_IPV4_ADDRESS_LENGTH] = {0};
    char addressV6[MAX_NUM_IPV6_ADDRESS_NUMBER][MAX_IPV6_ADDRESS_LENGTH] = {{0}};
    char dnsV4[MAX_NUM_DNS_ADDRESS_NUMBER][MAX_IPV4_ADDRESS_LENGTH] = {{0}};
    char dnsV6[MAX_NUM_DNS_ADDRESS_NUMBER][MAX_IPV6_ADDRESS_LENGTH] = {{0}};
    char pcscf[MAX_PCSCF_NUMBER][MAX_IPV6_ADDRESS_LENGTH] = {{0}};
    int addressV4Length = 0;
    int addressV6Length = 0;
    int nAddressLength = 0;
    int v4DnsLength = 0;
    int v6DnsLength = 0;
    int nDnsesLen = 0;
    int pcscfLength = 0;
    PdnInfo *pdnInfo = NULL;
    PdnInfo pdn_info;
    memset(&pdn_info, 0, sizeof(pdn_info));
    const int nPdnInfoSize = getPdnTableSize();

    if (response == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] response is null, return!",
                rid, __FUNCTION__);
        return;
    }

    for (int i = 0; i < nPdnInfoSize; i++) {
        pdn_info = getPdnInfo(i);
        int interfaceId = getInterfaceId(transIntfId);
        if ((interfaceId != INVALID_INTERFACE_ID) &&
                (pdn_info.transIntfId == transIntfId) &&
                !pdn_info.isDedicateBearer &&
                (pdn_info.active == DATA_STATE_ACTIVE)) {
            RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] copy pdn_info[%d] to data response for transIntfId=%d",
                    rid, __FUNCTION__, i, transIntfId);

            pdnInfo = &pdn_info;
            // There is aid bind to the interface of the response, so set to active
            response->active = pdnInfo->active;
            asprintf(&response->ifname, "%s%d",
                    NetAgentService::getCcmniInterfaceName(rid), interfaceId);

            if (strlen(pdnInfo->addressV4) > 0 && (protocol == IPV4 || protocol == IPV4V6)) {
                strncpy(addressV4, pdnInfo->addressV4, MAX_IPV4_ADDRESS_LENGTH - 1);
                addressV4Length += strlen(addressV4);
            }

            for (int j = 0; j < MAX_NUM_IPV6_ADDRESS_NUMBER; j++) {
                if (strlen(pdnInfo->addressV6[j]) > 0 && (protocol == IPV6 || protocol == IPV4V6)) {
                    strncpy(addressV6[j], pdnInfo->addressV6[j], MAX_IPV6_ADDRESS_LENGTH - 1);
                    addressV6Length += strlen(addressV6[j]);
                    if (j != 0 || addressV4Length != 0)
                        ++addressV6Length;  // add one space
                }
            }

            for (int j = 0; j < MAX_NUM_DNS_ADDRESS_NUMBER; j++) {
                // If there is no IP address but with corresponding
                // DNS server, it may result in TCP/IP break down due
                // to DNS server algorithm from N, so need check if
                // have IP address when report DNS
                if ((strlen(pdnInfo->dnsV4[j]) > 0) && (addressV4Length > 0)) {
                    strncpy(dnsV4[j], pdnInfo->dnsV4[j], MAX_IPV4_ADDRESS_LENGTH - 1);
                    v4DnsLength += strlen(pdnInfo->dnsV4[j]);
                    if (j != 0 || v6DnsLength != 0)
                        ++v4DnsLength;  // add one space
                }
                if ((strlen(pdnInfo->dnsV6[j]) > 0) && (addressV6Length > 0)) {
                    strncpy(dnsV6[j], pdnInfo->dnsV6[j], MAX_IPV6_ADDRESS_LENGTH - 1);
                    v6DnsLength += strlen(pdnInfo->dnsV6[j]);
                    if (j != 0 || v4DnsLength != 0)
                        ++v6DnsLength;  // add one space
                }
            }

            for (int j = 0; j < MAX_PCSCF_NUMBER; j++) {
                if (strlen(pdnInfo->pcscf[j]) > 0) {
                    strncpy(pcscf[j], pdnInfo->pcscf[j], MAX_IPV6_ADDRESS_LENGTH - 1);
                    pcscfLength += strlen(pdnInfo->pcscf[j]);
                    if (j != 0)
                        ++pcscfLength;  // add one space
                }
            }

            response->mtu = pdnInfo->mtu;
            response->rat = pdnInfo->rat;
        } else if (pdn_info.transIntfId == transIntfId) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] not need to create pdn_info[%d] for transIntfId=%d"
                    " because pdn_info{isDedicateBearer=%d, active=%d} is not satisfied",
                    rid, __FUNCTION__, i, transIntfId, pdn_info.isDedicateBearer,
                    pdn_info.active);
        }
    }

    response->status = PDP_FAIL_NONE;
    response->cid = transIntfId;

    nAddressLength = addressV4Length + addressV6Length;
    response->addresses = (char*)calloc(1, (nAddressLength * sizeof(char)) + 1);
    RFX_ASSERT(response->addresses != NULL);
    memset(response->addresses, 0, (nAddressLength * sizeof(char)) + 1);
    response->gateways = (char*)calloc(1, (nAddressLength * sizeof(char)) + 1);
    RFX_ASSERT(response->gateways != NULL);
    memset(response->gateways, 0, (nAddressLength * sizeof(char)) + 1);

    if (addressV4Length > 0) {
        strncat(response->addresses + strlen(response->addresses), addressV4,
            nAddressLength - strlen(response->addresses));
        strncat(response->gateways + strlen(response->gateways), addressV4,
            nAddressLength - strlen(response->gateways));
    }
    for (int i = 0; i < MAX_NUM_IPV6_ADDRESS_NUMBER; i++) {
        if (addressV6Length > 0 && strlen(addressV6[i]) > 0) {
            int currentLength = strlen(response->addresses);
            if (currentLength > 0) {
                strncat(response->addresses + currentLength, " ", nAddressLength - currentLength);
            }
            strncat(response->addresses + strlen(response->addresses), addressV6[i],
                    nAddressLength - strlen(response->addresses));

            currentLength = strlen(response->gateways);
            if (currentLength > 0) {
                strncat(response->gateways + currentLength, " ", nAddressLength - currentLength);
            }
            if (isIpv6Global(addressV6[i])) {
                strncat(response->gateways + strlen(response->gateways), UNSPECIFIED_IPV6_GATEWAY,
                        nAddressLength - strlen(response->gateways));
            } else {
                strncat(response->gateways + strlen(response->gateways), addressV6[i],
                    nAddressLength - strlen(response->gateways));
            }
        }
    }

    if (addressV4Length > 0 && addressV6Length > 0) {
        asprintf(&response->type, SETUP_DATA_PROTOCOL_IPV4V6);
    } else if (addressV6Length > 0) {
        asprintf(&response->type, SETUP_DATA_PROTOCOL_IPV6);
    } else {
        asprintf(&response->type, SETUP_DATA_PROTOCOL_IP);
    }

    nDnsesLen = v4DnsLength + v6DnsLength;
    response->dnses = (char*)calloc(1, (nDnsesLen * sizeof(char)) + 1);
    RFX_ASSERT(response->dnses != NULL);
    memset(response->dnses, 0, (nDnsesLen * sizeof(char)) + 1);

    for (int i = 0; i < MAX_NUM_DNS_ADDRESS_NUMBER; i++) {
        if (v6DnsLength > 0 && strlen(dnsV6[i]) > 0) {
            int currentLength = strlen(response->dnses);
            if (currentLength > 0) {
                strncat(response->dnses + currentLength, " ", nDnsesLen - currentLength);
            }
            strncat(response->dnses + strlen(response->dnses), dnsV6[i],
                    nDnsesLen - strlen(response->dnses));
        }
    }
    for (int i = 0; i < MAX_NUM_DNS_ADDRESS_NUMBER; i++) {
        if (v4DnsLength > 0 && strlen(dnsV4[i]) > 0) {
            int currentLength = strlen(response->dnses);
            if (currentLength > 0) {
                strncat(response->dnses + currentLength, " ", nDnsesLen - currentLength);
            }
            strncat(response->dnses + strlen(response->dnses), dnsV4[i],
                    nDnsesLen - strlen(response->dnses));
        }
    }

    response->pcscf= (char*)calloc(1, pcscfLength * sizeof(char) + 1);
    RFX_ASSERT(response->pcscf != NULL);
    memset(response->pcscf, 0, pcscfLength * sizeof(char) + 1);
    for (int i = 0; i < MAX_PCSCF_NUMBER; i++) {
        if (pcscfLength > 0 && strlen(pcscf[i]) > 0) {
            int currentLength = strlen(response->pcscf);
            if (currentLength > 0) {
                strncat(response->pcscf + currentLength, " ", pcscfLength - currentLength);
                currentLength = strlen(response->pcscf);
            }
            strncat(response->pcscf + currentLength, pcscf[i], pcscfLength - currentLength);
        }
    }
}

String8 RmcDcCommonReqHandler::responsesToString(MTK_RIL_Data_Call_Response_v11* responses, int num) {
    int rid = m_slot_id;
    String8 cmd("");

    if (num == 0 || responses == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] the number of data call responses is %d "
                "(if number is not 0, that means data call responses is null)",
                rid, __FUNCTION__, num);
        return String8::format("");
    }

    cmd.append(String8::format("DataCallResponse <-"));

    for (int i = 0; i < num; i++) {
        cmd.append(String8::format(" {status=%d, suggestedRetryTime=%d, transIntfId=%d, active=%d,"
                " type=%s, ifname=%s, addresses=%s, dnses=%s, gateways=%s, pcscf=%s, mtu=%d, rat=%d}",
                (&responses[i])->status,
                (&responses[i])->suggestedRetryTime,
                (&responses[i])->cid,
                (&responses[i])->active,
                (&responses[i])->type,
                (&responses[i])->ifname,
                (&responses[i])->addresses,
                (&responses[i])->dnses,
                (&responses[i])->gateways,
                (&responses[i])->pcscf,
                (&responses[i])->mtu,
                (&responses[i])->rat));
    }
    return cmd;
}

String8 RmcDcCommonReqHandler::ipv6AddressToString(int aid) {
    int rid = m_slot_id;
    String8 cmd("");

    // Check if AID is out of bound.
    if (!validateAid(aid)) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] aid=%d is out of bound",
                rid, __FUNCTION__, aid);
        return String8::format("");
    }

    for (int i = 0; i < MAX_NUM_IPV6_ADDRESS_NUMBER; i++) {
        if (i != 0) {
            cmd.append(String8::format("%s", ", "));
        }
        cmd.append(String8::format("%s", getIpv6Address(aid, i)));
    }

    return cmd;
}

void RmcDcCommonReqHandler::freeDataResponse(MTK_RIL_Data_Call_Response_v11* response) {
    if (response->ifname != NULL) {
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] type=%s, ifname=%s, addresses=%s, dnses=%s, gateways=%s, pcscf=%s",
                m_slot_id,
                __FUNCTION__,
                response->type,
                response->ifname,
                response->addresses,
                response->dnses,
                response->gateways,
                response->pcscf);
    }

    FREEIF(response->type);
    FREEIF(response->ifname);
    FREEIF(response->addresses);
    FREEIF(response->gateways);
    FREEIF(response->dnses);
    FREEIF(response->pcscf);
}

int RmcDcCommonReqHandler::convertIpv6Address(char* output, char* input, int isLinkLocal) {
    // input : 32.1.13.184.0.0.0.3.61.48.97.182.50.254.113.251
    // output : 2001:0DB8:0000:0003:3D30:61B6:32FE:71FB
    int rid = m_slot_id;
    char *p = NULL;
    char *address = NULL;
    int value = 0;
    int len = 0;
    int count = 8;
    char tmpOutput[MAX_IPV6_ADDRESS_LENGTH] = {0};

    if (output == NULL || input == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] output or input is NULL", rid, __FUNCTION__);
        return -1;
    }

    if (isLinkLocal == 1) {
        strncpy(output, IPV6_PREFIX, MAX_IPV6_ADDRESS_LENGTH - 1);
    } else {
        strncpy(output, NULL_ADDR, MAX_IPV6_ADDRESS_LENGTH - 1);
    }

    address = strsep(&input, "/");
    p = strtok(address, ".");

    //Skip the 64 bit of this PDP address if this pdp adress is a local link address
    if (isLinkLocal == 1) {
        while (count) {
            p = strtok(NULL, ".");
            count--;
        }
    }

    while (p) {
        memset(tmpOutput, 0, MAX_IPV6_ADDRESS_LENGTH);
        value = atoi(p);
        snprintf(tmpOutput, MAX_IPV6_ADDRESS_LENGTH, "%02X", value);
        strncat(output + strlen(output), tmpOutput, MAX_IPV6_ADDRESS_LENGTH-strlen(output)-1);
        p = strtok(NULL, ".");
        if (p == NULL) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] The format of IP address is illegal", rid, __FUNCTION__);
            return -1;
        }
        memset(tmpOutput, 0, MAX_IPV6_ADDRESS_LENGTH);
        value = atoi(p);
        snprintf(tmpOutput, MAX_IPV6_ADDRESS_LENGTH, "%02X:", value);
        strncat(output + strlen(output), tmpOutput, MAX_IPV6_ADDRESS_LENGTH-strlen(output)-1);
        p = strtok(NULL, ".");
    }

    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] before convertIpv6Address: %s", rid, __FUNCTION__, output);
    len = strlen(output);
    output[len-1] = '\0';

    if (input != NULL) {
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] IPv6 prefix length:%s", rid, __FUNCTION__, input);
        memset(tmpOutput, 0, MAX_IPV6_ADDRESS_LENGTH);
        snprintf(tmpOutput, MAX_IPV6_ADDRESS_LENGTH, "/%s", input);
        strncat(output + strlen(output), tmpOutput, MAX_IPV6_ADDRESS_LENGTH-strlen(output)-1);
    }

    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] convertIpv6Address: %s", rid, __FUNCTION__, output);

    return 0;
}

bool RmcDcCommonReqHandler::isIpv6Global(char *ipv6Addr) {
    int rid = m_slot_id;
    char *address = NULL;
    String8 cmd("");

    address = strsep(&ipv6Addr, "/");
    cmd.append(String8::format("{IPv6 address:%s, IPv6 prefix length:%s}", address, ipv6Addr));

    if (address != NULL) {
        struct sockaddr_in6 sa;
        int ret = 0;

        if (strncasecmp("FE80", address, strlen("FE80")) == 0) {
            RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] not global -> %s", rid, __FUNCTION__, cmd.string());
            return false;
        }

        // ret: -1, error occurs, ret: 0, invalid address, ret: 1, success;
        ret = inet_pton(AF_INET6, address, &(sa.sin6_addr));
        if (ret <= 0) {
            RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] inet_pton ret: %d -> %s",
                    rid, __FUNCTION__, ret, cmd.string());
            return false;
        }

        if (IN6_IS_ADDR_MULTICAST(&sa.sin6_addr)) {
            RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] multi-cast -> %s", rid, __FUNCTION__, cmd.string());
            if (IN6_IS_ADDR_MC_GLOBAL(&sa.sin6_addr)) {
                RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] global -> %s", rid, __FUNCTION__, cmd.string());
                return true;
            } else {
                RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] not global -> %s", rid, __FUNCTION__, cmd.string());
            }
        } else {
            if (IN6_IS_ADDR_LINKLOCAL(&sa.sin6_addr)) {
                RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] link-local -> %s", rid, __FUNCTION__, cmd.string());
            } else if (IN6_IS_ADDR_SITELOCAL(&sa.sin6_addr)) {
                RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] site-local -> %s", rid, __FUNCTION__, cmd.string());
            } else if (IN6_IS_ADDR_V4MAPPED(&sa.sin6_addr)) {
                RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] v4mapped -> %s", rid, __FUNCTION__, cmd.string());
            } else if (IN6_IS_ADDR_V4COMPAT(&sa.sin6_addr)) {
                RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] v4compat -> %s", rid, __FUNCTION__, cmd.string());
            } else if (IN6_IS_ADDR_LOOPBACK(&sa.sin6_addr)) {
                RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] host -> %s", rid, __FUNCTION__, cmd.string());
            } else if (IN6_IS_ADDR_UNSPECIFIED(&sa.sin6_addr)) {
                RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] unspecified -> %s", rid, __FUNCTION__, cmd.string());
            } else if (_IN6_IS_ULA(&sa.sin6_addr)) {
                RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] take uni-local as global -> %s",
                        rid, __FUNCTION__, cmd.string());
                return true;
            } else {
                RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] global -> %s", rid, __FUNCTION__, cmd.string());
                return true;
            }
        }
    } else {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] input ipv6 address is null!!", rid, __FUNCTION__);
    }
    return false;
}

void RmcDcCommonReqHandler::resetPco(int aid) {
    String8 cmd("");
    sp<RfxAtResponse> p_response;
    int rid = m_slot_id;

    cmd.append(String8::format("AT+EGPCO=%d", aid));
    p_response = atSendCommand(cmd);

    if (p_response == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get p_response!",
                rid, __FUNCTION__);
        return;
    }

    if (p_response->isAtResponseFail()) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d] %s returns ERROR", rid, cmd.string());
    }
}

void RmcDcCommonReqHandler::setPco(int aid, const char *option, const char *content) {
    String8 cmd("");
    sp<RfxAtResponse> p_response;
    int rid = m_slot_id;

    cmd.append(String8::format("AT+EGPCO=%d,\"%s\",\"%s\"", aid, option, content));
    p_response = atSendCommand(cmd);

    if (p_response == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get p_response!",
                rid, __FUNCTION__);
        return;
    }

    if (p_response->isAtResponseFail()) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d] %s returns ERROR", rid, cmd.string());
    }
}

void RmcDcCommonReqHandler::setPco(int aid, const char *buf) {
    String8 cmd("");
    sp<RfxAtResponse> p_response;
    int rid = m_slot_id;

    cmd.append(String8::format("AT+EGPCO=%d,%s", aid, buf));
    p_response = atSendCommand(cmd);

    if (p_response == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get p_response!",
                rid, __FUNCTION__);
        return;
    }

    if (p_response->isAtResponseFail()) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d] %s returns ERROR", rid, cmd.string());
    }
}

void RmcDcCommonReqHandler::getPco(int aid, const char *option, Vector<String8>& vContent) {
    // for PDN_AP_INITIATED case, apn and its index are not needed to get PCO, and are NULL
    getPco(PDN_AP_INITIATED, aid, NULL, option, vContent);
}

void RmcDcCommonReqHandler::getPco(int apnidx, const char* apn,
        const char *option, Vector<String8>& vContent) {
    // for PDN_MD_INITIATED case, apnidx/apn/proto in previous URC are used to get PCO
    getPco(PDN_MD_INITIATED, apnidx, apn, option, vContent);
}

void RmcDcCommonReqHandler::getPco(PDN_INITIATOR who, int idx, const char* apn,
        const char *option, Vector<String8>& vContent) {
    String8 cmd("");
    sp<RfxAtResponse> p_response;
    RfxAtLine *p_cur = NULL;
    int err = 0;
    int mdRspIdx = INVALID_AID;
    int rid = m_slot_id;
    char *mdOption = NULL;
    char *mdContent = NULL;

    if (option == NULL) {
        // obtain all PCO values from modem
        if (who == PDN_AP_INITIATED) {
            cmd.append(String8::format("AT+EGPCORDP=%d", idx));
        } else if (who == PDN_MD_INITIATED) {
            cmd.append(String8::format("AT+BGPCORDP=\"%s\",%d", apn, idx));
        } else {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] requested PDN format error!",
                    rid, __FUNCTION__);
            return;
        }
    } else {
        // obtain specific PCO value from modem
        if (who == PDN_AP_INITIATED) {
            cmd.append(String8::format("AT+EGPCORDP=%d,\"%s\"", idx, option));
        } else if (who == PDN_MD_INITIATED) {
            cmd.append(String8::format("AT+BGPCORDP=\"%s\",%d,\"%s\"", apn, idx, option));
        } else {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] requested PDN format error!",
                    rid, __FUNCTION__);
            return;
        }
    }

    if (who == PDN_AP_INITIATED) {
        p_response = atSendCommandMultiline(cmd, "+EGPCORDP:");
    } else if (who == PDN_MD_INITIATED) {
        p_response = atSendCommandMultiline(cmd, "+BGPCORDP:");
    } else {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] requested PDN format error!",
            rid, __FUNCTION__);
    }

    if (p_response == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get p_response!",
                rid, __FUNCTION__);
        return;
    }

    if (p_response->isAtResponseFail()) {
        RFX_LOG_D(RFX_LOG_TAG, "[%d] %s returns ERROR", rid, cmd.string());
        return;
    }

    for (p_cur = p_response->getIntermediates(); p_cur != NULL; p_cur = p_cur->getNext()) {
        p_cur->atTokStart(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when token start",
                    rid, __FUNCTION__);
            return;
        }

        mdRspIdx = p_cur->atTokNextint(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get idx",
                    rid, __FUNCTION__);
            return;
        }

        mdOption = p_cur->atTokNextstr(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get option or it's different to MD assigned",
                    rid, __FUNCTION__);
            return;
        }

        mdContent = p_cur->atTokNextstr(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get content",
                    rid, __FUNCTION__);
            return;
        }
        vContent.push_back(String8::format("%s,%s", mdOption, mdContent));
    }
}

bool RmcDcCommonReqHandler::isDataAllowed(const char* pReqApn) {
    char *apn = NULL;
    int bearer_bitmask = 0;
    bool bSupportWifiBearer = false;
    // 0 - Not registered
    // 1 - Registered
    int nWfcState = getMclStatusManager(m_slot_id)->getIntValue(RFX_STATUS_KEY_WFC_STATE, 0);
    // 0 - Not registered
    // 1 - Registered, home network
    // 2 - Not registered, but MT is currently searching a new operator to register
    // 3 - Registration denied
    // 4 - Unknown
    // 5 - Registered, roaming
    int nPsState = getMclStatusManager(m_slot_id)->getIntValue(RFX_STATUS_KEY_CELLULAR_PS_STATE, 0);
    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] nWfcState = %d, nPsState = %d, roaming data = %d",
            m_slot_id, __FUNCTION__, nWfcState, nPsState, s_dataSetting[m_slot_id][ROAMING_DATA]);

    if (1 == nWfcState) {
        for (int i = 0; i < s_nLastReqNum[m_slot_id]; i++) {
            if (0 == strcmp(pReqApn, s_LastApnTable[m_slot_id][i].apn)) {
                bearer_bitmask = s_LastApnTable[m_slot_id][i].bearerBitmask;
                RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] bearer_bitmask = %d",
                        m_slot_id, __FUNCTION__, bearer_bitmask);
                if (0 == bearer_bitmask
                        ||((bearer_bitmask & (1 << (RADIO_TECH_IWLAN - 1))) != 0)) {
                    bSupportWifiBearer = true;
                }
                break;
            }
        }
        if (!bSupportWifiBearer && 5 == nPsState && 0 == s_dataSetting[m_slot_id][ROAMING_DATA]) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] the PS state is roaming but data roaming is off",
                    m_slot_id, __FUNCTION__);
            return false;
        }
    }
    return true;
}

int RmcDcCommonReqHandler::getInterfaceId(int transIntfId) {
    if (0 <= transIntfId) {
        return transIntfId % TRANSACTION_ID_OFFSET;
    }
    return INVALID_INTERFACE_ID;
}

void RmcDcCommonReqHandler::requestClearAllPdnInfo(const sp<RfxMclMessage>& msg) {
    RFX_UNUSED(msg);
    RFX_LOG_I(RFX_LOG_TAG, "[%d][%s]", m_slot_id, __FUNCTION__);
    RFX_ASSERT(m_pPdnManager != NULL);
    m_pPdnManager->clearAllPdnInfo();
}

void RmcDcCommonReqHandler::requestQueryPco(int idx, int ia, const char* apn, const char* iptype) {
    if (!RmcDcUtility::isOp12Support()) {
        return;
    }

    std::string option = "FF00";
    Vector<String8> contents;

    char simMccMnc[MAX_LENGTH_MCCMNC] = {0};
    getMccmnc(simMccMnc);
    if (!RmcDcUtility::isOp12MccMnc(simMccMnc)) {
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] query PCO return: [%s] not OP12 SIM",
                m_slot_id, __FUNCTION__, simMccMnc);
        return;
    }
    if (getPdnActiveStatus(idx) == DATA_STATE_INACTIVE) {
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] no need to check inactive aid %d",
                m_slot_id, __FUNCTION__, idx);
        return;
    }

    if (ia) {
        getPco(idx, apn, option.c_str(), contents);
    } else {
        getPco(idx, option.c_str(), contents);
    }
    if (contents.size() == 0) {
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] query PCO return: no contents",
                m_slot_id, __FUNCTION__);
        return;
    }

    // pcoContent ex: FF00,311480:3
    // delimiter->        A      B
    std::string pcoContent = std::string(contents.itemAt(0).string());
    int delimiterA = pcoContent.find(",");
    if (delimiterA < 0) {
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] query PCO return: content's format missing ','",
                m_slot_id, __FUNCTION__);
        return;
    }

    int delimiterB = pcoContent.find(":");
    if (delimiterB < 0) {
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] query PCO return: content's format missing ':'",
                m_slot_id, __FUNCTION__);
        return;
    }

    if (option.compare(pcoContent.substr(0, delimiterA))) {
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] query PCO return: content's option is different",
                m_slot_id, __FUNCTION__);
        return;
    }

    std::string pcoValue = pcoContent.substr(delimiterB+1, std::string::npos);
    std::string pcoMccMnc = pcoContent.substr(delimiterA+1, delimiterB-delimiterA-1);

    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] idx:%d has PCO, content: %s, mccmnc: %s, val: %s,",
            m_slot_id, __FUNCTION__, idx, pcoContent.c_str(), pcoMccMnc.c_str(),
            pcoValue.c_str());

    sp<RfxMclMessage> urcMsg;
    if (ia) {
        RIL_PCO_Data_attached pcoDataIa;
        pcoDataIa.cid = getTransIntfId(idx);
        pcoDataIa.apn_name = (char*) apn;
        pcoDataIa.bearer_proto = (char*) iptype;
        pcoDataIa.pco_id = std::stoi(option, NULL, 16);
        pcoDataIa.contents_length = 1;
        pcoDataIa.contents = (char*) pcoValue.c_str();
        urcMsg = RfxMclMessage::obtainUrc(RFX_MSG_UNSOL_PCO_DATA_AFTER_ATTACHED,
                m_slot_id, RfxPcoIaData(&pcoDataIa, sizeof(pcoDataIa)));

        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] PCO cid %d, apn %s, proto %s, pco_id %d, contents %s",
                m_slot_id, __FUNCTION__, pcoDataIa.cid, pcoDataIa.apn_name,
                pcoDataIa.bearer_proto, pcoDataIa.pco_id, pcoDataIa.contents);
    } else {
        int tid = getTransIntfId(idx);
        int protoBitMask = 0; // 1:v4, 2:v6, 4:v4v6
        for (int i = 0; i < getPdnTableSize(); i++) {
            if (tid == getTransIntfId(i)) {
                std::string ipv4 = std::string(getIpv4Address(i));
                std::string ipv6 = std::string(getIpv6Address(i));
                ipv4.erase(std::remove_if(begin(ipv4), end(ipv4), ::isspace), end(ipv4));
                ipv6.erase(std::remove_if(begin(ipv6), end(ipv6), ::isspace), end(ipv6));
                protoBitMask |= (ipv4.size() > 0 && ipv6.size() == 0) ? 1<<IPV4:
                        (ipv4.size() == 0 && ipv6.size() > 0) ? 1<<IPV6:
                        (ipv4.size() > 0 && ipv6.size() > 0) ? 1<<IPV4V6 : 1<<IPV4;
            }
        }
        std::string proto;
        switch (protoBitMask) {
            case 1<<IPV4:
                proto = SETUP_DATA_PROTOCOL_IP;
                break;
            case 1<<IPV6:
                proto = SETUP_DATA_PROTOCOL_IPV6;
                break;
            case (1<<IPV4 | 1<<IPV6):
            case 1<<IPV4V6:
                proto = SETUP_DATA_PROTOCOL_IPV4V6;
                break;
            default:
                proto = SETUP_DATA_PROTOCOL_IP;
                break;
        }

        RIL_PCO_Data pcoData;
        pcoData.cid = getTransIntfId(idx);
        pcoData.bearer_proto = (char*) proto.c_str();
        pcoData.pco_id = std::stoi(option, NULL, 16);
        pcoData.contents_length = 1;
        pcoData.contents = (char*) pcoValue.c_str();
        urcMsg = RfxMclMessage::obtainUrc(RFX_MSG_UNSOL_PCO_DATA, m_slot_id,
                RfxPcoData(&pcoData, sizeof(pcoData)));

        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] PCO cid %d, proto %s, pcoid %d, contents %s",
                m_slot_id, __FUNCTION__,
                pcoData.cid, pcoData.bearer_proto, pcoData.pco_id, pcoData.contents);
    }

    String8 pcoStatusKey = String8(option.c_str());
    pcoStatusKey.append(":");
    pcoStatusKey.append(pcoValue.c_str());
    getMclStatusManager()->setString8Value(RFX_STATUS_KEY_PCO_STATUS, pcoStatusKey);
    responseToTelCore(urcMsg);
}

int RmcDcCommonReqHandler::confirmPdnUsage(const int aid, const bool bUsed, int sscMode) {
    int rid = m_slot_id;
    String8 cmd("");
    sp<RfxAtResponse> p_response;

    // Check if AID is out of bound.
    if (!validateAid(aid)) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] aid=%d is out of bound",
                rid, __FUNCTION__, aid);
        return CME_ERROR_NON_CME;
    }

    // AT+EAPNACT=<state>,<aid>;
    // <state>: 2 - NOT to use this PDN and NOT to receive "+CGEV" URCs for the corresponding bearers;
    // <state>: 3 - use this PDN and to receive "+CGEV" URCs for the corresponding bearers;
    // <state>: 4 - NOT to use this SSC mode3 PDU and NOT to receive "+CGEV" URCs for the corresponding bearers;
    // <state>: 5 - use this SSC mode3 PDU and to receive "+CGEV" URCs for the corresponding bearers;
    if (bUsed) {
        if (sscMode == SSC_MODE3) {
            cmd.append(String8::format("AT+EAPNACT=5,%d", aid));
        } else {
            cmd.append(String8::format("AT+EAPNACT=3,%d", aid));
        }
    } else {
        if (sscMode == SSC_MODE3) {
            cmd.append(String8::format("AT+EAPNACT=4,%d", aid));
        } else {
            cmd.append(String8::format("AT+EAPNACT=2,%d", aid));
        }
    }
    p_response = atSendCommand(cmd);

    if (p_response == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get p_response!",
                rid, __FUNCTION__);
        return CME_ERROR_NON_CME;
    }

    if (p_response->isAtResponseFail()) {
        if (p_response->getSuccess() == 0) {
            int cause = p_response->atGetCmeError();
            RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] CME CAUSE : %d",
                    rid, __FUNCTION__, cause);
            return cause;
        } else {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] CME CAUSE : CME_UNKNOWN",
                    rid, __FUNCTION__);
            return CME_UNKNOWN;
        }
    }

    return CME_SUCCESS;
}

int RmcDcCommonReqHandler::updatePdnDeactInfo(const int aid) {
    int rid = m_slot_id;
    int err = 0;
    RfxAtLine *pLine = NULL;
    char *cmdFormat = NULL;
    int rspAid = INVALID_AID;
    int reason = NO_REASON;
    int rat = 2454;  // default value.
    int cause = 0;
    sp<RfxAtResponse> p_response;

    // Check if AID is out of bound.
    if (!validateAid(aid)) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] aid=%d is out of bound",
                rid, __FUNCTION__, aid);
        return PDN_FAILED;
    }

    // AT+EPDN=<aid>,"deact_info"
    // Response:
    //    +EPDN:<aid>,"deact_info",<reason>,<rat type>
    //    +EPDN:<aid>,"err",<err>
    p_response = atSendCommandSingleline(String8::format("AT+EPDN=%d,\"deact_info\"", aid), "+EPDN:");

    if (p_response == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get p_response!",
                rid, __FUNCTION__);
        return PDN_FAILED;
    }

    if (p_response->isAtResponseFail()) {
        if (p_response->getSuccess() > 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] weird case, can not get fail cause due to final"
                    " response indicates success", rid, __FUNCTION__);
        } else {
            cause = p_response->atGetCmeError();
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] AT+EPDN(deact_info) for aid%d returns cme cause : %d",
                    rid, __FUNCTION__, aid, cause);
        }
        return PDN_FAILED;
    }

    pLine = p_response->getIntermediates();

    if (pLine == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get intermediate results!",
                rid, __FUNCTION__);
        return PDN_FAILED;
    }

    pLine->atTokStart(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when token start",
                rid, __FUNCTION__);
        return PDN_FAILED;
    }

    rspAid = pLine->atTokNextint(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing aid",
                rid, __FUNCTION__);
        return PDN_FAILED;
    }

    if (rspAid != aid) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] aid%d is different to MD assigned aid%d",
                rid, __FUNCTION__, aid, rspAid);
        return PDN_FAILED;
    }

    cmdFormat = pLine->atTokNextstr(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing cmd",
                rid, __FUNCTION__);
        return PDN_FAILED;
    }

    if (strncmp(DC_ERROR, cmdFormat, strlen(DC_ERROR)) == 0) {
        if (pLine->atTokHasmore()) {
            cause = pLine->atTokNextint(&err);  // Not use currently.
            if (err < 0) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing err cause",
                        rid, __FUNCTION__);
            } else {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] AT+EPDN(deact_info) for aid%d returns err cause : %d",
                        rid, __FUNCTION__, aid, cause);
            }
        }
        return PDN_FAILED;
    } else if (strncmp(DC_DEACT_INFO, cmdFormat, strlen(DC_DEACT_INFO)) == 0) {
        reason = pLine->atTokNextint(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing reason",
                    rid, __FUNCTION__);
            return PDN_FAILED;
        }

        if (reason == NO_REASON) {
            RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] no reason for pdn deact aid%d",
                    rid, __FUNCTION__, aid);
        } else {
            setDeactReason(aid, convertFailCauseToRilStandard(reason));
        }

        if (pLine->atTokHasmore()) {
            rat = pLine->atTokNextint(&err);  // Not use currently.
            if (err < 0) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing rat type",
                        rid, __FUNCTION__);
                return PDN_FAILED;
            }
        }
    } else {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] no matched command format (%s)",
                rid, __FUNCTION__, cmdFormat);
        return PDN_FAILED;
    }

    return PDN_SUCCESS;
}

bool RmcDcCommonReqHandler::isFallbackPdn(const int aid) {
    int rid = m_slot_id;
    int pdnsOfActive = 0;
    PdnInfo pdn_info;
    memset(&pdn_info, 0, sizeof(pdn_info));
    const int nPdnInfoSize = getPdnTableSize();
    String8 cmd("");

    // Check if AID is out of bound.
    if (!validateAid(aid)) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] aid=%d is out of bound",
                rid, __FUNCTION__, aid);
        return false;
    }

    if (getPdnActiveStatus(aid) != DATA_STATE_ACTIVE) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] no need to check non-active aid %d",
                rid, __FUNCTION__, aid);
        return false;
    }

    for (int i = 0; i < nPdnInfoSize; i++) {
        pdn_info = getPdnInfo(i);
        if ((pdn_info.active == DATA_STATE_ACTIVE) &&
                (getTransIntfId(pdn_info.aid) == getTransIntfId(aid))) {
            if (0 == pdnsOfActive) {
                cmd.append(String8::format("%d", pdn_info.aid));
            } else {
                cmd.append(String8::format(",%d", pdn_info.aid));
            }
            pdnsOfActive++;
        }
    }

    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] %s for aid=%d (all Of active aid <%s> for transIntfId=%d)",
            rid, __FUNCTION__, BTOS(pdnsOfActive > 1), aid, cmd.string(), getTransIntfId(aid));
    return (pdnsOfActive > 1) ? true : false;
}

bool RmcDcCommonReqHandler::notifyDeactReasonIfNeeded(const int deactivatedAid) {
    int rid = m_slot_id;

    // Check if AID is out of bound.
    if (!validateAid(deactivatedAid)) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] deactivatedAid=%d is out of bound",
                rid, __FUNCTION__, deactivatedAid);
        return false;
    }

    // Update the reason of why pdn was deactivated to FWK for checking if needs to do pdn retrying
    // except that there has no existed reason or another of fallback pdn of the same apn is still alive.
    if (getDeactReason(deactivatedAid) != NO_REASON && !isFallbackPdn(deactivatedAid)) {
        RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] notify deact reason %d for {deactAid=%d, transIntfId=%d}",
                rid, __FUNCTION__, getDeactReason(deactivatedAid), deactivatedAid,
                getTransIntfId(deactivatedAid));
        return true;
    }
    return false;
}

bool RmcDcCommonReqHandler::validateAid(int aid) {
    RFX_ASSERT(m_pPdnManager != NULL);
    return m_pPdnManager->validateAid(aid);
}

PdnInfo RmcDcCommonReqHandler::getPdnInfo(int aid) {
    RFX_ASSERT(m_pPdnManager != NULL);
    return m_pPdnManager->getPdnInfo(aid);
}

void RmcDcCommonReqHandler::setPdnInfo(int aid, PdnInfo* pdnInfo) {
    RFX_ASSERT(m_pPdnManager != NULL);
    m_pPdnManager->setPdnInfo(aid, pdnInfo);
}

void RmcDcCommonReqHandler::clearPdnInfo(int aid) {
    RFX_ASSERT(m_pPdnManager != NULL);
    m_pPdnManager->clearPdnInfo(aid);
}

bool RmcDcCommonReqHandler::isDedicateBearer(int aid) {
    RFX_ASSERT(m_pPdnManager != NULL);
    return m_pPdnManager->isDedicateBearer(aid);
}

void RmcDcCommonReqHandler::setSignalingFlag(int aid, int flag) {
    RFX_ASSERT(m_pPdnManager != NULL);
    m_pPdnManager->setSignalingFlag(aid, flag);
}

void RmcDcCommonReqHandler::setAid(int index, int aid) {
    RFX_ASSERT(m_pPdnManager != NULL);
    m_pPdnManager->setAid(index, aid);
}

void RmcDcCommonReqHandler::setAidAndPrimaryAid(int index, int aid, int primaryAid) {
    RFX_ASSERT(m_pPdnManager != NULL);
    m_pPdnManager->setAidAndPrimaryAid(index, aid, primaryAid);
}

void RmcDcCommonReqHandler::setIsEmergency(int aid, bool isEmergency) {
    RFX_ASSERT(m_pPdnManager != NULL);
    m_pPdnManager->setIsEmergency(aid, isEmergency);
}

void RmcDcCommonReqHandler::setIsDedicateBearer(int aid, bool isDedicateBearer) {
    RFX_ASSERT(m_pPdnManager != NULL);
    m_pPdnManager->setIsDedicateBearer(aid, isDedicateBearer);
}

void RmcDcCommonReqHandler::setReason(int aid, int reason) {
    RFX_ASSERT(m_pPdnManager != NULL);
    m_pPdnManager->setReason(aid, reason);
}

void RmcDcCommonReqHandler::setDeactReason(int aid, int deactReason) {
    RFX_ASSERT(m_pPdnManager != NULL);
    m_pPdnManager->setDeactReason(aid, deactReason);
}

void RmcDcCommonReqHandler::setSscMode(int aid, int sscMode) {
    RFX_ASSERT(m_pPdnManager != NULL);
    m_pPdnManager->setSscMode(aid, sscMode);
}

void RmcDcCommonReqHandler::setPdnType(int aid, int pdnType) {
    RFX_ASSERT(m_pPdnManager != NULL);
    m_pPdnManager->setPdnType(aid, pdnType);
}

void RmcDcCommonReqHandler::setProfileId(int aid, int profileId) {
    RFX_ASSERT(m_pPdnManager != NULL);
    m_pPdnManager->setProfileId(aid, profileId);
}

int RmcDcCommonReqHandler::getPdnTableSize() {
    RFX_ASSERT(m_pPdnManager != NULL);
    return m_pPdnManager->getPdnTableSize();
}

int RmcDcCommonReqHandler::getPdnActiveStatus(int aid) {
    RFX_ASSERT(m_pPdnManager != NULL);
    return m_pPdnManager->getPdnActiveStatus(aid);
}

int RmcDcCommonReqHandler::getTransIntfId(int aid) {
    RFX_ASSERT(m_pPdnManager != NULL);
    return m_pPdnManager->getTransIntfId(aid);
}

char* RmcDcCommonReqHandler::getApnName(int aid) {
    RFX_ASSERT(m_pPdnManager != NULL);
    return m_pPdnManager->getApnName(aid);
}

int RmcDcCommonReqHandler::getAid(int index) {
    RFX_ASSERT(m_pPdnManager != NULL);
    return m_pPdnManager->getAid(index);
}

int RmcDcCommonReqHandler::getPrimaryAid(int index) {
    RFX_ASSERT(m_pPdnManager != NULL);
    return m_pPdnManager->getPrimaryAid(index);
}

char* RmcDcCommonReqHandler::getIpv4Dns(int aid, int index) {
    RFX_ASSERT(m_pPdnManager != NULL);
    return m_pPdnManager->getIpv4Dns(aid, index);
}

char* RmcDcCommonReqHandler::getIpv6Dns(int aid, int index) {
    RFX_ASSERT(m_pPdnManager != NULL);
    return m_pPdnManager->getIpv6Dns(aid, index);
}

int RmcDcCommonReqHandler::getMtu(int aid) {
    RFX_ASSERT(m_pPdnManager != NULL);
    return m_pPdnManager->getMtu(aid);
}

int RmcDcCommonReqHandler::getSignalingFlag(int aid) {
    RFX_ASSERT(m_pPdnManager != NULL);
    return m_pPdnManager->getSignalingFlag(aid);
}

char* RmcDcCommonReqHandler::getIpv4Address(int aid) {
    RFX_ASSERT(m_pPdnManager != NULL);
    return m_pPdnManager->getIpv4Address(aid);
}

char* RmcDcCommonReqHandler::getIpv6Address(int aid) {
    RFX_ASSERT(m_pPdnManager != NULL);
    return m_pPdnManager->getIpv6Address(aid, 0);
}

char* RmcDcCommonReqHandler::getIpv6Address(int aid, int index) {
    RFX_ASSERT(m_pPdnManager != NULL);
    return m_pPdnManager->getIpv6Address(aid, index);
}

int RmcDcCommonReqHandler::getReason(int aid) {
    RFX_ASSERT(m_pPdnManager != NULL);
    return m_pPdnManager->getReason(aid);
}

int RmcDcCommonReqHandler::getDeactReason(int aid) {
    RFX_ASSERT(m_pPdnManager != NULL);
    return m_pPdnManager->getDeactReason(aid);
}

int RmcDcCommonReqHandler::getSscMode(int aid) {
    RFX_ASSERT(m_pPdnManager != NULL);
    return m_pPdnManager->getSscMode(aid);
}

int RmcDcCommonReqHandler::getPdnType(int aid) {
    RFX_ASSERT(m_pPdnManager != NULL);
    return m_pPdnManager->getPdnType(aid);
}

int RmcDcCommonReqHandler::getProfileId(int aid) {
    RFX_ASSERT(m_pPdnManager != NULL);
    return m_pPdnManager->getProfileId(aid);
}

int RmcDcCommonReqHandler::getSupportNetworkTypes(int apnTypeId) {
    int retValue = SUPPORT_TRANSPORT_MODE_UNKNOW;
    int supportedTypesBitmask = 0;
    int bearerBitmask = 0;

    bool bSupportWifiBearer = false;
    bool bSupportCellularBearer = false;

    for (int i = 0; i < s_nLastReqNum[m_slot_id]; i++){
        supportedTypesBitmask = s_LastApnTable[m_slot_id][i].supportedTypesBitmask;
        RFX_LOG_I(RFX_LOG_TAG, "[%d][%s]apnTypeId=%d supportedTypesBitmask=%d", m_slot_id,
                __FUNCTION__, apnTypeId, supportedTypesBitmask);
        // get bearerBitmast of apn type in APN Table
        if (supportedTypesBitmask & apnTypeId) {
            bearerBitmask = s_LastApnTable[m_slot_id][i].bearerBitmask;
            RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] bearerBitmask=%d", m_slot_id, __FUNCTION__,
                bearerBitmask);
            if (bearerBitmask == 0) {  // support all types
                retValue = SUPPORT_TRANSPORT_MODE_CELLULAR_WIFI;
                return retValue;
            } else {
                if ((bearerBitmask & (1 << (RADIO_TECH_IWLAN - 1))) != 0) {
                    bSupportWifiBearer = true;
                }
                if ((bearerBitmask & (1 << (RADIO_TECH_LTE - 1))) != 0) {
                    bSupportCellularBearer = true;
                }
            }
        }
    }
    RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] bSupportWifiBearer=%d bSupportCellularBearer=%d",
            m_slot_id, __FUNCTION__, bSupportWifiBearer, bSupportCellularBearer);
    if (bSupportWifiBearer && bSupportCellularBearer) {
        retValue = SUPPORT_TRANSPORT_MODE_CELLULAR_WIFI;
    } else {
        if (bSupportWifiBearer) {
            retValue = SUPPORT_TRANSPORT_MODE_WIFI_ONLY;
        } else if (bSupportCellularBearer) {
            retValue = SUPPORT_TRANSPORT_MODE_CELLULAR_ONLY;
        } else {
            return SUPPORT_TRANSPORT_MODE_UNKNOW;
        }
    }
    return retValue;
}

void RmcDcCommonReqHandler::updatePdnActiveStatus(int aid, int pdnActiveStatus) {
    RFX_ASSERT(m_pPdnManager != NULL);
    m_pPdnManager->updatePdnActiveStatus(aid, pdnActiveStatus);
}

void RmcDcCommonReqHandler::updateApnName(int aid, const char* apnName) {
    RFX_ASSERT(m_pPdnManager != NULL);
    m_pPdnManager->updateApnName(aid, apnName);
}

void RmcDcCommonReqHandler::updateTransIntfId(int aid, int transIntfId) {
    RFX_ASSERT(m_pPdnManager != NULL);
    m_pPdnManager->updateTransIntfId(aid, transIntfId);
}

void RmcDcCommonReqHandler::updateMtu(int aid, int mtu) {
    RFX_ASSERT(m_pPdnManager != NULL);
    m_pPdnManager->updateMtu(aid, mtu);
}

void RmcDcCommonReqHandler::updateRat(int aid, int rat) {
    RFX_ASSERT(m_pPdnManager != NULL);
    m_pPdnManager->updateRat(aid, rat);
}

void RmcDcCommonReqHandler::updateIpAddress(int aid, const char* ipv4Addr, const char* ipv6Addr) {
    RFX_ASSERT(m_pPdnManager != NULL);
    m_pPdnManager->updateIpAddress(aid, ipv4Addr, 0, ipv6Addr);
}

void RmcDcCommonReqHandler::updateIpv4Address(int aid, const char* ipv4Addr) {
    RFX_ASSERT(m_pPdnManager != NULL);
    m_pPdnManager->updateIpv4Address(aid, ipv4Addr);
}

void RmcDcCommonReqHandler::updateIpv6Address(int aid, const char* ipv6Addr) {
    RFX_ASSERT(m_pPdnManager != NULL);
    m_pPdnManager->updateIpv6Address(aid, 0, ipv6Addr);
}

void RmcDcCommonReqHandler::updateIpv6Address(int aid, int index, const char* ipv6Addr) {
    RFX_ASSERT(m_pPdnManager != NULL);
    m_pPdnManager->updateIpv6Address(aid, index, ipv6Addr);
}

void RmcDcCommonReqHandler::updateBearerId(int aid, int bearerId) {
    RFX_ASSERT(m_pPdnManager != NULL);
    m_pPdnManager->updateBearerId(aid, bearerId);
}

void RmcDcCommonReqHandler::updatePcscfAddress(int aid, int index, const char* pcscfAddr) {
    RFX_ASSERT(m_pPdnManager != NULL);
    m_pPdnManager->updatePcscfAddress(aid, index, pcscfAddr);
}

void RmcDcCommonReqHandler::updateIpv4Dns(int aid, int index, const char* v4Dns) {
    RFX_ASSERT(m_pPdnManager != NULL);
    m_pPdnManager->updateIpv4Dns(aid, index, v4Dns);
}

void RmcDcCommonReqHandler::updateIpv6Dns(int aid, int index, const char* v6Dns) {
    RFX_ASSERT(m_pPdnManager != NULL);
    m_pPdnManager->updateIpv6Dns(aid, index, v6Dns);
}

bool RmcDcCommonReqHandler::isSupportWifiBearer(int bearerBitmask) {
    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] bearerBitmask = %d", m_slot_id,
            __FUNCTION__, bearerBitmask);
    return ((bearerBitmask & (1 << (RADIO_TECH_IWLAN - 1))) != 0);
}

int RmcDcCommonReqHandler::getCmdIndexFromApnTable(const int slot_id, ApnTableReq *pApnTableReq) {
    int cmdIndex = 1;

    RFX_LOG_V(RFX_LOG_TAG, "[%d][%s] pApnTableReq apn=%s, user=%s, pwd=%s, apnTypeId=%d, protocol=%s, authtype=%d",
            slot_id, __FUNCTION__, pApnTableReq->apn, pApnTableReq->username, pApnTableReq->password,
            pApnTableReq->apnTypeId, pApnTableReq->protocol, pApnTableReq->authtype);

    // apn;user;password;type(default,mms,...);protocol;roaming_protocol;
    // authtype;carrier_enabled;max_conns;max_conns_time;wait_time;bearer_bitmask;inactive_timer;cmd_index
    for (int i = 0; i < s_nLastReqNum[slot_id]; i++) {

        RFX_LOG_V(RFX_LOG_TAG, "[%d][%s] s_nLastReqNum[%d][%d] apn=%s, user=%s, pwd=%s, supportedTypesBitmask=%d, protocol=%s, authtype=%d",
                slot_id, __FUNCTION__, slot_id, i, s_LastApnTable[slot_id][i].apn, s_LastApnTable[slot_id][i].user,
                s_LastApnTable[slot_id][i].password, s_LastApnTable[slot_id][i].supportedTypesBitmask,
                s_LastApnTable[slot_id][i].protocol, s_LastApnTable[slot_id][i].authType);

        if (pApnTableReq->apn && 0 != strcasecmp(pApnTableReq->apn, s_LastApnTable[slot_id][i].apn)) {
            RFX_LOG_V(RFX_LOG_TAG, "[%d][%s] apn is not the same: %s",
                    slot_id, __FUNCTION__, s_LastApnTable[slot_id][i].apn);
            continue;
        }

        if (pApnTableReq->username && 0 != strcasecmp(pApnTableReq->username, s_LastApnTable[slot_id][i].user)) {
            RFX_LOG_V(RFX_LOG_TAG, "[%d][%s] user is not the same: %s",
                    slot_id, __FUNCTION__, s_LastApnTable[slot_id][i].user);
            continue;
        }

        if (pApnTableReq->password && 0 != strcasecmp(pApnTableReq->password, s_LastApnTable[slot_id][i].password)) {
            RFX_LOG_V(RFX_LOG_TAG, "[%d][%s] pwd is not the same: %s",
                    slot_id, __FUNCTION__, s_LastApnTable[slot_id][i].password);
            continue;
        }

        if (pApnTableReq->apnTypeId != APN_TYPE_INVALID) {
            if ((s_LastApnTable[slot_id][i].supportedTypesBitmask & pApnTableReq->apnTypeId) != pApnTableReq->apnTypeId) {
                RFX_LOG_V(RFX_LOG_TAG, "[%d][%s] supportedTypesBitmask is not supported: %d",
                        slot_id, __FUNCTION__, s_LastApnTable[slot_id][i].supportedTypesBitmask);
                continue;
            }
        }

        if (pApnTableReq->protocol&& 0 != strcasecmp(pApnTableReq->protocol, s_LastApnTable[slot_id][i].protocol)) {
            RFX_LOG_V(RFX_LOG_TAG, "[%d][%s] protocol is not the same: %s",
                    slot_id, __FUNCTION__, s_LastApnTable[slot_id][i].protocol);
            continue;
        }
        if (pApnTableReq->authtype != s_LastApnTable[slot_id][i].authType) {
            RFX_LOG_V(RFX_LOG_TAG, "[%d][%s] authtype is not the same: %d",
                    slot_id, __FUNCTION__, s_LastApnTable[slot_id][i].authType);
            continue;
        }

        cmdIndex = s_ApnCmdIndex[slot_id][i];
        break;
    }

    return (cmdIndex-1);
}

void RmcDcCommonReqHandler::addEscapeSequence(char *buffer) {
    char tmp[MAX_APN_NAME_LENGTH] = {0};
    char tmp2[2] = {0};

    if (strlen(buffer) == 0) {
        return;
    }

    strncpy(tmp, buffer, MAX_APN_NAME_LENGTH-1);
    memset(buffer, 0, MAX_APN_NAME_LENGTH);
    for (unsigned int i = 0; i < strlen(tmp) && strlen(buffer) < (MAX_APN_NAME_LENGTH-3); i++) {
        if (tmp[i] == '\"') {
            strncat(buffer + strlen(buffer), "\\22", MAX_APN_NAME_LENGTH-strlen(buffer)-1);
        } else if (tmp[i] == '\\') {
            strncat(buffer + strlen(buffer), "\\5C", MAX_APN_NAME_LENGTH-strlen(buffer)-1);
        } else if (tmp[i] == '\n') {
            strncat(buffer + strlen(buffer), "\\0D", MAX_APN_NAME_LENGTH-strlen(buffer)-1);
        } else {
            memset(tmp2, 0, 2);
            strncpy(tmp2, &tmp[i], 1);
            strncat(buffer + strlen(buffer), tmp2, MAX_APN_NAME_LENGTH-strlen(buffer)-1);
        }
    }
}

// Support requirements to cleanup all data connections.
// This function is used for sync data connections status between Java Framework
// and RILD when phone process reinitiate.
void RmcDcCommonReqHandler::handleResetAllConnections(const sp<RfxMclMessage>& msg) {
    RFX_LOG_D(RFX_LOG_TAG, "[%s]", __FUNCTION__);
    sp<RfxMclMessage> responseMsg;
    const int nPdnInfoSize = getPdnTableSize();

    for (int i = 0; i < nPdnInfoSize; i++) {
        int aid = getAid(i);
        if (aid != INVALID_AID && !isDedicateBearer(i)) {
            deactivateDataCall(aid);
            clearPdnInfo(aid);
        }
    }

    responseMsg = RfxMclMessage::obtainResponse(RIL_E_SUCCESS, RfxVoidData(), msg);
    responseToTelCore(responseMsg);
}

void RmcDcCommonReqHandler::requestSetPreferredDataModem(const sp<RfxMclMessage>& msg) {
    int *pReqData = (int *)msg->getData()->getData();
    // 0: slot 0
    // 1: slot 1
    int preferredMd = pReqData[0];
    int rid = m_slot_id;
    RIL_Errno ret = RIL_E_SUCCESS;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> responseMsg;

    RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] requestSetPreferredDataModem, preferredMd=%d",
            rid, __FUNCTION__, preferredMd);

    if (preferredMd >= RFX_SLOT_ID_0 && preferredMd < MAX_RFX_SLOT_ID) {
        char feature[] = "EDATASIM Supported";
        if (getFeatureVersion(feature) == 1) {
            // AT+EDATASIM=<sim_slot>  sim_slot = 0(clear) or 1/2/3/4(slot)
            p_response = atSendCommand(String8::format("AT+EDATASIM=%d", preferredMd + 1));
            if (p_response == NULL) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get p_response!", rid, __FUNCTION__);
                ret = RIL_E_GENERIC_FAILURE;
            } else if (p_response->isATCmdRspErr()) {
                // If sync default data SIM to MD meet error, then keep sync data settings to MD.
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] AT command got ERROR", rid, __FUNCTION__);
                ret = RIL_E_GENERIC_FAILURE;
            }
        } else {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] EDATASIM not supported.", rid, __FUNCTION__);
        }
    }

    responseMsg = RfxMclMessage::obtainResponse(ret, RfxVoidData(), msg);
    responseToTelCore(responseMsg);
}

int RmcDcCommonReqHandler::createDataProfiles(RIL_MtkDataProfileInfo **oldDataProfilePtrs,
        RIL_MtkDataProfileInfo **dataProfilePtrs, int reqNum) {
    for (int i = 0; i < reqNum + MAX_COUNT_EXTRA_APN; i++) {
        dataProfilePtrs[i] = (RIL_MtkDataProfileInfo*) calloc(1, sizeof(RIL_MtkDataProfileInfo));
        if (i >= reqNum) {
            initialDataProfile(dataProfilePtrs[i], NULL);
        } else {
            initialDataProfile(dataProfilePtrs[i], oldDataProfilePtrs[i]);
        }
    }

    reqNum += insertExtraApns(dataProfilePtrs, reqNum);
    adjustBearerBitmask(dataProfilePtrs, reqNum);
    return reqNum;
}

void RmcDcCommonReqHandler::initialDataProfile(RIL_MtkDataProfileInfo *dest,
        RIL_MtkDataProfileInfo *src) {
    if (src == NULL) {
        dest->type = 1;
        dest->enabled = 1;
        asprintf(&dest->apn, "%s", "");
        asprintf(&dest->protocol, "%s", "IP");
        asprintf(&dest->roamingProtocol, "%s", "IP");
        asprintf(&dest->user, "%s", "");
        asprintf(&dest->password, "%s", "");
        asprintf(&dest->mvnoType, "%s", "");
        asprintf(&dest->mvnoMatchData, "%s", "");
    } else {
        memcpy(dest, src, sizeof(RIL_MtkDataProfileInfo));
        asprintf(&dest->apn, "%s", src->apn);
        asprintf(&dest->protocol, "%s", src->protocol);
        asprintf(&dest->roamingProtocol, "%s", src->roamingProtocol);
        asprintf(&dest->user, "%s", src->user);
        asprintf(&dest->password, "%s", src->password);
        asprintf(&dest->mvnoType, "%s", src->mvnoType);
        asprintf(&dest->mvnoMatchData, "%s", src->mvnoMatchData);
    }
}

void RmcDcCommonReqHandler::deleteDataProfiles(RIL_MtkDataProfileInfo **dest, int count) {
    for (int i = 0; i < count; i++) {
        if (dest[i] != NULL) {
            FREEIF(dest[i]->apn);
            FREEIF(dest[i]->protocol);
            FREEIF(dest[i]->roamingProtocol);
            FREEIF(dest[i]->user);
            FREEIF(dest[i]->password);
            FREEIF(dest[i]->mvnoType);
            FREEIF(dest[i]->mvnoMatchData);
            FREEIF(dest[i]);
        }
    }
    FREEIF(dest);
}

/**
 * As OEM has no emergency type in apns-conf.xml, need add emergency type
 * or add a new apn including emergency type that from vendor-apns-conf.xml.
 * 1. If check the apns are the same, add emergency type to existed apn.
 * 2. If the apns are not the same, add a new apn with emergency type.
 */
int RmcDcCommonReqHandler::insertExtraApns(RIL_MtkDataProfileInfo **dataProfilePtrs, int count) {
    TiXmlDocument *apnXml = new TiXmlDocument();
    if (!apnXml->LoadFile("vendor/etc/vendor-apns-conf.xml", TIXML_ENCODING_UTF8)) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] Load apn xml error", m_slot_id, __FUNCTION__);
        delete apnXml;
        return 0;
    }

    TiXmlElement* apns = apnXml->FirstChildElement("apns");
    if (apns == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] apns element is NULL", m_slot_id, __FUNCTION__);
        delete apnXml;
        return 0;
    }

    char mccmnc[MAX_LENGTH_MCCMNC] = {0};
    if (!getMccmnc(mccmnc)) {
        delete apnXml;
        return 0;
    }

    RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] mccmnc = %s", m_slot_id, __FUNCTION__, mccmnc);
    std::string mcc = std::string(mccmnc).substr(0, 3);
    std::string mnc = std::string(mccmnc).substr(3, std::string::npos);

    int index = 0;
    TiXmlElement* elem = apns->FirstChildElement("apn");
    while (elem) {
        TiXmlAttribute* attr = elem->FirstAttribute();
        if (attr == NULL) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] attr is NULL", m_slot_id, __FUNCTION__);
            continue;
        }

        bool match = false;
        std::string attrName = "";
        std::string attrValue = "";
        while (attr) {
            attrName = attr->Name();
            attrValue = attr->Value();

            // Leave directly if mccmnc is not matched.
            if ((attrName.compare("mcc") == 0 && attrValue.compare(mcc) != 0)
                    || (attrName.compare("mnc") == 0 && attrValue.compare(mnc) != 0)
                    || (attrName.compare("type") == 0
                    && attrValue.find("emergency") == std::string::npos)) {
                match = false;
                break;
            }
            match = true;

            setApnParameter(dataProfilePtrs[index + count], attrName, attrValue);
            attr = attr->Next();
        }

        if (match) {
            bool insert = true;
            for (int i = 0; i < count; i++) {
                if (compareApn(dataProfilePtrs[i], dataProfilePtrs[index + count])) {
                    RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] find same apn index = %d",
                            m_slot_id, __FUNCTION__, i);
                    insertExtraTypes(dataProfilePtrs[i], dataProfilePtrs[index + count]);
                    updateBearerBitmak(dataProfilePtrs[i], dataProfilePtrs[index + count]);
                    insert = false;
                    break;
                }
            }
            if (insert) {
                index++;
                RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] insert count = %d",
                       m_slot_id, __FUNCTION__, index);
                if (index == MAX_COUNT_EXTRA_APN) {
                    delete apnXml;
                    return index;
                }
            } else {
                initialDataProfile(dataProfilePtrs[index + count] , NULL);
            }
        } else {
            initialDataProfile(dataProfilePtrs[index + count] , NULL);
        }

        elem = elem->NextSiblingElement("apn");
    }

    delete apnXml;
    return index;
}

bool RmcDcCommonReqHandler::compareApn(RIL_MtkDataProfileInfo *profile1,
        RIL_MtkDataProfileInfo *profile2) {
    // Currently only check apn/mvnoType/mvnoMatchData, if equal, treat as the same apn.
    // other paramenters will compliance with OEM's apn configuration.
    if (strcasecmp(profile1->apn, profile2->apn) != 0) {
        return false;
    }
    if (strcasecmp(profile1->mvnoType, profile2->mvnoType) != 0) {
        return false;
    }
    if (strcasecmp(profile1->mvnoMatchData, profile2->mvnoMatchData) != 0) {
        return false;
    }

    return true;
}

void RmcDcCommonReqHandler::insertExtraTypes(RIL_MtkDataProfileInfo *dest,
        RIL_MtkDataProfileInfo *src) {
    // Currently only check emergency, add other types if need.
    if (src->supportedTypesBitmask & RIL_APN_TYPE_EMERGENCY) {
        dest->supportedTypesBitmask = (dest->supportedTypesBitmask | RIL_APN_TYPE_EMERGENCY);
    }
}

void RmcDcCommonReqHandler::updateBearerBitmak(RIL_MtkDataProfileInfo *dest,
        RIL_MtkDataProfileInfo *src) {
    if (isSupportWifiBearer(src->bearerBitmask)) {
        if (dest->bearerBitmask == 0) {
            dest->bearerBitmask = ALL_BEARER_BITMASK;
        } else {
            dest->bearerBitmask |= (1 << (RADIO_TECH_IWLAN - 1));
        }
    }
}

void RmcDcCommonReqHandler::setApnParameter(RIL_MtkDataProfileInfo *profile,
        std::string name, std::string value) {
    if (name.compare("carrier") == 0) {
        return;
    }
    if (name.compare("apn") == 0) {
        asprintf(&profile->apn, "%s", value.c_str());
        return;
    }
    if (name.compare("protocol") == 0) {
        asprintf(&profile->protocol, "%s", value.c_str());
        return;
    }
    if (name.compare("roaming_protocol") == 0) {
        asprintf(&profile->roamingProtocol, "%s", value.c_str());
        return;
    }
    if (name.compare("authtype") == 0) {
        profile->authType = atoi(value.c_str());
        return;
    }
    if (name.compare("user") == 0) {
        asprintf(&profile->user, "%s", value.c_str());
        return;
    }
    if (name.compare("password") == 0) {
        asprintf(&profile->password, "%s", value.c_str());
        return;
    }
    if (name.compare("wait_time") == 0) {
        profile->waitTime = atoi(value.c_str());
        return;
    }
    if (name.compare("max_conns") == 0) {
        profile->maxConns = atoi(value.c_str());
        return;
    }
    if (name.compare("max_conns_time") == 0) {
        profile->maxConnsTime = atoi(value.c_str());
        return;
    }
    if (name.compare("type") == 0) {
        // Currently only check emergency, add other types if need.
        if (value.find("emergency") != std::string::npos) {
            profile->supportedTypesBitmask |= RIL_APN_TYPE_EMERGENCY;
        }
        return;
    }
    if (name.compare("enabled") == 0) {
        profile->enabled = atoi(value.c_str());
        return;
    }

    if (name.compare("network_type_bitmask") == 0) {
        int bitmask = 0;
        int singleBitmask = -1;
        std::string::size_type p1, p2;
        p1 = 0;
        p2 = value.find("|");
        if (p2 == std::string::npos) {
            singleBitmask = atoi(value.c_str()) - 1;
            if (singleBitmask >= 0) {
                bitmask |= (1 << singleBitmask);
            }
        } else {
            std::string b = "";
            while(std::string::npos != p2) {
                b = value.substr(p1, p2 - p1);
                singleBitmask = atoi(b.c_str()) - 1;
                if (singleBitmask >= 0) {
                    bitmask |= (1 << singleBitmask);
                }
                p1 = p2 + std::string("|").size();
                p2 = value.find("|", p1);
            }
            b = value.substr(p1, std::string::npos);
            singleBitmask = atoi(b.c_str()) - 1;
            if (singleBitmask >= 0) {
                bitmask |= (1 << singleBitmask);
            }
        }
        profile->bearerBitmask = bitmask;
        return;
    }
    if (name.compare("mtu") == 0) {
        profile->mtu = atoi(value.c_str());
        return;
    }
    if (name.compare("mvno_type") == 0) {
        asprintf(&profile->mvnoType, "%s", value.c_str());
        return;
    }
    if (name.compare("mvno_match_data") == 0) {
        asprintf(&profile->mvnoMatchData, "%s", value.c_str());
        return;
    }
}

bool RmcDcCommonReqHandler::getMccmnc(char* mccmnc) {
    char key[MAX_LENGTH_PROPERTY_NAME] = {0};
    if (m_slot_id == 0) {
        snprintf(key, MAX_LENGTH_PROPERTY_NAME - 1, "%s", PROPERTY_MCC_MNC);
    } else {
        snprintf(key, MAX_LENGTH_PROPERTY_NAME - 1, "%s%s%d", PROPERTY_MCC_MNC, ".", m_slot_id);
    }
    rfx_property_get(key, mccmnc, "0");
    if (strlen(mccmnc) > 3) {
        return true;
    }

    memset(mccmnc, 0, MAX_LENGTH_MCCMNC);
    memset(key, 0, MAX_LENGTH_PROPERTY_NAME);
    snprintf(key, MAX_LENGTH_PROPERTY_NAME - 1, "%s%d", "vendor.ril.data.gsm_mcc_mnc", m_slot_id);
    rfx_property_get(key, mccmnc, "0");
    if (strlen(mccmnc) > 3) {
        return true;
    }

    memset(mccmnc, 0, MAX_LENGTH_MCCMNC);
    memset(key, 0, MAX_LENGTH_PROPERTY_NAME);
    if (m_slot_id == 0) {
        snprintf(key, MAX_LENGTH_PROPERTY_NAME - 1, "%s", PROPERTY_MCC_MNC_CDMA);
    } else {
        snprintf(key, MAX_LENGTH_PROPERTY_NAME - 1, "%s%s%d",
                PROPERTY_MCC_MNC_CDMA, ".", m_slot_id);
    }
    rfx_property_get(key, mccmnc, "0");
    if (strlen(mccmnc) > 3) {
        return true;
    }

    memset(mccmnc, 0, MAX_LENGTH_MCCMNC);
    memset(key, 0, MAX_LENGTH_PROPERTY_NAME);
    snprintf(key, MAX_LENGTH_PROPERTY_NAME - 1, "%s%d", "vendor.ril.data.cdma_mcc_mnc", m_slot_id);
    rfx_property_get(key, mccmnc, "0");
    if (strlen(mccmnc) > 3) {
        return true;
    }

    return false;
}

// Handle emergency apn as customization requirement.
// Update protocol&roaming protocol of modem's default emergency apn.
void RmcDcCommonReqHandler::updateMdDefaultEmergencyApn(RIL_MtkDataProfileInfo **dataProfilePtrs,
        int count) {
    int nonEmptyApn = -1;
    int emptyApn = -1;
    char strParam[MAX_AT_CMD_LENGTH] = {0};
    sp<RfxAtResponse> p_response;
    String8 cmd("");

    // Find emergency apns that has only emergency type.
    for (int i = 0; i < count; i++) {
        if (dataProfilePtrs[i]->supportedTypesBitmask == RIL_APN_TYPE_EMERGENCY) {
            if (strcmp("", dataProfilePtrs[i]->apn) == 0) {
                emptyApn = i;
            } else {
                nonEmptyApn = i;
            }
        }
    }

    RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] emergency apn index %d, %d",
            m_slot_id, __FUNCTION__, nonEmptyApn, emptyApn);

    // OEM never sync any emergency apn from Java Framework.
    // 1. nonEmptyApn == -1 means there is no emergency apn whose name is not empty
    // in vendor-apns-conf.xml, no need to handle.
    // 2. nonEmptyApn != -1 && emptyApn != -1 means there are 2 emergency apn, one support wifi,
    // one support cellular, will sync to modem directly, no need to handle here.
    // 3. nonEmptyApn != -1 && emptyApn == -1 means vendor-apns-xml has emergency apn whose name
    // is not empty, sync protocol and roaming protocol to modem for this case.
    if (nonEmptyApn != -1 && emptyApn == -1
            && dataProfilePtrs[nonEmptyApn]->bearerBitmask != (1 << (RADIO_TECH_IWLAN - 1))) {
        // Sync proptocol and roaming protocol to modem.
        cmd.append(String8::format("AT+EAPNSET=\"\",1,\"\",\"\""));
        p_response = atSendCommand(cmd);
        if (p_response == NULL || p_response->isAtResponseFail()) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] user/pwd error", m_slot_id, __FUNCTION__);
            return;
        }

        memset(strParam, 0, MAX_AT_CMD_LENGTH);
        snprintf(strParam, MAX_AT_CMD_LENGTH, "type=emergency;protocol=%s;"
                "roaming_protocol=%s;authtype=2;carrier_enabled=1;max_conns=0;"
                "max_conns_time=0;wait_time=0;bearer_bitmask=%d;inactive_timer=0",
                dataProfilePtrs[nonEmptyApn]->protocol,
                dataProfilePtrs[nonEmptyApn]->roamingProtocol,
                ALL_BEARER_EXCLUDE_IWLAN_BITMASK);

        cmd.clear();
        cmd.append(String8::format("AT+EAPNSET=\"\",2,\"%s\"", strParam));
        p_response = atSendCommand(cmd);
        if (p_response == NULL || p_response->isAtResponseFail()) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] parameter error", m_slot_id, __FUNCTION__);
        }
    }
}

void RmcDcCommonReqHandler::requestGetDataContextIds(const sp<RfxMclMessage>& msg) {
    RFX_UNUSED(msg);
    Vector<int> vTransIntfIdList;
    const int nPdnInfoSize = getPdnTableSize();

    vTransIntfIdList.reserve(nPdnInfoSize);

    for (int i = 0; i < nPdnInfoSize; i++) {
        if ((getPdnActiveStatus(i) == DATA_STATE_ACTIVE) &&
                (getTransIntfId(i) != INVALID_TRANS_INTF_ID) &&
                (std::find(vTransIntfIdList.begin(), vTransIntfIdList.end(),
                getTransIntfId(i)) == vTransIntfIdList.end())) {
                vTransIntfIdList.push_back(getTransIntfId(i));
        }
    }

    RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] the number of context ids = %d",
            m_slot_id, __FUNCTION__, (int)vTransIntfIdList.size());

    sendEvent(RFX_MSG_EVENT_RSP_DATA_CONTEXT_IDS,
            RfxIntsData((int*)vTransIntfIdList.array(), vTransIntfIdList.size()),
            RIL_CMD_PROXY_3, m_slot_id);
}

bool RmcDcCommonReqHandler::isHandoverRequest(int profileId, int *pTransIntfId) {
    const int nPdnInfoSize = getPdnTableSize();
    PdnInfo pdn_info;

    for (int i = 0; i < nPdnInfoSize; i++) {
        pdn_info = getPdnInfo(i);
        if (pdn_info.profileId == profileId && !pdn_info.isDedicateBearer &&
                pdn_info.active == DATA_STATE_ACTIVE) {
            *pTransIntfId = pdn_info.transIntfId;
            return true;
        }
    }
    return false;
}

void RmcDcCommonReqHandler::requestSendQualifiedNetworkTypesChanged(const sp<RfxMclMessage>& msg) {
    int *pReqData = (int *)msg->getData()->getData();
    int apnType = pReqData[0];
    int rat = pReqData[1];
    int data[3] = {0};
    sp<RfxMclMessage> urcMsg;

    data[0] = 1; // this element is 'mode' to indicate the parsing rule of the remain array elements.
    data[1] = apnType;
    data[2] = rat;

    urcMsg = RfxMclMessage::obtainUrc(RFX_MSG_URC_QUALIFIED_NETWORK_TYPES_CHANGED,
            m_slot_id, RfxIntsData(data, 3));
    responseToTelCore(urcMsg);

    RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] mode=%d, apnType=%d, rat=%d",
            m_slot_id, __FUNCTION__, data[0], data[1], data[2]);
}

void RmcDcCommonReqHandler::sendQualifiedNetworkTypesChanged(const int aid) {
    String8 cmd("");
    RfxAtLine *line = NULL;
    sp<RfxAtResponse> p_response;
    int cause = 0;
    int err = 0;
    int tempAid = 0;
    char *cmdFormat = NULL;
    int rat = 0;
    PdnInfo pdn_info;
    int data[3] = {0};
    sp<RfxMclMessage> urcMsg;

    cmd.append(String8::format("AT+EPDN=%d,\"ran\"", aid));
    p_response = atSendCommandSingleline(cmd, "+EPDN:");

    if (p_response == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get p_response!",
                m_slot_id, __FUNCTION__);
        return;
    }

    if (p_response->isAtResponseFail()) {
        if (p_response->getSuccess() > 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] weird case, can not get fail cause due to final"
                    " response indicates success", m_slot_id, __FUNCTION__);
        } else {
            cause = p_response->atGetCmeError();
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] AT+EPDN=%d,\"ran\" returns cme cause : %d",
                    m_slot_id, __FUNCTION__, aid, cause);
        }
        return;
    }

    line = p_response->getIntermediates();

    if (line == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get intermediate results!",
                m_slot_id, __FUNCTION__);
        return;
    }

    line->atTokStart(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when token start",
                m_slot_id, __FUNCTION__);
        return;
    }

    tempAid = line->atTokNextint(&err);
    if (err < 0 || (tempAid != aid)) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get aid or it's different to MD assigned",
                m_slot_id, __FUNCTION__);
        return;
    }

    cmdFormat = line->atTokNextstr(&err);
    if (err < 0 || strncmp(DC_RAN, cmdFormat, strlen(DC_RAN)) != 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing cmd",
                m_slot_id, __FUNCTION__);
        return;
    }

    rat = line->atTokNextint(&err);
    if (err < 0 || rat < 1) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get rat or it's value not greater or equal to 1",
                m_slot_id, __FUNCTION__);
        return;
    }

    pdn_info = getPdnInfo(aid);
    if (!pdn_info.isDedicateBearer && pdn_info.active == DATA_STATE_ACTIVE) {
        data[0] = 1; // this element is 'mode' to indicate the parsing rule of the remain array elements.
        int apnTypeId = RmcDcUtility::getApnTypeId(pdn_info.profileId);
        if (apnTypeId == RIL_APN_TYPE_IMS) {
            bool bAddMmsApnType = true;
            for (int i = 0; i < getPdnTableSize(); i++) {
                if (getPdnActiveStatus(i) == DATA_STATE_ACTIVE
                        && getProfileId(i) == RIL_DATA_PROFILE_VENDOR_MMS) {
                    bAddMmsApnType = false;
                    break;
                }
            }
            if (bAddMmsApnType) {
                apnTypeId |= RIL_APN_TYPE_MMS;
            }
        }
        data[1] = apnTypeId;
        data[2] = rat;
        urcMsg = RfxMclMessage::obtainUrc(RFX_MSG_URC_QUALIFIED_NETWORK_TYPES_CHANGED,
                m_slot_id, RfxIntsData(data, 3));
        responseToTelCore(urcMsg);
        RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] mode=%d, apnType=%d, rat=%d", m_slot_id,
                __FUNCTION__, data[0], data[1], data[2]);
    }
}

void RmcDcCommonReqHandler::setPdnInfoProperty() {
    Vector<int> vTransIntfIdList;
    const int nPdnInfoSize = getPdnTableSize();
    String8 keyPdnInfo("");
    String8 valuePdnInfo("");

    keyPdnInfo.append(String8::format("vendor.ril.data.pdn_info%d", m_slot_id));

    vTransIntfIdList.reserve(nPdnInfoSize);
    for (int i = 0; i < nPdnInfoSize; i++) {
        if ((getPdnActiveStatus(i) == DATA_STATE_ACTIVE) &&
                (getTransIntfId(i) != INVALID_TRANS_INTF_ID) &&
                (std::find(vTransIntfIdList.begin(), vTransIntfIdList.end(),
                getTransIntfId(i)) == vTransIntfIdList.end())) {
            PdnInfo pdn_info = getPdnInfo(i);
            if (pdn_info.profileId != RIL_DATA_PROFILE_IMS
                    && pdn_info.profileId != RIL_DATA_PROFILE_VENDOR_EMERGENCY) {
                valuePdnInfo.append(String8::format("%s_%d,",
                        RmcDcUtility::getProfileType(pdn_info.profileId),
                        getInterfaceId(getTransIntfId(i))));
                vTransIntfIdList.push_back(getTransIntfId(i));
            }
        }
    }

    rfx_property_set(keyPdnInfo, valuePdnInfo.string());

    RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] %s = %s",
            m_slot_id, __FUNCTION__, keyPdnInfo.string(), valuePdnInfo.string());
}

void RmcDcCommonReqHandler::mobileDataUsageNotify(const sp<RfxMclMessage>& msg) {
    // +EGCMD: <tx_bytes>,<tx_packets>,<rx_bytes>,<rx_packets>
    char *urc = (char*)msg->getData()->getData();
    int err = 0;
    RfxAtLine *pLine = NULL;
    sp<RfxMclMessage> urcMsg;
    unsigned int response[4] = {0};

    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] urc = %s", m_slot_id, __FUNCTION__, urc);

    pLine = new RfxAtLine(urc, NULL);

    if (pLine == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to new pLine!",
                m_slot_id, __FUNCTION__);
        goto error;
    }

    pLine->atTokStart(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when token start",
                m_slot_id, __FUNCTION__);
        goto error;
    }

    response[0] = pLine->atTokNextint(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing tx_bytes",
                m_slot_id, __FUNCTION__);
        goto error;
    }

    response[1] = pLine->atTokNextint(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing tx_packets",
                m_slot_id, __FUNCTION__);
        goto error;
    }

    response[2] = pLine->atTokNextint(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing rx_bytes",
                m_slot_id, __FUNCTION__);
        goto error;
    }

    response[3] = pLine->atTokNextint(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing rx_packets",
                m_slot_id, __FUNCTION__);
        goto error;
    }
    RFX_LOG_E(RFX_LOG_TAG, "urc parsing");
    urcMsg = RfxMclMessage::obtainUrc(RFX_MSG_URC_MOBILE_DATA_USAGE,
            m_slot_id, RfxIntsData(response, 4*sizeof(int)));
    responseToTelCore(urcMsg);

error:
    AT_LINE_FREE(pLine);
}

// Handle apns that support ims/mms/xcap/emergency, but have no bearerbitmask configured.
// e.g. There is an apn(A) support mms type and has no bearerbitmask configured,
// If there is another apn also has mms type but the BearerBitmask is 18(wifi only),
// then configure apn A with bearerbitmask cellular only, else configure apn A
// cellular and WiFi.
void RmcDcCommonReqHandler::adjustBearerBitmask(RIL_MtkDataProfileInfo **dataProfilePtrs,
        int count) {
    if (RmcDcUtility::isWifiApnExplicitlyConfigured()) {
        return;
    }

    for (int i = 0; i < count; i++) {
        if (dataProfilePtrs[i]->bearerBitmask == 0
                && isSupportWifiType(dataProfilePtrs[i]->supportedTypesBitmask)) {
            bool find = false;
            for (int j = 0; j < count; j++) {
                if (isSupportWifiType(dataProfilePtrs[i]->supportedTypesBitmask
                        & dataProfilePtrs[j]->supportedTypesBitmask)
                        && dataProfilePtrs[j]->bearerBitmask == (1 << (RADIO_TECH_IWLAN - 1))) {
                    find = true;
                    break;
                }
            }

            RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] %s find %d apn that support wifi only",
                    m_slot_id, __FUNCTION__, dataProfilePtrs[i]->apn, find);

            if (find) {
                dataProfilePtrs[i]->bearerBitmask = ALL_BEARER_EXCLUDE_IWLAN_BITMASK;
            } else {
                dataProfilePtrs[i]->bearerBitmask = ALL_BEARER_BITMASK;
            }
        }
    }
}

bool RmcDcCommonReqHandler::isSupportWifiType(int typesBitmask) {
    if (typesBitmask & RIL_APN_TYPE_IMS
            || typesBitmask & RIL_APN_TYPE_MMS
            || typesBitmask & RIL_APN_TYPE_XCAP
            || typesBitmask & RIL_APN_TYPE_EMERGENCY) {
        return true;
    }
    return false;
}

int RmcDcCommonReqHandler::getAddressLifeTime(const int aid, int *pLifeTime) {
    int rid = m_slot_id;
    int err = 0;
    RfxAtLine *pLine = NULL;
    char *cmdFormat = NULL;
    int rspAid = INVALID_AID;
    int cause = 0;
    sp<RfxAtResponse> p_response;

    // Check if AID is out of bound.
    if (!validateAid(aid)) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] aid=%d is out of bound",
                rid, __FUNCTION__, aid);
        return PDN_FAILED;
    }

    // AT+EGPCORDP=<aid>,"001E"
    // Response:
    //    +EGPCORDP:<aid>,"001E","NNNN" => "NNNN" is hex string. Example: 60s = "003C"
    p_response = atSendCommandSingleline(String8::format("AT+EGPCORDP=%d,\"001E\"", aid), "+EGPCORDP:");

    if (p_response == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get p_response!",
                rid, __FUNCTION__);
        return PDN_FAILED;
    }

    if (p_response->isAtResponseFail()) {
        if (p_response->getSuccess() > 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] weird case, can not get fail cause due to final"
                    " response indicates success", rid, __FUNCTION__);
        } else {
            cause = p_response->atGetCmeError();
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] AT+EGPCORDP for aid%d returns cme cause : %d",
                    rid, __FUNCTION__, aid, cause);
        }
        // According to DDM, when there is no response from network, any lifetime value set by UE is
        // not against 3GPP SPEC. So we first decide to set default lifetime as 12 hrs to avoid user
        // application being disturbed so early. Within this period of time, it is possible that pdu
        // is disconnected by modem default time configuration, operator requirement or even network.
        if (cause == CME_UNKNOWN) {
            *pLifeTime = MTK_DEFAULT_LIFE_TIME;
        }
        return PDN_FAILED;
    }

    pLine = p_response->getIntermediates();

    if (pLine == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get intermediate results!",
                rid, __FUNCTION__);
        return PDN_FAILED;
    }

    pLine->atTokStart(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when token start",
                rid, __FUNCTION__);
        return PDN_FAILED;
    }

    rspAid = pLine->atTokNextint(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing aid",
                rid, __FUNCTION__);
        return PDN_FAILED;
    }

    if (rspAid != aid) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] aid%d is different to MD assigned aid%d",
                rid, __FUNCTION__, aid, rspAid);
        return PDN_FAILED;
    }

    cmdFormat = pLine->atTokNextstr(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing cmd",
                rid, __FUNCTION__);
        return PDN_FAILED;
    }

    if (strncmp("001E", cmdFormat, strlen("001E")) == 0) {
        if (pLine->atTokHasmore()) {
            *pLifeTime = pLine->atTokNexthexint(&err);
            if (err < 0) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing life time",
                        rid, __FUNCTION__);
                return PDN_FAILED;
            }
        }
    } else {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] no matched command format (%s)",
                rid, __FUNCTION__, cmdFormat);
        return PDN_FAILED;
    }

    return PDN_SUCCESS;
}

bool RmcDcCommonReqHandler::isSscMode3Request(int reason, const char *apn, int *pTransIntfId) {
    const int nPdnInfoSize = getPdnTableSize();

    if (reason == ACT_REASON_SSC_MODE3) {
        for (int i = 0; i < nPdnInfoSize; i++) {
            if ((strncasecmp(getApnName(i), apn, MAX_APN_NAME_LENGTH-1) == 0) &&
                    getPdnActiveStatus(i) == DATA_STATE_LINKDOWN) {
                updatePdnActiveStatus(i, DATA_STATE_ACTIVE);
                *pTransIntfId = getTransIntfId(i);
                return true;
            }
        }
    }
    return false;
}

void RmcDcCommonReqHandler::requestStartKeepalive(const sp<RfxMclMessage>& msg) {
    String8 cmd("");
    RfxAtLine *line = NULL;
    sp<RfxAtResponse> p_response;
    int err = 0;
    sp<RfxMclMessage> responseMsg;
    int ret = 0;
    char sourceAddress[MAX_IPV6_ADDRESS_LENGTH] = {0};
    char destinationAddress[MAX_IPV6_ADDRESS_LENGTH] = {0};
    char srcPort[6] = {0}, dstPort[6] = {0};
    int type = IPV4_UDP;
    RIL_Errno status = RIL_E_INVALID_ARGUMENTS;
    RIL_KeepaliveStatus keepaliveStatus = {0, KEEPALIVE_INACTIVE};

    RIL_KeepaliveRequest *pReq = (RIL_KeepaliveRequest*)msg->getData()->getData();

    if (pReq->type == NATT_IPV4) {
        snprintf(sourceAddress, MAX_IPV4_ADDRESS_LENGTH, "%d.%d.%d.%d",
                pReq->sourceAddress[0] & 0xff, pReq->sourceAddress[1] & 0xff,
                pReq->sourceAddress[2] & 0xff, pReq->sourceAddress[3] & 0xff);
        snprintf(destinationAddress, MAX_IPV4_ADDRESS_LENGTH, "%d.%d.%d.%d",
                pReq->destinationAddress[0] & 0xff, pReq->destinationAddress[1] & 0xff,
                pReq->destinationAddress[2] & 0xff, pReq->destinationAddress[3] & 0xff);
    } else {
        snprintf(sourceAddress, MAX_IPV6_ADDRESS_LENGTH, "%02X%02X:%02X%02X:"
                "%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X",
                pReq->sourceAddress[0] & 0xff, pReq->sourceAddress[1] & 0xff,
                pReq->sourceAddress[2] & 0xff, pReq->sourceAddress[3] & 0xff,
                pReq->sourceAddress[4] & 0xff, pReq->sourceAddress[5] & 0xff,
                pReq->sourceAddress[6] & 0xff, pReq->sourceAddress[7] & 0xff,
                pReq->sourceAddress[8] & 0xff, pReq->sourceAddress[9] & 0xff,
                pReq->sourceAddress[10] & 0xff, pReq->sourceAddress[11] & 0xff,
                pReq->sourceAddress[12] & 0xff, pReq->sourceAddress[13] & 0xff,
                pReq->sourceAddress[14] & 0xff, pReq->sourceAddress[15] & 0xff);
        snprintf(destinationAddress, MAX_IPV6_ADDRESS_LENGTH, "%02X%02X:%02X%02X:"
                "%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X",
                pReq->destinationAddress[0] & 0xff, pReq->destinationAddress[1] & 0xff,
                pReq->destinationAddress[2] & 0xff, pReq->destinationAddress[3] & 0xff,
                pReq->destinationAddress[4] & 0xff, pReq->destinationAddress[5] & 0xff,
                pReq->destinationAddress[6] & 0xff, pReq->destinationAddress[7] & 0xff,
                pReq->destinationAddress[8] & 0xff, pReq->destinationAddress[9] & 0xff,
                pReq->destinationAddress[10] & 0xff, pReq->destinationAddress[11] & 0xff,
                pReq->destinationAddress[12] & 0xff, pReq->destinationAddress[13] & 0xff,
                pReq->destinationAddress[14] & 0xff, pReq->destinationAddress[15] & 0xff);
        type = IPV6_UDP;
    }

    snprintf(srcPort, sizeof(srcPort), "%d", pReq->sourcePort);
    snprintf(dstPort, sizeof(dstPort), "%d", pReq->destinationPort);

    cmd.append(String8::format("AT+EKALIVE=1,%d,%d,\"%s\",\"%s\",\"%s\",\"%s\",%d",
            type, getInterfaceId(pReq->cid), sourceAddress, destinationAddress, srcPort, dstPort,
            pReq->maxKeepaliveIntervalMillis/1000));

    p_response = atSendCommandSingleline(cmd, "+EKALIVE:");

    if (p_response == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get p_response!",
                m_slot_id, __FUNCTION__);
        goto finish;
    }

    if (p_response->isAtResponseFail()) {
        if (p_response->getSuccess() > 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] weird case, can not get fail cause due to final"
                    " response indicates success", m_slot_id, __FUNCTION__);
        } else {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] AT+EKALIVE=1,x returns cme cause : %d",
                    m_slot_id, __FUNCTION__, p_response->atGetCmeError());
        }
        goto finish;
    }

    line = p_response->getIntermediates();

    if (line == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get intermediate results!",
                m_slot_id, __FUNCTION__);
        goto finish;
    }

    line->atTokStart(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when token start",
                m_slot_id, __FUNCTION__);
        goto finish;
    }

    ret = line->atTokNextint(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get conn_id", m_slot_id, __FUNCTION__);
        goto finish;
    }
    keepaliveStatus.sessionHandle = (uint32_t)ret;

    ret = line->atTokNextint(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get md_status", m_slot_id, __FUNCTION__);
        goto finish;
    }
    keepaliveStatus.code = (RIL_KeepaliveStatusCode)ret;

    status = RIL_E_SUCCESS;

finish:
    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] sessionHandle=%d, code=%d", m_slot_id, __FUNCTION__,
            keepaliveStatus.sessionHandle, keepaliveStatus.code);
    responseMsg = RfxMclMessage::obtainResponse(status,
            RfxKeepaliveStatusData(&keepaliveStatus, sizeof(keepaliveStatus)), msg);
    responseToTelCore(responseMsg);
}

void RmcDcCommonReqHandler::requestStopKeepalive(const sp<RfxMclMessage>& msg) {
    String8 cmd("");
    RfxAtLine *line = NULL;
    sp<RfxAtResponse> p_response;
    int err = 0;
    sp<RfxMclMessage> responseMsg;

    int* param = (int*)(msg->getData()->getData());
    int conn_id = param[0];

    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] conn_id=%d", m_slot_id, __FUNCTION__, conn_id);

    cmd.append(String8::format("AT+EKALIVE=0,%d", conn_id));

    p_response = atSendCommand(cmd);

    if (p_response == NULL || p_response->isAtResponseFail()) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] AT+EKALIVE=0,x returns ERROR",
                m_slot_id, __FUNCTION__);
        responseMsg = RfxMclMessage::obtainResponse(RIL_E_INVALID_ARGUMENTS, RfxVoidData(), msg);
    } else {
        responseMsg = RfxMclMessage::obtainResponse(RIL_E_SUCCESS, RfxVoidData(), msg);
    }
    responseToTelCore(responseMsg);
}

void RmcDcCommonReqHandler::updateKeepaliveStatus(const sp<RfxMclMessage>& msg) {
    //+EKALIVE: <conn_id>,<md_status>
    char *urc = (char*)msg->getData()->getData();
    int err = 0;
    RfxAtLine *pLine = NULL;
    int ret = 0;
    sp<RfxMclMessage> urcMsg;
    RIL_KeepaliveStatus keepaliveStatus = {0, KEEPALIVE_INACTIVE};

    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] urc = %s", m_slot_id, __FUNCTION__, urc);

    pLine = new RfxAtLine(urc, NULL);

    if (pLine == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to new pLine!",
                m_slot_id, __FUNCTION__);
        goto error;
    }

    pLine->atTokStart(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when token start",
                m_slot_id, __FUNCTION__);
        goto error;
    }

    ret = pLine->atTokNextint(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing conn_id",
                m_slot_id, __FUNCTION__);
        goto error;
    }
    keepaliveStatus.sessionHandle = (uint32_t)ret;

    ret = pLine->atTokNextint(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing md_satus",
                m_slot_id, __FUNCTION__);
        goto error;
    }
    keepaliveStatus.code = (RIL_KeepaliveStatusCode)ret;

    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] sessionHandle=%d, code=%d", m_slot_id, __FUNCTION__,
            keepaliveStatus.sessionHandle, keepaliveStatus.code);
    urcMsg = RfxMclMessage::obtainUrc(RFX_MSG_URC_KEEPALIVE_STATUS, m_slot_id,
            RfxKeepaliveStatusData(&keepaliveStatus, sizeof(keepaliveStatus)));
    responseToTelCore(urcMsg);

error:
    AT_LINE_FREE(pLine);
}

void RmcDcCommonReqHandler::onNwLimitNotify(const sp<RfxMclMessage>& msg) {
    //+ENWLIMIT: <state>
    char *urc = (char*)msg->getData()->getData();
    int err = 0;
    RfxAtLine *pLine = NULL;
    sp<RfxMclMessage> urcMsg;
    int response[1] = {0};

    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] urc = %s", m_slot_id, __FUNCTION__, urc);

    pLine = new RfxAtLine(urc, NULL);

    if (pLine == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to new pLine!",
                m_slot_id, __FUNCTION__);
        goto error;
    }

    pLine->atTokStart(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when token start",
                m_slot_id, __FUNCTION__);
        goto error;
    }

    response[0] = pLine->atTokNextint(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing aid",
                m_slot_id, __FUNCTION__);
        goto error;
    }

    urcMsg = RfxMclMessage::obtainUrc(RFX_MSG_URC_NW_LIMIT, m_slot_id, RfxIntsData(response, 1));
    responseToTelCore(urcMsg);

error:
    AT_LINE_FREE(pLine);
}
