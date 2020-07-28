/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include <sys/system_properties.h>
#include <unistd.h>

#include <KpiSubscriber.h>
#include "KpiUtils.h"
#include "dmc_utils.h"
#include <vector>

#undef TAG
#define TAG "MDM-Subscriber"

#define PROP_MDM_SVC "init.svc.md_monitor"

using namespace libmdmonitor;

#define DMC_TRAP_RECEIVER_HOST_NAME "DMC_Trap_Receiver"

#define SUBSCRIBE_MDM_TRAP_ASYNC 1

/* ============================ Public Interfaces of KpiSubscriber ============================ */
KpiSubscriber::KpiSubscriber(MdmEventsSubscriber *subscriber):
        mEnabled(false),
        m_pCmdClient(NULL),
        m_sessionId(0),
        m_pTrapReceiver(NULL) {
    m_pSubscriber = subscriber;
}

KpiSubscriber::~KpiSubscriber()
{
    DMC_LOGD(TAG, "destruction()");
    disableSubscriber();
}

bool KpiSubscriber::enableSubscriber() {
    if (isEnabled()) {
        DMC_LOGE(TAG, "skip enableSubscriber() because already enabled!");
        return false;
    }

    DMC_LOGD(TAG, "enableSubscriber()");
    MONITOR_CMD_RESP resp = MONITOR_CMD_RESP_INTERNAL_ERROR;
    bool ret = false;

    do {
        // Start MDM service and then load Translator, otherwise, MDM service won't load EM/ICD layout file.
        if (!StartMdmServiceSync()) {
            DMC_LOGE(TAG, "MDM service not ready!");
            // TODO: Add back when MDM service init rc ready.
            // break;
        }
        m_pCmdClient = new MonitorCmdProxy<JsonCmdEncoder>(MONITOR_COMMAND_SERVICE_ABSTRACT_NAME);
        m_sessionId = m_pCmdClient->CreateSession();
        if (m_sessionId == SID_ERR) {
            DMC_LOGE(TAG, "Get invalid sessionId");
            break;
        }
        m_pTrapReceiver = new MonitorTrapReceiver(m_sessionId, DMC_TRAP_RECEIVER_HOST_NAME);
        m_pTrapReceiver->SetTrapHandler(TrapCB, this);
        resp = m_pCmdClient->SetTrapReceiver(m_sessionId, m_sessionId, DMC_TRAP_RECEIVER_HOST_NAME);
        if (resp != MONITOR_CMD_RESP_SUCCESS) {
            DMC_LOGE(TAG, "SetTrapReceiver() failed: %d", resp);
            break;
        }
        resp = m_pCmdClient->EnableTrap(m_sessionId, m_sessionId);
        DMC_LOGD(TAG, "EnableTrap() with sessionId = %" PRIu64", resp = %d", m_sessionId, resp);
        // Workaroud: If MD ICD is not enabled, we will always get 3s timeout to enable ICD.
        if (resp != MONITOR_CMD_RESP_SUCCESS && resp != MONITOR_CMD_RESP_OP_TIMEOUT) {
            DMC_LOGE(TAG, "EnableTrap() failed! with sessionId = %" PRIu64"", m_sessionId);
            break;
        }
        if (!SubscribeMsgToMdm(true, TRAP_TYPE_PSTIME, 0)) {
            DMC_LOGE(TAG, "Subscribe PSTIME failed with sessionId = %" PRIu64"", m_sessionId);
            break;
        }
        // Finally, all procedure success!
        ret = true;
        mEnabled = true;
        DMC_LOGD(TAG, "enableSubscriber() success, sessionId = %" PRIu64"", m_sessionId);
    } while(false);

    if (!ret) {
        if (NULL != m_pTrapReceiver) {
            delete m_pTrapReceiver;
            m_pTrapReceiver = NULL;
        }
        if (NULL != m_pCmdClient) {
            delete m_pCmdClient;
            m_pCmdClient = NULL;
        }
        m_sessionId = SID_ERR;
    }
    return ret;
}

void KpiSubscriber::disableSubscriber() {
    if (!isEnabled()) {
        DMC_LOGE(TAG, "skip disableSubscriber() because not enabled!");
        return;
    }
    // Unsubscribe all subscribed MDM traps
    SubscribeMsgToMdm(false, TRAP_TYPE_PSTIME, 0);
    std::vector<MSGID> subscribedMessages;
    m_pSubscriber->getSubscribedMsgs(subscribedMessages);
    for (unsigned int i = 0; i < subscribedMessages.size(); i++) {
        MSGID msgId = (MSGID)subscribedMessages[i];
        KPI_OBJ_TYPE kpiObj = m_pSubscriber->getObjTypeByMsgId(msgId);
        const TrapTable *pTrapTable = m_pSubscriber->getTrapTableByKpi(kpiObj);
        if (pTrapTable != NULL) {
            const TrapTable &trapTable = *pTrapTable;
            for (unsigned int j = 0; j < trapTable.Size(); j++) {
                const TrapInfo info = trapTable[j];
                TRAP_TYPE trapType = KpiUtils::getMdmTrapTypeBySource(info.m_sourceType);
                if (info.m_msgId == msgId) {
                    SubscribeMsgToMdm(false, trapType, msgId);
                }
            }
        }
    }

    if (m_sessionId != SID_ERR) {
        try {
            m_pCmdClient->DisableTrap(m_sessionId, m_sessionId);
        } catch (const std::runtime_error&) {
            DMC_LOGE(TAG, "DisableTrap() failed!");
        }
        m_pCmdClient->CloseSession(m_sessionId, m_sessionId);
    }

    if (NULL != m_pTrapReceiver) {
        delete m_pTrapReceiver;
        m_pTrapReceiver = NULL;
    }
    if (NULL != m_pCmdClient) {
        delete m_pCmdClient;
        m_pCmdClient = NULL;
    }
    DMC_LOGD(TAG, "disableSubscriber() success, sessionId = [%" PRIu64"]", (uint64_t)m_sessionId);
    m_sessionId = SID_ERR;
    mEnabled = false;
}

bool KpiSubscriber::isEnabled() {
    return mEnabled;
}

bool KpiSubscriber::StartMdmServiceSync() {
    int retry = -1;
    for (retry = -1; retry < 3; retry++) {
        if (!IsMdmServiceRunning()) {
            property_set("ctl.start", "md_monitor");
            DMC_LOGD(TAG, "Start MDM service");
            sleep(1);
        } else {
            DMC_LOGD(TAG, "MDM service is running");
            return true;
        }
    }
    return false;
}

// Shall never call this, there may exist other MDM user
void KpiSubscriber::StopMdmService() {
    if (IsMdmServiceRunning()) {
        property_set("ctl.stop", "md_monitor");
        DMC_LOGD(TAG, "Stop MDM service");
    }
}

// TODO: Check HIDL client
bool KpiSubscriber::IsMdmServiceRunning() {
    bool running = false;
    char buffer[PROP_VALUE_MAX] = {'\0'};
    property_get("init.svc.md_monitor", buffer, "stopped");

    if (NULL == strstr(buffer, "running")) {
        running = false;
    } else {
        running = true;
    }
    return running;
}

// ============================ Private Interfaces of KpiSubscriber ============================

void KpiSubscriber::TrapCB(
        void *param, uint32_t timestamp, TRAP_TYPE trapType,
        size_t len, const uint8_t *data, size_t discard_count) {
    KpiSubscriber *me = (KpiSubscriber *)param;
    if (NULL != me) {
        if (!me->isEnabled()) {
            DMC_LOGE(TAG, "Skip TrapCB() with disabled state");
            return;
        }

        // Workaround: Make data buffer to 4 bytes alignment.
        uint8_t *temp_buf = new uint8_t[len];
        memcpy(temp_buf, data, len);
        me->m_pSubscriber->TrapCallback(trapType, len, temp_buf, discard_count);
        delete [] temp_buf;
    }
}

bool KpiSubscriber::SubscriptionHandler(
        bool bSubscribe,
        KPI_OBJ_TYPE kpiObjType,
        KPI_SOURCE_TYPE source,
        MSGID msgId) {
    TRAP_TYPE trapType = KpiUtils::getMdmTrapTypeBySource(source);
    bool bExecuted = false;

    if (TRAP_TYPE_UNDEFINED == trapType) {
        DMC_LOGE(TAG, "Unexpected source = %s", KpiUtils::kpiSource2String(source));
        return false;
    }

    if (bSubscribe) { // Subscribe
        if (m_pSubscriber->getSubscribedMsgSize() == 0) {
            // Subscribe first message.
        }

        if (!m_pSubscriber->isMsgSubscribed(msgId)) {
            DMC_LOGD(TAG, "Subscribe trapType = %d, source = %s, msgId = %" PRIu64"",
                    kpiObjType, KpiUtils::kpiSource2String(source), msgId);
            SubscribeMsgToMdm(bSubscribe, trapType, msgId);
            bExecuted = true;
        }
    } else { // Unsubscribe
        if (m_pSubscriber->isMsgSubscribed(msgId)) {
            // The last listener that needs trapType/msgId. We should send unsubscribe the message.
            DMC_LOGD(TAG, "Unsubscribe trapType = %d, source = %s, msgId = %" PRIu64"",
                    kpiObjType, KpiUtils::kpiSource2String(source), msgId);
            SubscribeMsgToMdm(bSubscribe, trapType, msgId);
            bExecuted = true;
            // Cancel PSTIME message if no messages subscribed
            if (m_pSubscriber->getSubscribedMsgSize() == 1) {
                SubscribeMsgToMdm(false, TRAP_TYPE_PSTIME, 0);
            }
        }
    }
    return bExecuted;
}

bool KpiSubscriber::SubscribeHandlerBatch(
        bool bSubscribe, KPI_SOURCE_TYPE source, SubscriptionBatchMdmRequest *pRequests) {
    bool ret = false;
    KPI_MSGID *pMsgIds = NULL;
    std::vector<SubscriptionRequest> requests;
    if (pRequests->getRequestList(source, requests)) {
        TRAP_TYPE trapType = KpiUtils::getMdmTrapTypeBySource(source);
        unsigned int size = requests.size();
        if (size > 0) {
            if (pRequests->getMsgIds(source, &pMsgIds)) {
                if (size == 1) {
                    ret = SubscribeMsgToMdm(bSubscribe, trapType, (MSGID)pMsgIds[0]);
                } else {
                    ret = SubscribeMsgToMdmBatch(bSubscribe, trapType, (MSGID *)pMsgIds, size);
                }
            }
        }
        if (pMsgIds != NULL) {
            free(pMsgIds);
        }
    }
    return ret;
}

bool KpiSubscriber::SubscribeMsgToMdm(bool bSubscribe, TRAP_TYPE trapType, MSGID msgId) {
    MONITOR_CMD_RESP resp;
    if (m_pCmdClient == NULL) {
        DMC_LOGE(TAG, "Failed to subscribe/unsubscribe trap with NULL m_pCmdClient");
        return false;
    }

    bool bAsync = (SUBSCRIBE_MDM_TRAP_ASYNC)? true : false;

    if (bSubscribe) {
        if (MONITOR_CMD_RESP_SUCCESS != (resp = m_pCmdClient->SubscribeTrap(
                m_sessionId, m_sessionId, trapType, msgId, bAsync))) {
            DMC_LOGE(TAG, "Failed to subscribe trap [%d] message [%" PRIu64"] by sessionId [%" PRIu64"]. resp = [%d]",
                    trapType, (uint64_t)msgId, (uint64_t)m_sessionId, resp);
            return false;
        }
        DMC_LOGD(TAG, "Subscribe trap [%d] message [%" PRIu64"] by sessionId [%" PRIu64"] successfully.",
                trapType, (uint64_t)msgId, (uint64_t)m_sessionId);
    } else {
        if (MONITOR_CMD_RESP_SUCCESS != (resp = m_pCmdClient->UnsubscribeTrap(
                m_sessionId, m_sessionId, trapType, msgId, bAsync))) {
            DMC_LOGE(TAG, "Failed to unsubscribe trap [%d] message [%" PRIu64"] by sessionId [%" PRIu64"]. resp = [%d]",
                    trapType, (uint64_t)msgId, (uint64_t)m_sessionId, resp);
            return false;
        }
        DMC_LOGD(TAG, "Unsubscribe trap [%d] message [%" PRIu64"] by sessionId [%" PRIu64"] successfully.",
                trapType, (uint64_t)msgId, (uint64_t)m_sessionId);
    }
    return true;
}

bool KpiSubscriber::SubscribeMsgToMdmBatch(bool bSubscribe, TRAP_TYPE trapType, MSGID *msgId, uint32_t msgSize) {
    MONITOR_CMD_RESP resp;
    if (m_pCmdClient == NULL) {
        DMC_LOGE(TAG, "Failed to batch subscribe/unsubscribe trap with NULL m_pCmdClient");
        return false;
    }
    if (bSubscribe) {
        if (MONITOR_CMD_RESP_SUCCESS != (resp = m_pCmdClient->SubscribeMultiTrap(m_sessionId, m_sessionId, trapType, msgId, msgSize))) {
            DMC_LOGE(TAG, "Failed to subscribe multi-trap for type = %d with sessionId [%" PRIu64"]. resp = [%d]",
                    trapType, (uint64_t)m_sessionId, resp);
            return false;
        }
        DMC_LOGD(TAG, "Subscribe multi-trap for type = %d with sessionId [%" PRIu64"] successfully.",
                trapType, (uint64_t)m_sessionId);
    } else {
        if (MONITOR_CMD_RESP_SUCCESS != (resp = m_pCmdClient->UnsubscribeMultiTrap(m_sessionId, m_sessionId, trapType, msgId, msgSize))) {
            DMC_LOGE(TAG, "Failed to unsubscribe multi-trap for type = %d with sessionId [%" PRIu64"]. resp = [%d]",
                    trapType, (uint64_t)m_sessionId, resp);
            return false;
        }
        DMC_LOGD(TAG, "Unsubscribe multi-trap for type = %d with sessionId [%" PRIu64"] successfully.",
                trapType, (uint64_t)m_sessionId);
    }
    return true;

}



