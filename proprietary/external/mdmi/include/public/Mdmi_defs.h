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

#include "Mdmi.h"

#ifndef _MDMI_DEFS_H_
#define _MDMI_DEFS_H_

#define MAX_SESSIONS 1
#define INVALID_SESSION_ID (0)
#define INVALID_GROUP_ID (-1)


typedef int CoreSessionId;
typedef int LibSessionId;
typedef int MdmiErrorCode;

typedef enum {
    MDMI_LIB_UNDEFINED = -1,
    MDMI_LIB_BEGIN = 0,    
    MDMI_LIB_DEBUG = MDMI_LIB_BEGIN,
    MDMI_LIB_LTE,
    MDMI_LIB_GSM,
    MDMI_LIB_UMTS,
    MDMI_LIB_WIFI,
    MDMI_LIB_IMS,
    MDMI_LIB_COMMANDS,
    MDMI_LIB_WCDMA,
    MDMI_LIB_HSUPA,
    MDMI_LIB_HSDPA,
    MDMI_LIB_EMBMS,    
    MDMI_LIB_MAX
} MDMI_LIB_TYPE;

#define MDMI_LIB_NAME_DEBUG     "libVzw_mdmi_debug.so"
#define MDMI_LIB_NAME_LTE       "libVzw_mdmi_lte.so"
#define MDMI_LIB_NAME_GSM       "libVzw_mdmi_gsm.so"
#define MDMI_LIB_NAME_UMTS      "libVzw_mdmi_umts.so"
#define MDMI_LIB_NAME_WIFI      "libVzw_mdmi_wifi.so"
#define MDMI_LIB_NAME_IMS       "libVzw_mdmi_ims.so"
#define MDMI_LIB_NAME_COMMANDS  "libVzw_mdmi_commands.so"
#define MDMI_LIB_NAME_WCDMA     "libVzw_mdmi_wcdma.so"
#define MDMI_LIB_NAME_HSUPA     "libVzw_mdmi_hsupa.so"
#define MDMI_LIB_NAME_HSDPA     "libVzw_mdmi_hsdpa.so"
#define MDMI_LIB_NAME_EMBMS     "libVzw_mdmi_embms.so"

#define FN_NAME_CREATE_SESSION      "MdmiCreateSession"
#define FN_NAME_CLOSE_SESSION       "MdmiCloseSession"
#define FN_NAME_GET                 "MdmiGet"
#define FN_NAME_SET                 "MdmiSet"
#define FN_NAME_INVOKE              "MdmiInvoke"
#define FN_NAME_SET_EVENT_CALLBACK  "MdmiSetEventCallback"
#define FN_NAME_SUBSCRIBE           "MdmiSubscribe"
#define FN_NAME_UNSUBSCRIBE         "MdmiUnsubscribe"
#define FN_NAME_GET_SESSION_STATS   "MdmiGetSessionStats"

typedef struct {
    const char *libName;
} MdmiLibInfo_t;

typedef MdmiError (*FP_MDMI_CREATE_SESSION)(const wchar_t *, MdmiSession *);
typedef MdmiError (*FP_MDMI_CLOSE_SESSION)(MdmiSession);
typedef MdmiError (*FP_MDMI_GET)(MdmiSession, const MdmiObjectName *, MdmiValue *);
typedef MdmiError (*FP_MDMI_SET)(MdmiSession, const MdmiObjectName *, const MdmiValue *);
typedef MdmiError (*FP_MDMI_INVOKE)(MdmiSession, const MdmiObjectName *, const MdmiValue *);
typedef MdmiError (*FP_MDMI_SET_EVENT_CALLBACK)(MdmiSession, MdmiEventCallback, void *);
typedef MdmiError (*FP_MDMI_SUBSCRIBE)(MdmiSession, const MdmiObjectName *);
typedef MdmiError (*FP_MDMI_UNSUBSCRIBE)(MdmiSession, const MdmiObjectName *);
typedef MdmiError (*FP_MDMI_GET_SESSION_STATS)(MdmiSession, MdmiSessionStats *);

typedef struct {
    MDMI_LIB_TYPE type;
    FP_MDMI_CREATE_SESSION fpCreateSession;
    FP_MDMI_CLOSE_SESSION fpCloseSession;
    FP_MDMI_GET fpGet;
    FP_MDMI_SET fpSet;
    FP_MDMI_INVOKE fpInvoke;
    FP_MDMI_SET_EVENT_CALLBACK fpSetEventCallback;
    FP_MDMI_SUBSCRIBE fpSubscribe;
    FP_MDMI_UNSUBSCRIBE fpUnsubscribe;
    FP_MDMI_GET_SESSION_STATS fpGetSessionStats;
} MdmiLibFp_t;

#endif

