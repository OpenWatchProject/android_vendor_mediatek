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

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxVoidData.h"
#include "RmcSimBaseHandler.h"
#include "RmcCommSimDefs.h"
#include "rfx_properties.h"
#include <telephony/mtk_ril.h>
#include <ctype.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "usim_fcp_parser.h"
#ifdef __cplusplus
}
#endif

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/

/* OPERATOR*/
#define OPERATOR_OP09 "OP09"
#define SPEC_OP09_A "SEGDEFAULT"

RmcSimBaseHandler::RmcSimBaseHandler(int slot_id, int channel_id) :
        RfxBaseHandler(slot_id, channel_id) {
    setTag(String8("RmcSimBaseHandler"));
}

RmcSimBaseHandler::~RmcSimBaseHandler() {
}

RmcSimBaseHandler::SIM_HANDLE_RESULT RmcSimBaseHandler::needHandle(
        const sp<RfxMclMessage>& msg) {
    RFX_UNUSED(msg);
    return RESULT_IGNORE;
}

void RmcSimBaseHandler::handleRequest(const sp<RfxMclMessage>& msg) {
    RFX_UNUSED(msg);
}

void RmcSimBaseHandler::handleUrc(const sp<RfxMclMessage>& msg, RfxAtLine *urc) {
    RFX_UNUSED(msg);
    RFX_UNUSED(urc);
}

const int* RmcSimBaseHandler::queryTable(int channel_id, int *record_num) {
    RFX_UNUSED(channel_id);
    RFX_UNUSED(record_num);
    return NULL;
}

const char** RmcSimBaseHandler::queryUrcTable(int *record_num) {
    RFX_UNUSED(record_num);
    return NULL;
}

void RmcSimBaseHandler::setTag(String8 s) {
    mTag = s;
}

UICC_Status RmcSimBaseHandler::getSimStatus() {
    int err, count = 0;
    UICC_Status ret = UICC_NOT_READY;
    RfxAtLine *line = NULL;
    char *cpinResult = NULL;
    sp<RfxAtResponse> p_response = NULL;
    int pivot = 0;
    int inserted = 0;
    int currSimInsertedState = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_SIM_STATE);
    int currRadioState = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_RADIO_STATE);

    pthread_mutex_lock(&simStatusMutex);
    logD(mTag, "getSIMStatus: currRadioState %d, currSimInsertedState %d",
            currRadioState,currSimInsertedState);
    // Get SIM status
    do {
        // JB MR1, it will request sim status after receiver iccStatusChangedRegistrants,
        // but MD is off in the mean time, so it will get the exception result of CPIN.
        // For this special case, handle it specially.
        // check md off and sim inserted status, then return the result directly instead of request CPIN.
        // not insert: return SIM_ABSENT, insert: return SIM_NOT_READY or USIM_NOT_READY
        // TODO: wait s_md_off support
        /*
        if (s_md_off) {
            int slot = (1 << getMappingSIMByCurrentMode(rid));
            logI(mTag, "getSIMStatus s_md_off: %d slot: %d", s_md_off, slot);
            if (isDualTalkMode()) {
                pivot = 1 << m_slot_id;
                inserted = pivot & currSimInsertedState;
            } else {
                inserted = slot & currSimInsertedState;
            }
            if (!inserted) {
                ret = UICC_ABSENT;
                break;
            } else {
                ret = UICC_NOT_READY;
                break;
            }
        }
        */
        if (currRadioState == RADIO_STATE_UNAVAILABLE) {
             ret = UICC_NOT_READY;
             break;
        }
        p_response = atSendCommandSingleline("AT+CPIN?", "+CPIN:");
        if (p_response == NULL) {
            break;
        }
        err = p_response->getError();
        if (err != 0) {
            //if (err == AT_ERROR_INVALID_THREAD) {
            //  ret = UICC_BUSY;
            //} else {
                ret = UICC_NOT_READY;
            //}
        } else if (p_response->getSuccess() == 0) {
            switch (p_response->atGetCmeError()) {
                case CME_SIM_BUSY:
                    ret = UICC_BUSY;
                    break;
                case CME_SIM_NOT_INSERTED:
                case CME_SIM_FAILURE:
                    ret = UICC_ABSENT;
                    break;
                case CME_SIM_WRONG: {
                    RmcSimPinPukCount *retry = getPinPukRetryCount();
                    if (retry != NULL && retry->pin1 == 0 && retry->puk1 == 0) {
                        ret = UICC_PERM_BLOCKED; // PERM_DISABLED
                    } else if (retry->pin1 == -1 && retry->puk1 == -1 &&
                        retry->pin2 == -1 && retry->puk2 == -1) {
                        ret = UICC_ABSENT;
                    }else {
                        ret = UICC_NOT_READY;
                    }
                    if (retry != NULL) {
                        free(retry);
                    }
                    break;
                }
                default:
                    ret = UICC_NOT_READY;
                    break;
            }
        } else {
            // Success
            line = p_response->getIntermediates();
            if (line == NULL) {
                break;
            }

            line->atTokStart(&err);
            if (err < 0) {
                ret = UICC_NOT_READY;
                break;
            }
            cpinResult = line->atTokNextstr(&err);
            if (err < 0) {
                ret = UICC_NOT_READY;
                break;
            }
            if (cpinResult == NULL) {
                break;
            }
            logD(mTag, "getSIMStatus: cpinResult %s", cpinResult);

            String8 cpinStr(cpinResult);
            if (cpinStr == String8::format("SIM PIN")) {
                ret = UICC_PIN;
            } else if (cpinStr == String8::format("SIM PUK")) {
                ret = UICC_PUK;
            } else if (cpinStr == String8::format("PH-NET PIN") ||
                cpinStr == String8::format("PH-NET PUK")) {
                ret = UICC_NP;
            } else if (cpinStr == String8::format("PH-NETSUB PIN") ||
                cpinStr == String8::format("PH-NETSUB PUK")) {
                ret = UICC_NSP;
            } else if (cpinStr == String8::format("PH-SP PIN") ||
                cpinStr == String8::format("PH-SP PUK")) {
                ret = UICC_SP;
            } else if (cpinStr == String8::format("PH-CORP PIN") ||
                cpinStr == String8::format("PH-CORP PUK")) {
                ret = UICC_CP;
            } else if (cpinStr == String8::format("PH-FSIM PIN") ||
                cpinStr == String8::format("PH-FSIM PUK")) {
                ret = UICC_SIMP;
            } else if (cpinStr == String8::format("PH-NSSP PIN") ||
                    cpinStr == String8::format("PH-NSSP PUK")) {
                ret = UICC_NS_SP;
            } else if (cpinStr == String8::format("PH-SIMC PIN") ||
                    cpinStr == String8::format("PH-SIMC PUK")) {
                ret = UICC_SIM_C;
            } else if (cpinStr != String8::format("READY"))  {
                /* we're treating unsupported lock types as "sim absent" */
                ret = UICC_ABSENT;
            } else {
                ret = UICC_READY;
            }
        }
    } while (0);
    pthread_mutex_unlock(&simStatusMutex);
    logD(mTag, "getSIMStatus: ret %d", ret);
    return ret;
}

int RmcSimBaseHandler::queryAppTypeId(String8 aid) {
    int appTypeId = UICC_APP_SIM; // Default is SIM
    char *string8Null = NULL;

    if (aid.isEmpty() || aid == String8::format("%s", string8Null)) {
        // SIM or RUIM
        int cardType = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE, -1);
        logD(mTag, "queryAppTypeId aid is empty or \"(null)\", aid = %s, cardType = %d",
                aid.string(), cardType);
        if (cardType & RFX_CARD_TYPE_SIM) {
            appTypeId = UICC_APP_SIM;
        } else if (cardType & RFX_CARD_TYPE_USIM) {
            appTypeId = UICC_APP_USIM;
        } else if (cardType & RFX_CARD_TYPE_CSIM) {
            appTypeId = UICC_APP_CSIM;
        } else if (cardType & RFX_CARD_TYPE_RUIM) {
            appTypeId = UICC_APP_RUIM;
        } else {
            logD(mTag, "Could not get app id because card type is not ready!");
        }
    } else if (strncmp(aid.string(), "A0000000871002", 14) == 0) {
        // USIM
        appTypeId = UICC_APP_USIM; // USIM
    } else if (strncmp(aid.string(), "A0000000871004", 14) == 0) {
        // ISIM
        appTypeId = UICC_APP_ISIM;
    } else if (strncmp(aid.string(), "A0000003431002", 14) == 0) {
        // CSIM
        appTypeId = UICC_APP_CSIM;
    } else if (strncmp(aid.string(), "A000000000RUIM", 14) == 0) {
        // RUIM
        appTypeId = UICC_APP_RUIM;
    } else {
        logD(mTag, "Not support the aid %s", aid.string());
        appTypeId = -1;
    }

    return appTypeId;
}

bool RmcSimBaseHandler::bIsTc1()
{
    static int siTc1 = -1;

    if (siTc1 < 0)
    {
        char cTc1[RFX_PROPERTY_VALUE_MAX] = { 0 };

        rfx_property_get("ro.vendor.mtk_tc1_feature", cTc1, "0");
        siTc1 = atoi(cTc1);
    }

    return ((siTc1 > 0) ? true : false);
}

bool RmcSimBaseHandler::isSimInserted() {
    char iccid[RFX_PROPERTY_VALUE_MAX] = {0};
    String8 prop(PROPERTY_ICCID_PREIFX);

    prop.append(String8::format("%d", (m_slot_id + 1)));
    rfx_property_get(prop.string(), iccid, "");

    if ((strlen(iccid) > 0) && (strcmp(iccid, "N/A") != 0)){
        return true;
    }
    return false;
}

bool RmcSimBaseHandler::getIccId(char* value) {
    bool isIccIdReady = false;

    char iccid[RFX_PROPERTY_VALUE_MAX] = {0};
    String8 prop(PROPERTY_ICCID_PREIFX);

    prop.append(String8::format("%d", (m_slot_id + 1)));
    rfx_property_get(prop.string(), iccid, "");

    if (strlen(iccid) > 0) {
        isIccIdReady = true;

        if (strcmp(iccid, "N/A") != 0) {
            rfx_property_get(prop.string(), value, "");
        }
    }

    return isIccIdReady;
}

bool RmcSimBaseHandler::isSimSlotLockSupport() {
    char property_value[RFX_PROPERTY_VALUE_MAX] = {0};

    rfx_property_get("ro.vendor.sim_me_lock_mode", property_value, "");

    if ((strlen(property_value) > 0) && (strcmp(property_value, "3") == 0)){
        return true;
    }
    return false;
}

bool RmcSimBaseHandler::isCommontSlotSupport() {
    char property_value[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get(PROPERTY_COMMON_SLOT_SUPPORT, property_value, "0");
    return atoi(property_value) == 1 ? true:false;
}

RmcSimPinPukCount* RmcSimBaseHandler::getPinPukRetryCount() {
    sp<RfxAtResponse> p_response = NULL;
    int err;
    int ret;
    RfxAtLine*line;
    RmcSimPinPukCount *retry = (RmcSimPinPukCount*)calloc(1, sizeof(RmcSimPinPukCount));
    assert(retry != NULL);
    retry->pin1 = -1;
    retry->pin2 = -1;
    retry->puk1 = -1;
    retry->puk2 = -1;

    p_response = atSendCommandSingleline("AT+EPINC", "+EPINC:");

    if (p_response != NULL && p_response->getSuccess() > 0) {
        // Success
        do {
            line = p_response->getIntermediates();

            line->atTokStart(&err);

            if (err < 0) {
                logE(mTag, "get token error");
                break;
            }

            retry->pin1 = line->atTokNextint(&err);
            if (err < 0) {
                logE(mTag, "get pin1 fail");
                break;
            }

            retry->pin2 = line->atTokNextint(&err);
            if (err < 0) {
                logE(mTag, "get pin2 fail");
                break;
            }

            retry->puk1 = line->atTokNextint(&err);
            if (err < 0) {
                logE(mTag, "get puk1 fail");
                break;
            }

            retry->puk2 = line->atTokNextint(&err);
            if (err < 0) {
                logE(mTag, "get puk2 fail");
                break;
            }

            setPinPukRetryCountProp(retry);
        } while(0);

    } else {
        logE(mTag, "Fail to get PIN and PUK retry count!");
    }

    p_response = NULL;
    logD(mTag, "pin1:%d, pin2:%d, puk1:%d, puk2:%d",
            retry->pin1,retry->pin2,retry->puk1,retry->puk2);

    return retry;
}

void RmcSimBaseHandler::setPinPukRetryCountProp(RmcSimPinPukCount *retry) {
    String8 pin1("vendor.gsm.sim.retry.pin1");
    String8 pin2("vendor.gsm.sim.retry.pin2");
    String8 puk1("vendor.gsm.sim.retry.puk1");
    String8 puk2("vendor.gsm.sim.retry.puk2");

    pin1.append((m_slot_id == 0)? "" : String8::format(".%d", (m_slot_id+1)));
    pin2.append((m_slot_id == 0)? "" : String8::format(".%d", (m_slot_id+1)));
    puk1.append((m_slot_id == 0)? "" : String8::format(".%d", (m_slot_id+1)));
    puk2.append((m_slot_id == 0)? "" : String8::format(".%d", (m_slot_id+1)));

    rfx_property_set(pin1.string(), String8::format("%d", retry->pin1).string());
    rfx_property_set(pin2.string(), String8::format("%d", retry->pin2).string());
    rfx_property_set(puk1.string(), String8::format("%d", retry->puk1).string());
    rfx_property_set(puk2.string(), String8::format("%d", retry->puk2).string());

    if (isAOSPPropSupport()) {
        String8 aospPin;
        String8 aospPin2;
        String8 aospPuk;
        String8 aospPuk2;

        if (RfxRilUtils::rfxGetSimCount() >= 2) {
            String8 slot("gsm.slot");
            slot.append(String8::format("%d", (m_slot_id+1)));
            aospPin = String8::format("%s%s", slot.string(), ".num.pin1");
            aospPin2 = String8::format("%s%s", slot.string(), ".num.pin2");
            aospPuk = String8::format("%s%s", slot.string(), ".num.puk1");
            aospPuk2 = String8::format("%s%s", slot.string(), ".num.puk2");
        } else {
            aospPin = String8::format("%s", "gsm.sim.num.pin");
            aospPin2 = String8::format("%s", "gsm.sim.num.pin2");
            aospPuk = String8::format("%s", "gsm.sim.num.puk");
            aospPuk2 = String8::format("%s", "gsm.sim.num.puk2");
        }

        rfx_property_set(aospPin, String8::format("%d", retry->pin1).string());
        rfx_property_set(aospPin2, String8::format("%d", retry->pin2).string());
        rfx_property_set(aospPuk, String8::format("%d", retry->puk1).string());
        rfx_property_set(aospPuk2, String8::format("%d", retry->puk2).string());
    }
}

RmcSimSlotLockDeviceLockInfo* RmcSimBaseHandler::getSimSlotLockDeviceLockInfo() {
    sp<RfxAtResponse> p_response = NULL;
    int err;
    int ret;
    RfxAtLine*line;
    RmcSimSlotLockDeviceLockInfo *retry = (RmcSimSlotLockDeviceLockInfo*)calloc(1,
            sizeof(RmcSimSlotLockDeviceLockInfo));
    RFX_ASSERT(retry != NULL);
    retry->lock_state = -1;
    retry->algo = -1;
    retry->maximum = -1;
    retry->remain = -1;

    p_response = atSendCommandSingleline("AT+ESLBLOB=5", "+ESLBLOB:");

    if (p_response != NULL && p_response->getSuccess() > 0) {
        // Success
        do {
            line = p_response->getIntermediates();

            line->atTokStart(&err);

            if (err < 0) {
                logE(mTag, "get token error");
                break;
            }

            retry->lock_state = line->atTokNextint(&err);
            if (err < 0) {
                logE(mTag, "get lock_state fail");
                break;
            }

            retry->algo = line->atTokNextint(&err);
            if (err < 0) {
                logE(mTag, "get algo fail");
                break;
            }

            retry->maximum = line->atTokNextint(&err);
            if (err < 0) {
                logE(mTag, "get maximum fail");
                break;
            }

            retry->remain = line->atTokNextint(&err);
            if (err < 0) {
                logE(mTag, "get remain fail");
                break;
            }

            setSimSlotLockDeviceLockInfoProp(retry);
        } while (0);

    } else {
        logE(mTag, "Fail to get sml sbp retry count!");
    }

    p_response = NULL;
    logD(mTag, "lock_state:%d, algo:%d, maximum:%d, remain:%d",
            retry->lock_state, retry->algo, retry->maximum, retry->remain);

    return retry;
}

void RmcSimBaseHandler::setSimSlotLockDeviceLockInfoProp(RmcSimSlotLockDeviceLockInfo *retry) {
    rfx_property_set(PROPERTY_SIM_SLOT_LOCK_DEVICE_LOCK_REMAIN_COUNT,
            String8::format("%d", retry->remain).string());
}

bool RmcSimBaseHandler::isOP09AProject() {
    char optr_value[RFX_PROPERTY_VALUE_MAX] = { 0 };
    char seg_value[RFX_PROPERTY_VALUE_MAX] = { 0 };

    rfx_property_get("persist.vendor.operator.optr", optr_value, "0");
    rfx_property_get("persist.vendor.operator.seg", seg_value, "0");

    if ((strncmp(optr_value, OPERATOR_OP09, strlen(OPERATOR_OP09)) == 0) &&
            (strncmp(seg_value, SPEC_OP09_A, strlen(SPEC_OP09_A)) == 0)) {
        return true;
    }

    return false;
}

char* RmcSimBaseHandler::stringToUpper(char *str) {
    char *p = str;
    for (; *p != '\0'; p++) {
        *p = toupper(*p);
    }
    return str;
}

bool RmcSimBaseHandler::isAOSPPropSupport() {
    return true;
}

bool RmcSimBaseHandler::isSimIoFcp(char* response) {
    bool isFcp = false;

    if (NULL != response && strlen(response) >= 2) {
        if ('6' == response[0] && '2' == response[1]) {
            isFcp = true;
        }
    }

    return isFcp;
}

void RmcSimBaseHandler::sendSimStatusChanged() {
    sp<RfxMclMessage> unsol = RfxMclMessage::obtainUrc(RFX_MSG_URC_RESPONSE_SIM_STATUS_CHANGED,
            m_slot_id, RfxVoidData());
    responseToTelCore(unsol);
}

void RmcSimBaseHandler::handleCdmaCardType(const char *iccid) {
    int cardType = UNKOWN_CARD;
    String8 cdmaCardType("vendor.ril.cdma.card.type");
    cdmaCardType.append(String8::format(".%d", (m_slot_id + 1)));

    bool cdmalockedcard =  getMclStatusManager()->getBoolValue(RFX_STATUS_KEY_CDMA_LOCKED_CARD);

    // Card is locked if ESIMS:0,16 is reported.
    if (cdmalockedcard == true) {
        cardType = LOCKED_CARD;
    } else {
        int eusim = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE);
        // Card is absent if iccid is NULL
        if (iccid == NULL) {
            cardType = CARD_NOT_INSERTED;
        } else {
            if (isOp09Card(iccid)) {
                // OP09 card type
                if (getMclStatusManager()->getBoolValue(RFX_STATUS_KEY_CDMA3G_DUALMODE_CARD)
                        == 1) {
                    // OP09 3G dual mode card.
                    cardType = CT_UIM_SIM_CARD;
                } else {
                    if (eusim < 0) {
                        cardType = UNKOWN_CARD;
                        logE(mTag, "Do not get a valid CDMA card type: %d !", cardType);
                    } else if (eusim == 0) {
                        // Rarely happen, iccid exists but card type is empty.
                        cardType = CARD_NOT_INSERTED;
                    } else if (((eusim & RFX_CARD_TYPE_SIM) == 0) &&
                            ((eusim & RFX_CARD_TYPE_USIM) == 0) &&
                            ((eusim & RFX_CARD_TYPE_ISIM) == 0)) {
                        // OP09 3G single mode card.
                        cardType = CT_3G_UIM_CARD;
                    } else if (((eusim & RFX_CARD_TYPE_CSIM) != 0) &&
                            ((eusim & RFX_CARD_TYPE_USIM) != 0)) {
                        // Typical OP09 4G dual mode card.
                        cardType = CT_4G_UICC_CARD;
                    } else if (eusim == RFX_CARD_TYPE_SIM) {
                        // For non-C2K supported project, CT 3G card type is reported as
                        // "SIM" card and "+ECT3G" value is false.
                        cardType = CT_UIM_SIM_CARD;
                    } else if (((eusim & RFX_CARD_TYPE_USIM) != 0) &&
                            (strStartsWith(iccid, "8985231"))) {
                        // OP09 CTEXCEL card.
                        cardType = CT_EXCEL_GG_CARD;
                    } else if ((eusim == RFX_CARD_TYPE_USIM) ||
                            (eusim == (RFX_CARD_TYPE_USIM | RFX_CARD_TYPE_ISIM))) {
                        if (!RatConfig_isC2kSupported()) {
                            // CT 4G card
                            cardType = CT_4G_UICC_CARD;
                        } else {
                            // SIM card.
                            cardType = SIM_CARD;
                        }
                    } else {
                        cardType = UNKOWN_CARD;
                        logE(mTag, "Do not get a valid CDMA card type: %d !", cardType);
                    }
                }
            } else {
                // Non-OP09 card type.
                if (getMclStatusManager()->getBoolValue(RFX_STATUS_KEY_CDMA3G_DUALMODE_CARD)
                        == 1) {
                    // Non-OP09 CDMA 3G dual mode card.
                    cardType = UIM_SIM_CARD;
                } else {
                    int eusim = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE);

                    if (eusim < 0) {
                        cardType = UNKOWN_CARD;
                        logE(mTag, "Do not get a valid CDMA card type: %d !", cardType);
                    } else if (eusim == 0) {
                        // Rarely happen, iccid exists but card type is empty.
                        cardType = CARD_NOT_INSERTED;
                    } else if (((eusim & RFX_CARD_TYPE_SIM) == 0) &&
                            ((eusim & RFX_CARD_TYPE_USIM) == 0) &&
                            ((eusim & RFX_CARD_TYPE_ISIM) == 0)) {
                        // Non-OP09 3G single mode card.
                        cardType = UIM_CARD;
                    } else if (((eusim & RFX_CARD_TYPE_CSIM) != 0) &&
                            ((eusim & RFX_CARD_TYPE_USIM) != 0)) {
                        if (strStartsWith(iccid, "898601")) {
                            cardType = CT_4G_UICC_CARD;
                        } else {
                            // Typical non-OP09 4G dual mode card.
                            cardType = NOT_CT_UICC_CARD;
                        }
                        // Typical non-OP09 4G dual mode card.
                        cardType = NOT_CT_UICC_CARD;
                    } else if (eusim == RFX_CARD_TYPE_SIM) {
                        // SIM card.
                        cardType = SIM_CARD;
                    } else if ((eusim == RFX_CARD_TYPE_USIM) ||
                            (eusim == (RFX_CARD_TYPE_USIM | RFX_CARD_TYPE_ISIM))) {
                        if (!RatConfig_isC2kSupported() && strStartsWith(iccid, "898601")) {
                            // Wait Gsm +ESIMAPP: reported and set CDMA card type
                            // according to gsm mccmnc.
                            return;
                        } else {
                            // SIM card.
                            cardType = SIM_CARD;
                        }
                    } else {
                        cardType = UNKOWN_CARD;
                        logE(mTag, "Do not get a valid CDMA card type: %d !", cardType);
                    }
                }
            }
        }
    }

    // Set cdma card type.
    rfx_property_set(cdmaCardType, String8::format("%d", cardType).string());
    getMclStatusManager()->setIntValue(RFX_STATUS_KEY_CDMA_CARD_TYPE, cardType);

    if (isAOSPPropSupport()) {
        String8 simType("gsm.sim");
        simType.append(String8::format("%d%s", (m_slot_id + 1), ".type"));
        int aospType = -1;

        if (cardType ==  SIM_CARD) {
            if (getMclStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE)
                    == RFX_CARD_TYPE_SIM) {
                aospType = SINGLE_MODE_SIM_CARD;
            } else if (getMclStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE)
                    == RFX_CARD_TYPE_USIM) {
                aospType = SINGLE_MODE_USIM_CARD;
            }
        } else if ((cardType ==  UIM_CARD) || (cardType ==  CT_3G_UIM_CARD)) {
            aospType = SINGLE_MODE_RUIM_CARD;
        } else if ((cardType ==  UIM_SIM_CARD) || (cardType ==  CT_UIM_SIM_CARD)) {
            if (getMclStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE)
                    == RFX_CARD_TYPE_SIM) {
                aospType = SINGLE_MODE_SIM_CARD;
            } else if (getMclStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE)
                    == RFX_CARD_TYPE_USIM) {
                aospType = SINGLE_MODE_USIM_CARD;
            } else {
                aospType = SINGLE_MODE_RUIM_CARD;
            }
        } else if (cardType ==  CT_EXCEL_GG_CARD) {
            aospType = CT_NATIONAL_ROAMING_CARD;
        }

        rfx_property_set(simType, String8::format("%d", aospType).string());
        logD(mTag, "handleCdmaCardType aospType : %d !", aospType);
    }
}

bool RmcSimBaseHandler::isOp09Card(const char *iccid) {
    bool isOp09Card = false;

    // Compare with OP09 iccid prefix.
    if ((strStartsWith(iccid, "898603")) ||
            (strStartsWith(iccid, "898611")) ||
            (strStartsWith(iccid, "8985302")) ||
            (strStartsWith(iccid, "8985307")) ||
            (strStartsWith(iccid, "8985231")) ||
            (strStartsWith(iccid, "898120"))) {
        isOp09Card = true;
    }
    return isOp09Card;
}

int RmcSimBaseHandler::isApplicationIdExist(const char *aid) {
    int isAidExist = -1;
    sp<RfxAtResponse> p_response;
    int err;
    char *atr;
    RfxAtLine *line;

    // AT+CUAD[=<option>]
    // <option>: integer type.
    // 0:   no parameters requested in addition to <response>.
    // 1    include <active_application>.
    // +CUAD: <response>[,<active_application>[,<AID>]]
    // <response>: string type in hexadecimal character format.The response is the content of the
    // EFDIR.
    // <active_application>: integer type.
    // 0    no SIM or USIM active.
    // 1    active application is SIM.
    // 2    active application is USIM, followed by <AID>.
    // <AID>: string type in hexadecimal character format. AID of active USIM.

    p_response = atSendCommandSingleline("AT+CUAD=0", "+CUAD:");

    if (p_response != NULL) {
        err = p_response->getError();
    } else {
        goto error;
    }

    if (err < 0 || p_response->getSuccess() == 0) {
        goto error;
    }

    line = p_response->getIntermediates();
    line->atTokStart(&err);
    if (err < 0) goto error;

    atr = line->atTokNextstr(&err);
    if (err < 0) goto error;

    if (strstr(atr, aid) != NULL) {
        isAidExist = 1;
    } else {
        isAidExist = 0;
    }

    logD(mTag, "isApplicationIdExist isAidExist: %d", isAidExist);
    return isAidExist;

error:
    logE(mTag, "isApplicationIdExist, fail to parse response!");

    return isAidExist;
}

void RmcSimBaseHandler::makeSimRspFromUsimFcp(unsigned char ** simResponse) {
    int format_wrong = 0;
    unsigned char * fcpByte = NULL;
    unsigned short  fcpLen = 0;
    usim_file_descriptor_struct fDescriptor = {0, 0, 0, 0};
    usim_file_size_struct fSize  = {0};
    unsigned char simRspByte[GET_RESPONSE_EF_SIZE_BYTES] = {0};

    fcpLen = hexStringToByteArray(*simResponse, &fcpByte);

    if (FALSE == usim_fcp_query_tag(fcpByte, fcpLen, FCP_FILE_DES_T, &fDescriptor)) {
        logE(mTag, "USIM FD Parse fail:%s", *simResponse);
        format_wrong = 1;
        goto done;
    }

    if ((!IS_DF_ADF(fDescriptor.fd)) && (FALSE == usim_fcp_query_tag(fcpByte, fcpLen,
            FCP_FILE_SIZE_T, &fSize))) {
        logE(mTag, "USIM File Size fail:%s", *simResponse);
        format_wrong = 1;
        goto done;
    }

    if (IS_DF_ADF(fDescriptor.fd)) {
        simRspByte[RESPONSE_DATA_FILE_TYPE] = TYPE_DF;
        goto done;
    } else {
        simRspByte[RESPONSE_DATA_FILE_TYPE] = TYPE_EF;
    }

    simRspByte[RESPONSE_DATA_FILE_SIZE_1] = (fSize.file_size & 0xFF00) >> 8;
    simRspByte[RESPONSE_DATA_FILE_SIZE_2] = fSize.file_size & 0xFF;

    if (IS_LINEAR_FIXED_EF(fDescriptor.fd)) {
        simRspByte[RESPONSE_DATA_STRUCTURE] = EF_TYPE_LINEAR_FIXED;
        simRspByte[RESPONSE_DATA_RECORD_LENGTH] = fDescriptor.rec_len;
    } else if (IS_TRANSPARENT_EF(fDescriptor.fd)) {
        simRspByte[RESPONSE_DATA_STRUCTURE] = EF_TYPE_TRANSPARENT;

    } else if (IS_CYCLIC_EF(fDescriptor.fd)) {
        simRspByte[RESPONSE_DATA_STRUCTURE] = EF_TYPE_CYCLIC;
        simRspByte[RESPONSE_DATA_RECORD_LENGTH] = fDescriptor.rec_len;
    }


done:
    free(*simResponse);
    free(fcpByte);
    if (format_wrong != 1) {
        *simResponse = byteArrayToHexString(simRspByte, GET_RESPONSE_EF_SIZE_BYTES);
        if (RfxRilUtils::isEngLoad()) {
            logD(mTag, "simRsp done:%s", *simResponse);
        }
    } else {
        *simResponse = NULL;
        logE(mTag, "simRsp done, but simRsp is null because command format may be wrong");
    }
}

void RmcSimBaseHandler::resetSimPropertyAndStatusKey() {
    // reset CDMA systemProperty and statusKey
    String8 cdmaMccMnc("vendor.cdma.ril.uicc.mccmnc");
    cdmaMccMnc.append((m_slot_id == 0)? "" : String8::format(".%d", m_slot_id));
    rfx_property_set(cdmaMccMnc.string(), "");
    getMclStatusManager()->setString8Value(RFX_STATUS_KEY_UICC_CDMA_NUMERIC, String8(""));

    String8 cdmaSubscriberId("vendor.ril.uim.subscriberid");
    cdmaSubscriberId.append(String8::format(".%d", (m_slot_id + 1)));
    rfx_property_set(cdmaSubscriberId.string(), "");
    getMclStatusManager()->setString8Value(RFX_STATUS_KEY_C2K_IMSI, String8(""));

    getMclStatusManager()->setString8Value(RFX_STATUS_KEY_CDMA_SPN, String8(""));
    getMclStatusManager()->setIntValue(RFX_STATUS_KEY_UICC_SUB_CHANGED_STATUS, -1);

    // reset GSM systemProperty and statusKey
    String8 pin1("vendor.gsm.sim.retry.pin1");
    String8 pin2("vendor.gsm.sim.retry.pin2");
    String8 puk1("vendor.gsm.sim.retry.puk1");
    String8 puk2("vendor.gsm.sim.retry.puk2");
    pin1.append((m_slot_id == 0)? "" : String8::format(".%d", (m_slot_id + 1)));
    pin2.append((m_slot_id == 0)? "" : String8::format(".%d", (m_slot_id + 1)));
    puk1.append((m_slot_id == 0)? "" : String8::format(".%d", (m_slot_id + 1)));
    puk2.append((m_slot_id == 0)? "" : String8::format(".%d", (m_slot_id + 1)));
    rfx_property_set(pin1.string(), "");
    rfx_property_set(pin2.string(), "");
    rfx_property_set(puk1.string(), "");
    rfx_property_set(puk2.string(), "");

    String8 gsmMccMnc("vendor.gsm.ril.uicc.mccmnc");
    gsmMccMnc.append((m_slot_id == 0)? "" : String8::format(".%d", m_slot_id));
    rfx_property_set(gsmMccMnc.string(), "");
    getMclStatusManager()->setString8Value(RFX_STATUS_KEY_UICC_GSM_NUMERIC,
            String8(""));

    getMclStatusManager()->setString8Value(RFX_STATUS_KEY_GSM_IMSI, String8(""));
    getMclStatusManager()->setString8Value(RFX_STATUS_KEY_GSM_SPN, String8(""));
    getMclStatusManager()->setString8Value(RFX_STATUS_KEY_GSM_GID1, String8(""));
    getMclStatusManager()->setString8Value(RFX_STATUS_KEY_GSM_PNN, String8(""));
    getMclStatusManager()->setString8Value(RFX_STATUS_KEY_GSM_CACHE_FCP, String8(""));
    getMclStatusManager()->setString8Value(RFX_STATUS_KEY_GSM_CACHE_BINARY, String8(""));

    if (isAOSPPropSupport()) {
        String8 iccidPre("gsm.sim.preiccid_");
        String8 lockCount("gsm.sim");
        String8 aospPin;
        String8 aospPin2;
        String8 aospPuk;
        String8 aospPuk2;
        String8 simType("gsm.sim");
        String8 mlplMspl("ril.csim.mlpl_mspl_ver");

        mlplMspl.append(String8::format("%d", m_slot_id));
        iccidPre.append(String8::format("%d", m_slot_id));
        lockCount.append((m_slot_id == 0)? ".num.simlock" : String8::format("%d%s", m_slot_id,
                ".num.simlock"));
        simType.append(String8::format("%d%s", (m_slot_id + 1), ".type"));
        if (RfxRilUtils::rfxGetSimCount() >= 2) {
            String8 slot("gsm.slot");
            slot.append(String8::format("%d", (m_slot_id+1)));
            aospPin = String8::format("%s%s", slot.string(), ".num.pin1");
            aospPin2 = String8::format("%s%s", slot.string(), ".num.pin2");
            aospPuk = String8::format("%s%s", slot.string(), ".num.puk1");
            aospPuk2 = String8::format("%s%s", slot.string(), ".num.puk2");
        } else {
            aospPin = String8::format("%s", "gsm.sim.num.pin");
            aospPin2 = String8::format("%s", "gsm.sim.num.pin2");
            aospPuk = String8::format("%s", "gsm.sim.num.puk");
            aospPuk2 = String8::format("%s", "gsm.sim.num.puk2");
        }

        rfx_property_set(mlplMspl, ",");
        rfx_property_set(iccidPre, "");
        rfx_property_set(lockCount, "");
        rfx_property_set(simType, "");
        rfx_property_set(aospPin, "");
        rfx_property_set(aospPin2, "");
        rfx_property_set(aospPuk, "");
        rfx_property_set(aospPuk2, "");
    }
}

RIL_SimSlotStatus** RmcSimBaseHandler::decodeEslotsinfo(RfxAtLine *line, int *decodeerr,
        int *slotnum) {
    int err = -1;
    int slotNum = 0;
    RIL_SimSlotStatus *p_slot_status = NULL;
    RIL_SimSlotStatus **pp_slot_status = NULL;
    *decodeerr = -1;

    if (line == NULL) {
        goto error;
    }

    line->atTokStart(&err);
    if (err < 0) goto error;

    slotNum = line->atTokNextint(&err);
    if (err < 0) goto error;
    logD(mTag, "decodeEslotsinfo slotNum %d, RfxRilUtils::rfxGetSimCount() = %d", slotNum,
            RfxRilUtils::rfxGetSimCount());

    // Just report slot info with valid logical slotId.
    // TODO: it should be udpated if AOSP changes and needs to reports all of slot infos.
    if (slotNum > RfxRilUtils::rfxGetSimCount()) {
        slotNum = RfxRilUtils::rfxGetSimCount();
    }

    pp_slot_status = (RIL_SimSlotStatus **)calloc(slotNum, sizeof(RIL_SimSlotStatus *));
    RFX_ASSERT(pp_slot_status != NULL);
    p_slot_status = (RIL_SimSlotStatus *)calloc(slotNum, sizeof(RIL_SimSlotStatus));
    RFX_ASSERT(p_slot_status != NULL);
    memset(p_slot_status, 0, slotNum * sizeof(RIL_SimSlotStatus));

    /* init the pointer array */
    for (int i = 0; i < slotNum; i++) {
        pp_slot_status[i] = &(p_slot_status[i]);
    }

    for (int i = 0; i < slotNum; i++) {
        char *pinState = NULL;
        pinState = line->atTokNextstr(&err);
        if (err < 0) goto error;

        if (strstr(pinState, "+CME ERROR: 14") != NULL || strstr(pinState, "+CME ERROR:14") != NULL) {
            *decodeerr = RIL_E_SIM_ERR;
        }

        p_slot_status[i].slotState = (RIL_SlotState)(atoi(line->atTokNextstr(&err)));
        if (err < 0) goto error;
        p_slot_status[i].logicalSlotId = atoi(line->atTokNextstr(&err));
        if (err < 0) goto error;
        p_slot_status[i].atr = line->atTokNextstr(&err);
        if (err < 0) goto error;

        p_slot_status[i].eid = line->atTokNextstr(&err);
        if (err < 0) goto error;
        if (!((p_slot_status[i].eid == NULL) || (strlen(p_slot_status[i].eid) == 0))) {
            getMclStatusManager(i)->setBoolValue(RFX_STATUS_KEY_IS_EUICC_CARD, true);
            getMclStatusManager(i)->setString8Value(RFX_STATUS_KEY_SIM_EID,
                    String8::format("%s", p_slot_status[i].eid));
        } else {
            getMclStatusManager(i)->setBoolValue(RFX_STATUS_KEY_IS_EUICC_CARD, false);
            getMclStatusManager(i)->setString8Value(RFX_STATUS_KEY_SIM_EID, String8("N/A"));
        }

        p_slot_status[i].iccId = line->atTokNextstr(&err);
        if (err < 0) goto error;

        if (strstr(pinState, "+CPIN: CARD_RESTRICTED") != NULL) {
            p_slot_status[i].card_state = RIL_CARDSTATE_RESTRICTED;
        } else if (!((p_slot_status[i].eid == NULL) || (strlen(p_slot_status[i].eid) == 0))
                || !((p_slot_status[i].iccId == NULL) || (strlen(p_slot_status[i].iccId) == 0))) {
            p_slot_status[i].card_state = RIL_CARDSTATE_PRESENT;
        } else {
            p_slot_status[i].card_state = RIL_CARDSTATE_ABSENT;
        }
    }


    if (*decodeerr != RIL_E_SIM_ERR) {
        *decodeerr = RIL_E_SUCCESS;
    }

error:
    *slotnum = slotNum;
    logD(mTag, "decodeEslotsinfo *decodeerr:%d, *slotnum:%d", *decodeerr, *slotnum);
    return pp_slot_status;
}

