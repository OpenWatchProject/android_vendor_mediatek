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

#include "ril_phb.h"
#include <pthread.h>

#undef LOG_TAG
#define LOG_TAG "RIL-PHB"

#define SIM_CHANNEL_CTX getRILChannelCtxFromToken(t)
extern int isModemResetStarted;

int current_phb_storage[4] = {-1, -1, -1, -1};
void requestQueryPhbInfo(void *data, size_t datalen, RIL_Token t);
void requestClearPhbEntry(int index, RIL_Token t);
void requestWritePhbEntry(void *data, size_t datalen, RIL_Token t);
void requestReadPhbEntry(void *data, size_t datalen, RIL_Token t);
void requestQueryUPBCapability(void *data, size_t datalen, RIL_Token t);
void requestEditUPBEntry(void *data, size_t datalen, RIL_Token t);
void requestDeleteUPBEntry(void *data, size_t datalen, RIL_Token t);
void requestReadGasList(void *data, size_t datalen, RIL_Token t);
void requestReadUpbGrpEntry(void *data, size_t datalen, RIL_Token t);
void requestWriteUpbGrpEntry(void *data, size_t datalen, RIL_Token t);
void requestGetPhoneBookStringsLength(void *data, size_t datalen, RIL_Token t);
void requestGetPhoneBookMemStorage(void *data, size_t datalen, RIL_Token t);
void requestSetPhoneBookMemStorage(void *data, size_t datalen, RIL_Token t);
void loadUPBCapability(RIL_Token t);
void requestReadPhoneBookEntryExt(void *data, size_t datalen, RIL_Token t);
void requestWritePhoneBookEntryExt(void *data, size_t datalen, RIL_Token t);
void requestQueryUPBAvailable(void *data, size_t datalen, RIL_Token t);
void requestReadUPBEmail(void *data, size_t datalen, RIL_Token t);
void requestReadUPBSne(void *data, size_t datalen, RIL_Token t);
void requestReadUPBAnr(void *data, size_t datalen, RIL_Token t);
void requestReadAasList(void *data, size_t datalen, RIL_Token t);
void requestSetPhonebookReady(void *data, size_t datalen, RIL_Token t);

static pthread_mutex_t sPhbReadyPropertyMutex = PTHREAD_MUTEX_INITIALIZER;

static const char* getPhbStorageString(int type) {
    char* str = NULL;
    switch (type) {
        case RIL_PHB_ADN:
            str = "SM";
            break;
        case RIL_PHB_FDN:
            str = "FD";
            break;
        case RIL_PHB_MSISDN:
            str = "ON";
            break;
        case RIL_PHB_ECC:
            str = "EN";
            break;
    }

    return str;
}

int selectPhbStorage(int type, RIL_Token t) {
    ATResponse *p_response = NULL;
    const char *storage;
    char *cmd = NULL;
    int err = 0;
    int result = 1;
    const RIL_SOCKET_ID rid = getRILIdByChannelCtx(SIM_CHANNEL_CTX);

    if (current_phb_storage[rid] != type) {
        storage = getPhbStorageString(type);
        if (storage == NULL) {
            result = 0;
        }

        asprintf(&cmd, "AT+CPBS=\"%s\"", storage);
        err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);

        if (err < 0 || p_response->success == 0) {
            result = 0;
        }
        else {
            current_phb_storage[rid] = type;
        }
        free(cmd);
        at_response_free(p_response);
    }
    LOGD("Select the Storage: %d", rid);
    return result;
}

void onPhbStateChanged(RIL_SOCKET_ID rid, int isPhbReady) {
    LOGI("onPhbStateChanged rid=%d, isPhbReady=%d", rid, isPhbReady);

    if (SIM_COUNT >= 2) {
        LOGI("onPhbStateChanged isSimInserted=%d, isModemResetStarted=%d", isSimInserted(rid), isModemResetStarted);

        // MD may report +EIND: 2 during no sim, ignore it.
        // Always set PROPERTY_RIL_PHB_READY false when receive +EIND: 32.
        if (isSimInserted(rid) == 0 && isPhbReady == TRUE) {
            return;
        }

        if ((isPhbReady == TRUE) && (isModemResetStarted)) {
            return;
        }
    }

    if (isPhbReady == TRUE) {
        setMSimPropertyThreadSafe(rid, PROPERTY_RIL_PHB_READY, "true", &sPhbReadyPropertyMutex);
    } else {
        if (sizeof(current_phb_storage) / sizeof(current_phb_storage[0]) > rid) {
            current_phb_storage[rid] = -1;
        }
        setMSimPropertyThreadSafe(rid, PROPERTY_RIL_PHB_READY, "false", &sPhbReadyPropertyMutex);
    }

    RIL_onUnsolicitedResponse(RIL_UNSOL_PHB_READY_NOTIFICATION, &isPhbReady, sizeof(int), rid);
}

extern int rilPhbMain(int request, void *data, size_t datalen, RIL_Token t) {
    switch (request) {
        case RIL_REQUEST_QUERY_PHB_STORAGE_INFO:
            requestQueryPhbInfo(data, datalen, t);
            break;
        case RIL_REQUEST_WRITE_PHB_ENTRY:
            requestWritePhbEntry(data, datalen, t);
            break;
        case RIL_REQUEST_READ_PHB_ENTRY:
            requestReadPhbEntry(data, datalen, t);
            break;
        case RIL_REQUEST_QUERY_UPB_CAPABILITY:
            requestQueryUPBCapability(data, datalen, t);
            break;
        case RIL_REQUEST_EDIT_UPB_ENTRY:
            requestEditUPBEntry(data, datalen, t);
            break;
        case RIL_REQUEST_DELETE_UPB_ENTRY:
            requestDeleteUPBEntry(data, datalen, t);
            break;
        case RIL_REQUEST_READ_UPB_GAS_LIST:
            requestReadGasList(data, datalen, t);
            break;
        case RIL_REQUEST_READ_UPB_GRP:
            requestReadUpbGrpEntry(data, datalen, t);
            break;
        case RIL_REQUEST_WRITE_UPB_GRP:
            requestWriteUpbGrpEntry(data, datalen, t);
            break;
        case RIL_REQUEST_GET_PHB_STRING_LENGTH:
            requestGetPhoneBookStringsLength(data, datalen, t);
            break;
        case RIL_REQUEST_GET_PHB_MEM_STORAGE:
            requestGetPhoneBookMemStorage(data, datalen, t);
            break;
        case RIL_REQUEST_SET_PHB_MEM_STORAGE:
            requestSetPhoneBookMemStorage(data, datalen, t);
            break;
        case RIL_REQUEST_READ_PHB_ENTRY_EXT:
            requestReadPhoneBookEntryExt(data, datalen, t);
            break;
        case RIL_REQUEST_WRITE_PHB_ENTRY_EXT:
            requestWritePhoneBookEntryExt(data, datalen, t);
            break;
        case RIL_REQUEST_QUERY_UPB_AVAILABLE:
            requestQueryUPBAvailable(data, datalen, t);
            break;
        case RIL_REQUEST_READ_EMAIL_ENTRY:
            requestReadUPBEmail(data, datalen, t);
            break;
        case RIL_REQUEST_READ_SNE_ENTRY:
            requestReadUPBSne(data, datalen, t);
            break;
        case RIL_REQUEST_READ_ANR_ENTRY:
            requestReadUPBAnr(data, datalen, t);
            break;
        case RIL_REQUEST_READ_UPB_AAS_LIST:
            requestReadAasList(data, datalen, t);
            break;
        case RIL_REQUEST_SET_PHONEBOOK_READY:
            requestSetPhonebookReady(data, datalen, t);
        default:
            return 0; /* no match */
            break;
    }
    return 1;   /* request find */
}

extern int rilPhbUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx* p_channel) {
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);

    RIL_SIM_UNUSED_PARM(sms_pdu);

    if (strStartsWith(s, "+EIND: 2")) {
        onPhbStateChanged(rid, TRUE);
    } else if (strStartsWith(s, "+EIND: 32")) {
        onPhbStateChanged(rid, FALSE);
    } else {
        return 0;
    }

    return 1;
}


void resetPhbReady(RIL_SOCKET_ID rid) {
    int isPhbReady = FALSE;

    if (sizeof(current_phb_storage) / sizeof(current_phb_storage[0]) > rid) {
        current_phb_storage[rid] = -1;
    }

    setMSimProperty(rid, PROPERTY_RIL_PHB_READY, "false");
    RIL_onUnsolicitedResponse(RIL_UNSOL_PHB_READY_NOTIFICATION, &isPhbReady, sizeof(int), rid);
}

#define DLOGD(x...) if (s_isUserLoad != 1) LOGD( x )

#ifdef ESC_SUPPORT
#define SPEC_ENCODING_ARR_LEN 54
static unsigned short specialEncoding[SPEC_ENCODING_ARR_LEN][2] = {
#else
#define SPEC_ENCODING_ARR_LEN 44
static unsigned short specialEncoding[SPEC_ENCODING_ARR_LEN][2] = {
#endif
    {0x0040, 0x0000},  //@
    {0x00A3, 0x0001},
    {0x0024, 0x0002},
    {0x00A5, 0x0003},
    {0x00E8, 0x0004},
    {0x00E9, 0x0005},
    {0x00F9, 0x0006},
    {0x00EC, 0x0007},
    {0x00F2, 0x0008},
    {0x00C7, 0x0009},
    {0x0020, 0x0020},
    {0x00D8, 0x000B},
    {0x00F8, 0x000C},
    {0x0020, 0x0020},
    {0x00C5, 0x000E},
    {0x00E5, 0x000F},
    {0x0394, 0x0010},
    {0x005F, 0x0011},
    {0x03A6, 0x0012},
    {0x0393, 0x0013},
    {0x039B, 0x0014},
    {0x03A9, 0x0015},
    {0x03A0, 0x0016},
    {0x03A8, 0x0017},
    {0x03A3, 0x0018},
    {0x0398, 0x0019},
    {0x039E, 0x001A},
    {0x00C6, 0x001C},
    {0x00E6, 0x001D},
    {0x00DF, 0x001E},
    {0x00C9, 0x001F},
    {0x00A4, 0x0024},
    {0x00A1, 0x0040},
    {0x00C4, 0x005B},
    {0x00D6, 0x005C},
    {0x00D1, 0x005D},
    {0x00DC, 0x005E},
    {0x00A7, 0x005F},
    {0x00BF, 0x0060},
    {0x00E4, 0x007B},
    {0x00F6, 0x007C},
    {0x00F1, 0x007D},
    {0x00FC, 0x007E},
    {0x00E0, 0x007F},
#ifdef ESC_SUPPORT
    {0x0020, 0x0020},
    {0x005E, 0x1B14},
    {0x007B, 0x1B28},
    {0x007D, 0x1B29},
    {0x005C, 0x1B2F},
    {0x005B, 0x1B3C},
    {0x007E, 0x1B3D},
    {0x005D, 0x1B3E},
    {0x007C, 0x1B40},
    {0x20AC, 0x1B65}
#endif
};

// rilprotect start

#define RIL_E_DIAL_STRING_TOO_LONG RIL_E_OEM_ERROR_1
#define RIL_E_TEXT_STRING_TOO_LONG RIL_E_OEM_ERROR_2
#define RIL_E_SIM_MEM_FULL RIL_E_OEM_ERROR_3

int maxGrpNum[4] = {-1, -1, -1, -1};
int maxAnrNum[4] = {-1, -1, -1, -1};
int maxEmailNum[4] = {-1, -1, -1, -1};

static int isMatchGsm7bit(char *src, int maxLen) {
    int i = 0, j = 0;
    /* 3GPP TS 23.038 V9.1.1 section 6.2.1 - GSM 7 bit Default Alphabet */
    static unsigned char languageTable[128][4] = {
        {'0', '0', '4', '0'},   // @
        {'0', '0', 'a', '3'},   // \u00a3
        {'0', '0', '2', '4'},   // $
        {'0', '0', 'a', '5'},   // \u00a5
        {'0', '0', 'e', '8'},   // \u00e8
        {'0', '0', 'e', '9'},   // \u00e9
        {'0', '0', 'f', '9'},   // \u00f9
        {'0', '0', 'e', 'c'},   // \u00ec
        {'0', '0', 'f', '2'},   // \u00f2
        {'0', '0', 'c', '7'},   // \u00c7
        {'0', '0', '0', 'a'},   // \n
        {'0', '0', 'd', '8'},   // \u00d8
        {'0', '0', 'f', '8'},   // \u00f8
        {'0', '0', '0', 'd'},   // \r
        {'0', '0', 'c', '5'},   // \u00C5
        {'0', '0', 'e', '5'},   // \u00e5
        {'0', '3', '9', '4'},   // \u0394
        {'0', '0', '5', 'f'},   // _
        {'0', '3', 'a', '6'},   // \u03a6
        {'0', '3', '9', '3'},   // \u0393
        {'0', '3', '9', 'b'},   // \u039b
        {'0', '3', 'a', '9'},   // \u03a9
        {'0', '3', 'a', '0'},   // \u03a0
        {'0', '3', 'a', '8'},   // \u03a8
        {'0', '3', 'a', '3'},   // \u03a3
        {'0', '3', '9', '8'},   // \u0398
        {'0', '3', '9', 'e'},   // \u039e
        {'f', 'f', 'f', 'f'},   // \uffff    //escape
        {'0', '0', 'c', '6'},   // \u00c6
        {'0', '0', 'e', '6'},   // \u00e6
        {'0', '0', 'd', 'f'},   // \u00df
        {'0', '0', 'c', '9'},   // \u00c9
        {'0', '0', '2', '0'},   // space
        {'0', '0', '2', '1'},   // !
        {'0', '0', '2', '2'},   // "
        {'0', '0', '2', '3'},   // #
        {'0', '0', 'a', '4'},   // \u00a4
        {'0', '0', '2', '5'},   // %
        {'0', '0', '2', '6'},   // &
        {'0', '0', '2', '7'},   // '
        {'0', '0', '2', '8'},   // (
        {'0', '0', '2', '9'},   // )
        {'0', '0', '2', 'a'},   // *
        {'0', '0', '2', 'b'},   // +
        {'0', '0', '2', 'c'},   // ,
        {'0', '0', '2', 'd'},   // -
        {'0', '0', '2', 'e'},   // .
        {'0', '0', '2', 'f'},   //
        {'0', '0', '3', '0'},   // 0
        {'0', '0', '3', '1'},   // 1
        {'0', '0', '3', '2'},   // 2
        {'0', '0', '3', '3'},   // 3
        {'0', '0', '3', '4'},   // 4
        {'0', '0', '3', '5'},   // 5
        {'0', '0', '3', '6'},   // 6
        {'0', '0', '3', '7'},   // 7
        {'0', '0', '3', '8'},   // 8
        {'0', '0', '3', '9'},   // 9
        {'0', '0', '3', 'a'},   // :
        {'0', '0', '3', 'b'},   // ;
        {'0', '0', '3', 'c'},   // <
        {'0', '0', '3', 'd'},   // =
        {'0', '0', '3', 'e'},   // >
        {'0', '0', '3', 'f'},   // ?
        {'0', '0', 'a', '1'},   // \u00a1
        {'0', '0', '4', '1'},   // A
        {'0', '0', '4', '2'},   // B
        {'0', '0', '4', '3'},   // C
        {'0', '0', '4', '4'},   // D
        {'0', '0', '4', '5'},   // E
        {'0', '0', '4', '6'},   // F
        {'0', '0', '4', '7'},   // G
        {'0', '0', '4', '8'},   // H
        {'0', '0', '4', '9'},   // I
        {'0', '0', '4', 'a'},   // J
        {'0', '0', '4', 'b'},   // K
        {'0', '0', '4', 'c'},   // L
        {'0', '0', '4', 'd'},   // M
        {'0', '0', '4', 'e'},   // N
        {'0', '0', '4', 'f'},   // O
        {'0', '0', '5', '0'},   // P
        {'0', '0', '5', '1'},   // Q
        {'0', '0', '5', '2'},   // R
        {'0', '0', '5', '3'},   // S
        {'0', '0', '5', '4'},   // T
        {'0', '0', '5', '5'},   // U
        {'0', '0', '5', '6'},   // V
        {'0', '0', '5', '7'},   // W
        {'0', '0', '5', '8'},   // X
        {'0', '0', '5', '9'},   // Y
        {'0', '0', '5', 'a'},   // Z
        {'0', '0', 'c', '4'},   // \u00c4
        {'0', '0', 'd', '6'},   // \u00d6
        {'0', '0', 'd', '1'},   // \u00d1
        {'0', '0', 'd', 'c'},   // \u00dc
        {'0', '0', 'a', '7'},   // \u00a7
        {'0', '0', 'b', 'f'},   // \u00bf
        {'0', '0', '6', '1'},   // a
        {'0', '0', '6', '2'},   // b
        {'0', '0', '6', '3'},   // c
        {'0', '0', '6', '4'},   // d
        {'0', '0', '6', '5'},   // e
        {'0', '0', '6', '6'},   // f
        {'0', '0', '6', '7'},   // g
        {'0', '0', '6', '8'},   // h
        {'0', '0', '6', '9'},   // i
        {'0', '0', '6', 'a'},   // j
        {'0', '0', '6', 'b'},   // k
        {'0', '0', '6', 'c'},   // l
        {'0', '0', '6', 'd'},   // m
        {'0', '0', '6', 'e'},   // n
        {'0', '0', '6', 'f'},   // o
        {'0', '0', '7', '0'},   // p
        {'0', '0', '7', '1'},   // q
        {'0', '0', '7', '2'},   // r
        {'0', '0', '7', '3'},   // s
        {'0', '0', '7', '4'},   // t
        {'0', '0', '7', '5'},   // u
        {'0', '0', '7', '6'},   // v
        {'0', '0', '7', '7'},   // w
        {'0', '0', '7', '8'},   // x
        {'0', '0', '7', '9'},   // y
        {'0', '0', '7', 'a'},   // z
        {'0', '0', 'e', '4'},   // /u00ef
        {'0', '0', 'f', '6'},   // /u00f6
        {'0', '0', 'f', '1'},   // /u00f1
        {'0', '0', 'f', 'c'},   // /u00fc
        {'0', '0', 'e', '0'},   // /u00e0
    };

    LOGD("isMatchGsm7bit start: maxLen:%d", maxLen);

    if (src == NULL || maxLen < 0 || maxLen > 4) {
        return 0;
    }

    DLOGD("isMatchGsm7bit source = %c%c%c%c", src[j], src[j+1], src[j+2], src[j+3]);


    for (i = 0; i < 128; i++) {
        for (j = 0; j < maxLen && src[j] != '\0'; j++) {
            if (src[j] >= 'A' && src[j] <= 'Z') {
                src[j] = src[j] - 'A' + 'a';
            }

            if (src[j] != languageTable[i][j]) {
                break;
            }
        }

        // LOGD("isMatchGsm7bit i = %d, j = %d", i, j);

        if (j == 4) {
            DLOGD("isMatchGsm7bit return = %d", i);
            return i;
        }
    }

    return -1;
}

static int isGsm7bitExtension(char *src, int maxLen) {
    int i = 0, j = 0;
    /* 3GPP TS 23.038 V9.1.1 section 6.2.1 - GSM 7 bit Default Alphabet */
    static unsigned char languageTableExt[10][4] = {
        {'0', '0', '0', 'c'},   // Page break
        {'0', '0', '5', 'e'},   // ^
        {'0', '0', '7', 'b'},   // {
        {'0', '0', '7', 'd'},   // }
        {'0', '0', '5', 'c'},   // '\'
        {'0', '0', '5', 'b'},   // [
        {'0', '0', '7', 'e'},   // ~
        {'0', '0', '5', 'd'},   // ]
        {'0', '0', '7', 'c'},   // |
        {'2', '0', 'a', 'c'},   // EURO SIGN
    };

    static int gsm7bitValue[10] = {
        10,   // Page break
        20,   // ^
        40,   // {
        41,   // }
        47,   // '\'
        60,   // [
        61,   // ~
        62,   // ]
        64,   // |
        101,  // ERUO SIGN
    };

    DLOGD("isGsm7bitExtension source = %c%c%c%c", src[j], src[j+1], src[j+2], src[j+3]);

    for (i = 0; i < 10; i++) {
        for (j = 0; j < maxLen && src[j] != '\0'; j++) {
            if (src[j] >= 'A' && src[j] <= 'Z') {
                src[j] = src[j] - 'A' + 'a';
            }

            // LOGD("isGsm7bitExtension src[%d]=%c table[%d][%d]=%c", j, src[j], i, j, languageTableExt[i][j]);

            if (src[j] != languageTableExt[i][j]) {
                break;
            }
        }

        // LOGD("isGsm7bitExtension i = %d, j = %d", i, j);

        if (j == 4) {
            DLOGD("isGsm7bitExtension return = %d", gsm7bitValue[i]);
            return gsm7bitValue[i];
        }
    }

    return -1;
}

static int encodeUCS2_0x81(char *src, char *des, int maxLen) {
    int i, j, k, len = 0;
    unsigned int base = 0xFF000000;
    unsigned short tmpAlphaId[RIL_MAX_PHB_NAME_LEN+3+1];

    len = strlen(src);
    for (i=0, j=0; i < len; i+=4, j++) {
        tmpAlphaId[j] = (unsigned short) rild_sms_hexCharToDecInt(src+i, 4);
    }
    tmpAlphaId[j] = '\0';
    len = j;

    LOGD("0x81: len: %d", len);

    if (len <= 3)   // at least 3 characters
        return 0;
    if ( ((len+3)*2+1) > maxLen)  // the destinaiton buffer is not enough(include '\0')
        return 0;

    for (i = 0; i < len; i++) {
        int needSpecialEncoding = 0;

        if (tmpAlphaId[i] & 0x8000)  // Because base pointer of 0x81 is 0hhh hhhh h000 0000
            return 0;

        for (k = 0; k < SPEC_ENCODING_ARR_LEN; k++) {
            if (tmpAlphaId[i] == specialEncoding[k][0]) {
                DLOGD("Haman 0x81 found: i: %d, tmpAlphaId: %x", i, tmpAlphaId[i]);
                tmpAlphaId[i] = specialEncoding[k][1];
                needSpecialEncoding = 1;
                break;
            }
        }

        if (needSpecialEncoding != 1) {
            if (tmpAlphaId[i] < 0x80) {
#ifndef ESC_SUPPORT
                if (tmpAlphaId[i] == 0x0020 ||
                    tmpAlphaId[i] == 0x005E ||
                    tmpAlphaId[i] == 0x007B ||
                    tmpAlphaId[i] == 0x007D ||
                    tmpAlphaId[i] == 0x005B ||
                    tmpAlphaId[i] == 0x007E ||
                    tmpAlphaId[i] == 0x005D ||
                    tmpAlphaId[i] == 0x005C ||
                    tmpAlphaId[i] == 0x007C )
                    return 0;
                else
#endif
                {
                    if (tmpAlphaId[i] == 0x0060) {
                        if (base == 0xFF000000) {
                            base = 0;
                            tmpAlphaId[i] = 0x00E0;
                        } else {
                            return 0;
                        }
                    }

                    continue;
                }
            }

            if (base == 0xFF000000) {
                base = tmpAlphaId[i] & RIL_PHB_UCS2_81_MASK;
            }
            tmpAlphaId[i] ^= base;
            DLOGD("0x81: alpha: %x", tmpAlphaId[i]);
            if ( tmpAlphaId[i] >= 0x80)
                break;
            tmpAlphaId[i] |= 0x80;
        }
    }

    if (i != len)
        return 0;

    // Make up UCS2 0x81 String
    sprintf(des, "81%02X%02X", len, base>>7);
    int realLen = 0;
    char* pLen = des;
    des += 6;

    for (i=0; i < len; i++, des+=2) {
        if ((tmpAlphaId[i] & 0xFF00) != 0x1B00) {
            DLOGD("Haman1B 0x81: i: %d, tmpAlphaId: %x", i, tmpAlphaId[i]);
                sprintf(des, "%02X", tmpAlphaId[i]);
            DLOGD("Haman pleni: %s", pLen);
        }
        else {
            sprintf(des, "%04X", tmpAlphaId[i]);
            DLOGD("Haman 0x81: i: %d, tmpAlphaId: %x", i, tmpAlphaId[i]);
            DLOGD("Haman plenx: %s", pLen);
            des+=2;
            realLen++;
        }
    }

    realLen += len;
    if (realLen > len) {
        *(pLen + 2) = (char)((realLen/16) > 9 ? ((realLen/16) - 10 + 'A') : (realLen/16) + '0');
        *(pLen + 3) = (char)((realLen%16) > 9 ? ((realLen%16) - 10 + 'A') : (realLen%16) + '0');
    }
    *des = '\0';

    return realLen;
}

static int encodeUCS2_0x82(char *src, char *des, int maxLen) {
    int i, j, k, len;
    int min, max, base;
    int needSpecialEncoding = 0;
    unsigned short tmpAlphaId[RIL_MAX_PHB_NAME_LEN+4+1];

    int realLen = 0;
    char* pLen = des;

    min = max = base = -1;

    len = strlen(src);
    for (i=0, j=0; i < len; i+=4, j++) {
        tmpAlphaId[j] = (unsigned short) rild_sms_hexCharToDecInt(src+i, 4);
    }
    tmpAlphaId[j] = '\0';
    len = j;

    LOGD("0x82: len: %d", len);

    if (len <= 4)   // at least 4 characters
        return 0;
    if (((len+4)*2+1) > maxLen)  // the destinaiton buffer is not enough(include '\0')
        return 0;

    for (i=0; i < len; i++) {
        for (k = 0; k < SPEC_ENCODING_ARR_LEN; k++) {
            if (tmpAlphaId[i] == specialEncoding[k][0]) {
                needSpecialEncoding = 1;
                break;
            }
        }

        if (needSpecialEncoding == 0) {
            if ((tmpAlphaId[i] < 0x0080) && (tmpAlphaId[i] != 0x0060)) {
#ifndef ESC_SUPPORT
                 if (tmpAlphaId[i] == 0x0020 ||
                    tmpAlphaId[i] == 0x005E ||
                    tmpAlphaId[i] == 0x007B ||
                    tmpAlphaId[i] == 0x007D ||
                    tmpAlphaId[i] == 0x005B ||
                    tmpAlphaId[i] == 0x007E ||
                    tmpAlphaId[i] == 0x005D ||
                    tmpAlphaId[i] == 0x005C ||
                    tmpAlphaId[i] == 0x007C )
                    return 0;
#endif
            } else {    // (tmpAlphaId[i] >= 0x80) || (tmpAlphaId[i] == 0x0060)
                if (min < 0) {
                    min = max = tmpAlphaId[i];
                } else if (tmpAlphaId[i] < min) {
                    min = tmpAlphaId[i];
                } else if (tmpAlphaId[i] > max) {
                    max = tmpAlphaId[i];
                }
            }
        } else {
            needSpecialEncoding = 0;
        }
    }

    if (min < 0)
        min = max = 0;

    DLOGD("0x82: min: 0x%x, max: 0x%x", min, max);

    if ((max - min) >= 128)
        return 0;

    base = min;

    // encode the String
    for (i=0; i < len; i++) {
        for (k = 0; k < SPEC_ENCODING_ARR_LEN; k++) {
            if (tmpAlphaId[i] == specialEncoding[k][0]) {
                tmpAlphaId[i] = specialEncoding[k][1];
                needSpecialEncoding = 1;
                break;
            }
        }

        if (needSpecialEncoding == 0) {
            if ((tmpAlphaId[i] < 0x0080) && (tmpAlphaId[i] != 0x0060))
                continue;

            tmpAlphaId[i] -= base;
            tmpAlphaId[i] |= 0x80;
        } else {
            needSpecialEncoding = 0;
        }
    }

    // Make up UCS2 0x82 String
    sprintf(des, "82%02X%04X", len, base);
    des += 8;

    for (i=0; i < len; i++, des+=2) {
        if ((tmpAlphaId[i] & 0xFF00) != 0x1B00) {
            sprintf(des, "%02X", tmpAlphaId[i]);
        } else {
            sprintf(des, "%04X", tmpAlphaId[i]);
            des+=2;
            realLen++;
        }
    }

    realLen += len;

    if (realLen > len) {
        *(pLen + 2) = (char)((realLen/16) > 9 ? ((realLen/16) - 10 + 'A') : (realLen/16) + '0');
        *(pLen + 3) = (char)((realLen%16) > 9 ? ((realLen%16) - 10 + 'A') : (realLen%16) + '0');
    }

    *des = '\0';

    return realLen;
}

static char * ascii2UCS2(char * input) {
    if (input == 0) {
        return 0;
    }

    int len = strlen(input);
    char * ret = (char *)calloc(1, len*4+1);
    if (ret == NULL) {
        LOGE("ascii2UCS2 ret calloc fail");
        return NULL;
    }
    int i = 0;
    for (i=0; i < len; ++i) {
        sprintf(ret+i*4, "%04x", input[i]);
    }

    return ret;
}

void requestQueryPhbInfo(void *data, size_t datalen, RIL_Token t) {
    ATResponse *p_response = NULL;
    int err, type;

    int query_info[4];
    char *line, *tmp;

    RIL_SIM_UNUSED_PARM(datalen);

    type = ((int *)data)[0];
    if (selectPhbStorage(type, t)) {
        err = at_send_command_singleline("AT+CPBS?", "+CPBS:", &p_response, SIM_CHANNEL_CTX);
        if (err < 0 || p_response->success == 0) {
            goto error;
        }

        // +CPBS: <STORAGE>, <used>, <total>
        line = p_response->p_intermediates->line;
        err = at_tok_start(&line);
        if (err < 0) goto error;

        err = at_tok_nextstr(&line, &tmp);
        if (err < 0) goto error;

        err = at_tok_nextint(&line, &query_info[0]);
        if (err < 0) goto error;

        err = at_tok_nextint(&line, &query_info[1]);
        if (err < 0) goto error;

        at_response_free(p_response);


        err = at_send_command_singleline("AT+CPBR=?", "+CPBR:", &p_response, SIM_CHANNEL_CTX);
        if (err < 0 || p_response->success == 0) {
            goto error;
        }
        // +CPBR: (<bIndex>-<eIndex>), <max_num_len>, <max_alpha_len>
        line = p_response->p_intermediates->line;
        err = at_tok_start(&line);
        if (err < 0) goto error;

        err = at_tok_nextstr(&line, &tmp);
        if (err < 0) goto error;

        err = at_tok_nextint(&line, &query_info[2]);
        if (err < 0) goto error;

        err = at_tok_nextint(&line, &query_info[3]);
        if (err < 0) goto error;

        at_response_free(p_response);

        LOGD("PhbQueryInformation: %d %d %d %d", query_info[0],
                query_info[1], query_info[2], query_info[3]);

        RIL_onRequestComplete(t, RIL_E_SUCCESS, query_info, 4*sizeof(int));
        return;
    }
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

void requestClearPhbEntry(int index, RIL_Token t) {
    int err;
    char cmd[64] = {0};
    char *tmp = NULL;
    int i = 0;
    int len = 0;
    int maxNum = -1;
    ATResponse *p_response = NULL;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    if (RIL_SOCKET_1 == rid) {
        maxNum = maxGrpNum[0];
    } else if (RIL_SOCKET_2 == rid) {
        maxNum = maxGrpNum[1];
    } else if (RIL_SOCKET_3 == rid) {
        maxNum = maxGrpNum[2];
    } else if (RIL_SOCKET_4 == rid) {
        maxNum = maxGrpNum[3];
    } else {
        maxNum = -1;
    }

    LOGD("requestClearPhbEntry - maxNum = %d", maxNum);
    if (maxNum > 0) {
        snprintf(cmd, sizeof(cmd), "AT+EPBUM=2,5,%d,0", index);
        len = strlen((char*)cmd);
        LOGD("requestClearPhbEntry - len = %d", len);
        for (i = 0; i < maxNum * 2; i += 2) {
            cmd[len+i] = ',';
            cmd[len+i+1] = '0';
        }
        LOGD("cmd = %s", (char*)cmd);
        err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
        if (err < 0 || NULL == p_response) {
            LOGE("requestClearPhbEntry Fail");
        }
        at_response_free(p_response);
    }
}

void requestWritePhbEntry(void *data, size_t datalen, RIL_Token t) {
    ATResponse *p_response = NULL;
    int err, tmp;
    unsigned int i, j, len;
    int asciiNotSupported = 0;
    RIL_PhbEntryStructure *entry;
    char *line = NULL;
    char *cmd = NULL;
    char *number = NULL;
    char alphaId[RIL_MAX_PHB_NAME_LEN*4+4+1]={0};
    // If using UCS2, each character use 4 bytes,
    // if using 0x82 coding, we need extra 4 bytes,
    // we also need '\0'

    char temp[RIL_MAX_PHB_NAME_LEN*4+4+1] = {0};

    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    RilPhbCpbwEncode encodeMethod;

    RIL_SIM_UNUSED_PARM(datalen);

    entry = (RIL_PhbEntryStructure *)data;
    if (selectPhbStorage(entry->type, t)) {
        if (entry->alphaId != NULL || entry->number != NULL) {
            if (entry->alphaId == NULL) {
                line = "";
            } else {
                line = entry->alphaId;
            }
            if (entry->number == NULL) {
                number = "";
            } else {
                number = entry->number;
            }
            len = strlen(line);
            if (len == 0 && strlen(number) == 0) {
                LOGE("The number and alphaid can't be empty String at the same time: %s %s",
                        line, number);
                goto error;
            }

            // pack Alpha Id
            if ((len%4) != 0) {
                LOGE("The alphaId should encode using Hexdecimal: %s", line);
                goto error;
            } else if (len > (RIL_MAX_PHB_NAME_LEN*4)) {
                LOGE("The alphaId shouldn't longer than RIL_MAX_PHB_NAME_LEN");
                ret = RIL_E_TEXT_STRING_TOO_LONG;
                goto error;
            }

            for (i=0, j=0; i < len; i+=4, j++) {
                if (bIsTc1() == true) {
                    tmp = isMatchGsm7bit(line+i, 4);

                    if (tmp == -1) {
                        // Check if the char match GSM-7bit extension table
                        tmp = isGsm7bitExtension(line+i, 4);

                        if (tmp != -1) {
                            alphaId[j] = (char)(0x1b);
                            j++;
                         } else {
                            LOGD("requestWritePhbEntry isMatchGsm7bit = false");
                            break;
                         }
                    }
                } else {
                    tmp = rild_sms_hexCharToDecInt(line+i, 4);

                    if (tmp >= 128) {
                        break;
                    }

                    if (tmp == '`') {
                        // Special character, ascii not support it
                        asciiNotSupported = 1;
                        LOGD("Ascii not supported, there is a '`'.");
                    }
                }

                alphaId[j] = (char)(tmp);
                DLOGD("requestWritePhbEntry tmp=%d, alphaId=%d", tmp, alphaId[j]);
            }
            alphaId[j] = '\0';

            // LOGD("requestWritePhbEntry alphaId = %s", alphaId); // pii

            if (i != len || asciiNotSupported) {
                len /= 4;

                if (encodeUCS2_0x81(line, alphaId, sizeof(alphaId))) {  // try UCS2_0x81 coding
                    // UCS2 0x81 coding
                    encodeMethod = CPBW_ENCODE_UCS2_81;
                } else if (encodeUCS2_0x82(line, alphaId, sizeof(alphaId))) {
                    // try UCS2_0x82 coding
                    // UCS2 0x82 coding
                    encodeMethod = CPBW_ENCODE_UCS2_82;
                } else {
                    // UCS2 coding
                    encodeMethod = CPBW_ENCODE_UCS2;
                    LOGD("alphaId length=%zu, line length=%zu", sizeof(alphaId), strlen(line));
                    memcpy(alphaId, line, ((strlen(line) < sizeof(alphaId)) ? strlen(line) : sizeof(alphaId)));
                }
            } else {
                if (bIsTc1() == true) {
                    encodeMethod = CPBW_ENCODE_GSM7BIT;

                    LOGD("alphaId length=%zu, line length=%zu", sizeof(alphaId), strlen(line));
                    memcpy(alphaId, line, ((strlen(line) < sizeof(alphaId)) ? strlen(line) : sizeof(alphaId)));
                } else {
                    encodeMethod = CPBW_ENCODE_IRA;
                    // ASCII coding

                    // Replace \ to \5c and replace " to \22 for MTK modem
                    // the order is very important! for "\\" is substring of "\\22"
                    memcpy(temp, alphaId, sizeof(alphaId));
                    j = 0;
                    for (i=0; i < strlen(temp); i++) {
                        if (temp[i] == '\\') {
                            LOGD("Use Ascii encoding, there is a '\\', index=%d", i);
                            if ((j+2) < (sizeof(alphaId)-1)) {
                                alphaId[j++] = '\\';
                                alphaId[j++] = '5';
                                alphaId[j++] = 'c';
                            } else {
                                break;
                            }
                        } else {
                            alphaId[j++] = temp[i];
                        }

                        if (j == (sizeof(alphaId)-1)) {
                            break;
                        }
                    }
                    alphaId[j] = '\0';

                    memcpy(temp, alphaId, sizeof(alphaId));
                    j = 0;
                    for (i=0; i < strlen(temp); i++) {
                        if (temp[i] == '\"') {
                            LOGD("Use Ascii encoding, there is a '\"', index=%d", i);
                            if ((j+2) < (sizeof(alphaId)-1)) {
                                alphaId[j++] = '\\';
                                alphaId[j++] = '2';
                                alphaId[j++] = '2';
                            } else {
                                break;
                            }
                        } else {
                            alphaId[j++] = temp[i];
                        }

                        if (j == (sizeof(alphaId)-1)) {
                            break;
                        }
                    }
                    alphaId[j] = '\0';
                }
            }
            // move this action to FW
            // requestClearPhbEntry(entry->index, t);
            asprintf(&cmd, "AT+EPBW=%d, \"%s\", %d, \"%s\", 0, %d",
                        entry->index, number, entry->ton, alphaId, encodeMethod);
        } else {
            // delete
            asprintf(&cmd, "AT+EPBW=%d", entry->index);
        }

        if (cmd == NULL) {
            LOGE("EPBW cmd null Error!!!!");
            goto error;
        }
        err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
        free(cmd);
        if (err < 0 || p_response == NULL) {
            LOGE("EPBW Error!!!!");
            goto error;
        }

        if (p_response->success == 0) {
            switch (at_get_cme_error(p_response)) {
                case CME_SUCCESS:
                    /* While p_response->success is 0, the CME_SUCCESS means CME ERROR:0 => it is phone failure */
                    ret = RIL_E_GENERIC_FAILURE;
                    break;
                case CME_SIM_PIN2_REQUIRED:
                    ret = RIL_E_SIM_PIN2;
                    break;
                case CME_SIM_PUK2_REQUIRED:
                    ret = RIL_E_SIM_PUK2;
                    break;
                case CME_DIAL_STRING_TOO_LONG:
                    ret = RIL_E_DIAL_STRING_TOO_LONG;
                    break;
                case CME_TEXT_STRING_TOO_LONG:
                    ret = RIL_E_TEXT_STRING_TOO_LONG;
                    break;
                case CME_MEMORY_FULL:
                    ret = RIL_E_SIM_MEM_FULL;
                    break;
                default:
                    break;
            }
        } else {
            ret = RIL_E_SUCCESS;
        }
    }
error:
    RIL_onRequestComplete(t, ret, NULL, 0);
    at_response_free(p_response);
}

void requestReadPhbEntry(void *data, size_t datalen, RIL_Token t) {
    ATResponse *p_response = NULL;
    int err, type, bIndex, eIndex, count, i;
    char *line = NULL;
    char *cmd = NULL;
    ATLine *p_cur = NULL;
    RIL_PhbEntryStructure *entry = NULL;
    RIL_PhbEntryStructure **entries = NULL;

    RIL_SIM_UNUSED_PARM(datalen);

    type = ((int *)data)[0];
    if (selectPhbStorage(type, t)) {
        bIndex = ((int *)data)[1];
        eIndex = ((int *)data)[2];

        if ((eIndex - bIndex + 1) <= 0 || (eIndex - bIndex + 1) > RIL_MAX_PHB_ENTRY) {
            LOGE("Begin index or End Index is invalid: %d %d", bIndex, eIndex);
            goto error;
        }

        asprintf(&cmd, "AT+CPBR=%d, %d", bIndex, eIndex);
        err = at_send_command_multiline(cmd, "+CPBR:", &p_response, SIM_CHANNEL_CTX);
        free(cmd);
        if (err < 0 || p_response->success == 0) {
            goto error;
        }

        /* count the calls */
        for (count = 0, p_cur = p_response->p_intermediates
                ; p_cur != NULL
                ; p_cur = p_cur->p_next
        ) {
            count++;
        }

        /* allocate memory and parse +CPBR*/
        entries = calloc(1, sizeof(RIL_PhbEntryStructure*)*count);
        if (entries == NULL) {
            LOGE("entries calloc fail");
            goto error;
        }

        // +CPBR: <index>, <number>, <TON>, <alphaId>
        for (i = 0, p_cur = p_response->p_intermediates
                ; p_cur != NULL
                ; p_cur = p_cur->p_next, i++
        ) {
            line = p_cur->line;
            entries[i] = entry = calloc(1, sizeof(RIL_PhbEntryStructure));
            if (entry == NULL) {
                LOGE("entry calloc fail");
                goto error;
            }

            entry->type = type;

            err = at_tok_start(&line);
            if (err < 0) goto error;

            err = at_tok_nextint(&line, &entry->index);
            if (err < 0) goto error;

            err = at_tok_nextstr(&line, &entry->number);
            if (err < 0) goto error;

            err = at_tok_nextint(&line, &entry->ton);
            if (err < 0) goto error;

            err = at_tok_nextstr(&line, &entry->alphaId);
            if (err < 0) goto error;

            // LOGD("PhbEntry %d: %s, %d, %s", entry->index,
            //    entry->number, entry->ton, entry->alphaId); // pii
        }

        RIL_onRequestComplete(t, RIL_E_SUCCESS, entries, count*sizeof(RIL_PhbEntryStructure*));

        for (i=0; i < count; i++) {
            if (entries[i] != NULL) {
                free(entries[i]);
            }
        }
        free(entries);
        at_response_free(p_response);
        return;
    }
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);

    if (entries != NULL) {
        for (i=0; i < count; i++) {
            if (entries[i] != NULL) {
                free(entries[i]);
            }
        }
        free(entries);
    }
}

void requestQueryUPBCapability(void *data, size_t datalen, RIL_Token t) {
    ATResponse *p_response = NULL;
    int err;
    char *line;
    int upbCap[8]= {0};
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));

    RIL_SIM_UNUSED_PARM(data);
    RIL_SIM_UNUSED_PARM(datalen);

    err = at_send_command_singleline("AT+EPBUM=?", "+EPBUM:", &p_response, SIM_CHANNEL_CTX);

    if (err < 0 || NULL == p_response) {
        LOGE("requestQueryUPBCapability Fail");
        goto error;
    }

    if (p_response->success == 0) {
        switch (at_get_cme_error(p_response)) {
            LOGD("p_response = %d /n", at_get_cme_error(p_response) );
            case CME_SUCCESS:
                ret = RIL_E_GENERIC_FAILURE;
            break;
            case CME_UNKNOWN:
                LOGD("p_response: CME_UNKNOWN");
            break;
            default:
            break;
        }
    } else {
        ret = RIL_E_SUCCESS;
    }

    if (ret != RIL_E_SUCCESS) {
        LOGE("requestQueryUPBCapability: Error!");
        goto error;
    }

    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    LOGD("requestQueryUPBCapability: strlen of response is %zu", strlen(line) );
    err = at_tok_nextint(&line, &upbCap[0]);  // number of supported ANR for a ADN entry
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &upbCap[1]);  // number of supported EMAIL for a ADN entry
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &upbCap[2]);  // number of supported SNE for a ADN entry
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &upbCap[3]);  // maximum number of AAS entries
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &upbCap[4]);  // maximum length of the AAS in the entries
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &upbCap[5]);  // maximum number of GAS entries
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &upbCap[6]);  // maximum length of the GAS in the entries
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &upbCap[7]);  // maximum number of GRP entries
    if (err < 0) goto error;
    if (RIL_SOCKET_1 == rid) {
        maxGrpNum[0] = upbCap[7];
        maxAnrNum[0] = upbCap[0];
        maxEmailNum[0] = upbCap[1];
    }
    else if (RIL_SOCKET_2 == rid) {
        maxGrpNum[1] = upbCap[7];
        maxAnrNum[1] = upbCap[0];
        maxEmailNum[1] = upbCap[1];
    }
    else if (RIL_SOCKET_3 == rid) {
        maxGrpNum[2] = upbCap[7];
        maxAnrNum[2] = upbCap[0];
        maxEmailNum[2] = upbCap[1];
    }
    else if (RIL_SOCKET_4 == rid) {
        maxGrpNum[3] = upbCap[7];
        maxAnrNum[3] = upbCap[0];
        maxEmailNum[3] = upbCap[1];
    }

    LOGD("requestQueryUPBCapability: %d, %d, %d, %d, %d, %d, %d, %d",
        upbCap[0], upbCap[1], upbCap[2], upbCap[3], upbCap[4], upbCap[5],
        upbCap[6], upbCap[7]);
    RIL_onRequestComplete(t, RIL_E_SUCCESS,
            upbCap, 8*sizeof(int));

    at_response_free(p_response);
    return;
error:
    RIL_onRequestComplete(t, ret, NULL, 0);
    at_response_free(p_response);
}

void requestEditUPBEntry(void *data, size_t datalen, RIL_Token t) {
    ATResponse *p_response = NULL;
    int err, len, tmp, i, j;
    int asciiNotSupported = 0;
    const char** params   = (const char**)data;
    char *line, *cmd;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    RilPhbCpbwEncode encodeMethod;
    int paramCount = datalen/sizeof(int);

    // LOGD("requestEditUPBEntry: %s,%s,%s,%s paramCount=%d",
    //     params[0], params[1], params[2], params[3], paramCount); // pii

    // GAS, AAS, SNE
    if (0 == strcmp(params[0], "4") || 0 == strcmp(params[0], "3") || 0 == strcmp(params[0], "2")) {
        char temp[RIL_MAX_PHB_NAME_LEN*4+4+1] = {0};
        char str[RIL_MAX_PHB_NAME_LEN*4+4+1] = {0};  // If using UCS2, each character use 4 bytes,
                                                     // if using 0x82 coding, we need extra 4 bytes, we also need '\0'
        line = (char*)params[3];
        len = strlen(line);
        DLOGD("requestEditUPBEntry: line is %s, len=%d", line, len);

        if (len == 0) {
            LOGE("The group name can't be empty String: %s", line);
            goto error;
        }

        // pack gas
        if ((len%4) != 0) {
            LOGE("The gas should encode using Hexdecimal: %s", line);
            goto error;
        } else if (len > (RIL_MAX_PHB_NAME_LEN*4)) {
            LOGE("The gas shouldn't longer than RIL_MAX_PHB_NAME_LEN");
            ret = RIL_E_TEXT_STRING_TOO_LONG;
            goto error;
        }

        for (i=0, j=0; i < len; i+=4, j++) {
            if (bIsTc1() == true) {
                tmp = isMatchGsm7bit(line+i, 4);

                if (tmp == -1) {
                    // Check if the char match GSM-7bit extension table
                    tmp = isGsm7bitExtension(line+i, 4);

                    if (tmp != -1) {
                        str[j] = (char)(0x1b);
                        j++;
                     } else {
                        LOGD("requestWritePhbEntry isMatchGsm7bit = false");
                        break;
                     }
                }
            } else {
                tmp = rild_sms_hexCharToDecInt(line+i, 4);

                if (tmp >= 128) {
                    break;
                }

                if (tmp == '`') {
                    // Special character, ascii not support it
                    asciiNotSupported = 1;
                    LOGD("Ascii not supported, there is a '`'.");
                }
            }

            str[j] = (char)tmp;
        }
        str[j] = '\0';

        DLOGD("requestEditUPBEntry str = %s", str);

        if (i != len || asciiNotSupported) {
            len /= 4;

            if (encodeUCS2_0x81(line, str, sizeof(str))) {    // try UCS2_0x81 coding
                // UCS2 0x81 coding
                encodeMethod = CPBW_ENCODE_UCS2_81;
            }
            else {
                // UCS2 coding
                encodeMethod = CPBW_ENCODE_UCS2;
                memcpy(str, line, ((sizeof(str) < strlen(line)) ? sizeof(str) : strlen(line)));
            }
        }
        else {
            if (bIsTc1() == true) {
                encodeMethod = CPBW_ENCODE_GSM7BIT;
                memcpy(str, line, ((sizeof(str) < strlen(line)) ? sizeof(str) : strlen(line)));
            } else {
                // ASCII coding
                encodeMethod = CPBW_ENCODE_IRA;

                // Replace \ to \5c and replace " to \22 for MTK modem
                // the order is very important! for "\\" is substring of "\\22"
                memcpy(temp, str, sizeof(str));
                j = 0;
                for (i=0; i < (int)strlen(temp); i++) {
                    if (temp[i] == '\\') {
                        LOGD("Use Ascii encoding, there is a '\\', index=%d", i);
                        if ((j+2) < (int)(sizeof(str)-1)) {
                            str[j++] = '\\';
                            str[j++] = '5';
                            str[j++] = 'c';
                        } else {
                            break;
                        }
                    } else {
                        str[j++] = temp[i];
                    }

                    if (j == (sizeof(str)-1)) {
                        break;
                    }
                }
                str[j] = '\0';

                memcpy(temp, str, sizeof(str));
                j = 0;
                for (i=0; i < (int)strlen(temp); i++) {
                    if (temp[i] == '\"') {
                        LOGD("Use Ascii encoding, there is a '\"', index=%d", i);
                        if ((j+2) < (int)(sizeof(str)-1)) {
                            str[j++] = '\\';
                            str[j++] = '2';
                            str[j++] = '2';
                        } else {
                            break;
                        }
                    } else {
                        str[j++] = temp[i];
                    }

                    if (j == (sizeof(str)-1)) {
                        break;
                    }
                }
                str[j] = '\0';
            }
        }

        asprintf(&cmd, "AT+EPBUM=2,%s,%s,%s,\"%s\",%d", params[0], params[2], params[1], str, encodeMethod);
    }
    else if ( 0 == strcmp(params[0], "1") ) {  // Email
        char temp[RIL_MAX_PHB_EMAIL_LEN*4+4+1] = {0};
        char str[RIL_MAX_PHB_EMAIL_LEN*4+4+1] = {0};  // If using UCS2, each character use 4 bytes,
                                                      // if using 0x82 coding, we need extra 4 bytes, we also need '\0'
        line = (char*)params[3];
        len = strlen(line);
        // LOGD("requestEditUPBEntry: line is %s, len=%d", line, len); // pii

        if (len == 0) {
            LOGE("The email name can't be empty String: %s", line);
            goto error;
        }

        // pack email
        if ((len%4) != 0) {
            LOGE("The email should encode using Hexdecimal: %s", line);
            goto error;
        } else if (len > (RIL_MAX_PHB_EMAIL_LEN*4)) {
            LOGE("The email shouldn't longer than RIL_MAX_PHB_EMAIL_LEN");
            ret = RIL_E_TEXT_STRING_TOO_LONG;
            goto error;
        }

        for (i=0, j=0; i < len; i+=4, j++) {
            if (bIsTc1() == true) {
                tmp = isMatchGsm7bit(line+i, 4);

                if (tmp == -1) {
                    // Check if the char match GSM-7bit extension table
                    tmp = isGsm7bitExtension(line+i, 4);

                    if (tmp != -1) {
                        str[j] = (char)(0x1b);
                        j++;
                     } else {
                        LOGD("requestWritePhbEntry isMatchGsm7bit = false");
                        break;
                     }
                }
            } else {
                tmp = rild_sms_hexCharToDecInt(line+i, 4);

                if (tmp >= 128) {
                    break;
                }

                if (tmp == '`') {
                    // Special character, ascii not support it
                    asciiNotSupported = 1;
                    LOGD("Ascii not supported, there is a '`'.");
                }
            }

            str[j] = (char)tmp;
        }
        str[j] = '\0';

        // LOGD("requestEditUPBEntry str = %s", str); // pii

        if (i != len || asciiNotSupported) {  // we think email doesn't get into here.
            len /= 4;

            if (encodeUCS2_0x81(line, str, sizeof(str))) {    // try UCS2_0x81 coding
                // UCS2 0x81 coding
                encodeMethod = CPBW_ENCODE_UCS2_81;
            }
            else {
                // UCS2 coding
                encodeMethod = CPBW_ENCODE_UCS2;
                memcpy(str, line, ((sizeof(str) < strlen(line)) ? sizeof(str) : strlen(line)));
            }
        }
        else {
            if (bIsTc1() == true) {
                encodeMethod = CPBW_ENCODE_GSM7BIT;
                memcpy(str, line, ((sizeof(str) < strlen(line)) ? sizeof(str) : strlen(line)));
            } else {
                // ASCII coding
                encodeMethod = CPBW_ENCODE_IRA;

                // Replace \ to \5c and replace " to \22 for MTK modem
                // the order is very important! for "\\" is substring of "\\22"
                memcpy(temp, str, sizeof(str));
                j = 0;
                for (i=0; i < (int)strlen(temp); i++) {
                    if (temp[i] == '\\') {
                        LOGD("Use Ascii encoding, there is a '\\', index=%d", i);
                        if ((j+2) < (int)(sizeof(str)-1)) {
                            str[j++] = '\\';
                            str[j++] = '5';
                            str[j++] = 'c';
                        } else {
                            break;
                        }
                    } else {
                        str[j++] = temp[i];
                    }

                    if (j == (sizeof(str)-1)) {
                        break;
                    }
                }
                str[j] = '\0';

                memcpy(temp, str, sizeof(str));
                j = 0;
                for (i=0; i < (int)strlen(temp); i++) {
                    if (temp[i] == '\"') {
                        LOGD("Use Ascii encoding, there is a '\"', index=%d", i);
                        if ((j+2) < (int)(sizeof(str)-1)) {
                            str[j++] = '\\';
                            str[j++] = '2';
                            str[j++] = '2';
                        } else {
                            break;
                        }
                    } else {
                        str[j++] = temp[i];
                    }

                    if (j == (sizeof(str)-1)) {
                        break;
                    }
                }
                str[j] = '\0';
            }
        }

        asprintf(&cmd, "AT+EPBUM=2,%s,%s,%s,\"%s\"", params[0], params[2], params[1], str);
    }
    else if ( paramCount > 4 && 0 == strcmp(params[0], "0") ) {  // Anr
        asprintf(&cmd, "AT+EPBUM=2,%s,%s,%s,\"%s\",%s,%s", params[0], params[2],
                params[1], params[3], params[4], params[5]);
    } else {  // Error
        LOGE("requestEditUPBEntry: %s,%s,%s,%s paramCount=%d",
               params[0], params[1], params[2], params[3], paramCount);
        goto error;
    }
    err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
    free(cmd);
    if (err < 0 || p_response == NULL) {
        LOGE("EPBUM write Error!!!!");
        goto error;
    }

    if (p_response->success == 0) {
        switch (at_get_cme_error(p_response)) {
            case CME_SUCCESS:
                ret = RIL_E_GENERIC_FAILURE;
                break;
            case CME_TEXT_STRING_TOO_LONG:
                ret = RIL_E_TEXT_STRING_TOO_LONG;
                break;
            case CME_MEMORY_FULL:
                ret = RIL_E_SIM_MEM_FULL;
                break;
            default:
                break;
        }
    } else {
        ret = RIL_E_SUCCESS;
    }

error:
    RIL_onRequestComplete(t, ret, NULL, 0);
    at_response_free(p_response);
}

void requestDeleteUPBEntry(void *data, size_t datalen, RIL_Token t) {
    ATResponse *p_response = NULL;
    int err;
    char *cmd = NULL;
    char *line;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    int* params = (int *) data;

    if (datalen/sizeof(int) < 3) {
        LOGE("requestDeleteUPBEntry param is not enough. datalen is %zu", datalen);
        goto error;
    }

    asprintf(&cmd, "AT+EPBUM=3,%d,%d,%d", params[0], params[2], params[1]);
    err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
    free(cmd);

    if (err < 0 || p_response == NULL) {
        LOGE("requestDeleteUPBEntry Fail");
        goto error;
    }

    if (p_response->success == 0) {
        switch (at_get_cme_error(p_response)) {
            LOGD("p_response = %d /n", at_get_cme_error(p_response) );
            case CME_SUCCESS:
                ret = RIL_E_GENERIC_FAILURE;
            break;
            case CME_UNKNOWN:
                LOGD("p_response: CME_UNKNOWN");
            break;
            default:
            break;
        }
    } else {
        ret = RIL_E_SUCCESS;
    }
    if (ret != RIL_E_SUCCESS) {
        LOGD("requestDeleteUPBEntry: Error!");
        goto error;
    }

error:
    RIL_onRequestComplete(t, ret, NULL, 0);
    at_response_free(p_response);
}

void requestReadGasList(void *data, size_t datalen, RIL_Token t) {
    ATResponse **p_response = NULL;
    int err;
    char *cmd = NULL;
    char *line;
    int tmp, i, entryIndex;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    int* params = (int *) data;
    char **responses = NULL;

    if (datalen/sizeof(int) < 2 || params[0] < 0 || params[1] < params[0]) {
        LOGE("requestReadGasList param is not enough. datalen is %zu", datalen);
        goto error;
    }
    p_response = calloc(1, sizeof(ATResponse*)*(params[1] - params[0] + 1));
    if (p_response == NULL) {
        LOGE("p_response calloc fail");
        ret = RIL_E_NO_MEMORY;
        goto error;
    }
    responses = calloc(1, sizeof(char*)*(params[1] - params[0] + 1));
    if (responses == NULL) {
        LOGE("responses calloc fail");
        ret = RIL_E_NO_MEMORY;
        free(p_response);
        goto error;
    }

    for (i = params[0]; i <= params[1]; i++) {
        asprintf(&cmd, "AT+EPBUM=1,4,%d,0", i);

        err = at_send_command_singleline(cmd, "+EPBUM:", &p_response[i-1], SIM_CHANNEL_CTX);
        free(cmd);
        if (err < 0 || p_response[i-1] == NULL) {
            LOGD("requestReadGasList: the %d th gas entry is null", i);
            ret = RIL_E_SUCCESS;
            continue;  // it means the index is not used.
        }

        if (p_response[i-1]->success == 0) {
            switch (at_get_cme_error(p_response[i-1])) {
                LOGD("p_response = %d /n", at_get_cme_error(p_response[i-1]) );
                case CME_SUCCESS:
                    ret = RIL_E_GENERIC_FAILURE;
                    break;
                case CME_UNKNOWN:
                    ret = RIL_E_GENERIC_FAILURE;
                    LOGD("p_response: CME_UNKNOWN");
                    break;
                default:
                    ret = RIL_E_GENERIC_FAILURE;
                    LOGD("p_response: Not success");
                    break;
            }
        } else {
            ret = RIL_E_SUCCESS;
        }

        if (ret != RIL_E_SUCCESS) {
            LOGD("requestReadGasList: Error!");
            // goto error;
            continue;
        }

        if (p_response[i-1] == NULL || p_response[i-1]->p_intermediates == NULL) continue;
        line = p_response[i-1]->p_intermediates->line;
        err = at_tok_start(&line);
        LOGD("requestReadGasList: strlen of response is %zu", strlen(line) );

        err = at_tok_nextint(&line, &tmp);  // type
        if (err < 0 || tmp != 4) continue;

        err = at_tok_nextint(&line, &entryIndex);  // entry_index
        if (err < 0 || entryIndex < 1) continue;;
        LOGD("requestReadGasList: (entryIndex-1)=%d", entryIndex-1);

        err = at_tok_nextint(&line, &tmp);  // adn_index
        err = at_tok_nextstr(&line, &responses[entryIndex-1]);  // gas string
        if (err < 0) continue;

        err = at_tok_nextint(&line, &tmp);
        if (err < 0) continue;
        LOGD("requestReadGasList: encode type is %d", tmp);

        if (tmp == CPBW_ENCODE_IRA) {
            responses[entryIndex-1] = ascii2UCS2(responses[entryIndex-1]);
            if (responses[entryIndex-1] == NULL) {
                for (i = params[0]; i <= params[1]; i++) {
                    at_response_free(p_response[i-1]);
                }
                free(p_response);
                free(responses);
                ret = RIL_E_NO_MEMORY;
                goto error;
            }
        }
    }

    RIL_onRequestComplete(t, ret, responses, (params[1] - params[0] + 1)*sizeof(char*));
    for (i = params[0]; i <= params[1]; i++) {
        at_response_free(p_response[i-1]);
    }
    free(p_response);
    free(responses);
    return;
error:
    RIL_onRequestComplete(t, ret, NULL, 0);
}

void requestReadUpbGrpEntry(void *data, size_t datalen, RIL_Token t) {
    ATResponse *p_response = NULL;
    int err;
    char *cmd = NULL;
    char *line;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    int* params = (int *) data;
    int grpIds[10] = {0};
    int grpCount = 0;
    int tmp = 0;
    int paramCount = datalen/sizeof(int);

    if (isSimInserted(rid) == 0) {
        // LOGD("requestReadUpbGrpEntry no sim inserted");
        goto error;
    }

    if (paramCount < 1) {
        LOGE("requestReadUpbGrpEntry param is not enough. paramCount is %d", paramCount);
        goto error;
    }

    asprintf(&cmd, "AT+EPBUM=1,5,%d,1", params[0]);
    err = at_send_command_singleline(cmd, "+EPBUM:", &p_response, SIM_CHANNEL_CTX);
    free(cmd);
    if (err < 0 || NULL == p_response) {
        LOGE("requestReadUpbGrpEntry Fail");
        goto error;
    }

    if (p_response->success == 0) {
        switch (at_get_cme_error(p_response)) {
            LOGD("p_response = %d /n", at_get_cme_error(p_response) );
            case CME_SUCCESS:
                ret = RIL_E_GENERIC_FAILURE;
            break;
            case CME_UNKNOWN:
                LOGD("p_response: CME_UNKNOWN");
            break;
            default:
            break;
        }
    } else {
        ret = RIL_E_SUCCESS;
    }

    if (ret != RIL_E_SUCCESS) {
        LOGE("requestReadUpbGrpEntry: Error!");
        goto error;
    }

    if (p_response->p_intermediates != NULL) {
        line = p_response->p_intermediates->line;
        err = at_tok_start(&line);
        LOGD("requestReadUpbGrpEntry: strlen of response is %zu", strlen(line) );
        err = at_tok_nextint(&line, &tmp);  // field type , must be 5
        if (err < 0) goto error;

        err = at_tok_nextint(&line, &tmp);  // index1
        if (err < 0) goto error;

        err = at_tok_nextint(&line, &tmp);  // maximum number of supported entries in the file
        if (err < 0) {
            // LOGD("requestReadUpbGrpEntry: index2 is ignored!");
        }
        err = at_tok_nextint(&line, &grpIds[grpCount++]);  // group id
        if (err < 0) goto error;
        // LOGD("requestReadUpbGrpEntry: grpCount is %d, grpIds[%d] is %d!", grpCount, grpCount-1, grpIds[grpCount-1]);
        while (at_tok_hasmore(&line)) {
            err = at_tok_nextint(&line, &grpIds[grpCount++]);  // group id
            if (err < 0) goto error;
            // LOGD("requestReadUpbGrpEntry: grpCount is %d, grpIds[%d] is %d!", grpCount, grpCount-1,
            //        grpIds[grpCount-1]);
            if (grpCount >= 10) {
                break;
            }
        }
    }
    RIL_onRequestComplete(t, RIL_E_SUCCESS,
            grpIds, grpCount*sizeof(int));

    at_response_free(p_response);
    return;
error:
    RIL_onRequestComplete(t, ret, NULL, 0);
    at_response_free(p_response);
}

void requestWriteUpbGrpEntry(void *data, size_t datalen, RIL_Token t) {
    int err;
    char cmd[64] = {0};
    char *tmp = NULL;
    char *line;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    int* params = (int *) data;
    ATResponse *p_response = NULL;
    int i = 0;
    int paramCount = datalen/sizeof(int);
    LOGD("requestWriteUpbGrpEntry . datalen is %zu, size of int is %zu", datalen, sizeof(int));
    if (paramCount < 1) {
        LOGE("requestWriteUpbGrpEntry param is not enough. paramCount is %d", paramCount);
        goto error;
    }

    if (paramCount == 1) {
        // delete
        asprintf(&tmp, "AT+EPBUM=3,5,%d,1", params[0]);
        if (sizeof(cmd) > strlen(tmp)) {
            strncpy(cmd, tmp, strlen(tmp) + 1);
        }
        free(tmp);
    } else {
        // write
        asprintf(&tmp, "AT+EPBUM=2,5,%d,1", params[0]);
        if (sizeof(cmd) > strlen(tmp)) {
            strncpy(cmd, tmp, strlen(tmp) + 1);
        }
        free(tmp);
        for (i = 1; i < paramCount; i ++) {
            asprintf(&tmp, ",%d", params[i]);
            LOGD("sizeof(cmd)= %zu, strlen(cmd)=%zu, strlen(tmp)=%zu", sizeof(cmd), strlen(cmd), strlen(tmp));
            if ((sizeof(cmd) - strlen(cmd)) > strlen(tmp)) {
                strncat(cmd, tmp, strlen(tmp) + 1);
            }
            free(tmp);
        }
    }

    err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
    if (err < 0 || NULL == p_response) {
        LOGE("requestWriteUpbGrpEntry Fail");
        goto error;
    }

    if (p_response->success == 0) {
        switch (at_get_cme_error(p_response)) {
            LOGD("p_response = %d /n", at_get_cme_error(p_response) );
            case CME_SUCCESS:
                ret = RIL_E_GENERIC_FAILURE;
                break;
            case CME_UNKNOWN:
                LOGD("p_response: CME_UNKNOWN");
                break;
            default:
                break;
        }
    } else {
        ret = RIL_E_SUCCESS;
    }

    if (ret != RIL_E_SUCCESS) {
        LOGE("requestWriteUpbGrpEntry: Error!");
    }

error:
    RIL_onRequestComplete(t, ret, NULL, 0);
    at_response_free(p_response);
}

void requestGetPhoneBookStringsLength(void *data, size_t datalen, RIL_Token t) {
    LOGD("requestGetPhoneBookStringsLength: entering");
    ATResponse *p_response = NULL;
    int err;
    int cmeError;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    // <nlength>,<tlength>,<glength>,<slength>,<elength>
    int response[5] = {0};
    int tem;
    char *line;
    char * temStr;
    char* cmd = NULL;

    RIL_SIM_UNUSED_PARM(data);
    RIL_SIM_UNUSED_PARM(datalen);

    err = at_send_command_singleline("AT+CPBR=?", "+CPBR:", &p_response, SIM_CHANNEL_CTX);

    if (err < 0 || NULL == p_response) {
        LOGE("requestGetPhoneBookStringsLength Fail");
        goto error;
    }

    if (p_response->success == 0) {
        switch (at_get_cme_error(p_response)) {
            LOGD("p_response = %d /n", at_get_cme_error(p_response) );
            case CME_SUCCESS:
                ret = RIL_E_GENERIC_FAILURE;
            break;
            case CME_UNKNOWN:
                LOGD("p_response: CME_UNKNOWN");
            break;
            default:
            break;
        }
    } else {
        ret = RIL_E_SUCCESS;
    }

    if (ret != RIL_E_SUCCESS) {
        LOGE("requestGetPhoneBookStringsLength: Error!");
        goto error;
    }

    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0) goto error;
    LOGD("requestGetPhoneBookStringsLength: strlen of response is %zu", strlen(line));
    // +CPBR: (<bIndex>-<eIndex>), <max_num_len>, <max_alpha_len>
    err = at_tok_nextstr(&line, &temStr);
    if (err < 0) goto error;
    err = at_tok_nextint(&line, &response[0]);
    if (err < 0) goto error;
    err = at_tok_nextint(&line, &response[1]);
    if (err < 0) goto error;
    at_response_free(p_response);
    p_response = NULL;
    LOGD("requestGetPhoneBookStringsLength: end AT+CPBR=?");
    err = at_send_command_singleline("AT+EPBUM=?", "+EPBUM:", &p_response, SIM_CHANNEL_CTX);

        if (err < 0 || NULL == p_response) {
            LOGE("requestGetPhoneBookStringsLength epbum Fail");
            goto error;
        }

        if (p_response == NULL || p_response->success == 0) {
            switch (at_get_cme_error(p_response)) {
                LOGD("p_response = %d /n", at_get_cme_error(p_response) );
                case CME_SUCCESS:
                    ret = RIL_E_GENERIC_FAILURE;
                break;
                case CME_UNKNOWN:
                    LOGD("p_response epbum: CME_UNKNOWN");
                break;
                default:
                break;
            }
        } else {
            ret = RIL_E_SUCCESS;
        }

        if (ret != RIL_E_SUCCESS) {
            LOGE("requestGetPhoneBookStringsLength:epbum  Error!");
            goto error;
        }

        line = p_response->p_intermediates->line;
        err = at_tok_start(&line);
        LOGD("requestGetPhoneBookStringsLength: epbum strlen of response is %zu", strlen(line));
        // +CPBR: <n_anr>, <n_email>, <n_sne>, <n_aas>, <l_aas>, <n_gas>, <l_gas>, <n_grp>
        err = at_tok_nextint(&line, &tem);  // number of supported ANR for a ADN entry
        if (err < 0) goto error;

        err = at_tok_nextint(&line, &tem);  // number of supported EMAIL for a ADN entry
        if (err < 0) goto error;

        err = at_tok_nextint(&line, &tem);  // number of supported SNE for a ADN entry
        if (err < 0) goto error;

        err = at_tok_nextint(&line, &tem);  // maximum number of AAS entries
        if (err < 0) goto error;

        err = at_tok_nextint(&line, &tem);  // maximum length of the AAS in the entries
        if (err < 0) goto error;

        err = at_tok_nextint(&line, &tem);  // maximum number of GAS entries
        if (err < 0) goto error;

        err = at_tok_nextint(&line, &response[2]);  // maximum length of the GAS in the entries
        if (err < 0) goto error;
        at_response_free(p_response);
        p_response = NULL;
        LOGD("requestGetPhoneBookStringsLength: end AT+EPBUM=?");

        // query the SNE info +EPBUM:<type>,<Index1>,<M_NUM>,<A_NUM>,<L_SNE>
        err = at_send_command_singleline("AT+EPBUM=0,2,1", "+EPBUM:", &p_response,
            SIM_CHANNEL_CTX);
        if (err >= 0 && NULL != p_response && p_response->success > 0) {
            line = p_response->p_intermediates->line;
            err = at_tok_start(&line);
            err = at_tok_nextint(&line, &tem);
            err = at_tok_nextint(&line, &tem);
            err = at_tok_nextint(&line, &tem);
            err = at_tok_nextint(&line, &tem);
            err = at_tok_nextint(&line, &response[3]);
        }
        at_response_free(p_response);
        p_response = NULL;
        // query the email info +EPBUM:<type>,<Index1>,<M_NUM>,<A_NUM>,<L_EMAIL>
        err = at_send_command_singleline("AT+EPBUM=0,1,1", "+EPBUM:", &p_response, SIM_CHANNEL_CTX);
        if (err >= 0 && NULL != p_response && p_response->success > 0) {
            line = p_response->p_intermediates->line;
            err = at_tok_start(&line);
            err = at_tok_nextint(&line, &tem);
            err = at_tok_nextint(&line, &tem);
            err = at_tok_nextint(&line, &tem);
            err = at_tok_nextint(&line, &tem);
            err = at_tok_nextint(&line, &response[4]);
        }
        RIL_onRequestComplete(t, RIL_E_SUCCESS, response, 5 * sizeof(int));
        at_response_free(p_response);
        return;
    error:
        LOGE("requestGetPhoneBookStringsLength: process error");
        RIL_onRequestComplete(t, ret, response,  5 * sizeof(int));
        at_response_free(p_response);
}

void requestGetPhoneBookMemStorage(void *data, size_t datalen, RIL_Token t) {
    ATResponse *p_response = NULL;
    int err;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    RIL_PHB_MEM_STORAGE_RESPONSE* entry = NULL;
    char *line;

    RIL_SIM_UNUSED_PARM(data);
    RIL_SIM_UNUSED_PARM(datalen);

    err = at_send_command_singleline("AT+CPBS?", "+CPBS:", &p_response, SIM_CHANNEL_CTX);
    if (err < 0 || p_response == NULL) {
        goto error;
    }

    if (p_response->success == 0) {
        switch (at_get_cme_error(p_response)) {
            LOGD("p_response = %d /n", at_get_cme_error(p_response));
            case CME_SUCCESS:
                ret = RIL_E_GENERIC_FAILURE;
                goto error;
                break;
            default:
                goto error;
        }
    } else {
        ret = RIL_E_SUCCESS;
    }

    entry = calloc(1, sizeof(RIL_PHB_MEM_STORAGE_RESPONSE));
    if (entry == NULL) {
        LOGE("entry calloc fail");
        ret = RIL_E_NO_MEMORY;
        goto error;
    }
    // +CPBS: <STORAGE>, <used>, <total>
    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextstr(&line, &entry->storage);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &entry->used);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &entry->total);
    if (err < 0) goto error;

    RIL_onRequestComplete(t, RIL_E_SUCCESS, entry, sizeof(RIL_PHB_MEM_STORAGE_RESPONSE));
    at_response_free(p_response);
    free(entry);
    return;
error:
    RIL_onRequestComplete(t, ret, NULL, 0);
    at_response_free(p_response);
    if (entry != NULL) free(entry);
}

void requestSetPhoneBookMemStorage(void *data, size_t datalen, RIL_Token t) {
    ATResponse *p_response = NULL;
    char *cmd;
    int err;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    const char** strings = (const char**) data;
    const char *storage = strings[0];
    const char *password = strings[1];

    RIL_SIM_UNUSED_PARM(datalen);

    LOGD("Select the Storage: %s, password: %s", storage, password);
    if (password != NULL) {
        asprintf(&cmd, "AT+EPIN2=\"%s\"", password);
        err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
        free(cmd);
        if (err < 0 || p_response == NULL) {
            LOGE("requestSetPhoneBookMemStorage set pin2 Fail:%d", err);
            goto error;
        }

        if (p_response->success == 0) {
            switch (at_get_cme_error(p_response)) {
            case CME_SUCCESS:
                ret = RIL_E_GENERIC_FAILURE;
                goto error;
                break;
            default:
                goto error;
            }
        } else {
            ret = RIL_E_SUCCESS;
        }
        at_response_free(p_response);
        p_response = NULL;
    }

    asprintf(&cmd, "AT+CPBS=\"%s\"", storage);
    err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
    free(cmd);

    if (err < 0 || NULL == p_response) {
        ret = RIL_E_GENERIC_FAILURE;
        goto error;
    } else {
        int type = -1;
        if (0 == strcmp("SM", storage))
            type = RIL_PHB_ADN;
        if (0 == strcmp("FD", storage))
            type = RIL_PHB_FDN;
        if (0 == strcmp("ON", storage))
            type = RIL_PHB_MSISDN;
        if (0 == strcmp("EN", storage))
            type = RIL_PHB_ECC;
        current_phb_storage[rid] = type;
    }
    if (p_response == NULL || p_response->success == 0) {
        ret = RIL_E_GENERIC_FAILURE;
    } else {
        ret = RIL_E_SUCCESS;
    }
error:
    RIL_onRequestComplete(t, ret, NULL, 0);
    at_response_free(p_response);
}

void loadUPBCapability(RIL_Token t) {
    ATResponse *p_response = NULL;
    int err;
    char *line;
    int upbCap[8]= {0};
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));

    err = at_send_command_singleline("AT+EPBUM=?", "+EPBUM:", &p_response, SIM_CHANNEL_CTX);
    LOGD("loadUPBCapability ril %d", rid);
    if (err < 0 || NULL == p_response) {
        LOGE("loadUPBCapability Fail");
        goto error;
    }

    if (p_response->success == 0) {
        switch (at_get_cme_error(p_response)) {
            LOGD("p_response = %d /n", at_get_cme_error(p_response) );
            case CME_SUCCESS:
                ret = RIL_E_GENERIC_FAILURE;
            break;
            case CME_UNKNOWN:
                LOGD("p_response: CME_UNKNOWN");
            break;
            default:
            break;
        }
    } else {
        ret = RIL_E_SUCCESS;
    }

    if (ret != RIL_E_SUCCESS) {
        LOGE("loadUPBCapability: Error!");
        goto error;
    }

    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    LOGD("loadUPBCapability: strlen of response is %zu", strlen(line));
    err = at_tok_nextint(&line, &upbCap[0]);  // number of supported ANR for a ADN entry
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &upbCap[1]);  // number of supported EMAIL for a ADN entry
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &upbCap[2]);  // number of supported SNE for a ADN entry
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &upbCap[3]);  // maximum number of AAS entries
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &upbCap[4]);  // maximum length of the AAS in the entries
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &upbCap[5]);  // maximum number of GAS entries
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &upbCap[6]);  // maximum length of the GAS in the entries
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &upbCap[7]);  // maximum number of GRP entries
    if (err < 0) goto error;

    if (RIL_SOCKET_1 == rid) {
        maxGrpNum[0] = upbCap[7];
        maxAnrNum[0] = upbCap[0];
        maxEmailNum[0] = upbCap[1];
    }
    else if (RIL_SOCKET_2 == rid) {
        maxGrpNum[1] = upbCap[7];
        maxAnrNum[1] = upbCap[0];
        maxEmailNum[1] = upbCap[1];
    }
    else if (RIL_SOCKET_3 == rid) {
        maxGrpNum[2] = upbCap[7];
        maxAnrNum[2] = upbCap[0];
        maxEmailNum[2] = upbCap[1];
    }
    else if (RIL_SOCKET_4 == rid) {
        maxGrpNum[3] = upbCap[7];
        maxAnrNum[3] = upbCap[0];
        maxEmailNum[3] = upbCap[1];
    }

    LOGD("loadUPBCapability: %d, %d, %d, %d, %d, %d, %d, %d",
        upbCap[0], upbCap[1], upbCap[2], upbCap[3], upbCap[4], upbCap[5],
        upbCap[6], upbCap[7]);
error:
    at_response_free(p_response);
    // p_response = NULL;
}

void requestReadPhoneBookEntryExt(void *data, size_t datalen, RIL_Token t) {
    ATResponse *p_response = NULL;
    ATResponse *p_res = NULL;
    int err,  bIndex, eIndex, count, i, tmp;
    char *line, *eline, *cmd, *out;
    ATLine *p_cur;
    RIL_PHB_ENTRY *entry = NULL;
    RIL_PHB_ENTRY **entries = NULL;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    int upbCap[8] = {0};

    RIL_SIM_UNUSED_PARM(datalen);

    selectPhbStorage(RIL_PHB_ADN, t);
    if (maxAnrNum[rid] == -1) {
        loadUPBCapability(t);
    }
    LOGD("maxAnrNum[%d]=%d", rid, maxAnrNum[rid]);
      bIndex = ((int *)data)[0];
      eIndex = ((int *)data)[1];
      if ((eIndex - bIndex + 1) <= 0) {
          LOGE("Begin index or End Index is invalid: %d %d", bIndex, eIndex);
          goto error;
      }

      asprintf(&cmd, "AT+CPBR=%d, %d", bIndex, eIndex);
      err = at_send_command_multiline(cmd, "+CPBR:", &p_response, SIM_CHANNEL_CTX);
      free(cmd);

      if (err < 0 || p_response->success == 0) {
          LOGE("requestReadPhoneBookEntryExt CPBR error.");
          goto error;
      }
      /* count the calls */
      for (count = 0, p_cur = p_response->p_intermediates
              ; p_cur != NULL
              ; p_cur = p_cur->p_next
      ) {
          count++;
      }
      LOGD("requestReadPhoneBookEntryExt end count=%d", count);
//      /* allocate memory and parse +CPBR*/
      entries = calloc(1, sizeof(RIL_PHB_ENTRY*) * count);
      if (entries == NULL) {
          LOGE("entries calloc fail");
          goto error;
      }

      // +CPBR: <index>, <number>, <TON>, <alphaId>
      for (i = 0, p_cur = p_response->p_intermediates
              ; p_cur != NULL
              ; p_cur = p_cur->p_next, i++) {
          line = p_cur->line;
          entry = calloc(1, sizeof(RIL_PHB_ENTRY));
          if (entry == NULL) {
              LOGE("entry calloc fail");
              goto error;
          }
          entries[i] = entry;
          entry->hidden = 0;
          entry->index = 0;
          entry->type = 0;
          entry->number = "";
          entry->text = "";
          entry->group = "";
          entry->adnumber = "";
          entry->adtype = 0;
          entry->secondtext = "";
          entry->email = "";
          err = at_tok_start(&line);
          if (err < 0) goto error;
          err = at_tok_nextint(&line, &entry->index);
          if (err < 0) goto error;
          err = at_tok_nextstr(&line, &out);
          if (err < 0) goto error;
          entry->number = alloca(strlen(out) + 1);
          strncpy(entry->number, out, strlen(out) + 1);
          err = at_tok_nextint(&line, &entry->type);
          if (err < 0) goto error;
          err = at_tok_nextstr(&line, &out);
          if (err < 0) goto error;
          entry->text = alloca(strlen(out) + 1);
          strncpy(entry->text, out, strlen(out) + 1);
          entry->hidden = 0;
          DLOGD("PhbEntry %d: %s, %d, %s", entry->index,
              entry->number, entry->type, entry->text);
          // ANR
          if (maxAnrNum[rid] > 0 && (strlen(entry->text) > 0 || strlen(entry->number) > 0)) {
              if (bIsTc1() == true) {
                  asprintf(&cmd, "at+epbum=1,0,%d,%d", entry->index, 1);
              } else {
                  asprintf(&cmd, "at+epbum=1,0,%d,%d", entry->index, maxAnrNum[rid]);
              }
              // +epbum:<type>,<adn_index>,<ef_index>,<anr>,<type>,<aas>
              err = at_send_command_singleline(cmd, "+EPBUM:", &p_res, SIM_CHANNEL_CTX);
              free(cmd);
              if (err < 0) {
                  LOGE("requestReadPhoneBookEntryExt anr epbum error.");
                  // goto error;
              }
              if (p_res != NULL && p_res->success != 0) {
                  eline = p_res->p_intermediates->line;
                  err = at_tok_start(&eline);
                  // if (err < 0) goto error;
                  err = at_tok_nextint(&eline, &tmp);
                  // if (err < 0) goto error;
                  err = at_tok_nextint(&eline, &tmp);
                  // if (err < 0) goto error;
                  err = at_tok_nextint(&eline, &tmp);
                  // if (err < 0) goto error;
                  err = at_tok_nextstr(&eline, &out);
                  entry->adnumber = alloca(strlen(out) + 1);
                  strncpy(entry->adnumber, out, strlen(out) + 1);
                  // if (err < 0) goto error;
                  err = at_tok_nextint(&eline, &entry->adtype);
                  // if (err < 0) goto error;
                  at_response_free(p_res);
                  p_res = NULL;
                  DLOGD("PhbEntry anr %d: %s, %d", entry->index,
                                entry->adnumber, entry->adtype);
                  }
          }
          // email
          if (maxEmailNum[rid] > 0 && (strlen(entry->text) > 0 || strlen(entry->number) > 0)) {
              if (bIsTc1() == true) {
                  asprintf(&cmd, "at+epbum=1,1,%d,%d", entry->index, 1);
              } else {
                  asprintf(&cmd, "at+epbum=1,1,%d,%d", entry->index, maxEmailNum[rid]);
              }

              at_response_free(p_res);
              p_res = NULL;

              // +epbum:<type>,<adn_index>,<ef_index>,<email>
              err = at_send_command_singleline(cmd, "+EPBUM:", &p_res, SIM_CHANNEL_CTX);
              free(cmd);

              if (err < 0) {
                  LOGE("requestReadPhoneBookEntryExt email epbum err");
                  // goto error;
              }

              if (p_res != NULL && p_res->success != 0) {
                  eline = p_res->p_intermediates->line;
                  err = at_tok_start(&eline);
                  // if (err < 0) goto error;
                  err = at_tok_nextint(&eline, &tmp);
                  // if (err < 0) goto error;
                  err = at_tok_nextint(&eline, &tmp);
                  // if (err < 0) goto error;
                  err = at_tok_nextint(&eline, &tmp);
                  // if (err < 0) goto error;
                  err = at_tok_nextstr(&eline, &out);
                  entry->email = alloca(strlen(out) + 1);
                  strncpy(entry->email, out, strlen(out) + 1);
                  // if (err < 0) goto error;
                  at_response_free(p_res);
                  DLOGD("PhbEntry email %d: %s", entry->index, entry->email);
              }
          }
      }

      RIL_onRequestComplete(t, RIL_E_SUCCESS, entries, count*sizeof(RIL_PHB_ENTRY*));
      for (i=0; i < count; i++) {
          if (entries[i] != NULL) {
              free(entries[i]);
          }
      }
      free(entries);
      at_response_free(p_response);
      return;

error:
    LOGE("requestReadPhoneBookEntryExt error");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
    if (entries != NULL) {
      for (i=0; i < count; i++) {
          if (entries[i] != NULL) {
              free(entries[i]);
          }
      }
      free(entries);
  }
}

void requestWritePhoneBookEntryExt(void *data, size_t datalen, RIL_Token t) {
    ATResponse *p_response = NULL;
    int err, len, tmp, i, j;
    RIL_PHB_ENTRY *entry;
    char *line = NULL;
    char *cmd = NULL;
    char *number = NULL;
    char alphaId[RIL_MAX_PHB_NAME_LEN*4+4+1]={0, };  // If using UCS2, each character use 4 bytes,
                                                     // if using 0x82 coding, we need extra 4 bytes,
                                                     // we also need '\0'
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    RilPhbCpbwEncode encodeMethod;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));

    RIL_SIM_UNUSED_PARM(datalen);

    entry = (RIL_PHB_ENTRY *)data;
    selectPhbStorage(RIL_PHB_ADN, t);
        if (entry->text != NULL || entry->number != NULL) {
            if (entry->text == NULL) {
                line = "";
            } else {
                line = entry->text;
            }
            if (entry->number == NULL) {
                number = "";
            } else {
                number = entry->number;
            }
            len = strlen(line);
            if (len == 0 && strlen(number) == 0) {
                LOGE("The number and alphaid can't be empty String at the same time: %s %s",
                        line, number);
                goto error;
            }

            // pack Alpha Id
            if ((len%4) != 0) {
                LOGE("The alphaId should encode using Hexdecimal: %s", line);
                goto error;
            } else if (len > (RIL_MAX_PHB_NAME_LEN*4)) {
                LOGE("The alphaId shouldn't longer than RIL_MAX_PHB_NAME_LEN");
                ret = RIL_E_TEXT_STRING_TOO_LONG;
                goto error;
            }

            for (i=0, j=0; i < len; i+=4, j++) {
                if (bIsTc1() == true) {
                    tmp = isMatchGsm7bit(line+i, 4);

                    if (tmp == -1) {
                        // Check if the char match GSM-7bit extension table
                        tmp = isGsm7bitExtension(line+i, 4);

                        if (tmp != -1) {
                            alphaId[j] = (char)(0x1b);
                            j++;
                         } else {
                            LOGD("requestWritePhbEntry isMatchGsm7bit = false");
                            break;
                         }
                    }

                    alphaId[j] = (char)tmp;
                } else {
                    tmp = rild_sms_hexCharToDecInt(line+i, 4);

                    if (tmp >= 128) {
                        break;
                    }
                    alphaId[j] = (char)tmp;
                }
            }
            alphaId[j] = '\0';

            LOGD("requestWritePhoneBookEntryExt - i = %d, len = %d", i, len);
            if (i != len) {
                len /= 4;

                if (encodeUCS2_0x81(line, alphaId, sizeof(alphaId))) {  // try UCS2_0x81 coding
                    // UCS2 0x81 coding
                    encodeMethod = CPBW_ENCODE_UCS2_81;
                } else if (encodeUCS2_0x82(line, alphaId, sizeof(alphaId))) {
                    // try UCS2_0x82 coding
                    // UCS2 0x82 coding
                    encodeMethod = CPBW_ENCODE_UCS2_82;
                } else {
                    // UCS2 coding
                    encodeMethod = CPBW_ENCODE_UCS2;
                    LOGD("requestWritePhoneBookEntryExt - alphaId length = %zu, line length = %zu",
                            sizeof(alphaId), strlen(line));
                    memcpy(alphaId, line, ((sizeof(alphaId) < strlen(line)) ? sizeof(alphaId) : strlen(line)));
                }
            }
            else {
                if (bIsTc1() == true) {
                    LOGD("requestWritePhoneBookEntryExt - GSM-7Bit");
                    encodeMethod = CPBW_ENCODE_GSM7BIT;
                    memcpy(alphaId, line, ((sizeof(alphaId) < strlen(line)) ? sizeof(alphaId) : strlen(line)));
                    LOGD("requestWritePhoneBookEntryExt - sizeof(alphaId) = %zu , strlen(line) = %zu",
                            sizeof(alphaId), strlen(line));
                    DLOGD("requestWritePhoneBookEntryExt - line    = %s", line);
                    DLOGD("requestWritePhoneBookEntryExt - alphaId = %s", alphaId);
                } else {
                    encodeMethod = CPBW_ENCODE_IRA;
                    // ASCII coding
                }
            }
            requestClearPhbEntry(entry->index, t);
            asprintf(&cmd, "AT+EPBW=%d, \"%s\", %d, \"%s\", 0, %d",
                        entry->index, number, entry->type, alphaId, encodeMethod);

        } else {
            // delete
            asprintf(&cmd, "AT+EPBW=%d", entry->index);
        }


        err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
        free(cmd);
        if (err < 0 || p_response == NULL) {
            LOGE("requestWritePhoneBookEntryExt EPBW Error!!!!");
            goto error;
        }

        if (p_response->success == 0) {
            switch (at_get_cme_error(p_response)) {
                case CME_SUCCESS:
                    ret = RIL_E_GENERIC_FAILURE;
                    break;
                case CME_SIM_PIN2_REQUIRED:
                    ret = RIL_E_SIM_PIN2;
                    break;
                case CME_SIM_PUK2_REQUIRED:
                    ret = RIL_E_SIM_PUK2;
                    break;
                case CME_DIAL_STRING_TOO_LONG:
                    ret = RIL_E_DIAL_STRING_TOO_LONG;
                    break;
                case CME_TEXT_STRING_TOO_LONG:
                    ret = RIL_E_TEXT_STRING_TOO_LONG;
                    break;
                case CME_MEMORY_FULL:
                    ret = RIL_E_SIM_MEM_FULL;
                    break;
                default:
                    break;
            }
        } else {
            ret = RIL_E_SUCCESS;
        }
        LOGD("requestWritePhoneBookEntryExt ret %d", ret);
        at_response_free(p_response);
        p_response = NULL;

        if (maxAnrNum[rid] == -1) {
            loadUPBCapability(t);
        }

        // ANR
        if (ret == RIL_E_SUCCESS && maxAnrNum[rid] > 0) {
            // at+epbum=<op>,<type>,<adn_index>,<ef_index>,<anr>,<type>,<aas>
            if (bIsTc1() == true) {
                if (entry->adnumber != NULL && strlen(entry->adnumber) > 0) {
                    asprintf(&cmd, "at+epbum=2,0,%d,%d,\"%s\",%d,0", entry->index, 1, entry->adnumber, entry->adtype);
                } else {
                    asprintf(&cmd, "at+epbum=3,0,%d,%d", entry->index, 1);
                }
            } else {
                if (entry->adnumber != NULL && strlen(entry->adnumber) > 0) {
                    asprintf(&cmd, "at+epbum=2,0,%d,%d,\"%s\",%d,0", entry->index, maxAnrNum[rid],
                            entry->adnumber, entry->adtype);
                } else {
                    asprintf(&cmd, "at+epbum=3,0,%d,%d", entry->index, maxAnrNum[rid]);
                }
            }

            err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
            free(cmd);
            if (err < 0 || p_response->success == 0) {
                LOGE("requestWritePhoneBookEntryExt EPBUM ANR Error!!!!");
            }
            at_response_free(p_response);
            p_response = NULL;
        }

        // EMAIL
        if (ret == RIL_E_SUCCESS && maxEmailNum[rid] > 0) {
            if (bIsTc1() == true) {
                if (entry->email != NULL && strlen(entry->email) > 0) {
                    // at+epbum=<op>,<type>,<adn_index>,<ef_index>,<email>
                    asprintf(&cmd, "at+epbum=2,1,%d,%d,\"%s\"", entry->index, 1, entry->email);
                } else {
                    asprintf(&cmd, "at+epbum=3,1,%d,%d", entry->index, 1);
                }
            } else {
                if (entry->email != NULL && strlen(entry->email) > 0) {
                    // at+epbum=<op>,<type>,<adn_index>,<ef_index>,<email>
                    asprintf(&cmd, "at+epbum=2,1,%d,%d,\"%s\"", entry->index, maxEmailNum[rid], entry->email);
                } else {
                    asprintf(&cmd, "at+epbum=3,1,%d,%d", entry->index, maxEmailNum[rid]);
                }
            }

            err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
            free(cmd);
            if (err < 0 || p_response->success == 0) {
                LOGE("requestWritePhoneBookEntryExt EPBUM Email Error!!!!");
            }
            at_response_free(p_response);
            p_response = NULL;
        }

error:
    RIL_onRequestComplete(t, ret, NULL, 0);
    at_response_free(p_response);
}

// PHB Enhance
void requestQueryUPBAvailable(void *data, size_t datalen, RIL_Token t) {
    LOGD("requestQueryUPBAvailable: entering");
    ATResponse *p_response = NULL;
    int err;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    int response[3] = {0};
    int tem;
    char *line;
    char * temStr;
    char* cmd = NULL;
    int* params = (int *) data;
    int paramCount = datalen/sizeof(int);

    // query the info +EPBUM:<type>, <INDEX1>, <M_NUM>, <A_NUM>, <L_XXX>
    asprintf(&cmd, "AT+EPBUM=0,%d,%d", params[0], params[1]);
    err = at_send_command_singleline(cmd, "+EPBUM:", &p_response,
        SIM_CHANNEL_CTX);
    free(cmd);
    if (err >= 0 && NULL != p_response && p_response->success > 0) {
        line = p_response->p_intermediates->line;
        err = at_tok_start(&line);
        err = at_tok_nextint(&line, &tem);
        err = at_tok_nextint(&line, &tem);
        err = at_tok_nextint(&line, &response[0]);
        err = at_tok_nextint(&line, &response[1]);
        err = at_tok_nextint(&line, &response[2]);
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, 3 * sizeof(int));
    at_response_free(p_response);
    return;
}

void requestReadUPBEmail(void *data, size_t datalen, RIL_Token t) {
    ATResponse *p_response = NULL;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    int err = 0;
    int i, tmp, strLen;
    char *line = NULL;
    char *cmd = NULL;
    char *out = NULL;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    int upbCap[8] = {0};

    int* params = (int *) data;
    int paramCount = datalen/sizeof(int);
    char* response = NULL;

    if (isSimInserted(rid) == 0) {
        LOGD("requestReadUPBEmail no sim inserted");
        goto error;
    }

    // selectPhbStorage(RIL_PHB_ADN, t);
    if (maxEmailNum[rid] == -1) {
        loadUPBCapability(t);
    }

    // email
    if (maxEmailNum[rid] > 0) {
      asprintf(&cmd, "at+epbum=1,1,%d,%d", params[0], params[1]);

      // +epbum:<type>,<adn_index>,<ef_index>,<email>
      err = at_send_command_singleline(cmd, "+EPBUM:", &p_response, SIM_CHANNEL_CTX);
      free(cmd);

      if (err < 0) {
          // LOGE("requestReadUPBEmail email epbum err: %d", err);
          goto error;
      }

      if (p_response != NULL && p_response->success == 0) {
        switch (at_get_cme_error(p_response)) {
            LOGD("p_response = %d /n", at_get_cme_error(p_response) );
            case CME_SUCCESS:
                ret = RIL_E_GENERIC_FAILURE;
            break;
            case CME_UNKNOWN:
                LOGD("p_response: CME_UNKNOWN");
            break;
            default:
            break;
        }
      } else {
        ret = RIL_E_SUCCESS;
      }

      if (p_response != NULL && p_response->success != 0 && p_response->p_intermediates != NULL) {
          line = p_response->p_intermediates->line;
          err = at_tok_start(&line);
          if (err < 0) goto error;
          err = at_tok_nextint(&line, &tmp);
          if (err < 0) goto error;
          err = at_tok_nextint(&line, &tmp);
          if (err < 0) goto error;
          err = at_tok_nextint(&line, &tmp);
          if (err < 0) goto error;
          err = at_tok_nextstr(&line, &out);
          if (err < 0) goto error;
          response = alloca(strlen(out) + 1);
          strncpy(response, out, strlen(out) + 1);
          // LOGD("requestReadUPBEmail email %s",  response); // pii
          // response = ascii2UCS2(response);
      }

      strLen = response == NULL ? 0 : strlen(response);
      RIL_onRequestComplete(t, ret, response, strLen);
      at_response_free(p_response);
      return;
    }
error:
    // LOGE("requestReadUPBEmail error: %d", err);
    RIL_onRequestComplete(t, ret, NULL, 0);
    at_response_free(p_response);
}

void requestReadUPBSne(void *data, size_t datalen, RIL_Token t) {
    ATResponse *p_response = NULL;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    int err = 0;
    int i, tmp, strLen;
    char *line = NULL;
    char *cmd = NULL;
    ATLine *p_cur = NULL;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    int upbCap[8] = {0};

    int* params = (int *) data;
    int paramCount = datalen/sizeof(int);
    char* response = NULL;

    if (isSimInserted(rid) == 0) {
        LOGD("requestReadUPBSne no sim inserted");
        goto error;
    }

    if (paramCount < 1) {
        LOGE("requestReadUpbGrpEntry param is not enough. paramCount is %d", paramCount);
        goto error;
    }

    // SNE
    asprintf(&cmd, "at+epbum=1,2,%d,%d", params[0], 1);
    // +epbum:<type>,<adn_index>,<ef_index>,<sne>,<encode_type>
    err = at_send_command_singleline(cmd, "+EPBUM:", &p_response, SIM_CHANNEL_CTX);
    free(cmd);
    if (err < 0) {
        // LOGE("requestReadUPBSne sne epbum err: %d", err);
        goto error;
    }

    if (p_response != NULL && p_response->success == 0) {
        switch (at_get_cme_error(p_response)) {
            LOGD("p_response = %d /n", at_get_cme_error(p_response) );
            case CME_SUCCESS:
                ret = RIL_E_GENERIC_FAILURE;
                break;
            case CME_UNKNOWN:
                LOGD("p_response: CME_UNKNOWN");
                break;
            default:
                break;
        }
    } else {
        ret = RIL_E_SUCCESS;
    }

      LOGD("requestReadUPBSne ret: %d", ret);
      if (p_response != NULL && p_response->success != 0 && p_response->p_intermediates != NULL) {
          ret = RIL_E_SUCCESS;
          line = p_response->p_intermediates->line;
          err = at_tok_start(&line);
          LOGD("requestReadUPBSne: strlen of response is %zu", strlen(line) );
          err = at_tok_nextint(&line, &tmp);
          if (err < 0) goto error;
          err = at_tok_nextint(&line, &tmp);
          if (err < 0) goto error;
          err = at_tok_nextint(&line, &tmp);
          if (err < 0) goto error;
          err = at_tok_nextstr(&line, &response);
          if (err < 0) goto error;
          err = at_tok_nextint(&line, &tmp);
          if (err < 0) goto error;
          LOGD("requestReadUPBSne: encode type is %d", tmp);

          if (tmp == CPBW_ENCODE_IRA) {
              response = ascii2UCS2(response);
              if (response == NULL) {
                  ret = RIL_E_NO_MEMORY;
                  goto error;
              }
          }
          DLOGD("requestReadUPBSne sne %s",  response);
      }

      strLen = (response == NULL ? 0 : strlen(response));
      RIL_onRequestComplete(t, ret, response, strLen);
      at_response_free(p_response);
      return;
error:
    // LOGE("requestReadUPBSne error: %d", err);
    RIL_onRequestComplete(t, ret, NULL, 0);
    at_response_free(p_response);
}

void requestReadUPBAnr(void *data, size_t datalen, RIL_Token t) {
    ATResponse *p_response = NULL;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    int err = 0;
    int i = 0;
    int tmp = 0;
    char *line = NULL;
    char *cmd = NULL;
    char *out = NULL;
    ATLine *p_cur = NULL;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    int upbCap[8] = {0};

    int* params = (int *) data;
    int paramCount = datalen/sizeof(int);
    // char* response = NULL;
    RIL_PhbEntryStructure *entry = NULL;
    RIL_PhbEntryStructure **entries = NULL;

    if (isSimInserted(rid) == 0) {
        LOGD("requestReadUPBAnr no sim inserted");
        goto error;
    }

    // selectPhbStorage(RIL_PHB_ADN, t);
    if (maxAnrNum[rid] == -1) {
        loadUPBCapability(t);
    }
    // ANR
    if ( maxAnrNum[rid] > 0 ) {
        entries = calloc(1, sizeof(RIL_PhbEntryStructure*)*1);
        if (entries == NULL) {
            LOGE("entries calloc fail");
            goto error;
        }
        entries[0] = entry = calloc(1, sizeof(RIL_PhbEntryStructure));
        if (entry == NULL) {
            LOGE("entry calloc fail");
            goto error;
        }

        asprintf(&cmd, "at+epbum=1,0,%d,%d", params[0], params[1]);
        // +epbum:<type>,<adn_index>,<ef_index>,<anr>,<type>,<aas>
        err = at_send_command_singleline(cmd, "+EPBUM:", &p_response, SIM_CHANNEL_CTX);
        free(cmd);
        if (err < 0) {
            // LOGE("requestReadUPBAnr anr epbum error: %d", err);
            goto error;
        }
        if (p_response != NULL && p_response->success == 0) {
            switch (at_get_cme_error(p_response)) {
                LOGD("p_response = %d /n", at_get_cme_error(p_response) );
                case CME_SUCCESS:
                    ret = RIL_E_GENERIC_FAILURE;
                break;
                case CME_UNKNOWN:
                    LOGD("p_response: CME_UNKNOWN");
                break;
                default:
                break;
            }
        } else {
            ret = RIL_E_SUCCESS;
        }
        LOGD("requestReadUPBAnr ret: %d", ret);
        if (p_response != NULL && p_response->success != 0 && p_response->p_intermediates != NULL) {
            ret = RIL_E_SUCCESS;
            line = p_response->p_intermediates->line;
            err = at_tok_start(&line);
            if (err < 0) goto error;
            err = at_tok_nextint(&line, &tmp);
            if (err < 0) goto error;
            err = at_tok_nextint(&line, &tmp);
            if (err < 0) goto error;
            err = at_tok_nextint(&line, &tmp);
            if (err < 0) goto error;
            err = at_tok_nextstr(&line, &out);
            entry->number = alloca(strlen(out) + 1);
            strncpy(entry->number, out, strlen(out) + 1);
            if (err < 0) goto error;
            err = at_tok_nextint(&line, &entry->ton);
            if (err < 0) goto error;
            err = at_tok_nextint(&line, &entry->index);
            at_response_free(p_response);
            p_response = NULL;
            // LOGD("requestReadUPBAnr PhbEntry anr %d: %s, %d", entry->index,
            //                entry->number, entry->ton); // pii
            entry->alphaId = "";
            entry->type = 0;
        }

        RIL_onRequestComplete(t, ret, entries, 1*sizeof(RIL_PhbEntryStructure*));
        if (entries[0] != NULL) {
            free(entries[0]);
        }
        free(entries);
        at_response_free(p_response);
        return;
     }
error:
    // LOGE("requestReadUPBAnr error: %d", err);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
    if (entries != NULL) {
        if (entries[0] != NULL) {
            free(entries[0]);
        }
        free(entries);
    }
}

void requestReadAasList(void *data, size_t datalen, RIL_Token t) {
    ATResponse **p_response = NULL;
    int err;
    char *cmd = NULL;
    char *line;
    int tmp, i, entryIndex;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    int* params = (int *) data;
    char **responses = NULL;

    if (datalen/sizeof(int) < 2 || params[0] < 0 || params[1] < params[0]) {
        LOGE("requestReadAasList param is not enough. datalen is %zu", datalen);
        goto error;
    }
    p_response = calloc(1, sizeof(ATResponse*)*(params[1] - params[0] + 1));
    if (p_response == NULL) {
        LOGE("p_response calloc fail");
        ret = RIL_E_NO_MEMORY;
        goto error;
    }
    responses = calloc(1, sizeof(char*)*(params[1] - params[0] + 1));
    if (responses == NULL) {
        LOGE("responses calloc fail");
        ret = RIL_E_NO_MEMORY;
        goto error;
    }

    for (i = params[0]; i <= params[1]; i++) {
        asprintf(&cmd, "AT+EPBUM=1,3,%d,0", i);

        err = at_send_command_singleline(cmd, "+EPBUM:", &p_response[i-1], SIM_CHANNEL_CTX);
        free(cmd);
        if (err < 0 || p_response[i-1] == NULL) {
            LOGD("requestReadAasList: the %d th aas entry is null", i);
            ret = RIL_E_SUCCESS;
            continue;  // it means the index is not used.
        }

        if (p_response[i-1]->success == 0) {
            switch (at_get_cme_error(p_response[i-1])) {
                LOGD("p_response = %d /n", at_get_cme_error(p_response[i-1]) );
                case CME_SUCCESS:
                    ret = RIL_E_GENERIC_FAILURE;
                    break;
                case CME_UNKNOWN:
                    ret = RIL_E_GENERIC_FAILURE;
                    LOGD("p_response: CME_UNKNOWN");
                    break;
                default:
                    ret = RIL_E_GENERIC_FAILURE;
                    LOGD("p_response: Not success");
                    break;
            }
        } else {
            ret = RIL_E_SUCCESS;
        }

        if (ret != RIL_E_SUCCESS) {
            LOGD("requestReadAasList: Error!");
            // goto error;
            continue;
        }

        if (p_response[i-1] == NULL || p_response[i-1]->p_intermediates == NULL) continue;
        line = p_response[i-1]->p_intermediates->line;
        err = at_tok_start(&line);
        LOGD("requestReadAasList: strlen of response is %zu", strlen(line));

        err = at_tok_nextint(&line, &tmp);  // type
        if (err < 0 || tmp != 3) continue;

        err = at_tok_nextint(&line, &entryIndex);  // entry_index
        if (err < 0 || entryIndex < 1) continue;;
        LOGD("requestReadAasList: (entryIndex-1)=%d", entryIndex-1);

        err = at_tok_nextint(&line, &tmp);  // adn_index
        err = at_tok_nextstr(&line, &responses[entryIndex-1]);  // aas string
        if (err < 0) continue;
        LOGD("requestReadAasList:  %s", responses[entryIndex-1]);

        err = at_tok_nextint(&line, &tmp);
        if (err < 0) continue;
        LOGD("requestReadAasList: encode type is %d", tmp);

        if (tmp == CPBW_ENCODE_IRA) {
            responses[entryIndex-1] = ascii2UCS2(responses[entryIndex-1]);
            if (responses[entryIndex-1] == NULL) {
                for (i = params[0]; i <= params[1]; i++) {
                    at_response_free(p_response[i-1]);
                }
                free(p_response);
                free(responses);
                ret = RIL_E_NO_MEMORY;
                goto error;
            }
        }

        LOGD("requestReadAasList:  [%d], %s", entryIndex-1, responses[entryIndex-1]);
    }

    RIL_onRequestComplete(t, ret, responses, (params[1] - params[0] + 1)*sizeof(char*));
    for (i = params[0]; i <= params[1]; i++) {
        at_response_free(p_response[i-1]);
    }
    free(p_response);
    free(responses);
    return;
error:
    RIL_onRequestComplete(t, ret, NULL, 0);
}

void requestSetPhonebookReady(void *data, size_t datalen, RIL_Token t) {
    RIL_SIM_UNUSED_PARM(datalen);
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    int ready = ((int *)data)[0];
    if (ready == 0) {
        setMSimProperty(rid, PROPERTY_RIL_PHB_READY, "false");
    } else if (ready == 1) {
        setMSimProperty(rid, PROPERTY_RIL_PHB_READY, "true");
    }
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    LOGD("requestSetPhonebookReady complete: %d", ready);
    return;
}
// PHB Enhance
