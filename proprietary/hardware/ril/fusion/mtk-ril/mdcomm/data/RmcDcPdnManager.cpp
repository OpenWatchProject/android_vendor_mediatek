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

#include <string.h>

#include "RfxStatusDefs.h"
#include "RmcDcUtility.h"
#include "RmcDcPdnManager.h"

#define RFX_LOG_TAG "RmcDcPdnManager"

/*
 * RmcDcPdnManager class description:
 * The class is defined to manage the PDN table information in rild, it is slot based which
 * means each slot has such a PdnManager instance. Other class should get latest PDN info
 * from it and update the corresponding PdnInfo if it's changed.
 *
 * Not consider concurrency access currently because RILD only use one data channel and all
 * operations are executed on one thread. Should consider it if we change the thread and data
 * access model.
 */

RmcDcPdnManager::RmcDcPdnManager(int slot_id, int channel_id) :
        RfxBaseHandler(slot_id, channel_id), mPdnTableSize(0),
        m_pPdnInfo(NULL), mNumOfConnectedPdn(0) {
    initPdnTable();
    initAtCmds();
}

RmcDcPdnManager::~RmcDcPdnManager() {
    if (m_pPdnInfo != NULL) {
        free(m_pPdnInfo);
        m_pPdnInfo = NULL;
    }
    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s]: destructor.", m_slot_id, __FUNCTION__);
}

void RmcDcPdnManager::initPdnTable() {
    mPdnTableSize = getModemSupportPdnNumber();
    if (m_pPdnInfo == NULL) {
        m_pPdnInfo = (PdnInfo*) calloc(mPdnTableSize, sizeof(PdnInfo));
        RFX_ASSERT(m_pPdnInfo != NULL);
        clearAllPdnInfo();
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s]: pdnTableSize = %d, m_pPdnInfo = %p", m_slot_id,
                __FUNCTION__, mPdnTableSize, m_pPdnInfo);
    }
}

void RmcDcPdnManager::initAtCmds() {
    sp<RfxAtResponse> p_response;

    // enable ATTACH PDN URC
    p_response = atSendCommand("AT+EIAREG=1");
    if (p_response == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get IAREG=1 response", m_slot_id, __FUNCTION__);
    }
    p_response = atSendCommand("AT+ECCAUSE=1");
    if (p_response == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get ECCAUSE=1 response", m_slot_id, __FUNCTION__);
    }
}

int RmcDcPdnManager::getPdnTableSize() {
    return mPdnTableSize;
}

// Get PDN info, it is a copy of PDN info in the table.
PdnInfo RmcDcPdnManager::getPdnInfo(int aid) {
    RFX_ASSERT(validateAid(aid));
    if (DEBUG_MORE_INFO) {
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s]: aid = %d, pdnInfo = %s", m_slot_id, __FUNCTION__,
                aid, pdnInfoToString(&m_pPdnInfo[aid]).string());
    }
    return m_pPdnInfo[aid];
}

void RmcDcPdnManager::setPdnInfo(int aid, PdnInfo* pdnInfo) {
    // Use memcpy instead of assign value by attributes for better performance.
    // NOTES: No pointer member in PdnInfo, couldn't use memcpy if any pointer member in PdnInfo.
    RFX_ASSERT(pdnInfo != NULL);
    int previousPdnActiveStatus = m_pPdnInfo[aid].active;
    int newPdnActiveStatus = pdnInfo->active;
    memcpy(&m_pPdnInfo[aid], pdnInfo, sizeof(PdnInfo));

    if (previousPdnActiveStatus != newPdnActiveStatus) {
        updateAndNotifyPdnConnectStatusChanged();
    }
}

void RmcDcPdnManager::clearPdnInfo(int aid) {
    RFX_ASSERT(validateAid(aid));
    clearPdnInfo(&m_pPdnInfo[aid]);

    updateAndNotifyPdnConnectStatusChanged();
}

void RmcDcPdnManager::clearAllPdnInfo() {
    if (DEBUG_MORE_INFO) {
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s].", m_slot_id, __FUNCTION__);
        dumpAllPdnInfo();
    }

    for (int i = 0; i < mPdnTableSize; i++) {
        clearPdnInfo(&m_pPdnInfo[i]);
    }
    updateAndNotifyPdnConnectStatusChanged();
}

void RmcDcPdnManager::clearPdnInfo(PdnInfo* info) {
    memset(info, 0, sizeof(PdnInfo));
    info->transIntfId = INVALID_TRANS_INTF_ID;
    info->primaryAid = INVALID_AID;
    info->aid = INVALID_AID;
    info->bearerId = INVALID_AID;
    info->ddcId = INVALID_AID;
    info->mtu = 0;  // means not be specified
    info->reason = NO_CAUSE;
    info->deactReason = NO_REASON;
    info->rat = 1; // 1: cellular
    info->pdnType = INVALID_PDN_TYPE;
    info->profileId = -1;
    info->sscMode = SSC_UNKNOWN;
}

void RmcDcPdnManager::updatePdnActiveStatus(int aid, int pdnActiveStatus) {
    RFX_ASSERT(validateAid(aid));
    int prevActiveStatus = m_pPdnInfo[aid].active;
    m_pPdnInfo[aid].active = pdnActiveStatus;

    // Notify PDN active status changed, compare with previous status to reduce unnecessary call.
    if (prevActiveStatus != pdnActiveStatus) {
        updateAndNotifyPdnConnectStatusChanged();
    }
}

void RmcDcPdnManager::updateIpAddress(int aid, const char* ipv4Addr, int index,
        const char* ipv6Addr) {
    RFX_ASSERT(validateAid(aid));
    if (ipv4Addr != NULL) {
        snprintf(m_pPdnInfo[aid].addressV4, MAX_IPV4_ADDRESS_LENGTH, "%s", ipv4Addr);
    }
    RFX_ASSERT(index >= 0 && index < MAX_NUM_IPV6_ADDRESS_NUMBER);
    if (ipv6Addr != NULL) {
        snprintf(m_pPdnInfo[aid].addressV6[index], MAX_IPV6_ADDRESS_LENGTH, "%s", ipv6Addr);
    }
}

void RmcDcPdnManager::updateDns(int aid, int v4DnsNumber, int v6DnsNumber,
        const char** v4Dns, const char** v6Dns) {
    RFX_ASSERT(validateAid(aid));
    for (int i = 0; i < v4DnsNumber; i++) {
        snprintf(m_pPdnInfo[aid].dnsV4[i], MAX_IPV4_ADDRESS_LENGTH, "%s", *(v4Dns + i));
    }
    for (int i = 0; i < v6DnsNumber; i++) {
        snprintf(m_pPdnInfo[aid].dnsV6[i], MAX_IPV6_ADDRESS_LENGTH, "%s", *(v6Dns + i));
    }
}

// Update status key to notify PDN status, request by AGPS.
void RmcDcPdnManager::updateAndNotifyPdnConnectStatusChanged() {
    int numOfConnectedPdn = 0;
    for (int i = 0; i < mPdnTableSize; i++) {
        if (m_pPdnInfo[i].active == DATA_STATE_ACTIVE) {
            numOfConnectedPdn++;
        }
    }

    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s]: mNumOfConnectedPdn = %d, numOfConnectedPdn = %d.",
            m_slot_id, __FUNCTION__, mNumOfConnectedPdn, numOfConnectedPdn);
    if (mNumOfConnectedPdn != numOfConnectedPdn) {
        if (numOfConnectedPdn > 0) {
            getMclStatusManager()->setIntValue(RFX_STATUS_KEY_DATA_CONNECTION,
                    DATA_STATE_CONNECTED);
        } else {
            getMclStatusManager()->setIntValue(RFX_STATUS_KEY_DATA_CONNECTION,
                    DATA_STATE_DISCONNECTED);
        }
    }
    mNumOfConnectedPdn = numOfConnectedPdn;
}

void RmcDcPdnManager::dumpAllPdnInfo() {
    for (int i = 0; i < mPdnTableSize; i++) {
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s]: %s", m_slot_id,
                __FUNCTION__, pdnInfoToString(&m_pPdnInfo[i]).string());
    }
}

String8 RmcDcPdnManager::pdnInfoToString(PdnInfo* pdnInfo) {
    String8 cmd("");

    cmd.append(String8::format("PdnInfo{transIntfId=%d, primaryAid=%d, aid=%d, apnName=%s, "
            "active=%d, addrV4=%s, ",
            pdnInfo->transIntfId,
            pdnInfo->primaryAid,
            pdnInfo->aid,
            pdnInfo->apn,
            pdnInfo->active,
            pdnInfo->addressV4));

    cmd.append(String8::format("addrV6="));
    for (int i = 0; i < MAX_NUM_IPV6_ADDRESS_NUMBER; i++) {
        if (i != 0) {
            cmd.append(String8::format("%s", ", "));
        }
        cmd.append(String8::format("%s", pdnInfo->addressV6[i]));
    }

    cmd.append(String8::format(", mtu=%d, rat=%d, reason=%d, deactReason=%d, pdnType=%d, profileId=%d, sscMode=%d}",
            pdnInfo->mtu,
            pdnInfo->rat,
            pdnInfo->reason,
            pdnInfo->deactReason,
            pdnInfo->pdnType,
            pdnInfo->profileId,
            pdnInfo->sscMode));

    return cmd;
}

// Get modem support PDN number, MD response the AID range by AT+CGDCONT=?.
// It is used to decide the size of PDN table.
int RmcDcPdnManager::getModemSupportPdnNumber() {
    sp<RfxAtResponse> p_response;
    int maxPdnSupportNumer = 0;
    int err = 0;
    RfxAtLine *p_cur = NULL;
    int rid = m_slot_id;

    p_response = atSendCommandMultiline("AT+CGDCONT=?", "+CGDCONT:");
    if (p_response == NULL || p_response->isAtResponseFail()) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] AT+CGDCONT=? response error", rid, __FUNCTION__);
        goto error;
    } else {
        //  The response would be the following liness
        //  +CGDCONT: (0-10),"IP",,,(0),(0),(0-1),...
        //  +CGDCONT: (0-10),"IPV6",,,(0),(0),(0-1),...
        //  +CGDCONT: (0-10),"IPV4V6",,,(0),(0),(0-1),...
        for (p_cur = p_response->getIntermediates(); p_cur != NULL; p_cur =
                p_cur->getNext()) {
            p_cur->atTokStart(&err);
            if (err < 0) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when token start",
                        rid, __FUNCTION__);
                continue;
            }

            char* range;
            char* tok;
            char* restOfString;
            int value[2] = { 0 };
            int count = 0;
            range = p_cur->atTokNextstr(&err);
            if (err < 0) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing range of MD support pdn",
                        rid, __FUNCTION__);
                continue;
            }

            if (range != NULL) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] the MD support pdn range:%s",
                        rid, __FUNCTION__, range);
            }

            tok = strtok_r(range, "(-)", &restOfString);
            while (tok != NULL) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] the %d's token is %s",
                        rid, __FUNCTION__, count + 1, tok);
                value[count] = atoi(tok);
                count++;
                tok = strtok_r(restOfString, "(-)", &restOfString);
            }

            if (DEBUG_MORE_INFO) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] Support range is (%d - %d)",
                        rid, __FUNCTION__, value[0], value[1]);
            }

            maxPdnSupportNumer = value[1] - value[0] + 1;
        }
    }

    RFX_LOG_V(RFX_LOG_TAG, "[%d][%s] maxPdnSupportNumer = %d",
            rid, __FUNCTION__, maxPdnSupportNumer);
    if (0 >= maxPdnSupportNumer) {
        goto error;
    }
    return maxPdnSupportNumer;

error:
    RFX_LOG_E(RFX_LOG_TAG, "RIL%d %s error: return default number  = %d.",
            m_slot_id, __FUNCTION__, DEFAULT_PDN_TABLE_SIZE);
    return DEFAULT_PDN_TABLE_SIZE;
}
