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

#include "RpDataUtils.h"
#include <cutils/jstring.h>

/*****************************************************************************
 * Class RpDataUtils
 *****************************************************************************/

RpDataUtils::RpDataUtils() {
}

RpDataUtils::~RpDataUtils() {
}

const char* RpDataUtils::requestToString(int reqId) {
    switch (reqId) {
        case RIL_REQUEST_SETUP_DATA_CALL:
            return "SETUP_DATA_CALL";
        case RIL_REQUEST_DEACTIVATE_DATA_CALL:
            return "DEACTIVATE_DATA_CALL";
        case RIL_REQUEST_DATA_CALL_LIST:
            return "GET_DATA_CALL";
        case RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE:
            return "GET_LAST_FAIL_CAUSE";
        case RIL_REQUEST_SET_INITIAL_ATTACH_APN:
            return "SET_INITIAL_ATTACH_APN";
        case RIL_REQUEST_ALLOW_DATA:
            return "ALLOW_DATA";
        case RIL_REQUEST_SET_DATA_PROFILE:
            return "SET_DATA_PROFILE";
        case RIL_REQUEST_SET_PS_REGISTRATION:
            return "RIL_REQUEST_SET_PS_REGISTRATION";
        // M: MPS feature
        case RIL_LOCAL_REQUEST_DEACTIVATE_ALL_DATA_CALL:
            return "DEACTIVATE_ALL_DATA_CALL";
        case RIL_LOCAL_C2K_REQUEST_DEACTIVATE_ALL_DATA_CALL:
            return "C2K_DEACTIVATE_ALL_DATA_CALL";
        case RIL_LOCAL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE_ALT:
            return "GET_LAST_FAIL_CAUSE_ALT";
        case RIL_LOCAL_REQUEST_SETUP_DATA_CALL_ALT:
            return "SETUP_DATA_CALL_ALT";
        case RIL_REQUEST_SYNC_DATA_SETTINGS_TO_MD:
            return "RIL_REQUEST_SYNC_DATA_SETTINGS_TO_MD";
        case RIL_LOCAL_REQUEST_GET_IMS_DATA_CALL_INFO:
            return "RIL_LOCAL_REQUEST_GET_IMS_DATA_CALL_INFO";
        case RIL_LOCAL_REQUEST_REUSE_IMS_DATA_CALL:
            return "RIL_LOCAL_REQUEST_REUSE_IMS_DATA_CALL";
        case RIL_REQUEST_SET_LINK_CAPACITY_REPORTING_CRITERIA:
            return "SET_LINK_CAPACITY_REPORTING_CRITERIA";
        default:
            return "UNKNOWN_REQUEST";
    }
}

const char* RpDataUtils::urcToString(int reqId) {
    switch (reqId) {
        case RIL_UNSOL_DATA_CALL_LIST_CHANGED:
            return "DATA_CALL_LIST_CHANGED";
        case RIL_LOCAL_GSM_UNSOL_INTER_3GPP_IRAT_STATE_CHANGE:
            return "RIL_LOCAL_GSM_UNSOL_INTER_3GPP_IRAT_STATE_CHANGE";
        case RIL_LOCAL_C2K_UNSOL_INTER_3GPP_IRAT_STATE_CHANGE:
            return "RIL_LOCAL_C2K_UNSOL_INTER_3GPP_IRAT_STATE_CHANGE";
        // M: PDN deactivation failure info to RILP
        case RIL_LOCAL_GSM_UNSOL_PDN_DEACT_FAILURE_IND:
            return "RIL_LOCAL_GSM_UNSOL_PDN_DEACT_FAILURE_IND";
        case RIL_UNSOL_PCO_DATA:
            return "RIL_UNSOL_PCO_DATA";
        case RIL_UNSOL_PCO_DATA_AFTER_ATTACHED:
            return "RIL_UNSOL_PCO_DATA_AFTER_ATTACHED";
        case RIL_UNSOL_VOLTE_LTE_CONNECTION_STATUS:
            return "RIL_UNSOL_VOLTE_LTE_CONNECTION_STATUS";
        default:
            return "UNKNOWN_URC";
    }
}

const char *RpDataUtils::radioGroupToString(int radioGroupId) {
    switch (radioGroupId) {
        case RADIO_TECH_GROUP_GSM:
            return "GSM";
        case RADIO_TECH_GROUP_C2K:
            return "C2K";
        default:
            return "INVALID SRC";
    }
}

/*
 * major slot is the one that with 4G capability.
 * return index from 0.
 */
int RpDataUtils::getMajorSlot() {
    char tmp[PROPERTY_VALUE_MAX] = { 0 };
    int simId = 0;
    property_get(RP_DATA_PROPERTY_4G_SIM, tmp, "1");
    simId = atoi(tmp);
    RFX_LOG_D(RP_DC_UTIL_LOG_TAG , "getMajorSlot: slot = %d.", (simId - 1));
    return (simId - 1);  // make first slot index as 0
}

bool RpDataUtils::isCdmaCard(const int cardType) {
    switch (cardType) {
    case UIM_CARD:
    case UIM_SIM_CARD:
    case CT_3G_UIM_CARD:
    case CT_UIM_SIM_CARD:
    case CT_4G_UICC_CARD:
    case NOT_CT_UICC_CARD:
        return true;
    default:
        return false;
    }
}

/**
 * NOTES: need to recycle the parcel after used.
 */
Parcel* RpDataUtils::createResponseParcelFromMessage(const sp<RfxMessage>& message) {
    Parcel* parcelSource = message->getParcel();
    Parcel* parcelTarget = new Parcel();
    parcelTarget->writeInt32(RESPONSE_SOLICITED); // RESPONSE_SOLICITED
    parcelTarget->writeInt32(message->getPToken() & 0xFFFFFFFF);
    parcelTarget->writeInt32(message->getError() & 0xFFFFFFFF);
    int offset = sizeof(int32_t) * 3;
    if (message!= NULL && parcelSource->dataSize() > offset) {
        parcelTarget->appendFrom(parcelSource, offset, parcelSource->dataSize()-offset);
    }
    RFX_LOG_D(RP_DC_UTIL_LOG_TAG , "[%s] source data size = %zu, target data size = %zu",
            __FUNCTION__, parcelSource->dataSize(), parcelTarget->dataSize());
    return parcelTarget;
}

Vector<RpDataConnectionInfo*>* RpDataUtils::parseDataConnectionInfo(Parcel* parcel) {
    int ver = parcel->readInt32();
    int num = parcel->readInt32();
    RFX_LOG_D(RP_DC_UTIL_LOG_TAG , "RpDataConnectionInfo ver: %d, num: %d", ver, num);
    Vector<RpDataConnectionInfo*>* list = new Vector<RpDataConnectionInfo*>();
    for (int i = 0; i < num; i++) {
        RpDataConnectionInfo* info = new RpDataConnectionInfo();
        int status = parcel->readInt32();  // status
        int suggestedRetryTime = parcel->readInt32();  // suggestedRetryTime
        info->interfaceId = parcel->readInt32();  // cid(interfaeId in rilj)
        int active = parcel->readInt32();  // active
        info->type = parcel->readString16();  // type
        info->ifname = parcel->readString16();  // ifname
        info->addresses = parcel->readString16();  // addresses
        parcel->readString16();  // dnses
        parcel->readString16();  // gateways
        parcel->readString16();  // pcscf
        parcel->readInt32();  // mtu
        parcel->readInt32();  // erat_type
        info->dump();
        list->add(info);
    }
    return list;
}

void RpDataUtils::freeDataList(Vector<RpDataConnectionInfo*>* list) {
    if (list != NULL) {
        int size = list->size();
        for (int i = 0; i < size; i++) {
            delete list->itemAt(i);
        }
        delete list;
    }
}

bool RpDataUtils::syncDataConnectionApn(Vector<RpDataConnectionInfo*>* old,
                                        Vector<RpDataConnectionInfo*>* current) {
    bool updated = false;
    int oldSize = old->size();
    int newSize = current->size();
    for (int i = 0; i < newSize; i++) {
        for (int j = 0; j < oldSize; j++) {
            if (current->itemAt(i)->interfaceId == old->itemAt(j)->interfaceId) {
                current->itemAt(i)->apn = old->itemAt(j)->apn;
                updated = true;
                break;
            }
        }
    }
    return updated;
}

void RpDataUtils::storeIaProperty(int slotId, String16 apn) {
    char iccid[PROPERTY_VALUE_MAX] = { 0 };
    char iaProperty[PROPERTY_VALUE_MAX] = { 0 };
    int major = RpDataUtils::getMajorSlot();
    if (slotId != major) {
        RFX_LOG_D(RP_DC_UTIL_LOG_TAG ,
                "storeIaProperty slot not major! current:%d,major:%d", slotId, major);
        return;
    }

    property_get(PROPERTY_ICCID_SIM[slotId], iccid, "");
    snprintf(iaProperty, PROPERTY_VALUE_MAX, "%s,%s", iccid, String8(apn).string());
    RFX_LOG_D(RP_DC_UTIL_LOG_TAG ,
            "storeIaProperty record apn: %s, slotId: %d", String8(apn).string(), slotId);
    property_set(RP_DATA_PROPERTY_IA[slotId], iaProperty);
}

// SETUP_DATA_CALL RIL interface AOSP refactoring start
int RpDataUtils::getProtocolTypeId(const char *protocol){
    int type = IPV4;

    if (protocol == NULL) {
        return type;
    }

    if (!strcmp(protocol, SETUP_DATA_PROTOCOL_IP)) {
        type = IPV4;
    } else if (!strcmp(protocol, SETUP_DATA_PROTOCOL_IPV6)) {
        type = IPV6;
    } else if (!strcmp(protocol, SETUP_DATA_PROTOCOL_IPV4V6)) {
        type = IPV4V6;
    }
    return type;
}
// SETUP_DATA_CALL RIL interface AOSP refactoring end

char *RpDataUtils::strdupReadString(Parcel *p) {
    size_t stringlen;
    const char16_t *s16;

    s16 = p->readString16Inplace(&stringlen);

    return strndup16to8(s16, stringlen);
}

bool RpDataUtils::isPreferredDataMode() {
    char preferredDataMode[PROPERTY_VALUE_MAX] = {0};
    property_get("vendor.ril.data.preferred_data_mode", preferredDataMode, "0");
    RFX_LOG_D(RP_DC_UTIL_LOG_TAG, "isPreferredDataMode: preferred_data_mode = %d",
            atoi(preferredDataMode));
    return atoi(preferredDataMode) == 1 ? true : false;
}

void RpDataUtils::setAllowDataSlot(bool allow, int slot) {
    if (allow) {
        RFX_LOG_I(RP_DC_UTIL_LOG_TAG, "setAllowDataSlot allow= %d slot= %d", allow, slot);
        property_set("vendor.ril.data.allow_data_slot", String8::format("%d", slot).string());
    }
}

int RpDataUtils::getAllowDataSlot() {
    char allowDataSlot[PROPERTY_VALUE_MAX] = { 0 };
    property_get("vendor.ril.data.allow_data_slot", allowDataSlot, "-1");
    int allowSlot = atoi(allowDataSlot);
    RFX_LOG_I(RP_DC_UTIL_LOG_TAG, "getAllowDataSlot slot prop = %d", allowSlot);
    return allowSlot;
}
