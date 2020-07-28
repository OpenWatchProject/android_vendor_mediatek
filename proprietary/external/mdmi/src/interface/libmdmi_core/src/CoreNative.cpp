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

#include "CoreNative.h"
#include "CoreSession.h"
#include "MdmiPermission.h"
#include "Mdmi_utils.h"

const static char* TAG = "CoreNative";

// Single instance of CoreSession to control all MDMI libraries
static CoreSession *g_pCoreSession = NULL;

bool checkSessionState(const char *caller, MDMI_LIB_TYPE type, MdmiSession sessionId) {
    if (g_pCoreSession == NULL) {
        MDMI_LOGE(TAG, "%s failed with NULL g_pCoreSession", caller);
        return false;
    }
    
    if (!g_pCoreSession->isLibSessionValid(type, (LibSessionId)sessionId)) {
        MDMI_LOGE(TAG, "%s failed with invalid SID = %d for libType = %d", caller, sessionId, type);
        return false;
    }

    return true;
}

// ==================== Public Functions ========================
MdmiError MdmiCoreCreateSession(MDMI_LIB_TYPE type, const wchar_t *address, MdmiSession *session) {
    MdmiError ret = MDMI_NO_ERROR;

    if (!android::checkPermission(type)) {
        return MDMI_ERROR_PERMISSION_DENIED_GROUPID_FAIL;
    }

    if (g_pCoreSession == NULL) {
        g_pCoreSession = new CoreSession();
        ret = g_pCoreSession->createCoreSession();
    }

    if (ret != MDMI_NO_ERROR) {
        MDMI_LOGE(TAG, "MdmiCoreCreateSession() libType = %d failed! ret = %d", type, ret);
        delete g_pCoreSession;
        g_pCoreSession = NULL;
        return ret;
    }

    ret = g_pCoreSession->createLibSession(type, (LibSessionId *)session);
    MDMI_LOGD(TAG, "MdmiCoreCreateSession() libType = %d, Lib SID = %d, ret = %d",
            type, *session, ret);

    return ret;
}

MdmiError MdmiCoreCloseSession(MDMI_LIB_TYPE type, MdmiSession session) {
    MdmiError ret = MDMI_NO_ERROR;

    if (!android::checkPermission(type)) {
        return MDMI_ERROR_PERMISSION_DENIED_GROUPID_FAIL;
    }

    if (!checkSessionState("MdmiCoreCloseSession()", type, session)) {
        return MDMI_ERROR_INVALID_SESSION;
    }

    ret = g_pCoreSession->closeLibSession(type, (LibSessionId)session);

    if (g_pCoreSession->getCurrentLibSessionCount() == 0) {
        MDMI_LOGD(TAG, "MdmiCoreCloseSession() close core session");
        g_pCoreSession->closeCoreSession();
        delete g_pCoreSession;
        g_pCoreSession = NULL;
    }
    MDMI_LOGD(TAG, "MdmiCoreCloseSession() type = %d, SID = %d, ret = %d", type, session, ret);

    return ret;
}

MdmiError MdmiCoreGet(MDMI_LIB_TYPE type, MdmiSession session, const MdmiObjectName* name, MdmiValue* value) {
    MdmiError ret = MDMI_NO_ERROR;

    // TODO: complete me
    /*
    if (!android::checkPermission(type)) {
        return MDMI_ERROR_PERMISSION_DENIED_GROUPID_FAIL;
    }

    if (!pSession || !pSession->IsLibSessionValid(type, session)) {
        return MDMI_ERROR_INVALID_SESSION;
    }

    if (!pSession->IsOIDValid(type, name)) {
        return MDMI_ERROR_INVALID_OID;
    }

    ret = pSession->GetKpiBuffer(name, value);
    */
    return ret;
}

MdmiError MdmiCoreSet(MDMI_LIB_TYPE type, MdmiSession session, const MdmiObjectName* name, const MdmiValue* value) {
    MDMI_LOGE(TAG, "MdmiCoreSet() NOT supported");
    return MDMI_ERROR_INVALID_OPERATION;
}

MdmiError MdmiCoreInvoke(MDMI_LIB_TYPE type, MdmiSession session, const MdmiObjectName* name, const MdmiValue* value) {
    MDMI_LOGE(TAG, "MdmiCoreInvoke() NOT supported");
    return MDMI_NO_ERROR;
}

MdmiError MdmiCoreSetEventCallback(MDMI_LIB_TYPE type, MdmiSession session, MdmiEventCallback callback, void* state) {

    if (!android::checkPermission(type)) {
        return MDMI_ERROR_PERMISSION_DENIED_GROUPID_FAIL;
    }
    if (!checkSessionState("MdmiCoreSetEventCallback()", type, session)) {
        return MDMI_ERROR_INVALID_SESSION;
    }

    MDMI_LOGD(TAG, "MdmiCoreSetEventCallback()");
    g_pCoreSession->setSessionCallback(type, callback, state);

    return MDMI_NO_ERROR;
}

MdmiError MdmiCoreSubscribe(MDMI_LIB_TYPE type, MdmiSession session, const MdmiObjectName* eventName) {
    MdmiError ret = MDMI_NO_ERROR;

    if (!android::checkPermission(type)) {
        return MDMI_ERROR_PERMISSION_DENIED_GROUPID_FAIL;
    }

    if (!checkSessionState("MdmiCoreSubscribe()", type, session)) {
        return MDMI_ERROR_INVALID_SESSION;
    }

    ret = g_pCoreSession->subscribe(type, eventName);
    MDMI_LOGD(TAG, "MdmiCoreSubscribe() libType = %d, SID = %d, ret = %d", type, session, ret);

    return ret;
}

MdmiError MdmiCoreUnsubscribe(MDMI_LIB_TYPE type, MdmiSession session, const MdmiObjectName* eventName) {
    MdmiError ret = MDMI_NO_ERROR;

    if (!android::checkPermission(type)) {
        return MDMI_ERROR_PERMISSION_DENIED_GROUPID_FAIL;
    }
    if (!checkSessionState("MdmiCoreUnsubscribe()", type, session)) {
        return MDMI_ERROR_INVALID_SESSION;
    }

    ret = g_pCoreSession->unsubscribe(eventName);
    MDMI_LOGD(TAG, "MdmiCoreUnsubscribe() libType = %d, SID = %d, ret = %d", type, session, ret);

    return ret;
}

MdmiError MdmiCoreGetSessionStats(MDMI_LIB_TYPE type, MdmiSession session, MdmiSessionStats* stats) {
    MdmiError ret = MDMI_NO_ERROR;

    if (!android::checkPermission(type)) {
        return MDMI_ERROR_PERMISSION_DENIED_GROUPID_FAIL;
    }
    if (!checkSessionState("MdmiCoreGetSessionStats()", type, session)) {
        return MDMI_ERROR_INVALID_SESSION;
    }

    ret = g_pCoreSession->getLibSessionStats(type, stats);
    MDMI_LOGD(TAG, "MdmiCoreUnsubscribe() libType = %d, SID = %d, ret = %d", type, session, ret);

    return ret;
}
