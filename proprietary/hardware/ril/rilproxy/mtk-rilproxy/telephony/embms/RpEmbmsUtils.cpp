/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2015. All rights reserved.
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

/*****************************************************************************
 * Include
 *****************************************************************************/

#include "RpEmbmsUtils.h"
#include <telephony/mtk_ril.h>
#include <cutils/jstring.h>
#include <cutils/properties.h>
#include <string.h>
#include <stdlib.h>
#include "RfxRootController.h"
#define RFX_LOG_TAG "RP_EMBMS_UTIL"

/*****************************************************************************
 * Class RpEmbmsUtils
 *****************************************************************************/

RpEmbmsUtils::RpEmbmsUtils() {
}

RpEmbmsUtils::~RpEmbmsUtils() {
}

RpEmbmsSessionInfo* RpEmbmsUtils::findSessionByTransId(
                    Vector<RpEmbmsSessionInfo*>* list, int trans_id, int* pIndex) {
    RpEmbmsSessionInfo* pSessionInfo = NULL;
    *pIndex = -1;

    if (list != NULL) {
        int size = list->size();
        for (int i = 0; i < size; i++) {
            pSessionInfo = list->itemAt(i);
            if (pSessionInfo->mTransId == trans_id) {
                RFX_LOG_D(RFX_LOG_TAG, "Find trans_id:%d", pSessionInfo->mTransId);
                *pIndex = i;
                break;
            }
        }
    }
    if (*pIndex == -1) {
        pSessionInfo = NULL;
    }
    return pSessionInfo;
}

RpEmbmsSessionInfo* RpEmbmsUtils::findSessionByTmgi(
                    Vector<RpEmbmsSessionInfo*>* list, int tmgi_len, char* pTmgi, int* pIndex) {
    RpEmbmsSessionInfo* pSessionInfo = NULL;
    *pIndex = -1;

    if (list != NULL) {
        int size = list->size();
        for (int i = 0; i < size; i++) {
            pSessionInfo = list->itemAt(i);
            RFX_LOG_D(RFX_LOG_TAG, "tmgi[%d]:[%s],len[%d]", i, pSessionInfo->mTmgi,
                pSessionInfo->mTmgiLen);
            if (pSessionInfo->mTmgiLen == tmgi_len) {
                if (strcmp(pSessionInfo->mTmgi, pTmgi) == 0) {
                    *pIndex = i;
                    RFX_LOG_D(RFX_LOG_TAG, "find tmgi[%d]:%s", i, pSessionInfo->mTmgi);
                    break;
                }
            }
        }
    }
    if (*pIndex == -1) {
        pSessionInfo = NULL;
    }
    return pSessionInfo;
}

void RpEmbmsUtils::freeSessionList(Vector<RpEmbmsSessionInfo*>* list) {
    if (list != NULL) {
        int size = list->size();
        for (int i = 0; i < size; i++) {
            delete list->itemAt(i);
        }
        list->clear();
    }
}

char *RpEmbmsUtils::strdupReadString(Parcel *p) {
    size_t stringlen;
    const char16_t *s16;

    s16 = p->readString16Inplace(&stringlen);

    return strndup16to8(s16, stringlen);
}

void RpEmbmsUtils::writeStringToParcel(Parcel &p, const char *s) {
    char16_t *s16 = NULL;
    size_t s16_len = 0;
    s16 = strdup8to16(s, &s16_len);
    p.writeString16(s16, s16_len);
    free(s16);
}

int RpEmbmsUtils::getDefaultDataSlotId() {
    RfxRootController *root = RFX_OBJ_GET_INSTANCE(RfxRootController);
    // Default get slot 0 StatusManager.
    int data = root->getNonSlotScopeStatusManager()->getIntValue(RFX_STATUS_KEY_DEFAULT_DATA_SIM);
    RFX_LOG_D(RFX_LOG_TAG, "getDefaultDataSlotId: %d", data);
    return data;
}

bool RpEmbmsUtils::revertTmgi(const uint8_t* input, char * output, int length) {
    int i = 0;
    char tmp_char;
    RFX_LOG_D(RFX_LOG_TAG, "start revertTmgi");

    if (input == NULL || length !=  EMBMS_MAX_BYTES_TMGI) {
        printf("revertTmgi error: input null or wrong len\n");
        return false;
    }
    sprintf(output, "%02X%02X%02X%02X%02X%02X",
        input[0], input[1], input[2],
        input[3], input[4], input[5]);
    RFX_LOG_D(RFX_LOG_TAG, "revertTmgi from %s", output);

    // digit[0~5] are service id
    // digit[6~11] are: mcc2, mcc1, mnc3(might be F), mcc3, mnc2, mnc1
    // swap digit 6&7
    tmp_char = output[6];
    output[6] = output[7];
    output[7] = tmp_char;
    // swap digit 8&9
    tmp_char = output[8];
    output[8] = output[9];
    output[9] = tmp_char;
    // swap digit 9&11
    tmp_char = output[9];
    output[9] = output[11];
    output[11] = tmp_char;
    // check if digit 11 is F
    if (output[11] == 'F' || output[11] == 'f') {
        output[11] = '\0';
    }

    RFX_LOG_D(RFX_LOG_TAG, "revertTmgi to [%s]", output);
    return true;
}

bool RpEmbmsUtils::convertTmgi(const char * input, uint8_t* output) {
    int inputLen = 0, i = 0;
    char tmpTmgiStr[EMBMS_MAX_LEN_TMGI+1];
    uint8_t tmpValue = 0, tmpValue2;
    RFX_LOG_D(RFX_LOG_TAG, "start convertTmgi");
    memset(tmpTmgiStr, 0, sizeof(tmpTmgiStr));

    if (input == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "convertTmgi error: input null");
        return false;
    }

    inputLen = strlen(input);

    if (inputLen !=  EMBMS_MAX_LEN_TMGI && inputLen != (EMBMS_MAX_LEN_TMGI - 1)) {
        RFX_LOG_E(RFX_LOG_TAG,"convertTmgi error: wrong len");
        return false;
    } else if (inputLen == EMBMS_MAX_LEN_TMGI - 1) {
        tmpTmgiStr[EMBMS_MAX_LEN_TMGI - 1] = 'F';
    }
    strncpy(tmpTmgiStr, input, inputLen);

    // Swap MCC1 & MCC2
    tmpValue = tmpTmgiStr[6];
    tmpTmgiStr[6] = tmpTmgiStr[7];
    tmpTmgiStr[7] = tmpValue;

    // Swap MCC3 & MNC3
    tmpValue = tmpTmgiStr[8];
    tmpValue2 = tmpTmgiStr[9];
    tmpTmgiStr[8] = tmpTmgiStr[11];
    tmpTmgiStr[9] = tmpValue;
    tmpTmgiStr[11] = tmpValue2;

    for (i = 0; i < EMBMS_MAX_BYTES_TMGI; i++) {
        sscanf(&tmpTmgiStr[2 * i], "%2hhx", output + i);
    }
    RFX_LOG_D(RFX_LOG_TAG, "convertTmgi from [%s] to [%02X%02X%02X%02X%02X%02X]",
        input, output[0]
        , output[1], output[2]
        , output[3], output[4]
        , output[5]);
    return true;
}

bool RpEmbmsUtils::isEmbmsSupport() {
    bool ret = false;
    char prop[PROPERTY_VALUE_MAX] = {0};

    property_get("persist.vendor.radio.embms.support", prop, "-1");
    if (!strcmp(prop, "1")) {
        return true;
    } else if (!strcmp(prop, "0")) {
        return false;
    }

    property_get("ro.vendor.mtk_embms_support", prop, "0");
    if (!strcmp(prop, "1")) {
        ret = true;
    }

    return ret;
}

bool RpEmbmsUtils::isAtCmdEnableSupport() {
    bool ret = false;
    char prop[PROPERTY_VALUE_MAX] = {0};

    // for RJIL old middleware version
    if (RpEmbmsUtils::isRjilSupport()) {
        return false;
    }

    property_get("persist.vendor.radio.embms.atenable", prop, "1");
    if (!strcmp(prop, "1")) {
        return true;
    } else if (!strcmp(prop, "0")) {
        return false;
    }

    return ret;
}

// for RJIL old middleware version with spec v1.8
bool RpEmbmsUtils::isRjilSupport() {
    // On android N all use latest Middleware, for RJIL also support requirement >= 1.9
    return false;
}

bool RpEmbmsUtils::isDualLteSupport() {
    // On android N all use latest Middleware, for RJIL also support requirement >= 1.9
    bool ret = false;
    char prop[PROPERTY_VALUE_MAX] = {0};

    property_get("persist.vendor.radio.mtk_ps2_rat", prop, "G");
    RFX_LOG_E(RFX_LOG_TAG,"isDualLteSupport:%s", prop);
    if (strchr(prop, 'L') != NULL) {
        ret = true;
    }
    return ret;
}

char* RpEmbmsUtils::addLogMask(int input) {
    char* out = NULL;
    int len = asprintf(&out, "%X", input);
    if (len == 1) {
        out[0] = '*';
        return out;
    }
    for (int i = 0; i < (len/2); ++i) {
        out[i] = '*';
    }
    return out;
}
