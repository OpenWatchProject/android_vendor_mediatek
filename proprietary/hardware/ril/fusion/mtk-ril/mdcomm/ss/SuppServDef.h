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

#include <telephony/mtk_ril.h>

#ifndef SUPP_SERV_DEF
#define SUPP_SERV_DEF

/* USSD messages using the default alphabet are coded with the GSM 7-bit default alphabet         *
 * given in clause 6.2.1. The message can then consist of up to 182 user characters (3GPP 23.038) */
#define MAX_RIL_USSD_STRING_LENGTH 255

/**
 * number string cannot more than MAX_RIL_USSD_NUMBER_LENGTH digits
 * 160 byte GSM 7bit USSD = 182 characters
 */
#define MAX_RIL_USSD_NUMBER_LENGTH 182

/**
 * Use RILD's feature version manager to konw if current modem platform
 * support USSD domain selection
 */
#define FEATURE_MD_USSD_DOMAIN_SELECTION "USSD_DOMAIN_SELECTION"

#define RIL_E_409_CONFLICT RIL_E_OEM_ERROR_25
#define RIL_E_UT_XCAP_403_FORBIDDEN RIL_E_OEM_ERROR_2
#define RIL_E_UT_UNKNOWN_HOST RIL_E_OEM_ERROR_3
#define RIL_E_404_NOT_FOUND RIL_E_OEM_ERROR_4
#define RIL_E_CALL_BARRED RIL_E_OEM_ERROR_5
#define RIL_E_412_PRECONDITION_FAILED RIL_E_OEM_ERROR_6
#define RIL_E_832_TERMINAL_BASE_SOLUTION RIL_E_OEM_ERROR_7
#define RIL_E_845_NO_DATA_CONNECTION RIL_E_NETWORK_NOT_READY
#define RIL_E_USSD_TO_DIAL_VIDEO RIL_E_OEM_ERROR_1

/*
 * <classx> is a sum of integers each representing a class of information (default 7):
 * 1    voice (telephony)
 * 2    data (refers to all bearer services; with <mode>=2 this may refer only to some bearer service if TA does not support values 16, 32, 64 and 128)
 * 4    fax (facsimile services)
 * 8    short message service
 * 16   data circuit sync
 * 32   data circuit async
 * 64   dedicated packet access
 * 128  dedicated PAD access
 */
typedef enum {
    CLASS_NONE                      = 0,
    CLASS_VOICE                     = 1,
    CLASS_DATA                      = 2,
    CLASS_FAX                       = 4,
    CLASS_DEFAULT                   = 7,
    CLASS_SMS                       = 8,
    CLASS_DATA_SYNC                 = 16,
    CLASS_DATA_ASYNC                = 32,
    CLASS_DEDICATED_PACKET_ACCESS   = 64,
    CLASS_DEDICATED_PAD_ACCESS      = 128,
    CLASS_MTK_LINE2                 = 256,
    CLASS_MTK_VIDEO                 = 512
} AtInfoClassE;

typedef enum {
    BS_ALL_E                        = 0,
    BS_TELE_ALL_E                   = 10,
    BS_TELEPHONY_E                  = 11,
    BS_TELE_DATA_ALL_E              = 12,
    BS_TELE_FAX_E                   = 13,
    BS_TELE_SMS_E                   = 16,
    BS_TELE_VGCS_E                  = 17, /* Not supported by framework */
    BS_TELE_VBS_E                   = 18, /* Not supported by framework */
    BS_TELE_ALL_EXCEPT_SMS_E        = 19,
    BS_DATA_ALL_E                   = 20,
    BS_DATA_ASYNC_ALL_E             = 21,
    BS_DATA_SYNC_ALL_E              = 22,
    BS_DATA_CIRCUIT_SYNC_E          = 24,
    BS_DATA_CIRCUIT_ASYNC_E         = 25,
    BS_DATA_SYNC_TELE_E             = 26, /* Supported by framework */
    BS_GPRS_ALL_E                   = 99
} BsCodeE;

/***
 * "AO"    BAOC (Barr All Outgoing Calls) (refer 3GPP TS 22.088 [6] clause 1)
 * "OI"    BOIC (Barr Outgoing International Calls) (refer 3GPP TS 22.088 [6] clause 1)
 * "OX"    BOIC exHC (Barr Outgoing International Calls except to Home Country) (refer 3GPP TS 22.088 [6] clause 1)
 * "AI"    BAIC (Barr All Incoming Calls) (refer 3GPP TS 22.088 [6] clause 2)
 * "IR"    BIC Roam (Barr Incoming Calls when Roaming outside the home country) (refer 3GPP TS 22.088 [6] clause 2)
 * "AB"    All Barring services (refer 3GPP TS 22.030 [19]) (applicable only for <mode>=0)
 * "AG"    All outGoing barring services (refer 3GPP TS 22.030 [19]) (applicable only for <mode>=0)
 * "AC"    All inComing barring services (refer 3GPP TS 22.030 [19]) (applicable only for <mode>=0)
 */

typedef enum {
    CB_BAOC,
    CB_BOIC,
    CB_BOIC_EXHC,
    CB_BAIC,
    CB_BIC_ROAM,
    CB_ABS,
    CB_AOBS,
    CB_AIBS,
    CB_ACR,
    CB_SUPPORT_NUM
} CallBarServicesE;


typedef enum {
    CCFC_E_QUERY,
    CCFC_E_SET
} CallForwardOperationE;

typedef enum {
    CB_E_QUERY,
    CB_E_SET
} CallBarringOperationE;

typedef enum {
    CW_E_QUERY,
    CW_E_SET
} CallWaitingOperationE;

/*
 * 0    unconditional
 * 1    mobile busy
 * 2    no reply
 * 3    not reachable
 * 4    all call forwarding (refer 3GPP TS 22.030 [19])
 * 5    all conditional call forwarding (refer 3GPP TS 22.030 [19])
 */
typedef enum {
    CF_U         = 0,
    CF_BUSY      = 1,
    CF_NORPLY    = 2,
    CF_NOTREACH  = 3,
    CF_ALL       = 4,
    CF_ALLCOND   = 5,
    CF_NOTREGIST = 6
} CallForwardReasonE;

/*
 * status is:
 * 0 = disable
 * 1 = enable
 * 2 = interrogate
 * 3 = registeration
 * 4 = erasure
*/
typedef enum {
    SS_DEACTIVATE   = 0,    // disable
    SS_ACTIVATE     = 1,    // enable
    SS_INTERROGATE  = 2,    // interrogate
    SS_REGISTER     = 3,    // registeration
    SS_ERASE        = 4     // erasure
} SsStatusE;

typedef enum {
    HAS_NONE    = 0,
    HAS_SIA     = 1,
    HAS_SIB     = 2,
    HAS_SIC     = 4
} HasSIFlagE;

/*
 * Check if CBS data coding scheme is UCS2 3GPP 23.038
 *
 * Coding Group Use of bits 3..0
 * Bits 7..4
 *
 * 0000         Language using the GSM 7 bit default alphabet
 *
 *              Bits 3..0 indicate the language:
 *              0000 German
 *              0001 English
 *              0010 Italian
 *              0011 French
 *              0100 Spanish
 *              0101 Dutch
 *              0110 Swedish
 *              0111 Danish
 *              1000 Portuguese
 *              1001 Finnish
 *              1010 Norwegian
 *              1011 Greek
 *              1100 Turkish
 *              1101 Hungarian
 *              1110 Polish
 *              1111 Language unspecified
 *
 * 0001         0000 GSM 7 bit default alphabet; message preceded by language indication.
 *
 *                   The first 3 characters of the message are a two-character representation
 *                   of the language encoded according to ISO 639 [12], followed by a CR character.
 *                   The CR character is then followed by 90 characters of text.
 *
 *              0001 UCS2; message preceded by language indication
 *
 *                   The message starts with a two GSM 7-bit default alphabet character representation
 *                   of the language encoded according to ISO 639 [12]. This is padded to the octet
 *                   boundary with two bits set to 0 and then followed by 40 characters of UCS2-encoded message.
 *                   An MS not supporting UCS2 coding will present the two character language identifier followed
 *                   by improperly interpreted user data.
 *
 *              0010..1111 Reserved
 *
 * 0010..       0000 Czech
 *              0001 Hebrew
 *              0010 Arabic
 *              0011 Russian
 *              0100 Icelandic
 *
 *              0101..1111 Reserved for other languages using the GSM 7 bit default alphabet, with
 *                         unspecified handling at the MS
 *
 * 0011         0000..1111 Reserved for other languages using the GSM 7 bit default alphabet, with
 *                         unspecified handling at the MS
 *
 * 01xx         General Data Coding indication
 *              Bits 5..0 indicate the following:
 *
 *              Bit 5, if set to 0, indicates the text is uncompressed
 *              Bit 5, if set to 1, indicates the text is compressed using the compression algorithm defined in 3GPP TS 23.042 [13]
 *
 *              Bit 4, if set to 0, indicates that bits 1 to 0 are reserved and have no message class meaning
 *              Bit 4, if set to 1, indicates that bits 1 to 0 have a message class meaning:
 *
 *              Bit 1  Bit 0  Message Class:
 *              0      0      Class 0
 *              0      1      Class 1 Default meaning: ME-specific.
 *              1      0      Class 2 (U)SIM specific message.
 *              1      1      Class 3 Default meaning: TE-specific (see 3GPP TS 27.005 [8])
 *
 *              Bits 3 and 2 indicate the character set being used, as follows:
 *              Bit 3  Bit 2  Character set:
 *              0      0      GSM 7 bit default alphabet
 *              0      1      8 bit data
 *              1      0      UCS2 (16 bit) [10]
 *              1      1      Reserved
 *
 *  1000        Reserved coding groups
 *
 *  1001        Message with User Data Header (UDH) structure:
 *
 *              Bit 1  Bit 0  Message Class:
 *              0      0      Class 0
 *              0      1      Class 1 Default meaning: ME-specific.
 *              1      0      Class 2 (U)SIM specific message.
 *              1      1      Class 3 Default meaning: TE-specific (see 3GPP TS 27.005 [8])
 *
 *              Bits 3 and 2 indicate the alphabet being used, as follows:
 *              Bit 3  Bit 2  Alphabet:
 *              0      0      GSM 7 bit default alphabet
 *              0      1      8 bit data
 *              1      0      USC2 (16 bit) [10]
 *              1      1      Reserved
 *
 *  1010..1101  Reserved coding groups
 *
 *  1101        l1 protocol message defined in 3GPP TS 24.294[19]
 *
 *  1110        Defined by the WAP Forum [15]
 *
 *  1111        Data coding / message handling
 *
 *              Bit 3 is reserved, set to 0.
 *
 *              Bit 2  Message coding:
 *              0      GSM 7 bit default alphabet
 *              1      8 bit data
 *
 *              Bit 1  Bit 0  Message Class:
 *              0      0      No message class.
 *              0      1      Class 1 user defined.
 *              1      0      Class 2 user defined.
 *              1      1      Class 3
 *              default meaning: TE specific
 *              (see 3GPP TS 27.005 [8])
*/
typedef enum {
    DCS_GSM7,
    DCS_8BIT,
    DCS_UCS2,
    MAX_DCS_SUPPORT
} GsmCbsDcsE;

/*
 * 0   unconditional call forwarding is active
 * 1   some of the conditional call forwardings are active
 * 2   call has been forwarded
 * 3   call is waiting
 * 4   this is a CUG call (also <index> present)
 * 5   outgoing calls are barred
 * 6   incoming calls are barred
 * 7   CLIR suppression rejected
 * 8   call has been deflected
*/
typedef enum {
    CODE_IMS_MO_UNCONDITIONAL_CF_ACTIVE = 100,
    CODE_IMS_MO_SOME_CF_ACTIVE,              //101
    CODE_IMS_MO_CALL_FORWARDED,              //102
    CODE_IMS_MO_CALL_IS_WAITING,             //103
    CODE_IMS_MO_CUG_CALL,                    //104
    CODE_IMS_MO_OUTGOING_CALLS_BARRED,       //105
    CODE_IMS_MO_INCOMING_CALLS_BARRED,       //106
    CODE_IMS_MO_CLIR_SUPPRESSION_REJECTED,   //107
    CODE_IMS_MO_CALL_DEFLECTED               //108
} NotificationCodeImsMoE;

/*
 * 0   this is a forwarded call (MT call setup)
 * 1   this is a CUG call (also <index> present) (MT call setup)
 * 2   call has been put on hold (during a voice call)
 * 3   call has been retrieved (during a voice call)
 * 4   multiparty call entered (during a voice call)
 * 5   call on hold has been released (this is not a SS notification) (during a voice call)
 * 6   forward check SS message received (can be received whenever)
 * 7   call is being connected (alerting) with the remote party in alerting state in explicit call transfer operation (during a voice call)
 * 8   call has been connected with the other remote party in explicit call transfer operation (also number and subaddress parameters may be present) (during a voice call or MT call setup)
 * 9   this is a deflected call (MT call setup)
 * 10  additional incoming call forwarded
 * 11  MT is a forwarded call (CF)
 * 12  MT is a forwarded call (CFU)
 * 13  MT is a forwarded call (CFC)
 * 14  MT is a forwarded call (CFB)
 * 15  MT is a forwarded call (CFNRy)
 * 16  MT is a forwarded call (CFNRc)
*/
typedef enum {
    CODE_IMS_MT_FORWARDED_CALL = 100,
    CODE_IMS_MT_CUG_CALL,                   //101
    CODE_IMS_MT_CALL_ON_HOLD,               //102
    CODE_IMS_MT_CALL_RETRIEVED,             //103
    CODE_IMS_MT_MULTI_PARTY_CALL,           //104
    CODE_IMS_MT_ON_HOLD_CALL_RELEASED,      //105
    CODE_IMS_MT_FORWARD_CHECK_RECEIVED,     //106
    CODE_IMS_MT_CALL_CONNECTING_ECT,        //107
    CODE_IMS_MT_CALL_CONNECTED_ECT,         //108
    CODE_IMS_MT_DEFLECTED_CALL,             //109
    CODE_IMS_MT_ADDITIONAL_CALL_FORWARDED,  //110
    CODE_IMS_MT_FORWARDED_CF,               //111
    CODE_IMS_MT_FORWARDED_CF_UNCOND,        //112
    CODE_IMS_MT_FORWARDED_CF_COND,          //113
    CODE_IMS_MT_FORWARDED_CF_BUSY,          //114
    CODE_IMS_MT_FORWARDED_CF_NO_REPLY,      //115
    CODE_IMS_MT_FORWARDED_CF_NOT_REACHABLE  //116
} NotificationCodeImsMtE;

#define SS_OP_DEACTIVATION     "#"
#define SS_OP_ACTIVATION       "*"
#define SS_OP_INTERROGATION    "*#"
#define SS_OP_REGISTRATION     "**"
#define SS_OP_ERASURE          "##"

#define BS_ALL                   ""
#define BS_TELE_ALL              "10"
#define BS_TELEPHONY             "11"
#define BS_TELE_DATA_ALL         "12"
#define BS_TELE_FAX              "13"
#define BS_TELE_SMS              "16"
#define BS_TELE_VGCS             "17" /* Not supported by framework */
#define BS_TELE_VBS              "18" /* Not supported by framework */
#define BS_TELE_ALL_EXCEPT_SMS   "19"
#define BS_DATA_ALL              "20"
#define BS_DATA_ASYNC_ALL        "21"
#define BS_DATA_SYNC_ALL         "22"
#define BS_DATA_CIRCUIT_SYNC     "24" /* This is also for VT call */
#define BS_DATA_CIRCUIT_ASYNC    "25"
#define BS_DATA_SYNC_TELE        "26" /* Supported by framework */
#define BS_GPRS_ALL              "99"

#define CALL_FORWAED_NONE               ""
#define CALL_FORWARD_UNCONDITIONAL      "21"
#define CALL_FORWARD_BUSY               "67"
#define CALL_FORWARD_NOREPLY            "61"
#define CALL_FORWARD_NOT_REACHABLE      "62"
#define CALL_FORWARD_ALL                "002"
#define CALL_FORWARD_ALL_CONDITIONAL    "004"
#define CALL_FORWARD_NOT_REGISTERED     "68"

#define TYPE_ADDRESS_INTERNATIONAL 145

#define SS_CHANNEL_CTX getRILChannelCtxFromToken(t)

/* PROPERTY_TBCW_MODE is the mode for the terminal-based call waiting. */
#define PROPERTY_TBCW_MODE "persist.vendor.radio.terminal-based.cw"
#define TBCW_DISABLED "disabled_tbcw"
#define TBCW_ON       "enabled_tbcw_on"
#define TBCW_OFF      "enabled_tbcw_off"
#define TBCW_ON_VOLTE_ONLY  "tbcw_on_volte_only"
#define TBCW_OFF_VOLTE_ONLY "tbcw_off_volte_only"

#define TBCW_VOLTE_ONLY   1
#define TBCW_VOLTE_AND_CS 0

/* PROPERTY_TBCLIR_x is the mode for the terminal-based CLIR. */
#define PROPERTY_TBCLIR_N "persist.vendor.radio.terminal-based.clir_n"
#define PROPERTY_TBCLIR_M "persist.vendor.radio.terminal-based.clir_m"

#define PROPERTY_ERROR_MESSAGE_FROM_XCAP "vendor.gsm.radio.ss.errormsg"
#define PROPERTY_MODEM_VERSION "persist.vendor.ss.modem_version"

#endif /* SUPP_SERV_DEF */
