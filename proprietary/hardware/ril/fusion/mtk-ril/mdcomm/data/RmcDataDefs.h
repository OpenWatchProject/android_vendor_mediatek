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

#ifndef __RMC_DATA_DEFS_H__
#define __RMC_DATA_DEFS_H__

#include <telephony/mtk_ril.h>

#define ME_PDN_ACT "ME PDN ACT"
#define NW_ACT "NW ACT"
#define NW_MODIFY "NW MODIFY"
#define NW_PDN_ACT "NW PDN ACT"
#define NW_PDN_DEACT "NW PDN DEACT"
#define ME_PDN_DEACT "ME PDN DEACT"
#define NW_DEACT "NW DEACT"
#define ME_DEACT "ME DEACT"
#define NW_REACT "NW REACT"

// M: Test mode.
#define CLEAR_PDN_TABLE "CLEAR PDN TABLE"
#define CLEAR_ALL_PDN_TABLE "CLEAR ALL PDN TABLE"
#define SET_TRANSACTION_INTERFACE_ID "SET TRANSACTION INTERFACE ID"
#define REMOVE_TRANSACTION_INTERFACE_ID "REMOVE TRANSACTION INTERFACE ID"
#define REMOVE_ALL_TRANSACTION_INTERFACE_ID "REMOVE ALL TRANSACTION INTERFACE ID"

#define MAX_APN_NAME_LENGTH 100

#define AUTHTYPE_NONE       0
#define AUTHTYPE_PAP        1
#define AUTHTYPE_CHAP       2
#define AUTHTYPE_PAP_CHAP   3

#define SETUP_DATA_PROTOCOL_IP      "IP"
#define SETUP_DATA_PROTOCOL_IPV6    "IPV6"
#define SETUP_DATA_PROTOCOL_IPV4V6  "IPV4V6"

#define INVALID_AID -1
#define INVALID_INTERFACE_ID -1
#define INVALID_TRANS_INTF_ID -1
#define INVALID_LIFE_TIME -1
#define INVALID_VALUE -1

#define INVALID_PDN_TYPE 0
#define IPV4        1
#define IPV6        2
#define IPV4V6      3

#define NULL_IPV4_ADDRESS "0.0.0.0"
#define MAX_IPV4_ADDRESS_LENGTH 17
//xxx.xxx.xxx.xxx
#define MAX_IPV6_ADDRESS_LENGTH 65
//xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx
//xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx

#define MAX_NUM_IPV6_ADDRESS_NUMBER 2
#define MAX_NUM_DNS_ADDRESS_NUMBER 3

#define PROPERTY_DATA_NETWORK_TYPE "gsm.network.type"

#define IMS_EIMS_PDN_INFO "vendor.ims.eims.pdn.info"

#define OPERATOR_OP07 "OP07"
#define OPERATOR_OP08 "OP08"
#define OPERATOR_OP09 "OP09"
#define OPERATOR_OP12 "OP12"
#define OPERATOR_OP20 "OP20"
#define SEGDEFAULT "SEGDEFAULT"

#define IPV6_PREFIX "FE80:0000:0000:0000:"
#define NULL_ADDR ""

#define REQUEST_TYPE_NORMAL 0
#define REQUEST_TYPE_HANDOVER 3

#define DC_NEW "new"
#define DC_UPDATE "update"
#define DC_CHANGE "dcchg"
#define DC_ERROR "err"
#define DC_DEACT_INFO "deact_info"
#define DC_ADDRESS "addr"
#define DC_RAN "ran"

#define BEARER_BITMASK_OFFSET 11
#define ALL_BEARER_EXCLUDE_IWLAN_BITMASK 0x7ffdffff
#define ALL_BEARER_BITMASK 0x7fffffff

#define TRANSACTION_ID_OFFSET 100

#define DRB_NOT_EXISTED 0

#define NO_REASON 0

// Fast dormancy begin
#define PROPERTY_MTK_FD_SUPPORT    "ro.vendor.mtk_fd_support"
#define PROPERTY_FD_ON_ONLY_R8_NETWORK  "persist.vendor.radio.fd.on.only.r8"
#define PROPERTY_RIL_FD_MODE       "vendor.ril.fd.mode"

#define PROPERTY_FD_SCREEN_ON_TIMER     "persist.vendor.radio.fd.counter"
#define PROPERTY_FD_SCREEN_ON_R8_TIMER  "persist.vendor.radio.fd.r8.counter"
#define PROPERTY_FD_SCREEN_OFF_TIMER    "persist.vendor.radio.fd.off.counter"
#define PROPERTY_FD_SCREEN_OFF_R8_TIMER "persist.vendor.radio.fd.off.r8.counter"

/* Fast dormancy related timer: units: 0.1 sec */
#define DEFAULT_FD_SCREEN_ON_TIMER "150"
#define DEFAULT_FD_SCREEN_ON_R8_TIMER "150"
#define DEFAULT_FD_SCREEN_OFF_TIMER "50"
#define DEFAULT_FD_SCREEN_OFF_R8_TIMER "50"

static const char PROPERTY_FD_TIMER[4][50] = {
    PROPERTY_FD_SCREEN_OFF_TIMER,
    PROPERTY_FD_SCREEN_ON_TIMER,
    PROPERTY_FD_SCREEN_OFF_R8_TIMER,
    PROPERTY_FD_SCREEN_ON_R8_TIMER
};

#define FASTDORMANCY_ON 0x3FFFFF
#define FASTDORMANCY_LEGACY_OFF 0x400000
#define FASTDORMANCY_LOW_DATA_EXPECTED 2
#define FASTDORMANCY_PARAMETER_LENGTH 2
// Fast dormancy end

#define BTOS(x) ((x) ? "true" : "false")

#define AT_LINE_FREE(line)   \
if (line) {                  \
    delete line;             \
    line = NULL;             \
}

#define FREEIF(data)    \
if (data != NULL) {     \
    free(data);         \
    data = NULL;        \
}

#define UNSPECIFIED_IPV6_GATEWAY    "::"

#define SM_CAUSE_BASE 0xC00 // Defined in AT_DOCUMENT_full.doc
#define SM_CAUSE_END SM_CAUSE_BASE + 0x72
#define ESM_CAUSE_BASE 0xD00 // ESM cause
#define ESM_CAUSE_END ESM_CAUSE_BASE + 0xFF
#define PAM_CAUSE_BASE 0x1200 // PAM cause
#define PAM_CAUSE_END PAM_CAUSE_BASE + 0xC9
#define CME_CAUSE_BASE_V14 0x64 // Defined in 3GPP TS 27.007 V14
#define CME_CAUSE_END_V14 CME_CAUSE_BASE_V14 + 0x64

// SSC Mode3 PDU
#define MTK_PDU_SSC_MODE_3 (-1100)
#define MTK_DEFAULT_LIFE_TIME 0xA8C0

const char PROPERTY_ICCID_SIM[MAX_SIM_COUNT][25] = {
    "vendor.ril.iccid.sim1",
    "vendor.ril.iccid.sim2",
    "vendor.ril.iccid.sim3",
    "vendor.ril.iccid.sim4",
};

typedef struct {
    int transIntfId; // trans_id * 100 + interface_id
    int primaryAid;
    int aid;
    int ddcId;  // for dedicate bearer
    int isDedicateBearer;
    int isEmergency;  // 0: normal, 1: emergency PDN
    int active;  // 0: inactive, 1: link down, 2: active
    int signalingFlag;
    char apn[MAX_APN_NAME_LENGTH];
    char addressV4[MAX_IPV4_ADDRESS_LENGTH];
    char addressV6[MAX_NUM_IPV6_ADDRESS_NUMBER][MAX_IPV6_ADDRESS_LENGTH];
    char dnsV4[MAX_NUM_DNS_ADDRESS_NUMBER][MAX_IPV4_ADDRESS_LENGTH];
    char dnsV6[MAX_NUM_DNS_ADDRESS_NUMBER][MAX_IPV6_ADDRESS_LENGTH];
    int bearerId;
    char pcscf[MAX_PCSCF_NUMBER][MAX_IPV6_ADDRESS_LENGTH];
    int mtu;
    int rat;    /* 0: UNSPEC, 1: MOBILE_3GPP, 2: WIFI, 3: MOBILE_3GPP2
                   4: TYPE_MAX, 0xFE: RDS_RAN_NONE, 0xFF: RDS_RAN_DENY */
    int reason;  // -1: no cause, 0: ipv4 only, 1: ipv6 only, 2: single bearer only allowed
    int deactReason;  // for pdn deact reason
    int pdnType; //for pdn type
    int profileId;
    int sscMode;
} PdnInfo;

typedef struct {
    int activeCid;
    int reason;
} MePdnActiveInfo;

typedef enum {
    DATA_STATE_INACTIVE = 0,
    DATA_STATE_LINKDOWN = 1,
    DATA_STATE_ACTIVE = 2
} PDN_STATUS;

typedef enum {
    NO_CAUSE = -1,
    IPV4_ONLY = 0,
    IPV6_ONLY = 1,
    SINGLE_BEARER_ONLY_ALLOWED = 2
} PDN_REASON;

typedef enum {
    LOCAL_REJECT_CAUSE = -2,
    CHANNEL_CLOSED = -1,
    PDN_FAILED = 0,
    PDN_SUCCESS = 1
} PDN_SETUP_STATUS;

typedef enum {
    NETAGENT_ADDR_TYPE_UNKNOWN    = 0,
    NETAGENT_ADDR_TYPE_IPV4       = 1,
    NETAGENT_ADDR_TYPE_IPV6       = 2,
    NETAGENT_ADDR_TYPE_IPV4V6     = 3,
    NETAGENT_ADDR_TYPE_ANY        = 4
} NETAGENT_ADDR_TYPE;

typedef enum {
    NETAGENT_IFST_NONE        = 0,
    NETAGENT_IFST_UP          = 16
} NETAGENT_IFST_STATUS;

typedef enum {
    IMS_RELEASE_PDN = 0,
    IMS_REQUEST_PDN = 1
} IMS_NOTIFY_TYPE;

typedef enum {
    ADDRESS_NULL = 0,
    ADDRESS_IPV4 = 1,
    ADDRESS_IPV6_UNIQUE_LOCAL = 2,
    ADDRESS_IPV6_SITE_LOCAL = 3,
    ADDRESS_IPV6_LINK_LOCAL = 4,
    ADDRESS_IPV6_GLOBAL = 5
} ADDRESS_TYPE;

typedef enum {
    DC_EVENT_UNKNOWN = 0x0,
    DC_EVENT_DATA_RESET_NOTIFY = 0x81, //129
    DC_EVENT_IP_NORMAL_CHANGE = 0x100,
    DC_EVENT_NO_RA = 0x101,
    DC_EVENT_HANDOVER_NORMAL_CHANGE = 0x200
} DC_CHANGE_EVENT;

typedef enum {
    PS_ACT_GSM = 0,
    PS_ACT_GSM_COMPACT,
    PS_ACT_UTRAN,
    PS_ACT_GSM_W_EGPRS,
    PS_ACT_UTRAN_W_HSDPA,
    PS_ACT_UTRAN_W_HSUPA,
    PS_ACT_UTRAN_W_HSDPA_AND_HSUPA,
    PS_ACT_E_UTRAN,
} PS_ACCESS_TECHNOLOGY;

typedef enum {
    DRB_STATE = 0,
    DRB_ACT,
    DRB_NUM,
} DRB_PARAMETER;

typedef enum {
    PDN_MD_INITIATED = 1,
    PDN_AP_INITIATED,
} PDN_INITIATOR;

/**
 * AOSP reasons are defined by REQUEST_REASON_XXX in DataService.java
 * and MTK reasons are defined by ACTIVATE_REASON_XXX in MtkRILConstants.java.
 * Please make sure the values defined here are the same as those in the upper layers.
 */
typedef enum {
    ACT_REASON_UNKNOWN = 0,
    ACT_REASON_AOSP_NORMAL = 1,
    ACT_REASON_AOSP_HANDOVER = 3,
    ACT_REASON_SSC_MODE3 = 1001,
} PDN_ACT_REASON;

/**
 * AOSP reasons are defined by REQUEST_REASON_XXX in DataService.java
 * and MTK reasons are defined by DEACTIVATE_REASON_XXX in MtkRILConstants.java.
 * Please make sure the values defined here are the same as those in the upper layers.
 */
typedef enum {
    DEACT_REASON_UNKNOWN = 0,
    DEACT_REASON_AOSP_NORMAL = 1,
    DEACT_REASON_AOSP_SHUTDOWN = 2,
    DEACT_REASON_AOSP_HANDOVER = 3,
    DEACT_REASON_NORMAL = 2001,
    DEACT_REASON_RA_INITIAL_FAIL = 2002,
    DEACT_REASON_NO_PCSCF = 2003,
    DEACT_REASON_RA_REFRESH_FAIL = 2004,
    DEACT_REASON_APN_CHANGED = 2005,
} PDN_DEACT_REASON;

/**
 * These are EGACT deactivate reasons used by modem, add/remove/modify should be reviewed by
 * both AP and MD.
 */
typedef enum {
    ATCMD_DEACT_CAUSE_EGACT_CAUSE_DONT_CARE = 0, //EGACT_CAUSE_DONT_CARE
    ATCMD_DEACT_CAUSE_EGACT_CAUSE_DEACT_NORMAL = 1, //EGACT_CAUSE_DEACT_NORMAL
    ATCMD_DEACT_CAUSE_EGACT_CAUSE_DEACT_RA_INITIAL_ERROR = 2, //EGACT_CAUSE_DEACT_RA_INITIAL_ERROR
    ATCMD_DEACT_CAUSE_EGACT_CAUSE_DEACT_NO_PCSCF = 3, //EGACT_CAUSE_DEACT_NO_PCSCF
    ATCMD_DEACT_CAUSE_EGACT_CAUSE_DEACT_RA_REFRESH_ERROR = 4, //EGACT_CAUSE_DEACT_RA_REFRESH_ERROR
    ATCMD_DEACT_CAUSE_EGACT_CAUSE_DEACT_RA_ERROR = 5, //EGACT_CAUSE_DEACT_RA_ERROR
    ATCMD_DEACT_CAUSE_EGACT_CAUSE_USER_FORCE_TO_LOCAL_RELEASE = 6, //Used in user wants to deact a PDN immediately (without peer to peer air message waiting)
    ATCMD_DEACT_CAUSE_EGACT_CAUSE_AOSP_SHUTDOWN = 7,
    ATCMD_DEACT_CAUSE_EGACT_CAUSE_AOSP_HANDOVER = 8,
    ATCMD_DEACT_CAUSE_EGACT_CAUSE_APN_CHANGED = 9,
} ATCMD_DEACT_CAUSE_ENUM;

typedef enum {
    SSC_UNKNOWN = -1,
    SSC_MODE1 = 0,
    SSC_MODE2 = 1,
    SSC_MODE3 = 2,
} SSC_MODE;

#endif /* __RMC_DATA_DEFS_H__ */
