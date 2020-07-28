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
#include "RfxIntsData.h"
#include "RmcSimBaseHandler.h"
#include "RmcCdmaSimUrcHandler.h"
#include "RfxMessageId.h"
#include "rfx_properties.h"
#include <telephony/mtk_ril.h>
#include <string.h>

/*****************************************************************************
 * Variable
 *****************************************************************************/
using ::android::String8;

static const char* cdmaUrcList[] = {
    "+EUSIM:",
    "+ECT3G:",
    "+ECSIMP:",
    "+ESIMS:",
    "+ESIMAPP:",
    "+ESCG:",
    "+ESIMIND:",
};

// Register unsolicited message
RFX_REGISTER_DATA_TO_URC_ID(RfxIntsData, RFX_MSG_URC_UICC_SUBSCRIPTION_STATUS_CHANGED);

/*****************************************************************************
 * Class RmcCdmaSimUrcHandler
 *****************************************************************************/
RmcCdmaSimUrcHandler::RmcCdmaSimUrcHandler(int slot_id, int channel_id) :
        RmcSimBaseHandler(slot_id, channel_id) {
    setTag(String8("RmcCdmaSimUrc"));
}

RmcCdmaSimUrcHandler::~RmcCdmaSimUrcHandler() {
}

const char** RmcCdmaSimUrcHandler::queryUrcTable(int *record_num) {
    const char** p = cdmaUrcList;
    *record_num = sizeof(cdmaUrcList)/sizeof(char*);
    return p;
}

RmcSimBaseHandler::SIM_HANDLE_RESULT RmcCdmaSimUrcHandler::needHandle(
        const sp<RfxMclMessage>& msg) {
    RmcSimBaseHandler::SIM_HANDLE_RESULT result = RmcSimBaseHandler::RESULT_IGNORE;
    String8 ss(msg->getRawUrc()->getLine());
    RfxAtLine *urc = NULL;

    for (unsigned int i = 0; i < (sizeof(cdmaUrcList)/sizeof(char *)); i++) {
        if (ss.find(cdmaUrcList[i]) == 0) {
            if (strcmp(cdmaUrcList[i], "+ESIMAPP:") == 0) {
                urc = new RfxAtLine(ss, NULL);
                int err = 0;
                int appType = -1;

                urc->atTokStart(&err);
                if (err < 0) {
                    break;
                }

                appType = urc->atTokNextint(&err);
                if (err < 0) {
                    break;
                }

                // Only handle CSIM and RUIM
                if (appType == UICC_APP_CSIM || appType == UICC_APP_RUIM) {
                    result = RmcSimBaseHandler::RESULT_NEED;
                }
            } else if (strcmp(cdmaUrcList[i], "+ESIMIND:") == 0) {
                urc = new RfxAtLine(ss, NULL);
                int err = 0;
                int appType = -1;

                urc->atTokStart(&err);
                if (err < 0) {
                    break;
                }

                // Skip event id.
                urc->atTokNextint(&err);
                if (err < 0) {
                    break;
                }

                appType = urc->atTokNextint(&err);
                if (err < 0) {
                    break;
                }

                // Only handle CSIM and RUIM
                if ((appType == UICC_APP_CSIM) || (appType == UICC_APP_RUIM)) {
                    result = RmcSimBaseHandler::RESULT_NEED;
                }
            } else {
                result = RmcSimBaseHandler::RESULT_NEED;
            }
            break;
        }
    }

    if (urc != NULL) {
        delete(urc);
    }
    return result;
}

void RmcCdmaSimUrcHandler::handleUrc(const sp<RfxMclMessage>& msg, RfxAtLine *urc) {
    String8 ss(urc->getLine());
    RfxAtLine *cdmaUrc = new RfxAtLine(msg->getRawUrc()->getLine(), NULL);

    if (ss.find("+EUSIM:") == 0) {
        handleCardType(msg, urc);
    } else if (ss.find("+ECT3G:") == 0) {
        handleCdma3gDualmodeValue(msg, urc);
    } else if (ss.find("+ECSIMP:") == 0) {
        handleUiccSubscriptionStatus(msg, urc);
    } else if (ss.find("+ESIMS:") == 0) {
        if (ss.find("+ESIMS: 0,16") == 0) {
            getMclStatusManager()->setBoolValue(RFX_STATUS_KEY_CDMA_LOCKED_CARD, true);
        } else {
            handleSimStateChanged(msg, urc);
        }
    } else if (ss.find("+ESIMAPP:") == 0) {
        handCmdaMccMnc(msg, urc);
    } else if (ss.find("+ESCG:") == 0) {
        handCdma3GSwitchCard(msg, urc);
    } else if (ss.find("+ESIMIND:") == 0) {
        handleSimIndication(msg, urc);
    } else {
        logE(mTag, "Can not handle: %s", msg->getRawUrc()->getLine());
    }

    if (cdmaUrc != NULL) {
        delete(cdmaUrc);
    }
}

void RmcCdmaSimUrcHandler::handleCardType(const sp<RfxMclMessage>& msg, RfxAtLine *urc) {
    RFX_UNUSED(msg);
    RFX_UNUSED(urc);
    int cardType = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE);
    if ((cardType & (RFX_CARD_TYPE_RUIM | RFX_CARD_TYPE_CSIM)) == 0) {
        resetCDMASimState();
    }
}

void RmcCdmaSimUrcHandler::handleCdma3gDualmodeValue(const sp<RfxMclMessage>& msg, RfxAtLine *urc) {
    RfxAtLine *atLine = urc;
    int err = 0, value = -1;
    bool result = false;
    String8 cdma3GDualMode("vendor.gsm.ril.ct3g");
    cdma3GDualMode.append((m_slot_id == 0)? "" : String8::format(".%d", (m_slot_id + 1)));

    do {
        atLine->atTokStart(&err);
        if (err < 0) {
            break;
        }

        value = atLine->atTokNextint(&err);
        if (err < 0) {
            break;
        }

        if (value == 1) {
            getMclStatusManager()->setBoolValue(RFX_STATUS_KEY_CDMA3G_DUALMODE_CARD, true);
            rfx_property_set(cdma3GDualMode, String8::format("%d", value).string());
        } else {
            getMclStatusManager()->setBoolValue(RFX_STATUS_KEY_CDMA3G_DUALMODE_CARD, false);
            rfx_property_set(cdma3GDualMode, String8::format("%d", 0).string());
        }

        result = true;
    } while (0);

    if (!result) {
        logE(mTag, "handleCdma3gDualmodeValue fail: %s", msg->getRawUrc()->getLine());
    }
}

void RmcCdmaSimUrcHandler::handleUiccSubscriptionStatus(const sp<RfxMclMessage>& msg, RfxAtLine *urc) {
    RfxAtLine *atLine = urc;
    int err = 0, activate = -1;
    bool result = false;

    do {
        atLine->atTokStart(&err);
        if (err < 0) {
            break;
        }

        activate = atLine->atTokNextint(&err);
        if (err < 0) {
            break;
        }

        getMclStatusManager()->setIntValue(RFX_STATUS_KEY_UICC_SUB_CHANGED_STATUS, activate);

        sp<RfxMclMessage> unsol = RfxMclMessage::obtainUrc(RFX_MSG_URC_UICC_SUBSCRIPTION_STATUS_CHANGED,
                m_slot_id, RfxIntsData((void*)&activate, sizeof(int)));
        responseToTelCore(unsol);

        result = true;
    } while (0);

    if (!result) {
        logE(mTag, "handleUiccSubscriptionStatus fail: %s", msg->getRawUrc()->getLine());
    }
}

void RmcCdmaSimUrcHandler::handCmdaMccMnc(const sp<RfxMclMessage>& msg, RfxAtLine *urc) {
    int appTypeId = -1, channelId = -1, err = 0;
    char *pMcc = NULL, *pMnc = NULL;
    RfxAtLine *atLine = urc;
    String8 numeric("");
    bool result = false;
    String8 cdmaMccMnc("vendor.cdma.ril.uicc.mccmnc");
    cdmaMccMnc.append((m_slot_id == 0)? "" : String8::format(".%d", m_slot_id));

    do {
        atLine->atTokStart(&err);
        if (err < 0) {
            break;
        }

        appTypeId = atLine->atTokNextint(&err);
        if (err < 0) {
            break;
        }
        // TODO: Check apptype if to continue handle imsi.

        channelId = atLine->atTokNextint(&err);
        if (err < 0) {
            break;
        }

        pMcc = atLine->atTokNextstr(&err);
        if (err < 0) {
            break;
        }

        pMnc = atLine->atTokNextstr(&err);
        if (err < 0) {
            break;
        }

        numeric.append(String8::format("%s%s", pMcc, pMnc));

        rfx_property_set(cdmaMccMnc, numeric.string());
        getMclStatusManager()->setString8Value(RFX_STATUS_KEY_UICC_CDMA_NUMERIC, numeric);

        result = true;
    } while (0);

    if (!result) {
        logE(mTag, "handCmdaMccMnc fail: %s", msg->getRawUrc()->getLine());
    }

    int cardType = -1;
    int eusim = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE);
    if (((eusim & RFX_CARD_TYPE_CSIM) != 0) && ((eusim & RFX_CARD_TYPE_USIM) != 0)) {

        if (!numeric.isEmpty()) {
            if ((0 == strcmp(numeric.string(), "46003"))
                    || (0 == strcmp(numeric.string(), "45502"))
                    || (0 == strcmp(numeric.string(), "46012"))) {
                cardType = CT_4G_UICC_CARD;
            } else {
                cardType = NOT_CT_UICC_CARD;
            }
        } else {
            String8 iccid = getMclStatusManager()->getString8Value(RFX_STATUS_KEY_SIM_ICCID);
            if (!iccid.isEmpty()) {
                if (isOp09Card(iccid.string())) {
                    cardType = CT_4G_UICC_CARD;
                } else {
                    cardType = NOT_CT_UICC_CARD;
                }
            }
        }

        if (cardType != -1) {
            String8 cdmaCardType("vendor.ril.cdma.card.type");
            cdmaCardType.append(String8::format(".%d", (m_slot_id + 1)));
            // Set cdma card type.
            rfx_property_set(cdmaCardType, String8::format("%d", cardType).string());
            getMclStatusManager()->setIntValue(RFX_STATUS_KEY_CDMA_CARD_TYPE, cardType);
        }
    }

    if (isAOSPPropSupport()) {
        String8 simType("gsm.sim");
        simType.append(String8::format("%d%s", (m_slot_id + 1), ".type"));
        int aospType = -1;

        if (cardType == CT_4G_UICC_CARD) {
            aospType = DUAL_MODE_TELECOM_LTE_CARD;
        } else if (cardType == NOT_CT_UICC_CARD) {
            aospType = DUAL_MODE_CG_CARD;
        }

        if (aospType != -1) {
            rfx_property_set(simType, String8::format("%d", aospType).string());
            logD(mTag, "handCmdaMccMnc aospType : %d !", aospType);
        }
    }
}

void RmcCdmaSimUrcHandler::handCdma3GSwitchCard(const sp<RfxMclMessage>& msg, RfxAtLine *urc) {
    RFX_UNUSED(msg);
    String8 ss(urc->getLine());
    int switchcard = -1;

    if (ss.find("+ESCG: 3,1") == 0) {
        switchcard = AP_TRIGGER_SWITCH_SIM;
    } else if (ss.find("+ESCG: 3,2") == 0) {
        switchcard = GMSS_TRIGGER_SWITCH_SIM;
    } else if (ss.find("+ESCG: 4,1") == 0) {
        switchcard = AP_TRIGGER_SWITCH_RUIM;
    } else if (ss.find("+ESCG: 4,2") == 0) {
        switchcard = GMSS_TRIGGER_SWITCH_RUIM;
    } else {
        switchcard = -1;
    }

    getMclStatusManager()->setIntValue(RFX_STATUS_KEY_CDMA3G_SWITCH_CARD, switchcard);
}

void RmcCdmaSimUrcHandler::handleSimStateChanged(const sp<RfxMclMessage>& msg, RfxAtLine *urc) {
    RFX_UNUSED(msg);
    // +ESIMS: 0,0: SIM Missing
    // +ESIMS: 0,13: Recovery start
    // +ESIMS: 0,10: Virtual SIM off
    // +ESIMS: 0,11: SIM plug out
    // +ESIMS: 0.15: IMEI Lock
    // +ESIMS: 1,9: Virtual SIM on
    // +ESIMS: 1,14: Recovery end
    // +ESIMS: 1,12: SIM plug in
    String8 ss(urc->getLine());
    String8 cdma3GDualMode("vendor.gsm.ril.ct3g");
    String8 cdmaCardType("vendor.ril.cdma.card.type");

    cdma3GDualMode.append((m_slot_id == 0)? "" : String8::format(".%d", (m_slot_id + 1)));
    cdmaCardType.append(String8::format(".%d", (m_slot_id + 1)));

    if ((ss.find("+ESIMS: 0,11") == 0) || (ss.find("+ESIMS: 0,13") == 0)) {
        resetCDMASimState();

        getMclStatusManager()->setIntValue(RFX_STATUS_KEY_CDMA_CARD_TYPE, CARD_NOT_INSERTED);
        rfx_property_set(cdmaCardType, String8::format("%d", CARD_NOT_INSERTED).string());

        getMclStatusManager()->setBoolValue(RFX_STATUS_KEY_CDMA3G_DUALMODE_CARD, false);
        rfx_property_set(cdma3GDualMode.string(), "");

        getMclStatusManager()->setIntValue(RFX_STATUS_KEY_CDMA3G_SWITCH_CARD, -1);
        getMclStatusManager()->setBoolValue(RFX_STATUS_KEY_CDMA_LOCKED_CARD, false);
        if (isOP09AProject() && (m_slot_id == 1) &&
                (getMclStatusManager()->getIntValue(RFX_STATUS_KEY_ESIMIND_APPLIST) >= 0)) {
            getMclStatusManager()->setIntValue(RFX_STATUS_KEY_ESIMIND_APPLIST,
                    RFX_UICC_APPLIST_NONE);
        }
    }
}

void RmcCdmaSimUrcHandler::resetCDMASimState() {
    logD(mTag, "resetCDMASimState");
    String8 cdmaMccMnc("vendor.cdma.ril.uicc.mccmnc");
    String8 cdmaSubscriberId("vendor.ril.uim.subscriberid");
    String8 tempString8Value;

    cdmaMccMnc.append((m_slot_id == 0)? "" : String8::format(".%d", m_slot_id));
    cdmaSubscriberId.append(String8::format(".%d", (m_slot_id + 1)));

    tempString8Value = getMclStatusManager()->getString8Value(RFX_STATUS_KEY_UICC_CDMA_NUMERIC);
    if (!tempString8Value.isEmpty()) {
        rfx_property_set(cdmaMccMnc.string(), "");
        getMclStatusManager()->setString8Value(RFX_STATUS_KEY_UICC_CDMA_NUMERIC, String8(""));
    }

    tempString8Value = getMclStatusManager()->getString8Value(RFX_STATUS_KEY_C2K_IMSI);
    if (!tempString8Value.isEmpty()) {
        rfx_property_set(cdmaSubscriberId.string(), "");
        getMclStatusManager()->setString8Value(RFX_STATUS_KEY_C2K_IMSI, String8(""));
    }

    if (getMclStatusManager()->getIntValue(RFX_STATUS_KEY_UICC_SUB_CHANGED_STATUS, -1) != -1) {
        getMclStatusManager()->setIntValue(RFX_STATUS_KEY_UICC_SUB_CHANGED_STATUS, -1);
    }
}

void RmcCdmaSimUrcHandler::handleSimIndication(const sp<RfxMclMessage>& msg, RfxAtLine *urc) {
    RFX_UNUSED(msg);
    RfxAtLine *atLine = urc;
    int err = 0, indEvent = -1;
    int appId = 0;
    char *rawData = NULL;

    String8 cdmaSubscriberId("vendor.ril.uim.subscriberid");
    cdmaSubscriberId.append(String8::format(".%d", (m_slot_id + 1)));

    do {
        atLine->atTokStart(&err);
        if (err < 0) {
            break;
        }

        // +ESIMIND: <event id>, <app id> , <SPN>, <Full IMSI>, <GID1>, <PNN full name TLV>
        // CDMA only reports SPN and IMSI context.
        indEvent = atLine->atTokNextint(&err);
        if (err < 0) {
            break;
        }

        if (indEvent == 3) {
            appId = atLine->atTokNextint(&err);
            if (err < 0) {
                break;
            }

            rawData = atLine->atTokNextstr(&err);
            if (err < 0) {
                break;
            }

            getMclStatusManager()->setString8Value(RFX_STATUS_KEY_CDMA_SPN,
                    String8::format("%s", rawData));

            rawData = NULL;
            rawData = atLine->atTokNextstr(&err);
            if (err < 0) {
                break;
            }
            rfx_property_set(cdmaSubscriberId, rawData);
            getMclStatusManager()->setString8Value(RFX_STATUS_KEY_C2K_IMSI,
                    String8::format("%s", rawData));

            return;
        }
    } while (0);

    logE(mTag, "handleSimIndication, Invalid parameters");
}
