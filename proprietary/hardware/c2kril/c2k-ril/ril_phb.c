/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <alloca.h>
#include <getopt.h>
#include <sys/socket.h>
#include <cutils/sockets.h>
#include <termios.h>
#include "atchannel.h"
#include "ril_callbacks.h"
#include "icc.h"
#include <utils/Log.h>

#include <cutils/properties.h>
#include <netdb.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <linux/route.h>
#include "misc.h"
#include "at_tok.h"
#include <libmtkrilutils.h>

#define LOG_TAG "C2K_RIL-PHB"

#define PHB_CHANNEL_CTX         getRILChannelCtxFromToken(t)
#define CDMA_UIM_ALPHAID_LENGTH 14
#define CDMA_UIM_NUMBER_LENGTH  20
#define PHB_NAME_MAX_SIZE       256
#define LEN_BYTES_0X81          2
#define UNICODE_CHAR_LEN        2
#define PHB_UNUSED(x)           ((void)(x))

#define RIL_E_DIAL_STRING_TOO_LONG RIL_E_OEM_ERROR_1
#define RIL_E_TEXT_STRING_TOO_LONG RIL_E_OEM_ERROR_2
#define PROPERTY_C2K_RIL_PHB_READY "vendor.cdma.sim.ril.phbready"

/* M: PHB Revise
   To distinguish the PHB event between GSM and C2K,
   we take the first bit as the ready/not ready value
   and the second bit as the type: 0 for GSM and 1 for C2K
*/
static int GSM_PHB_NOT_READY = 0;   // 00
static int GSM_PHB_READY = 1;       // 01
static int C2K_PHB_NOT_READY = 2;   // 10
static int C2K_PHB_READY = 3;       // 11

#define RIL_PHB_UCS2_81_MASK    0x7f80
#define SPEC_ENCODING_ARR_LEN   54
static unsigned short specialEncoding[SPEC_ENCODING_ARR_LEN][2] = {
    {0x0040, 0x0000},
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
};

typedef enum
{
    PHB_ENCODE_UCS2 = 1,
    PHB_ENCODE_UCS2_81 = PHB_ENCODE_UCS2,
    PHB_ENCODE_ASCII = 2,
    PHB_ENCODE_MAX
} RilC2kPhbEncode;

typedef struct {
    int index;
    char *number;
    int type;
    char *name;
    int coding;
} RIL_Ruim_Phonebook_Record;

/**
 * When access UIM card PHB, we need check the PHB params frequently,
 * so we add 3 static global variables to remeber the params
 * of UIM card PHB, when use them, we can get them directly,
 * and dont need to send AT commands to modem .
 *
 * Use static global variables instead of using system property.[2013-11-8]
 *
 */
// the max count the UIM card can save
static int s_PHB_max_capacity;
// the max bytes count of the number field
static int s_PHB_number_max_bytes;
// the max bytes count of the alphaid(name) field
static int s_PHB_alphaid_max_bytes;

static const struct timeval TIMEVAL_0 = {0,0};

// RIL request handle function
static void requestGetPhbStorageInfo(void *data, size_t datalen, RIL_Token t);
static void requestWritePhbEntry(void *data, size_t datalen, RIL_Token t);
static void requestReadPhbEntry(void *data, size_t datalen, RIL_Token t);

static unsigned int getNameMaxLength();
static unsigned int getNumberMaxLength();
static int parsePhb(char *line, RIL_Ruim_Phonebook_Record *readPb);
static int getPhbSize(int pbSize[]);
static int getCurrentPhbStorage(RIL_Token t);
static int selectPhbStorage(int type, RIL_Token t);
static void setPhbStorageInfo(int max, int numberMax, int alphaMax);
static void convertNameFromMd(char * dest_alphaid, char * src_alphaid);
static RilC2kPhbEncode convertNameToMd(char* dest_alphaid, char* src_alphaid, int size, int* err);
static void convertNumberToMd(char *srcNumber, char *destNumber, int ton);
static void convertNumberFromMd(char *srcNumber, char *destNumber);
static RilC2kPhbEncode getCodingFromName(char *alphaid);
static void decode0x81(unsigned char* DesStrP, unsigned char* SrcStrP);
static void decode0x82(unsigned char* DesStrP, unsigned char* SrcStrP);
static int encode0x81(char *src, char *des, int maxLen);
static int encode0x82(char *src, char *des, int maxLen);
static int hexCharToDecInt(char *hex, int length);
static int getPhbStorageType(char* str);
static const char* getPhbStorageString(int type);
static int checkNameLength(char *name);

typedef struct {
    int ready;
} PhbReadyParam;

int rilPhbMain(int request, void *data, size_t datalen, RIL_Token t) {
    switch (request) {
        case RIL_REQUEST_QUERY_PHB_STORAGE_INFO:
            requestGetPhbStorageInfo(data, datalen, t);
            break;
        case RIL_REQUEST_WRITE_PHB_ENTRY:
            requestWritePhbEntry(data, datalen, t);
            break;
        case RIL_REQUEST_READ_PHB_ENTRY:
            requestReadPhbEntry(data, datalen, t);
            break;
        default:
            return 0; /* no matched request */
    }
    return 1;
}

int rilPhbUnsolicited(const char *s, const char *sms_pdu) {
    char *line = NULL;
    int err;
    PHB_UNUSED(sms_pdu);
    if (strStartsWith(s, "+CIEV:")) {
        int ind;
        int val;

        char *dup = strdup(s);

        if (!dup) {
            return 1;
        }

        line = dup;
        err = at_tok_start(&line);
        if (err < 0) {
            free(dup);
            return 1;
        }
        err = at_tok_nextint(&line, &ind);
        if (err < 0) {
            free(dup);
            return 1;
        }
        err = at_tok_nextint(&line, &val);
        if (err < 0) {
            free(dup);
            return 1;
        }

        /* <ind> 101: MD PHB ready/unready */
        if (ind == 101) {
            LOGD("[%s] start s: %s", __FUNCTION__, s);
            PhbReadyParam* mParam = (PhbReadyParam*)malloc(sizeof(PhbReadyParam));
            if (mParam == NULL) {
                free(dup);
                return 1;
            }
            if (1 == val) {
                mParam->ready = 1;
            } else {
                mParam->ready = 0;
            }
            RIL_requestProxyTimedCallback(reportPbSmsReady,
                    mParam, &TIMEVAL_0, SMS_CHANNEL);
            free(dup);
            return 1;
        }
        free(dup);
    }

    return 0;
}

void reportPbSmsReady(void* param) {
    PhbReadyParam* mParam = (PhbReadyParam*)param;
    char * value = (mParam->ready == 1) ? "true" : "false";
    int slotid = 0;
    slotid = getCdmaModemSlot();
    if (slotid > 0 && slotid <=4) {
        setMSimProperty(slotid-1, PROPERTY_C2K_RIL_PHB_READY, value);
    } else {
        LOGE("[%s] unsupport slot id %d", __FUNCTION__, slotid);
    }
    LOGD("[%s] value = %s, slotid = %d", __FUNCTION__, value, slotid);
    if (param > 0) {
        RIL_onUnsolicitedResponse(RIL_UNSOL_PHB_READY_NOTIFICATION, &C2K_PHB_READY,
                sizeof(int));
    } else {
        RIL_onUnsolicitedResponse(RIL_UNSOL_PHB_READY_NOTIFICATION,
                &C2K_PHB_NOT_READY, sizeof(int));
    }
    free(mParam);
}

// Info returned to RILJ
// storageInfo[0]: used phb entry
// storageInfo[1]: total pbh entry
// storageInfo[2]: max number length
// storageInfo[3]: max name length
static void requestGetPhbStorageInfo(void *data, size_t datalen, RIL_Token t) {
    int err = 0;
    int pbSize[4] = {0};
    int storageInfo[4] = {0};
    ATResponse *p_response = NULL;
    char *line = NULL;
    char *storage = NULL;
    int *readInfo = (int*) data;
    int before_type = -1;

    PHB_UNUSED(datalen);

    before_type = getCurrentPhbStorage(t);
    LOGD("[%s] current storage: %d, after: %d", __FUNCTION__,
            before_type, readInfo[0]);

    // Change storage
    if (before_type != readInfo[0]) {
        if (0 == selectPhbStorage(readInfo[0], t)) {
            goto error;
        }
    }

    // Get storage info (max storage size, max number length, max name length)
    // From ^CPBR:(0-249),21,14
    err = getPhbSize(pbSize);
    if (err < 0) goto error;

    LOGD( "[%s] getPhbSize: %d, %d, %d, %d", __FUNCTION__, pbSize[0], pbSize[1],
            pbSize[2], pbSize[3]);
    storageInfo[1] = pbSize[1] - pbSize[0] + 1;
    // max number length value varies in different MD version
    // here we use spec value instead
    storageInfo[2] = CDMA_UIM_NUMBER_LENGTH;
    storageInfo[3] = pbSize[3];

    // Get used/total phb entry info by AT+CPBS?
    err = at_send_command_singleline("AT+CPBS?", "+CPBS:", &p_response, PHB_CHANNEL_CTX);
    if (err < 0 || p_response==NULL || p_response->success == 0) {
        goto error;
    }
    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0) {
        LOGE("[%s] at_tok_start err", __FUNCTION__);
        goto error;
    }

    // read "SM" eg.
    err = at_tok_nextstr(&line, &storage);
    if (err < 0) goto error;
    // read used length
    err = at_tok_nextint(&line, &storageInfo[0]);
    if (err < 0) {
        LOGE("[%s] at_tok_nextint err : number length", __FUNCTION__);
        goto error;
    }
    // read total length
    err = at_tok_nextint(&line, &storageInfo[1]);
    if (err < 0) {
        LOGE("[%s] at_tok_nextint err : text length", __FUNCTION__);
        goto error;
    }

    // restore storage back
    if (before_type != readInfo[0]) {
       if (0 == selectPhbStorage(before_type, t)) {
           goto error;
       }
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, storageInfo, sizeof(storageInfo));
    at_response_free(p_response);
    LOGD("[%s] returns: %d, %d, %d, %d", __FUNCTION__, storageInfo[0], storageInfo[1],
            storageInfo[2], storageInfo[3]);
    // Cache them
    setPhbStorageInfo(storageInfo[1], storageInfo[2], storageInfo[3]);
    return;

error:
    LOGE("[%s] return error = %d", __FUNCTION__, err);
    // restore back
    if (before_type != -1) {
        selectPhbStorage(before_type, t);
    }
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestWritePhbEntry(void *data, size_t datalen, RIL_Token t) {
    int err = RIL_E_GENERIC_FAILURE;
    ATResponse *p_response = NULL;
    char * cmd = NULL;
    int ton = 0;
    int index = 0;
    char *number = NULL;
    char *name = NULL;
    RilC2kPhbEncode coding = PHB_ENCODE_UCS2;
    RIL_PhbEntryStructure *pb = (RIL_PhbEntryStructure*)data;
    unsigned int alphaMaxLength = CDMA_UIM_ALPHAID_LENGTH;
    unsigned int numberMaxLength = CDMA_UIM_NUMBER_LENGTH;
    int alphaMaxBufferSize = 0;
    int before_type = -1;

    PHB_UNUSED(datalen);

    if (pb == NULL) {
        LOGE("[%s] Error data, pb = NULL.", __FUNCTION__);
        goto error;
    }

    // Get current phb storage type
    before_type = getCurrentPhbStorage(t);
    LOGE("[%s] Storage current: %d, request: %d", __FUNCTION__, before_type, pb->type);
    if (pb->type != before_type) {
        if (0 == selectPhbStorage(pb->type, t)) {
            goto error;
        }
    }

    // AT^CPBW=[<index>][,<number>[,<type>[,<text>,<coding>]]]
    index = pb->index - 1; // pb->index start from 1
    if (index < 0) {
        LOGE("[%s] Error: index = %d", __FUNCTION__, index);
        goto error;
    }

    // Number length check
    numberMaxLength = getNumberMaxLength();
    if (pb->number != NULL && strlen(pb->number) > numberMaxLength) {
        err = RIL_E_DIAL_STRING_TOO_LONG;
        LOGE("[%s] Error: number exceed max length: %zu", __FUNCTION__, strlen(pb->number));
        goto error;
    }

    number = (char *)malloc(numberMaxLength * sizeof(char) * 4 + 1);
    assert(number != NULL);
    memset(number, 0, numberMaxLength * sizeof(char) * 4 + 1);
    if (pb->number != NULL && strlen(pb->number) > 0) {
        convertNumberToMd(pb->number, number, pb->ton);
    }

    if (strlen(number) > 0) {
        ton = pb->ton;
    } else {
        ton = 0;
    }

    alphaMaxLength = getNameMaxLength();
    // if using 0x81, need extra 4bytes
    alphaMaxBufferSize = alphaMaxLength * sizeof(char) * 4 + 1 + 4;
    name = (char *)malloc(alphaMaxBufferSize);
    assert(name != NULL);
    memset(name, 0, alphaMaxBufferSize);
    if (pb->alphaId != NULL) {
        coding = convertNameToMd(name, pb->alphaId, alphaMaxBufferSize, &err);
        if (err != RIL_E_SUCCESS) {
            goto error;
        }
    }

    if (0 == ton) {
        asprintf(&cmd, "AT^CPBW=%d,\"%s\",,\"%s\",%d", index, number, name, coding);
    } else {
        asprintf(&cmd, "AT^CPBW=%d,\"%s\",%d,\"%s\",%d", index, number, ton, name, coding);
    }
    err = at_send_command(cmd, &p_response, PHB_CHANNEL_CTX);
    free(cmd);
    if ((err < 0) || (p_response == NULL) || (p_response->success == 0)) {
        err = RIL_E_GENERIC_FAILURE;
        goto error;
    }

    // Restore storage back
    if (before_type != pb->type) {
        if (0 == selectPhbStorage(before_type, t) ) {
            err = RIL_E_GENERIC_FAILURE;
            goto error;
        }
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);

    free(number);
    free(name);
    LOGD("[%s] write done, return!", __FUNCTION__);
    return;

error:
    LOGE("[%s] ERROR", __FUNCTION__);
    if ((pb != NULL) && (before_type != pb->type)) {
        selectPhbStorage(before_type, t);
    }
    if (number != NULL) {
        free(number);
    }
    if (name != NULL) {
        free(name);
    }
    at_response_free(p_response);

    RIL_onRequestComplete(t, err, NULL, 0);
}

static void requestReadPhbEntry(void *data, size_t datalen, RIL_Token t) {
    int err = 0;
    ATResponse *p_response = NULL;
    char * cmd = NULL;
    int bindex = 0;
    int eindex = 0;
    int count = 0;
    int currentindex = 0;
    int i = 0;
    int countValide = 0;
    RIL_Ruim_Phonebook_Record  *o_pb = NULL;
    RIL_PhbEntryStructure  * pb = NULL;
    ATLine *p_cur = NULL;
    unsigned int maxAlphaIdLength = CDMA_UIM_ALPHAID_LENGTH;
    unsigned int maxNumberLength = CDMA_UIM_NUMBER_LENGTH;
    int *readInfo = (int*)data;
    int before_type=-1;

    PHB_UNUSED(datalen);

    before_type = getCurrentPhbStorage(t);
    LOGE("[%s] storage current: %d, request: %d, bIndex: %d, eIndex: %d", __FUNCTION__,
            before_type, readInfo[0], readInfo[1],readInfo[2]);
    if (readInfo[0] != before_type) {
        if (0 == selectPhbStorage(readInfo[0], t)) {
            goto error;
        }
    }

    // Index start from 0 for CPBR
    bindex = readInfo[1] - 1;
    eindex = readInfo[2] - 1;

    if (bindex < 0) {
        goto error;
    }

    count = eindex - bindex + 1;
    if (count < 0) {
        goto error;
    }

    // alloc memory from stack, no need free
    o_pb = (RIL_Ruim_Phonebook_Record *)alloca(count * sizeof(RIL_Ruim_Phonebook_Record));
    assert(o_pb != NULL);
    memset (o_pb, 0, count * sizeof(RIL_Ruim_Phonebook_Record));

    // Ready phb entry from MD one by one from start index to end index
    countValide = 0;
    for (currentindex = bindex; currentindex <= eindex; currentindex++) {
        asprintf(&cmd, "AT^CPBR=%d", currentindex);
        err = at_send_command_multiline(cmd, "^CPBR:", &p_response, PHB_CHANNEL_CTX);
        free(cmd);
        cmd = NULL;

        if (err != 0 || p_response->success == 0) {
            goto error;
        }
        p_cur = p_response->p_intermediates;
        if (p_cur != NULL) {
            err = parsePhb(p_cur->line, o_pb + countValide);
            if (err != 0) {
                continue;
            }
            countValide++;
        } else {
            continue;
        }
    }

    // Convert phb entry to RILJ format
    pb = (RIL_PhbEntryStructure *)alloca(countValide * sizeof(RIL_PhbEntryStructure));
    assert(pb != NULL);
    memset (pb, 0, countValide * sizeof(RIL_PhbEntryStructure));
    maxAlphaIdLength = getNameMaxLength();
    maxNumberLength = getNumberMaxLength();
    for (i = 0; i < countValide ; i++) {
        pb[i].type = 0;
        pb[i].index = o_pb[i].index + 1;
        pb[i].number = (char *)malloc(maxNumberLength * 4 + 1);
        assert(pb[i].number != NULL);
        if (NULL == pb[i].number) {
            LOGD("[%s] Failed to malloc pb[%d].number", __FUNCTION__, i);
            goto error;
        }
        memset(pb[i].number, 0, maxNumberLength * 4 + 1);
        convertNumberFromMd(o_pb[i].number, pb[i].number);

        pb[i].ton = o_pb[i].type;
        pb[i].alphaId = (char *)malloc(maxAlphaIdLength * 4 + 1);
        assert(pb[i].alphaId != NULL);
        if (NULL == pb[i].alphaId) {
            LOGD("[%s] Failed to malloc pb[%d].alphaId", __FUNCTION__, i);
            goto error;
        }
        memset(pb[i].alphaId, 0, maxAlphaIdLength * 4 + 1);
        convertNameFromMd(pb[i].alphaId, o_pb[i].name);
        if ((strlen(pb[i].alphaId)) % 4 != 0) {
            LOGE("[%s] convertNameFromMd error! after convert strlen = %zu",
                __FUNCTION__, strlen(pb[i].alphaId));
            memset(pb[i].alphaId, 0, maxAlphaIdLength * 4 + 1);
        }
    }

    if (readInfo[0] != before_type) {
        if (0 == selectPhbStorage(before_type, t)) {
            goto error;
        }
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, pb, countValide * sizeof (RIL_PhbEntryStructure));

    at_response_free(p_response);
    for (i = 0; i < countValide ; i++) {
        if (pb != NULL) {
            if (pb[i].number != NULL) free(pb[i].number);
            if (pb[i].alphaId != NULL) free(pb[i].alphaId);
        }
    }
    LOGD("[%s] Read PHB success, count = %d", __FUNCTION__, countValide);
    return;

error:
    LOGE("[%s] ERROR!", __FUNCTION__);
    if (before_type != -1) {
        selectPhbStorage(before_type, t);
    }
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    for (i = 0; i < countValide ; i++) {
        if (pb != NULL) {
            if (pb[i].number != NULL) free(pb[i].number);
            if (pb[i].alphaId != NULL) free(pb[i].alphaId);
        }
    }
}

// Get current PHB storage type from MD by "AT+CPBS?"
static int getCurrentPhbStorage(RIL_Token t) {
    int err = 0;
    int type = 0;
    ATResponse *p_response_storage = NULL;
    char *current_storage = NULL;
    char *line = NULL;

    // Get current storage type
    err = at_send_command_singleline("AT+CPBS?", "+CPBS:",
            &p_response_storage, PHB_CHANNEL_CTX);

    if (err < 0 || p_response_storage == NULL || p_response_storage->success == 0) {
        goto error;
    }
    line = p_response_storage->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0) {
        LOGE("[%s] at_tok_start err", __FUNCTION__);
        goto error;
    }

    err = at_tok_nextstr(&line, &current_storage);
    if (err < 0) goto error;
    type = getPhbStorageType(current_storage);
    at_response_free(p_response_storage);
    return type;
error:
    LOGE("[%s] return error = %d", __FUNCTION__, err);
    at_response_free(p_response_storage);
    return type;
}

// Change MD PHB storage by AT+CPBS=\"%s\"
// Sync with GSM PHB for the return value
// Returns:
//   0: fail
//   1: success
static int selectPhbStorage(int type, RIL_Token t) {
    ATResponse *p_response = NULL;
    const char *storage;
    char *cmd = NULL;
    int err, result = 1;

    storage = getPhbStorageString(type);
    if (storage == NULL) {
        result = 0;
    }

    asprintf(&cmd, "AT+CPBS=\"%s\"", storage);
    if (cmd == NULL) {
        result = 0;
        LOGD("[%s] error result: %d", __FUNCTION__, result);
        return result;
    }
    err = at_send_command(cmd, &p_response, PHB_CHANNEL_CTX);

    if (err < 0 || p_response == NULL || p_response->success == 0) {
       result = 0;
    }
    free(cmd);
    at_response_free(p_response);

    LOGD("[%s] result: %d", __FUNCTION__, result);
    return result;
}

// Cache storage info get from MD
static void setPhbStorageInfo(int max, int numberMax, int alphaMax) {
    s_PHB_max_capacity = max;
    s_PHB_number_max_bytes = numberMax;
    s_PHB_alphaid_max_bytes = alphaMax;
}

static void convertNameFromMd(char *dest_alphaid, char *src_alphaid) {
    int i = 0;
    int len = 0;
    int lenByte = 0;
    unsigned char alphaidBytes[PHB_NAME_MAX_SIZE] = {0};
    unsigned char alphaidBytesAfterDecode[PHB_NAME_MAX_SIZE] = {0};
    char resstr[3] = {0};
    char * temp = NULL;

    if (src_alphaid == NULL || dest_alphaid == NULL) {
        return ;
    }

    len = strlen(src_alphaid);
    if (len <= 0) {
        return;
    }

    if (src_alphaid[0] == '8' && src_alphaid[1] == '0') {
        // 0x80 format: 80xxxxxx, ex: 8096b6b6b6b6b6ff
        for (i = 2; i < len; i++) {
            dest_alphaid[i - 2] = src_alphaid[i];
        }
        LOGD("[%s] 0x80 encoding, copyed data count = %d", __FUNCTION__, i);
    } else if (len > 4 && src_alphaid[0] == '8' && src_alphaid[1] == '1') {
        // 0x81 format: 81[len][base]xxxxxx, ex: 81060896b6b6b6b6b6ff
        lenByte = hexCharToDecInt((char *)&src_alphaid[2], LEN_BYTES_0X81);
        if ((len - 4) < (lenByte + 1) * 2) {
            LOGE("[%s] 0x81 error string, len = %d ,lenByte: %d",
                    __FUNCTION__, len, lenByte);
            return;
        }
        alphaidBytes[0] = lenByte;
        temp = src_alphaid + 4;
        for (i = 0; i < lenByte + 1; i++) {
            alphaidBytes[i+1] = hexCharToDecInt(temp, 1);
            temp++;
            alphaidBytes[i+1] = alphaidBytes[i+1] << 4 | hexCharToDecInt(temp, 1);
            temp++;
        }
        decode0x81(alphaidBytesAfterDecode, alphaidBytes);
        for (i = 0; i < lenByte * 2; i++) {
            memset(resstr, 0, 3);
            if (alphaidBytesAfterDecode[i] == 0) {
                resstr[0] = resstr[1] = '0';
            } else {
                sprintf(resstr,"%02x", alphaidBytesAfterDecode[i]);
            }
            strncpy(dest_alphaid + (2 * i), resstr, 2);
        }
        if (isUserLoad() == 0) {
            LOGD("[%s] 0x81 decode dest_alphaid: %s ", __FUNCTION__, dest_alphaid);
        }
    } else if (len > 4 && src_alphaid[0] == '8' && src_alphaid[1] == '2') {
        lenByte = hexCharToDecInt((char *)&src_alphaid[2], LEN_BYTES_0X81);
        if ((len - 4) < (lenByte + 2) * 2) {
            LOGE("[%s]0x82 error string, len = %d ,lenByte: %d", __FUNCTION__, len, lenByte);
            return;
        }
        alphaidBytes[0] = lenByte;
        temp = src_alphaid + 4;
        for (i = 0; i < lenByte + 2; i++) {
            alphaidBytes[i+1] = hexCharToDecInt(temp, 1);
            temp++;
            alphaidBytes[i+1] = alphaidBytes[i+1] << 4 | hexCharToDecInt(temp, 1);
            temp++;
        }
        decode0x82(alphaidBytesAfterDecode, alphaidBytes);
        for (i = 0; i < lenByte*2; i++) {
            memset(resstr, 0, 3);
            if (alphaidBytesAfterDecode[i] == 0) {
                resstr[0] = resstr[1] = '0';
            } else {
                sprintf(resstr,"%02x", alphaidBytesAfterDecode[i]);
            }
            strncpy(dest_alphaid+(2*i), resstr, 2);
        }
        if (isUserLoad() == 0) {
            LOGD("[%s]0x82 dest_alphaid: %s ", __FUNCTION__, dest_alphaid);
        }
    } else {
        len = strlen(src_alphaid);

        // MD3 alpha id max length may not sync with return value of CPBR
        // Add length check to prevent memory overflow
        if (len > CDMA_UIM_ALPHAID_LENGTH) {
            LOGE("[%s] ASCII len = %d", __FUNCTION__, len);
            len = CDMA_UIM_ALPHAID_LENGTH;
        }

        for (i = 0; i < len; i++) {
            memset(resstr, 0, 3);
            dest_alphaid[4 * i] = dest_alphaid[4 * i + 1] = '0';
            sprintf(resstr,"%0x", src_alphaid[i]);
            strncpy(dest_alphaid+(4*i+2), resstr, 2);
        }
        if (isUserLoad() == 0) {
            LOGD("[%s] ASCII dest_alphaid: %s", __FUNCTION__, dest_alphaid);
        }
    }

}

static RilC2kPhbEncode convertNameToMd(char * dest_alphaid,
        char * src_alphaid, int size, int *err) {
    int i = 0;
    int j = 0;
    int len = 0;
    RilC2kPhbEncode encoding = PHB_ENCODE_UCS2;

    if (dest_alphaid == NULL || src_alphaid == NULL) {
        if (isUserLoad() == 0) {
            LOGD("[%s] Error! dest_alphaid: %s, src_alphaid: %s", __FUNCTION__,
                    dest_alphaid, src_alphaid);
        }
        *err = RIL_E_GENERIC_FAILURE;
        return encoding;
    }

    encoding = getCodingFromName(src_alphaid);

    if (encoding == PHB_ENCODE_UCS2) {
        // try using 0x81 encoding
        if (encode0x81(src_alphaid, dest_alphaid, size)) {
            encoding = PHB_ENCODE_UCS2_81;
            LOGD("[%s] 0x81 encoding", __FUNCTION__);
        } else {
            dest_alphaid[0] = '8';
            dest_alphaid[1] = '0';
            len = strlen(src_alphaid);
            for (i = 0; i < len; i++) {
                dest_alphaid[i + 2] = src_alphaid[i];
            }
        }
    } else {
        len = (int)(strlen(src_alphaid) / 4);
        for (j = 0; j < len; j++) {
            i = j * 4 + 2;
            dest_alphaid[j] = hexCharToDecInt((char *)&src_alphaid[i], UNICODE_CHAR_LEN);
        }
    }

    if (isUserLoad() == 0) {
        LOGD("[%s] encoding: %d, alphaid: %s", __FUNCTION__, encoding, dest_alphaid);
    }
    *err = checkNameLength(dest_alphaid);
    return encoding;
}

static void convertNumberToMd(char *srcNumber, char *destNumber, int ton) {
    int i = 0;
    int j = 0;
    int len = 0;

    if (srcNumber == NULL || destNumber == NULL) {
        return;
    }

    if (ton == 0x91) {
        destNumber[j] = '+';
        j++;
    }

    len = strlen(srcNumber);
    for(; i < len; i++) {
        switch (srcNumber[i]) {
            case 'P':
            case 'p':
                destNumber[j++] = 'P';
                break;
            case 'W':
            case 'w':
                destNumber[j++] = 'T';
                break;
            default:
                destNumber[j++] = srcNumber[i];
        }
    }
}

static void convertNumberFromMd(char *srcNumber, char *destNumber) {
    int i = 0;
    int j = 0;
    int len = 0;

    if (srcNumber == NULL || destNumber == NULL) {
        return ;
    }

    len = strlen(srcNumber);
    if (len <= 0) {
        return;
    }

    if (srcNumber[i] == '+') {
        i++;
    }

    for(; i < len; i++) {
        switch (srcNumber[i]) {
            case 'P':
                destNumber[j++] = 'p'; // framework just know lower case 'p'
                break;
            case 'T':
            case 't':
                destNumber[j++] = 'w'; // framework just know lower case 'w'
                break;
            default:
                destNumber[j++] = srcNumber[i];
        }
    }
}

static RilC2kPhbEncode getCodingFromName(char *alphaid) {
    int i = 0;
    int countUS2 = 0;
    int len = strlen(alphaid);

    if (len % 4 != 0 || len == 0) {
        return PHB_ENCODE_ASCII;
    }

    countUS2 = len / 4;

    for (i = 0; i < countUS2; i++) {
        // return UCS2 encoding for non ASCII(>0x7F) chars
        if (alphaid[i * 4] != '0' || alphaid[i * 4 + 1] != '0' ||
            hexCharToDecInt((char *)&alphaid[i * 4 + 2], UNICODE_CHAR_LEN) > 0x7F) {
            return PHB_ENCODE_UCS2;
        }
    }
    return PHB_ENCODE_ASCII;
}

static void decode0x81(unsigned char* DesStrP, unsigned char* SrcStrP) {
    unsigned char i;
    unsigned char length;
    int basePoint;
    int tempValue;
    char*  charPtr = (char*)DesStrP;

    length = SrcStrP[0];
    basePoint = SrcStrP[1] << 7;
    for (i = 2; i < length + 2; i++) {
        if (SrcStrP[i] < 0x80) {
            *charPtr++ = 0;
            *charPtr++ = SrcStrP[i];
        } else {
            tempValue = basePoint + (SrcStrP[i] & 0x7f);
            *charPtr++ = (tempValue & 0xff00) >> 8;
            *charPtr++ = tempValue & 0x00ff;
        }
    }
    LOGD("[%s] src: %s, dest: %s", __FUNCTION__, SrcStrP, DesStrP);
}

static void decode0x82(unsigned char* DesStrP, unsigned char* SrcStrP) {
    unsigned char  i;
    int length;
    int basePoint;
    int tempValue;
    char*  charPtr = (char*)DesStrP;

    length = SrcStrP[0];
    basePoint = (SrcStrP[1] << 8) + SrcStrP[2];
    for (i = 3; i < length + 3; i++) {
        if(SrcStrP[i] < 0x80) {
            *charPtr++ = 0;
            *charPtr++ = SrcStrP[i];
        } else {
            tempValue = basePoint + (SrcStrP[i] & 0x7f);
            *charPtr++ = (tempValue & 0xff00) >> 8;
            *charPtr++ = tempValue & 0x00ff;
        }
    }
}

static int encode0x81(char *src, char *des, int maxLen) {
    int i, j, k, len = 0;
    unsigned int base = 0xFF000000;
    unsigned short tmpAlphaId[CDMA_UIM_ALPHAID_LENGTH + 3 + 1];

    len = strlen(src);
    for (i = 0, j = 0; i < len; i += 4, j++) {
        tmpAlphaId[j] = (unsigned short) hexCharToDecInt(src + i, 4);
    }
    tmpAlphaId[j] = '\0';
    len = j;

    LOGD("[%s] len: %d, maxLen: %d", __FUNCTION__, len, maxLen);

    if (len <= 3) {
        // at least 3 characters
        return 0;
    }

    if (((len + 3) * 2 + 1) > maxLen) {
        // the destinaiton buffer is not enough(include '\0')
        return 0;
    }

    for (i = 0; i < len; i++) {
        int needSpecialEncoding = 0;

        if (tmpAlphaId[i] & 0x8000) // Because base pointer of 0x81 is 0hhh hhhh h000 0000
            return 0;

        for (k = 0; k < SPEC_ENCODING_ARR_LEN; k++) {
            if (tmpAlphaId[i] == specialEncoding[k][0]) {
                LOGD("[%s] found: i: %d, tmpAlphaId: %d", __FUNCTION__, i, tmpAlphaId[i]);
                tmpAlphaId[i] = specialEncoding[k][1];
                needSpecialEncoding = 1;
                break;
            }
        }

        if (needSpecialEncoding != 1) {
            if (tmpAlphaId[i] < 0x80) {
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

            if (base == 0xFF000000) {
                base = tmpAlphaId[i] & RIL_PHB_UCS2_81_MASK;
            }
            tmpAlphaId[i] ^= base;
            if ( tmpAlphaId[i] >= 0x80) {
                break;
            }
            tmpAlphaId[i] |= 0x80;
        }
    }

    if (i != len) {
        return 0;
    }

    // Make up UCS2 0x81 String
    sprintf(des, "81%02X%02X", len, base>>7);
    int realLen = 0;
    char* pLen = des;
    des += 6;

    for (i = 0; i < len; i++, des += 2) {
        if ((tmpAlphaId[i] & 0xFF00) != 0x1B00) {
            sprintf(des, "%02X", tmpAlphaId[i]);
        } else{
            sprintf(des, "%04X", tmpAlphaId[i]);
            des += 2;
            realLen++;
        }
    }

    realLen += len;
    if (realLen > len) {
        *(pLen + 2) = (char)((realLen/16) > 9 ? ((realLen/16) - 10 + 'A') : (realLen/16) + '0');
        *(pLen + 3) = (char)((realLen%16) > 9 ? ((realLen%16) - 10 + 'A') : (realLen%16) + '0');
        LOGD("[%s] set len: %d to realLen: %d", __FUNCTION__, len, realLen);
    }
    *des = '\0';

    return realLen;
}

static int encode0x82(char *src, char *des, int maxLen) {
    PHB_UNUSED(src);
    PHB_UNUSED(des);
    PHB_UNUSED(maxLen);
    // TODO:
    return 0;
}

static unsigned int getNameMaxLength() {
    if (s_PHB_alphaid_max_bytes == 0) {
        int err = 0;
        int pbSize[4] = {0};
        err = getPhbSize(pbSize);
        if (err < 0) {
            return s_PHB_alphaid_max_bytes;
        }
        s_PHB_alphaid_max_bytes = pbSize[3];
    }
    return (unsigned int) s_PHB_alphaid_max_bytes;
}

static unsigned int getNumberMaxLength() {
    // max number length value varies in different MD version
    // here we use spec value instead
    s_PHB_number_max_bytes = CDMA_UIM_NUMBER_LENGTH;
    return s_PHB_number_max_bytes;
}

static int parsePhb(char *line, RIL_Ruim_Phonebook_Record *readPb)
{
    // set default value
    memset(readPb, 0, sizeof(RIL_Ruim_Phonebook_Record));
    readPb->index = -1;
    readPb->coding = 1;

    int err = -1;
    if (!strStartsWith(line, "^CPBR:")) {
        LOGE("[%s] it not start with ^CPBR", __FUNCTION__);
        goto error;
    }
    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &(readPb->index));
    if (err < 0) goto error;

    err = at_tok_nextstr(&line, &(readPb->number));
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &(readPb->type));
    if (err < 0) goto error;

    err = at_tok_nextstr(&line, &(readPb->name));
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &(readPb->coding));
    if (err < 0) goto error;

    return 0;

error:
    LOGE("[%s] parsing ^CPBR error", __FUNCTION__);
    return err;
}

// Get ^CPBR:(0-249),21,14 info
static int getPhbSize(int pbSize[])
{
    int err = 0;
    int ret = 0;
    ATResponse *p_response = NULL;

    char *line = NULL;
    char *storage = NULL;

    /*
        AT^CPBR=?
        ^CPBR:(0-249),21,14
        OK
    */
    err = at_send_command_singleline("AT^CPBR=?", "^CPBR:", &p_response, getChannelCtxbyProxy());

    if ((err < 0) || (p_response == NULL) || (p_response->success == 0)) {
        ret = -1;
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);

    if (err < 0) {
        LOGE("[%s] at_tok_start err", __FUNCTION__);
        ret = -1;
        goto error;
    }

    // read "(0-249)" eg.
    err = at_tok_nextstr(&line, &storage);
    if (err < 0) {
        ret = -1;
        goto error;
    }

    // read begin, end index
    err = sscanf(storage, "(%d-%d)", &pbSize[0], &pbSize[1]);
    if (err == EOF) {
        ret = -1;
        goto error;
    }

    // read number length
    err = at_tok_nextint(&line, &pbSize[2]);
    if (err < 0) {
        LOGE("[%s] at_tok_nextint err : number length", __FUNCTION__);
        ret = -1;
        goto error;
    }

    // read text length
    err = at_tok_nextint(&line, &pbSize[3]);
    if (err < 0) {
        LOGE("[%s] at_tok_nextint err : text length", __FUNCTION__);
        ret = -1;
        goto error;
    }

error:
    at_response_free(p_response);
    return ret;
}

static int hexCharToDecInt(char *hex, int length) {
    int i = 0;
    int value, digit;

    for (i = 0, value = 0; i < length && hex[i] != '\0'; i++) {
        if (hex[i] >= '0' && hex[i] <= '9') {
            digit = hex[i] - '0';
        } else if ( hex[i] >= 'A' && hex[i] <= 'F') {
            digit = hex[i] - 'A' + 10;
        } else if ( hex[i] >= 'a' && hex[i] <= 'f') {
            digit = hex[i] - 'a' + 10;
        } else {
            return -1;
        }
        value = value * 16 + digit;
    }

    return value;
}

static const char* getPhbStorageString(int type) {
    char* str = NULL;
    switch(type) {
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

static int getPhbStorageType(char* str) {
    int type = -1;
    LOGD("[%s] str: %s", __FUNCTION__, str);
    if (strcmp("SM", str) == 0) {
       type=RIL_PHB_ADN;
    } else if (strcmp("FD", str) == 0) {
       type=RIL_PHB_FDN;
    } else if (strcmp("ON", str) == 0) {
       type=RIL_PHB_MSISDN;
    } else if (strcmp("EN", str) == 0) {
       type=RIL_PHB_ECC;
    }
    return type;
}

static int checkNameLength(char *name) {
    unsigned int nameMax = CDMA_UIM_ALPHAID_LENGTH;

    if (name == NULL) {
        return RIL_E_GENERIC_FAILURE;
    }

    nameMax = getNameMaxLength();

    if (name != NULL) {
        if (name[0] == '8' && name[1] == '0') {
            if ((strlen(name) - 2)/4 > (nameMax/2 - 1) ) {
                return RIL_E_TEXT_STRING_TOO_LONG;
            }
        } else if (name[0] == '8' && name[1] == '1') {
            // 0x81 max len = MAX - 3
            if ((strlen(name) - 6) / 2 > nameMax - 3 ) {
                return RIL_E_TEXT_STRING_TOO_LONG;
            }
        } else if (name[0] == '8' && name[1] == '2') {
            // Not support currently
        } else {
            if(strlen(name) > nameMax) {
                return RIL_E_TEXT_STRING_TOO_LONG;
            }
        }
    }
    return RIL_E_SUCCESS;
}
