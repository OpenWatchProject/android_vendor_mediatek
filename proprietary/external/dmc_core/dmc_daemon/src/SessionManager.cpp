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
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#include "SessionManager.h"

#include "dmc_utils.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>


#undef TAG
#define TAG "DMC-SessionManager"


//============= SessionEntry ==================
// The SessionId starts from 1.
SessionId SessionEntry::sSidCounter = INVALID_SESSION_ID;

SessionEntry::SessionEntry(const SessionInfo *pInfo) {
    mSessionId = incSessionId();
    mSessionInfo.decryptPacket = pInfo->decryptPacket;
    strncpy(mSessionInfo.identity, pInfo->identity, MAX_IDENTITY_LEN);
    mSessionInfo.identity[MAX_IDENTITY_LEN] = '\0';
    strncpy(mSessionInfo.targetVersion, pInfo->targetVersion, MAX_VERSION_LEN);
    mSessionInfo.targetVersion[MAX_VERSION_LEN] = '\0';
}

SessionEntry::~SessionEntry() {
}

// Return true if sessionId & identity is the same.
bool SessionEntry::equal(const SessionEntry *entry) {
    if (mSessionId != entry->getSessionId()) {
        return false;
    }
    if (strcmp(entry->getSessionInfo()->identity, mSessionInfo.identity) != 0) {
        return false;
    }
    return true;
}

SessionId SessionEntry::incSessionId() {
    sSidCounter++;
    return sSidCounter;
}

void SessionEntry::resetSessionId() {
    sSidCounter = INVALID_SESSION_ID;
}

//============= SessionManager ==================
SessionManager::SessionManager() {
    DMC_LOGD(TAG, "constructor()");
}

SessionManager::~SessionManager() {
    DMC_LOGD(TAG, "destructor()");
    reset();
}

DMC_RESULT_CODE SessionManager::getCurrentSessionEntry(SessionEntry **entry) {
    // Return first entry
    if (getSessionNum() > 0) {
        *entry = (SessionEntry *)&mSessionList[0];
        DMC_LOGD(TAG, "getCurrentSessionEntry() identity = %s, SID = %d",
                (*entry)->getSessionInfo()->identity, (*entry)->getSessionId());
        return DMC_RESULT_SUCCESS;
    } else {
        DMC_LOGW(TAG, "No sesssion entry found!");
        return DMC_RESULT_FAIL;
    }
}

unsigned int SessionManager::getSessionNum() {
    return mSessionList.size();
}

SessionId SessionManager::createSessionEntry(const SessionInfo *info) {
    if (getSessionNum() >= MAX_SESSION_NUM) {
        DMC_LOGW(TAG, "We only support %d DMC session now!", MAX_SESSION_NUM);
        return INVALID_SESSION_ID;
    }

    SessionEntry *pEntry = NULL;
    if (!getSessionEntry(info->identity, &pEntry)) {
        SessionEntry entry(info);
        mSessionList.push_back(entry);
        DMC_LOGD(TAG, "createSessionEntry() SID = %d, identity = %s",
                entry.getSessionId(), entry.getSessionInfo()->identity);
        return entry.getSessionId();
    }
    return INVALID_SESSION_ID;
}

bool SessionManager::removeSessionEntry(SessionEntry &entryRemoved) {
    bool bRemoved = false;
    std::vector<SessionEntry>::iterator it;
    for (it = mSessionList.begin(); it != mSessionList.end(); ++it) {
        SessionEntry entry = *it;
        if (entry.equal(&entryRemoved)) {
            bRemoved = true;
            DMC_LOGD(TAG, "removeSessionEntry() SID = %d, identity = %s",
                    entry.getSessionId(), entry.getSessionInfo()->identity);
            break;
        }
    }
    if (bRemoved) {
        mSessionList.erase(it);
    }
    return bRemoved;
}


bool SessionManager::getSessionEntry(const char *identity, SessionEntry **ppEntry) {
    bool bFound = false;
    for (unsigned int i = 0; i < getSessionNum(); i++) {
        const SessionInfo *entryInfo = mSessionList[i].getSessionInfo();
        char identity_low[MAX_IDENTITY_LEN] = {0};
        toLower(identity, identity_low);

        if (strcmp(identity_low, entryInfo->identity) == 0) {
            *ppEntry = (SessionEntry *)&mSessionList[i];
            bFound = true;
            break;
        }
    }
    return bFound;
}

bool SessionManager::getSessionEntry(SessionId sid, SessionEntry **ppEntry) {
    bool bFound = false;
    for (unsigned int i = 0; i < getSessionNum(); i++) {
        SessionEntry *session = &mSessionList[i];
        if (session->getSessionId() == sid) {
            *ppEntry = (SessionEntry *)session;
            bFound = true;
            break;
        }
    }
    return bFound;
}

void SessionManager::reset() {
    DMC_LOGD(TAG, "reset()");
    mSessionList.clear();
    std::vector<SessionEntry>(mSessionList).swap(mSessionList);
    SessionEntry::resetSessionId();
}

void SessionManager::toLower(const char *str_in, char *str_out) {
    strncpy(str_out, str_in, sizeof(str_out));

    while (*str_out) {
        *str_out = tolower(*str_out);
        str_out++;
    }
}


