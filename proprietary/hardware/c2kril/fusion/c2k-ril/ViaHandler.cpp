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
#include "ViaHandler.h"
#include "RfxLog.h"
#include "RfxBaseHandler.h"
#include "RfxIntsData.h"

/*****************************************************************************
 * Class ViaHandler
 *****************************************************************************/

#undef LOG_TAG
#define LOG_TAG "ViaHandler"

ViaHandler::ViaHandler() : mSystemId(-1) {
}

ViaHandler::~ViaHandler() {
}

void ViaHandler::sendCommandDemo(RfxBaseHandler* handler, char* str) {
    RFX_UNUSED(str);
    sp<RfxAtResponse> p_response;
    RfxAtLine* line = NULL;
    int err = 0;
    char* tmp_str = NULL;

    p_response = handler->atSendCommandSingleline("AT+CCID?", "+CCID:");

    if (p_response == NULL
            || p_response->getError() != 0
            || p_response->getSuccess() == 0
            || p_response->getIntermediates() == NULL) {
        goto error;
    }

    line = p_response->getIntermediates();

    line->atTokStart(&err);
    if (err < 0) {
        goto error;
    }

    tmp_str = line->atTokNextstr(&err);
    if (err < 0) {
        goto error;
    }
    return;

error:
    RFX_LOG_E(LOG_TAG, "[%s] error", __FUNCTION__);
}

void ViaHandler::handleCdmaSubscription(RfxBaseHandler* handler, char **p_response, RIL_Errno *result) {
    const char *mTag = "RmcCdmaSimRequest";
    int err;
    int skip = 0;
    int sidnidCount = 0;
    RfxAtLine *p_cur = NULL;
    RfxAtLine *line_mdn = NULL;
    RfxAtLine *line_sidnid = NULL;
    RfxAtLine *line_min = NULL;
    RfxAtLine *line_prl = NULL;
    sp<RfxAtResponse> p_response_mdn = NULL;
    sp<RfxAtResponse> p_response_sidnid = NULL;
    sp<RfxAtResponse> p_response_min = NULL;
    sp<RfxAtResponse> p_response_prl = NULL;
    *result = RIL_E_SIM_ERR;
    size_t maxsize = 255;

    do {
        // Get mobile direcory number(MDN).
        p_response_mdn = handler->atSendCommandMultiline("AT+CNUM?", "+CNUM:");
        if (p_response_mdn == NULL) {
            break;
        }
        if (p_response_mdn->getSuccess() == 0) {
            err = p_response_mdn->getError();
            if (err < 0) {
                break;
            }
            if (p_response_mdn->atGetCmeError() == CME_SIM_NOT_INSERTED) {
                *result = RIL_E_SIM_ABSENT;
            }
            break;
        }

        /*Although +CNUM? may response multiline groups when read a UICC card,
        we only need to report the first group to FW.*/
        line_mdn = p_response_mdn->getIntermediates();
        if (line_mdn == NULL) {
            break;
        }

        line_mdn->atTokStart(&err);
        if (err < 0) {
            break;
        }

        skip = line_mdn->atTokNextint(&err);
        if (err < 0) {
            break;
        }

        char *cnum = line_mdn->atTokNextstr(&err);
        if (err < 0) {
            break;
        }

        p_response[0] = (char *)calloc(maxsize, sizeof(char));
        if (strlen(cnum) < maxsize) {
            strncat(p_response[0], cnum, strlen(cnum));
            p_response[0][strlen(cnum)] = '\0';
        } else {
            strncat(p_response[0], cnum, maxsize-1);
            p_response[0][maxsize-1] = '\0';
        }

        // Get sid and nid.
        p_response_sidnid = handler->atSendCommandMultiline("AT+CSNID?", "+CSNID:");
        if (p_response_sidnid == NULL) {
            break;
        }
        if (p_response_sidnid->getSuccess() == 0) {
            err = p_response_sidnid->getError();
            if (err < 0) {
                break;
            }
            if (p_response_sidnid->atGetCmeError() == CME_SIM_NOT_INSERTED) {
                *result = RIL_E_SIM_ABSENT;
            }
            break;
        }

        for (sidnidCount = 0, p_cur = p_response_sidnid->getIntermediates(); p_cur != NULL;
                p_cur = p_cur->getNext()) {
            sidnidCount++;
        }
        if (sidnidCount > 0) {
            char **sid = (char **)alloca(sidnidCount * sizeof (char *));
            memset(sid,0,sidnidCount * sizeof (char *));
            if (sid == NULL) {
                break;
            }

            char **nid = (char **)alloca(sidnidCount * sizeof (char *));
            memset(nid,0,sidnidCount * sizeof (char *));
            if (nid == NULL) {
                break;
            }

            int i = 0;
            bool resultOk = true;
            for (p_cur = p_response_sidnid->getIntermediates(); p_cur != NULL;
                    p_cur = p_cur->getNext(), i++) {
                line_sidnid = p_cur;
                line_sidnid->atTokStart(&err);
                if (err < 0) {
                    resultOk = false;
                    break;
                }
                sid[i] = line_sidnid->atTokNextstr(&err);
                if (err < 0) {
                    resultOk = false;
                    break;
                }
                nid[i] = line_sidnid->atTokNextstr(&err);
                if (err < 0) {
                    resultOk = false;
                    break;
                }
            }
            if (!resultOk) {
                break;
            }

            char *finalSid = (char *)calloc((sidnidCount * 20 + sidnidCount), sizeof(char));
            if (finalSid == NULL) {
                break;
            }
            memset(finalSid, 0, (sidnidCount * 20 + sidnidCount) * sizeof(char));

            char *finalNid = (char *)calloc((sidnidCount * 20 + sidnidCount), sizeof(char));
            if (finalNid == NULL) {
                if (finalSid != NULL) {
                    free(finalSid);
                }
                break;
            }
            memset(finalNid, 0, (sidnidCount * 20 + sidnidCount) * sizeof(char));

            for (i = 0; i < sidnidCount; i++) {
                if (strlen(sid[i]) < 20) {
                    strncat(finalSid, sid[i], strlen(sid[i]));
                } else {
                    strncat(finalSid, sid[i], 20);
                }
                strncat(finalSid, ",", 1);
                if (strlen(nid[i]) < 20) {
                    strncat(finalNid, nid[i], strlen(nid[i]));
                } else {
                    strncat(finalNid, nid[i], 20);
                }
                strncat(finalNid, ",", 1);
            }

            if (strlen(finalSid) > 0) {
                finalSid[strlen(finalSid) - 1] = '\0';
            }
            if (strlen(finalNid) > 0) {
                finalNid[strlen(finalNid) - 1] = '\0';
            }

            p_response[1] = finalSid;
            p_response[2] = finalNid;
        }

        // Get mobile identification number(MIN).
        p_response_min = handler->atSendCommandSingleline("AT+VMIN?", "+VMIN:");
        if (p_response_min == NULL) {
            break;
        }
        if (p_response_min->getSuccess() == 0) {
            err = p_response_min->getError();
            if (err < 0) {
                break;
            }
            if (p_response_min->atGetCmeError() == CME_SIM_NOT_INSERTED) {
                *result = RIL_E_SIM_ABSENT;
            }
            break;
        }

        line_min = p_response_min->getIntermediates();
        line_min->atTokStart(&err);
        if (err < 0) {
            break;
        }

        char *vmin = line_min->atTokNextstr(&err);
        if (err < 0) {
            break;
        }

        p_response[3] = (char *)calloc(maxsize, sizeof(char));
        if (strlen(vmin) < maxsize) {
            strncat(p_response[3], vmin, strlen(vmin));
            p_response[3][strlen(vmin)] = '\0';
        } else {
            strncat(p_response[3], vmin, maxsize-1);
            p_response[3][maxsize-1] = '\0';
        }

        // Get PRL id.
        p_response_prl = handler->atSendCommandSingleline("AT+VPRLID?", "+VPRLID:");
        if (p_response_prl == NULL) {
            break;
        }
        if (p_response_prl->getSuccess() == 0) {
            err = p_response_prl->getError();
            if (err < 0) {
                break;
            }
            if (p_response_prl->atGetCmeError() == CME_SIM_NOT_INSERTED) {
                *result = RIL_E_SIM_ABSENT;
            }
            break;
        }

        line_prl = p_response_prl->getIntermediates();
        line_prl->atTokStart(&err);
        if (err < 0) {
            break;
        }

        char *prl = line_prl->atTokNextstr(&err);
        if (err < 0) {
            break;
        }

        p_response[4] = (char *)calloc(maxsize, sizeof(char));
        if (strlen(prl) < maxsize) {
            strncat(p_response[4], prl, strlen(prl));
            p_response[4][strlen(prl)] = '\0';
        } else {
            strncat(p_response[4], prl, maxsize-1);
            p_response[4][maxsize-1] = '\0';
        }

        RFX_LOG_D(mTag, "handleCdmaSubscription CNUM: %.8s, Sid: %s, Nid: %s, VIMIN: %s, VPRLID: %s",
                p_response[0], p_response[1], p_response[2], p_response[3], p_response[4]);

        *result = RIL_E_SUCCESS;
    } while (0);

    if (*result) {
        RFX_LOG_E(mTag, "handleCdmaSubscription fail!/n");
    }

    p_response_mdn = NULL;
    p_response_sidnid = NULL;
    p_response_min = NULL;
    p_response_prl = NULL;
}

void ViaHandler::requestSetPreferredVoicePrivacyMode(RfxBaseHandler* handler, int value, RIL_Errno *result) {
    sp<RfxAtResponse> p_response = handler->atSendCommand(String8::format("AT+VP=%d", value));
    if (p_response->getError() != 0 || p_response->getSuccess() != 1) {
        *result = RIL_E_INVALID_ARGUMENTS;
    } else {
        *result = RIL_E_SUCCESS;
    }
}

void ViaHandler::requestQueryPreferredVoicePrivacyMode(RfxBaseHandler* handler, int *value, RIL_Errno *result) {
    int err;
    RfxAtLine* line = NULL;
    sp<RfxAtResponse> p_response;

    p_response = handler->atSendCommandSingleline("AT+VP?", "+VP:");
    if (p_response->getError() != 0 || p_response->getSuccess() != 1) {
        *result = RIL_E_GENERIC_FAILURE;
        return;
    }
    line = p_response->getIntermediates();
    line->atTokStart(&err);
    if (err < 0) {
        *result = RIL_E_GENERIC_FAILURE;
        return;
    }
    *value = line->atTokNextint(&err);
    if (err < 0) {
        *result = RIL_E_GENERIC_FAILURE;
        return;
    }

    *result = RIL_E_SUCCESS;
}

int ViaHandler::convertCdmaEvdoSig(int sig, int tag) {
    int calculatedSig;

    switch (tag) {
        case SIGNAL_CDMA_DBM:
        case SIGNAL_EVDO_DBM:
            calculatedSig = 113 - sig*2;
            if (calculatedSig == 113) {
            calculatedSig = 120;
            }
            break;
        case SIGNAL_CDMA_ECIO:
            calculatedSig = sig* -5;
            break;
        case SIGNAL_EVDO_ECIO:
            calculatedSig = sig / 8;
            break;
        default:
            calculatedSig = -1;
    }
    return calculatedSig;
}

void ViaHandler::handleCdmaPrlChanged(const sp<RfxMclMessage>& msg, RfxBaseHandler* handler, int slotId) {
    int err;
    int type;
    int prl;
    RfxAtLine *line = msg->getRawUrc();
    sp<RfxMclMessage> urc;
    char slotIdStr[32];
    char prlKey[32] = "vendor.cdma.prl.version";
    char prlValue[32];

    line->atTokStart(&err);
    if (err < 0) goto error;

    type = line->atTokNextint(&err);
    if (err < 0) goto error;

    prl = line->atTokNextint(&err);
    if (err < 0) goto error;

    sprintf(slotIdStr, "%d", slotId);
    strncat(prlKey, slotIdStr, strlen(slotIdStr));
    sprintf(prlValue, "%d", prl);
    rfx_property_set(prlKey, prlValue);
    RFX_LOG_D(LOG_TAG, "prlKey: %s, prlValue: %s.", prlKey, prlValue);

    urc = RfxMclMessage::obtainUrc(RFX_MSG_URC_CDMA_PRL_CHANGED, slotId, RfxIntsData(&prl, 1));
    handler->responseToTelCore(urc);
    return;

error:
    RFX_LOG_E(LOG_TAG, "handle Cdma Prl error: %s", msg->getRawUrc()->getLine());
    return;
}

void ViaHandler::registerForViaUrc(RfxBaseHandler* handler) {
    const char* urc[] = {
        "+CIEV: 131"
        };
    handler->registerToHandleURC(urc, sizeof(urc)/sizeof(char *));
    return;
}

void ViaHandler::handleViaUrc(const sp<RfxMclMessage>& msg, RfxBaseHandler* handler, int slotId) {
    if (handler->strStartsWith(msg->getRawUrc()->getLine(), "+CIEV: 131")) {
        handleCdmaPrlChanged(msg, handler, slotId);
    }
    return;
}

const char **ViaHandler::getViaAllowedUrcForNw() {
    static const char* allowed_urc[] = {
        (char *)"+CIEV: 131"
    };
    return allowed_urc;
}

int ViaHandler::getCdmaLocationInfo(RfxBaseHandler* handler,
        RIL_VOICE_REG_STATE_CACHE *voice_reg_state_cache) {
    // +VLOCINFO:<rev>,<mcc>,<mnc>,<sid>,<nid>,<bs_id>,<bs_lat>,<bs_long>,<sector_id>,<subnet_mask>
    int err = 0;
    sp<RfxAtResponse> p_response;
    RfxAtLine* line;
    int skip = 0;

    // send AT command
    p_response = handler->atSendCommandSingleline("AT+VLOCINFO?", "+VLOCINFO:");

    // check error
    err = p_response->getError();
    if (err != 0
            || p_response == NULL
            || p_response->getSuccess() == 0
            || p_response->getIntermediates() == NULL) {
        goto error;
    }

    // handle intermediate
    line = p_response->getIntermediates();

    line->atTokStart(&err);
    if (err < 0) goto error;
    skip = line->atTokNextint(&err);
    if (err < 0) goto error;
    skip = line->atTokNextint(&err);
    if (err < 0) goto error;
    skip = line->atTokNextint(&err);
    if (err < 0) goto error;
    (*voice_reg_state_cache).system_id = line->atTokNextint(&err);
    if (err < 0) {
        goto error;
    } else {
        char *system_id = NULL;
        asprintf(&system_id, "%d", (*voice_reg_state_cache).system_id);
        if (system_id != NULL) {
            if ((*voice_reg_state_cache).system_id != mSystemId) {
               mSystemId = (*voice_reg_state_cache).system_id;
               rfx_property_set("vendor.cdma.operator.sid", system_id);
            }
            free(system_id);
        }
    }
    (*voice_reg_state_cache).network_id = line->atTokNextint(&err);
    if (err < 0) goto error;
    (*voice_reg_state_cache).base_station_id = line->atTokNextint(&err);
    if (err < 0) goto error;
    (*voice_reg_state_cache).base_station_latitude = line->atTokNextint(&err);
    if (err < 0) goto error;
    (*voice_reg_state_cache).base_station_longitude = line->atTokNextint(&err);
    if (err < 0) goto error;
    if (line->atTokHasmore()) {
        (*voice_reg_state_cache).sector_id = line->atTokNextstr(&err);
        if (err < 0) goto error;
        (*voice_reg_state_cache).subnet_mask = line->atTokNextstr(&err);
        if (err < 0) goto error;
    }
    return 1;

error:
    RFX_LOG_E(LOG_TAG, "AT+VLOCINFO? response error");
    return -1;
}

int ViaHandler::getCdmaLocationInfo(RfxBaseHandler* handler,
        CDMA_CELL_LOCATION_INFO *cdma_cell_location) {
    // +VLOCINFO:<rev>,<mcc>,<mnc>,<sid>,<nid>,<bs_id>,<bs_lat>,<bs_long>,<sector_id>,<subnet_mask>
    int err = 0;
    sp<RfxAtResponse> p_response;
    RfxAtLine* line;
    int skip = 0;

    // send AT command
    p_response = handler->atSendCommandSingleline("AT+VLOCINFO?", "+VLOCINFO:");

    // check error
    err = p_response->getError();
    if (err != 0
            || p_response == NULL
            || p_response->getSuccess() == 0
            || p_response->getIntermediates() == NULL) {
        goto error;
    }

    // handle intermediate
    line = p_response->getIntermediates();

    line->atTokStart(&err);
    if (err < 0) goto error;
    skip = line->atTokNextint(&err);
    if (err < 0) goto error;
    (*cdma_cell_location).mcc = line->atTokNextint(&err);
    if (err < 0) goto error;
    (*cdma_cell_location).mnc = line->atTokNextint(&err);
    if (err < 0) goto error;
    (*cdma_cell_location).system_id = line->atTokNextint(&err);
    if (err < 0) goto error;
    (*cdma_cell_location).network_id = line->atTokNextint(&err);
    if (err < 0) goto error;
    (*cdma_cell_location).base_station_id = line->atTokNextint(&err);
    if (err < 0) goto error;
    (*cdma_cell_location).base_station_latitude = line->atTokNextint(&err);
    if (err < 0) goto error;
    (*cdma_cell_location).base_station_longitude = line->atTokNextint(&err);
    if (err < 0) goto error;
    if (line->atTokHasmore()) {
        (*cdma_cell_location).sector_id = line->atTokNextstr(&err);
        if (err < 0) goto error;
        (*cdma_cell_location).subnet_mask = line->atTokNextstr(&err);
        if (err < 0) goto error;
    }

    return 1;

error:
    RFX_LOG_E(LOG_TAG, "getCdmaLocationInfo, AT+VLOCINFO? response error");
    return -1;
}

const char **ViaHandler::getAgpsUrc(){
    static const char* AGPS_URC[] = {
        (char *)"^GPSTCPCONNREQ",
        (char *)"^GPSTCPCLOSEREQ",
    };
    return AGPS_URC;
}

void ViaHandler::requestAgpsConnind(RfxBaseHandler* handler, int connected, RIL_Errno *result) {
    const char *mTag = "agps-ril";
    *result = RIL_E_SUCCESS;

    // send AT command
    char * cmd = NULL;
    asprintf(&cmd, "AT^GPSTCPCONNIND=%d", connected);
    sp<RfxAtResponse> p_response = handler->atSendCommand(cmd);
    free(cmd);

    if (p_response->getError() != RIL_E_SUCCESS || p_response->getSuccess() != 1) {
        RFX_LOG_E(mTag, "[RmcAgpsRequestHandler]atSendCommand error: %d", p_response->getError());
        *result = RIL_E_GENERIC_FAILURE;
    }
}
