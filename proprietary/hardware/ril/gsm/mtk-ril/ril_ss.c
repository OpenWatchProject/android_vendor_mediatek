/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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
#include <libmtkrilutils.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <alloca.h>
#include "atchannels.h"
#include "at_tok.h"
#include "misc.h"
#include "sysenv_utils.h"
#include <getopt.h>
#include <sys/socket.h>
#include <cutils/sockets.h>
#include <cutils/properties.h>
#include <termios.h>
#include <ril_callbacks.h>
#include <ril_gsm_util.h>  //for UCS2 de/coding

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "RIL-SS"


#include <log/log.h>


typedef enum {
    CCFC_E_QUERY,
    CCFC_E_SET
} CallForwardOperationE;

/*
 * 0    unconditional
 * 1    mobile busy
 * 2    no reply
 * 3    not reachable
 * 4    all call forwarding (refer 3GPP TS 22.030 [19])
 * 5    all conditional call forwarding (refer 3GPP TS 22.030 [19])
 */
typedef enum {
    CF_U        = 0,
    CF_BUSY     = 1,
    CF_NORPLY   = 2,
    CF_NOTREACH = 3,
    CF_ALL      = 4,
    CF_ALLCOND  = 5
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

typedef enum {
    USSI_REQUEST  = 1,  // UE initial USSI request
    USSI_RESPONSE = 2   // Response network USSI
} UssiAction;

typedef enum {
    SEND_NOTHING_BACK  = 0,  // No need to send anything back to framework
    SEND_RESPONSE_BACK = 1,  // Send USSD Response back to framework
    SEND_URC_BACK      = 2   // Send USSD URC back to framework
} UssdReportCase;

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

#define CRSS_CALL_WAITING             0
#define CRSS_CALLED_LINE_ID_PREST     1
#define CRSS_CALLING_LINE_ID_PREST    2
#define CRSS_CONNECTED_LINE_ID_PREST  3

#define TYPE_ADDRESS_INTERNATIONAL 145

#define SS_CHANNEL_CTX getRILChannelCtxFromToken(t)

/***
 * "AO"  BAOC (Barr All Outgoing Calls) (refer 3GPP TS 22.088 [6] clause 1)
 * "OI"  BOIC (Barr Outgoing International Calls) (refer 3GPP TS 22.088 [6] clause 1)
 * "OX"  BOIC exHC (Barr Outgoing International Calls except to Home Country) (refer 3GPP TS 22.088 [6] clause 1)
 * "AI"  BAIC (Barr All Incoming Calls) (refer 3GPP TS 22.088 [6] clause 2)
 * "IR"  BIC Roam (Barr Incoming Calls when Roaming outside the home country) (refer 3GPP TS 22.088 [6] clause 2)
 * "AB"  All Barring services (refer 3GPP TS 22.030 [19]) (applicable only for <mode>=0)
 * "AG"  All outGoing barring services (refer 3GPP TS 22.030 [19]) (applicable only for <mode>=0)
 * "AC"  All inComing barring services (refer 3GPP TS 22.030 [19]) (applicable only for <mode>=0)
 */
const char *callBarFacilityStrings[CB_SUPPORT_NUM] = {
    "AO",
    "OI",
    "OX",
    "AI",
    "IR",
    "AB",
    "AG",
    "AC"
};

const char *callBarServiceCodeStrings[CB_SUPPORT_NUM] = {
    "33",
    "331",
    "332",
    "35",
    "351",
    "330",
    "333",
    "353"
};

static const char *GsmCbsDcsStringp[MAX_DCS_SUPPORT] = {"GSM7", "8BIT", "UCS2"};
static const struct timeval TIMEVAL_0S = {0, 0};
static int isECMCCSSReceived = 0;
static int isCRINGReceived = 0;
static RIL_SuppSvcNotification tmpSvcNotify;
static RIL_SOCKET_ID tmpSvcNotifyRid;

extern int callWaiting;

/// USSD usage {
static void requestSendUssdDomainSelect(int request, void *data, size_t datalen, RIL_Token t);
static void requestCancelUssdDomainSelect(int request, void *data, size_t datalen, RIL_Token t);
static void handleOnUssi(const char **data, size_t datalen, RIL_SOCKET_ID rid);
static void handleUssiCSFB(void *param);
static int convertUssiToUssdMode(const char **data, size_t datalen, RIL_SOCKET_ID rid);
static void sendFailureReport(int request, void *data, size_t datalen,
        RIL_Token t, RIL_Errno ret);

// USSI utility functions
static UssiAction getUssiAction();
static void setUssiAction(UssiAction action);
static const char *ussiActionToString(UssiAction action);

// USSI preconditions
static int isFdnAllowed(const char* ussd, RIL_SOCKET_ID rid);
static int isVopsOn(RIL_SOCKET_ID rid);
static int isImsRegOn(RIL_SOCKET_ID rid);
static int isInImsCall(RIL_SOCKET_ID rid);
static int isWfcRegOn(RIL_SOCKET_ID rid);

static char *encryptString(char* input);

extern inCallNumberPerSim[MAX_SIM_COUNT];   // in ril_cc.h
extern dispatch_flag;  // in ril_ims_cc.h

// Java framework has the ability to know which action it should take, then
// we can pass correct EIUSD's argument to IMS Stack
// However, if USSI comes from GSM way, there is no dirtect information for RIL SS to
// determine that, so we need a state variable to remember it.
// If we get "further user action required" (<n>=1, <m>=1) from USSI URC,
// we set this state variable to USSI_RESPONSE. It means current USSI session is
// interactive, AP has to tell IMS stack it's not a request, it's a response.
// Otherwirse, we treat the action as USSI_REQUEST by default.
static UssiAction mUssiAction = USSI_REQUEST;

// To cancel correct USSD session (CS or IMS), need a flag to memorize
// which domain it is for current ongoing USSD session
// 0: CS,  1: IMS
static int mOngoingSessionDomain = 0;

// Snapshot the USSI string. If we receive error from USSI URC, need to do USSD CSFB
// using the same string
static char *mUssiSnapshot = NULL;

RIL_Token mUssiToken = NULL;
/// USSD usage }

static const char *ssStatusToOpCodeString(SsStatusE status)
{
    /**
     *   Activation:    *SC*SI#
     *   Deactivation:  #SC*SI#
     *   Interrogation: *#SC*SI#
     *   Registration:  *SC*SI# and **SC*SI#
     *   Erasure:       ##SC*SI#
     */
    switch (status) {
        case SS_ACTIVATE:
            return SS_OP_ACTIVATION;
            break;
        case SS_DEACTIVATE:
            return SS_OP_DEACTIVATION;
            break;
        case SS_INTERROGATE:
            return SS_OP_INTERROGATION;
            break;
        case SS_REGISTER:
            return SS_OP_REGISTRATION;
            break;
        case SS_ERASE:
            return SS_OP_ERASURE;
            break;
        default:
            assert(0);
            break;
    }

    return "";
}


/* This table shall be sync with siToServiceClass() in GsmMmiCode.java */
const char *InfoClassToMmiBSCodeString (int infoClass)
{
    /**
     * Basic Service
     * group number (note)  Telecommunication Service       MMI Service Code
     *
     * 1 to 12              All tele and bearer services    no code required
     *
     *                      Teleservices
     * 1 to 6, 12           All teleservices                10
     * 1                    Telephony                       11
     * 2 to 6               All data teleservices           12
     * 6                    Facsimile services              13
     * 2                    Short Message Services          16
     * 1, 3 to 6, 12        All teleservices except SMS     19
     * 12                   Voice group services
     *                      Voice Group Call Service (VGCS) 17
     *                      Voice Broadcast Service (VBS)   18
     *
     *                      Bearer Service
     * 7 to 11              All bearer services             20
     * 7                    All async services              21
     * 8                    All sync services               22
     * 8                    All data circuit sync           24
     * 7                    All data circuit async          25
     * 13                   All GPRS bearer services        99
     */

    switch(infoClass)
    {
        case CLASS_NONE:
            return BS_ALL;
            break;
        case CLASS_VOICE:
            return BS_TELEPHONY;
            break;
        case (CLASS_DATA_ASYNC | CLASS_DATA_SYNC):
            return BS_DATA_ALL;
            break;
        case CLASS_FAX:
            return BS_TELE_FAX;
            break;
        case CLASS_SMS:
            return BS_TELE_SMS;
            break;
        case (CLASS_VOICE | CLASS_SMS | CLASS_FAX):
            return BS_TELE_ALL;
            break;
        case (CLASS_SMS | CLASS_FAX):
            return BS_TELE_DATA_ALL;
            break;
        case (CLASS_VOICE | CLASS_FAX):
            return BS_TELE_ALL_EXCEPT_SMS;
            break;
        case CLASS_DATA_SYNC:
            return BS_DATA_CIRCUIT_SYNC;
            break;
        case CLASS_DATA_ASYNC:
            return BS_DATA_CIRCUIT_ASYNC;
            break;
        case (CLASS_DATA_SYNC | CLASS_DEDICATED_PACKET_ACCESS):
            return BS_DATA_SYNC_ALL;
            break;
        case (CLASS_DATA_ASYNC | CLASS_DEDICATED_PAD_ACCESS):
            return BS_DATA_ASYNC_ALL;
            break;
        case (CLASS_DATA_SYNC | CLASS_VOICE):
            return BS_DATA_SYNC_TELE;
            break;
        case CLASS_DEDICATED_PACKET_ACCESS:
            return BS_GPRS_ALL;
            break;
        case (CLASS_MTK_VIDEO | CLASS_DATA_SYNC):
            return BS_DATA_CIRCUIT_SYNC;
            break;
        case CLASS_MTK_VIDEO:
            return BS_DATA_CIRCUIT_SYNC;
            break;
        default:
            LOGE("RILD unknown infoClass: %d", infoClass);
            break;
    }
    return "";
}

static int MmiBSCodeToInfoClassX (int serviceCode)
{
    if (isLteSupport()) {
        LOGD("[MmiBSCodeToInfoClassX]Return %d directly.", serviceCode);
        return serviceCode;
    } else {
        switch (serviceCode) {
            /* BS_ALL_E = BS_TELE_ALL_E + BS_DATA_ALL_E */
            case BS_ALL_E:
                return (int) (CLASS_SMS + CLASS_FAX + CLASS_VOICE + CLASS_DATA_ASYNC + CLASS_DATA_SYNC);
            case BS_TELE_ALL_E:
                return (int) (CLASS_SMS + CLASS_FAX + CLASS_VOICE);
            case BS_TELEPHONY_E:
                return (int) CLASS_VOICE;
            case BS_TELE_DATA_ALL_E:
                return (int) (CLASS_SMS + CLASS_FAX);
            case BS_TELE_FAX_E:
                return (int) CLASS_FAX;
            case BS_TELE_SMS_E:
                return (int) CLASS_SMS;
            case BS_TELE_ALL_EXCEPT_SMS_E:
                return (int) (CLASS_FAX + CLASS_VOICE);
            /**
             * Note for code 20:
             * From TS 22.030 Annex C:
             *   "All GPRS bearer services" are not included in "All tele and bearer services"
             *   and "All bearer services"."
             *   So SERVICE_CLASS_DATA, which (according to 27.007) includes GPRS
             */
            case BS_DATA_ALL_E:
                return (int)(CLASS_DATA_ASYNC + CLASS_DATA_SYNC);
            case BS_DATA_ASYNC_ALL_E:
                return (int)(CLASS_DEDICATED_PAD_ACCESS + CLASS_DATA_ASYNC);
            case BS_DATA_SYNC_ALL_E:
                return (int)(CLASS_DEDICATED_PACKET_ACCESS + CLASS_DATA_SYNC);
            case BS_DATA_CIRCUIT_SYNC_E:
                return (int)(CLASS_DATA_SYNC + CLASS_MTK_VIDEO); /* Also for video call */
            case BS_DATA_CIRCUIT_ASYNC_E:
                return (int) CLASS_DATA_ASYNC;
            case BS_DATA_SYNC_TELE_E:
                return (int)(CLASS_DATA_SYNC + CLASS_VOICE);
            case BS_GPRS_ALL_E:
                return (int) CLASS_DEDICATED_PACKET_ACCESS;
            default:
                return (int) CLASS_NONE;
        }
    }
}


static const char *callForwardReasonToServiceCodeString(CallForwardReasonE cfreason)
{
    switch (cfreason) {
        case CF_U:
            return CALL_FORWARD_UNCONDITIONAL;
            break;
        case CF_BUSY:
            return CALL_FORWARD_BUSY;
            break;
        case CF_NORPLY:
            return CALL_FORWARD_NOREPLY;
            break;
        case CF_NOTREACH:
            return CALL_FORWARD_NOT_REACHABLE;
            break;
        case CF_ALL:
            return CALL_FORWARD_ALL;
            break;
        case CF_ALLCOND:
            return CALL_FORWARD_ALL_CONDITIONAL;
            break;
        default:
            return CALL_FORWAED_NONE;
            break;
    }
}


static GsmCbsDcsE
checkCbsDcs(int dcs)
{
    GsmCbsDcsE result = DCS_GSM7;

    if ((dcs == 0x11) ||((dcs & 0x4C) == 0x48) ||((dcs & 0x9C) == 0x98)) {
        result = DCS_UCS2;
    } else if (((dcs & 0x4C) == 0x44) ||((dcs & 0x9C) == 0x94) ||((dcs & 0xF4) == 0xF4)) {
        result = DCS_8BIT;
    }

    return result;
}

static char *convertToUCS2(char *data)
{
    /** USSD messages using the default alphabet are coded with the
       * GSM 7-bit default alphabet  given in clause 6.2.1. The message can then consist of
       * up to 182 user characters (3GPP 23.038).
       * Callee expects const char * in UCS2 Hex decimal format.
      */
    #define MAX_RIL_USSD_STRING_LENGTH 255

    size_t  stringlen     = 0;
    bytes_t ucs2String    = NULL;
    bytes_t ucs2HexString = NULL;

    // data equals string8, since it passed by dispatchString
    const char *string8 = (const char *)(data);

    ucs2String = (bytes_t) calloc(2*(MAX_RIL_USSD_STRING_LENGTH+1), sizeof(char));
    if (ucs2String == NULL) {
        LOGE("convertToUCS2:ucs2String malloc fail");
        return NULL;
    }
    // memory overwrite if strlen(string8) is larger than MAX_RIL_USSD_STRING_LENGTH
    stringlen = utf8_to_ucs2((cbytes_t)string8, MIN(strlen(string8),
            MAX_RIL_USSD_STRING_LENGTH), ucs2String);
    ucs2HexString = (bytes_t) calloc(2*stringlen*2+1, sizeof(char));
    if (ucs2HexString == NULL) {
        LOGE("convertToUCS2:ucs2HexString malloc fail");
        // free here in the case of ucs2HexString is null.
        free(ucs2String);
        return NULL;
    }

    gsm_hex_from_bytes((char *)ucs2HexString, ucs2String, 2*stringlen);

    free(ucs2String);

    LOGW("convertToUCS2= %s", (char *) ucs2HexString);

    return (char *) ucs2HexString;
}

/**
 * RIL_REQUEST_SEND_USSD
 *
 * Send a USSD message
 *
 * If a USSD session already exists, the message should be sent in the
 * context of that session. Otherwise, a new session should be created.
 *
 * The network reply should be reported via RIL_UNSOL_ON_USSD
 *
 * Only one USSD session may exist at a time, and the session is assumed
 * to exist until:
 *   a) The android system invokes RIL_REQUEST_CANCEL_USSD
 *   b) The implementation sends a RIL_UNSOL_ON_USSD with a type code
 *      of "0" (USSD-Notify/no further action) or "2" (session terminated)
 *
 * "data" is a const char * containing the USSD request in UTF-8 format
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  FDN_CHECK_FAILURE
 *  GENERIC_FAILURE
 *
 * See also: RIL_REQUEST_CANCEL_USSD, RIL_UNSOL_ON_USSD
 */
static void requestSendUSSD(int request, void *data, size_t datalen, RIL_Token t,
        UssdReportCase reportCase)
{
    UNUSED(datalen);
    const char* p_ussdRequest = convertToUCS2(data);
    ATResponse* p_response = NULL;
    int err;
    char* cmd = NULL;
    RIL_Errno ret = RIL_E_INTERNAL_ERR;
    int strLen = 0;
    char* pTmpStr = NULL;
    RILChannelCtx *pChannel = SS_CHANNEL_CTX;
    if (reportCase == SEND_URC_BACK) {
        pChannel = getChannelCtxbyProxy();
    }

    if (p_ussdRequest == NULL) {
        LOGE("requestSendUSSD: p_ussdRequest malloc fail");
        goto error;
    }

    /**
     * AT+ECUSD=<m>, <n>, <str>, <dcs>
     * <m>: 1 for SS, 2 for USSD
     * <n>: 1 for execute SS or USSD, 2 for cancel USSD session
     * <str>: string type parameter, the SS or USSD string
     */

    /**
     * 01xx    General Data Coding indication
     *
     * Bits 5..0 indicate the following:
     *   Bit 5, if set to 0, indicates the text is uncompressed
     *   Bit 5, if set to 1, indicates the text is compressed using the compression algorithm defined in 3GPP TS 23.042 [13]
     *
     *   Bit 4, if set to 0, indicates that bits 1 to 0 are reserved and have no message class meaning
     *   Bit 4, if set to 1, indicates that bits 1 to 0 have a message class meaning:
     *
     *     Bit 1   Bit 0       Message Class:
     *       0       0           Class 0
     *       0       1           Class 1 Default meaning: ME-specific.
     *       1       0           Class 2 (U)SIM specific message.
     *       1       1           Class 3 Default meaning: TE-specific (see 3GPP TS 27.005 [8])
     *
     *   Bits 3 and 2 indicate the character set being used, as follows:
     *
     *     Bit 3   Bit 2       Character set:
     *       0       0           GSM 7 bit default alphabet
     *       0       1           8 bit data
     *       1       0           UCS2 (16 bit) [10]
     *       1       1           Reserved
     */
    //BEGIN mtk08470 [20130109][ALPS00436983]
    // USSD string cannot more than MAX_RIL_USSD_NUMBER_LENGTH digits
    // We convert input char to unicode hex string and store it to p_ussdRequest.
    // For example, convert input "1" to "3100"; So len of p_ussdRequest is 4 times of input
    strLen = strlen(p_ussdRequest)/4;
    if (strLen > MAX_RIL_USSD_NUMBER_LENGTH) {
        LOGW("USSD stringlen = %d, max = %d", strLen, MAX_RIL_USSD_NUMBER_LENGTH);
        strLen = MAX_RIL_USSD_NUMBER_LENGTH;
    }
    pTmpStr = calloc(1, (4*strLen+1));
    if (pTmpStr == NULL) {
        LOGE("Malloc fail");
        goto error;
    }
    memcpy(pTmpStr, p_ussdRequest, 4*strLen);
    /* END mtk08470 [20130109][ALPS00436983] */
    asprintf(&cmd, "AT+ECUSD=2,1,\"%s\",72", pTmpStr); /* <dcs> = 0x48 */

    err = at_send_command(cmd, &p_response, pChannel);

    free(cmd);
    free(pTmpStr);

    if (err < 0 || NULL == p_response) {
        LOGE("requestSendUSSD Fail");
        goto error;
    }

    switch (at_get_cme_error(p_response)) {
        case CME_SUCCESS:
            ret = RIL_E_SUCCESS;
            break;
        case CME_CALL_BARRED:
        case CME_OPR_DTR_BARRING:
            ret = RIL_E_OEM_ERROR_1;  // CALL_BARRED;
            break;
        case CME_PHB_FDN_BLOCKED:
            ret = RIL_E_FDN_CHECK_FAILURE;
            break;
        default:
            at_send_command("AT+ECUSD=2,2", NULL, pChannel);
            break;
    }

error:
    at_response_free(p_response);
    if (reportCase == SEND_RESPONSE_BACK) {
        LOGD("requestSendUSSD: send response back to framework");
        if (ret == RIL_E_SUCCESS) {
            RIL_onRequestComplete(t, ret, NULL, 0);
        } else {
            sendFailureReport(request, data, datalen, t, ret);
        }
    } else if (reportCase == SEND_URC_BACK) {
        if (ret != RIL_E_SUCCESS) {
            LOGD("requestSendUSSD: send urc back to framework");

            // Generate a generic failure USSD URC
            char *genericUssdFail[2] = {(char *) "4", (char *) ""};
            RIL_onUnsolicitedResponse(
                    RIL_UNSOL_ON_USSD,
                    genericUssdFail,
                    2 * sizeof(char *),
                    getRILIdByChannelCtx(SS_CHANNEL_CTX));
        } else {
            LOGD("requestSendUSSD: no need to send anything to framework, waiting CS USSD URC");
        }
    } else {
        LOGD("requestSendUSSD: no need to send anything to framework");
    }
}


/**
 * RIL_REQUEST_CANCEL_USSD
 *
 * Cancel the current USSD session if one exists
 *
 * "data" is null
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 */
static void requestCancelUssd(void *data, size_t datalen, RIL_Token t)
{
    UNUSED(datalen);
    UNUSED(data);
    ATResponse *p_response = NULL;
    RILChannelCtx *rilChannel = getRILChannelCtxFromToken(t);
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(rilChannel);
    SIM_Status simState = getSIMStatus(rid);
    int err;

    /* MD still returns OK when SIM is absent. We need to directy send error code back
     * to framework in this kind of situation, or it cannot pass the VTS test case.
     */
    if (simState == SIM_ABSENT) {
        LOGD("requestCancelUssd, SIM is absent, skip");
        RIL_onRequestComplete(t, RIL_E_INTERNAL_ERR, NULL, 0);
        return;
    }

    /**
     * AT+ECUSD=<m>, <n>, <str>
     * <m>: 1 for SS, 2 for USSD
     * <n>: 1 for execute SS or USSD, 2 for cancel USSD session
     * <str>: string type parameter, the SS or USSD string
     */

    err = at_send_command("AT+ECUSD=2,2", &p_response, SS_CHANNEL_CTX);

    if (err < 0 || p_response->success == 0) {
        RIL_onRequestComplete(t, RIL_E_INTERNAL_ERR, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
    at_response_free(p_response);
}


static void requestClirOperation(void *data, size_t datalen, RIL_Token t)
{
    int* n = (int *) data;
    ATResponse* p_response = NULL;
    int err;
    char* cmd = NULL;
    char* line = NULL;
    RIL_Errno ret = RIL_E_INTERNAL_ERR;
    int response[2] = {0};

    if (datalen != 0) {
        /**
         * Set CLIR: +CLIR=[<n>]
         * "data" is int *
         * ((int *)data)[0] is "n" parameter from TS 27.007 7.7
         *  <n> (parameter sets the adjustment for outgoing calls)
         */
        asprintf(&cmd, "AT+CLIR=%d", n[0]);
        err = at_send_command(cmd, &p_response, SS_CHANNEL_CTX);
        free(cmd);
    } else {
        /**
         * Get CLIR: +CLIR?
         * This action will trigger CLIR interrogation. Need to check FDN so use proprietary command
         */

        /**
         * AT+ECUSD=<m>, <n>, <str>
         * <m>: 1 for SS, 2 for USSD
         * <n>: 1 for execute SS or USSD, 2 for cancel USSD session
         * <str>: string type parameter, the SS or USSD string
         */
        err = at_send_command_singleline("AT+ECUSD=1,1,\"*#31#\"", "+CLIR:", &p_response, SS_CHANNEL_CTX);
    }

    if (err < 0 || NULL == p_response) {
        LOGE("requestClirOperation Fail");
        goto error;
    }

    switch (at_get_cme_error(p_response)) {
        case CME_SUCCESS:
            break;
        case CME_CALL_BARRED:
        case CME_OPR_DTR_BARRING:
            ret = RIL_E_OEM_ERROR_1;  // CALL_BARRED;
            goto error;
        case CME_PHB_FDN_BLOCKED:
            ret = RIL_E_FDN_CHECK_FAILURE;
            goto error;
        default:
            goto error;
    }

    /* For Get CLIR only */
    if (p_response->p_intermediates != NULL) {
        line = p_response->p_intermediates->line;

        assert(line);

        if (at_tok_start(&line) < 0) {
            goto error;
        }

        /**
         * <n> parameter sets the adjustment for outgoing calls
         * 0   presentation indicator is used according to the subscription of the CLIR service
         * 1   CLIR invocation
         * 2   CLIR suppression
         */
        if (at_tok_nextint(&line, &response[0]) < 0) {
            goto error;
        }

        /**
         * <m> parameter shows the subscriber CLIR service status in the network
         * 0   CLIR not provisioned
         * 1   CLIR provisioned in permanent mode
         * 2   unknown (e.g. no network, etc.)
         * 3   CLIR temporary mode presentation restricted
         * 4   CLIR temporary mode presentation allowed
         */
        if (at_tok_nextint(&line, &response[1]) < 0) {
           goto error;
        }
    }

    /* return success here */
    ret = RIL_E_SUCCESS;

error:
    /* For SET CLIR responseVoid will ignore the responses */
    RIL_onRequestComplete(t, ret, response, 2 * sizeof(int));
    at_response_free(p_response);
}


/**
 * RIL_REQUEST_GET_CLIR
 *
 * Gets current CLIR status
 * "data" is NULL
 * "response" is int *
 * ((int *)data)[0] is "n" parameter from TS 27.007 7.7
 * ((int *)data)[1] is "m" parameter from TS 27.007 7.7
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 */
static void requestGetClir(void *data, size_t datalen, RIL_Token t)
{
    requestClirOperation(data, datalen, t);
}


/**
 * RIL_REQUEST_SET_CLIR
 *
 * "data" is int *
 * ((int *)data)[0] is "n" parameter from TS 27.007 7.7
 *
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 */
static void requestSetClir(void *data, size_t datalen, RIL_Token t)
{
    requestClirOperation(data, datalen, t);
}


static void requestCallForwardOperation(void *data, size_t datalen,
        RIL_Token t, CallForwardOperationE op)
{
    UNUSED(datalen);
    RIL_CallForwardInfo *p_args = (RIL_CallForwardInfo*) data;
    ATResponse *p_response = NULL;
    int err;
    char *cmd = NULL;
    char *precmd = NULL;
    ATLine *p_cur = NULL;
    RIL_Errno ret = RIL_E_INTERNAL_ERR;
    RIL_CallForwardInfo **pp_CfInfoResponses = NULL;
    RIL_CallForwardInfo *p_CfInfoResponse = NULL;
    HasSIFlagE eSiStatus = HAS_NONE;
    char *pStrTmp = NULL;
    int resLength = 0;
    int dnlen = 0;
    int serviceClass = 0;

    /**
     * AT+ECUSD=<m>, <n>, <str>
     * <m>: 1 for SS, 2 for USSD
     * <n>: 1 for execute SS or USSD, 2 for cancel USSD session
     * <str>: string type parameter, the SS or USSD string
     */

    /**
     *                      SC  SIA  SIB  SIC
     * CFU                  21  DN   BS   -
     * CF Busy              67  DN   BS   -
     * CF No Reply          61  DN   BS   T
     * CF Not Reachable     62  DN   BS   -
     * All CF               002 DN   BS   T
     * All conditional CF   004 DN   BS   T
     */

    /**
     * 3GPP 24.082 and 3GPP 24.030
     * Registration         **SC*DN*BS(*T)#
     * Erasure              ##SC**BS#
     * Activation           *SC**BS#
     * Deactivation         #SC**BS#
     * Interrogation        *#SC**BS#
     */
    if ((CCFC_E_QUERY == op) && (p_args->reason >= CF_ALL)) {
        LOGE("CF_ALL & CF_ALLCOND cannot be used in QUERY");
        goto error;
    }

    /* For Query Call Forwarding in O version, RILJ doesn't assign cf.status. The default value of
     * cf.status is 0, which means Deactivation in SsStatusE. We need chaneg it to Interrogation.
     */
    if (CCFC_E_QUERY == op) {
        LOGW("Call Forwarding: change DEACTIVATE to INTERROGATE");
        p_args->status = SS_INTERROGATE;
    }

    if ((p_args->number != NULL) && (p_args->status == SS_ACTIVATE)) {
        LOGW("Call Forwarding: change ACTIVATE to REGISTER");
        p_args->status = SS_REGISTER;
    }

    /* Check Op Code and MMI Service Code */
    asprintf(&cmd, "AT+ECUSD=1,1,\"%s%s",
            ssStatusToOpCodeString((SsStatusE) p_args->status),
            callForwardReasonToServiceCodeString((CallForwardReasonE) p_args->reason));

    precmd = cmd;

    /* Check SIA: Dial number. Only Registration need to pack DN and others are ignored. */
    if ((p_args->number != NULL) &&
        ((p_args->status == SS_REGISTER) || (p_args->status == SS_ACTIVATE))) {

        eSiStatus |= HAS_SIA;
        dnlen = strlen((const char *) p_args->number);
        /* BEGIN mtk08470 [20130109][ALPS00436983] */
        /* number string cannot more than MAX_RIL_USSD_NUMBER_LENGTH digits */
        if (dnlen > MAX_RIL_USSD_NUMBER_LENGTH) {
            LOGE("cur number len = %d, max = %d", dnlen, MAX_RIL_USSD_NUMBER_LENGTH);
            free(precmd);
            goto error;
        }
        /* END mtk08470 [20130109][ALPS00436983] */

        if ((p_args->toa == TYPE_ADDRESS_INTERNATIONAL) && (strncmp((const char *)p_args->number, "+", 1))) {
            asprintf(&cmd, "%s*+%s", precmd, p_args->number);
            dnlen++;
        } else {
            asprintf(&cmd, "%s*%s", precmd, p_args->number);
        }
        const char *tmpNumber = encryptString(p_args->number);
        LOGD("toa:%d, number:%s, len:%d", p_args->toa, tmpNumber, dnlen);

        free((char*)tmpNumber);
        free(precmd);
        precmd = cmd;
    } else {
        if ((p_args->number == NULL) && (p_args->status == SS_REGISTER)) {
            LOGE("Call Forwarding Error: Address cannot be NULL in registration!");
            free(cmd);
            goto error;
        }
    }

    /* Check SIB: Basic Sevice Group */
    if (p_args->serviceClass != 0) {
        if (eSiStatus == HAS_SIA) {
            asprintf(&cmd, "%s*%s", precmd, InfoClassToMmiBSCodeString(p_args->serviceClass));
        } else {
            asprintf(&cmd, "%s**%s", precmd, InfoClassToMmiBSCodeString(p_args->serviceClass));
        }

        eSiStatus |= HAS_SIB;
        serviceClass = p_args->serviceClass;
        LOGD("Reserve serviceClass. serviceClass = %d", serviceClass);
        LOGD("BS code from serviceClass = %s", InfoClassToMmiBSCodeString(serviceClass));

        free(precmd);
        precmd = cmd;
    }

    /* Check SIC: No reply timer */
    /* shall we check CF_ALL and CF_ALLCOND ? In ril.h time is for CF_NORPLY only. */
    if (((p_args->reason == CF_NORPLY) || (p_args->reason == CF_ALL) || (p_args->reason == CF_ALLCOND)) &&
        ((p_args->status == SS_REGISTER) || (p_args->status == SS_ACTIVATE)) &&
        (p_args->timeSeconds != 0)) {

        if (eSiStatus == HAS_NONE) {
            asprintf(&cmd, "%s***%d", precmd, p_args->timeSeconds);
        } else if (eSiStatus == HAS_SIA) {
            asprintf(&cmd, "%s**%d", precmd, p_args->timeSeconds);
        } else {
            asprintf(&cmd, "%s*%d", precmd, p_args->timeSeconds);
        }

        free(precmd);
        precmd = cmd;
    }

    /* Check END */
    asprintf(&cmd, "%s#\"", precmd);

    free(precmd);

    if (CCFC_E_QUERY == op) {
        /**
         * RIL_REQUEST_QUERY_CALL_FORWARD_STATUS
         *
         * "data" is const RIL_CallForwardInfo *
         *
         * "response" is const RIL_CallForwardInfo **
         * "response" points to an array of RIL_CallForwardInfo *'s, one for
         * each distinct registered phone number.
         *
         * For example, if data is forwarded to +18005551212 and voice is forwarded
         * to +18005559999, then two separate RIL_CallForwardInfo's should be returned
         *
         * If, however, both data and voice are forwarded to +18005551212, then
         * a single RIL_CallForwardInfo can be returned with the service class
         * set to "data + voice = 3")
         *
         * Valid errors:
         *  SUCCESS
         *  RADIO_NOT_AVAILABLE
         *  GENERIC_FAILURE
         */

        err = at_send_command_multiline(cmd, "+CCFC:", &p_response, SS_CHANNEL_CTX);
    } else {
        /* add DN length */
        if (dnlen != 0) {
            precmd = cmd;
            asprintf(&cmd, "%s,,%d", precmd, dnlen);
            free(precmd);
        }

        /**
         * RIL_REQUEST_SET_CALL_FORWARD
         *
         * Configure call forward rule
         *
         * "data" is const RIL_CallForwardInfo *
         * "response" is NULL
         *
         * Valid errors:
         *  SUCCESS
         *  RADIO_NOT_AVAILABLE
         *  GENERIC_FAILURE
         */

        err = at_send_command(cmd, &p_response, SS_CHANNEL_CTX);
    }

    free(cmd);

    if (err < 0 || NULL == p_response) {
        LOGE("requestCallForwardOperation Fail");
        goto error;
    }

    switch (at_get_cme_error(p_response)) {
        case CME_SUCCESS:
            ret = RIL_E_SUCCESS;
            break;
        case CME_CALL_BARRED:
        case CME_OPR_DTR_BARRING:
            ret = RIL_E_OEM_ERROR_1;  // CALL_BARRED;
            goto error;
            break;
        case CME_PHB_FDN_BLOCKED:
            ret = RIL_E_FDN_CHECK_FAILURE;
            goto error;
            break;
        default:
            goto error;
    }

    if (CCFC_E_QUERY == op) {
        for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next) {
            resLength++;
        }

        LOGI("%d of +CCFC: received!", resLength);

        pp_CfInfoResponses = (RIL_CallForwardInfo **)
                alloca(resLength * sizeof(RIL_CallForwardInfo *));
        memset(pp_CfInfoResponses, 0, resLength * sizeof(RIL_CallForwardInfo *));

        resLength = 0; /* reset resLength for decoding */

        for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next) {
            char *line  = NULL;
            int  bsCode = 0;

            line = p_cur->line;

            if (line == NULL) {
                LOGE("CCFC: NULL line");
                break;
            }

            if (p_CfInfoResponse == NULL) {
                p_CfInfoResponse = (RIL_CallForwardInfo *) alloca(sizeof(RIL_CallForwardInfo));
                memset(p_CfInfoResponse, 0, sizeof(RIL_CallForwardInfo));
                p_CfInfoResponse->reason = p_args->reason;
            }

            ((RIL_CallForwardInfo **) pp_CfInfoResponses)[resLength] = p_CfInfoResponse;

            /**
             * For Query CCFC only
             * +CCFC: <status>, <class1>[, <number>, <type>
             * [, <subaddr>, <satype>[, <time>]]]
             */

            if (at_tok_start(&line) < 0) {
                LOGE("+CCFC: fail");
                continue;
            }

            if (at_tok_nextint(&line, &(p_CfInfoResponse->status)) < 0) {
                LOGE("+CCFC: status fail!");
                /* continue; */
            }

            if (at_tok_nextint(&line, &bsCode) < 0) {
                LOGE("+CCFC: bsCode fail!");
                /* continue; */
            }

            if (serviceClass != 0 && p_CfInfoResponse->status == 0 && bsCode == 0) {
                p_CfInfoResponse->serviceClass = serviceClass;
            } else {
                p_CfInfoResponse->serviceClass = MmiBSCodeToInfoClassX(bsCode);
            }

            if (at_tok_hasmore(&line)) {
                if (at_tok_nextstr(&line, &(p_CfInfoResponse->number)) < 0) {
                    LOGE("+CCFC: number fail!");
                }

                if (at_tok_nextint(&line, &(p_CfInfoResponse->toa)) < 0) {
                    LOGE("+CCFC: toa fail!");
                }
            }

            if (at_tok_hasmore(&line)) {
                /* skip subaddr */
                if (at_tok_nextstr(&line, &(pStrTmp)) < 0) {
                    LOGE("+CCFC: sub fail!");
                }

                /* skip satype */
                if (at_tok_nextint(&line, &(p_CfInfoResponse->timeSeconds)) < 0) {
                    LOGE("+CCFC: sa type fail!");
                }

                if (at_tok_nextint(&line, &(p_CfInfoResponse->timeSeconds)) < 0) {
                    LOGE("+CCFC: time fail!");
                }
            }

#ifdef MTK_VT3G324M_SUPPORT
            if (isLteSupport()) {
                if (p_CfInfoResponse->serviceClass == CLASS_DATA_SYNC) {
                    p_CfInfoResponse->serviceClass = CLASS_MTK_VIDEO;
                }
            }
#endif

            /* If it is a User Load, need to hide number informaiton */
            const char *tmpNumber = encryptString(p_CfInfoResponse->number);
            LOGD("CfInfoResponse status:%d class:%d num:%s toa:%d time:%d",
                     p_CfInfoResponse->status,
                     p_CfInfoResponse->serviceClass,
                     tmpNumber,
                     p_CfInfoResponse->toa,
                     p_CfInfoResponse->timeSeconds);
            free((char*)tmpNumber);

            p_CfInfoResponse = NULL;
            resLength++;
        }
        LOGI("%d of +CCFC: decoded!", resLength);
    }

error:
    RIL_onRequestComplete(t, ret, pp_CfInfoResponses, resLength * sizeof(RIL_CallForwardInfo *));
    at_response_free(p_response);
}


/**
 * RIL_REQUEST_QUERY_CALL_FORWARD_STATUS
 *
 * "data" is const RIL_CallForwardInfo *
 *
 * "response" is const RIL_CallForwardInfo **
 * "response" points to an array of RIL_CallForwardInfo *'s, one for
 * each distinct registered phone number.
 *
 * For example, if data is forwarded to +18005551212 and voice is forwarded
 * to +18005559999, then two separate RIL_CallForwardInfo's should be returned
 *
 * If, however, both data and voice are forwarded to +18005551212, then
 * a single RIL_CallForwardInfo can be returned with the service class
 * set to "data + voice = 3")
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 */
static void requestQueryCallForwardStatus(void *data, size_t datalen, RIL_Token t)
{
    requestCallForwardOperation(data, datalen, t, CCFC_E_QUERY);
}


/**
 * RIL_REQUEST_SET_CALL_FORWARD
 *
 * Configure call forward rule
 *
 * "data" is const RIL_CallForwardInfo *
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 */
static void requestSetCallForward(void *data, size_t datalen, RIL_Token t)
{
    requestCallForwardOperation(data, datalen, t, CCFC_E_SET);
}


static void requestCallWaitingOperation(void *data, size_t datalen, RIL_Token t)
{
    int *p_int = (int *) data;
    ATResponse *p_response = NULL;
    int err;
    char *cmd = NULL;
    RIL_Errno ret = RIL_E_INTERNAL_ERR;
    int response[2]={0};
    ATLine *p_cur = NULL;
    int resLength = 0;
    int sendBsCode = 0;
    int responseForAll = 0;

    /**
     * AT+ECUSD=<m>, <n>, <str>
     * <m>: 1 for SS, 2 for USSD
     * <n>: 1 for execute SS or USSD, 2 for cancel USSD session
     * <str>: string type parameter, the SS or USSD string
     */

    /**
     *       SC    SIA SIB SIC
     * WAIT  43    BS  -   -
     */

    if (datalen == sizeof(int)) {

        sendBsCode = p_int[0];
        LOGD("sendBsCode = %d", sendBsCode);

        /* From call settings: sendBsCode = 512(CLASS_MTK_VIDEO) */
        /* From MMI command: sendBsCode = 528(CLASS_MTK_VIDEO + CLASS_DATA_SYNC) */
        if (isLteSupport()) {
            if ((sendBsCode == CLASS_MTK_VIDEO) ||
                (sendBsCode == (CLASS_MTK_VIDEO + CLASS_DATA_SYNC))) {
                sendBsCode = CLASS_DATA_SYNC;
            }
        }

        asprintf(&cmd, "AT+ECUSD=1,1,\"*#43#\"");
        err = at_send_command_multiline(cmd, "+CCWA:", &p_response, SS_CHANNEL_CTX);

    } else if (datalen == 2 * sizeof(int)) {
        LOGD("p_int[0] = %d, p_int[1] = %d", p_int[0], p_int[1]);
        if (p_int[1] != 0) {
            /* with InfoClass */
            asprintf(&cmd, "AT+ECUSD=1,1,\"%s43*%s#\"",
                    ssStatusToOpCodeString(p_int[0]),
                    InfoClassToMmiBSCodeString(p_int[1]));
        } else {
            /* User did not input InfoClass */
            asprintf(&cmd, "AT+ECUSD=1,1,\"%s43#\"",
                    ssStatusToOpCodeString(p_int[0]));
        }

        err = at_send_command(cmd, &p_response, SS_CHANNEL_CTX);
    } else {
        goto error;
    }

    free(cmd);

    if (err < 0 || NULL == p_response) {
        LOGE("requestCallWaitingOperation Fail");
        goto error;
    }

    switch (at_get_cme_error(p_response)) {
        case CME_SUCCESS:
            break;
        case CME_CALL_BARRED:
        case CME_OPR_DTR_BARRING:
            ret = RIL_E_OEM_ERROR_1;  // CALL_BARRED;
            goto error;
            break;
        case CME_PHB_FDN_BLOCKED:
            ret = RIL_E_FDN_CHECK_FAILURE;
            goto error;
            break;
        default:
            goto error;
    }

    /* For Query CCWA only */
    if (p_response->p_intermediates != NULL) {
        for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next) {
            resLength++;
        }

        LOGI("%d of +CCWA: received!", resLength);

        resLength = 0; /* reset resLength for decoding */

        if (isLteSupport()) {
            response[1] = 0;
        }

        for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next) {
            char *line  = NULL;
            int  bsCode = 0;

            line = p_cur->line;

            if (line == NULL) {
                LOGE("CCWA: NULL line");
                break;
            }

            if (at_tok_start(&line) < 0) {
                goto error;
            }

            /**
             * <status>
             * 0   not active
             * 1   active
             */
            if (at_tok_nextint(&line, &response[0]) < 0) {
                goto error;
            }

            /**
             * <classx> is a sum of integers each representing a class of information (default 7):
             * 1   voice (telephony)
             * 2   data (refers to all bearer services; with <mode>=2 this may refer only
             *     to some bearer service if TA does not support values 16, 32, 64 and 128)
             * 4   fax (facsimile services)
             * 8   short message service
             * 16  data circuit sync
             * 32  data circuit async
             * 64  dedicated packet access
             * 128 dedicated PAD access
             */
            if (at_tok_nextint(&line, &bsCode) < 0) {
                goto error;
            }

            if (isLteSupport()) {
                if (sendBsCode != 0) {
                    if (sendBsCode == bsCode) {
                        /* Set response[1] to 1 to indicated that the call waiting is enabled */
                        /* (Refer to CallWaitingCheckBoxPreference.java). */
                        response[1] = 1;

                        /* Check if call waiting is queried via MMI command */
                        if (p_int[0] == CLASS_MTK_VIDEO + CLASS_DATA_SYNC) {
                            response[1] = CLASS_MTK_VIDEO + CLASS_DATA_SYNC;
                        }

                        LOGD("response = %d, %d", response[0], response[1]);
                        break;
                    }
                } else {    /* For call wating query by MMI command */
                    if (response[0] == 1) {
                        response[1] |= (bsCode == CLASS_DATA_SYNC) ? CLASS_MTK_VIDEO + CLASS_DATA_SYNC : bsCode;
                    }
                    if (responseForAll == 0) {
                        responseForAll = response[0];
                    }
                }
            } else {
                response[1] |= MmiBSCodeToInfoClassX(bsCode);
            }
            LOGD("response = %d, %d", response[0], response[1]);
            resLength++;
        }

        LOGI("%d of +CCWA: decoded!", resLength);

        /*
           For solving [ALPS00113964]Call waiting of VT hasn't response when turn on call waiting item, MTK04070, 2012.01.12
           sendBsCode = 0   --> Voice Call Waiting, refer to SERVICE_CLASS_NONE  in CommandInterface.java, GsmPhone.java
           sendBsCode = 512 --> Video Call Waiting, refer to SERVICE_CLASS_VIDEO in CommandInterface.java, GsmPhone.java

           Query Call Waiting: Network returned +CCWA: 1, 11 or/and +CCWA: 1, 24
           MmiBSCodeToInfoClassX method will convert 11 to 1(CLASS_VOICE), and convert 24 to 16(CLASS_DATA_SYNC) + 512(CLASS_MTK_VIDEO)

           CallWaiting settings checked response[1] value, 0 as disabled and 1 as enabled.
        */
        if (isLteSupport()) {
            /* For call wating query by MMI command */
            if (sendBsCode == 0) {
                response[0] = responseForAll;
            }
        } else {
            if (sendBsCode != 0) {
                LOGD("sendBsCode = %d", sendBsCode);
                int tmpValue = response[1] & sendBsCode;
                response[1] = (tmpValue != 0);
                LOGD("response[1] = %d, tmpValue = %d", response[1], tmpValue);
            }
        }
    }

    ret = RIL_E_SUCCESS;

error:
    /* For SET CCWA responseVoid will ignore the responses */
    RIL_onRequestComplete(t, ret, response, 2 * sizeof(int));
    at_response_free(p_response);
}


/**
 * RIL_REQUEST_QUERY_CALL_WAITING
 *
 * Query current call waiting state
 *
 * "data" is const int *
 * ((const int *)data)[0] is the TS 27.007 service class to query.
 * "response" is a const int *
 * ((const int *)response)[0] is 0 for "disabled" and 1 for "enabled"
 *
 * If ((const int *)response)[0] is = 1, then ((const int *)response)[1]
 * must follow, with the TS 27.007 service class bit vector of services
 * for which call waiting is enabled.
 *
 * For example, if ((const int *)response)[0]  is 1 and
 * ((const int *)response)[1] is 3, then call waiting is enabled for data
 * and voice and disabled for everything else
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 */
static void requestQueryCallWaiting(void *data, size_t datalen, RIL_Token t)
{
    requestCallWaitingOperation(data, datalen, t);
}


/**
 * RIL_REQUEST_SET_CALL_WAITING
 *
 * Configure current call waiting state
 *
 * "data" is const int *
 * ((const int *)data)[0] is 0 for "disabled" and 1 for "enabled"
 * ((const int *)data)[1] is the TS 27.007 service class bit vector of services to modify
 *
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 */
static void requestSetCallWaiting(void *data, size_t datalen, RIL_Token t)
{
    requestCallWaitingOperation(data, datalen, t);
}

extern void requestCallBarring(void *data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response = NULL;
    int err = -1;
    char *cmd = NULL;
    const char **strings = (const char**) data;
    int response = -1;
    char *line;
    const char *p_serviceClass;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    RILChannelCtx *rilChannel = getRILChannelCtxFromToken(t);
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(rilChannel);
    int resLength = 0;
    int *p_res = NULL;
    int sendBsCode = 0;
    int allResponse = -1;

    /* [ALPS00451149][MTK02772]
     * CLCK is query before MSG_ID_SIM_MMI_READY_IND
     * FD's flag is ready after receive this msg
     * solution: request again if modem response busy, max 2.5s */
    int isSimBusy = 0;
    int count = 0;

    if (getRadioState(rid) == RADIO_STATE_OFF) {
        RLOGE("requestCallBarring: Radio Off");
        RIL_onRequestComplete(t, RIL_E_RADIO_NOT_AVAILABLE, NULL, 0);
        return;
    }

    do {
        /* ALPS00839044: Modem needs more time for some special cards. */
        /* The detail of the time 2.5s is in the note of this CR. */
        if (count == 13) {
            RLOGE("Set Facility Lock: CME_SIM_BUSY and time out.");
            goto error;
        }

        if (datalen == 4*sizeof(char*)) {
            /* Query Facility Lock */
            if ((0 == strcmp("AB", strings[0])) ||
                (0 == strcmp("AG", strings[0])) ||
                (0 == strcmp("AC", strings[0]))) {
                    RLOGE("Call Barring Error: %s Cannot be used for Query!", strings[0]);
                    goto error;
            }

            if ((NULL != strings[2]) && (0 != strcmp(strings[2], "0"))) {
                p_serviceClass = strings[2];
                sendBsCode = atoi(p_serviceClass);
            }

            if (isLteSupport()) {
                if (sendBsCode == CLASS_MTK_VIDEO) {
                    sendBsCode = CLASS_DATA_SYNC;
                }
            }
            RLOGD("sendBsCode = %d", sendBsCode);

            /* PASSWD is given and CLASS is necessary. Because of NW related operation */
            /* asprintf(&cmd, "AT+CLCK=\"%s\",2,\"%s\",\"%s\"", strings[0], strings[1], strings[2]);
                asprintf(&cmd, "AT+ECUSD=1,1,\"*#%s**%s#\"", callBarFacToServiceCodeStrings(strings[0]),
                                                      InfoClassToMmiBSCodeString(atoi(p_serviceClass)));

            } else {*/
            /* BS_ALL NULL BSCodeString */
            // When query call barring setting, don't send BS code because some network cannot support BS code.
            asprintf(&cmd, "AT+ECUSD=1,1,\"*#%s#\"", callBarFacToServiceCodeStrings(strings[0]));
            //}
            err = at_send_command_multiline(cmd, "+CLCK:", &p_response, rilChannel);
        } else if ( datalen == 5*sizeof(char*) ) {
            if (NULL == strings[2]) {
                RLOGE("Set Facility Lock: Pwd cannot be null!");
                ret = RIL_E_PASSWORD_INCORRECT;
                goto error;
            }

            /* Set Facility Lock */
            if (strlen(strings[2]) != 4) {
                RLOGE("Set Facility Lock: Incorrect passwd length:%d", strlen(strings[2]));
                ret = RIL_E_PASSWORD_INCORRECT;
                goto error;
            }

            if ((NULL != strings[3]) && (0 != strcmp(strings[3], "0"))) {
                p_serviceClass = strings[3];

                /* Network operation. PASSWD is necessary */
                /* asprintf(&cmd, "AT+CLCK=\"%s\",%s,\"%s\",\"%s\"", strings[0], strings[1], strings[2], strings[3]); */
                if (0 == strcmp(strings[1], "0")) {
                    asprintf(&cmd, "AT+ECUSD=1,1,\"#%s*%s*%s#\"", callBarFacToServiceCodeStrings(strings[0]),
                            strings[2], InfoClassToMmiBSCodeString(atoi(p_serviceClass)));
                } else {
                    asprintf(&cmd, "AT+ECUSD=1,1,\"*%s*%s*%s#\"", callBarFacToServiceCodeStrings(strings[0]),
                            strings[2], InfoClassToMmiBSCodeString(atoi(p_serviceClass)));
                }
            } else {
                /* For BS_ALL BS==NULL */
                if (0 == strcmp(strings[1], "0")) {
                    asprintf(&cmd, "AT+ECUSD=1,1,\"#%s*%s#\"", callBarFacToServiceCodeStrings(strings[0]),
                            strings[2]);
                } else {
                    asprintf(&cmd, "AT+ECUSD=1,1,\"*%s*%s#\"", callBarFacToServiceCodeStrings(strings[0]),
                            strings[2]);
                }
            }
            err = at_send_command(cmd, &p_response, rilChannel);
        } else {
            goto error;
        }

        free(cmd);
        cmd = NULL;

        if (err < 0 || NULL == p_response) {
            RLOGE("getFacilityLock Fail");
            goto error;
        }
        switch (at_get_cme_error(p_response)) {
            case CME_SIM_BUSY:
                RLOGD("simFacilityLock: CME_SIM_BUSY");
                sleepMsec(200);
                count++;
                isSimBusy = 1;
                at_response_free(p_response);
                p_response = NULL;
                break;
            default:
                RLOGD("simFacilityLock: default");
                isSimBusy = 0;
                break;
        }
    } while (isSimBusy == 1);

    if (p_response->success == 0) {
        switch (at_get_cme_error(p_response)) {
            case CME_SIM_PIN_REQUIRED:
            case CME_SIM_PUK_REQUIRED:
                ret = RIL_E_PASSWORD_INCORRECT;
                goto error;
                break;
            case CME_SIM_PIN2_REQUIRED:
                ret = RIL_E_SIM_PIN2;
                goto error;
                break;
            case CME_SIM_PUK2_REQUIRED:
                ret = RIL_E_SIM_PUK2;
                goto error;
                break;
            case CME_INCORRECT_PASSWORD:
                ret = RIL_E_PASSWORD_INCORRECT;
                goto error;
                break;
            case CME_CALL_BARRED:
            case CME_OPR_DTR_BARRING:
                ret = RIL_E_GENERIC_FAILURE;
                goto error;
                break;
            case CME_PHB_FDN_BLOCKED:
                ret = RIL_E_FDN_CHECK_FAILURE;
                goto error;
                break;
            default:
                goto error;
        }
    } else {
        ret = RIL_E_SUCCESS;
    }

    /* For Query command only */
    if (p_response->p_intermediates != NULL) {
        ATLine *p_cur;
        int serviceClass;

        if (!isLteSupport()) {
            for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next) {
                resLength++;
            }
            RLOGI("%d of +CLCK: received!", resLength);

            p_res = alloca(resLength * sizeof(int));
            resLength = 0; /* reset resLength for decoding */
        }

        for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next) {
            char *line = p_cur->line;
            assert(line);

            err = at_tok_start(&line);

            if (err < 0) {
                    RLOGE("CLCK: decode error 1!");
                goto error;
            }

            err = at_tok_nextint(&line, &response); /* 0 disable 1 enable */

            if (!isLteSupport()) {
                p_res[resLength] = 0; /* Set Init value to 0 */
            }

            if (at_tok_hasmore(&line)) {
                if (isLteSupport()) {
                    err = at_tok_nextint(&line, &serviceClass); /* enabled service code */
                    if (sendBsCode == serviceClass) {
                       break;
                    }

                    /* For solving ALPS01415650.
                     * When BsCode is not specified, return all responses for all service classes */
                       LOGD("sendBsCode = %d, response = %d, serviceClass = %d", sendBsCode, response, serviceClass);
                       if ((sendBsCode == 0) && (response != 0)) {
                            allResponse = (allResponse == -1) ? 0 : allResponse;
                            if (serviceClass == CLASS_DATA_SYNC) {
                                allResponse |= CLASS_MTK_VIDEO;
                            } else {
                                allResponse |= serviceClass;
                            }
                       }
                } else {
                    err = at_tok_nextint(&line, &p_res[resLength]); /* enabled service code */
                    RLOGD("Status:%d, BSCode:%d\n", response, p_res[resLength]);
                }
            }

            if (isLteSupport() && (sendBsCode != 0)) {
                response = 0;
            }
            if (err < 0) {
                RLOGE("CLCK: decode error 2!");
                goto error;
            }

            if (!isLteSupport()) {
                resLength++;
            }
        }
    }

    if (err < 0) {
        goto error;
    } else {
        if (isLteSupport()) {
            /* The specific call barring is activated(enabled), return service class value(Refer to
             * CallBarringBasePreference.java). */
            if (sendBsCode != 0) {
                if (response != 0) {
                    response = atoi(p_serviceClass);
                }
            } else {
                if (allResponse != -1) {
                    response = allResponse;
                }
            }
            RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(int));
        } else {
            RLOGD("sendBsCode = %d", sendBsCode);
            RLOGD("resLength = %d", resLength);
            if (resLength >= 1) {
                /* For CLCK NW(Call Barring) results: we shall combind the results */
                if (response != 0) {
                    response = 0;
                    do {
                        resLength--;
                        RLOGD("response = %d, MmiBSCodeToInfoClassX(p_res[resLength]) = %d", response,
                                MmiBSCodeToInfoClassX(p_res[resLength]));
                        if (((sendBsCode != 0) && (sendBsCode & MmiBSCodeToInfoClassX(p_res[resLength])))
                            || sendBsCode == 0) {
                            response |= MmiBSCodeToInfoClassX(p_res[resLength]);
                        }
                    } while (resLength > 0);
                }
                RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(int));
            } else {
                /* For those non NW (Call Barring) results*/
                RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(int));
            }
        }
        RLOGD("simFacilityLock response:%d", response);

        at_response_free(p_response);
        p_response = NULL;
        return;
    }

error:
    RIL_onRequestComplete(t, ret, NULL, 0);
    at_response_free(p_response);
}

const char *callBarFacToServiceCodeStrings(const char *fac)
{
    int i;
    for (i = 0; i < CB_SUPPORT_NUM; i++) {
        if (0 == strcmp(fac, callBarFacilityStrings[i])) {
            return callBarServiceCodeStrings[i];
        }
    }
    /* not found! return default */
    return callBarServiceCodeStrings[CB_ABS];
}

/**
 * RIL_REQUEST_CHANGE_BARRING_PASSWORD
 *
 * Change call barring facility password
 *
 * "data" is const char **
 *
 * ((const char **)data)[0] = facility string code from TS 27.007 7.4 (eg "AO" for BAOC)
 * ((const char **)data)[1] = old password
 * ((const char **)data)[2] = new password
 *
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 */
static void requestChangeBarringPassword(void *data, size_t datalen, RIL_Token t)
{
    const char **strings = (const char**)data;
    ATResponse *p_response = NULL;
    int err;
    char *cmd = NULL;
    RIL_Errno ret = RIL_E_INTERNAL_ERR;

    /**
     * "data" is const char **
     *
     * ((const char **)data)[0] = facility string code from TS 27.007 7.4 (eg "AO" for BAOC)
     * ((const char **)data)[1] = old password
     * ((const char **)data)[2] = new password
     * ((const char **)data)[3] = new password confirmed
     */
    if (datalen == 3 * sizeof(char*)) {
        if (strings[0] == NULL || strlen(strings[0]) == 0 ||
            strings[1] == NULL || strlen(strings[1]) == 0 ||
            strings[2] == NULL || strlen(strings[2]) == 0) {
            RLOGE("ChangeBarringPassword: Null parameters.");
            ret = RIL_E_INVALID_ARGUMENTS;
            goto error;
        }
        asprintf(&cmd, "AT+ECUSD=1,1,\"**03*%s*%s*%s*%s#\"",
                callBarFacToServiceCodeStrings(strings[0]), strings[1], strings[2], strings[2]);
    } else if (datalen == 4 * sizeof(char*)) {
        if (strings[0] == NULL || strlen(strings[0]) == 0 ||
            strings[1] == NULL || strlen(strings[1]) == 0 ||
            strings[2] == NULL || strlen(strings[2]) == 0 ||
            strings[3] == NULL || strlen(strings[3]) == 0) {
            RLOGE("ChangeBarringPassword: Null parameters.");
            ret = RIL_E_INVALID_ARGUMENTS;
            goto error;
        }
        asprintf(&cmd, "AT+ECUSD=1,1,\"**03*%s*%s*%s*%s#\"",
                callBarFacToServiceCodeStrings(strings[0]), strings[1], strings[2], strings[3]);
    } else {
        goto error;
    }

    err = at_send_command(cmd, &p_response, SS_CHANNEL_CTX);

    free(cmd);

    if (err < 0 || NULL == p_response) {
        LOGE("requestChangeBarringPassword Fail");
        goto error;
    }

    switch (at_get_cme_error(p_response)) {
        case CME_SUCCESS:
            ret = RIL_E_SUCCESS;
            break;
        case CME_INCORRECT_PASSWORD:
            ret = RIL_E_PASSWORD_INCORRECT;
            break;
        case CME_CALL_BARRED:
        case CME_OPR_DTR_BARRING:
            ret = RIL_E_OEM_ERROR_1;  // CALL_BARRED;
            break;
        case CME_PHB_FDN_BLOCKED:
            ret = RIL_E_FDN_CHECK_FAILURE;
            break;
        default:
            break;
    }

error:
    RIL_onRequestComplete(t, ret, NULL, 0);
    at_response_free(p_response);
}

/**
 * RIL_REQUEST_QUERY_CLIP
 *
 * Queries the status of the CLIP supplementary service
 *
 * (for MMI code "*#30#")
 *
 * "data" is NULL
 * "response" is an int *
 * (int *)response)[0] is 1 for "CLIP provisioned"
 *                     and 0 for "CLIP not provisioned"
 *                     and 2 for "unknown, e.g. no network etc"
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE (radio resetting)
 *  GENERIC_FAILURE
 */
static void requestQueryClip(void *data, size_t datalen, RIL_Token t)
{
    UNUSED(datalen);
    UNUSED(data);
    ATResponse *p_response = NULL;
    int err;
    char *line = NULL;
    RIL_Errno ret = RIL_E_INTERNAL_ERR;
    int response[2]={0};

    /**
     * AT+ECUSD=<m>, <n>, <str>
     * <m>: 1 for SS, 2 for USSD
     * <n>: 1 for execute SS or USSD, 2 for cancel USSD session
     * <str>: string type parameter, the SS or USSD string
     */
    err = at_send_command_singleline("AT+ECUSD=1,1,\"*#30#\"", "+CLIP:", &p_response, SS_CHANNEL_CTX);

    if (err < 0 || NULL == p_response) {
        LOGE("requestQueryClip Fail");
        goto error;
    }

    switch (at_get_cme_error(p_response)) {
        case CME_SUCCESS:
            break;
        case CME_CALL_BARRED:
        case CME_OPR_DTR_BARRING:
            ret = RIL_E_OEM_ERROR_1;  // CALL_BARRED;
            goto error;
            break;
        case CME_PHB_FDN_BLOCKED:
            ret = RIL_E_FDN_CHECK_FAILURE;
            goto error;
            break;
        default:
            goto error;
    }

    if ( p_response->p_intermediates != NULL ) {
        line = p_response->p_intermediates->line;

        if (at_tok_start(&line) < 0) {
            goto error;
        }

        /**
         * <n> (parameter sets/shows the result code presentation status in the MT/TA):
         * 0   disable
         * 1   enable
         */
        if (at_tok_nextint(&line, &response[0]) < 0) {
            goto error;
        }

        /**
         * <m> (parameter shows the subscriber CLIP service status in the network):
         * 0   CLIP not provisioned
         * 1   CLIP provisioned
         * 2   unknown (e.g. no network, etc.)
         */
        if (at_tok_nextint(&line, &response[1]) < 0) {
           goto error;
        }
    }

    /* return success here */
    ret = RIL_E_SUCCESS;

error:
    RIL_onRequestComplete(t, ret, &response[1], sizeof(int));
    at_response_free(p_response);
}

/**
 * RIL_REQUEST_SET_SUPP_SVC_NOTIFICATION
 *
 * Enable/disable supplementary service related notifications
 * from the network.
 *
 * Notifications are reported via RIL_UNSOL_SUPP_SVC_NOTIFICATION.
 *
 * "data" is int *
 * ((int *)data)[0] is == 1 for notifications enabled
 * ((int *)data)[0] is == 0 for notifications disabled
 *
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 *
 * See also: RIL_UNSOL_SUPP_SVC_NOTIFICATION.
 */
static void requestSetSuppSvcNotification(void *data, size_t datalen, RIL_Token t)
{
    int *n = (int *) data;
    ATResponse *p_response = NULL;
    int err;
    char *cmd = NULL;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;

    if (datalen != 0) {
        /**
         * +CSSN=[<n>[, <m>]]
         * "data" is int *
         * ((int *)data)[0] is == 1 for notifications enabled
         * ((int *)data)[0] is == 0 for notifications disabled
         */
        asprintf(&cmd, "AT+CSSN=%d,%d", n[0], n[0]);
        err = at_send_command(cmd, &p_response, SS_CHANNEL_CTX);
        free(cmd);
    }
    else {
        goto error;
    }

    if (err < 0 || NULL == p_response) {
        LOGE("requestSetSuppSvcNotification Fail");
        goto error;
    }

    switch (at_get_cme_error(p_response)) {
        case CME_SUCCESS:
            break;
        default:
            goto error;
    }

    ret = RIL_E_SUCCESS;

error:
    RIL_onRequestComplete(t, ret, NULL, 0);
    at_response_free(p_response);
}

/**
 * RIL_UNSOL_ON_USSD
 *
 * Called when a new USSD message is received.
 *
 * "data" is const char **
 * ((const char **)data)[0] points to a type code, which is
 *  one of these string values:
 *      "0"   USSD-Notify -- text in ((const char **)data)[1]
 *      "1"   USSD-Request -- text in ((const char **)data)[1]
 *      "2"   Session terminated by network
 *      "3"   other local client (eg, SIM Toolkit) has responded
 *      "4"   Operation not supported
 *      "5"   Network timeout
 *
 * The USSD session is assumed to persist if the type code is "1", otherwise
 * the current session (if any) is assumed to have terminated.
 *
 * ((const char **)data)[1] points to a message string if applicable, which
 * should be coded as dcs in ((const char **)data)[2]
 */
static void onUssd(char *s, RIL_SOCKET_ID rid)
{
    LOGD("onUssd: %s", s);

    char *p_data[3];
    char *p_utf8Data = NULL;
    char *p_ucs2Data = NULL;
    char *p_str = NULL;
    char *line = s;
    const char *dcs;
    int iDCS;
    bool hasStr = false;
    int length = 0;
    char *dcsString = NULL;
    char* newMode = NULL;

    /* Coverity for declaim in the begining. */
    int numStrings;
    bytes_t utf8String = NULL;
    bytes_t hexData = NULL;
    int len = 0, maxLen = 0, i = 0;
    size_t responselen = 0;
    char **p_cur = NULL;
    char *finalData[2];

    /* Initialize finalData */
    for(int i = 0; i < 2; i++) {
        finalData[i] = NULL;
    }

    /**
     * USSD response from the network, or network initiated operation
     * +CUSD: <m>[, <str>, <dcs>] to the TE.
     */
    /*
     * <m>:
     * 0   no further user action required (network initiated USSD Notify, or no further information needed after mobile initiated operation)
     * 1   further user action required (network initiated USSD Request, or further information needed after mobile initiated operation)
     * 2   USSD terminated by network
     * 3   other local client has responded
     * 4   operation not supported
     * 5   network time out
     */

    if (at_tok_start(&line) < 0) {
        goto error;
    }

    /* Get <m> */
    if (at_tok_nextstr(&line, &p_data[0]) < 0) {
       goto error;
    }

    if (strcmp(p_data[0], "3") == 0) {
        /* do not display any MMI dialog for +CUSD: 3,
         * no matter there is USSD message or not.
         */
        LOGD("Ignore +CUSD: 3, return directly");
        return;
    }

    length++;

    /* Check if there is <str> */
    if (at_tok_hasmore(&line)) {
        /* Get <str> */
        if (at_tok_nextstr(&line, &p_str) < 0) {
            goto error;
        }
        hasStr = true;

        length++;

        /* Get <dcs> */
        if (at_tok_nextint(&line, (int*) &iDCS) < 0) {
            LOGE("No <dcs> information");
            goto error;
        }

        length++;
        LOGD("onUssd: length = %d", length);
        LOGD("onUssd: line = %s", line);
        LOGD("onUssd: p_data[0] = %s", p_data[0]);
        LOGD("onUssd: p_str = %s", p_str);
        LOGD("onUssd: dcs = %d", iDCS);

        /* Refer to GSM 23.038, section 5 CBS Data Coding Scheme
        *The message starts with a two GSM 7-bits default alphabet character.
        *Need to ignore these two bytes.
        */
        GsmCbsDcsE dcsType = checkCbsDcs(iDCS);
        if (iDCS == 0x11) {
           LOGD("Ignore the first two bytes for DCS_UCS2");
           p_str += 4;
        }

        p_data[1] = p_str;
        LOGD("onUssd: p_data[1] (p_str) = %s", p_data[1]);
        if (isLteSupport()) {
            if (dcsType == DCS_8BIT) {
                p_data[2] = (char *) GsmCbsDcsStringp[dcsType];
                LOGD("onUssd: p_data[2] (dcsString) = %s", p_data[2]);
            } else {
                /* DCS is set as "UCS2" by AT+CSCS in ril_callbacks.c */
                p_data[2] = strdup("UCS2");
                LOGD("onUssd: p_data[2] (dcsString) = %s", p_data[2]);
            }
        } else {
            p_data[2] = (char *) GsmCbsDcsStringp[dcsType];
            LOGD("onUssd: p_data[2] (dcsString) = %s", p_data[2]);
        }
    }

    /* Decode response data by using ril_gsm_util. */
    responselen = length * sizeof(char *);
    p_cur = (char **) p_data;
    /* Only need m and str, dcs is only for str reference and no need to pass it. */

    numStrings = responselen / sizeof(char *);

    if (numStrings > 1) {
        /* the last string is for dcs reference */
        numStrings = 2;
        hexData = (bytes_t) calloc(strlen(p_cur[1]), sizeof(char));
        if (hexData == NULL) {
            LOGE("onUssd: hexData malloc fail");
            goto error;
        }

        len = gsm_hex_to_bytes((cbytes_t) p_cur[1], strlen(p_cur[1]), hexData);
        LOGD("onUssd: add value to hexData = %s", hexData);
        LOGD("onUssd: len = %d", len);

        dcs = p_cur[2];
        LOGD("onUssd: dcs = p_cur[2] = %s", dcs);

        maxLen = (!strcmp(dcs, "UCS2")) ? (len / 2) : len;
        if ((maxLen < 0) || (maxLen > MAX_RIL_USSD_STRING_LENGTH)) {
            free(hexData);
            hexData = NULL;
            LOGE("onUssd: max length is invalid, len = %d", maxLen);
            goto error;
            /* return RIL_ERRNO_INVALID_RESPONSE; */
        } else {
            LOGE("Ussd string length:%d/n", maxLen);
        }
    }

    /* each string*/
    for (i = 0; i < numStrings; i++) {
        if (i == 1) {
            utf8String = (bytes_t) calloc(2*len+1, sizeof(char));
            if (utf8String == NULL) {
                free(hexData);   /* Coverity */
                hexData = NULL;  /* Coverity */
                LOGE("onUssd: utf8String malloc fail");
                goto error;
            }

            /* The USSD strings need to be transform to utf8 */
            if (!strcmp(dcs, "GSM7")) {
                LOGD("Ussd GSM7");

                /* Fix ALPS03334395
                 * If there is a null character '\0' in the USSD string, it should be
                 * ignore in order to avoid cutting off the string after null character
                 */
                int ussd_str_len = utf8_from_unpackedgsm7((cbytes_t) hexData, 0, len, utf8String);
                int j, k;
                for (j = 0; j < ussd_str_len; j++) {
                    if (*(utf8String + j) == 0) {
                        for (k = j; k < ussd_str_len; k++) {
                            *(utf8String + k) = *(utf8String + k + 1);
                        }
                        ussd_str_len--;
                    }
                }
                LOGD("onUssd for loop, utf8String = %s, length = %d", utf8String, ussd_str_len);
            } else if (!strcmp(dcs, "UCS2")) {
                LOGD("Ussd UCS2");
                /* Some character can't display when receive the ussd notification */
                zero4_to_space(hexData, len);
                ucs2_to_utf8((cbytes_t) hexData, len/2, utf8String);
            } else {
                LOGD("Ussd not GSM7 or UCS2");
                utf8_from_gsm8((cbytes_t) hexData, len, utf8String);
            }
            finalData[1] = (char *) utf8String;
            LOGD("onUssd: each string of finalData[1]= %s", finalData[1]);
        } else {
            finalData[i] = p_cur[i];
            LOGD("onUssd: each string of finalData[%d]= %s", i, finalData[i]);
        }
    }

    /* Change to comply with AOSP */
    if (strcmp(finalData[0], "2") == 0) {
        newMode = strdup("0");
        finalData[0] = newMode;
        LOGD("onUssd: finalData[0] new value = %s", finalData[0]);
    }

    /* Network might response empty str */
    if (hasStr) {
        LOGD("onUssd: finalData[1]= %s", finalData[1]);
    }

    if (strcmp(finalData[0], "1")) {
        LOGD("onUssd, further user action required, mOngoingSessionDomain: CS");
        mOngoingSessionDomain = 0;
    }

    RIL_onUnsolicitedResponse(
            RIL_UNSOL_ON_USSD,
            finalData, 2 * sizeof(char *),
            rid);

    free(hexData);
    free(utf8String);
    free(newMode);
    return;

error:
    LOGE("Parse RIL_UNSOL_ON_USSD fail: %s/n", s);
}

static void requestSetClip(void * data, size_t datalen, RIL_Token t)
{
    int* n = (int *) data;
    ATResponse* p_response = NULL;
    int err = 0; //Coverity, follow the err init value in at_send_command_full_nolock().
    char* cmd = NULL;
    char* line = NULL;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    int response[2] = {0};

    if (datalen != 0) {
        /**
         * Set CLIP: +CLIP=[<n>]
         * "data" is int *
         * ((int *)data)[0] is "n" parameter from TS 27.007 7.6
         *  <n> (Set command enables or disables the presentation of the CLI at the TE)
         */
        asprintf(&cmd, "AT+CLIP=%d", n[0]);
        err = at_send_command(cmd, &p_response, SS_CHANNEL_CTX);
        free(cmd);
    }

    if (err < 0 || NULL == p_response) {
        LOGE("requestSetClip Fail");
        goto error;
    }

    switch (at_get_cme_error(p_response)) {
        case CME_SUCCESS:
            break;
        case CME_CALL_BARRED:
        case CME_OPR_DTR_BARRING:
            ret = RIL_E_OEM_ERROR_1;  // CALL_BARRED;
            goto error;
        case CME_PHB_FDN_BLOCKED:
            ret = RIL_E_FDN_CHECK_FAILURE;
        default:
            goto error;
    }

    /* return success here */
    ret = RIL_E_SUCCESS;

error:
    /* For SET CLIP responseVoid will ignore the responses */
    RIL_onRequestComplete(t, ret, response, 2 * sizeof(int));
    at_response_free(p_response);
}

static void requestColpOperation(void * data, size_t datalen, RIL_Token t)
{
    int* n = (int *) data;
    ATResponse* p_response = NULL;
    int err;
    char* cmd = NULL;
    char* line = NULL;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    int response[2] = {0};

    if (datalen != 0) {
        /**
         * Set COLP: +COLP=[<n>]
         * "data" is int *
         * ((int *)data)[0] is "n" parameter from TS 27.007 7.8
         *  <n> (parameter sets the adjustment for outgoing calls)
         */
        asprintf(&cmd, "AT+COLP=%d", n[0]);
        err = at_send_command(cmd, &p_response, SS_CHANNEL_CTX);
        free(cmd);
    } else {
        /**
         * AT+ECUSD=<m>, <n>, <str>
         * <m>: 1 for SS, 2 for USSD
         * <n>: 1 for execute SS or USSD, 2 for cancel USSD session
         * <str>: string type parameter, the SS or USSD string
         */
        err = at_send_command_singleline("AT+ECUSD=1,1,\"*#76#\"", "+COLP:",
                &p_response, SS_CHANNEL_CTX);
    }

    if (err < 0 || NULL == p_response) {
        LOGE("requestColpOperation Fail");
        goto error;
    }

    switch (at_get_cme_error(p_response)) {
        case CME_SUCCESS:
            break;
        case CME_CALL_BARRED:
        case CME_OPR_DTR_BARRING:
            ret = RIL_E_OEM_ERROR_1;  // CALL_BARRED;
            goto error;
            break;
        case CME_PHB_FDN_BLOCKED:
            ret = RIL_E_FDN_CHECK_FAILURE;
            goto error;
            break;
        default:
            goto error;
    }

    /* For Get COLP only */
    if ( p_response->p_intermediates != NULL ) {
        line = p_response->p_intermediates->line;

        if (at_tok_start(&line) < 0) {
            goto error;
        }

        /**
         * <n> parameter sets/shows the result code presentation status to the TE
         * 0   COLP disabled in MS
         * 1   COLP enabled in MS
         */
        if (at_tok_nextint(&line, &response[0]) < 0) {
            goto error;
        }

        /**
         * <m> parameter shows the subscriber COLP service status in the network
         * 0   COLP not provisioned
         * 1   COLP provisioned in permanent mode
         */
        if (at_tok_nextint(&line, &response[1]) < 0) {
            goto error;
        }
    }

    /* return success here */
    ret = RIL_E_SUCCESS;

error:
    /* For SET COLP responseVoid will ignore the responses */
    RIL_onRequestComplete(t, ret, response, 2 * sizeof(int));
    at_response_free(p_response);
}

/**
 * RIL_REQUEST_GET_COLP
 *
 * Gets current COLP status
 * "data" is NULL
 * "response" is int *
 * ((int *)data)[0] is "n" parameter from TS 27.007 7.8
 * ((int *)data)[1] is "m" parameter from TS 27.007 7.8
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 */
static void requestGetColp(void * data, size_t datalen, RIL_Token t)
{
    requestColpOperation(data, datalen, t);
}

/**
 * RIL_REQUEST_GET_COLR
 *
 * Gets current COLR status
 * "data" is NULL
 * "response" is int *
 * ((int *)data)[0] is "n" parameter proprietary for provision status
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 */
static void requestGetColr(void * data, size_t datalen, RIL_Token t)
{
    UNUSED(datalen);
    UNUSED(data);
    ATResponse* p_response = NULL;
    int err;
    char* line = NULL;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    int response[2]={0};

    err = at_send_command_singleline("AT+ECUSD=1,1,\"*#77#\"", "+COLR:", &p_response, SS_CHANNEL_CTX);

    if (err < 0 || NULL == p_response) {
        LOGE("requestColrOperation Fail");
        goto error;
    }

    switch (at_get_cme_error(p_response)) {
        case CME_SUCCESS:
            break;
        case CME_CALL_BARRED:
        case CME_OPR_DTR_BARRING:
            ret = RIL_E_OEM_ERROR_1;  // CALL_BARRED;
            goto error;
            break;
        case CME_PHB_FDN_BLOCKED:
            ret = RIL_E_FDN_CHECK_FAILURE;
            goto error;
            break;
        default:
            goto error;
    }

    /* For Get COLR only */
    if ( p_response->p_intermediates != NULL ) {
        line = p_response->p_intermediates->line;

        if (at_tok_start(&line) < 0) {
            goto error;
        }

        /**
         * <n> parameter sets the adjustment for outgoing calls
         * 0   COLR not provisioned
         * 1   COLR provisioned
         * 2   unknown
         */
        if (at_tok_nextint(&line, &response[0]) < 0) {
            goto error;
        }
    }

    /* return success here */
    ret = RIL_E_SUCCESS;

error:
    RIL_onRequestComplete(t, ret, response, sizeof(int));
    at_response_free(p_response);
}

static void requestSendCnap(void *data, size_t datalen, RIL_Token t)
{
    UNUSED(data);
    UNUSED(datalen);
    ATResponse* p_response = NULL;
    int err;
    char* line = NULL;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    int response[2] = {0};
    RILChannelCtx *rilChannel = getRILChannelCtxFromToken(t);
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(rilChannel);

    // Block CNAP request if during VoLTE call
    if (isInImsCall(rid) && !isWfcRegOn(rid)) {
        LOGD("requestSendCnap, return error because there is a VoLTE call ongoing");
        goto error;
    }

    /**
     * AT+ECUSD=<m>, <n>, <str>
     * <m>: 1 for SS, 2 for USSD
     * <n>: 1 for execute SS or USSD, 2 for cancel USSD session
     * <str>: string type parameter, the SS or USSD string
     */
    err = at_send_command_singleline("AT+ECUSD=1,1,\"*#300#\"", "+CNAP:",
            &p_response, SS_CHANNEL_CTX);

    if (err < 0 || NULL == p_response) {
       LOGE("requestSendCnap Fail");
       goto error;
    }

    switch (at_get_cme_error(p_response)) {
       case CME_SUCCESS:
          ret = RIL_E_SUCCESS;
          break;
       case CME_CALL_BARRED:
       case CME_OPR_DTR_BARRING:
          ret = RIL_E_OEM_ERROR_1;  // CALL_BARRED;
          goto error;
          break;
       case CME_PHB_FDN_BLOCKED:
          ret = RIL_E_FDN_CHECK_FAILURE;
          goto error;
          break;
       default:
          goto error;
          break;
    }

    /* For Get CNAP only */
    if (p_response->p_intermediates != NULL) {
        line = p_response->p_intermediates->line;

        if (at_tok_start(&line) < 0) {
            LOGE("+CNAP: dail");
            goto error;
        }

        /**
         * <n> integer type (parameter sets/shows the result code presentation status to the TE)
         * 0   disable
         * 1   enable
         */
        if (at_tok_nextint(&line, &response[0]) < 0) {
            goto error;
        }

        /**
         * <m> integer type (parameter shows the subscriber CNAP service status in the network)
         * 0   CNAP not provisioned
         * 1   CNAP provisioned
         * 2   unknown (e.g. no network, etc.)
         */
        if (at_tok_nextint(&line, &response[1]) < 0) {
           goto error;
        }
    }

    /* return success here */
    ret = RIL_E_SUCCESS;

error:
    RIL_onRequestComplete(t, ret, response, 2 * sizeof(int));
    at_response_free(p_response);
}

static void requestSendUSSI(int request, void *data, size_t datalen, RIL_Token t)
{
    const char *ussi = (const char*) data;
    int action = (getUssiAction() == USSI_REQUEST) ? 1 : 2;
    const char* p_ussdRequest = convertToUCS2(ussi);
    ATResponse* p_response = NULL;
    int err;
    char* cmd = NULL;
    RIL_Errno ret = RIL_E_INTERNAL_ERR;
    char* pTmpStr = NULL;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(SS_CHANNEL_CTX);

    LOGD("requestSendUSSI: action = %d, ussi = %s", action, ussi);

    // Snapshot the USSI string in USSD domain selector
    if (mUssiSnapshot != NULL) {
        free(mUssiSnapshot);
    }

    mUssiSnapshot = strdup(ussi);
    if (mUssiSnapshot == NULL) {
        LOGE("requestSendUSSI: mUssiSnapshot strdup fail");
    }

    // Save the token info, it may be needed if CSFB occurs
    LOGD("Copy RIL_Token t to mUssiToken, t = %d", t);
    mUssiToken = t;

    asprintf(&cmd, "AT+EIUSD=2,1,%d,\"%s\",\"en\",0", action, ussi);
    LOGD("requestSendUSSI: cmd=%s", cmd);

    err = at_send_command(cmd, &p_response, SS_CHANNEL_CTX);
    free(cmd);

    if (err < 0 || NULL == p_response) {
        LOGE("requestSendUSSI Fail");
        goto error;
    }

    switch (at_get_cme_error(p_response)) {
        case CME_SUCCESS:
            ret = RIL_E_SUCCESS;
            break;
        case CME_CALL_BARRED:
        case CME_OPR_DTR_BARRING:
            ret = RIL_E_OEM_ERROR_1;  // CALL_BARRED;
            break;
        case CME_PHB_FDN_BLOCKED:
            ret = RIL_E_FDN_CHECK_FAILURE;
            break;
        default:
            sleep(1); // Let the UI have time to show up the dialog
            ret = RIL_E_INTERNAL_ERR;
            break;
    }

    if (strStartsWith(p_response->finalResponse, "ERROR")) {
        LOGE("requestSendUSSI Error.");
        ret = RIL_E_INTERNAL_ERR;
    }

error:
    at_response_free(p_response);
    if (ret == RIL_E_SUCCESS) {
        RIL_onRequestComplete(t, ret, NULL, 0);
    } else {
        // CSFB if INTERNAL_ERR occurs and no "VoLTE call" ongoing. (WFC Call is allowed)
        if (ret == RIL_E_INTERNAL_ERR) {
            if (isWfcRegOn(rid)) {
                LOGD("requestSendUSSI: generic fail, do CSFB directly (camps on WFC)");
                requestSendUSSD(request, data, datalen, t, SEND_RESPONSE_BACK);
                return;
            } else {
                if (!isInImsCall(rid)) {
                    LOGD("requestSendUSSI: generic fail, do CSFB directly (no VoLTE Call)");
                    requestSendUSSD(request, data, datalen, t, SEND_RESPONSE_BACK);
                    return;
                }
            }
        }
        sendFailureReport(request, data, datalen, t, ret);
    }
}

static void requestCancelUssi(void * data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response = NULL;
    int err;

    err = at_send_command("AT+EIUSD=2,2,2,\"\",\"en\",0", &p_response, SS_CHANNEL_CTX);

    if (err < 0 || p_response->success == 0) {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
    at_response_free(p_response);
}

/// M: SS easy config propery @{
static void requestSetSuppServProperty(void *data, size_t datalen, RIL_Token t)
{
    UNUSED(datalen);
    const char** strings = (const char**)data;

    LOGD("requestSetSuppServProperty, name:%s, value:%s", strings[0], strings[1]);

    property_set(strings[0], strings[1]);

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}
/// @}

/// M: Process USSI URC @{
static bytes_t hexString2Byte(const char* hexString)
{
    size_t count = 0;
    char* pos = hexString;
    size_t byteLength = strlen(hexString)/2;
    bytes_t byteArray = (bytes_t) calloc(byteLength + 1, sizeof(byte_t));

    if (byteArray == NULL) {
        LOGE("hexString2Byte: byteArray malloc fail");
    } else {
        for(count = 0; count < byteLength; count++) {
            sscanf(pos, "%2hhx", &byteArray[count]);
            pos += 2;
        }
    }

    return byteArray;
}

static void onUssi(char *s, RIL_SOCKET_ID rid)
{
    LOGD("onUssi: %s", s);

    int length = 8;
    char* p_data[8];
    char* line = s;
    int i = 0;

    /**
     * USSI response from the network, or network initiated operation
     * +EIUSD: <m>,<n>,<str>,<lang>,<error_code>,<alertingpattern>,<sip_cause>
     * <m>:
     * 1   USSD notify
     * 2   SS notify
     * 3   MD execute result
     * <n>:  if m=1
     * 0   no further user action required
     * 1   further user action required
     * 2   USSD terminated by network
     * 3   other local client has responded
     * 4   operation not supported
     * 5    network time out
     * <n>:if m=3, value is return value of MD
     * 0   execute success
     * 1   common error
     * 2   IMS unregistered
     * 3   IMS busy
     * 4   NW error response
     * 5   session not exist
     * 6   NW not support(404)
     * <str>: USSD/SS string
     * <lang>: USSD language
     * < error_code > USSD error code in xml
     * < alertingpattern > alerting pattern of NW initiated INVITE
     * <sip_cause> sip error code
     */

    if (at_tok_start(&line) < 0) {
        LOGD("onUssi: at_tok_start, error");
        goto error;
    }

    for (i = 0 ; i < (length - 1) ; i++) {
        if (at_tok_nextstr(&line, &p_data[i]) < 0) {
            LOGD("onUssi: at_tok_nextstr: error, p_data[%d] = %s", i, p_data[i]);
            goto error;
        }
        LOGD("onUssi: p_data[%d] = %s", i, p_data[i]);
    }

    asprintf(&p_data[7], "%d", rid);

    bytes_t ussdString = hexString2Byte(p_data[2]);
    p_data[2] = ussdString;

    handleOnUssi(p_data, length * sizeof(char *), rid);

    free(p_data[7]);

    if (ussdString != NULL) {
        free(ussdString);
    }
    return;

error:
    LOGE("Parse RIL_UNSOL_ON_USSI fail: %s/n", s);
}
/// @}

/**
 * RIL_UNSOL_CALL_FORWARDING
 *
 * Notify application to show CFU icon.
 *
 */
static void onCfuNotify(char *s, RIL_SOCKET_ID rid)
{
    char* line = s;
    int response[2] = {0}; /* status, line ID*/

    if (at_tok_start(&line) < 0) {
        goto error;
    }

    /* Get Status
     * <status>
     * 0 hide CFU icon
     * 1 show CFU icon
     */
    if (at_tok_nextint(&line, &(response[0])) < 0) {
        goto error;
    }

    /* Get Line info
     * 1 Line 1
     * 2 Line 2
     */
    if (at_tok_nextint(&line, &(response[1])) < 0) {
        goto error;
    }

    LOGD("onCfuNotify: status = %d, line ID = %d", response[0], response[1]);
    RIL_UNSOL_RESPONSE (RIL_UNSOL_CALL_FORWARDING, response, sizeof(response), rid);
    return;

error:
    LOGE("Parse RIL_UNSOL_CALL_FORWARDING fail: %s/n", s);
}

void onECMCCSSReceived() {
    LOGD("onECMCCSSReceived, isECMCCSSReceived = 1");
    isECMCCSSReceived = 1;
}

/* reset tmp SSN object and flags about ECMCCSS and CRING */
void resetECMCCSS() {
    LOGD("resetECMCCSS, clean isECMCCSSReceived, isCRINGReceived and tmpSvcNotify");
    isECMCCSSReceived = 0;
    isCRINGReceived = 0;
    memset(&tmpSvcNotify, 0, sizeof(RIL_SuppSvcNotification));
}

/* Need to be invoked by RIL-CC module */
void onCRINGReceived() {
    LOGD("onCRINGReceived, isCRINGReceived = 1");
    isCRINGReceived = 1;

    /* ECMCCSS has arrived already */
    if (isECMCCSSReceived) {
        LOGD("onCRINGReceived, isECMCCSSReceived = 1, return tmpSvcNotify");
        RIL_UNSOL_RESPONSE (RIL_UNSOL_SUPP_SVC_NOTIFICATION, &tmpSvcNotify,
                sizeof(RIL_SuppSvcNotification), tmpSvcNotifyRid);
        resetECMCCSS();
    }
}

/* Need to be invoked by RIL-CC module */
void onECPI133Received() {
    resetECMCCSS();
}

/**
 * RIL_UNSOL_SUPP_SVC_NOTIFICATION
 *
 * Reports supplementary service related notification from the network.
 *
 * "data" is a const RIL_SuppSvcNotification *
 *
 */
void onLteSuppSvcNotification(char *s, RIL_SOCKET_ID rid)
{
    RIL_SuppSvcNotification svcNotify;
    char *line = s;
    int callId, service;
    char *rawString = NULL;
    char *tmpString = NULL;

    memset(&svcNotify, 0, sizeof(RIL_SuppSvcNotification));
    svcNotify.notificationType = 0;  /* MO: 0, MT: 1 */
    svcNotify.code = CODE_IMS_MO_UNCONDITIONAL_CF_ACTIVE;

    if (strStartsWith(s, "+ECMCCSS:")) {
       /*
        * +ECMCCSS=<call_id><service><raw_string>
        * call_id: The id(index) of current call which the URC is related to.
        * service: URC type -> 13  - Call Forwarding
        *                      256 - Outgoing call barring
        *                      259 - Call Waiting
        */

       /* +ECMCCSS */
       if (at_tok_start(&line) < 0) {
          LOGE("Parse +ECMCCSS fail(start token): %s\n", s);
          return;
       }

        /* call_id */
        if (at_tok_nextint(&line, &callId) < 0) {
            LOGE("Parse +ECMCCSS fail(call_id): %s\n", s);
            return;
        }

        /* service */
        if (at_tok_nextint(&line, &service) < 0) {
            LOGE("Parse +ECMCCSS fail(service): %s\n", s);
            return;
        }

        /* Raw string */
        rawString = line + 1;
        /* if (at_tok_nextstr(&line, &rawString) < 0) {
           LOGE("Parse +ECMCCSS fail(Raw string): %s\n", s);
           return;
        }*/

        LOGD("[onLteSuppSvcNotification] call_id = %d, service = %d, Raw string = %s\n",
                callId, service, encryptString(rawString));

        svcNotify.index = callId;

        switch (service) {
        case 13:        /* Call Forwarding */
               /* Refer to CODE_1_CALL_FORWARDED in SuppServiceNotification.java */
               svcNotify.code = CODE_IMS_MO_CALL_FORWARDED;
               /* Retrieve forwarded-to number or SIP URI */
               tmpString = (char*) calloc(1, 256 * sizeof(char));
               if (tmpString == NULL) {
                   LOGE("onLteSuppSvcNotification: tmpString malloc fail");
                   return;
               }
               retrieveCallForwardedToNumber(rawString, tmpString);
               svcNotify.number = tmpString;
               break;

           case 256:        /* Outgoing call barring */
               /* Reuse <code1> in +CSSI URC */
               svcNotify.code = CODE_IMS_MO_OUTGOING_CALLS_BARRED;
               break;

           case 259:        /* Call waiting */
               svcNotify.code = CODE_IMS_MO_CALL_IS_WAITING;     /* Reuse <code1> in +CSSI URC */
               /* Sometime rawString is empty, so do not need to check its content
               if (NULL == strstr(rawString, "call-waiting")) {
                  return;
               }
               */
               break;
        case 257:   /* Call Forwarded*/
            onECMCCSSReceived();
            svcNotify.notificationType = 1;
            svcNotify.code = CODE_IMS_MT_FORWARDED_CALL;
            break;
        }

        /* If it is a User Load, need to hide number informaiton */
        const char *tmpNumber = encryptString(svcNotify.number);
        LOGD("[onLteSuppSvcNotification] \
                    svcNotify.notificationType = %d, svcNotify.code = %d, svcNotify.number = %s\n",
                    svcNotify.notificationType, svcNotify.code, tmpNumber);
        free((char*)tmpNumber);

        if (service == 257) {
            /* CRING has arrived already, send SSN to RILJ */
            if (isCRINGReceived) {
                LOGD("ECMCCSS and CRING have been received! Send SSN!");
                RIL_onUnsolicitedResponse(RIL_UNSOL_SUPP_SVC_NOTIFICATION, &svcNotify,
                        sizeof(RIL_SuppSvcNotification), rid);
                resetECMCCSS();
            } else {
                /* CRING has not arrived yet, copy SSN object "svcNotify" to "tmpSvcNotify" */
                LOGD("CRING not received, keep svcNotify and rid until CRING comes");
                memcpy(&tmpSvcNotify, &svcNotify, sizeof(RIL_SuppSvcNotification));
                tmpSvcNotifyRid = rid;
            }

            return;
        }
    }

    RIL_onUnsolicitedResponse(RIL_UNSOL_SUPP_SVC_NOTIFICATION, &svcNotify,
            sizeof(RIL_SuppSvcNotification), rid);

    if (tmpString != NULL) {
        free(tmpString);
    }
}

/**
 * Retrieve call forwarded-to number or sip uri from History-Info(rawString).
 *
 * @param rawString Contains History-Info of SIP response
 * @param number    To store the last forwarded-to number or sip uri
 *
 */
void retrieveCallForwardedToNumber(char *rawString, char *number)
{
    char *begin = strchr(rawString, ':');
    char *end, *end1;
    char tmpStr[256];
    int len;

    /* For example:
     * History-Info: <sip:UserB@ims.example.com>;index=1, <sip:UserC@ims.example.com?Reason=SIP;cause=302;text="CDIV">;index=1.1,
     * <sip:UserD@ims.example.com?Reason=SIP;cause=302;text="CDIV">;index=1.2
     */

    /* Find ':' of History-Info */
    if (begin == NULL) {
        return;
    }

    /* Find each number or sip uri */
    end = begin;
    while ((begin != NULL) && (end != NULL)) {
        end = strchr(begin, '>');
        begin = strchr(begin, '<');
        if ((begin != NULL) && (end != NULL)) {
              len = end - begin - 1;
            if ((len > 0) && (len < 256)) {
                strncpy(tmpStr, begin+1, len);
                tmpStr[len] = '\0';
                strncpy(number, tmpStr, strlen(tmpStr) + 1);
                /* Extract number of SIP uri only */
                if (strStartsWith(tmpStr, "sip:") || strStartsWith(tmpStr, "tel:")) {
                    strncpy(number, tmpStr + 4, strlen(tmpStr + 4) + 1);
                    end1 = strchr(number, '?');
                    if (end1 == NULL) {
                        end1 = strchr(number, ';');
                    }
                    if (end1 != NULL) {
                        number[end1 - number] = '\0';
                    }
                }
                const char *tmpNumber = encryptString(number);
                LOGD("[retrieveCallForwardedToNumber]number = %s", tmpNumber);
                free((char*)tmpNumber);
            }
            begin = end + 1;
        }
    }
}

void updateCFUQueryType(const char *cmd)
{
    int fd;
    int n;
    struct env_ioctl en_ctl;
    char *name = NULL;
    char *value = NULL;

    do {
        value = calloc(1, BUF_MAX_LEN);
        if (value == NULL) {
            RLOGE("malloc value fail");
            break;
        }
        value[0] = *(cmd+8);

        property_set(SETTING_QUERY_CFU_TYPE, value);

        name = calloc(1, BUF_MAX_LEN);
        if (name == NULL) {
            RLOGE("malloc name fail");
            free(value);
            break;
        }

        memset(&en_ctl, 0x00, sizeof(struct env_ioctl));

        fd = open("/proc/lk_env", O_RDWR);

        if (fd < 0) {
            RLOGE("ERROR open fail %d\n", fd);
            free(name);
            free(value);
            break;
        }

        if (BUF_MAX_LEN > strlen(SETTING_QUERY_CFU_TYPE)) {   // Coverity
            strncpy(name, SETTING_QUERY_CFU_TYPE, strlen(SETTING_QUERY_CFU_TYPE));
            name[strlen(SETTING_QUERY_CFU_TYPE)] = 0;
        } else {
            strncpy(name, SETTING_QUERY_CFU_TYPE, BUF_MAX_LEN - 1);
            name[BUF_MAX_LEN - 1] = 0;
        }

        en_ctl.name = name;
        en_ctl.value = value;
        en_ctl.name_len = strlen(name)+1;

        en_ctl.value_len = strlen(value)+1;
        LOGD("write %s = %s\n", name, value);
        n = ioctl(fd, ENV_WRITE, &en_ctl);
        if (n<0) {
            printf("ERROR write fail %d\n", n);
        }
        free(name);
        free(value);
        close(fd);
    } while (0);
}

extern int rilSsMain(int request, void *data, size_t datalen, RIL_Token t)
{
    switch (request)
    {
        case RIL_REQUEST_SEND_USSD:
            requestSendUssdDomainSelect(RIL_REQUEST_SEND_USSD, data, datalen, t);
        break;
        case RIL_REQUEST_CANCEL_USSD:
            requestCancelUssdDomainSelect(RIL_REQUEST_CANCEL_USSD, data, datalen, t);
        break;
        case RIL_REQUEST_GET_CLIR:
            requestGetClir(data, datalen, t);
        break;
        case RIL_REQUEST_SET_CLIR:
            requestSetClir(data, datalen, t);
        break;
        case RIL_REQUEST_QUERY_CALL_FORWARD_STATUS:
            requestQueryCallForwardStatus(data, datalen, t);
        break;
        case RIL_REQUEST_SET_CALL_FORWARD:
            requestSetCallForward(data, datalen, t);
        break;
        case RIL_REQUEST_QUERY_CALL_WAITING:
            requestQueryCallWaiting(data, datalen, t);
        break;
        case RIL_REQUEST_SET_CALL_WAITING:
            requestSetCallWaiting(data, datalen, t);
        break;
        case RIL_REQUEST_CHANGE_BARRING_PASSWORD:
            requestChangeBarringPassword(data, datalen, t);
        break;
        case RIL_REQUEST_QUERY_CLIP:
            requestQueryClip(data, datalen, t);
        break;
        case RIL_REQUEST_SET_SUPP_SVC_NOTIFICATION:
            requestSetSuppSvcNotification(data, datalen, t);
        break;
        /* MTK proprietary start */
        case RIL_REQUEST_SET_CLIP:
            requestSetClip(data, datalen, t);
        break;
        case RIL_REQUEST_GET_COLP:
            requestGetColp(data, datalen, t);
        break;
        case RIL_REQUEST_GET_COLR:
            requestGetColr(data, datalen, t);
        break;
        case RIL_REQUEST_SEND_CNAP:
            requestSendCnap(data, datalen, t);
        break;
        case RIL_REQUEST_SEND_USSI:
            requestSendUssdDomainSelect(RIL_REQUEST_SEND_USSI, data, datalen, t);
        break;
        case RIL_REQUEST_CANCEL_USSI:
            requestCancelUssdDomainSelect(RIL_REQUEST_CANCEL_USSI, data, datalen, t);
        break;
        case RIL_REQUEST_SET_SS_PROPERTY:
            requestSetSuppServProperty(data,datalen,t);
        break;
        /* MTK proprietary end */
        default:
            return 0; /* no matched request */
        break;
    }
    return 1; /* request found and handled */
}


extern int rilSsUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx *p_channel)
{
    UNUSED(sms_pdu);
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);

    if (strStartsWith(s, "+CUSD:")) {
        onUssd((char *) s, rid);
        return 1;
    } else if (strStartsWith(s,"+ECFU:")) {
        LOGD("Call Forwarding Flag: %s", s);
        onCfuNotify((char *) s, rid);
        return 1;
    } else if (strStartsWith(s, "+ECMCCSS:")) {
        LOGD("LTE SS Runtime Indication URC: %s", encryptString(s));
        onLteSuppSvcNotification((char *) s, rid);
        return 1;
    } else if (strStartsWith(s, "+CCWA:")) {
        callWaiting = 1;
        return 1;
    /// M: Process USSI AT command @{
    } else if (strStartsWith(s,"+EIUSD:")) {
        onUssi((char *)s,rid);
        return 1;
    }
    /// @}
    return 0;
}

/**
 * The API is used for FDN check and emergnecy number approved.
 */
static int isFdnAllowed(const char* ussd, RIL_SOCKET_ID rid) {
    ATResponse *p_response = NULL;
    int err;
    char *cmd = NULL;
    char *line = NULL;
    RIL_Errno ret = RIL_E_INTERNAL_ERR;
    int responses[2] = {0};

    if (ussd == NULL || strlen(ussd) == 0) {
        LOGD("isFdnAllowed: Null parameters.");
        goto error;
    }

    /**
     * AT+EAPPROVE=<dial_number>
     * <dial_number>: string, dialing number.
     */
    asprintf(&cmd, "AT+EAPPROVE=\"%s\"", ussd);
    err = at_send_command_singleline(cmd, "+EAPPROVE:",
            &p_response, getChannelCtxbyProxy());
    free(cmd);

    if (err < 0 || NULL == p_response) {
        LOGE("isFdnAllowed Fail");
        goto error;
    }

    switch (at_get_cme_error(p_response)) {
        case CME_SUCCESS:
            break;
        default:     // AT CMD format error, should not be here
            goto error;
    }

    if (p_response->p_intermediates != NULL) {
        line = p_response->p_intermediates->line;

        if (at_tok_start(&line) < 0) {
            goto error;
        }

        /**
         * <is_allowed> : integer
         * 0   The number is not allowed
         * 1   The number is allowed
         */
        if (at_tok_nextint(&line, &responses[0]) < 0) {
            goto error;
        }

        /**
         * <is_emergency> : integer
         * 0   The number is not emergnecy number
         * 1   The number is emergency number
         */
        if (at_tok_nextint(&line, &responses[1]) < 0) {
           goto error;
        }
    }

    /* return success here */
    ret = RIL_E_SUCCESS;

error:
    at_response_free(p_response);
    if (ret == RIL_E_SUCCESS) {
        if (responses[1] == 0) {       // not an ECC number, determined by MD PHB
            return (responses[0] == 1) ? 1 : 0;
        } else if (responses[1] == 1) { // Approve if it is an ECC number
            return 1;
        }
    }
    return 1;                       // Approve if we get any kind of CME error
}

/**
 * The API is used for IMS VoPS check.
 */
static int isVopsOn(RIL_SOCKET_ID rid) {
    ATResponse *p_response = NULL;
    int err;
    char *line = NULL;
    RIL_Errno ret = RIL_E_INTERNAL_ERR;
    int responses[2] = {0};

    /**
     * Query IMS network reporting
     * AT+CIREP?
     * +CIREP: <reporting>,<nwimsvops>
     */
    err = at_send_command_singleline("AT+CIREP?", "+CIREP:",
            &p_response, getChannelCtxbyProxy());

    if (err < 0 || NULL == p_response) {
        LOGE("isVopsOn Fail");
        goto error;
    }

    switch (at_get_cme_error(p_response)) {
        case CME_SUCCESS:
            break;
        default:     // AT CMD format error, should not be here
            goto error;
    }

    if (p_response->p_intermediates != NULL) {
        line = p_response->p_intermediates->line;

        if (at_tok_start(&line) < 0) {
            goto error;
        }

        /**
         * <reporting> : integer
         * Enables or disables reporting of changes in the IMSVOPS supported
         * indication received from the network and reporting of PS to CS SRVCC,
         * PS to CS vSRVCC and CS to PS SRVCC handover information.
         *
         * 0   Disable reporting
         * 1   Enable reporting
         */
        if (at_tok_nextint(&line, &responses[0]) < 0) {
            goto error;
        }

        /**
         * <nwimsvops> : integer
         * Gives the last IMS Voice Over PS sessions (IMSVOPS) supported
         * indication received from network.
         *
         * 0   IMSVOPS support indication is not received from network, or is negative
         * 1   IMSVOPS support indication as received from network is positive
         */
        if (at_tok_nextint(&line, &responses[1]) < 0) {
           goto error;
        }
    }

    /* return success here */
    ret = RIL_E_SUCCESS;

error:
    at_response_free(p_response);
    if (ret == RIL_E_SUCCESS) {
        return (responses[1] == 1) ? 1 : 0;
    }
    return 1;    // Assume VoPS is on by default
}

/**
 * The API is used for IMS registration status check.
 */
static int isImsRegOn(RIL_SOCKET_ID rid) {
    ATResponse *p_response = NULL;
    int err;
    int skip;
    char *line = NULL;
    RIL_Errno ret = RIL_E_INTERNAL_ERR;
    int responses[2]={0};

    /**
     * Query IMS registration information
     * AT+CIREG?
     * +CIREG: <n>,<reg_info>[,<ext_info>]
     */
    err = at_send_command_singleline("AT+CIREG?", "+CIREG:",
            &p_response, getChannelCtxbyProxy());

    if (err < 0 || NULL == p_response) {
        LOGE("isImsRegOn Fail");
        goto error;
    }

    switch (at_get_cme_error(p_response)) {
        case CME_SUCCESS:
            break;
        default:     // AT CMD format error, should not be here
            goto error;
    }

    if (p_response->p_intermediates != NULL) {
        line = p_response->p_intermediates->line;

        if (at_tok_start(&line) < 0) {
            goto error;
        }

        /**
         * <mode>
         */
        if (at_tok_nextint(&line, &skip) < 0) {
            goto error;
        }

        if (skip < 0) {
            LOGE("The <mode> is an invalid value!!!");
            goto error;
        } else {
            /**
            * <reg_info> : integer
            * Indicates the IMS registration status. The UE is seen as registered as long as
            * one or more of its public user identities are registered with any of its
            * contact addresses
            *
            * 0   not registered
            * 1   registered
            */
            if (at_tok_nextint(&line, &responses[0]) < 0) {
                goto error;
            }

            responses[1] = 1;  // RADIO_TECH_3GPP
        }

        at_response_free(p_response);
        return (responses[0] == 1) ? 1 : 0;
    }

error:
    LOGE("There is something wrong with the AT+CIREG?, return false for isImsRegOn()");
    at_response_free(p_response);
    return 0;
}

static int isInImsCall(RIL_SOCKET_ID rid) {
    int isImsOn = isImsRegOn(rid);
    LOGD("isInImsCall: inCallNumberPerSim[rid] = %d, isImsOn = %s",
            inCallNumberPerSim[rid], (isImsOn == 1) ? "True" : "False");

    if (inCallNumberPerSim[rid] > 0 && isImsOn) {
        return 1;
    } else {
        return 0;
    }
}

static int isWfcRegOn(RIL_SOCKET_ID rid) {
    int phoneId = getMappingSIMByCurrentMode(rid);
    char wfcState[PROPERTY_VALUE_MAX] = {0};
    getMSimProperty(phoneId, "persist.vendor.radio.wfc_state", wfcState);

    LOGD("wfcState = %s", wfcState);
    return atoi(wfcState);
}

static UssiAction getUssiAction() {
    LOGD("getUssiAction(): mUssiAction = %s", ussiActionToString(mUssiAction));
    return mUssiAction;
}

static void setUssiAction(UssiAction action) {
    LOGD("setUssiAction(): %s -> %s", ussiActionToString(mUssiAction),
            ussiActionToString(action));
    if (mUssiAction == action) {
        return;
    }
    mUssiAction = action;
}

static const char *ussiActionToString(UssiAction action) {
    switch (action) {
        case USSI_REQUEST:
            return "USSI_REQUEST";
        case USSI_RESPONSE:
            return "USSI_RESPONSE";
        default:
            // not possible here!
            return NULL;
    }
}

static void requestSendUssdDomainSelect(int request, void *data, size_t datalen, RIL_Token t) {
    const char* ussd = (const char*) data;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(SS_CHANNEL_CTX);

    // Check FDN
    //    Modem IMC / Call UA do not check FDN, AP need to make sure if FDN allow this
    //    USSD code or not by itself. (Actually, modem L4 does check FDN. But here, AP checks
    //    it anyway no matter what the domain it is)
    if (!isFdnAllowed(ussd, rid)) {
        LOGD("requestSendUssdDomainSelect: it's not allowed by FDN check");
        sendFailureReport(request, data, datalen, t, RIL_E_FDN_CHECK_FAILURE);
        return;
    }

    if (isImsRegOn(rid)) {
        if (isWfcRegOn(rid)) {
            // VoPS works only for cellular netowrk.
            // If UE is registered on VoWiFi, no need to check VoPS.
            LOGD("requestSendUssdDomainSelect: requestSendUSSI() over VoWiFi");
            requestSendUSSI(request, data, datalen, t);
        } else {
            // Check VoPS.
            //    From spec IR.92 D.5.2, the domain selection rules follow voice call.
            //    So, if VoPS is disabled, we should not try USSD over IMS.
            if (!isVopsOn(rid)) {
                LOGD("requestSendUssdDomainSelect: it's not allowed by VoPS check, CSFB");
                requestSendUSSD(request, data, datalen, t, SEND_RESPONSE_BACK);
            } else {
                LOGD("requestSendUssdDomainSelect: requestSendUSSI() over VoLTE");
                requestSendUSSI(request, data, datalen, t);
            }
        }
    } else {
        LOGD("requestSendUssdDomainSelect: requestSendUSSD()");
        requestSendUSSD(request, data, datalen, t, SEND_RESPONSE_BACK);
    }
}

static void requestCancelUssdDomainSelect(int request, void *data, size_t datalen, RIL_Token t) {
    UNUSED(request);
    LOGD("requestCancelUssdDomainSelect: mOngoingSessionDomain: %s",
            (mOngoingSessionDomain == 0) ? "CS" : "IMS");
    if (mOngoingSessionDomain == 1) {
        requestCancelUssi(data, datalen, t);
    } else {
        requestCancelUssd(data, datalen, t);
    }

    // Reset the flag
    mOngoingSessionDomain = 0;
}

static void handleOnUssi(const char **data, size_t datalen, RIL_SOCKET_ID rid) {
    int ussdMode = convertUssiToUssdMode(data, datalen, rid);
    char **finalData = (char **)malloc(2 * sizeof(char *));
    assert(finalData != NULL);

    asprintf(&finalData[0], "%d", ussdMode);  // USSD mode
    asprintf(&finalData[1], "%s", data[2]);   // USSD string

    /**
     * USSD mode meanings
     *  0 : USSD_MODE_NOTIFY (0)
     *  1 : USSD_MODE_REQUEST (1)
     *  2 : USSD_MODE_NW_RELEASE (2)
     *  3 : USSD_MODE_LOCAL_CLIENT (3)
     *  4 : USSD_MODE_NOT_SUPPORTED (4)
     *  5 : USSD_MODE_NW_TIMEOUT (5)
     *  6 : USSD_MODE_NOT_SUPPORTED_NO_CSFB (6)
     */

    if (ussdMode != 4) {
        LOGD("handleOnUssi, no need to do CSFB, ussdMode = %d", ussdMode);

        if (ussdMode == 1) {
            LOGD("handleOnUssi, further user action required, mOngoingSessionDomain: IMS");
            mOngoingSessionDomain = 1;
        } else if (ussdMode == 6) {
            LOGD("handleOnUssi, convert the ussd mode back to 4");
            ussdMode = 4;
        }

        RIL_onUnsolicitedResponse(
                RIL_UNSOL_ON_USSD,
                finalData, 2 * sizeof(char *),
                rid);
        free(finalData[0]);
        free(finalData[1]);
        free(finalData);
    } else {
        RIL_requestProxyTimedCallback(handleUssiCSFB, (void *)finalData, &TIMEVAL_0S,
                getRILChannelId(RIL_SS, rid), "handleUssiCSFB");
    }
}

static void handleUssiCSFB(void *param) {
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getChannelCtxbyProxy());
    char **finalData = (char **)param;

    if (isInImsCall(rid) && !isWfcRegOn(rid)) {
        LOGD("handleUssiCSFB, do not CSFB, isInImsCall = true");
        RIL_onUnsolicitedResponse(
                RIL_UNSOL_ON_USSD,
                finalData, 2 * sizeof(char *),
                rid);
    } else if (mUssiSnapshot != NULL && mUssiToken != NULL) {
        LOGD("handleUssiCSFB, mUssiSnapshot = %s, mUssiToken = %d", mUssiSnapshot, mUssiToken);

        requestSendUSSD(RIL_REQUEST_SEND_USSD, mUssiSnapshot, strlen(mUssiSnapshot),
                mUssiToken, SEND_URC_BACK);
        mUssiToken = NULL;
    } else {
        LOGE("handleUssiCSFB fails, mUssiSnapshot or mUssiToken is NULL");
    }

    if (finalData != NULL) {
        if (finalData[0] != NULL) {
            free(finalData[0]);
        }
        if (finalData[1] != NULL) {
            free(finalData[1]);
        }
        free(finalData);
    }
}

static int convertUssiToUssdMode(const char **data, size_t datalen, RIL_SOCKET_ID rid) {
    int newUssdMode = 0;

    /**
     * USSI response from the network, or network initiated operation
     * +EIUSD: <m>,<n>,<str>,<lang>,<error_code>,<alertingpattern>,<sip_cause>
     * <m>:
     *    1   USSD notify
     *    2   SS notify
     *    3   MD execute result
     * <n>:  if m=1
     *    0   no further user action required
     *    1   further user action required
     *    2   USSD terminated by network
     *    3   other local client has responded
     *    4   operation not supported
     *    5    network time out
     * <n>:if m=3, value is return value of MD
     *    0   execute success
     *    1   common error
     *    2   IMS unregistered
     *    3   IMS busy
     *    4   NW error response
     *    5   session not exist
     *    6   NW not support(404)
     * <str>: USSD/SS string
     * <lang>: USSD language
     * <error_code> USSD error code in xml
     * <alertingpattern> alerting pattern of NW initiated INVITE
     * <sip_cause> sip error code
     */
    const char *m = data[0];
    const char *n = data[1];
    const char *error_code = data[4];

    LOGD("convertUssiToUssdUrc: m = %s, n = %s", m, n);

    // Initialization
    setUssiAction(USSI_REQUEST);  // Every time UE receives an USSI URC from modem,
                                  // we recover the state of mUssiAction to USSI_REQUEST.
                                  // If we found the EIUSD URC is "further user action required",
                                  // then change it to USSI_RESPONSE.
    newUssdMode = 4;  // Set USSD mode as general USSD error (4) first,
                      // e.g., not supported, time out, network error

    // USSD mode
    if (strcmp(m, "1") == 0) {
        if (strcmp(n, "0") == 0 || strcmp(n, "2") == 0) {
            newUssdMode = 0;  // USSD_MODE_NOTIFY (0)
                              // <m> = 1, <n> = 0, no further action
                              // <m> = 1, <n> = 2, terminated by network
        } else if (strcmp(n, "1") == 0) {
            newUssdMode = 1;  // USSD_MODE_REQUEST (1)
                              // <m> = 1, <n> = 1, further action required
            setUssiAction(USSI_RESPONSE);  // Pull up the flag to let USSD domain selector
                                           // know the action of "next" received USSD
                                           // is a response, not a request
        } else if (strcmp(n, "4") == 0) {
            /**
            * If AP receives <error_code> != 0 from modem when <m> = 1 & <n> = 4,
            * no need to CSFB
            *
            * From TS 24.390:
            * <error-code> is an integer. The following values are defined.
            * If the received value is not listed below, it must be treated as 1.
            * 1    error - unspecified
            * 2    language/alphabet not supported
            * 3    unexpected data value
            * 4    USSD-busy
            */
            if (strcmp(error_code, "0") != 0) {
                newUssdMode = 6;  // USSD_MODE_NOT_SUPPORTED_NO_CSFB (6)
                                  // <m> = 1, <n> = 4, <error-code> != 0
                                  // It means modem UA get normal response
                                  // from IMS server, no need to CSFB
            } else {
                newUssdMode = 4;  // USSD_MODE_NOT_SUPPORTED (4)
                                  // <m> = 1, <n> = 4, <error-code> = 0
                                  // operation not supported
            }
        } else if (strcmp(n, "5") == 0) {
            newUssdMode = 5;  // USSD_MODE_NW_TIMEOUT (5)
                              // <m> = 1, <n> = 5, netowrk time out
            setUssiAction(USSI_REQUEST);   // Reset the flag to let USSD domain selector
                                           // know the action of "next" received USSD
                                           // is a request, not a response. Because the current
                                           // USSI session is over
        }
    } else if (strcmp(m, "3") == 0) {
        if (strcmp(n, "0") == 0) {
            newUssdMode = 0;  // USSD_MODE_NOTIFY (0)
                              // <m> = 3, <n> = 5, execute success
        }
    }

    LOGD("convertUssiToUssdUrc: return ussdMode = %d", newUssdMode);

    return newUssdMode;
}

/**
 * Some uppper USSI Framework (ex. Q version of AOSP) only accept SUCCES in request phase.
 * To adapt to this design in this case, we return SUCCESS first and then generate an URC
 * to report the FAILIRUE if any error occurs during request phase.
 */
static void sendFailureReport(int request, void *data, size_t datalen, RIL_Token t, RIL_Errno ret) {
    UNUSED(datalen);
    char *genericUssdFail[2] = {(char *) "4", (char *) ""};  // Generate a generic failure USSD URC

    LOGD("sendFailureReport, ret = %d", ret);

    // For FDN check failure, we "directly" return FDN error cause when the request comes from
    // GSM pipe (ex. AOSP's GsmMmiCode). Because if USSD request is sent from GSM pipe, it can
    // accept error during request phase. And we should report it to let UI shows correspondnig
    // error meesage
    if (request == RIL_REQUEST_SEND_USSD && ret == RIL_E_FDN_CHECK_FAILURE) {
        // Let the UI have time to show up the "USSD code running" dialog
        sleep(1);

        RIL_onRequestComplete(t, ret, NULL, 0);
        return;
    }

    // Return SUCCESS first
    LOGD("sendFailureReport, Return SUCCESS first by response");
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);

    // Let the UI have time to show up the "USSD code running" dialog
    sleep(1);

    // And then report the FAILIRUE by URC
    LOGD("sendFailureReport, Report the FAILIRUE by URC");
    RIL_onUnsolicitedResponse(
            RIL_UNSOL_ON_USSD,
            genericUssdFail,
            2 * sizeof(char *),
            getRILIdByChannelCtx(SS_CHANNEL_CTX));
}

static char* encryptString(char* input) {
    if (NULL == input) {
        return NULL;
    }
    int len = strlen(input);

    int pivot = (rand() % 9) + 1;
    char* array = (char *) malloc(len + 2);

    if (array == NULL) {
        LOGE("encryptString, malloc failed!");
        return NULL;
    }

    strncpy(array, input, len);
    for (int i = 0; i < len; i++) {
        array[i] = (char) (array[i] + pivot);
    }
    char pivotStr[16] = {0};
    sprintf(pivotStr, "%d", pivot);
    array[len] = pivotStr[0];
    array[len + 1] = '\0';
    return array;
}
