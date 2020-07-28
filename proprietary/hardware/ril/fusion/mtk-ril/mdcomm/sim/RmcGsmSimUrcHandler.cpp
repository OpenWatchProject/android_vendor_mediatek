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
#include "RmcSimBaseHandler.h"
#include "RmcGsmSimUrcHandler.h"
#include <stdlib.h>
#include <string>
#include <telephony/mtk_ril.h>
#include "rfx_properties.h"
#include "RfxTokUtils.h"
#include "RmcCommSimDefs.h"

#ifdef __cplusplus
extern "C"
{
#endif
#include "usim_fcp_parser.h"
#ifdef __cplusplus
}
#endif


using ::android::String8;

static const char* gsmUrcList[] = {
    "+ESIMAPP:",
    "+ESIMIND:",
};


/*****************************************************************************
 * Class RfxController
 *****************************************************************************/
RmcGsmSimUrcHandler::RmcGsmSimUrcHandler(int slot_id, int channel_id) :
        RmcSimBaseHandler(slot_id, channel_id){
    setTag(String8("RmcGsmSimUrc"));
}

RmcGsmSimUrcHandler::~RmcGsmSimUrcHandler() {
}

RmcSimBaseHandler::SIM_HANDLE_RESULT RmcGsmSimUrcHandler::needHandle(
        const sp<RfxMclMessage>& msg) {
    RmcSimBaseHandler::SIM_HANDLE_RESULT result = RmcSimBaseHandler::RESULT_IGNORE;
    char* ss = msg->getRawUrc()->getLine();

    if (strStartsWith(ss, "+ESIMAPP:")) {
        RfxAtLine *urc = new RfxAtLine(ss, NULL);
        int err = 0;
        int app_id = -1;

        urc->atTokStart(&err);
        app_id = urc->atTokNextint(&err);
        // Only handle SIM(3) and USIM(1)
        if (app_id == UICC_APP_USIM || app_id == UICC_APP_SIM) {
            result = RmcSimBaseHandler::RESULT_NEED;
        }

        delete(urc);
    } else if (strStartsWith(ss, "+ESIMIND:")) {
        RfxAtLine *urc = new RfxAtLine(ss, NULL);
        int err = 0;
        int app_id = -1;
        int event_id = -1;

        urc->atTokStart(&err);
        event_id = urc->atTokNextint(&err);
        app_id = urc->atTokNextint(&err);
        // Only handle SIM(3) and USIM(1)
        if (app_id == UICC_APP_USIM || app_id == UICC_APP_SIM || app_id == UICC_APP_ISIM) {
            result = RmcSimBaseHandler::RESULT_NEED;
        }

        delete(urc);
    }
    return result;
}

void RmcGsmSimUrcHandler::handleUrc(const sp<RfxMclMessage>& msg, RfxAtLine *urc) {
    String8 ss(urc->getLine());

    if (strStartsWith(ss, "+ESIMAPP:")) {
        handleMccMnc(msg);
    }  else if (strStartsWith(ss, "+ESIMIND:")) {
        handleSimIndication(msg, urc);
    }
}

const char** RmcGsmSimUrcHandler::queryUrcTable(int *record_num) {
    const char **p = gsmUrcList;
    *record_num = sizeof(gsmUrcList)/sizeof(char*);
    return p;
}

void RmcGsmSimUrcHandler::handleMccMnc(const sp<RfxMclMessage>& msg) {
    int appTypeId = -1, channelId = -1, err = 0;
    char *pMcc = NULL, *pMnc = NULL;
    RfxAtLine *atLine = msg->getRawUrc();
    String8 numeric("");
    String8 prop("vendor.gsm.ril.uicc.mccmnc");

    prop.append((m_slot_id == 0)? "" : String8::format(".%d", m_slot_id));

    do {
        atLine->atTokStart(&err);
        if (err < 0) {
            break;
        }

        appTypeId = atLine->atTokNextint(&err);
        if (err < 0) {
            break;
        }

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

        logD(mTag, "numeric: %s", numeric.string());

        rfx_property_set(prop, numeric.string());

        getMclStatusManager()->setString8Value(RFX_STATUS_KEY_UICC_GSM_NUMERIC, numeric);
    } while(0);

    int eusim = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE);
    String8 iccid = getMclStatusManager()->getString8Value(RFX_STATUS_KEY_SIM_ICCID);

    if ((!RatConfig_isC2kSupported()) && (strStartsWith(iccid, "898601"))
            && ((eusim == RFX_CARD_TYPE_USIM) || (eusim == (
            RFX_CARD_TYPE_USIM | RFX_CARD_TYPE_ISIM)))) {
        int cardType = -1;

        if (!numeric.isEmpty()) {
            if (strStartsWith(numeric.string(), "46011") || strStartsWith(numeric.string(), "20404")) {
                cardType = CT_4G_UICC_CARD;
            } else {
                cardType = SIM_CARD;
            }
        }

        if (cardType != -1) {
            // Set cdma card type.
            String8 cdmaCardType("vendor.ril.cdma.card.type");
            cdmaCardType.append(String8::format(".%d", (m_slot_id + 1)));
            rfx_property_set(cdmaCardType, String8::format("%d", cardType).string());
            getMclStatusManager()->setIntValue(RFX_STATUS_KEY_CDMA_CARD_TYPE, cardType);
        }

        int aospType = -1;
        if (isAOSPPropSupport()) {
            String8 simType("gsm.sim");
            simType.append(String8::format("%d%s", (m_slot_id + 1), ".type"));

            if (cardType == CT_4G_UICC_CARD) {
                aospType = DUAL_MODE_TELECOM_LTE_CARD;
            } else if (cardType == SIM_CARD) {
                if (getMclStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE)
                        == RFX_CARD_TYPE_SIM) {
                    aospType = SINGLE_MODE_SIM_CARD;
                } else if (getMclStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE)
                        == RFX_CARD_TYPE_USIM) {
                    aospType = SINGLE_MODE_USIM_CARD;
                }
            }

            if (aospType != -1) {
                rfx_property_set(simType, String8::format("%d", aospType).string());
                logD(mTag, "aospType : %d !", aospType);
            }
        }

        logI(mTag, "handleMccMnc, CDMA card type: %d, aospType: %d", cardType, aospType);
    }
}

int RmcGsmSimUrcHandler::parseSimIndication(RfxStatusKeyEnum key, RfxAtLine* atLine) {
    int err = 0, len = -1;
    char *raw = NULL;

    raw = atLine->atTokNextstr(&err);
    if (err < 0) {
        return len;
    }
    String8 value(raw);
    getMclStatusManager()->setString8Value(key, value);
    len = value.length();
    return len;
}

void RmcGsmSimUrcHandler::handleSimIndication(const sp<RfxMclMessage>& msg, RfxAtLine *urc) {
    int err = 0, indEvent = -1, appId = -1, len = -1, info_index = 0;
    char *raw = NULL;
    RfxAtLine *atLine = urc;
    int applist = 0;
    int fileNum = -1;
    String8 key("");
    char* value = NULL;
    char* res = NULL;
    RFX_UNUSED(msg);

    atLine->atTokStart(&err);
    if (err < 0) {
        goto error;
    }

    indEvent = atLine->atTokNextint(&err);
    if (err < 0) {
        goto error;
    }
    appId = atLine->atTokNextint(&err);
    if (err < 0) {
        goto error;
    }

    switch (indEvent) {
        case 3:
            logD(mTag, "Notify information related to data registration, slotId:%d", m_slot_id);
            // SPN->IMSI->GID1->PNN 1st item
            while ((atLine->atTokHasmore() && info_index < RmcGsmSimUrcHandler::FIELD_END)) {
                if (appId == UICC_APP_USIM || appId == UICC_APP_SIM) {
                    switch (info_index) {
                        case RmcGsmSimUrcHandler::FIELD_SPN:
                            len = parseSimIndication(RFX_STATUS_KEY_GSM_SPN, atLine);
                            if (len <= 0) {
                                logD(mTag, "Can't retrieve SPN, len=%d", len);
                            }
                            break;
                        case RmcGsmSimUrcHandler::FIELD_GID1:
                            len = parseSimIndication(RFX_STATUS_KEY_GSM_GID1, atLine);
                            if (len <= 0) {
                                logD(mTag, "Can't retrieve GID1, len=%d", len);
                            }
                            break;
                        case RmcGsmSimUrcHandler::FIELD_PNN:
                            len = parseSimIndication(RFX_STATUS_KEY_GSM_PNN, atLine);
                            if (len <= 0) {
                                logD(mTag, "Can't retrieve PNN, len=%d", len);
                            }
                            break;
                        case RmcGsmSimUrcHandler::FIELD_IMSI:
                            len = parseSimIndication(RFX_STATUS_KEY_GSM_IMSI, atLine);
                            if (len <= 0) {
                                logD(mTag, "Can't retrieve IMSI, len=%d", len);
                            }
                            break;
                    }
                } else if (appId == UICC_APP_ISIM) {
                    // Only parse IMPI when appId is UICC_APP_ISIM
                    if (info_index == RmcGsmSimUrcHandler::FIELD_IMPI) {
                        len = parseSimIndication(RFX_STATUS_KEY_GSM_IMPI, atLine);
                        if (len <= 0) {
                            logD(mTag, "Can't retrieve IMPI, len=%d", len);
                        }
                    } else {
                        atLine->atTokNextstr(&err);
                        if (err < 0) {
                            goto error;
                        }
                    }
                }
                info_index++;
            }
            break;
        case 4:
            fileNum = atLine->atTokNextint(&err);
            if (err < 0) {
                goto error;
            }
            key.append(String8::format("%d%s%d", appId, ",", fileNum));
            for (int i = 0; (i < fileNum) && (atLine->atTokHasmore()); i++) {
                value = atLine->atTokNextstr(&err);
                if (err < 0) {
                    goto error;
                }

                bool isCPHS = false;
                if ((appId == UICC_APP_USIM) && (strcmp(value, "7F206F14") == 0)) {
                    key.append(String8::format("%s%s", ",", "7FFF6F14"));
                    isCPHS = true;
                } else if ((appId == UICC_APP_USIM) && (strcmp(value, "7F206F18") == 0)) {
                    key.append(String8::format("%s%s", ",", "7FFF6F18"));
                    isCPHS = true;
                } else {
                    key.append(String8::format("%s%s", ",", value));
                }

                value = NULL;
                value = atLine->atTokNextstr(&err);
                if (err < 0) {
                    break;
                }

                if (strlen(value) > 4) {
                    String8 sw(value);
                    key.append(String8::format("%s%s", ",", string(sw.string()).substr(0, 4).c_str()));

                    asprintf(&res, "%s", value + 4);
                    if (appId == UICC_APP_USIM && !isCPHS) {
                        makeSimRspFromUsimFcp((unsigned char**)&res);
                    }
                    key.append(String8::format("%s", res));
                } else {
                    key.append(String8::format("%s%s", ",", value));
                }

                value = NULL;
                if (res != NULL) {
                    free(res);
                    res = NULL;
                }
            }

            getMclStatusManager()->setString8Value(RFX_STATUS_KEY_GSM_CACHE_FCP, key);
            logD(mTag, "handleSimIndication cache fcp: %s", key.string());
            break;
        case 5:
            fileNum = atLine->atTokNextint(&err);
            if (err < 0) {
                goto error;
            }
            key.append(String8::format("%d%s%d", appId, ",", fileNum));
            for (int i = 0; (i < fileNum) && (atLine->atTokHasmore()); i++) {
                value = atLine->atTokNextstr(&err);
                if (err < 0) {
                    goto error;
                }
                key.append(String8::format("%s%s", ",", value));
                value = NULL;
                value = atLine->atTokNextstr(&err);
                if (err < 0) {
                    break;
                }
                key.append(String8::format("%s%s", ",", value));
                value = NULL;
            }

            getMclStatusManager()->setString8Value(RFX_STATUS_KEY_GSM_CACHE_BINARY, key);
            logD(mTag, "handleSimIndication cache binary: %s", key.string());
            break;
        default:
            logD(mTag, "Not support the SIM indication event %d", indEvent);
            break;
    }

    return;
error:
    logE(mTag, "handleSimIndication, Invalid parameters");
}

