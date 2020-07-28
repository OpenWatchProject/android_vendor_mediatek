/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
/*
 * File name:  RfxDefs.h
 * Author: Jun Liu (MTK80064)
 * Description:
 *  Define some global macros, enums of ril proxy framework.
 */

#ifndef __RFX_DEFS_H__
#define __RFX_DEFS_H__
/*****************************************************************************
 * Include
 *****************************************************************************/
#include <telephony/mtk_ril.h>
/*****************************************************************************
 * Define
 *****************************************************************************/
#define RFX_SLOT_ID_UNKNOWN (-1)
#define PROPERTY_3G_SIM "persist.vendor.radio.simswitch"
#define PROPERTY_MODEM_EE  "vendor.ril.mux.ee.md1"
#define MAX_AT_RESPONSE (8 * 1024)

typedef enum {
    RFX_SLOT_ID_0 = 0,
    RFX_SLOT_ID_1,
    RFX_SLOT_ID_2,
    RFX_SLOT_ID_3,
    MAX_RFX_SLOT_ID
} RFX_SLOT_ID;
//#define RFX_SLOT_COUNT SIM_COUNT

#define RFX_OBJ_DEBUG

typedef enum {
    // Notice: value of id should be the same with the
    //         client id sended from modem
    RIL_IMS_Client_INVALID,
    RIL_IMS_Client_VILTE,
    RIL_IMS_Client_RCS,
    RIL_IMS_Client_UCE,
    // 5 bit to identify client id in fragment protocol,
    // Max value(31) is reserved for Admin
    RIL_IMS_Client_ADMIN = 31
} RilImsClientId;

typedef enum {
    RIL_IMS_CLIENT_READY = 1
} RilImsConfig;

#define RIL_SUPPORT_PROXYS  RIL_SUPPORT_CHANNELS

#define RIL_CMD_PROXY_5     RIL_CMD_4  // RIL_DATA
#define RIL_CMD_PROXY_1     RIL_CMD_3  // RIL_SIM, RIL_STK, RIL_SMS
#define RIL_CMD_PROXY_2     RIL_CMD_2  // RIL_CC, RIL_SS
#define RIL_CMD_PROXY_3     RIL_CMD_1  // RIL_NW, RIL_OEM
#define RIL_CMD_PROXY_URC   RIL_URC  // RIL_CMD_PROXY_4
#define RIL_CMD_PROXY_6     RIL_ATCI
#define RIL_CMD_PROXY_7     RIL_CMD_7  // RIL_RT
#define RIL_CMD_PROXY_8     RIL_CMD_8  // RIL_NRT
#define RIL_CMD_PROXY_9     RIL_CMD_9  // RIL_RADIO
#define RIL_CMD_PROXY_10    RIL_CMD_10  // RIL_DATA2
#define RIL_CMD_PROXY_11    RIL_CMD_11  // RIL_VSIM
#define RIL_CMD_PROXY_IMS   RIL_CMD_IMS  // ViLTE user data

#define RIL_PROXY_OFFSET    RIL_CHANNEL_OFFSET
#define RIL_CMD2_PROXY_5    RIL_CMD2_4
#define RIL_CMD2_PROXY_1    RIL_CMD2_3
#define RIL_CMD2_PROXY_2    RIL_CMD2_2
#define RIL_CMD2_PROXY_3    RIL_CMD2_1
#define RIL_CMD2_PROXY_URC  RIL_URC2  // RIL_CMD2_PROXY_4
#define RIL_CMD2_PROXY_6    RIL_ATCI2
#define RIL_CMD2_PROXY_7    RIL_CMD2_7
#define RIL_CMD2_PROXY_8    RIL_CMD2_8
#define RIL_CMD2_PROXY_9    RIL_CMD2_9
#define RIL_CMD2_PROXY_10   RIL_CMD2_10
#define RIL_CMD2_PROXY_11    RIL_CMD2_11  // RIL_VSIM
#define RIL_CMD2_PROXY_IMS  RIL_CMD2_IMS  // ViLTE user data

#define RIL_PROXY_SET3_OFFSET    RIL_CHANNEL_SET3_OFFSET
#define RIL_CMD3_PROXY_5    RIL_CMD3_4
#define RIL_CMD3_PROXY_1    RIL_CMD3_3
#define RIL_CMD3_PROXY_2    RIL_CMD3_2
#define RIL_CMD3_PROXY_3    RIL_CMD3_1
#define RIL_CMD3_PROXY_URC  RIL_URC3  // CMD3_PROXY_4
#define RIL_CMD3_PROXY_6    RIL_ATCI3
#define RIL_CMD3_PROXY_7    RIL_CMD3_7
#define RIL_CMD3_PROXY_8    RIL_CMD3_8
#define RIL_CMD3_PROXY_9    RIL_CMD3_9
#define RIL_CMD3_PROXY_10   RIL_CMD3_10
#define RIL_CMD3_PROXY_11    RIL_CMD3_11  // RIL_VSIM
#define RIL_CMD3_PROXY_IMS  RIL_CMD3_IMS  // ViLTE user data

#define RIL_PROXY_SET4_OFFSET    RIL_CHANNEL_SET4_OFFSET
#define RIL_CMD4_PROXY_5    RIL_CMD4_4
#define RIL_CMD4_PROXY_1    RIL_CMD4_3
#define RIL_CMD4_PROXY_2    RIL_CMD4_2
#define RIL_CMD4_PROXY_3    RIL_CMD4_1
#define RIL_CMD4_PROXY_URC  RIL_URC4  // RIL_CMD4_PROXY_4
#define RIL_CMD4_PROXY_6    RIL_ATCI4
#define RIL_CMD4_PROXY_7    RIL_CMD4_7
#define RIL_CMD4_PROXY_8    RIL_CMD4_8
#define RIL_CMD4_PROXY_9    RIL_CMD4_9
#define RIL_CMD4_PROXY_10   RIL_CMD4_10
#define RIL_CMD4_PROXY_11    RIL_CMD4_11  // RIL_VSIM
#define RIL_CMD4_PROXY_IMS  RIL_CMD4_IMS  // ViLTE user data

typedef enum {
    SLOT,
    NON_SLOT
} SLOT_CATEGORY;

// AT response error
/* SYNC TO 27.007 section 9.2.1 */
typedef enum {
    CME_ERROR_NON_CME = -1,
    /* Note: this is the only difference from spec mapping. 0 is phone failure defined in 27.007 */
    CME_SUCCESS = 0,
    CME_NO_CONNECTION_TO_PHONE = 1,
    CME_PHONE_ADAPTOR_LINK_RESERVED = 2,
    CME_OPERATION_NOT_ALLOWED_ERR = 3,
    CME_OPERATION_NOT_SUPPORTED = 4,
    CME_PH_SIM_PIN_REQUIRED = 5,
    CME_PH_FSIM_PIN_REQUIRED = 6,
    CME_PH_FSIM_PUK_REQUIRED = 7,
    CME_OPR_DTR_BARRING = 8,
    CME_SIM_NOT_INSERTED = 10,
    CME_CALL_BARRED = CME_SIM_NOT_INSERTED, // overwrite CME: 10
    CME_SIM_PIN_REQUIRED = 11,
    CME_SIM_PUK_REQUIRED = 12,
    CME_SIM_FAILURE = 13,
    CME_SIM_BUSY = 14,
    CME_SIM_WRONG = 15,
    CME_INCORRECT_PASSWORD = 16,
    CME_SIM_PIN2_REQUIRED = 17,
    CME_SIM_PUK2_REQUIRED = 18,
    CME_MEMORY_FULL = 20,
    CME_INVALID_INDEX = 21,
    CME_NOT_FOUND = 22,
    CME_MEMORY_FAILURE = 23,
    CME_TEXT_STRING_TOO_LONG = 24,
    CME_INVALID_CHARACTERS_IN_TEXT_STRING = 25,
    CME_DIAL_STRING_TOO_LONG = 26,
    CME_INVALID_CHARACTERS_IN_DIAL_STRING = 27,
    CME_NO_NETWORK_SERVICE = 30,
    CME_NETWORK_TIMEOUT = 31,
    CME_NETWORK_NOT_ALLOWED = 32,
    CME_NETWORK_PERSONALIZATION_PIN_REQUIRED = 40,
    CME_NETWORK_PERSONALIZATION_PUK_REQUIRED = 41,
    CME_NETWORK_SUBSET_PERSONALIZATION_PIN_REQUIRED = 42,
    CME_NETWORK_SUBSET_PERSONALIZATION_PUK_REQUIRED = 43,
    CME_SERVICE_PROVIDER_PERSONALIZATION_PIN_REQUIRED = 44,
    CME_SERVICE_PROVIDER_PERSONALIZATION_PUK_REQUIRED = 45,
    CME_CORPORATE_PERSONALIZATION_PIN_REQUIRED = 46,
    CME_CORPORATE_PERSONALIZATION_PUK_REQUIRED = 47,
    CME_HIDDEN_KEY_REQUIRED = 48,
    CME_UNKNOWN = 100,
    /* 3GPP TS 27.007 Errors related to a failure to activate a context, start */
    CME_INSUFFICIENT_RESOURCES = 126,
    CME_MISSING_UNKNOWN_APN = 127,
    CME_UNKNOWN_PDP_ADDRESS_TYPE = 128,
    CME_USER_AUTHENTICATION_FAILED = 129,
    CME_ACTIVATION_REJECT_GGSN = 130 ,
    CME_ACTIVATION_REJECT_UNSPECIFIED = 131,
    CME_SERVICE_OPTION_NOT_SUPPORTED = 132,
    CME_SERVICE_OPTION_NOT_SUBSCRIBED = 133,
    CME_SERVICE_OPTION_OUT_OF_ORDER = 134,
    CME_NSAPI_IN_USE = 135,
    CME_REGULAR_DEACTIVATION = 136,
    CME_QOS_NOT_ACCEPTED = 137,
    CME_NETWORK_FAILURE = 138,
    CME_REACTIVATION_REQUESTED = 139,
    CME_FEATURE_NOT_SUPPORTED = 140,
    CME_SEMANTIC_ERROR_IN_TFT = 141,
    CME_SYNTACTICAL_ERROR_IN_TFT = 142,
    CME_UNKNOWN_PDP_CONTEXT = 143,
    CME_SEMANTIC_ERROR_IN_PACKET_FILTER = 144,
    CME_SYNTACTICAL_ERROR_IN_PACKET_FILTER = 145,
    CME_PDP_CONTEXT_WITHOU_TFT_ALREADY_ACTIVATED = 146,
    CME_MULTICAST_GROUP_MEMBERSHIP_TIMEOUT = 147,
    CME_BCM_VIOLATION = 148,
    CME_PDP_AUTHENTICATION_FAILED = 149,
    CME_LAST_PDN_NOT_ALLOW = 151,
    CME_LAST_PDN_NOT_ALLOW_LR11 = 171,
    CME_OPERATOR_DETERMINED_BARRING = 177,
    CME_MAXIMUM_NMUBER_PDP_CONTEXTS_REACHED = 178,
    CME_REQUESTED_APN_NOT_SUPPORTED_IN_RAT_AND_PLMN = 179,
    /* 3GPP TS 27.007 Errors related to a failure to activate a context, end */
    // Define SIM ME lock NS_SP and SIM_C error code.
    CME_LINK_NS_SP_PIN_REQUIRED = 257,
    CME_LINK_NS_SP_PUK_REQUIRED = 258,
    CME_LINK_SIM_C_PIN_REQUIRED = 259,
    CME_LINK_SIM_C_PUK_REQUIRED = 260,
    // Remote SIM ME Lock added error code
    CME_VERIFICATION_FAILED = 270,
    CME_REBOOT_REQUEST = 271,
    // RJIL Subsidy Lock error codes
    CME_REBOOT_REQUIRED = 280,
    CME_BLOB_VALIDATION_FAILED = 281,
    CME_BLOB_CONFIG_DATA_INVALID = 282,
    CME_BLOB_IMEI_MISMATCH = 283,
    CME_BLOB_LENGTH_MISMATCH = 284,
    CME_UNKNOWN_ERROR = 285,
    /// M: IMS SS fail cause  @{
    CME_403_FORBIDDEN = 403,
    CME_404_NOT_FOUND = 404,
    CME_409_CONFLICT = 409,
    CME_412_PRECONDITION_FAILED = 412,
    /// @}

    CME_BT_SAP_UNDEFINED = 611,
    CME_BT_SAP_NOT_ACCESSIBLE = 612,
    CME_BT_SAP_CARD_REMOVED = 613,
    CME_SIM_TECHNICAL_PROBLEM = 616, // Technical problem

    /// M: IMS SS fail cause  @{
    CME_831_CONNECTION_ERROR = 831,
    CME_832_TERMINAL_BASE_SOLUTION = 832,
    CME_842_UNEXPECTED_ATCMD = 842,
    CME_844_UNPROVISIONED = 844,
    CME_845_NO_DATA_CONNECTION = 845,
    /// @}

    // Add new error for hold call failed caused by terminated
    CME_HOLD_FAILED_CAUSED_BY_TERMINATED = 1000,
    CME_ASYNC_IMS_CALL_CONTROL = 1001,

    CME_MM_IMSI_UNKNOWN_IN_VLR = 2052,
    CME_MM_IMEI_NOT_ACCEPTED = 2053,
    CME_MM_ILLEGAL_ME = 2054,
    CME_PHB_FDN_BLOCKED = 2600,
    CME_UNKNOWN_PDP_ADDR_OR_TYPE = 3100,
    CME_LOCAL_REJECTED_DUE_TO_PS_SWITCH = 3914,
    CME_L4C_CONTEXT_CONFLICT_DEACT_ALREADY_DEACTIVATED = 4105, //(0x1009)
    /// M: [C2K]Dynamic switch support. @{
    CME_NOT_ALLOW_RADIO_ON = 619,
    /// @}
    CME_MULTI_ALLOW_ERR = 4117,
    // [C2K][IRAT] Add new fail cause to represent deactivated fail during IRAT.
    CME_L4C_CMD_CONFLICT_C2K_AP_DEACT_RETRY_NEEDED = 4225, //(0x1081)
    /// M: [SS]USSI CSFB support. @{
    CME_USSI_NOT_SUPPORTED = 65535
    /// @}
} AT_CME_Error;

typedef enum {
    INIT = -1,
    NO_RESULT,   /* no intermediate response expected */
    NUMERIC,     /* a single intermediate response starting with a 0-9 */
    SINGLELINE,  /* a single intermediate response starting with a prefix */
    MULTILINE,    /* multiple line intermediate response
                    starting with a prefix */
    /* atci start */
    RAW
    /* atci end */
} AtCommandType;

typedef enum {
    RIL_RUN_MODE_NORMORL,
    RIL_RUN_MODE_MOCK
}RilRunMode;

/**
 * For request priority mechanism.
 */
typedef enum {
    MTK_RIL_REQUEST_PRIORITY_HIGH,
    MTK_RIL_REQUEST_PRIORITY_MEDIUM,
    MTK_RIL_REQUEST_PRIORITY_LOW
} MTK_RIL_REQUEST_PRIORITY;

/* Constants for response types */
#define RESPONSE_SOLICITED 0
#define RESPONSE_UNSOLICITED 1
#define RESPONSE_SOLICITED_ACK 2
#define RESPONSE_SOLICITED_ACK_EXP 3
#define RESPONSE_UNSOLICITED_ACK_EXP 4
///

// AT command error code
#define AT_ERROR_GENERIC -1
#define AT_ERROR_COMMAND_PENDING -2
#define AT_ERROR_CHANNEL_CLOSED -3
#define AT_ERROR_TIMEOUT -4
#define AT_ERROR_INVALID_THREAD -5 /* AT commands may not be issued from
                                       reader thread (or unsolicited response
                                       callback */
#define AT_ERROR_INVALID_RESPONSE -6 /* eg an at_send_command_singleline that
                                        did not get back an intermediate
                                        response */
#define AT_ERROR_RADIO_UNAVAILABLE -7 /* send at command fail if modem is not available */

#define RAW_DATA_ERROR_GENERIC -1

// IMS error cause
// Hold call error cause
#define RIL_E_ERROR_IMS_CONTROL_CALL_FAILED_CALL_TERMINATED RIL_E_OEM_ERROR_1
// Async call control error cause
#define RIL_E_ERROR_ASYNC_IMS_CALL_CONTROL_WAIT_RESULT RIL_E_OEM_ERROR_2

typedef enum {
    URC_DISPATCH_TO_RILJ = 0,
    URC_DISPATCH_TO_IMSRIL = 1,
    URC_DISPATCH_TO_RILJ_IMSRIL = 2
} UrcDispatchRule;

// To indicate useless ID
#define INVALID_ID -1

// for log reduction
typedef enum {
    VERBOSE,
    DEBUG,
    INFO,
    WARN,
    ERROR
} LogLevel;

typedef enum {
    RFOFF_CAUSE_UNSPECIFIED = 0,
    RFOFF_CAUSE_DUPLEX_MODE = 1,
    RFOFF_CAUSE_POWER_OFF = 2,
    RFOFF_CAUSE_SIM_SWITCH = 3,
} RadioCause;

#endif /* __RFX_DEFS_H__ */
