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

#include "GsmUtil.h"
#include "SSUtil.h"
#include "SuppServDef.h"
#include "rfx_properties.h"
#include "RfxLog.h"

#include <assert.h>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <telephony/mtk_ril.h>

#ifdef RFX_LOG_TAG
#undef RFX_LOG_TAG
#endif
#define RFX_LOG_TAG "SS-UTIL"

/***
 * "AO"  BAOC (Barr All Outgoing Calls) (refer 3GPP TS 22.088 [6] clause 1)
 * "OI"  BOIC (Barr Outgoing International Calls) (refer 3GPP TS 22.088 [6] clause 1)
 * "OX"  BOIC exHC (Barr Outgoing International Calls except to Home Country) (refer 3GPP TS 22.088 [6] clause 1)
 * "AI"  BAIC (Barr All Incoming Calls) (refer 3GPP TS 22.088 [6] clause 2)
 * "IR"  BIC Roam (Barr Incoming Calls when Roaming outside the home country) (refer 3GPP TS 22.088 [6] clause 2)
 * "AB"  All Barring services (refer 3GPP TS 22.030 [19]) (applicable only for <mode>=0)
 * "AG"  All outGoing barring services (refer 3GPP TS 22.030 [19]) (applicable only for <mode>=0)
 * "AC"  All inComing barring services (refer 3GPP TS 22.030 [19]) (applicable only for <mode>=0)
 * "ACR" Incoming Call Barring of Anonymous Communication Rejection (ACR) (3GPP TS 24.611)
 */
const char * callBarFacilityStrings[CB_SUPPORT_NUM] = {
    "AO",
    "OI",
    "OX",
    "AI",
    "IR",
    "AB",
    "AG",
    "AC",
    "ACR"
};

const char * callBarServiceCodeStrings[CB_SUPPORT_NUM] = {
    "33",
    "331",
    "332",
    "35",
    "351",
    "330",
    "333",
    "353",
    "157"
};

const char * GsmCbsDcsStringp[MAX_DCS_SUPPORT] = {"GSM7","8BIT","UCS2"};

const char *InfoClassToMmiBSCodeString (AtInfoClassE infoClass) {
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

    switch((int) infoClass)
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
            RFX_LOG_E(RFX_LOG_TAG, "RILD unknown infoClass: %d", infoClass);
            break;
    }
    return "";
}

const char PAUSE = ',';
const char WAIT = ';';
const char WILD = 'N';

// No use in 93.
int MmiBSCodeToInfoClassX(int serviceCode, int ssServiceClassFeature) {
    if (ssServiceClassFeature == 1) {
    // RFX_LOG_D(RFX_LOG_TAG, "[MmiBSCodeToInfoClassX]Return %d directly.", serviceCode);
        return serviceCode;
    } else {
        switch (serviceCode) {
            /* BS_ALL_E = BS_TELE_ALL_E + BS_DATA_ALL_E */
            case BS_ALL_E:
                return (int)(CLASS_SMS + CLASS_FAX + CLASS_VOICE + CLASS_DATA_ASYNC + CLASS_DATA_SYNC);
            case BS_TELE_ALL_E:
                return (int)(CLASS_SMS + CLASS_FAX + CLASS_VOICE);
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

const char *ssStatusToOpCodeString(SsStatusE status)
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

char* convertToUCS2(char* data)
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
    const char* string8 = (const char *)(data);

    ucs2String = (bytes_t) calloc(2*(MAX_RIL_USSD_STRING_LENGTH+1),sizeof(char));
    if (ucs2String == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "convertToUCS2:ucs2String malloc fail");
        return NULL;
    }
    // memory overwrite if strlen(string8) is larger than MAX_RIL_USSD_STRING_LENGTH
    stringlen = utf8_to_ucs2((cbytes_t)string8, MIN(strlen(string8),
            MAX_RIL_USSD_STRING_LENGTH), ucs2String);
    cbytes_t test;
    ucs2HexString = (bytes_t) calloc(2*stringlen*2+1,sizeof(char));
    if (ucs2HexString == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "convertToUCS2:ucs2HexString malloc fail");
        // free here in the case of ucs2HexString is null.
        free(ucs2String);
        return NULL;
    }

    gsm_hex_from_bytes((char *)ucs2HexString, ucs2String, 2*stringlen);

    free(ucs2String);

    RFX_LOG_W(RFX_LOG_TAG, "convertToUCS2= %s", ucs2HexString);

    return (char *)ucs2HexString;
}

const char * callBarFacToServiceCodeStrings(const char * fac)
{
    int i;

    for (i = 0; i < CB_SUPPORT_NUM; i++) {
        if (0 == strcmp(fac, callBarFacilityStrings[i])) {
            break;
        }
    }

    if (i < CB_SUPPORT_NUM) {
        return callBarServiceCodeStrings[i];
    } else {
        /* not found! return default */
        return callBarServiceCodeStrings[CB_ABS];
    }
}

GsmCbsDcsE checkCbsDcs(int dcs)
{
    GsmCbsDcsE result = DCS_GSM7;

    if ((dcs == 0x11) ||((dcs & 0x4C) == 0x48) ||((dcs & 0x9C) == 0x98)) {
        result = DCS_UCS2;
    } else if (((dcs & 0x4C) == 0x44) ||((dcs & 0x9C) == 0x94) ||((dcs & 0xF4) == 0xF4)) {
        result = DCS_8BIT;
    }

    return result;
}

const char *callForwardReasonToServiceCodeString(CallForwardReasonE cfreason)
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
        case CF_NOTREGIST:
            return CALL_FORWARD_NOT_REGISTERED;
            break;
        default:
            return CALL_FORWAED_NONE;
            break;
    }
}

void clearErrorMessageFromXcap(int slotId) {
    char *propName = NULL;
    char propValue[MTK_PROPERTY_VALUE_MAX] = {0};
    int idx = 0;

    vector<string> propNames(50);
    if (asprintf(&propName, "%s.%d",
            (char *) PROPERTY_ERROR_MESSAGE_FROM_XCAP, slotId) < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "Allocate prop name failed");
        return;
    }

    char *tmpPropName = NULL;
    asprintf(&tmpPropName, "%s.%d", (char*) propName, idx);

    propNames[idx] = string(tmpPropName);

    rfx_property_get(propNames[idx].c_str(), propValue, "");
    while(strlen(propValue) != 0) {
        rfx_property_set(propNames[idx].c_str(), "");

        RFX_LOG_D(RFX_LOG_TAG, "clear prop: [%s]: [%s]", propNames[idx].c_str(), propValue);

        rfx_property_get(propNames[idx].c_str(), propValue, "");
        RFX_LOG_D(RFX_LOG_TAG, "after clear prop: [%s]: [%s]", propNames[idx].c_str(), propValue);

        idx++;
        propNames[idx] = string(propName) + string(".") + to_string(idx);
        rfx_property_get(propNames[idx].c_str(), propValue, "");
    }

    free(propName);
    free(tmpPropName);
}

void setErrorMessageFromXcap(int slotId, AT_CME_Error errorCode, const char* message) {
    char *fullMsg = NULL;

    if (message == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "setErrorMessageFromXcap: NULL string.");
        return;
    }
    int len = strlen(message);

    if (len == 0) {
        RFX_LOG_E(RFX_LOG_TAG, "setErrorMessageFromXcap: strin length is 0.");
        return;
    }

    switch (errorCode) {
        case CME_409_CONFLICT:
            break;
        default:
            return;
    }

    asprintf(&fullMsg, "%d;%s", errorCode, message);

    len = strlen(fullMsg);

    char *propName = NULL;

    asprintf(&propName, "%s.%d", (char *) PROPERTY_ERROR_MESSAGE_FROM_XCAP, slotId);

    RFX_LOG_D(RFX_LOG_TAG, "propName: [%s]", propName);
    RFX_LOG_D(RFX_LOG_TAG, "fullMsg: [%s]", fullMsg);

    int idx = 0;

    char *tmpMsg = fullMsg;

    while (len > 0) {
        RFX_LOG_D(RFX_LOG_TAG, "len: [%d], max: [%d]", len, MTK_PROPERTY_VALUE_MAX);

        char tmpStr[MTK_PROPERTY_VALUE_MAX] = {0};
        int copyLen = 0;
        if (len >= MTK_PROPERTY_VALUE_MAX - 1) {
            copyLen = MTK_PROPERTY_VALUE_MAX - 1;
        }

        if (len < MTK_PROPERTY_VALUE_MAX) {
            copyLen = len;
        }

        strncpy(tmpStr, tmpMsg, copyLen);
        tmpMsg += copyLen;
        // printf("%s\n", tmpStr);

        char *tmpPropName = NULL;

        asprintf(&tmpPropName, "%s.%d", (char*) propName, idx);

        // setMSimProperty(slotId,
        //         propName,
        //         tmpStr);

        RFX_LOG_D(RFX_LOG_TAG, "set xcap error sysprop: [%s][%s]", tmpPropName, tmpStr);
        rfx_property_set(tmpPropName, tmpStr);

        free(tmpPropName);

        idx++;
        len -= (MTK_PROPERTY_VALUE_MAX - 1);
    }

    RFX_LOG_D(RFX_LOG_TAG, "setErrorMessageFromXcap done.");
    free(fullMsg);
    free(propName);
}


/** True if c is ISO-LATIN characters 0-9, *, # , + */
bool isReallyDialable(char c) {
    return (c >= '0' && c <= '9') || c == '*' || c == '#' || c == '+';
}

/** This any anything to the right of this char is part of the
 *  post-dial string (eg this is PAUSE or WAIT)
 */
bool isStartsPostDial (char c) {
    return c == PAUSE || c == WAIT;
}

/**
 * Extracts the network address portion and canonicalize.
 *
 * This function is equivalent to extractNetworkPortion(), except
 * for allowing the PLUS character to occur at arbitrary positions
 * in the address portion, not just the first position.
 *
 */
char* extractNetworkPortionAlt(char* phoneNumber) {
    if (NULL == phoneNumber) {
        return NULL;
    }

    int len = strlen(phoneNumber);
    std::string tmp_result = "";
    char* result = (char *) malloc(len + 1);
    bool haveSeenPlus = false;

    if (result == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "extractNetworkPortionAlt, malloc failed!");
        return NULL;
    }

    for (int i = 0; i < len; i++) {
        if ('+' == phoneNumber[i]) {
            if (haveSeenPlus) {
                continue;
            }
            haveSeenPlus = true;
        }
        if (isReallyDialable(phoneNumber[i])) {
            tmp_result += phoneNumber[i];
        } else if (isStartsPostDial(phoneNumber[i])) {
            break;
        }
    }
    strncpy(result, tmp_result.c_str(), len);
    result[len] = '\0';
    return result;
}

char* encryptString(char* input) {
    if (NULL == input) {
        return NULL;
    }
    int len = strlen(input);

    int pivot = (rand() % 9) + 1;
    char* array = (char *) malloc(len + 2);

    if (array == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "encryptString, malloc failed!");
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
