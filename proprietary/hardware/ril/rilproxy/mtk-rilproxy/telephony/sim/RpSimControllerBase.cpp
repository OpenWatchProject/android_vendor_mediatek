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

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RpSimControllerBase.h"
#include "RfxStatusDefs.h"
#include <log/log.h>
#include <cutils/properties.h>
#include <cutils/jstring.h>


/*****************************************************************************
 * Class RfxController
 *****************************************************************************/

RFX_IMPLEMENT_CLASS("RpSimControllerBase", RpSimControllerBase, RfxController);

RpSimControllerBase::RpSimControllerBase() {
}

const static char* ICC_TYPE_AID_PREFIX[ICC_TYPE_MAX] = {
    "A0000000090001",  // SIM
    "A0000000871002",  // USIM
    "A0000000871004",  // ISIM
    "a0000003431002",  // CSIM
    "FFFFFFFFFFFFFF"  // FIXME: RUIM
};

RpSimControllerBase::~RpSimControllerBase() {
}

const char* RpSimControllerBase::requestToString(int reqId) {
    switch (reqId) {
        case RIL_REQUEST_GET_SIM_STATUS:
            return "GET_SIM_STATUS";
        case RIL_REQUEST_GET_IMSI:
            return "GET_IMSI";
        case RIL_REQUEST_ENTER_SIM_PIN:
            return "ENTER_SIM_PIN";
        case RIL_REQUEST_ENTER_SIM_PUK:
            return "ENTER_SIM_PUK";
        case RIL_REQUEST_ENTER_SIM_PIN2:
            return "ENTER_SIM_PIN2";
        case RIL_REQUEST_ENTER_SIM_PUK2:
            return "ENTER_SIM_PUK2";
        case RIL_REQUEST_CHANGE_SIM_PIN:
            return "CHANGE_SIM_PIN";
        case RIL_REQUEST_CHANGE_SIM_PIN2:
            return "CHANGE_SIM_PIN2";
        case RIL_REQUEST_QUERY_FACILITY_LOCK:
            return "QUERY_FACILITY_LOCK";
        case RIL_REQUEST_SET_FACILITY_LOCK:
            return "SET_FACILITY_LOCK";
        case RIL_REQUEST_SIM_OPEN_CHANNEL:
            return "SIM_OPEN_CHANNEL";
        case RIL_REQUEST_SIM_CLOSE_CHANNEL:
            return "SIM_CLOSE_CHANNEL";
        case RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL:
            return "SIM_TRANSMIT_APDU_CHANNEL";
        case RIL_REQUEST_SET_UICC_SUBSCRIPTION:
            return "SET_UICC_SUBSCRIPTION";
        case RIL_REQUEST_SIM_IO:
            return "SIM_IO";
        case RIL_REQUEST_SIM_AUTHENTICATION:
            return "SIM_AUTHENTICATION";
        case RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC:
            return "SIM_TRANSMIT_APDU_BASIC";
        case RIL_REQUEST_SET_ALLOWED_CARRIERS:
            return "RIL_REQUEST_SET_ALLOWED_CARRIERS";
        case RIL_REQUEST_GET_ALLOWED_CARRIERS:
            return "RIL_REQUEST_GET_ALLOWED_CARRIERS";
        default:
            RLOGE("[RpSimControllerBase] reqId %d, <UNKNOWN_REQUEST>", reqId);
            return "<UNKNOWN_REQUEST>";
    }
}

const char* RpSimControllerBase::urcToString(int reqId) {
    switch (reqId) {
        case RIL_LOCAL_GSM_UNSOL_CARD_TYPE_NOTIFY:
            return "UNSOL_CARD_TYPE_NOTIFY";
        case RIL_LOCAL_GSM_UNSOL_CT3G_DUALMODE_CARD:
            return "UNSOL_CT3G_DUALMODE_CARD";
        case RIL_LOCAL_C2K_UNSOL_CDMA_CARD_TYPE_NOTIFY:
            return "UNSOL_CDMA_CARD_TYPE_NOTIFY";
        case RIL_LOCAL_C2K_UNSOL_CDMA_CARD_READY:
            return "UNSOL_CDMA_CARD_READY";
        case RIL_LOCAL_C2K_UNSOL_CDMA_IMSI_READY:
            return "UNSOL_CDMA_IMSI_READY";
        case RIL_UNSOL_UICC_SUBSCRIPTION_STATUS_CHANGED:
            return "UNSOL_UICC_SUBSCRIPTION_STATUS_CHANGED";
        case RIL_UNSOL_SIM_PLUG_IN:
            return "UNSOL_SIM_PLUG_IN";
        case RIL_UNSOL_SIM_PLUG_OUT:
            return "UNSOL_SIM_PLUG_OUT";
        case RIL_UNSOL_TRAY_PLUG_IN:
            return "UNSOL_TRAY_PLUG_IN";
        case RIL_UNSOL_SIM_SLOT_LOCK_POLICY_NOTIFY:
            return "UNSOL_SIM_SLOT_LOCK_POLICY_NOTIFY";
        default:
            RLOGE("[RpSimControllerBase] urc %d, <UNKNOWN_URC>", reqId);
            return "<UNKNOWN_URC>";
    }
}

// The API will help to allocate memory so remember to free it while you don't use the buffer anymore
char* RpSimControllerBase::strdupReadString(Parcel *p) {
    size_t stringlen;
    const char16_t *s16 = NULL;

    s16 = p->readString16Inplace(&stringlen);

    return strndup16to8(s16, stringlen);
}

void RpSimControllerBase::writeStringToParcel(Parcel *p, const char *s) {
    char16_t *s16 = NULL;
    size_t s16_len = 0;
    s16 = strdup8to16(s, &s16_len);
    p->writeString16(s16, s16_len);
    free(s16);
}

bool RpSimControllerBase::supportCardType(const int icc_types) {
    bool result = false;
    int slotId = getSlotId();
    int allIccType = 0;

    do {
        if (slotId >= 0 && slotId <= 4) {
            allIccType = getStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE, -1);
        } else {
            RLOGE("[RpSimControllerBase] supportCardType, slotId %d is wrong!", slotId);
            break;
        }

        if ((allIccType > 0) && (icc_types & allIccType) != 0) {
            result = true;
        }
    } while (0);

    // RLOGD("[RpSimControllerBase] supportCardType, result %d! (slot %d)", result, slotId);
    return result;
}


RILD_RadioTechnology_Group RpSimControllerBase::choiceDestViaAid(char* aid_ptr) {
    RILD_RadioTechnology_Group dest = RADIO_TECH_GROUP_GSM;

    do {
        if (aid_ptr == NULL && supportCardType(ICC_SIM)) {
            // This is SIM only case so send to GSM RILD
            RLOGD("[RpSimControllerBase] choiceDestination, card type is SIM only (slot %d)",
                    getSlotId());
            break;
        } else if (aid_ptr == NULL && supportCardType(ICC_RUIM)) {
            // This is RUIM only case so send to C2K RILD
            RLOGD("[RpSimControllerBase] choiceDestination, card type is RUIM only (slot %d)",
                    getSlotId());
            dest = RADIO_TECH_GROUP_C2K;
            break;
        } else if (aid_ptr != NULL) {
            // Get target application according to AID
            int iccType = 0;
            for (iccType = 0; iccType < ICC_TYPE_MAX; iccType++) {
                if ((aid_ptr != NULL) && (ICC_TYPE_AID_PREFIX[iccType] != NULL) &&
                        (strncmp(aid_ptr, ICC_TYPE_AID_PREFIX[iccType], AID_PREFIX_LEN) == 0)) {
                    // Match!
                    break;
                }
            }

            if (iccType == ICC_TYPE_SIM || iccType == ICC_TYPE_USIM || iccType == ICC_TYPE_ISIM) {
                // GSM RILD is our destination
                break;
            } else if (iccType == ICC_TYPE_CSIM || iccType == ICC_TYPE_RUIM) {
                // C2K RILD is our destination
                dest = RADIO_TECH_GROUP_C2K;
                break;
            } else {
                RLOGD("[RpSimControllerBase] choiceDestination, not support the AID %s (slot %d)",
                        aid_ptr, getSlotId());
                dest = choiceDestViaCurrCardType();
            }
        } else {
            RLOGE("[RpSimControllerBase] choiceDestination, AID is null!!! (slot %d)", getSlotId());
        }
    } while (0);

    return dest;
}

// The API will return RADIO_TECH_GROUP_GSM if there is "USIM/SIM" in the card,
// e.g. 4G CDMA card, USIM, SIM...etc.
// Otherwise return RADIO_TECH_GROUP_C2K
RILD_RadioTechnology_Group RpSimControllerBase::choiceDestViaCurrCardType() {
    RILD_RadioTechnology_Group dest = RADIO_TECH_GROUP_GSM;
    int slotId = getSlotId();
    int allIccType = 0;

    do {
        if (slotId >= 0 && slotId <= 4) {
            allIccType = getStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE, -1);
        } else {
            RLOGE("[RpSimControllerBase] choiceDestViaCurrCardType, slotId %d is wrong!", slotId);
            break;
        }

        if ((allIccType <= 0) || (allIccType == RFX_CARD_TYPE_SIM) ||
                ((RFX_CARD_TYPE_USIM & allIccType) != 0)) {
            // Send GSM RILD the request if there is SIM/USIM
            break;
        }

        if (((RFX_CARD_TYPE_CSIM & allIccType) != 0) || ((RFX_CARD_TYPE_RUIM & allIccType) != 0)) {
            // Send C2K RILD the request if this is pure CDMA card
            dest = RADIO_TECH_GROUP_C2K;
            break;
        }

        RLOGE("[RpSimControllerBase] choiceDestViaCurrCardType, No 3GPP and 3GPP2?!(slot %d)",
                getSlotId());
    } while (0);

    return dest;
}

int RpSimControllerBase::isSupportCommonSlot() {
    /* Return 1 if support SIM Hot Swap with Common Slot Feature */
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.vendor.mtk_sim_hot_swap_common_slot", property_value, "0");
    return atoi(property_value);
}

bool RpSimControllerBase::isOp09Card(const char *iccid) {
    bool isOp09Card = false;

    // Compare with OP09 iccid prefix.
    if ((strncmp(iccid, "898603", 6) == 0) ||
            (strncmp(iccid, "898611", 6) == 0) ||
            (strncmp(iccid, "8985302", 7) == 0) ||
            (strncmp(iccid, "8985307", 7) == 0) ||
            (strncmp(iccid, "8985231", 7) == 0) ||
            (strncmp(iccid, "898120", 6) == 0)) {
        isOp09Card = true;
    }
    RLOGD("[RpSimController] isOp09Card: %d, iccid: %.7s", isOp09Card, iccid);
    return isOp09Card;
}

bool RpSimControllerBase::isSimSlotLockSupport() {
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.vendor.sim_me_lock_mode", property_value, "");
    return strcmp(property_value, "3") == 0;
}
