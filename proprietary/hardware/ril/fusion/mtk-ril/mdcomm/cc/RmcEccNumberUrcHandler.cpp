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

#include "RmcEccNumberUrcHandler.h"
#include "rfx_properties.h"
#include "RfxStringData.h"
#include <string.h>

#define RFX_LOG_TAG "RmcEccNumberUrcHandler"

#define MAX_PROP_CHARS   50

static const char PROPERTY_NW_ECC_LIST[MAX_SIM_COUNT][MAX_PROP_CHARS] = {
    "vendor.ril.ecc.service.category.list",
    "vendor.ril.ecc.service.category.list.1",
    "vendor.ril.ecc.service.category.list.2",
    "vendor.ril.ecc.service.category.list.3",
};

static const char PROPERTY_NW_ECC_MCC[MAX_SIM_COUNT][MAX_PROP_CHARS] = {
    "vendor.ril.ecc.service.category.mcc",
    "vendor.ril.ecc.service.category.mcc.1",
    "vendor.ril.ecc.service.category.mcc.2",
    "vendor.ril.ecc.service.category.mcc.3",
};

/*****************************************************************************
 * Class RmcEccNumberUrcHandler
 *****************************************************************************/


RFX_IMPLEMENT_HANDLER_CLASS(RmcEccNumberUrcHandler, RIL_CMD_PROXY_URC);

// register data
RFX_REGISTER_DATA_TO_URC_ID(RfxStringData, RFX_MSG_URC_CC_GSM_SIM_ECC);
RFX_REGISTER_DATA_TO_URC_ID(RfxStringData, RFX_MSG_URC_CC_C2K_SIM_ECC);
RFX_REGISTER_DATA_TO_URC_ID(RfxIntsData, RFX_MSG_URC_CC_ECC_NUMBER_TEST);

RmcEccNumberUrcHandler::RmcEccNumberUrcHandler(int slot_id, int channel_id) :
        RfxBaseHandler(slot_id, channel_id),
        mNeedRefreshNwEcc(false) {
    const char* urc[] = {
        "+ESIMECC",
        "+CECC",
        "+CEN1",
        "+CEN2",
        "+EECCTEST"
    };

    registerToHandleURC(urc, sizeof(urc)/sizeof(char *));
}

RmcEccNumberUrcHandler::~RmcEccNumberUrcHandler() {
}

void RmcEccNumberUrcHandler::onHandleUrc(const sp<RfxMclMessage>& msg) {
    String8 ss(msg->getRawUrc()->getLine());

    logV(RFX_LOG_TAG, "[%s] URC: %s", __FUNCTION__, ss.string());
    if (ss.find("+ESIMECC") == 0) {
        handleGsmSimEcc(msg);
    } else if (ss.find("+CECC") == 0) {
        handleC2kSimEcc(msg);
    } else if (ss.find("+CEN1") == 0 || ss.find("+CEN2") == 0) {
        handleNetworkEcc(msg);
    } else if (ss.find("+EECCTEST") == 0) {
        handleTestCommand(msg);
    }
}

void RmcEccNumberUrcHandler::handleGsmSimEcc(const sp<RfxMclMessage>& msg) {
    sp<RfxMclMessage> urc = RfxMclMessage::obtainUrc(
        RFX_MSG_URC_CC_GSM_SIM_ECC,
        m_slot_id,
        RfxStringData(msg->getRawUrc()->getLine()));
    responseToTelCore(urc);
}

void RmcEccNumberUrcHandler::handleC2kSimEcc(const sp<RfxMclMessage>& msg) {
    sp<RfxMclMessage> urc = RfxMclMessage::obtainUrc(
        RFX_MSG_URC_CC_C2K_SIM_ECC,
        m_slot_id,
        RfxStringData(msg->getRawUrc()->getLine()));
    responseToTelCore(urc);
}

void RmcEccNumberUrcHandler::handleNetworkEcc(const sp<RfxMclMessage>& msg) {
    String8 ss(msg->getRawUrc()->getLine());
    int err = 0;
    RfxAtLine* line = msg->getRawUrc();
    sp<RfxMclMessage> urc = NULL;

    if (line == NULL) {
        logE(RFX_LOG_TAG, "[%s] NULL line in URC", __FUNCTION__);
        return;
    }

    line->atTokStart(&err);
    if (err < 0) goto error;
    if (ss.find("+CEN1") == 0) {
        // Handle +CEN1 URC
        // Example: +CEN1:1,460,011
        int reporting = line->atTokNextint(&err);
        if (err < 0) goto error;
        char* mcc = line->atTokNextstr(&err);
        if (err < 0) goto error;

        char oldMcc[RFX_PROPERTY_VALUE_MAX] = {0};
        rfx_property_get(PROPERTY_NW_ECC_MCC[m_slot_id], oldMcc, "0");
        if (strcmp(oldMcc, mcc) != 0) {
            // MCC is changed, clear mapping list
            rfx_property_set(PROPERTY_NW_ECC_LIST[m_slot_id], "");
            // Update MCC
            rfx_property_set(PROPERTY_NW_ECC_MCC[m_slot_id], mcc);
            logD(RFX_LOG_TAG, "[%s] reset %s from %s to %s", __FUNCTION__,
                PROPERTY_NW_ECC_MCC[m_slot_id], oldMcc, mcc);
        }
        mNeedRefreshNwEcc = true;
    } else {
        // Handle +CEN2 URC
        // Example: +CEN2:4,119
        int category = line->atTokNextint(&err);
        if (err < 0) goto error;
        char* number = line->atTokNextstr(&err);
        if (err < 0) goto error;

        // Clear network ECC after receive first CEN2 no matter MCC change or not
        if (mNeedRefreshNwEcc) {
            rfx_property_set(PROPERTY_NW_ECC_LIST[m_slot_id], "");
            mNeedRefreshNwEcc = false;
            logD(RFX_LOG_TAG, "[%s] clear NW ECC after receive first CEN2", __FUNCTION__);
        }

        char eccList[RFX_PROPERTY_VALUE_MAX] = {0};
        char newEccList[RFX_PROPERTY_VALUE_MAX] = {0};
        char findStr[MAX_PROP_CHARS] = {0};
        rfx_property_get(PROPERTY_NW_ECC_LIST[m_slot_id], eccList, "");
        sprintf(findStr, ";%s,", number);
        char *ptr = strstr(eccList, findStr);
        if (ptr == NULL) {
            // Not found number in original ECC list
            sprintf(newEccList, "%s;%s,%d", eccList, number, category);
            logV(RFX_LOG_TAG, "[%s] append number %s to ecc list: %s", __FUNCTION__,
                number, newEccList);
        } else {
            char tmpEccList[RFX_PROPERTY_VALUE_MAX] = {0};

            memset(newEccList, 0, RFX_PROPERTY_VALUE_MAX);
            memset(tmpEccList, 0, RFX_PROPERTY_VALUE_MAX);

            // Copy the mapping string before finding number
            int len = ptr - eccList;
            strncpy(tmpEccList, eccList, len);

            ptr = strstr(ptr + 1, ";");
            if (ptr != NULL) {
                // Copy the mapping string after finding number
                len = sizeof(tmpEccList) - strlen(tmpEccList);
                strncat(tmpEccList, ptr, len - 1);
            }

            sprintf(newEccList, "%s;%s,%d", tmpEccList, number, category);
            logV(RFX_LOG_TAG, "[%s] update number %s in ecc list: %s", __FUNCTION__,
                number, newEccList);
        }

        // Update mapping list
        rfx_property_set(PROPERTY_NW_ECC_LIST[m_slot_id], newEccList);
    }

    // Response to uppper layer (ecc list not used by upper layer)
    // Upper layer requires ECC with SIM and W/O SIM for all ECC sources
    // (Network ECC, SIM ECC, AP set ECC, ...)
    urc = RfxMclMessage::obtainUrc(
            RFX_MSG_UNSOL_ECC_NUM,
            m_slot_id,
            RfxStringData((char *)String8("").string()));
    responseToTelCore(urc);

    return;
error:
    logE(RFX_LOG_TAG, "[%s] parsing error!", __FUNCTION__);
}

void RmcEccNumberUrcHandler::handleTestCommand(const sp<RfxMclMessage>& msg) {
    int err = 0;
    int command[1];
    sp<RfxMclMessage> urc;
    RfxAtLine* line = msg->getRawUrc();
    line->atTokStart(&err);
    if (err < 0) goto error;

    command[0] = line->atTokNextint(&err);
    if (err < 0) goto error;

    urc = RfxMclMessage::obtainUrc(
        RFX_MSG_URC_CC_ECC_NUMBER_TEST,
        m_slot_id,
        RfxIntsData(command, 1));
    responseToTelCore(urc);

    return;
error:
    logE(RFX_LOG_TAG, "[%s] parsing error!", __FUNCTION__);
}
