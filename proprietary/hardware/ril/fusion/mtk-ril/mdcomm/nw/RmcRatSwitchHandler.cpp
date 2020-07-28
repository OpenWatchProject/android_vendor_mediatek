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

#include "RmcRatSwitchHandler.h"
#include "RfxMclDispatcherThread.h"
#include <telephony/mtk_ril.h>
#include "rfx_properties.h"
#include <ratconfig.h>

const int request[] = {
    RFX_MSG_REQUEST_ABORT_QUERY_AVAILABLE_NETWORKS,
    RFX_MSG_REQUEST_LOCAL_ABORT_AVAILABLE_NETWORK,
    RFX_MSG_REQUEST_SET_PREFERRED_NETWORK_TYPE,
    RFX_MSG_REQUEST_GET_PREFERRED_NETWORK_TYPE,
    RFX_MSG_REQUEST_VOICE_RADIO_TECH,
    RFX_MSG_REQUEST_GET_GMSS_RAT_MODE,
    RFX_MSG_RIL_REQUEST_STOP_NETWORK_SCAN,
    RFX_MSG_REQUEST_OM_SET_DISABLE_2G,
    RFX_MSG_REQUEST_OM_GET_DISABLE_2G
};

const int event[] = {
};

RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxVoidData, RFX_MSG_REQUEST_ABORT_QUERY_AVAILABLE_NETWORKS);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxVoidData, RFX_MSG_REQUEST_LOCAL_ABORT_AVAILABLE_NETWORK);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxVoidData, RFX_MSG_RIL_REQUEST_STOP_NETWORK_SCAN);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_OM_SET_DISABLE_2G);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxIntsData, RFX_MSG_REQUEST_OM_GET_DISABLE_2G);

// register handler to channel
RFX_IMPLEMENT_HANDLER_CLASS(RmcRatSwitchHandler, RIL_CMD_PROXY_9);

RmcRatSwitchHandler::RmcRatSwitchHandler(int slot_id, int channel_id) :
        RmcNetworkHandler(slot_id, channel_id) {

    m_slot_id = slot_id;
    m_channel_id = channel_id;
    mLastReqRatType = -1;
    mRetryCount = 1;
    registerToHandleRequest(request, sizeof(request)/sizeof(int));
    registerToHandleEvent(event, sizeof(event)/sizeof(int));

    tc1_support = false;

#ifdef MTK_TC1_COMMON_SERVICE
    tc1_support = true;
#endif
}

RmcRatSwitchHandler::~RmcRatSwitchHandler() {
}

void RmcRatSwitchHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {
    int request = msg->getId();
    // logD(LOG_TAG, "[onHandleRequest] %s", RFX_ID_TO_STR(request));
    switch(request) {
        case RFX_MSG_REQUEST_ABORT_QUERY_AVAILABLE_NETWORKS:
            requestAbortQueryAvailableNetworks(msg);
            break;
        case RFX_MSG_REQUEST_SET_PREFERRED_NETWORK_TYPE:
            requestSetPreferredNetworkType(msg);
            break;
        case RFX_MSG_REQUEST_GET_PREFERRED_NETWORK_TYPE:
            requestGetPreferredNetworkType(msg);
            break;
        case RFX_MSG_REQUEST_VOICE_RADIO_TECH:
            requestVoiceRadioTech(msg);
            break;
        case RFX_MSG_REQUEST_GET_GMSS_RAT_MODE:
            requestGetGmssRatMode(msg);
            break;
        case RFX_MSG_RIL_REQUEST_STOP_NETWORK_SCAN:
        case RFX_MSG_REQUEST_LOCAL_ABORT_AVAILABLE_NETWORK:
            requestStopNetworkScan(msg);
            break;
        case RFX_MSG_REQUEST_OM_SET_DISABLE_2G:
            requestSetDisable2G(msg);
            break;
        case RFX_MSG_REQUEST_OM_GET_DISABLE_2G:
            requestGetDisable2G(msg);
            break;
        default:
            logE(LOG_TAG, "Should not be here");
            break;
    }
}

void RmcRatSwitchHandler::onHandleEvent(const sp<RfxMclMessage>& msg) {
    int id = msg->getId();
    switch(id) {

        default:
            logE(LOG_TAG, "should not be here");
            break;
    }
}

bool RmcRatSwitchHandler::isRatPreferred() {
    bool isPreferred = true;

    if (isGcfTestMode() > 0 || isTestSim(m_slot_id) > 0) {
        isPreferred = false;  // no need preferred in test mode/test SIM
    }

    // logD(LOG_TAG, "isTddMD=%d,isWcdmaPreferred=%d,baseband=%d",
    //         isTddMD, isWcdmaPreferred, baseband);

    return isPreferred;
}

void RmcRatSwitchHandler::requestAbortQueryAvailableNetworks(const sp<RfxMclMessage>& msg) {
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> resp;
    RIL_Errno ril_errno = RIL_E_SUCCESS;

    logD(LOG_TAG, "requestAbortQueryAvailableNetworks execute while plmnListOngoing=%d", mPlmnListOngoing);
    if (mPlmnListOngoing == 1) {
        mPlmnListAbort = 1;
        p_response = atSendCommand("AT+CAPL");
        if (p_response->getError() < 0 || p_response->getSuccess() == 0) {
            mPlmnListAbort =0;
            ril_errno = RIL_E_GENERIC_FAILURE;
            logD(LOG_TAG, "requestAbortQueryAvailableNetworks fail,clear plmnListAbort flag");
        }
    }
    resp = RfxMclMessage::obtainResponse(msg->getId(), ril_errno,
            RfxVoidData(), msg, false);
    responseToTelCore(resp);
}

void RmcRatSwitchHandler::requestStopNetworkScan(const sp<RfxMclMessage>& msg) {
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> resp;
    RIL_Errno ril_errno = RIL_E_SUCCESS;

    logD(LOG_TAG, "requestStopNetworkScan execute while plmnListOngoing=%d", mPlmnListOngoing);
    if (mPlmnListOngoing == 1) {
        if (msg->getId() != RFX_MSG_REQUEST_LOCAL_ABORT_AVAILABLE_NETWORK) { // skip local abort.
            mPlmnListAbort = 1;
        }
        p_response = atSendCommand("AT+CAPL");
        if (p_response->getError() < 0 || p_response->getSuccess() == 0) {
            mPlmnListAbort =0;
            ril_errno = RIL_E_GENERIC_FAILURE;
            logD(LOG_TAG, "requestStopNetworkScan fail, clear plmnListAbort flag");
        }
    }
    resp = RfxMclMessage::obtainResponse(msg->getId(), ril_errno,
            RfxVoidData(), msg, false);
    responseToTelCore(resp);
}


bool RmcRatSwitchHandler::isInCall() {
    int ret = false;

    for (int slotId = RFX_SLOT_ID_0; slotId < RfxRilUtils::rfxGetSimCount(); slotId++) {
        if (getMclStatusManager(slotId)->getIntValue(RFX_STATUS_KEY_VOICE_CALL_COUNT, 0) > 0) {
            ret = true;
            break;
        }
    }

    return ret;
}

void RmcRatSwitchHandler::requestSetPreferredNetworkType(const sp<RfxMclMessage>& msg) {
    sp<RfxAtResponse> p_response;
    int req_type, rat, rat1;
    RIL_Errno ril_errno = RIL_E_MODE_NOT_SUPPORTED;
    bool isPreferred = false;
    int *pInt = (int *)msg->getData()->getData();
    char optr[RFX_PROPERTY_VALUE_MAX] = {0};

    req_type = pInt[0];
    rat = -1;
    rat1= 0;

    rfx_property_get("persist.vendor.radio.simswitch", optr, "1");
    int currMajorSim = (atoi(optr)-1);
    // logD(LOG_TAG, "requestSetPreferredNetworkType currMajorSim = %d", currMajorSim);

    isPreferred = isRatPreferred();
    if (isPreferred) {
        if (req_type > PREF_NET_TYPE_NR_ONLY &&
                req_type <= PREF_NET_TYPE_NR_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA) {
            rat1 = 128;  // NR preferred
        }
    }

    switch(req_type)
    {
        case PREF_NET_TYPE_GSM_WCDMA_AUTO:
            rat = 2;  // 2/3G AUTO
            break;
        case PREF_NET_TYPE_GSM_WCDMA:
        case PREF_NET_TYPE_TD_SCDMA_GSM:
        case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA:
            rat = 2;  // 2/3G AUTO
            if(isPreferred){
                rat1 = 2;  // 3G preferred
            }
            break;
        case PREF_NET_TYPE_GSM_ONLY:
            rat = 0;  // 2G only
            break;
        case PREF_NET_TYPE_WCDMA:
        case PREF_NET_TYPE_TD_SCDMA_ONLY:
        case PREF_NET_TYPE_TD_SCDMA_WCDMA:
            rat = 1;  // 3G only
            break;
        case PREF_NET_TYPE_LTE_GSM_WCDMA:
        case PREF_NET_TYPE_TD_SCDMA_GSM_LTE:
        case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_LTE:
            rat = 6;  // 2/3/4G AUTO
            if (isPreferred) {
                rat1 = 4;  //4G preferred
            }
            break;
        case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA:
        case PREF_NET_TYPE_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA:
            rat = 14;  // LTE CDMA EVDO GSM/WCDMA mode
            if (isPreferred) {
                rat1 = 4;  //4G preferred
            }
            break;
        case PREF_NET_TYPE_LTE_CDMA_EVDO_GSM:
            rat = 12;  // LTE CDMA EVDO GSM mode
            break;
        case PREF_NET_TYPE_LTE_ONLY:
            rat = 3;  // LTE only for EM mode
            break;
        case PREF_NET_TYPE_LTE_WCDMA:
        case PREF_NET_TYPE_TD_SCDMA_LTE:
        case PREF_NET_TYPE_TD_SCDMA_WCDMA_LTE:
            rat = 5;  // LTE/WCDMA for EM mode
            break;
        case PREF_NET_TYPE_LTE_GSM:
            rat = 4;  // 2/4G
            break;
        case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:
        case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO:
            rat = 10;  // GSM/WCDMA/C2K mode
            break;
        case PREF_NET_TYPE_CDMA_EVDO_GSM:
            rat = 8;  // CDMA EVDO GSM mode
            break;
        case PREF_NET_TYPE_CDMA_GSM:
            rat = 8;  // CDMA GSM mode
            rat1 = 32;
            break;
        case PREF_NET_TYPE_LTE_CDMA_EVDO:
            rat = 11;   // LTE/C2K mode
            break;
        case PREF_NET_TYPE_CDMA_EVDO_AUTO:
            rat = 7;    // C2K 1x/Evdo
            break;
        case PREF_NET_TYPE_CDMA_ONLY:
            rat = 7;    // C2K 1x/Evdo
            rat1 = 32;  // C2K 1x only
            break;
        case PREF_NET_TYPE_EVDO_ONLY:
            rat = 7;    // C2K 1x/Evdo
            rat1 = 64;  // C2K Evdo only
            break;
        case PREF_NET_TYPE_NR_ONLY:
            rat = 15;    // NR only
            break;
        case PREF_NET_TYPE_NR_LTE:
            rat = 19;    // NR/LTE
            break;
        case PREF_NET_TYPE_NR_LTE_CDMA_EVDO:
            rat = 26;    // NR/LTE/C2K
            break;
        case PREF_NET_TYPE_NR_LTE_GSM_WCDMA:
        case PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM:
        case PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM_WCDMA:
            rat = 22;    // NR/LTE/GSM/WCDMA(TDS-CDMA)
            break;
        case PREF_NET_TYPE_NR_LTE_CDMA_EVDO_GSM_WCDMA:
        case PREF_NET_TYPE_NR_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA:
            rat = 30;    // NR/LTE/C2K/GSM/WCDMA(TDS-CDMA)
            break;
        case PREF_NET_TYPE_NR_LTE_WCDMA:
        case PREF_NET_TYPE_NR_LTE_TDSCDMA:
        case PREF_NET_TYPE_NR_LTE_TDSCDMA_WCDMA:
            rat = 21;    //NR/LTE/WCDMA(TDS-CDMA)
            break;
        default:
            rat = -1;
            break;
    }

    if (rat >= 0) {
        if (getNonSlotMclStatusManager()->getBoolValue(RFX_STATUS_KEY_MODEM_POWER_OFF,
                false) == true) {
            ril_errno = RIL_E_GENERIC_FAILURE;
            logE(LOG_TAG, "SetPreferredNetworkType: Skip retry in radio off");
            goto finish;
        } else if (isInCall() == true) {
            ril_errno = RIL_E_OPERATION_NOT_ALLOWED;
            logE(LOG_TAG, "requestSetPreferredNetworkType: Skip retry in call");
            goto finish;
        }
        // send AT command
        p_response = atSendCommand(String8::format("AT+ERAT=%d,%d", rat, rat1));
        if (p_response->getError() >= 0 && p_response->getSuccess() != 0) {
            logV(LOG_TAG, "SetPreferredNetworkType: ok, count=%d, MajorSim=%d, Preferred=%d",
                    mRetryCount, currMajorSim, isPreferred);
            mLastReqRatType = req_type;
            ril_errno = RIL_E_SUCCESS;
        } else {
            logE(LOG_TAG, "SetPreferredNetworkType: fail, count=%d, error=%d, MajorSim=%d",
                    mRetryCount, p_response->atGetCmeError(), currMajorSim);
            if (p_response->atGetCmeError() == CME_OPERATION_NOT_SUPPORTED) {
                ril_errno = RIL_E_MODE_NOT_SUPPORTED;
                logE(LOG_TAG, "SetPreferredNetworkType: fail, modem not support dual C2K RAT");
            } else {  // retry 40 times if set rat fail.
                if (mRetryCount < 40) {
                    mRetryCount++;
                    sp<RfxMclMessage> mclMessage = RfxMclMessage::obtainRequest(msg->getId(),
                            msg->getData(), msg->getSlotId(), msg->getToken(),
                            msg->getSendToMainProtocol(), msg->getRilToken(), ms2ns(1000),
                            msg->getTimeStamp(), msg->getAddAtFront());
                    RfxMclDispatcherThread::enqueueMclMessageDelay(mclMessage);
                    return;
                }
                if (p_response->atGetCmeError() == CME_OPERATION_NOT_ALLOWED_ERR) {
                    ril_errno = RIL_E_OPERATION_NOT_ALLOWED;
                }
            }
        }
        p_response = NULL;
    }

finish:
    mRetryCount = 1;
    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), ril_errno,
            RfxVoidData(), msg, false);
    // response to TeleCore
    responseToTelCore(response);
}

void RmcRatSwitchHandler::requestGetPreferredNetworkType(const sp<RfxMclMessage>& msg) {
    sp<RfxAtResponse> p_response;
    int err, skip, nt_type, prefer_type = 0, return_type;
    RfxAtLine* line1;
    sp<RfxMclMessage> response;

    if (mLastReqRatType > -1) {
        return_type = mLastReqRatType;
        goto queryDone;
    }

    p_response = atSendCommandSingleline("AT+ERAT?", "+ERAT:");

    err = p_response->getError();
    if (err < 0 || p_response->getSuccess() == 0)
        goto error;

    line1 = p_response->getIntermediates();

    // go to start position
    line1->atTokStart(&err);
    if(err < 0) goto error;

    //skip <curr_rat>
    skip = line1->atTokNextint(&err);
    if(err < 0) goto error;

    //skip <gprs_status>
    skip = line1->atTokNextint(&err);
    if(err < 0) goto error;

    //get <rat>
    nt_type = line1->atTokNextint(&err);
    if(err < 0) goto error;

    //get <prefer rat>
    prefer_type = line1->atTokNextint(&err);
    if(err < 0) goto error;

    if (nt_type == 0) {
        return_type = PREF_NET_TYPE_GSM_ONLY;
    } else if (nt_type == 1) {
        return_type = PREF_NET_TYPE_WCDMA;
    } else if (nt_type == 2 && prefer_type == 0) {
        return_type = PREF_NET_TYPE_GSM_WCDMA_AUTO;
    } else if (nt_type == 2 && prefer_type == 1) {
        logE(LOG_TAG, "Dual mode but GSM prefer, mount to AUTO mode");
        return_type = PREF_NET_TYPE_GSM_WCDMA_AUTO;
    } else if (nt_type == 2 && prefer_type == 2) {
        return_type = PREF_NET_TYPE_GSM_WCDMA;
    //for LTE -- START
    } else if (nt_type == 6 && prefer_type == 4) {
        //4G Preferred (4G, 3G/2G) item
        //Bause we are not defind LTE preferred,
        //so return by NT_LTE_GSM_WCDMA_TYPE temporary
        return_type = PREF_NET_TYPE_LTE_GSM_WCDMA;
    } else if (nt_type == 6 && prefer_type == 0) {
        //4G/3G/2G(Auto) item
        return_type = PREF_NET_TYPE_LTE_GSM_WCDMA;
    } else if (nt_type == 14) {
        // LTE CDMA EVDO GSM/WCDMA mode
        return_type = PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA;
    } else if (nt_type == 12) {
        // LTE CDMA EVDO GSM mode
        return_type = PREF_NET_TYPE_LTE_CDMA_EVDO_GSM;
    } else if (nt_type == 3 && prefer_type == 0) {
        //4G only
        return_type = PREF_NET_TYPE_LTE_ONLY;
    } else if (nt_type == 5 && prefer_type == 0) {
        // 4G/3G
        return_type = PREF_NET_TYPE_LTE_WCDMA;
    } else if (nt_type == 10) {
        // 2G/3G/C2K
        return_type = PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO;
    } else if (nt_type == 8 && prefer_type == 0) {
        // 2G/C2K 1x/evdo
        return_type = PREF_NET_TYPE_CDMA_EVDO_GSM;
    } else if (nt_type == 8 && prefer_type == 32) {
        // 2G/C2K 1x
        return_type = PREF_NET_TYPE_CDMA_GSM;
    } else if(nt_type == 11) {
        // LC mode
        return_type = PREF_NET_TYPE_LTE_CDMA_EVDO;
    } else if (nt_type == 7 && prefer_type == 0) {
        // C2K 1x/evdo
        return_type = PREF_NET_TYPE_CDMA_EVDO_AUTO;
    } else if (nt_type == 7 && prefer_type == 32) {
        // C2K 1x only
        return_type = PREF_NET_TYPE_CDMA_ONLY;
    } else if (nt_type == 7 && prefer_type == 64) {
        // C2K evdo only
        return_type = PREF_NET_TYPE_EVDO_ONLY;
    } else if (nt_type == 4) {
        // 4G/2G
        return_type = PREF_NET_TYPE_LTE_GSM;
    } else if (nt_type == 15) {
        // 5G only
        return_type = PREF_NET_TYPE_NR_ONLY;
    } else if (nt_type == 19) {
        // 5G/4G
        return_type = PREF_NET_TYPE_NR_LTE;
    } else if (nt_type == 26) {
        // 5G/4G/C2K
        return_type = PREF_NET_TYPE_NR_LTE_CDMA_EVDO;
    } else if (nt_type == 22) {
        // 5G/4G/3G/2G
        return_type = PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM_WCDMA; //NETWORK_MODE_NR_LTE_GSM_WCDMA; NETWORK_MODE_NR_LTE_TDSCDMA_GSM;
    } else if (nt_type == 30) {
        // 5G/4G/C2K/3G/2G
        return_type = PREF_NET_TYPE_NR_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA; //NETWORK_MODE_NR_LTE_CDMA_EVDO_GSM_WCDMA;
    } else if (nt_type == 21) {
        // 5G/4G/3G
        return_type =PREF_NET_TYPE_NR_LTE_TDSCDMA_WCDMA; //NETWORK_MODE_NR_LTE_WCDMA; NETWORK_MODE_NR_LTE_TDSCDMA;
    } else {
        goto error;
    }

queryDone:
    response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxIntsData(&return_type, 1), msg, false);
    responseToTelCore(response);
    return;

error:
    response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
            RfxVoidData(), msg, false);
    responseToTelCore(response);
}

void RmcRatSwitchHandler::requestVoiceRadioTech(const sp<RfxMclMessage>& msg) {
    RFX_UNUSED(msg);
    // do nothing
}

void RmcRatSwitchHandler::requestGetGmssRatMode(const sp<RfxMclMessage>& msg) {
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> response;
    int err = 0;
    int data[5] = { 0 };
    RfxAtLine* line = NULL;

    p_response = atSendCommandSingleline("AT+EGMSS?", "+EGMSS:");

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

    for (int i = 0; i < 5; i++) {
        if (i == 1) {  // MCC
            data[i] = atoi(line->atTokNextstr(&err));
        } else {
            data[i] = line->atTokNextint(&err);
        }
        if (err < 0) {
            goto error;
        }
    }

    response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxIntsData(data, 5), msg, false);
    responseToTelCore(response);
    return;

error:
    logE(LOG_TAG, "requestGetGmssRatMode error");
    response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
            RfxIntsData(), msg, false);
    responseToTelCore(response);
}

void RmcRatSwitchHandler::requestSetDisable2G(const sp<RfxMclMessage>& msg) {
    RIL_Errno ril_errno = RIL_E_MODE_NOT_SUPPORTED;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> resp;
    int mode = 0;

    int *pInt = (int *)msg->getData()->getData();
    mode = pInt[0];
    if (mode == 1 || mode == 0) {

        if (isOp07Support() || tc1_support) {
            logD(LOG_TAG, "[OP07] requestSetDisable2G %d", mode);
            p_response = atSendCommand(String8::format("AT+EDRAT=%d", mode));
            if (p_response->getError() >= 0 && p_response->getSuccess() != 0) {
                rfx_property_set("persist.vendor.radio.disable.2g",
                        String8::format("%d", mode).string());
                ril_errno = RIL_E_SUCCESS;
            }
        } else if (isOp08Support()) {
            logD(LOG_TAG, "[OP08] requestSetDisable2G %d", mode);
            p_response = atSendCommand(String8::format("AT+EGMC=1,\"rat_disable_with_ecc\",%d", mode));
            if (p_response->getError() >= 0 && p_response->getSuccess() != 0) {
                ril_errno = RIL_E_SUCCESS;
            }
        } else {
            logD(LOG_TAG, "NOT SUPPORT, requestSetDisable2G %d", mode);
        }
    }
    p_response = NULL;

error:
    resp = RfxMclMessage::obtainResponse(msg->getId(), ril_errno,
            RfxVoidData(), msg, false);
    responseToTelCore(resp);
}

void RmcRatSwitchHandler::requestGetDisable2G(const sp<RfxMclMessage>& msg) {
    RIL_Errno ril_errno = RIL_E_MODE_NOT_SUPPORTED;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> resp;
    RfxAtLine* line;
    int err=0, mode=0;
    char *type=NULL;
    char property_value[RFX_PROPERTY_VALUE_MAX] = {0};

    if (isOp07Support() || tc1_support) {
        rfx_property_get("persist.vendor.radio.disable.2g", property_value, "0");
        mode = atoi(property_value);
        logD(LOG_TAG, "[OP07] requestGetDisable2G %d", mode);
        ril_errno = RIL_E_SUCCESS;
    } else if (isOp08Support()) {
        p_response = atSendCommandSingleline("AT+EGMC=0,\"rat_disable_with_ecc\"", "+EGMC:");

        err = p_response->getError();
        if (err < 0 || p_response->getSuccess() == 0)
            goto error;

        line = p_response->getIntermediates();

        // go to start position
        line->atTokStart(&err);
        if(err < 0) goto error;

        // get "type"
        type = line->atTokNextstr(&err);
        if(err < 0) goto error;

        if (strncmp(type, "rat_disable_with_ecc", strlen("rat_disable_with_ecc")) != 0) {
            goto error;
        }

        // get <mode>
        mode = line->atTokNextint(&err);
        if(err < 0) goto error;

        logD(LOG_TAG, "[OP08] requestGetDisable2G %d", mode);
        ril_errno = RIL_E_SUCCESS;
    } else {
        logD(LOG_TAG, "NOT SUPPORT, requestGetDisable2G %d", mode);
    }

error:
    resp = RfxMclMessage::obtainResponse(msg->getId(), ril_errno,
            RfxIntsData(&mode, 1), msg, false);
    responseToTelCore(resp);
}

