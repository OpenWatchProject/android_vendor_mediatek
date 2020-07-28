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

#ifndef __CORE_SESSION_H__
#define __CORE_SESSION_H__

#include <stdint.h>
#include "Mdmi.h"
#include "Mdmi_defs.h"
#include <vendor/mediatek/hardware/dmc/1.0/IDmcService.h>

using vendor::mediatek::hardware::dmc::V1_0::IDmcService;
using vendor::mediatek::hardware::dmc::V1_0::IDmcEventCallback;
using vendor::mediatek::hardware::dmc::V1_0::dmcCommand_e;
using vendor::mediatek::hardware::dmc::V1_0::dmcResult_e;
using vendor::mediatek::hardware::dmc::V1_0::dmcSessionConfig_t;
using vendor::mediatek::hardware::dmc::V1_0::dmcSessionStats_t;
using vendor::mediatek::hardware::dmc::V1_0::dmcObjectId_t;
using vendor::mediatek::hardware::dmc::V1_0::dmcValue_t;

using android::hardware::Return;
using android::hardware::hidl_death_recipient;
using android::hidl::base::V1_0::IBase;
using android::sp;
using android::wp;

// Print the KPI buffer
#define PRINT_OUTPUT_BUFFER 0

#define MDMI_IDENTITY "mdmi"
#define MDMI_VERSION "v2.8.2"
#define DMC_HIDL_SERVER_NAME "dmc_hidl_service"
#define OID_NAME_LEN_MAX (50)

typedef struct {
    MdmiSession sessionId;
    MdmiEventCallback callback;
    void *param;
} SessionInfo_t;

class DmcDeathRecipient : public hidl_death_recipient {
    void serviceDied(uint64_t cookie, const wp<IBase>& who) override;
};

class DmcEventCallback : public IDmcEventCallback {
    Return<dmcResult_e> onHandleEvent(
            CoreSessionId session, const dmcObjectId_t &objectId, const dmcValue_t &eventValue) override;
};

class CoreSession {
private:
    static void sessionCallback(
            const MdmiObjectName* eventName, const MdmiValue* eventValue, void* param, MDMI_LIB_TYPE libType);
    static LibSessionId mapLibSessionId(MDMI_LIB_TYPE libType);

    // DMC interface
    bool tryGetDmcService(const char *caller);
    void resetDmcConnection();

public:
    CoreSession();
    ~CoreSession();

    static SessionInfo_t sSessionInfo[MDMI_LIB_MAX];
    static CoreSessionId sCoreSessionId;
    static sp<IDmcService> s_pDmcConnection;
    static sp<IDmcEventCallback> s_pDmcEventCallback;

    static void objNameCoreToHidl(const MdmiObjectName *oidIn, dmcObjectId_t *oidOut);
    static void objNameHidlToCore(const dmcObjectId_t *oidIn, MdmiObjectName *oidOut);
    static void printKpiBuf(const char *prefix, const unsigned char *buf, unsigned int buf_len);
    static void printBufHex(const unsigned char *bin, unsigned int binsz, char **result);
    static void getOidNameString(const MdmiObjectName* eventName, char *result);

    MdmiErrorCode createCoreSession(void);
    MdmiErrorCode closeCoreSession(void);
    CoreSessionId getCoreSessionId(void);

    MdmiErrorCode createLibSession(MDMI_LIB_TYPE libType, LibSessionId *sessionId);
    MdmiErrorCode closeLibSession(MDMI_LIB_TYPE libType, LibSessionId sessionId);
    int getCurrentLibSessionCount(void);
    bool isLibSessionValid(MDMI_LIB_TYPE libType, LibSessionId sessionId);

    MdmiErrorCode subscribe(MDMI_LIB_TYPE libType, const MdmiObjectName* eventName);
    MdmiErrorCode unsubscribe(const MdmiObjectName* eventName);
    void setSessionCallback(MDMI_LIB_TYPE libType, MdmiEventCallback callback, void *param);
    void *getSessionParam(MDMI_LIB_TYPE libType);
    MdmiErrorCode getLibSessionStats(MDMI_LIB_TYPE libType, MdmiSessionStats *statistics);
    // int GetKpiBuffer(const MdmiObjectName *name, MdmiValue* value);
};

#endif // __CORE_SESSION_H__
