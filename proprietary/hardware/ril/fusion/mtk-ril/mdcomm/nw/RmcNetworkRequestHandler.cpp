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

#include "RmcNetworkRequestHandler.h"
#include "rfx_properties.h"
#include "ViaBaseHandler.h"
#include "RfxViaUtils.h"

static const int request[] = {
    RFX_MSG_REQUEST_SIGNAL_STRENGTH,
    RFX_MSG_REQUEST_SIGNAL_STRENGTH_WITH_WCDMA_ECIO,
    RFX_MSG_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC,
    RFX_MSG_REQUEST_SET_NETWORK_SELECTION_MANUAL,
    RFX_MSG_REQUEST_SET_NETWORK_SELECTION_MANUAL_WITH_ACT,
    RFX_MSG_REQUEST_SET_BAND_MODE,
    RFX_MSG_REQUEST_QUERY_AVAILABLE_BAND_MODE,
    RFX_MSG_REQUEST_GET_NEIGHBORING_CELL_IDS,
    RFX_MSG_REQUEST_SET_LOCATION_UPDATES,
    RFX_MSG_REQUEST_GET_CELL_INFO_LIST,
    RFX_MSG_REQUEST_SET_UNSOL_CELL_INFO_LIST_RATE,
    RFX_MSG_REQUEST_GET_POL_CAPABILITY,
    RFX_MSG_REQUEST_GET_POL_LIST,
    RFX_MSG_REQUEST_SET_POL_ENTRY,
    RFX_MSG_REQUEST_CDMA_SET_ROAMING_PREFERENCE,
    RFX_MSG_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE,
    RFX_MSG_REQUEST_GET_FEMTOCELL_LIST,
    RFX_MSG_REQUEST_ABORT_FEMTOCELL_LIST,
    RFX_MSG_REQUEST_SELECT_FEMTOCELL,
    RFX_MSG_REQUEST_SCREEN_STATE,
    RFX_MSG_REQUEST_SET_UNSOLICITED_RESPONSE_FILTER,
    RFX_MSG_REQUEST_QUERY_FEMTOCELL_SYSTEM_SELECTION_MODE,
    RFX_MSG_REQUEST_SET_FEMTOCELL_SYSTEM_SELECTION_MODE,
    // RFX_MSG_REQUEST_VSS_ANTENNA_CONF,
    // RFX_MSG_REQUEST_VSS_ANTENNA_INFO,
    RFX_MSG_REQUEST_SET_SERVICE_STATE,
    RFX_MSG_REQUEST_SET_PSEUDO_CELL_MODE,
    RFX_MSG_REQUEST_GET_PSEUDO_CELL_INFO,
    RFX_MSG_REQUEST_SET_ROAMING_ENABLE,
    RFX_MSG_REQUEST_GET_ROAMING_ENABLE,
    RFX_MSG_REQUEST_SET_LTE_RELEASE_VERSION,
    RFX_MSG_REQUEST_GET_LTE_RELEASE_VERSION,
    RFX_MSG_REQUEST_SET_SIGNAL_STRENGTH_REPORTING_CRITERIA,
    RFX_MSG_REQUEST_SET_SYSTEM_SELECTION_CHANNELS,
    RFX_MSG_REQUEST_GET_TS25_NAME,
    RFX_MSG_REQUEST_GET_EHRPD_INFO_FOR_IMS,
    RFX_MSG_REQUEST_ENABLE_CA_PLUS_FILTER,
    RFX_MSG_REQUEST_REGISTER_CELLULAR_QUALITY_REPORT,
    RFX_MSG_REQUEST_GET_SUGGESTED_PLMN_LIST,
    RFX_MSG_REQUEST_CONFIG_A2_OFFSET,
    RFX_MSG_REQUEST_CONFIG_B1_OFFSET,
    RFX_MSG_REQUEST_ENABLE_SCG_FAILURE,
    RFX_MSG_REQUEST_DISABLE_NR,
    RFX_MSG_REQUEST_SET_TX_POWER,
    RFX_MSG_REQUEST_SEARCH_STORED_FREQUENCY_INFO,
    RFX_MSG_REQUEST_SEARCH_RAT,
    RFX_MSG_REQUEST_SET_BACKGROUND_SEARCH_TIMER
};

static const int events[] = {
    RFX_MSG_EVENT_EXIT_EMERGENCY_CALLBACK_MODE,
    RFX_MSG_EVENT_FEMTOCELL_UPDATE,
    RFX_MSG_EVENT_CONFIRM_RAT_BEGIN,
    RFX_MSG_EVENT_CS_NETWORK_STATE,
    RFX_MSG_EVENT_PS_NETWORK_STATE,
    RFX_MSG_EVENT_RERESH_PHYSICAL_CONFIG,
    RFX_MSG_EVENT_RSP_DATA_CONTEXT_IDS
};

// register data
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxIntsData, RFX_MSG_REQUEST_SIGNAL_STRENGTH);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxVoidData, RFX_MSG_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringData, RfxVoidData, RFX_MSG_REQUEST_SET_NETWORK_SELECTION_MANUAL);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxVoidData, RFX_MSG_REQUEST_SET_NETWORK_SELECTION_MANUAL_WITH_ACT);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_SET_BAND_MODE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxIntsData, RFX_MSG_REQUEST_QUERY_AVAILABLE_BAND_MODE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxNeighboringCellData, RFX_MSG_REQUEST_GET_NEIGHBORING_CELL_IDS);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_SET_LOCATION_UPDATES);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxCellInfoData, RFX_MSG_REQUEST_GET_CELL_INFO_LIST);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_SET_UNSOL_CELL_INFO_LIST_RATE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxIntsData, RFX_MSG_REQUEST_GET_POL_CAPABILITY);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxStringsData, RFX_MSG_REQUEST_GET_POL_LIST);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxVoidData, RFX_MSG_REQUEST_SET_POL_ENTRY);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_CDMA_SET_ROAMING_PREFERENCE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxIntsData, RFX_MSG_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxStringsData, RFX_MSG_REQUEST_GET_FEMTOCELL_LIST);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxVoidData, RFX_MSG_REQUEST_ABORT_FEMTOCELL_LIST);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxVoidData, RFX_MSG_REQUEST_SELECT_FEMTOCELL);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_SCREEN_STATE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_SET_UNSOLICITED_RESPONSE_FILTER);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxIntsData, RFX_MSG_REQUEST_QUERY_FEMTOCELL_SYSTEM_SELECTION_MODE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_SET_FEMTOCELL_SYSTEM_SELECTION_MODE);
// RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxIntsData, RFX_MSG_REQUEST_VSS_ANTENNA_CONF);
// RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxIntsData, RFX_MSG_REQUEST_VSS_ANTENNA_INFO);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_SET_SERVICE_STATE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_SET_PSEUDO_CELL_MODE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxIntsData, RFX_MSG_REQUEST_GET_PSEUDO_CELL_INFO);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_SET_ROAMING_ENABLE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxIntsData, RFX_MSG_REQUEST_GET_ROAMING_ENABLE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_SET_LTE_RELEASE_VERSION);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxIntsData, RFX_MSG_REQUEST_GET_LTE_RELEASE_VERSION);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxIntsData,
        RFX_MSG_REQUEST_SIGNAL_STRENGTH_WITH_WCDMA_ECIO);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxSsrcData, RfxVoidData,
        RFX_MSG_REQUEST_SET_SIGNAL_STRENGTH_REPORTING_CRITERIA);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxSscData, RfxVoidData,
        RFX_MSG_REQUEST_SET_SYSTEM_SELECTION_CHANNELS);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxStringData, RFX_MSG_REQUEST_GET_TS25_NAME);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_ENABLE_CA_PLUS_FILTER);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxVoidData, RFX_MSG_REQUEST_REGISTER_CELLULAR_QUALITY_REPORT);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxStringData, RFX_MSG_REQUEST_GET_EHRPD_INFO_FOR_IMS);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxStringsData, RFX_MSG_REQUEST_GET_SUGGESTED_PLMN_LIST);
// NR request
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxIntsData, RFX_MSG_REQUEST_CONFIG_A2_OFFSET);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxIntsData, RFX_MSG_REQUEST_CONFIG_B1_OFFSET);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxIntsData, RFX_MSG_REQUEST_ENABLE_SCG_FAILURE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxIntsData, RFX_MSG_REQUEST_DISABLE_NR);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxIntsData, RFX_MSG_REQUEST_SET_TX_POWER);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxIntsData, RFX_MSG_REQUEST_SEARCH_STORED_FREQUENCY_INFO);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxIntsData, RFX_MSG_REQUEST_SEARCH_RAT);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxIntsData, RFX_MSG_REQUEST_SET_BACKGROUND_SEARCH_TIMER);

RFX_REGISTER_DATA_TO_URC_ID(RfxStringsData, RFX_MSG_URC_FEMTOCELL_INFO);
RFX_REGISTER_DATA_TO_URC_ID(RfxIntsData, RFX_MSG_URC_RESPONSE_PS_NETWORK_STATE_CHANGED);

RFX_REGISTER_DATA_TO_EVENT_ID(RfxIntsData, RFX_MSG_EVENT_RSP_DATA_CONTEXT_IDS);

// register handler to channel
RFX_IMPLEMENT_HANDLER_CLASS(RmcNetworkRequestHandler, RIL_CMD_PROXY_3);

RmcNetworkRequestHandler::RmcNetworkRequestHandler(int slot_id, int channel_id) :
        RmcNetworkHandler(slot_id, channel_id),
        m_emergency_only(0){
    int err;
    sp<RfxAtResponse> p_response;
    logV(LOG_TAG, "%s[%d] start", __FUNCTION__, slot_id);
    m_slot_id = slot_id;
    m_channel_id = channel_id;
    registerToHandleRequest(request, sizeof(request)/sizeof(int));
    registerToHandleEvent(events, sizeof(events)/sizeof(int));

    pthread_mutex_lock(&s_signalStrengthMutex[m_slot_id]);
    resetSignalStrengthCache(signal_strength_cache[m_slot_id], CACHE_GROUP_ALL);
    pthread_mutex_unlock(&s_signalStrengthMutex[m_slot_id]);
    ril_wfc_reg_status[m_slot_id] = 0;

    updatePseudoCellMode();

    /* Check if modem support EREG = 5 */
    // REG_REPORT_MODE:
    //     0: EREG=5 is not support
    //     1: EREG=5 is support and phase out ECREG/ECGREG/ECEREG
    char EREG5[] = "REG_REPORT_MODE";
    int support_ereg5 = getFeatureVersion(EREG5, 0);

    if (support_ereg5 == SUPPORT_EREG5) {
        support_ereg_5 = 1;
    } else {
        logE(LOG_TAG, "Modem does not support AT+EREG=5.");
        support_ereg_5 = 0;
    }

    if (support_ereg_5 == 0) {  // for older MD.
        /* Enable network registration, location info and EMM cause value unsolicited result */
        atSendCommand("AT+CEREG=3");
    }

    /* Enable 93 network registration, location info and cause value unsolicited result */
    atSendCommand("AT+EREG=3");
    /* Enable 93 network registration, location info and GMM cause value unsolicited result */
    atSendCommand("AT+EGREG=3");
    /* Enable packet switching data service capability URC */
    atSendCommand("AT+PSBEARER=1");
    /* Enable received signal level indication */
    atSendCommand("AT+ECSQ=1");
    /* Config the Signal notify frequency */
    atSendCommand("AT+ECSQ=3,0,2");
    /* Enable getting NITZ, include TZ and Operator Name*/
    /* To Receive +CTZE and +CIEV: 10*/
    atSendCommand("AT+CTZR=3");
    /* Enable CSG network URC */
    if (isFemtocellSupport()) {
        atSendCommand("AT+ECSG=4,1");
        /* Enable C2K femtocell URC */
        atSendCommand("AT+EFCELL=1");
    }
    /* Check if modem support ECELL ext3,ext4 */
    p_response = atSendCommand("AT+ECELL=4");
    err = p_response->getError();
    if (err < 0 || p_response->getSuccess() == 0) {
        logE(LOG_TAG, "modem does not support AT+ECELL=4.");
        ECELLext3ext4Support = 0;
    }
    /* Config cell info list extend c2k */
    atSendCommand("AT+ECELL=7,1");
    /* To support return EONS if available in RIL_REQUEST_OPERATOR START */
    atSendCommand("AT+EONS=1");
    /* ALPS00574862 Remove redundant +COPS=3,2;+COPS? multiple cmd in REQUEST_OPERATOR */
    atSendCommand("AT+EOPS=3,2");
    /* check whether calibration data is downloaded or not */
    p_response = atSendCommand("AT+ECAL?");
    err = p_response->getError();
    if (err < 0 || p_response->getSuccess() == 0) {
        logE(LOG_TAG, "bootup get calibration status failed, err=%d", err);
    }
    /* 2G only feature */
    if (isDisable2G()) {
        atSendCommand("AT+EDRAT=1");
    }
    /*Modulation report*/
    if (isEnableModulationReport()) {
        atSendCommand("AT+EMODCFG=1");
    }
    /* Disable +EMCCMNC URC */
    atSendCommand("AT+EMCCMNC=0");

    /* Enable IMS ECC URC*/
    atSendCommand("AT+EIMSESS=1");
    // TODO: consider boot up screen off test scenario
}

RmcNetworkRequestHandler::~RmcNetworkRequestHandler() {
}

void RmcNetworkRequestHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {
    // logD(LOG_TAG, "[onHandleRequest] %s", RFX_ID_TO_STR(msg->getId()));
    int request = msg->getId();
    switch(request) {
        case RFX_MSG_REQUEST_SIGNAL_STRENGTH:
        case RFX_MSG_REQUEST_SIGNAL_STRENGTH_WITH_WCDMA_ECIO:
            requestSignalStrength(msg);
            break;
        case RFX_MSG_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC:
            requestSetNetworkSelectionAutomatic(msg);
            break;
        case RFX_MSG_REQUEST_SET_NETWORK_SELECTION_MANUAL:
            requestSetNetworkSelectionManual(msg);
            break;
        case RFX_MSG_REQUEST_SET_NETWORK_SELECTION_MANUAL_WITH_ACT:
            requestSetNetworkSelectionManualWithAct(msg);
            break;
        case RFX_MSG_REQUEST_SET_BAND_MODE:
            requestSetBandMode(msg);
            break;
        case RFX_MSG_REQUEST_QUERY_AVAILABLE_BAND_MODE:
            requestQueryAvailableBandMode(msg);
            break;
        case RFX_MSG_REQUEST_GET_NEIGHBORING_CELL_IDS:
            requestGetNeighboringCellIds(msg);
            break;
        case RFX_MSG_REQUEST_SET_LOCATION_UPDATES:
            requestSetLocationUpdates(msg);
            break;
        case RFX_MSG_REQUEST_GET_CELL_INFO_LIST:
            requestGetCellInfoList(msg);
            break;
        case RFX_MSG_REQUEST_SET_UNSOL_CELL_INFO_LIST_RATE:
            requestSetCellInfoListRate(msg);
            break;
        case RFX_MSG_REQUEST_GET_POL_CAPABILITY:
            requestGetPOLCapability(msg);
            break;
        case RFX_MSG_REQUEST_GET_POL_LIST:
            requestGetPOLList(msg);
            break;
        case RFX_MSG_REQUEST_SET_POL_ENTRY:
            requestSetPOLEntry(msg);
            break;
        case RFX_MSG_REQUEST_CDMA_SET_ROAMING_PREFERENCE:
            requestSetCdmaRoamingPreference(msg);
            break;
        case RFX_MSG_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE:
            requestQueryCdmaRoamingPreference(msg);
            break;
        case RFX_MSG_REQUEST_GET_FEMTOCELL_LIST:
            requestGetFemtocellList(msg);
            break;
        case RFX_MSG_REQUEST_ABORT_FEMTOCELL_LIST:
            requestAbortFemtocellList(msg);
            break;
        case RFX_MSG_REQUEST_SELECT_FEMTOCELL:
            requestSelectFemtocell(msg);
            break;
        case RFX_MSG_REQUEST_SCREEN_STATE:
            requestScreenState(msg);
            break;
        case RFX_MSG_REQUEST_SET_UNSOLICITED_RESPONSE_FILTER:
            requestSetUnsolicitedResponseFilter(msg);
            break;
        case RFX_MSG_REQUEST_QUERY_FEMTOCELL_SYSTEM_SELECTION_MODE:
            requestQueryFemtoCellSystemSelectionMode(msg);
            break;
        case RFX_MSG_REQUEST_SET_FEMTOCELL_SYSTEM_SELECTION_MODE:
            requestSetFemtoCellSystemSelectionMode(msg);
            break;
        case RFX_MSG_REQUEST_VSS_ANTENNA_CONF:
            requestAntennaConf(msg);
            break;
        case RFX_MSG_REQUEST_VSS_ANTENNA_INFO:
            requestAntennaInfo(msg);
            break;
        case RFX_MSG_REQUEST_SET_SERVICE_STATE:
            requestSetServiceState(msg);
            break;
        case RFX_MSG_REQUEST_SET_PSEUDO_CELL_MODE:
            requestSetPseudoCellMode(msg);
            break;
        case RFX_MSG_REQUEST_GET_PSEUDO_CELL_INFO:
            requestGetPseudoCellInfo(msg);
            break;
        case RFX_MSG_REQUEST_SET_ROAMING_ENABLE:
            setRoamingEnable(msg);
            break;
        case RFX_MSG_REQUEST_GET_ROAMING_ENABLE:
            getRoamingEnable(msg);
            break;
        case RFX_MSG_REQUEST_SET_LTE_RELEASE_VERSION:
            requestSetLteReleaseVersion(msg);
            break;
        case RFX_MSG_REQUEST_GET_LTE_RELEASE_VERSION:
            requestGetLteReleaseVersion(msg);
            break;
        case RFX_MSG_REQUEST_SET_SIGNAL_STRENGTH_REPORTING_CRITERIA:
            requestSetSignalStrengthReportingCriteria(msg);
            break;
        case RFX_MSG_REQUEST_SET_SYSTEM_SELECTION_CHANNELS:
            requestSetSystemSelectionChannels(msg);
            break;
        case RFX_MSG_REQUEST_GET_TS25_NAME:
            requestGetTs25Name(msg);
            break;
        case RFX_MSG_REQUEST_ENABLE_CA_PLUS_FILTER:
            requestEnableCaPlusFilter(msg);
            break;
        case RFX_MSG_REQUEST_REGISTER_CELLULAR_QUALITY_REPORT:
            registerCellularQualityReport(msg);
            break;
        case RFX_MSG_REQUEST_GET_EHRPD_INFO_FOR_IMS:
            requestGetEhrpdInfo(msg);
            break;
        case RFX_MSG_REQUEST_GET_SUGGESTED_PLMN_LIST:
            requestGetSuggestedPlmnList(msg);
            break;
// NR request
        case RFX_MSG_REQUEST_CONFIG_A2_OFFSET:
            requestConfigA2Offset(msg);
            break;
        case RFX_MSG_REQUEST_CONFIG_B1_OFFSET:
            requestConfigB1Offset(msg);
            break;
        case RFX_MSG_REQUEST_ENABLE_SCG_FAILURE:
            requestEnableSCGFailure(msg);
            break;
        case RFX_MSG_REQUEST_DISABLE_NR:
            requestDisableNr(msg);
            break;
        case RFX_MSG_REQUEST_SET_TX_POWER:
            requestSetTxPower(msg);
            break;
        case RFX_MSG_REQUEST_SEARCH_STORED_FREQUENCY_INFO:
            requestSearchStoreFrenquencyInfo(msg);
            break;
        case RFX_MSG_REQUEST_SEARCH_RAT:
            requestSearchRat(msg);
            break;
        case RFX_MSG_REQUEST_SET_BACKGROUND_SEARCH_TIMER:
            requestSetBackgroundSearchTimer(msg);
            break;
        default:
            logE(LOG_TAG, "Should not be here");
            break;
    }
}

void RmcNetworkRequestHandler::requestSignalStrength(const sp<RfxMclMessage>& msg)
{
    sp<RfxAtResponse> p_response;
    RfxAtLine* p_cur;
    int err;
    sp<RfxMclMessage> response;
    int len = sizeof(RIL_SIGNAL_STRENGTH_CACHE)/sizeof(int);
    int *resp = new int[len];
    pthread_mutex_lock(&s_signalStrengthMutex[m_slot_id]);

    RIL_RadioState state =
        (RIL_RadioState) getMclStatusManager()->getIntValue(RFX_STATUS_KEY_RADIO_STATE);
    if (RADIO_STATE_OFF == state) {
        resetSignalStrengthCache(signal_strength_cache[m_slot_id], CACHE_GROUP_ALL);
    } else {
        // send AT command
        p_response = atSendCommandMultiline("AT+ECSQ", "+ECSQ:");

        // check error
        err = p_response->getError();
        if (err != 0 ||
                p_response == NULL ||
                p_response->getSuccess() == 0 ||
                p_response->getIntermediates() == NULL) {
            goto error;
        }

        for (p_cur = p_response->getIntermediates()
             ; p_cur != NULL
             ; p_cur = p_cur->getNext()
             ) {
            err = getSignalStrength(p_cur);

            if (err != 0)
                continue;
        }
    }

    printSignalStrengthCache((char*) __FUNCTION__);

    // copy signal strength cache to int array
    memcpy(resp, signal_strength_cache[m_slot_id], len*sizeof(int));
    pthread_mutex_unlock(&s_signalStrengthMutex[m_slot_id]);
    // returns the whole cache, including GSM, WCDMA, TD-SCDMA, CDMA, EVDO, LTE
    response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxIntsData((void*)resp, len*sizeof(int)), msg, false);
    // response to TeleCore
    responseToTelCore(response);
    delete [] resp;

    return;

error:
    pthread_mutex_unlock(&s_signalStrengthMutex[m_slot_id]);
    logE(LOG_TAG, "requestSignalStrength must never return an error when radio is on");
    response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
            RfxIntsData(), msg, false);
    // response to TeleCore
    responseToTelCore(response);
    delete [] resp;
    return;
}

void RmcNetworkRequestHandler::requestSetNetworkSelectionAutomatic(const sp<RfxMclMessage>& msg) {
    int err;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> response;
    RIL_Errno ril_errno;

    // Use +COPS to do auto selection mode if it's a test
    if (isGcfTestMode() > 0 || isTestSim(m_slot_id) > 0) {
        p_response = atSendCommand("AT+COPS=0");
    } else {
        p_response = atSendCommand("AT+EOPS=0");
    }

    err = p_response->getError();
    if (err < 0 || p_response->getSuccess() == 0) {
        ril_errno = RIL_E_REQUEST_NOT_SUPPORTED;
    } else {
        ril_errno = RIL_E_SUCCESS;
        mdEops[m_slot_id]->mode = 0;
    }

    response = RfxMclMessage::obtainResponse(msg->getId(), ril_errno,
            RfxVoidData(), msg, false);
    // response to TeleCore
    responseToTelCore(response);
}

void RmcNetworkRequestHandler::requestSetNetworkSelectionManual(const sp<RfxMclMessage>& msg) {
    int err = 0;
    const char *numeric_code;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> response;
    RIL_Errno ril_errno = RIL_E_GENERIC_FAILURE;
    int len, i;

    numeric_code = (char*)(msg->getData()->getData());

    if (NULL == numeric_code) {
        logE(LOG_TAG, "numeric is null!");
        ril_errno = RIL_E_INVALID_ARGUMENTS;
        goto error;
    }

    len = strlen(numeric_code);
    if (len == 6 && (strncmp(numeric_code, "123456", len) == 0)) {
        logE(LOG_TAG, "numeric is incorrect %s", numeric_code);
        ril_errno = RIL_E_INVALID_ARGUMENTS;
        goto error;
    }
    if (len == 5 || len == 6) {
        // check if the numeric code is valid digit or not
        for(i = 0; i < len ; i++) {
            if(numeric_code[i] < '0' || numeric_code[i] > '9')
                break;
        }
        if (i == len) {
            if (err >= 0) {
                mdEops[m_slot_id]->mode = 1;
                p_response = atSendCommand(String8::format("AT+COPS=1, 2, \"%s\"", numeric_code));
                err = p_response->getError();
                if (!(err < 0 || p_response->getSuccess() == 0)) {
                    ril_errno = RIL_E_SUCCESS;
                } else {
                    ril_errno = RIL_E_INVALID_STATE;
                }
            }
        } else {
            logE(LOG_TAG, "the numeric code contains invalid digits");
        }
    } else {
        logE(LOG_TAG, "the data length is invalid for Manual Selection");
    }

error:
    response = RfxMclMessage::obtainResponse(msg->getId(), ril_errno,
            RfxVoidData(), msg, false);
    // response to TeleCore
    responseToTelCore(response);
}

void RmcNetworkRequestHandler::requestSetNetworkSelectionManualWithAct(const sp<RfxMclMessage>& msg) {
    int err, len, i;;
    const char *numeric_code, *act, *mode;
    RIL_Errno ril_errno = RIL_E_GENERIC_FAILURE;
    bool isSemiAutoMode = false;
    bool noneBlockAutoMode = false;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> response;
    const char **pReqData = (const char **)msg->getData()->getData();

    numeric_code = pReqData[0];
    act = pReqData[1];

    if (NULL == numeric_code || NULL == act) {
        logE(LOG_TAG, "numeric or act is null!");
        ril_errno = RIL_E_INVALID_ARGUMENTS;
        goto error;
    }

    if (msg->getData()->getDataLength()/sizeof(char *) == 3) {
        mode = pReqData[2];
        if (mode != NULL) {
            if (strncmp(mode, "1", 1) == 0) {
                logD(LOG_TAG, "Semi auto network selection mode");
                isSemiAutoMode = true;
            } else if (strncmp(mode, "2", 1) == 0) {
                logD(LOG_TAG, "none blocking Auto Mode");
                noneBlockAutoMode = true;
            }
        }
    }

    len = strlen(numeric_code);

    if (len == 6 && (strncmp(numeric_code, "123456", len) == 0)) {
        logE(LOG_TAG, "numeric is incorrect %s", numeric_code);
        ril_errno = RIL_E_INVALID_ARGUMENTS;
        goto error;
    }
    if (noneBlockAutoMode) {
        p_response = atSendCommand("AT+EOPS=0");
        err = p_response->getError();
        if (!(err < 0 || p_response->getSuccess() == 0)) {
            ril_errno = RIL_E_SUCCESS;
            mdEops[m_slot_id]->mode = 0;
        }
    } else if (len == 5 || len == 6) {
        // check if the numeric code is valid digit or not
        for(i = 0; i < len ; i++) {
            if( numeric_code[i] < '0' || numeric_code[i] > '9')
                break;
        }

        if (i == len) {
            if (strlen(act) == 1 && '0' <= act[0] && act[0] <= '9') {
                if(isSemiAutoMode == true) {
                    p_response = atSendCommand(String8::format("AT+EOPS=5, 2, \"%s\", %s", numeric_code, act));
                } else {
                    mdEops[m_slot_id]->mode = 1;
                    p_response = atSendCommand(String8::format("AT+COPS=1, 2, \"%s\", %s", numeric_code, act));
                }
            } else {
                if (isSemiAutoMode == true) {
                    p_response = atSendCommand(String8::format("AT+EOPS=5, 2, \"%s\"", numeric_code));
                } else {
                    mdEops[m_slot_id]->mode = 1;
                    p_response = atSendCommand(String8::format("AT+COPS=1, 2, \"%s\"", numeric_code));
                }
            }
            err = p_response->getError();
            if (!(err < 0 || p_response->getSuccess() == 0)) {
                ril_errno = RIL_E_SUCCESS;
            }
        } else {
            logE(LOG_TAG, "the numeric code contains invalid digits");
        }
    } else {
        logE(LOG_TAG, "the data length is invalid for Manual Selection");
    }

error:
    response = RfxMclMessage::obtainResponse(msg->getId(), ril_errno,
            RfxVoidData(), msg, false);
    // response to TeleCore
    responseToTelCore(response);
}

void RmcNetworkRequestHandler::requestSetBandMode(const sp<RfxMclMessage>& msg) {
    int req, err, gsm_band, umts_band;
    unsigned int lte_band_1_32, lte_band_33_64;
    char *cmd;
    RIL_Errno ril_errno = RIL_E_REQUEST_NOT_SUPPORTED;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> resp;
    int *pInt = (int *)msg->getData()->getData();

    req = pInt[0];
    switch (req) {
        case BM_AUTO_MODE: //"unspecified" (selected by baseband automatically)
            gsm_band = 0xff;
            umts_band = 0xffff;
            break;
        case BM_EURO_MODE: //"EURO band" (GSM-900 / DCS-1800 / WCDMA-IMT-2000)
            gsm_band = GSM_BAND_900 | GSM_BAND_1800;
            umts_band = UMTS_BAND_I;
            break;
        case BM_US_MODE: //"US band" (GSM-850 / PCS-1900 / WCDMA-850 / WCDMA-PCS-1900)
            gsm_band = GSM_BAND_850 | GSM_BAND_1900;
            umts_band = UMTS_BAND_II | UMTS_BAND_V;
            break;
        case BM_JPN_MODE: //"JPN band" (WCDMA-800 / WCDMA-IMT-2000)
            gsm_band = 0;
            umts_band = UMTS_BAND_I | UMTS_BAND_VI;
            break;
        case BM_AUS_MODE: //"AUS band" (GSM-900 / DCS-1800 / WCDMA-850 / WCDMA-IMT-2000)
            gsm_band = GSM_BAND_900 | GSM_BAND_1800;
            umts_band = UMTS_BAND_I | UMTS_BAND_V;
            break;
        case BM_AUS2_MODE: //"AUS band 2" (GSM-900 / DCS-1800 / WCDMA-850)
            gsm_band = GSM_BAND_900 | GSM_BAND_1800;
            umts_band = UMTS_BAND_V;
            break;
        case BM_40_BROKEN:
        case BM_CELLULAR_MODE: //"Cellular (800-MHz Band)"
        case BM_PCS_MODE: //"PCS (1900-MHz Band)"
        case BM_CLASS_3: //"Band Class 3 (JTACS Band)"
        case BM_CLASS_4: //"Band Class 4 (Korean PCS Band)"
        case BM_CLASS_5: //"Band Class 5 (450-MHz Band)"
        case BM_CLASS_6: // "Band Class 6 (2-GMHz IMT2000 Band)"
        case BM_CLASS_7: //"Band Class 7 (Upper 700-MHz Band)"
        case BM_CLASS_8: //"Band Class 8 (1800-MHz Band)"
        case BM_CLASS_9: //"Band Class 9 (900-MHz Band)"
        case BM_CLASS_10: //"Band Class 10 (Secondary 800-MHz Band)"
        case BM_CLASS_11: //"Band Class 11 (400-MHz European PAMR Band)"
        case BM_CLASS_15: //"Band Class 15 (AWS Band)"
        case BM_CLASS_16: //"Band Class 16 (US 2.5-GHz Band)"
        default:
            gsm_band = -1;
            umts_band = -1;
            break;
    }

    if (gsm_band != -1 && umts_band != -1) {
        /******************************************************
        * If the modem doesn't support certain group of bands, ex. GSM or UMTS
        * It might just ignore the parameter.
        *******************************************************/
        p_response = atSendCommand(String8::format("AT+EPBSE=%d, %d", gsm_band, umts_band));
        if (p_response->getError() >= 0 && p_response->getSuccess() != 0) {
            ril_errno = RIL_E_SUCCESS;
        }
    } else if (req == BM_40_BROKEN) {
        lte_band_1_32 = pInt[1];
        lte_band_33_64 = pInt[2];
        p_response = atSendCommand(String8::format("AT+EPBSE=,,%u,%u", lte_band_1_32, lte_band_33_64));
        if (p_response->getError() >= 0 && p_response->getSuccess() != 0) {
            ril_errno = RIL_E_SUCCESS;
        }
    } else if (req == BM_FOR_DESENSE_RADIO_ON || req == BM_FOR_DESENSE_RADIO_OFF
            || req == BM_FOR_DESENSE_RADIO_ON_ROAMING || req == BM_FOR_DESENSE_B8_OPEN) {
        requestQueryCurrentBandMode();
        int c2k_radio_on;
        int need_config_umts = 0;
        int force_switch = pInt[1];
        gsm_band = bands[0];
        umts_band = bands[1];
        lte_band_1_32 = bands[2];
        lte_band_33_64 = bands[3];
        logE(LOG_TAG, "BM FOR DESENCE, gsm_band:%d, umts_band : %d, lte_band_1_32 : %d, lte_band_33_64: %d, req: %d ",
                gsm_band, umts_band, lte_band_1_32, lte_band_33_64, req);
        if (req == BM_FOR_DESENSE_RADIO_ON) {
            if (umts_band & 0x00000080) {
                need_config_umts = 1;
                umts_band = umts_band & 0xffffff7f;
            }
        } else {
            if ((umts_band & 0x00000080) == 0) {
                need_config_umts = 1;
                umts_band = umts_band | 0x00000080;
            }
        }
        if (req == BM_FOR_DESENSE_RADIO_OFF) {
            c2k_radio_on = 0;
        } else {
            c2k_radio_on = 1;
        }
        logE(LOG_TAG, "BM FOR DESENCE, need_config_umts: %d, force_switch : %d", need_config_umts, force_switch);
        if (need_config_umts == 1 || force_switch == 1) {
            int skipDetach = 0;
            int detachCount = 0;
            while (skipDetach == 0 && detachCount < 10) {
                if (req == BM_FOR_DESENSE_B8_OPEN) {
                    p_response = atSendCommand(String8::format("AT+EPBSE=%d,%d,%d,%d", gsm_band, umts_band,
                        lte_band_1_32, lte_band_33_64));
                } else {
                    p_response = atSendCommand(String8::format("AT+EPBSE=%d,%d,%d,%d,%d", gsm_band, umts_band,
                        lte_band_1_32, lte_band_33_64, c2k_radio_on));
                }
                if (p_response->getError() >= 0 && p_response->getSuccess() != 0) {
                    logE(LOG_TAG, "Set band mode: success");
                    skipDetach = 1;
                    ril_errno = RIL_E_SUCCESS;
                } else {
                    detachCount++;
                    logE(LOG_TAG, "Set band mode: fail, count=%d", detachCount);
                    sleep(1);
                }
            }
        }
    }
    resp = RfxMclMessage::obtainResponse(msg->getId(), ril_errno,
            RfxVoidData(), msg, false);
    responseToTelCore(resp);
}

void RmcNetworkRequestHandler::requestQueryCurrentBandMode() {
    int err, gsm_band, umts_band;
    sp<RfxAtResponse> p_response;
    RfxAtLine* line;

    p_response = atSendCommandSingleline("AT+EPBSE?", "+EPBSE:");
    if (p_response->getError() < 0 || p_response->getSuccess() == 0) {
        logE(LOG_TAG, "Query current band mode: fail, err=%d", p_response->getError());
        return;
    }

    line = p_response->getIntermediates();

    line->atTokStart(&err);
    if (err < 0) return;

    // get supported GSM bands
    gsm_band = line->atTokNextint(&err);
    if (err < 0) return;

    // get supported UMTS bands
    umts_band = line->atTokNextint(&err);
    if (err < 0) return;

    bands[0] = gsm_band;
    bands[1] = umts_band;
    bands[2] = line->atTokNextint(&err);
    bands[3] = line->atTokNextint(&err);
    logE(LOG_TAG, "requestQueryCurrentBandMode, gsm_band:%d, umts_band : %d, lte_band_1_32 : %d, lte_band_33_64: %d",
            bands[0], bands[1], bands[2], bands[3]);
}

void RmcNetworkRequestHandler::requestQueryAvailableBandMode(const sp<RfxMclMessage>& msg) {
    int err, gsm_band, umts_band;
    int band_mode[10], index=1;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> resp;
    RfxAtLine* line;

    p_response = atSendCommandSingleline("AT+EPBSE?", "+EPBSE:");

    if (p_response->getError() < 0 || p_response->getSuccess() == 0)
        goto error;

    line = p_response->getIntermediates();

    line->atTokStart(&err);
    if (err < 0) goto error;

    // get supported GSM bands
    gsm_band = line->atTokNextint(&err);
    if (err < 0) goto error;

    // get supported UMTS bands
    umts_band = line->atTokNextint(&err);
    if (err < 0) goto error;

    //0 for "unspecified" (selected by baseband automatically)
    band_mode[index++] = BM_AUTO_MODE;

    if (gsm_band !=0 || umts_band != 0) {
        // 1 for "EURO band" (GSM-900 / DCS-1800 / WCDMA-IMT-2000)
        if ((gsm_band == 0 || (gsm_band | GSM_BAND_900 | GSM_BAND_1800) == gsm_band) &&
                (umts_band == 0 || (umts_band | UMTS_BAND_I) == umts_band)) {
            band_mode[index++] = BM_EURO_MODE;
        }

        // 2 for "US band" (GSM-850 / PCS-1900 / WCDMA-850 / WCDMA-PCS-1900)
        if ((gsm_band == 0 || (gsm_band | GSM_BAND_850 | GSM_BAND_1900) == gsm_band) &&
                (umts_band == 0 || (umts_band | UMTS_BAND_II | UMTS_BAND_V) == umts_band)) {
            band_mode[index++] = BM_US_MODE;
        }

        // 3 for "JPN band" (WCDMA-800 / WCDMA-IMT-2000)
        if ((umts_band | UMTS_BAND_I | UMTS_BAND_VI) == umts_band) {
            band_mode[index++] = BM_JPN_MODE;
        }

        // 4 for "AUS band" (GSM-900 / DCS-1800 / WCDMA-850 / WCDMA-IMT-2000)
        if ((gsm_band == 0 || (gsm_band | GSM_BAND_900 | GSM_BAND_1800)==gsm_band) &&
                (umts_band == 0 || (umts_band | UMTS_BAND_I | UMTS_BAND_V)==umts_band)) {
            band_mode[index++] = BM_AUS_MODE;
        }

        // 5 for "AUS band 2" (GSM-900 / DCS-1800 / WCDMA-850)
        if ((gsm_band == 0 || (gsm_band | GSM_BAND_900 | GSM_BAND_1800)==gsm_band) &&
                (umts_band == 0 || (umts_band | UMTS_BAND_V)==umts_band)) {
            band_mode[index++] = BM_AUS2_MODE;
        }
    }
    band_mode[0] = index - 1;
    resp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxIntsData(band_mode, index), msg, false);
    responseToTelCore(resp);
    return;

error:
    resp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
            RfxVoidData(), msg, false);
    responseToTelCore(resp);
}


void RmcNetworkRequestHandler::requestGetNeighboringCellIds(const sp<RfxMclMessage>& msg) {
    int err, skip, nt_type;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> resp;
    RfxAtLine* line;

    int rat,rssi,ci,lac,psc;
    int i = 0;
    int j = 0;
    RIL_NeighboringCell nbr[6];
    RIL_NeighboringCell *p_nbr[6];

    // logD(LOG_TAG, "Enter requestGetNeighboringCellIds()");
    p_response = atSendCommandMultiline("AT+ENBR", "+ENBR:");
    err = p_response->getError();

    if (err < 0 || p_response->getSuccess() == 0)
        goto error;

    line = p_response->getIntermediates();
    while(line != NULL) {
        line->atTokStart(&err);
        if (err < 0) goto error;

        rat = line->atTokNextint(&err);
        if (err < 0) goto error;

        rssi = line->atTokNextint(&err);
        if (err < 0) goto error;

        if (((rat == 1) && (rssi < 0 || rssi > 31) && (rssi != 99))
                || ((rat == 2) && (rssi < 0 || rssi > 91))) {
            logE(LOG_TAG, "The rssi of the %d-th is invalid: %d", i, rssi);
            goto error;
        }

        nbr[i].rssi = rssi;

        if (rat == 1) {
            ci = line->atTokNextint(&err);
            if (err < 0) goto error;

            lac = line->atTokNextint(&err);
            if (err < 0) goto error;

            err = asprintf(&nbr[i].cid, "%04X%04X", lac, ci);
            if (err < 0) {
                logE(LOG_TAG, "Using asprintf and getting ERROR");
                goto error;
            }
            logD(LOG_TAG, "CURRENT_ENBR_RAT 1 :: NC[%d], rssi:%d, cid:%s", i, nbr[i].rssi, nbr[i].cid);
        } else if (rat == 2) {
            psc = line->atTokNextint(&err);
            if (err < 0) goto error;

            err = asprintf(&nbr[i].cid, "%08X", psc);
            if (err < 0) {
                logE(LOG_TAG, "Using asprintf and getting ERROR");
                goto error;
            }
            logD(LOG_TAG, "CURRENT_ENBR_RAT 3 :: NC[%d], rssi:%d, psc:%d", i, rssi, psc);
        } else {
            goto error;
        }
        p_nbr[i] = &nbr[i];
        i++;
        line = line->getNext();
    }

    resp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxNeighboringCellData(p_nbr, i), msg, false);
    responseToTelCore(resp);
    for(j=0;j<i;j++)
        free(nbr[j].cid);
    return;

error:
    logE(LOG_TAG, "requestGetNeighboringCellIds has error occur!!");
    resp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
            RfxVoidData(), msg, false);
    responseToTelCore(resp);
    for(j=0;j<i;j++)
        free(nbr[j].cid);
}

void RmcNetworkRequestHandler::requestSetLocationUpdates(const sp<RfxMclMessage>& msg) {
    int enabled;
    RIL_Errno err_no = RIL_E_SUCCESS;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> resp;
    int *pInt = (int *)msg->getData()->getData();

    enabled = pInt[0];
    if (enabled == 1 || enabled == 0) {
        // EREG=3, full information; EREG=5 or 9, only <state> and <eAct>
        if (support_ereg_5 == 1) {
            p_response = atSendCommand(String8::format("AT+EREG=%d", enabled ? 3 : 9));
            if (p_response->getError() < 0 || p_response->getSuccess() == 0) {
                p_response = atSendCommand(String8::format("AT+EREG=%d", enabled ? 3 : 5));
                if (p_response->getError() < 0 || p_response->getSuccess() == 0) {
                    err_no = RIL_E_SIM_ABSENT;
                }
            }
        } else {
            // when egreg=5 is not supported.
            if (enabled == 1) {
                // turn off the old optimization - TODO: remove it someday
                p_response = atSendCommand("AT+ECREG=0");
                if (p_response->getError() < 0 || p_response->getSuccess() == 0) {
                    err_no = RIL_E_SIM_ABSENT;
                    logD(LOG_TAG, "There is something wrong with the exectution of AT+ECREG=0");
                }
                /* enable EREG URC with full information */
                p_response = atSendCommand("AT+EREG=3");
                if (p_response->getError() < 0 || p_response->getSuccess() == 0) {
                    err_no = RIL_E_SIM_ABSENT;
                    logD(LOG_TAG, "There is something wrong with the exectution of AT+EREG=3");
                }
            } else {
                // enable old EREG optimization by ECREG=1
                p_response = atSendCommand("AT+ECREG=1");
                if (p_response->getError() < 0 || p_response->getSuccess() == 0) {
                    err_no = RIL_E_SIM_ABSENT;
                    logD(LOG_TAG, "There is something wrong with the exectution of AT+ECREG=1");
                }
            }
        }
    }

    resp = RfxMclMessage::obtainResponse(msg->getId(), err_no,
            RfxVoidData(), msg, false);
    responseToTelCore(resp);
}

void RmcNetworkRequestHandler::requestGetCellInfoList(const sp<RfxMclMessage>& msg) {
    int err = 0;
    int num = 0;
    RIL_CellInfo_v12 *response = NULL;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> resp;
    RfxAtLine* line;

    p_response = atSendCommandSingleline("AT+ECELL", "+ECELL:");

    // +ECELL: <num_of_cell>...
    if (p_response->getError() < 0 || p_response->getSuccess() == 0)
        goto error;

    line = p_response->getIntermediates();

    line->atTokStart(&err);
    if (err < 0) goto error;

    num = line->atTokNextint(&err);
    if (err < 0) goto error;
    if (num < 1) {
        logD(LOG_TAG, "No cell info listed, num=%d", num);
        goto error;
    }
    // logD(LOG_TAG, "Cell info listed, number=%d",num);

    response = (RIL_CellInfo_v12 *) alloca(num * sizeof(RIL_CellInfo_v12));
    memset(response, 0, num * sizeof(RIL_CellInfo_v12));

    err = getCellInfoListV12(line, num, response);
    if (err < 0) goto error;

    resp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxCellInfoData(response, num * sizeof(RIL_CellInfo_v12)), msg, false);
    responseToTelCore(resp);
    freeOperatorNameOfCellInfo(num, response);
    return;

error:
    // if error and last valid PLMN is available, use it for fwk's country code.
    char last_plmn[8];
    int mnc_len = 2;
    if (getLastValidPlmn(last_plmn, 8)) {
        if (strlen(last_plmn) == 6) mnc_len = 3;
        char mcc_str[4] = {0};
        char mnc_str[4] = {0};
        char empty_string = '\0';
        strncpy(mcc_str, last_plmn, 3);
        strncpy(mnc_str, last_plmn+3, (strlen(last_plmn)-3));

        num = 1;
        response = (RIL_CellInfo_v12 *) alloca(num * sizeof(RIL_CellInfo_v12));
        memset(response, 0, num * sizeof(RIL_CellInfo_v12));
        response[0].cellInfoType = RIL_CELL_INFO_TYPE_GSM;
        response[0].registered = 0;
        response[0].timeStampType = RIL_TIMESTAMP_TYPE_UNKNOWN;
        response[0].timeStamp = 0;
        response[0].connectionStatus = NONE_SERVING;
        response[0].CellInfo.gsm.cellIdentityGsm.mnc_len = mnc_len;
        response[0].CellInfo.gsm.cellIdentityGsm.mcc = atoi(mcc_str);
        response[0].CellInfo.gsm.cellIdentityGsm.mnc = atoi(mnc_str);
        response[0].CellInfo.gsm.cellIdentityGsm.operName.long_name = &empty_string;
        response[0].CellInfo.gsm.cellIdentityGsm.operName.short_name = &empty_string;
        resp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
                RfxCellInfoData(response, num * sizeof(RIL_CellInfo_v12)), msg, false);
        responseToTelCore(resp);
    } else {
        logE(LOG_TAG, "requestGetCellInfoList must never return error when radio is on");
        resp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_NO_NETWORK_FOUND,
                RfxVoidData(), msg, false);
        responseToTelCore(resp);
    }
}

void RmcNetworkRequestHandler::requestSetCellInfoListRate(const sp<RfxMclMessage>& msg) {
    int time = -1;
    RIL_Errno err_no = RIL_E_GENERIC_FAILURE;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> resp;
    int *pInt = (int *)msg->getData()->getData();

    time = pInt[0];

    // logE(LOG_TAG, "requestSetCellInfoListRate:%d", time);
    if (time == 0) {
        p_response = atSendCommand(String8::format("AT+ECELL=1"));
    } else if (time > 0) {
        p_response = atSendCommand(String8::format("AT+ECELL=0"));
    } else {
        goto finish;
    }

    if (p_response->getError() >= 0 &&
            p_response->getSuccess() > 0) {
        err_no = RIL_E_SUCCESS;
    }

finish:
    resp = RfxMclMessage::obtainResponse(msg->getId(), err_no,
            RfxVoidData(), msg, false);
    responseToTelCore(resp);
}

void RmcNetworkRequestHandler::requestGetPOLCapability(const sp<RfxMclMessage>& msg) {
    int err;
    char *mClose, *mOpen, *mHyphen;
    int result[4] = {0};
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> resp;
    RfxAtLine* line;

    p_response = atSendCommandSingleline("AT+CPOL=?", "+CPOL:");

    if (p_response->getError() < 0) {
        logE(LOG_TAG, "requestGetPOLCapability Fail");
        goto error;
    }

    if (p_response->getSuccess() == 0) {
        logE(LOG_TAG, "CME ERROR: %d/n", p_response->atGetCmeError());
        goto error;
    }

    //+CPOL: (<bIndex>-<eIndex>), (<bformatValue>-<eformatValue>)
    line = p_response->getIntermediates();

    // AT< +CPOL: (0-39), (0-2)
    line->atTokStart(&err);
    if (err < 0) goto error;

    // AT< +CPOL: (0-39), (0-2)
    //            ^
    mOpen = line->atTokChar(&err);
    if (err < 0) goto error;

    mHyphen = strchr(mOpen, '-');
    if (mHyphen != NULL && mOpen < mHyphen ) {
        // AT< +CPOL: (0-39), (0-2)
        //             ^
        result[0] = strtol((mOpen+1), NULL, 10);
        logD(LOG_TAG, "requestGetPOLCapability result 0: %d", result[0]);
    } else {
        goto error;
    }

    mClose = strchr(mHyphen, ')');
    if (mClose != NULL && mHyphen < mClose) {
        // AT< +CPOL: (0-39), (0-2)
        //               ^^
        result[1] = strtol((mHyphen+1), NULL, 10);
        logD(LOG_TAG, "requestGetPOLCapability result 1: %d", result[1]);
    } else {
        goto error;
    }

    // AT< +CPOL: (0-39), (0-2)
    //                    ^
    mOpen = line->atTokChar(&err);
    if (err < 0) goto error;

    mHyphen = strchr(mOpen, '-');
    if (mHyphen != NULL && mOpen < mHyphen ) {
        // AT< +CPOL: (0-39), (0-2)
        //                     ^
        result[2] = strtol((mOpen+1), NULL, 10);
        logD(LOG_TAG, "requestGetPOLCapability result 2: %d", result[2]);
    } else {
        goto error;
    }

    mClose = strchr(mHyphen, ')');
    if (mClose != NULL && mHyphen < mClose) {
        // AT< +CPOL: (0-39), (0-2)
        //                       ^
        result[3] = strtol((mHyphen+1), NULL, 10);
        logD(LOG_TAG, "requestGetPOLCapability result 3: %d", result[3]);
    } else {
        goto error;
    }

    logD(LOG_TAG, "requestGetPOLCapability: %d %d %d %d", result[0],
            result[1], result[2], result[3]);

    resp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxIntsData(result, 4), msg, false);
    responseToTelCore(resp);
    return;

error:
    logD(LOG_TAG, "requestGetPOLCapability: goto error");
    resp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
            RfxVoidData(), msg, false);
    responseToTelCore(resp);
}

void RmcNetworkRequestHandler::requestGetPOLList(const sp<RfxMclMessage>& msg) {
    int err, i, j, count, len, nAct, tmpAct;
    char **response = NULL;
    char *tmp, *block_p = NULL;
    sp<RfxAtResponse> p_response;
    RfxAtLine *p_cur;
    sp<RfxMclMessage> resp;

    p_response = atSendCommandMultiline("AT+CPOL?", "+CPOL:");
    err = p_response->getError();
    if (err < 0 || p_response->getSuccess() == 0) {
        goto error;
    }

    /* count the entries */
    for (count = 0, p_cur = p_response->getIntermediates()
            ; p_cur != NULL
            ; p_cur = p_cur->getNext()) {
        count++;
    }
    logD(LOG_TAG, "requestGetPOLList!! count is %d", count);

    response = (char **) calloc(1, sizeof(char*) * count *4); // for string, each one is 25 bytes
    /* In order to support E-UTRAN, nAct will be 2 digital,
    changed from 60 to 62 for addition 1 digital and buffer.*/
    block_p = (char *) calloc(1, count* sizeof(char)*62);

    if (response == NULL || block_p == NULL) {
        logE(LOG_TAG, "requestGetPOLList calloc fail");
        goto error;
    }
    //+CPOL: <index>, <format>, <oper>, <GSM_Act>, <GSM_Compact_Act>, <UTRAN_Act>, <E-UTRAN Act>
    for (i = 0,j=0, p_cur = p_response->getIntermediates()
            ; p_cur != NULL
            ; p_cur = p_cur->getNext(), i++,j+=4) {
        logD(LOG_TAG, "requestGetPOLList!! line is %s", p_cur->getLine());

        p_cur->atTokStart(&err);
        if (err < 0) goto error;

        /* get index*/
        tmp = p_cur->atTokNextstr(&err);
        if (err < 0) goto error;

        response[j+0] = &block_p[i*62];
        response[j+1] = &block_p[i*62+8];
        response[j+2] = &block_p[i*62+10];
        response[j+3] = &block_p[i*62+58];

        sprintf(response[j+0], "%s", tmp);

        logD(LOG_TAG, "requestGetPOLList!! index is %s",response[j+0]);
        /* get format*/
        tmp = p_cur->atTokNextstr(&err);
        if (err < 0) goto error;

        sprintf(response[j+1], "%s", tmp);
        logD(LOG_TAG, "requestGetPOLList!! format is %s",response[j+1]);
        /* get oper*/
        tmp = p_cur->atTokNextstr(&err);
        if (err < 0) goto error;

        sprintf(response[j+2], "%s", tmp);
        logD(LOG_TAG, "requestGetPOLList!! oper is %s",response[j+2]);
        nAct = 0;

        if(p_cur->atTokHasmore()) {
            /* get <GSM AcT> */
            tmpAct = p_cur->atTokNextint(&err);
            if (err < 0) goto error;

            if (tmpAct == 1) {
                nAct = 1;
            }

            /*get <GSM compact AcT> */
            tmpAct = p_cur->atTokNextint(&err);
            if (err < 0) goto error;

            if(tmpAct == 1) {
                nAct |= 0x02;
            }

            /*get <UTRAN AcT> */
            tmpAct = p_cur->atTokNextint(&err);
            if (err < 0) goto error;

            if (tmpAct == 1) {
                nAct |= 0x04;
            }

            /*get <E-UTRAN AcT> */
            if (p_cur->atTokHasmore()) {
                logD(LOG_TAG, "get E-UTRAN AcT");
                tmpAct = p_cur->atTokNextint(&err);
                if (err < 0) goto error;

                if(tmpAct == 1) {
                    nAct |= 0x08;
                }
            }
        }
        /* ALPS00368351 To distinguish SIM file without <AcT> support, we set AcT to zero */
        // if(nAct == 0) { nAct = 1;} // No act value for SIM. set to GSM
        logD(LOG_TAG, "Act = %d",nAct);
        sprintf(response[j+3], "%d", nAct);
        logD(LOG_TAG, "requestGetPOLList!! act is %s",response[j+3]);
    }
    resp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxStringsData(response, count*4), msg, false);
    responseToTelCore(resp);
    free(response);
    free(block_p);
    return;

error:
    logE(LOG_TAG, "requestGetPOLList return error");
    if (response) {
        free(response);
    }
    if (block_p) {
        free(block_p);
    }
    resp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
            RfxVoidData(), msg, false);
    responseToTelCore(resp);
}

void RmcNetworkRequestHandler::requestSetPOLEntry(const sp<RfxMclMessage>& msg) {
    int i;
    int nAct[4] = {0};
    int nActTmp = 0;
    const char **strings = (const char **)msg->getData()->getData();
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;

    sp<RfxAtResponse> p_response;
    RfxAtLine *p_cur;
    sp<RfxMclMessage> resp;

    if (msg->getData()->getDataLength() < (int)(3 * sizeof(char*))) {
        logE(LOG_TAG, "requestSetPOLEntry no enough input params. datalen is %d, size of char* is %zu",
                msg->getData()->getDataLength(), sizeof(char*));
        p_response = atSendCommand(String8::format("AT+CPOL=%s", strings[0]));
    } else if (strings[1] == NULL || strcmp("", strings[1]) == 0) { // no PLMN, then clean the entry
        p_response = atSendCommand(String8::format("AT+CPOL=%s", strings[0]));
    } else {
        nActTmp = atoi(strings[2]);
        logD(LOG_TAG, "requestSetPOLEntry Act = %d", nActTmp);

        for (i = 0; i < 4; i++) {
            if (((nActTmp >> i) & 1) == 1) {
                logD(LOG_TAG, "i = %d",i);
                nAct[i] = 1;
            }
        }

        /* ALPS00368351: To update file without <AcT> support, modem suggest not to set any nAcT parameter */
        if (nActTmp == 0) {
            logD(LOG_TAG, "requestSetPOLEntry no Act assigned,strings[2]=%s",strings[2]);
            p_response = atSendCommand(String8::format("AT+CPOL=%s,2,\"%s\"", strings[0], strings[1]));
        } else {
            logD(LOG_TAG, "R8, MOLY and LR9 can supoort 7 arguments");
            p_response = atSendCommand(String8::format("AT+CPOL=%s,2,\"%s\",%d,%d,%d,%d", strings[0], strings[1], nAct[0], nAct[1], nAct[2], nAct[3]));
        }
    }
    if (p_response->getError() < 0) {
        logE(LOG_TAG, "requestSetPOLEntry Fail");
        goto finish;
    }

    if (p_response->getSuccess() == 0) {
        switch (p_response->atGetCmeError()) {
            logD(LOG_TAG, "p_response = %d/n", p_response->atGetCmeError());
            case CME_SUCCESS:
                ret = RIL_E_GENERIC_FAILURE;
            break;
            case CME_UNKNOWN:
                logD(LOG_TAG, "p_response: CME_UNKNOWN");
            break;
            default:
            break;
        }
    } else {
        ret = RIL_E_SUCCESS;
    }

finish:
    resp = RfxMclMessage::obtainResponse(msg->getId(), ret,
            RfxVoidData(), msg, false);
    responseToTelCore(resp);
}

void RmcNetworkRequestHandler::requestSetCdmaRoamingPreference(const sp<RfxMclMessage>& msg) {
    int err;
    int reqRoamingType = -1;
    int roamingType = -1;
    RIL_Errno ril_errno = RIL_E_REQUEST_NOT_SUPPORTED;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> response;

    int *pInt = (int *) msg->getData()->getData();
    reqRoamingType = pInt[0];

    // AT$ROAM=<type>
    // <type>=0: set the device to Sprint only mode
    // <type>=1: set the device to automatic mode

    if (reqRoamingType == CDMA_ROAMING_MODE_HOME) {
        // for Home Networks only
        roamingType = 0;
    } else if (reqRoamingType == CDMA_ROAMING_MODE_ANY) {
        // for Roaming on Any Network
        roamingType = 1;
    } else {
        logE(LOG_TAG, "requestSetCdmaRoamingPreference, Not support reqRoamingType=%d", reqRoamingType);
    }

    if (roamingType >= 0) {
        p_response = atSendCommand(String8::format("AT$ROAM=%d", roamingType));
        err = p_response->getError();
        if (err != 0 || p_response == NULL || p_response->getSuccess() == 0) {
            logE(LOG_TAG, "requestSetCdmaRoamingPreference error, reqRoamingType=%d", reqRoamingType);
        } else {
            ril_errno = RIL_E_SUCCESS;
        }
    }

    response = RfxMclMessage::obtainResponse(msg->getId(), ril_errno, RfxVoidData(), msg, false);
    responseToTelCore(response);
}

void RmcNetworkRequestHandler::requestQueryCdmaRoamingPreference(const sp<RfxMclMessage>& msg) {
    int err;
    int roamingType = -1;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> response;
    RfxAtLine* line;

    // AT$ROAM=<type>
    // <type>=0: set the device to Sprint only mode
    // <type>=1: set the device to automatic mode

    p_response = atSendCommandSingleline("AT$ROAM?", "$ROAM:");
    err = p_response->getError();
    if (err != 0 ||
            p_response == NULL ||
            p_response->getSuccess() == 0 ||
            p_response->getIntermediates() == NULL) goto error;

    line = p_response->getIntermediates();

    line->atTokStart(&err);
    if (err < 0) goto error;

    // <type>
    roamingType = line->atTokNextint(&err);
    if (err < 0) goto error;

    if (roamingType == 0) {
        // for Home Networks only
        roamingType = CDMA_ROAMING_MODE_HOME;
    } else if (roamingType == 1) {
        // for Roaming on Any Network
        roamingType = CDMA_ROAMING_MODE_ANY;
    } else {
        logE(LOG_TAG, "requestQueryCdmaRoamingPreference, Not support roamingType=%d", roamingType);
        goto error;
    }

    response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxIntsData(&roamingType, 1), msg, false);
    responseToTelCore(response);
    return;
error:
    logE(LOG_TAG, "requestQueryCdmaRoamingPreference error");
    response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_REQUEST_NOT_SUPPORTED,
            RfxIntsData(&roamingType, 1), msg, false);
    responseToTelCore(response);
}

void RmcNetworkRequestHandler::updateSignalStrength()
{
    sp<RfxAtResponse> p_response;
    RfxAtLine* p_cur;
    int err_cnt = 0;
    int err;
    sp<RfxMclMessage> urc;
    int len = sizeof(RIL_SIGNAL_STRENGTH_CACHE)/sizeof(int);
    int *resp = new int[len];
    pthread_mutex_lock(&s_signalStrengthMutex[m_slot_id]);

    RIL_RadioState state =
        (RIL_RadioState) getMclStatusManager()->getIntValue(RFX_STATUS_KEY_RADIO_STATE);
    if (RADIO_STATE_OFF == state) {
        resetSignalStrengthCache(signal_strength_cache[m_slot_id], CACHE_GROUP_ALL);
    } else {
        // send AT command
        p_response = atSendCommandMultiline("AT+ECSQ", "+ECSQ:");

        // check error
        err = p_response->getError();
        if (err != 0 ||
                p_response == NULL ||
                p_response->getSuccess() == 0 ||
                p_response->getIntermediates() == NULL) {
            goto error;
        }

        for (p_cur = p_response->getIntermediates()
             ; p_cur != NULL
             ; p_cur = p_cur->getNext()
             ) {
            err = getSignalStrength(p_cur);

            if (err != 0)
                continue;
        }
    }
    // copy signal strength cache to int array
    memcpy(resp, signal_strength_cache[m_slot_id], len*sizeof(int));
    pthread_mutex_unlock(&s_signalStrengthMutex[m_slot_id]);

    printSignalStrengthCache((char*) __FUNCTION__);

    // returns the whole cache, including GSM, WCDMA, TD-SCDMA, CDMA, EVDO, LTE
    urc = RfxMclMessage::obtainUrc(RFX_MSG_URC_SIGNAL_STRENGTH,
            m_slot_id, RfxIntsData((void*)resp, len*sizeof(int)));
    responseToTelCore(urc);
    urc = RfxMclMessage::obtainUrc(RFX_MSG_URC_SIGNAL_STRENGTH_WITH_WCDMA_ECIO,
            m_slot_id, RfxIntsData((void*)resp, len*sizeof(int)));

    // response to TeleCore
    responseToTelCore(urc);
    delete [] resp;

    return;

error:
    pthread_mutex_unlock(&s_signalStrengthMutex[m_slot_id]);
    logE(LOG_TAG, "updateSignalStrength ERROR: %d", err);
    delete [] resp;
    return;
}

void RmcNetworkRequestHandler::setUnsolResponseFilterSignalStrength(bool enable)
{
    sp<RfxAtResponse> p_response;

    // The ePDG/RNS framework need to monitoring the LTE RSRP signal strength across the threshold.
    char threshold[RFX_PROPERTY_VALUE_MAX] = {0};

    if (enable) {
        // enable
        /* Enable get ECSQ URC */
        p_response = atSendCommand("AT+ECSQ=1");
        if (p_response->getError() != 0 || p_response->getSuccess() == 0)
            logW(LOG_TAG, "There is something wrong with the exectution of AT+ECSQ=1");

        // The ePDG/RNS framework need to monitoring the LTE RSRP signal strength across the threshold.
        // So we send command to adjust signaling threshold to MD1 whenever screen on/off.
        rfx_property_get("vendor.net.handover.thlte", threshold, "");
        /*
        if (strlen(threshold) == 0) {
            logD(LOG_TAG, "vendor.net.handover.thlte is empty");
        }
        */
    } else {
        // disable
        /* Disable get ECSQ URC */
        p_response = atSendCommand("AT+ECSQ=0");
        if (p_response->getError() != 0 || p_response->getSuccess() == 0)
            logW(LOG_TAG, "There is something wrong with the exectution of AT+ECSQ=0");

        // The ePDG/RNS framework need to monitoring the LTE RSRP signal strength across the threshold.
        // So we send command to adjust signaling threshold to MD1 whenever screen on/off.
        rfx_property_get("vendor.net.handover.thlte", threshold, "");
        if (strlen(threshold) != 0) {
            p_response = atSendCommand(String8::format("AT+ECSQ=3,3,%s", threshold));
            logD(LOG_TAG, "requestScreenState(), adjust signaling threshold %s", threshold);
            if (p_response->getError() != 0 || p_response->getSuccess() == 0)
                logW(LOG_TAG, "There is something wrong with the exectution of AT+ECSQ=3,3..");
        } else {
            // logD(LOG_TAG, "vendor.net.handover.thlte is empty");
        }
    }
}

void RmcNetworkRequestHandler::setUnsolResponseFilterNetworkState(bool enable)
{
    sp<RfxAtResponse> p_response;

    if (enable) {
        // enable; screen is on.

        if (support_ereg_5 == 0) {
            // turn off the old optimization - TODO: remove it someday
            p_response = atSendCommand("AT+ECREG=0");
            if (p_response->getError() != 0 || p_response->getSuccess() == 0)
                logW(LOG_TAG, "There is something wrong with the exectution of AT+ECREG=0");

            p_response = atSendCommand("AT+ECGREG=0");
            if (p_response->getError() != 0 || p_response->getSuccess() == 0)
                logW(LOG_TAG, "There is something wrong with the exectution of AT+ECGREG=0");

            p_response = atSendCommand("AT+ECEREG=0");
            if (p_response->getError() != 0 || p_response->getSuccess() == 0)
                logW(LOG_TAG, "There is something wrong with the exectution of AT+ECEREG=0");
            // turn off the old optimization
        }

        /* enable EREG URC with full information */
        p_response = atSendCommand("AT+EREG=3");
        if (p_response->getError() != 0 || p_response->getSuccess() == 0)
            logW(LOG_TAG, "There is something wrong with the exectution of AT+EREG=3");

        /* enable EGERG URC with full information */
        p_response = atSendCommand("AT+EGREG=3");
        if (p_response->getError() != 0 || p_response->getSuccess() == 0)
            logW(LOG_TAG, "There is something wrong with the exectution of AT+EGREG=3");

        /* Enable PSBEARER URC */
        p_response = atSendCommand("AT+PSBEARER=1");
        if (p_response->getError() != 0 || p_response->getSuccess() == 0)
            logW(LOG_TAG, "There is something wrong with the exectution of AT+PSBEARER=1");

        // AP phase out CEREG with newer 93MD. Still use it with older one.
        if (support_ereg_5 == 0) {
            /* enable CEREG */
            p_response = atSendCommand("AT+CEREG=3");
            if (p_response->getError() != 0 || p_response->getSuccess() == 0) {
                p_response = atSendCommand("AT+CEREG=2");
                if (p_response->getError() != 0 || p_response->getSuccess() == 0) {
                    logW(LOG_TAG, "There is something wrong with the exectution of AT+CEREG=2");
                }
            }
        }

        /* Enable ECSG URC */
        if (isFemtocellSupport()) {
            p_response = atSendCommand("AT+ECSG=4,1");
            if (p_response->getError() != 0 || p_response->getSuccess() == 0)
                logW(LOG_TAG, "There is something wrong with the exectution of AT+ECEREG=0");
        }

        /* Enable EMODCFG URC */
        if (isEnableModulationReport()) {
            p_response = atSendCommand("AT+EMODCFG=1");
            if (p_response->getError() != 0 || p_response->getSuccess() == 0)
                logW(LOG_TAG, "There is something wrong with the exectution of AT+EMODCFG=1");
        }

        /* Enable EREGINFO URC */
        p_response = atSendCommand("AT+EREGINFO=1");
        if (p_response->getError() != 0 || p_response->getSuccess() == 0)
            logW(LOG_TAG, "There is something wrong with the exectution of AT+EREGINFO=1");

        /* Query EIPRL URC */
        p_response = atSendCommand("AT+EIPRL?");
        if (p_response->getError() != 0 || p_response->getSuccess() == 0)
            // logW(LOG_TAG, "There is something wrong with the exectution of AT+EIPRL?");

        /* Enable EFCELL URC */
        if (isFemtocellSupport()) {
            p_response = atSendCommand("AT+EFCELL=1");
            if (p_response->getError() != 0 || p_response->getSuccess() == 0) {
                logW(LOG_TAG, "There is something wrong with the exectution of AT+EFCELL=1");
            } else {
                p_response = atSendCommand("AT+EFCELL?");
                if (p_response->getError() != 0 || p_response->getSuccess() == 0)
                    logW(LOG_TAG, "There is something wrong with the exectution of AT+EFCELL?");
            }
        }

        /* Enable EDEFROAM URC */
        p_response = atSendCommand("AT+EDEFROAM=1");
        if (p_response->getError() != 0 || p_response->getSuccess() == 0) {
            logW(LOG_TAG, "There is something wrong with the exectution of AT+EDEFROAM=1");
        } else {
            p_response = atSendCommand("AT+EDEFROAM?");
            if (p_response->getError() != 0 || p_response->getSuccess() == 0)
                logW(LOG_TAG, "There is something wrong with the exectution of AT+EDEFROAM?");
        }

        /* Enable EMCCMNC URC */
        if (supportMccMncUrc()) {
            p_response = atSendCommand("AT+EMCCMNC=1");
            if (p_response->getError() != 0 || p_response->getSuccess() == 0) {
                logW(LOG_TAG, "There is something wrong with the exectution of AT+EMCCMNC=1");
            }
        }
    } else {
        // disable; screen is off
        if (support_ereg_5 == 1) {
            /* enable EREG URC when <stat>&<eAct> changes */
            p_response = atSendCommand("AT+EREG=9");
            if (p_response->getError() != 0 || p_response->getSuccess() == 0) {
                p_response = atSendCommand("AT+EREG=5");
                if (p_response->getError() != 0 || p_response->getSuccess() == 0) {
                    logW(LOG_TAG, "There is something wrong with the exectution of AT+EREG=5");
                }
            }

            /* enable EGREG URC when <stat>&<eAct> changes */
            p_response = atSendCommand("AT+EGREG=9");
            if (p_response->getError() != 0 || p_response->getSuccess() == 0) {
                p_response = atSendCommand("AT+EGREG=5");
                if (p_response->getError() != 0 || p_response->getSuccess() == 0) {
                    logW(LOG_TAG, "There is something wrong with the exectution of AT+EGREG=5");
                }
            }
        }

        // TODO - remove the following codes which are for old 93MD.
        if (support_ereg_5 == 0) {
            // enable old EREG optimization by ECREG=1
            p_response = atSendCommand("AT+ECREG=1");
            if (p_response->getError() != 0 || p_response->getSuccess() == 0)
                logW(LOG_TAG, "There is something wrong with the exectution of AT+ECREG=1");
            // enable old EGREG optimization by ECGREG=1
            p_response = atSendCommand("AT+ECGREG=1");
            if (p_response->getError() != 0 || p_response->getSuccess() == 0)
                logW(LOG_TAG, "There is something wrong with the exectution of AT+ECGREG=1");
            // enable old CEREG optimization by ECEREG=1
            p_response = atSendCommand("AT+ECEREG=1");
            if (p_response->getError() != 0 || p_response->getSuccess() == 0)
                logW(LOG_TAG, "There is something wrong with the exectution of AT+ECEREG=1");
        }

        /* Disable PSBEARER URC */
        p_response = atSendCommand("AT+PSBEARER=0");
        if (p_response->getError() != 0 || p_response->getSuccess() == 0)
            logW(LOG_TAG, "There is something wrong with the exectution of AT+PSBEARER=0");

        if (isFemtocellSupport()) {
            /* Disable ECSG URC */
            p_response = atSendCommand("AT+ECSG=4,0");
            if (p_response->getError() != 0 || p_response->getSuccess() == 0)
                logW(LOG_TAG, "There is something wrong with the exectution of AT+ECSG=4,0");
        }

        /* Disable EMODCFG URC */
        if (isEnableModulationReport()) {
            p_response = atSendCommand("AT+EMODCFG=0");
            if (p_response->getError() != 0 || p_response->getSuccess() == 0)
                logW(LOG_TAG, "There is something wrong with the exectution of AT+EMODCFG=0");
        }

        /* Disable EREGINFO URC */
        p_response = atSendCommand("AT+EREGINFO=0");
        if (p_response->getError() != 0 || p_response->getSuccess() == 0)
            logW(LOG_TAG, "There is something wrong with the exectution of AT+EREGINFO=0");

        /* Disable EFCELL URC */
        if (isFemtocellSupport()) {
            p_response = atSendCommand("AT+EFCELL=0");
            if (p_response->getError() != 0 || p_response->getSuccess() == 0)
                logW(LOG_TAG, "There is something wrong with the exectution of AT+EFCELL=1");
        }

        /* Disable EDEFROAM URC */
        p_response = atSendCommand("AT+EDEFROAM=0");
        if (p_response->getError() != 0 || p_response->getSuccess() == 0)
            logW(LOG_TAG, "There is something wrong with the exectution of AT+EDEFROAM=0");

        /* Disable EMCCMNC URC */
        if (supportMccMncUrc()) {
            p_response = atSendCommand("AT+EMCCMNC=0");
            if (p_response->getError() != 0 || p_response->getSuccess() == 0) {
                logW(LOG_TAG, "There is something wrong with the exectution of AT+EMCCMNC=0");
            }
        }
    }
}

void RmcNetworkRequestHandler::setUnsolResponseFilterLinkCapacityEstimate(bool enable) {
    // We should not turn off link capacity update if one of the following condition is true.
    // 1. The device is charging.
    // 2. When the screen is on.
    // 3. When data tethering is on.
    // 4. When the update mode is IGNORE_SCREEN_OFF.
    // In all other cases, we turn off link capacity update.
    // AT+EXLCE=<mode> to turn on/off link capacity update.
    // <mode> = 0: disable; 1: enable;
    sp<RfxAtResponse> pAtResponse = atSendCommand(String8::format("AT+EXLCE=%d", enable));

    if (pAtResponse == NULL) {
        logE(LOG_TAG, "setUnsolResponseFilterLinkCapacityEstimate: Fail to set AT+EXLCE=%d", enable);
    } else if (pAtResponse->isAtResponseFail()) {
        logE(LOG_TAG, "setUnsolResponseFilterLinkCapacityEstimate: Fail to set AT+EXLCE=%d, ERROR:%d",
                enable, pAtResponse->atGetCmeError());
    }
}

void RmcNetworkRequestHandler::requestScreenState(const sp<RfxMclMessage>& msg) {
    /************************************
    * Control the URC: ECSQ,CREG,CGREG,CEREG
    * CIREG,PSBEARER,ECSG,EMODCFG,EREGINFO
    *************************************/

    int on_off, err;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> response;
    int *pInt = (int *)msg->getData()->getData();

    on_off = pInt[0];

    // The ePDG/RNS framework need to monitoring the LTE RSRP signal strength across the threshold.
    char threshold[RFX_PROPERTY_VALUE_MAX] = {0};

    if (on_off)
    {
        // screen is on

        setUnsolResponseFilterNetworkState(true);

        setUnsolResponseFilterSignalStrength(true);
        updateSignalStrength();
    }
    else
    {
        // screen is off

        setUnsolResponseFilterNetworkState(false);

        setUnsolResponseFilterSignalStrength(false);
    }

    response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxVoidData(), msg, false);
    // response to TeleCore
    responseToTelCore(response);

}

void RmcNetworkRequestHandler::requestSetUnsolicitedResponseFilter(const sp<RfxMclMessage>& msg) {
    /************************************
    * Control the URC: ECSQ,CREG,CGREG,CEREG,
    * CIREG,PSBEARER,ECSG,EMODCFG,EREGINFO,
    * EXLCE.
    *************************************/

    RIL_UnsolicitedResponseFilter filter;
    int err;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> response;
    RIL_UnsolicitedResponseFilter *pUnsolicitedResponseFilter
            = (RIL_UnsolicitedResponseFilter *)msg->getData()->getData();

    filter = pUnsolicitedResponseFilter[0];

    if ((filter & RIL_UR_SIGNAL_STRENGTH) == RIL_UR_SIGNAL_STRENGTH) {
        // enable
        setUnsolResponseFilterSignalStrength(true);
        updateSignalStrength();
    } else {
        // disable
        setUnsolResponseFilterSignalStrength(false);
    }
    if ((filter & RIL_UR_FULL_NETWORK_STATE) == RIL_UR_FULL_NETWORK_STATE) {
        // enable
        setUnsolResponseFilterNetworkState(true);
    } else {
        // disable
        setUnsolResponseFilterNetworkState(false);
    }
    if ((filter & LINK_CAPACITY_ESTIMATE) == LINK_CAPACITY_ESTIMATE) {
        // enable
        setUnsolResponseFilterLinkCapacityEstimate(true);
    } else {
        // disable
        setUnsolResponseFilterLinkCapacityEstimate(false);
    }
    if ((filter & PHYSICAL_CHANNEL_CONFIG) == PHYSICAL_CHANNEL_CONFIG) {
        // enable
        physicalConfigSwitch[m_slot_id] = true;
    } else {
        // disable
        physicalConfigSwitch[m_slot_id] = false;
    }


    response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxVoidData(), msg, false);
    // response to TeleCore
    responseToTelCore(response);
}

int RmcNetworkRequestHandler::isEnableModulationReport()
{
    char optr[RFX_PROPERTY_VALUE_MAX] = {0};

    rfx_property_get("persist.vendor.operator.optr", optr, "");

    return (strcmp("OP08", optr) == 0) ? 1 : 0;
}

void RmcNetworkRequestHandler::requestGetFemtocellList(const sp<RfxMclMessage>& msg) {
    /* +ECSG: <num_plmn>,<plmn_id>,<act>,<num_csg>,<csg_id>,<csg_icon_type>,<hnb_name>,<sig>[,...]
       AT Response Example
       +ECSG: 3,"46000",2,1,<csg_id_A>,<csg_type_A>,<hnb_name_A>,<sig>,"46002",7,1,<csg_id_B>,<csg_type_B>,<hnb_name_B>,<sig>,"46002",7,1,<csg_id_C>,<csg_type_C>,<hnb_name_C>,<sig> */
    int err, len, i, j, num, act, csgId ,csgIconType,numCsg, sig;
    sp<RfxMclMessage> response;
    RfxAtLine* line;
    sp<RfxAtResponse> p_response;
    char **femtocellList = NULL;
    char *femtocell = NULL, *plmn_id = NULL, *hnb_name = NULL;
    char shortname[MAX_OPER_NAME_LENGTH];
    int size_femto = 7;
    int support_sig = 1;

    m_csgListOngoing = 1;

    // check <sig> available
    p_response = atSendCommandSingleline("AT+ECSG=6", "+ECSG");
    if (p_response == NULL ||
            p_response->getError() != 0 ||
            p_response->getSuccess() == 0 ||
            p_response->getIntermediates() == NULL) {
        logE(LOG_TAG, "modem does not support AT+ECELL=6.");
        support_sig = 0;

        // send AT command
        p_response = atSendCommandSingleline("AT+ECSG=0", "+ECSG");

        // check error
        if (p_response == NULL ||
                p_response->getError() != 0 ||
                p_response->getSuccess() == 0 ||
                p_response->getIntermediates() == NULL) {
            goto error;
        }
    }

    // handle intermediate
    line = p_response->getIntermediates();

    // go to start position
    line->atTokStart(&err);
    if (err < 0) goto error;

    // <num_plmn>
    num = line->atTokNextint(&err);
    if (err < 0) goto error;

    // allocate memory
    femtocellList = (char **) calloc(1, sizeof(char*) * num * size_femto);
    femtocell = (char *) calloc(1, num* sizeof(char) * size_femto * MAX_OPER_NAME_LENGTH);

    if (femtocellList == NULL || femtocell == NULL) {
        logE(LOG_TAG, "requestGetFemtocellList calloc fail");
        goto error;
    }

    for (i = 0, j = 0; i < num; i++, j+=size_femto) {
        /* <plmn_id>,<act>,<num_csg>,<csg_id>,<csg_icon_type>,<hnb_name> */
        femtocellList[j+0] = &femtocell[(j+0)*MAX_OPER_NAME_LENGTH];
        femtocellList[j+1] = &femtocell[(j+1)*MAX_OPER_NAME_LENGTH];
        femtocellList[j+2] = &femtocell[(j+2)*MAX_OPER_NAME_LENGTH];
        femtocellList[j+3] = &femtocell[(j+3)*MAX_OPER_NAME_LENGTH];
        femtocellList[j+4] = &femtocell[(j+4)*MAX_OPER_NAME_LENGTH];
        femtocellList[j+5] = &femtocell[(j+5)*MAX_OPER_NAME_LENGTH];
        femtocellList[j+6] = &femtocell[(j+6)*MAX_OPER_NAME_LENGTH];

        /* get <plmn_id> numeric code*/
        plmn_id = line->atTokNextstr(&err);
        if (err < 0) goto error;
        sprintf(femtocellList[j+0], "%s", plmn_id);

        int len = strlen(femtocellList[j+0]);
        if (len == 5 || len == 6) {
            err = getOperatorNamesFromNumericCode(
                      femtocellList[j+0], femtocellList[j+1],shortname, MAX_OPER_NAME_LENGTH);
            if (err < 0) goto error;
        } else {
            goto error;
        }

        /* get <AcT> 0 is "2G", 2 is "3G", 7 is "4G"*/
        act = line->atTokNextint(&err);
        if (err < 0) goto error;
        sprintf(femtocellList[j+2], "%d", act);

        /* get <num_csg> fwk no need*/
        numCsg = line->atTokNextint(&err);
        if (err < 0) goto error;

        /* get <csgId> */
        csgId = line->atTokNextint(&err);
        if (err < 0) goto error;
        sprintf(femtocellList[j+3], "%d", csgId);

        /* get <csgIconType> */
        csgIconType = line->atTokNextint(&err);
        if (err < 0) goto error;
        sprintf(femtocellList[j+4], "%d", csgIconType);

        /* get <hnbName> */
        hnb_name = line->atTokNextstr(&err);
        if (err < 0) goto error;
        sprintf(femtocellList[j+5], "%s", hnb_name);

        if (support_sig == 1) {
            /* get <sig> */
            sig = line->atTokNextint(&err);
            if (err < 0) goto error;
            sprintf(femtocellList[j+6], "%d", sig);
        } else {
            sprintf(femtocellList[j+6], "-200"); // fake low sig
        }

        logD(LOG_TAG, "requestGetFemtocellList (%s, %s, %s, %s, %s, %s, %s)",
                femtocellList[j+0],
                femtocellList[j+1],
                femtocellList[j+2],
                femtocellList[j+3],
                femtocellList[j+4],
                femtocellList[j+5],
                femtocellList[j+6]);
    }

    response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxStringsData(femtocellList, num*size_femto), msg, false);
    responseToTelCore(response);
    free(femtocellList);
    free(femtocell);

    m_csgListOngoing = 0;
    m_csgListAbort =0; /* always clear here to prevent race condition scenario */
    return;

error:
    logE(LOG_TAG, "requestGetFemtocellList must never return error when radio is on");
    if (m_csgListAbort == 1) {
        // requestGetFemtocellList is canceled
        response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_CANCELLED,
            RfxStringsData(), msg, false);
    } else {
        response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
            RfxStringsData(), msg, false);
    }
    // response to TeleCore
    responseToTelCore(response);
    if (femtocellList != NULL) free(femtocellList);
    if (femtocell) free(femtocell);
    m_csgListOngoing = 0;
    m_csgListAbort =0; /* always clear here to prevent race condition scenario */
}

void RmcNetworkRequestHandler::requestAbortFemtocellList(const sp<RfxMclMessage>& msg) {
    sp<RfxMclMessage> response;
    sp<RfxAtResponse> p_response;

    if (m_csgListOngoing == 1) {
        m_csgListAbort = 1;
        p_response = atSendCommandSingleline("AT+ECSG=2", "+ECSG:");
        // check error
        if (p_response == NULL ||
                p_response->getError() != 0 ||
                p_response->getSuccess() == 0) {
            m_csgListAbort = 0;
            logE(LOG_TAG, "requestAbortFemtocellList fail.");
            goto error;
        }
    }

    response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxVoidData(), msg, false);
    // response to TeleCore
    responseToTelCore(response);
    return;

error:
    logE(LOG_TAG, "requestAbortFemtocellList must never return error when radio is on");
    response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
            RfxVoidData(), msg, false);
    // response to TeleCore
    responseToTelCore(response);
}

void RmcNetworkRequestHandler::requestSelectFemtocell(const sp<RfxMclMessage>& msg) {
    sp<RfxMclMessage> response;
    sp<RfxAtResponse> p_response;
    const char **strings = (const char **)msg->getData()->getData();
    //parameters:  <plmn> , <act> , <csg id>

    // check parameters
    if ((msg->getData()->getDataLength() < (int)(3 * sizeof(char*)))
            || (strings[0] == NULL)
            || (strings[1] == NULL)
            || (strings[2] == NULL)) {
        logE(LOG_TAG, "requestSelectFemtocell parameters wrong datalen = %d",
                msg->getData()->getDataLength());
        goto error;
    }

    // check <plmn> is valid digit
    for (size_t i = 0; i < strlen(strings[0]); i++) {
        if (strings[0][i] < '0' || strings[0][i] > '9') {
            logE(LOG_TAG, "requestSelectFemtocell parameters[0] wrong");
            goto error;
        }
    }

    // check <csg id>
    for (size_t i = 0; i < strlen(strings[2]); i++) {
        if (strings[2][i] < '0' || strings[2][i] > '9') {
            logE(LOG_TAG, "requestSelectFemtocell parameters[2] wrong");
            goto error;
        }
    }

    p_response = atSendCommand(String8::format("AT+ECSG=1,\"%s\",%s,%s", strings[0],strings[2],strings[1]));
    // check error
    if (p_response == NULL ||
            p_response->getError() != 0 ||
            p_response->getSuccess() == 0) {
        goto error;
    }

    response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxVoidData(), msg, false);
    // response to TeleCore
    responseToTelCore(response);
    return;

error:
    logE(LOG_TAG, "requestSelectFemtocell must never return error when radio is on");
    response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
            RfxVoidData(), msg, false);
    // response to TeleCore
    responseToTelCore(response);
}
void RmcNetworkRequestHandler::requestQueryFemtoCellSystemSelectionMode(const sp<RfxMclMessage>& msg) {
    int mode, err;
    int response[2] = { 0 };
    RIL_Errno ril_errno = RIL_E_MODE_NOT_SUPPORTED;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> resp;
    RfxAtLine* line;

    logD(LOG_TAG, "requestQueryFemtoCellSystemSelectionMode sending AT command");
    p_response = atSendCommandSingleline("AT+EFSS?", "+EFSS:");

    err = p_response->getError();
    if (err != 0 ||
            p_response == NULL ||
            p_response->getSuccess() == 0 ||
            p_response->getIntermediates() == NULL) goto error;

    // handle intermediate
    line = p_response->getIntermediates();

    /* +EFSS: <mode>
       AT Response Example
       +EFSS: 0 */

    // go to start position
    line->atTokStart(&err);
    if (err < 0) goto error;

    mode = line->atTokNextint(&err);
    if (err < 0) goto error;

    logD(LOG_TAG, "requestQueryFemtoCellSystemSelectionMode sucess, free memory");
    resp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxIntsData(&mode, 1), msg, false);
    responseToTelCore(resp);
    return;
error:
    logD(LOG_TAG,
        "requestQueryFemtoCellSystemSelectionMode must never return error when radio is on");
    resp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
            RfxIntsData(&mode, 1), msg, false);
    responseToTelCore(resp);
}

void RmcNetworkRequestHandler::requestSetFemtoCellSystemSelectionMode(const sp<RfxMclMessage>& msg) {
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> response;
    RIL_Errno ril_errno = RIL_E_GENERIC_FAILURE;
    int *pInt = (int *)msg->getData()->getData();
    int mode = pInt[0];

    logD(LOG_TAG, "requestSetFemtoCellSystemSelectionMode: mode=%d", mode);

    if ((mode >= 0) && (mode <= 2)) {
        p_response = atSendCommand(String8::format("AT+EFSS=%d", mode));
        if (p_response->getError() >= 0 || p_response->getSuccess() != 0) {
            ril_errno = RIL_E_SUCCESS;
        }
    } else {
        logE(LOG_TAG, "mode is invalid");
    }
    response = RfxMclMessage::obtainResponse(msg->getId(), ril_errno,
            RfxVoidData(), msg, false);
    // response to TeleCore
    responseToTelCore(response);
    return;
}

void RmcNetworkRequestHandler::requestAntennaConf(const sp<RfxMclMessage>& msg) {
    int antennaType, err;
    int response[2] = { 0 };
    RIL_Errno ril_errno = RIL_E_MODE_NOT_SUPPORTED;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> resp;
    int *pInt = (int *)msg->getData()->getData();

    antennaType = pInt[0];
    response[0] = antennaType;
    response[1] = 0; // failed

    logD(LOG_TAG, "Enter requestAntennaConf(), antennaType = %d ", antennaType);
    // AT command format as below : (for VZ_REQ_LTEB13NAC_6290)
    // AT+ERFTX=8, <type>[,<param1>,<param2>]
    // <param1> is decoded as below:
    //    1 - Normal dual receiver operation(default UE behaviour)
    //    2 - Single receiver operation 'enable primary receiver only'(disable secondary/MIMO receiver)
    //    3 - Single receiver operation 'enable secondary/MIMO receiver only (disable primary receiver)
    switch(antennaType){
        case 0:    // 0: signal information is not available on all Rx chains
            antennaType = 0;
            break;
        case 1:    // 1: Rx diversity bitmask for chain 0
            antennaType = 2;
            break;
        case 2:    // 2: Rx diversity bitmask for chain 1 is available
            antennaType = 3;
            break;
        case 3:    // 3: Signal information on both Rx chains is available.
            antennaType = 1;
            break;
        default:
            logE(LOG_TAG, "requestAntennaConf: configuration is an invalid");
            break;
    }
    p_response = atSendCommand(String8::format("AT+ERFTX=8,1,%d", antennaType));
    if (p_response->getError() < 0 || p_response->getSuccess() == 0) {
        if (antennaType == 0) {
            // This is special handl for disable all Rx chains
            // <param1>=0 - signal information is not available on all Rx chains
            ril_errno = RIL_E_SUCCESS;
            response[1] = 1;  // success
            antennaTestingType = antennaType;
        }
    } else {
        ril_errno = RIL_E_SUCCESS;
        response[1] = 1; // success
        // Keep this settings for query antenna info.
        antennaTestingType = antennaType;
    }
    resp = RfxMclMessage::obtainResponse(msg->getId(), ril_errno,
            RfxIntsData(response, 2), msg, false);
    responseToTelCore(resp);
}
void RmcNetworkRequestHandler::requestAntennaInfo(const sp<RfxMclMessage>& msg) {
    RIL_Errno ril_errno = RIL_E_MODE_NOT_SUPPORTED;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> resp;
    RfxAtLine* line;

    int param1, param2, err, skip;
    int response[6] = { 0 };
    memset(response, 0, sizeof(response));
    int *primary_antenna_rssi   = &response[0];
    int *relative_phase         = &response[1];
    int *secondary_antenna_rssi = &response[2];
    int *phase1                 = &response[3];
    int *rxState_0              = &response[4];
    int *rxState_1              = &response[5];
    *primary_antenna_rssi   = 0;  // <primary_antenna_RSSI>
    *relative_phase         = 0;  // <relative_phase>
    *secondary_antenna_rssi = 0;  // <secondary_antenna_RSSI>
    *phase1                 = 0;  // N/A
    *rxState_0              = 0;  // rx0 status(0: not vaild; 1:valid)
    *rxState_1              = 0;  // rx1 status(0: not vaild; 1:valid)
    // AT+ERFTX=8, <type> [,<param1>,<param2>]
    // <type>=0 is used for VZ_REQ_LTEB13NAC_6290
    // <param1> represents the A0 bit in ANTENNA INFORMATION REQUEST message
    // <param2> represents the A1 bit in ANTENNA INFORMATION REQUEST message
    switch(antennaTestingType) {
        case 0:    // signal information is not available on all Rx chains
            param1 = 0;
            param2 = 0;
            break;
        case 1:    // Normal dual receiver operation (default UE behaviour)
            param1 = 1;
            param2 = 1;
            break;
        case 2:    // enable primary receiver only
            param1 = 1;
            param2 = 0;
            break;
        case 3:    // enable secondary/MIMO receiver only
            param1 = 0;
            param2 = 1;
            break;
        default:
            logE(LOG_TAG, "requestAntennaInfo: configuration is an invalid, antennaTestingType: %d", antennaTestingType);
            goto error;
    }
    logD(LOG_TAG, "requestAntennaInfo: antennaType=%d, param1=%d, param2=%d", antennaTestingType, param1, param2);
    if (antennaTestingType == 0) {
        p_response = atSendCommand(String8::format("AT+ERFTX=8,0,%d,%d", param1, param2));
        if (p_response->getError() >= 0 || p_response->getSuccess() != 0) {
            ril_errno = RIL_E_SUCCESS;
        }
        resp = RfxMclMessage::obtainResponse(msg->getId(), ril_errno,
                RfxIntsData(response, 6), msg, false);
        responseToTelCore(resp);
        return;
    }
    // set antenna testing type
    p_response = atSendCommand(String8::format("AT+ERFTX=8,1,%d", antennaTestingType));
    if (p_response->getError() >= 0 || p_response->getSuccess() != 0) {
        p_response = atSendCommandSingleline(String8::format("AT+ERFTX=8,0,%d,%d", param1, param2),
                "+ERFTX:");
        if (p_response->getError() >= 0 || p_response->getSuccess() != 0) {
            // handle intermediate
            line = p_response->getIntermediates();
            // go to start position
            line->atTokStart(&err);
            if (err < 0) goto error;
            // skip <op=8>
            skip = line->atTokNextint(&err);
            if (err < 0) goto error;
            // skip <type=0>
            skip = line->atTokNextint(&err);
            if (err < 0) goto error;
            (*primary_antenna_rssi) = line->atTokNextint(&err);
            if (err < 0) {
                // response for AT+ERFTX=8,0,0,1
                // Ex: +ERFTX: 8,0,,100
            } else {
                // response for AT+ERFTX=8,0,1,1 or AT+ERFTX=8,0,1,0
                // Ex: +ERFTX: 8,0,100,200,300 or +ERFTX: 8,0,100
                *rxState_0 = 1;
            }
            if (line->atTokHasmore()) {
                (*secondary_antenna_rssi) = line->atTokNextint(&err);
                if (err < 0) {
                    logE(LOG_TAG, "ERROR occurs <secondary_antenna_rssi> form antenna info request");
                    goto error;
                } else {
                    // response for AT+ERFTX=8,0,1,0
                    // Ex: +ERFTX: 8,0,100
                    *rxState_1 = 1;
                }
                if (line->atTokHasmore()) {
                    // response for AT+ERFTX=8,0,1,1
                    // Ex: +ERFTX: 8,0,100,200,300
                    (*relative_phase) = line->atTokNextint(&err);
                    if (err < 0) {
                        logE(LOG_TAG, "ERROR occurs <relative_phase> form antenna info request");
                        goto error;
                    }
                }
            }
            ril_errno = RIL_E_SUCCESS;
        }
    } else {
        logE(LOG_TAG, "Set antenna testing type getting ERROR");
        goto error;
    }
error:
    resp = RfxMclMessage::obtainResponse(msg->getId(), ril_errno,
            RfxIntsData(response, 6), msg, false);
    responseToTelCore(resp);
}

void RmcNetworkRequestHandler::requestSetServiceState(const sp<RfxMclMessage>& msg) {
    int voice_reg_state, data_reg_state;
    int voice_roaming_type, data_roaming_type;
    int ril_voice_reg_state, ril_rata_reg_state;
    RIL_Errno ril_errno = RIL_E_GENERIC_FAILURE;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> resp;
    int *pInt = (int *)msg->getData()->getData();

    voice_reg_state = pInt[0];
    data_reg_state = pInt[1];
    voice_roaming_type = pInt[2];
    data_roaming_type = pInt[3];
    ril_voice_reg_state = pInt[4];
    ril_rata_reg_state = pInt[5];

    if ((voice_reg_state >= 0 && voice_reg_state <= 3) &&
            (data_reg_state >= 0 && data_reg_state <= 3) &&
            (voice_roaming_type >= 0 && voice_roaming_type <= 3) &&
            (data_roaming_type >= 0 && data_roaming_type <= 3) &&
            (ril_voice_reg_state >= 0 && ril_voice_reg_state <= 14) &&
            (ril_rata_reg_state >= 0 && ril_rata_reg_state <= 14)) {
        /*****************************
        * If all parameters are valid,
        * set to MD
        ******************************/
        p_response = atSendCommand(String8::format("AT+ESRVSTATE=%d,%d,%d,%d,%d,%d",
                voice_reg_state,
                data_reg_state,
                voice_roaming_type,
                data_roaming_type,
                ril_voice_reg_state,
                ril_rata_reg_state));
        if (p_response->getError() >= 0 && p_response->getSuccess() != 0) {
            ril_errno = RIL_E_SUCCESS;
        }
    }

    updateCellularPsState();

    resp = RfxMclMessage::obtainResponse(msg->getId(), ril_errno,
            RfxVoidData(), msg, false);
    responseToTelCore(resp);
}

void RmcNetworkRequestHandler::handleConfirmRatBegin(const sp<RfxMclMessage>& msg) {
    RFX_UNUSED(msg);
    sp<RfxAtResponse> p_response;
    bool ret = true;
    int err;
    int count = 0;

    while (ret) {
        p_response = atSendCommand("AT+ERPRAT");
        err = p_response->getError();

        ret = (err < 0 || 0 == p_response->getSuccess()) ? true:false;
        logV(LOG_TAG, "confirmRatBegin, send command AT+ERPRAT, err = %d, ret=%d, count=%d",
            err, ret, count);
        count++;
        // If get wrong result, we need to check whether go on or not.
        if (ret) {
            if (count == 30) {
                logD(LOG_TAG, "confirmRatBegin, reach the maximum time, return directly.");
                break;
            }

            RIL_RadioState state = (RIL_RadioState) getMclStatusManager()->getIntValue(RFX_STATUS_KEY_RADIO_STATE);
            if (RADIO_STATE_UNAVAILABLE == state || RADIO_STATE_OFF == state) {
                logD(LOG_TAG, "confirmRatBegin, radio unavliable/off, return directly.");
                break;
            }
            // Go on retry after 5 seconds.
            sleep(1);
        }
    };
}

void RmcNetworkRequestHandler::handleCsNetworkStateEvent(const sp<RfxMclMessage>& msg) {
    int *pInt = (int *) msg->getData()->getData();
    int prevRegState = pInt[0];
    int prevRat = pInt[1];
    int newRegState = pInt[2];
    int newRat = pInt[3];
    if ((prevRegState != newRegState
            || RfxNwServiceState::isCdmaGroup(prevRat) != RfxNwServiceState::isCdmaGroup(newRat))
            && RfxNwServiceState::isInService(newRegState)) {
        logV(LOG_TAG, "handleCsNetworkStateEvent, need updateSignalStrength, "
                "prevRegState=%d, prevRat=%d, newRegState=%d, newRat=%d",
                prevRegState, prevRat, newRegState, newRat);
        updateSignalStrength();
    }
}

void RmcNetworkRequestHandler::handlePsNetworkStateEvent(const sp<RfxMclMessage>& msg) {
    // response[0] = state, response[1] = mccmnc, response[2] = rat.
    int response[3];
    int *pInt = (int *)msg->getData()->getData();
    // state (consider iwlan)
    response[0] = pInt[2];
    // rat (consider iwlan)
    response[2] = pInt[3];
    int operNumericLength = 0;

    int err;
    int skip;
    char *eops_response = NULL;
    sp<RfxMclMessage> urc;
    RfxAtLine* line;
    sp<RfxAtResponse> p_response;

    // Update SignalStrength for PS state change
    int prevRegState = pInt[0];
    int prevRat = pInt[1];
    int newRegState = pInt[2];
    int newRat = pInt[3];
    if ((prevRegState != newRegState
            || RfxNwServiceState::isCdmaGroup(prevRat) != RfxNwServiceState::isCdmaGroup(newRat))
            && RfxNwServiceState::isInService(newRegState)) {
        logV(LOG_TAG, "handlePsNetworkStateEvent, need updateSignalStrength, "
                "prevRegState=%d, prevRat=%d, newRegState=%d, newRat=%d",
                prevRegState, prevRat, newRegState, newRat);
        updateSignalStrength();
    }

    /* Format should be set during initialization */
    p_response = atSendCommandSingleline("AT+EOPS?", "+EOPS:");

    // check error
    err = p_response->getError();
    if (err != 0 ||
            p_response == NULL ||
            p_response->getSuccess() == 0 ||
            p_response->getIntermediates() == NULL) {
        logE(LOG_TAG, "EOPS got error response");
    } else {
        // handle intermediate
        line = p_response->getIntermediates();

        // go to start position
        line->atTokStart(&err);
        if (err >= 0) {
            /* <mode> */
            skip = line->atTokNextint(&err);
            if ((err >= 0) && (skip >= 0 && skip <= 4 && skip != 2)) {
                // a "+EOPS: 0" response is possible
                if (line->atTokHasmore()) {
                    /* <format> */
                    skip = line->atTokNextint(&err);
                    if (err >= 0 && skip == 2)
                    {
                        /* <oper> */
                        eops_response = line->atTokNextstr(&err);
                        /* Modem might response invalid PLMN ex: "", "000000" , "??????", all convert to "000000" */
                        if (!((eops_response[0] >= '0') && (eops_response[0] <= '9'))) {
                            // logE(LOG_TAG, "EOPS got invalid plmn response");
                            memset(eops_response, 0, operNumericLength);
                        }
                    }
                }
            }
        }
    }
    if (eops_response != NULL) {
        getMclStatusManager()->setString8Value(RFX_STATUS_KEY_OPERATOR_INCLUDE_LIMITED,
                String8::format("%s", eops_response));
        response[1] = atoi(eops_response);
    } else {
        getMclStatusManager()->setString8Value(RFX_STATUS_KEY_OPERATOR_INCLUDE_LIMITED, String8(""));
        response[1] = 0;
    }

    urc = RfxMclMessage::obtainUrc(RFX_MSG_URC_RESPONSE_PS_NETWORK_STATE_CHANGED,
            m_slot_id, RfxIntsData(response, 3));
    // response to TeleCore
    responseToTelCore(urc);

    return;
}

void RmcNetworkRequestHandler::updateCellularPsState() {
}

void RmcNetworkRequestHandler::triggerPollNetworkState() {
    logD(LOG_TAG, "triggerPollNetworkState");

    // update signal strength
    atSendCommand("AT+ECSQ");

    // update voice/data/Operator
    sp<RfxMclMessage> urc = RfxMclMessage::obtainUrc(RFX_MSG_URC_RESPONSE_VOICE_NETWORK_STATE_CHANGED,
            m_slot_id, RfxVoidData());
    responseToTelCore(urc);
}

void RmcNetworkRequestHandler::onHandleTimer() {
    // do something
}

void RmcNetworkRequestHandler::onHandleEvent(const sp<RfxMclMessage>& msg) {
    int id = msg->getId();
    switch (id) {
        case RFX_MSG_EVENT_EXIT_EMERGENCY_CALLBACK_MODE:
            triggerPollNetworkState();
            break;
        case RFX_MSG_EVENT_FEMTOCELL_UPDATE:
            updateFemtoCellInfo();
            break;
        case RFX_MSG_EVENT_CONFIRM_RAT_BEGIN:
            handleConfirmRatBegin(msg);
            break;
    case RFX_MSG_EVENT_CS_NETWORK_STATE:
            handleCsNetworkStateEvent(msg);
            break;
        case RFX_MSG_EVENT_PS_NETWORK_STATE:
            handlePsNetworkStateEvent(msg);
            break;
        case RFX_MSG_EVENT_RERESH_PHYSICAL_CONFIG:
            currentPhysicalChannelConfigs(false);
            break;
        case RFX_MSG_EVENT_RSP_DATA_CONTEXT_IDS:
            currentPhysicalChannelConfigs(handleGetDataContextIds(msg));
            break;
        default:
            logE(LOG_TAG, "onHandleEvent, should not be here");
            break;
    }
}

void RmcNetworkRequestHandler::updatePseudoCellMode() {
    sp<RfxAtResponse> p_response;
    char *property = NULL;
    char prop[RFX_PROPERTY_VALUE_MAX] = {0};

    p_response = atSendCommandSingleline("AT+EAPC?", "+EAPC:");
    if (p_response->getError() >= 0 && p_response->getSuccess() != 0) {
        // set property if modem support APC, EM will check this property to show APC setting
        rfx_property_set("vendor.ril.apc.support", "1");
        // check if the APC mode was set before, if yes, send the same at command again
        // AT+EAPC? was apc query command, if return it, means APC mode was not set before
        asprintf(&property, "persist.vendor.radio.apc.mode%d", m_slot_id);
        rfx_property_get(property, prop, "AT+EAPC?");
        logV(LOG_TAG, "updatePseudoCellMode: %s = %s", property, prop);
        free(property);
        if (strcmp("AT+EAPC?", prop) != 0) {
            atSendCommand(prop);
        }
    }
}

void RmcNetworkRequestHandler::requestSetPseudoCellMode(const sp<RfxMclMessage>& msg) {
    RIL_Errno ril_errno = RIL_E_GENERIC_FAILURE;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> resp;
    char *property = NULL;
    char *cmd = NULL;
    int *pInt = (int *)msg->getData()->getData();
    int apc_mode = pInt[0];
    int urc_enable = pInt[1];
    int timer = pInt[2];
    /*
    *  apc_mode = 0: disable APC feature
    *  apc_mode = 1: set APC mode I, if detect a pseudo cell, not attach it
    *  apc_mode = 2: set APC mode II, if detect a pseudo cell, also attach it
    */
    asprintf(&cmd, "AT+EAPC=%d,%d,%d", apc_mode, urc_enable, timer);
    p_response = atSendCommand(cmd);
    if (p_response->getError() >= 0 && p_response->getSuccess() != 0) {
        ril_errno = RIL_E_SUCCESS;
    }
    if (ril_errno == RIL_E_SUCCESS) {
        asprintf(&property, "persist.vendor.radio.apc.mode%d", m_slot_id);
        rfx_property_set(property, cmd);
        free(property);
    } else {
        logE(LOG_TAG, "requestSetPseudoCellMode failed");
    }
    free(cmd);
    resp = RfxMclMessage::obtainResponse(msg->getId(), ril_errno,
            RfxVoidData(), msg, false);
    responseToTelCore(resp);
}


void RmcNetworkRequestHandler::setRoamingEnable(const sp<RfxMclMessage>& msg) {
    RIL_Errno ril_errno = RIL_E_GENERIC_FAILURE;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> resp;
    char *cmd = NULL;
    int *pInt = (int*)msg->getData()->getData();
    /*
        data[0] : phone id (0,1,2,3,...)
        data[1] : international_voice_text_roaming (0,1)
        data[2] : international_data_roaming (0,1)
        data[3] : domestic_voice_text_roaming (0,1)
        data[4] : domestic_data_roaming (0,1)
        data[5] : domestic_LTE_data_roaming (0,1)

    +EROAMBAR:<protocol_index>, (not ready now)
        <BAR_Dom_Voice_Roaming_Enabled>,
        <BAR_Dom_Data_Roaming_Enabled>,
        <Bar_Int_Voice_Roaming_Enabled>,
        <Bar_Int_Data_Roaming_Enabled>,
        <Bar_LTE_Data_Roaming_Enabled>
        NOTE: The order is different.
    */
    int err = 0;
    int (*p)[6] = (int(*)[6])pInt;
    // rever the setting from enable(fwk) to bar(md)
    for (int i = 1; i < 6; i++) {
        (*p)[i] = (*p)[i] == 0 ? 1: 0;
    }

    asprintf(&cmd, "AT+EROAMBAR=%d,%d,%d,%d,%d"
        , (*p)[3]  // BAR_Dom_Voice_Roaming_Enabled
        , (*p)[4]  // BAR_Dom_Data_Roaming_Enabled
        , (*p)[1]  // Bar_Int_Voice_Roaming_Enabled
        , (*p)[2]  // Bar_Int_Data_Roaming_Enabled
        , (*p)[5]);  // Bar_LTE_Data_Roaming_Enabled
    logD(LOG_TAG, "setRoamingEnable %s", cmd);
    p_response = atSendCommand(cmd);
    if (p_response->getError() >= 0 && p_response->getSuccess() != 0) {
        ril_errno = RIL_E_SUCCESS;
    }
    free(cmd);
    resp = RfxMclMessage::obtainResponse(msg->getId(), ril_errno,
            RfxVoidData(), msg, false);
    responseToTelCore(resp);
}

void RmcNetworkRequestHandler::getRoamingEnable(const sp<RfxMclMessage>& msg) {
    /* +EROAMBAR:<protocol_index>, (not ready now)
        <BAR_Dom_Voice_Roaming_Enabled>,
        <BAR_Dom_Data_Roaming_Enabled>,
        <Bar_Int_Voice_Roaming_Enabled>,
        <Bar_Int_Data_Roaming_Enabled>,
        <Bar_LTE_Data_Roaming_Enabled>
     Expected Result:
     response[0]: phone id (0,1,2,3,...)
     response[1] : international_voice_text_roaming (0,1)
     response[2] : international_data_roaming (0,1)
     response[3] : domestic_voice_text_roaming (0,1)
     response[4] : domestic_data_roaming (0,1)
     response[5] : domestic_LTE_data_roaming (1) */
    RfxAtLine *line = NULL;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> resp;
    int roaming[6] = {1, 0, 1, 1, 1, 1};  // default value
    int err = 0;

    p_response = atSendCommandSingleline("AT+EROAMBAR?", "+EROAMBAR:");

    // check error
    err = p_response->getError();
    if (err != 0 ||
          p_response == NULL ||
          p_response->getSuccess() == 0 ||
          p_response->getIntermediates() == NULL)
        goto error;

    // handle intermediate
    line = p_response->getIntermediates();

    // go to start position
    line->atTokStart(&err);
    if (err < 0) goto error;

    /* //DS
     roaming[0] = line->atTokNextint(&err);
     if (err < 0) goto error; */

    roaming[0] = 0;

    // <BAR_Dom_Voice_Roaming_Enabled>
    roaming[3] = line->atTokNextint(&err);
    if (err < 0) goto error;

    // <BAR_Dom_Data_Roaming_Enabled>
    roaming[4] = line->atTokNextint(&err);
    if (err < 0) goto error;


    // <Bar_Int_Voice_Roaming_Enabled>
    roaming[1] = line->atTokNextint(&err);
    if (err < 0) goto error;


    // <Bar_Int_Data_Roaming_Enabled>
    roaming[2] = line->atTokNextint(&err);
    if (err < 0) goto error;


    // <Bar_LTE_Data_Roaming_Enabled>
    roaming[5] = line->atTokNextint(&err);
    if (err < 0) goto error;


    // rever the setting from enable(fwk) to bar(md)
    for (int i = 1; i < 6; i++) {
        roaming[i] = roaming[i] == 0 ? 1:0;
    }

    resp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
                RfxIntsData(roaming, 6), msg, false);
    // response to TeleCore
    responseToTelCore(resp);
    return;
    error:
    logE(LOG_TAG, "getRoamingEnable must never return error when radio is on");
    resp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
                RfxVoidData(), msg, false);
    // response to TeleCore
    responseToTelCore(resp);
}

void RmcNetworkRequestHandler::requestGetPseudoCellInfo(const sp<RfxMclMessage>& msg) {
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> resp;
    RfxAtLine* line;
    int err;
    int apc_mode;
    int urc_enable;
    int timer;
    int num;

    // <apc_mode>[<urc_enable><time>
    // <num>[<type><plmn><lac><cid><arfcn><bsic>[<type><plmn><lac><cid><arfcn><bsic>]]]
    // num: 0 or 1 or 2
    int response[16] = {0};

    p_response = atSendCommandSingleline("AT+EAPC?", "+EAPC:");
    if (p_response->getError() < 0 || p_response->getSuccess() == 0) {
        goto error;
    }

    /*  response:
     *    +EAPC:<apc_mode>[,<urc_enable>,<time>,<count>
     *      [,<type>,<plmn>,<lac>,<cid>,<arfcn>,<bsic>[,<type>,<plmn>,<lac>,<cid>,<arfcn>,<bsic>]]]
     */
    line = p_response->getIntermediates();

    line->atTokStart(&err);
    if (err < 0) goto error;

    apc_mode = line->atTokNextint(&err);
    if (err < 0) goto error;
    response[0] = apc_mode;

    if (line->atTokHasmore()) {
        urc_enable = line->atTokNextint(&err);
        if (err < 0) goto error;
        response[1] = urc_enable;

        timer = line->atTokNextint(&err);
        if (err < 0) goto error;
        response[2] = timer;

        num = line->atTokNextint(&err);
        if (err < 0) goto error;
        response[3] = num;

        for (int i = 0; i < num; i++) {
            response[i*6 + 4] = line->atTokNextint(&err);
            if (err < 0) goto error;

            if (line->atTokHasmore()) {
                response[i*6 + 5] = line->atTokNextint(&err);
                if (err < 0) goto error;

                response[i*6 + 6] = line->atTokNextint(&err);
                if (err < 0) goto error;

                response[i*6 + 7] = line->atTokNextint(&err);
                if (err < 0) goto error;

                response[i*6 + 8] = line->atTokNextint(&err);
                if (err < 0) goto error;

                response[i*6 + 9] = line->atTokNextint(&err);
                if (err < 0) goto error;
            }
        }
    }

    resp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxIntsData(response, 16), msg, false);
    responseToTelCore(resp);
    return;

error:
    logE(LOG_TAG, "requestGetPseudoCellInfo failed err=%d", p_response->getError());
    resp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
            RfxVoidData(), msg, false);
    responseToTelCore(resp);
}

int RmcNetworkRequestHandler::isDisable2G()
{
    int ret = 0;
    char property_value[RFX_PROPERTY_VALUE_MAX] = {0};
    char optr[RFX_PROPERTY_VALUE_MAX] = {0};

    rfx_property_get("persist.vendor.operator.optr", optr, "");
    rfx_property_get("persist.vendor.radio.disable.2g", property_value, "0");
    logV(LOG_TAG, "[isDisable2G] optr:%s, disable.2g:%s", optr, property_value);

    if (strcmp("OP07", optr) == 0) {
        if (atoi(property_value) == 1) {
            ret = 1;
        } else {
            ret = 0;
        }
    } else {
        ret = 0;
    }

    return ret;
}

void RmcNetworkRequestHandler::requestSetLteReleaseVersion(const sp<RfxMclMessage>& msg) {
    // +ECASW=<mode>
    // <mode>: integer type. Setting mode.
    // <mode>=0 turn off LTE Carrier Aggregation
    // <mode>=1 turn on LTE Carrier Aggregation

    RIL_Errno ril_errno = RIL_E_GENERIC_FAILURE;
    int *pInt = (int *) msg->getData()->getData();
    int mode = pInt[0];

    if (mode < 0) {
        ril_errno = RIL_E_INVALID_ARGUMENTS;
    } else {
        // send AT command
        sp<RfxAtResponse> p_response = atSendCommand(String8::format("AT+ECASW=%d", mode));
        if (p_response->getError() >= 0 && p_response->getSuccess() != 0) {
            ril_errno = RIL_E_SUCCESS;
        }
    }

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), ril_errno,
            RfxVoidData(), msg, false);
    responseToTelCore(response);
}

void RmcNetworkRequestHandler::requestGetLteReleaseVersion(const sp<RfxMclMessage>& msg) {
    int err, mode = 0;
    RfxAtLine* line;
    sp<RfxMclMessage> response;

    sp<RfxAtResponse> p_response = atSendCommandSingleline("AT+ECASW?", "+ECASW:");

    err = p_response->getError();
    if (err < 0 || p_response->getSuccess() == 0) {
        goto error;
    }

    line = p_response->getIntermediates();

    // go to start position
    line->atTokStart(&err);
    if (err < 0) goto error;

    // get <mode>
    mode = line->atTokNextint(&err);
    if (err < 0) goto error;

    response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxIntsData(&mode, 1), msg, false);
    responseToTelCore(response);
    return;

error:
    response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
            RfxIntsData(&mode, 1), msg, false);
    responseToTelCore(response);
}

void RmcNetworkRequestHandler::fillCidToPhysicalChannelConfig(RIL_PhysicalChannelConfig* pcc) {
    pcc->num_cids = 0;
    for (int i = 0; i < 16; i++) {
        if (data_context_ids[m_slot_id][i] > 0) {
            pcc->num_cids += 1;
            pcc->contextIds[i] = data_context_ids[m_slot_id][i];
        }
    }
}

void RmcNetworkRequestHandler::currentPhysicalChannelConfigs(bool forceUpdate) {
    sp<RfxMclMessage> urc;
    // HIDL CellConnectionStatus.NONE = 0
    //      CellConnectionStatus.PRIMARY_SERVING = 1
    //      CellConnectionStatus.SECONDARY_SERVING = 2
    // physicalConfig[0] = pcell status
    // physicalConfig[1] = pcell band (kHz)
    // physicalConfig[2] = scell status
    // physicalConfig[3] = scell band (kHz)
    int physicalConfig[4] = {0};

    if (!physicalConfigSwitch[m_slot_id]) return;  // It's off.

    RIL_DATA_REG_STATE_CACHE* temp_data_reg_state =
            (RIL_DATA_REG_STATE_CACHE*) calloc(1, sizeof(RIL_DATA_REG_STATE_CACHE));
    if (temp_data_reg_state == NULL) {
        logE(LOG_TAG, "currentPhysicalChannelConfigs: alloc temp_data_reg_state fail");
        return;
    }
    memcpy(temp_data_reg_state, data_reg_state_cache[m_slot_id], sizeof(RIL_DATA_REG_STATE_CACHE));

    // caculate num to report
    int num = 0;
    if (isInService(temp_data_reg_state->registration_state)) {
        if (temp_data_reg_state->radio_technology == 14 ||
            temp_data_reg_state->radio_technology == 19) {
            // LTE or LTE_CA
            pthread_mutex_lock(&s_caMutex[m_slot_id]);
            if (ca_cache[m_slot_id] != NULL) {
                if (ca_cache[m_slot_id]->pcell_bw > 0) {
                    num = 1;
                }
                if (ca_cache[m_slot_id]->scell_bw1 > 0) {
                    num = 2;
                }
                if (data_reg_state_cache[m_slot_id]->endc_available > 0) {
                    num += 1;
                }
            }
            pthread_mutex_unlock(&s_caMutex[m_slot_id]);
        } else if (temp_data_reg_state->radio_technology == 1 ||
            temp_data_reg_state->radio_technology == 2) {
            // GPRS or EDGE
            num = 1;
        } else if (temp_data_reg_state->radio_technology == 6 ||
            temp_data_reg_state->radio_technology == 8 ||
            temp_data_reg_state->radio_technology == 13) {
            // C2K
            num = 0;
        } else {
            // else 3G cases
            num = 1;
            if (m_dc_support[m_slot_id]) {
                num = 2;
            }
        }
    }

    RIL_PhysicalChannelConfig rilPhysicalconfig[num];

    if (num > 0 &&
        isInService(temp_data_reg_state->registration_state)) {
        memset(rilPhysicalconfig, 0, num * sizeof(RIL_PhysicalChannelConfig));
        if (temp_data_reg_state->radio_technology == 14 ||
            temp_data_reg_state->radio_technology == 19) {
            // LTE or LTE_CA
            pthread_mutex_lock(&s_caMutex[m_slot_id]);
            if (ca_cache[m_slot_id] != NULL) {
                if (ca_cache[m_slot_id]->pcell_bw > 0) {
                    physicalConfig[0] = 1;
                    // unit of ca_cache[m_slot_id]->pcell_bw is 0.1MHz
                    // 1MHz = 1000 KHz
                    physicalConfig[1] = 100 * ca_cache[m_slot_id]->pcell_bw;

                    rilPhysicalconfig[0].rat = RADIO_TECH_LTE;
                    rilPhysicalconfig[0].status = PRIMARY_SERVING;
                    rilPhysicalconfig[0].cellBandwidthDownlink = physicalConfig[1];
                    rilPhysicalconfig[0].fr = LOW;
                    rilPhysicalconfig[0].channelNumber = 0x7FFFFFFF;
                    rilPhysicalconfig[0].physicalCellId = 0x7FFFFFFF;
                    fillCidToPhysicalChannelConfig(&rilPhysicalconfig[0]);
                }
                if (ca_cache[m_slot_id]->scell_bw1 > 0) {
                    physicalConfig[2] = 2;
                    physicalConfig[3] = 100 * ca_cache[m_slot_id]->scell_bw1;

                    rilPhysicalconfig[1].rat = RADIO_TECH_LTE;
                    rilPhysicalconfig[1].status = SECONDARY_SERVING;
                    rilPhysicalconfig[1].cellBandwidthDownlink = physicalConfig[3];
                    rilPhysicalconfig[1].fr = LOW;
                    rilPhysicalconfig[1].channelNumber = 0x7FFFFFFF;
                    rilPhysicalconfig[1].physicalCellId = 0x7FFFFFFF;
                    fillCidToPhysicalChannelConfig(&rilPhysicalconfig[1]);
                }
                // eAct = 0x4000
                if (data_reg_state_cache[m_slot_id]->endc_available == 1 &&
                    data_reg_state_cache[m_slot_id]->endc_sib == 1 &&
                    data_reg_state_cache[m_slot_id]->dcnr_restricted == 0) {
                    rilPhysicalconfig[num-1].rat = RADIO_TECH_NR;
                    rilPhysicalconfig[num-1].status = SECONDARY_SERVING;
                    rilPhysicalconfig[num-1].cellBandwidthDownlink = physicalConfig[3];
                    rilPhysicalconfig[num-1].fr = LOW;
                    rilPhysicalconfig[num-1].channelNumber = 0x7FFFFFFF;
                    rilPhysicalconfig[num-1].physicalCellId = 0x7FFFFFFF;
                    fillCidToPhysicalChannelConfig(&rilPhysicalconfig[num-1]);
                }
            }
            pthread_mutex_unlock(&s_caMutex[m_slot_id]);
        } else if (temp_data_reg_state->radio_technology == 1 ||
            temp_data_reg_state->radio_technology == 2) {
            // GPRS or EDGE
            physicalConfig[0] = 1;
            physicalConfig[1] = 200;

            rilPhysicalconfig[0].rat = RADIO_TECH_GSM;
            rilPhysicalconfig[0].status = PRIMARY_SERVING;
            rilPhysicalconfig[0].cellBandwidthDownlink = physicalConfig[1];
            rilPhysicalconfig[0].fr = LOW;
            rilPhysicalconfig[0].channelNumber = 0x7FFFFFFF;
            rilPhysicalconfig[0].physicalCellId = 0x7FFFFFFF;
        } else if (temp_data_reg_state->radio_technology == 6 ||
            temp_data_reg_state->radio_technology == 8 ||
            temp_data_reg_state->radio_technology == 13) {
            // C2K
        } else {
            // else 3G cases
            physicalConfig[0] = 1;
            physicalConfig[1] = 5000;

            rilPhysicalconfig[0].rat = RADIO_TECH_UMTS;
            rilPhysicalconfig[0].status = PRIMARY_SERVING;
            rilPhysicalconfig[0].cellBandwidthDownlink = physicalConfig[1];
            rilPhysicalconfig[0].fr = LOW;
            rilPhysicalconfig[0].channelNumber = 0x7FFFFFFF;
            rilPhysicalconfig[0].physicalCellId = 0x7FFFFFFF;
            if (m_dc_support[m_slot_id]) {
                physicalConfig[2] = 2;
                physicalConfig[3] = 5000;

                rilPhysicalconfig[1].rat = RADIO_TECH_UMTS;
                rilPhysicalconfig[1].status = SECONDARY_SERVING;
                rilPhysicalconfig[1].cellBandwidthDownlink = physicalConfig[3];
                rilPhysicalconfig[1].fr = LOW;
                rilPhysicalconfig[1].channelNumber = 0x7FFFFFFF;
                rilPhysicalconfig[1].physicalCellId = 0x7FFFFFFF;
            }
        }
    }

    bool hasChanged = false;
    for (int i = 0; i < 4; i++) {
        if (physicalConfig[i] != physicalConfig_cache[m_slot_id][i]) {
            hasChanged = true;
            break;
        }
    }
    if (forceUpdate) hasChanged = true;
    logD(LOG_TAG, "physicalConfig = [%d, %d, %d, %d], hasChanged=%d, forceUpdate=%d",
        physicalConfig[0],
        physicalConfig[1],
        physicalConfig[2],
        physicalConfig[3],
        (hasChanged? 1: 0),
        (forceUpdate? 1: 0));
    // only send URC when it's changed.
    if (hasChanged) {
        if (num != 0) {
            urc = RfxMclMessage::obtainUrc(RFX_MSG_URC_PHYSICAL_CHANNEL_CONFIGS_MTK,
                    m_slot_id,
                    RfxIntsData((void*)rilPhysicalconfig, num*sizeof(RIL_PhysicalChannelConfig)));
            responseToTelCore(urc);
        } else {
            // send an empty one to clear
            RIL_PhysicalChannelConfig pcc = {};
            memset(&pcc, 0, sizeof(RIL_PhysicalChannelConfig));

            urc = RfxMclMessage::obtainUrc(RFX_MSG_URC_PHYSICAL_CHANNEL_CONFIGS_MTK,
                    m_slot_id,
                    RfxIntsData((void*)&pcc, sizeof(RIL_PhysicalChannelConfig)));
            responseToTelCore(urc);
        }
    }
    // cache the physical config info.
    for (int i = 0; i < 4; i++) {
        physicalConfig_cache[m_slot_id][i] = physicalConfig[i];
    }
    if (temp_data_reg_state != NULL) free(temp_data_reg_state);
}

void RmcNetworkRequestHandler::requestSetSignalStrengthReportingCriteria
        (const sp<RfxMclMessage>& msg) {
    sp<RfxMclMessage> resp;
    RIL_Errno ril_err = RIL_E_INVALID_ARGUMENTS;
    RIL_SignalStrength_Reporting_Criteria* ssrc =
            (RIL_SignalStrength_Reporting_Criteria*) msg->getData()->getData();
    logD(LOG_TAG, "requestSetSignalStrengthReportingCriteria %d, %d, %d"
            "thresholdsDbm[0] = %d"
            "thresholdsDbm[1] = %d"
            "thresholdsDbm[2] = %d"
            "thresholdsDbm[3] = %d",
            ssrc->hysteresisMs,
            ssrc->hysteresisDb,
            (int) ssrc->accessNetwork,
            ssrc->thresholdsDbm[0],
            ssrc->thresholdsDbm[1],
            ssrc->thresholdsDbm[2],
            ssrc->thresholdsDbm[3]);

    resp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
                RfxVoidData(), msg, false);
    responseToTelCore(resp);
    return;
}

int getUmtsBandHex(int band_number) {
    int band_hex = 0;
    // band 1 = 1
    // band 2 = 2
    // band 3 = 4
    if (band_number > 26) {
        return 0;
    }
    band_hex = 1 << (band_number - 1);
    return band_hex;
}

int getLteBandHex(int band_number) {
    int band_hex = 0;
    // band 1 = 1
    // band 2 = 2
    // band 3 = 4
    // band 33 = 1
    // band 34 = 2
    // band 35 = 4
    while(band_number > 32) band_number = band_number - 32;
    band_hex = 1 << (band_number - 1);
    return band_hex;
}

void RmcNetworkRequestHandler::requestSetSystemSelectionChannels
        (const sp<RfxMclMessage>& msg) {
    // +EMMCHLCK=<mode>,[<act>,<band indicator>,[<number of channel>,]<arfcn>,[<arfcn>,<arfcn>,...,<arfcn>],<cell id>]
    sp<RfxMclMessage> resp;
    RIL_Errno ril_err = RIL_E_RADIO_NOT_AVAILABLE;
    RIL_SystemSelectionChannels* ssc =
            (RIL_SystemSelectionChannels*) msg->getData()->getData();
    sp<RfxAtResponse> p_response;
    int err;
    // disable
    String8 log("requestSetSystemSelectionChannels ");
    log.append(String8::format(",specifyChannels=%d", ssc->specifyChannels));

    p_response = atSendCommand("AT+EMMCHLCK=0");
    err = p_response->getError();
    if (err < 0 || p_response->getSuccess() == 0) {
        logE(LOG_TAG, "requestSetSystemSelectionChannels disable fail");
        goto error;
    }

    if (ssc->specifyChannels > 0) {
        // config
        log.append(String8::format(",specifiers_length=%d", ssc->specifiers_length));
        for (int i = 0; i < ssc->specifiers_length; i++) {
            String8 cmd("AT+EMMCHLCK=3");
            log.append(String8::format(",specifiers[%d].radio_access_network=%d", i, ssc->specifiers[i].radio_access_network));
            // <act>
            switch (ssc->specifiers[i].radio_access_network) {
                case GERAN:
                    cmd.append(",0");
                    break;
                case UTRAN:
                    cmd.append(",2");
                    break;
                case EUTRAN:
                    cmd.append(",7");
                    break;
                default:
                    goto error;
            }
            log.append(String8::format(",specifiers[%d].bands_length=%d", i, ssc->specifiers[i].bands_length));
            // <band indicator>
            int band_indicator = 0;
            for (int j = 0; j < ssc->specifiers[i].bands_length; j++) {
                // only support one band now
                switch (ssc->specifiers[i].radio_access_network) {
                    case GERAN:
                        if (j == 0) {
                            // GSM, it's 1 only when GERAN_BAND_PCS1900
                            if (ssc->specifiers[i].bands.geran_bands[j] == GERAN_BAND_PCS1900) {
                                band_indicator = 1;
                            } else {
                                band_indicator = 0;
                            }
                        }
                        log.append(String8::format(",specifiers[%d].bands[%d]=%d", i, j, ssc->specifiers[i].bands.geran_bands[j]));
                        break;
                    case UTRAN:
                        band_indicator = band_indicator | getUmtsBandHex(ssc->specifiers[i].bands.utran_bands[j]);
                        log.append(String8::format(",specifiers[%d].bands[%d]=%d", i, j, ssc->specifiers[i].bands.utran_bands[j]));
                        break;
                    case EUTRAN:
                        band_indicator = band_indicator | getLteBandHex(ssc->specifiers[i].bands.eutran_bands[j]);
                        log.append(String8::format(",specifiers[%d].bands[%d]=%d", i, j, ssc->specifiers[i].bands.eutran_bands[j]));
                        break;
                    default:
                        goto error;
                }
            }
            cmd.append(String8::format(",%x", band_indicator));
            log.append(String8::format(",specifiers[%d].channels_length=%d", i, ssc->specifiers[i].channels_length));
            // <number of channel>
            cmd.append(String8::format(",%d", ssc->specifiers[i].channels_length));
            // <arfcn>
            for (int j = 0; j < ssc->specifiers[i].channels_length; j++) {
                log.append(String8::format(",specifiers[%d].channels[%d]=%d", i, j, ssc->specifiers[i].channels[j]));
                cmd.append(String8::format(",%d", ssc->specifiers[i].channels[j]));
            }
            p_response = atSendCommand(cmd.string());
            err = p_response->getError();
            if (err < 0 || p_response->getSuccess() == 0) {
                logE(LOG_TAG, "requestSetSystemSelectionChannels config fail");
                goto error;
            }
        }
        // enable
        p_response = atSendCommand("AT+EMMCHLCK=2");
        err = p_response->getError();
        if (err < 0 || p_response->getSuccess() == 0) {
            logE(LOG_TAG, "requestSetSystemSelectionChannels enable fail");
            goto error;
        }
    }
    logD(LOG_TAG, "%s", log.string());

    resp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
                RfxVoidData(), msg, false);
    responseToTelCore(resp);
    return;
error:
    resp = RfxMclMessage::obtainResponse(msg->getId(), ril_err,
                RfxVoidData(), msg, false);
    responseToTelCore(resp);

}

void RmcNetworkRequestHandler::requestGetTs25Name(const sp<RfxMclMessage>& msg) {
    sp<RfxMclMessage> response;
    int *pInt = (int *) msg->getData()->getData();
    int mcc = pInt[0];
    int mnc = pInt[1];
    char *mccmnc = NULL;
    char longname[MAX_OPER_NAME_LENGTH], shortname[MAX_OPER_NAME_LENGTH];

    logD(LOG_TAG, "requestGetTs25Name input %d %d", mcc, mnc);
    if (mnc > 99) asprintf(&mccmnc, "%d%03d", mcc, mnc);
    else asprintf(&mccmnc, "%d%02d", mcc, mnc);
    getPLMNNameFromNumeric(mccmnc, longname, shortname, MAX_OPER_NAME_LENGTH);
    free(mccmnc);
    logD(LOG_TAG, "requestGetTs25Name result %s %s", longname, shortname);
    // response long name only.
    response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxStringData(longname, (strlen(longname)+1)), msg, false);
    responseToTelCore(response);
    return;
}

void RmcNetworkRequestHandler::requestEnableCaPlusFilter(const sp<RfxMclMessage>& msg) {
    sp<RfxMclMessage> response;
    int *pInt = (int *) msg->getData()->getData();
    bool enabled = pInt[0] == 0 ? false : true;

    ca_filter_switch[m_slot_id] = enabled;
    logD(LOG_TAG, "requestEnableCaPlusFilter enabled %d", pInt[0]);
    response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxVoidData(), msg, false);
    responseToTelCore(response);
    return;
}

bool RmcNetworkRequestHandler::handleGetDataContextIds(const sp<RfxMclMessage>& msg) {
    bool changed = false;
    int m_data_context_ids[16] = {};
    int *pInt = (int *)msg->getData()->getData();
    int size = msg->getData()->getDataLength() / sizeof(int);

    memcpy((void*) m_data_context_ids, (void*) data_context_ids[m_slot_id], 16 * sizeof(int));
    for (int i = 0; i < 16; i++) {
        if (i < size) {
            data_context_ids[m_slot_id][i] = pInt[i];
        } else {
            data_context_ids[m_slot_id][i] = -1;
        }
    }
    if (memcmp((void*) m_data_context_ids, (void*) data_context_ids[m_slot_id], 16 * sizeof(int)) != 0)
        changed = true;
    logD(LOG_TAG, "handleGetDataContextIds [%d, %d, %d, %d, ...]",
            data_context_ids[m_slot_id][0],
            data_context_ids[m_slot_id][1],
            data_context_ids[m_slot_id][2],
            data_context_ids[m_slot_id][3]);
    return changed;
}

/* MUSE WFC requirement
 * AT+EHOMEAS=<mode>,<signal_type>,<TIn>,<TOut>,<timer>[,<num_of_extended_threshold>[,<threshold1>...]
 * <enable>: enabled, 0 = disable; 1 = enabled
 * <type>: quality type
 * <threshold_array>: threshold value array
 */
void RmcNetworkRequestHandler::registerCellularQualityReport(const sp<RfxMclMessage>& msg) {
    char** params = (char**) msg->getData()->getData();
    char* atCmd = (char*)"AT+EHOMEAS";
    sp<RfxAtResponse> p_response;
    int err = 0;
    RIL_Errno ril_errno = RIL_E_GENERIC_FAILURE;
    sp<RfxMclMessage> response;
    char theshold1[6];memset(theshold1, 0, sizeof(theshold1));
    char theshold2[6];memset(theshold2, 0, sizeof(theshold2));
    int thresholdCount = 1;
    char* pExtended_thresholds = NULL;
    char* pStart = params[2];
    char* pComma;

    pComma = strchr(pStart, ',');
    if (pComma == NULL) {
        // only one threshold
        strncpy(theshold1, pStart, sizeof(theshold1)-1);
        strncpy(theshold2, pStart, sizeof(theshold2)-1);
    } else {
        memcpy(theshold1, pStart, pComma - pStart);
        theshold1[(pComma - pStart)] = 0;
        thresholdCount++;
        pStart = pComma + 1;

        pComma = strchr(pStart, ',');
        if (pComma == NULL) {
            // only two thresholds
            strncpy(theshold2, pStart, sizeof(theshold2)-1);
        } else {
            memcpy(theshold2, pStart, pComma - pStart);
            theshold2[(pComma - pStart)] = 0;
            thresholdCount++;
            pStart = pComma + 1;
            pExtended_thresholds = pStart;

            pComma = strchr(pStart, ',');
            while (pComma != NULL) {
                thresholdCount++;
                pStart = pComma + 1;
                pComma = strchr(pStart, ',');
            }
        }
    }

    String8 cmd;
    if (thresholdCount <= 2) {
        cmd = String8::format("%s=%s,%s,%s,%s,%s", atCmd, params[0], params[1], theshold1,
                theshold2, params[3]);
    } else {
        logD(LOG_TAG, "registerCellularQualityReport %d, %s",thresholdCount, pExtended_thresholds);
        cmd = String8::format("%s=%s,%s,%s,%s,%s,%d,%s", atCmd, params[0], params[1], theshold1,
                theshold2, params[3], thresholdCount-2, pExtended_thresholds);
    }

    p_response = atSendCommand(cmd);
    err = p_response->getError();
    if (!(err < 0 || p_response->getSuccess() == 0)) {
        ril_errno = RIL_E_SUCCESS;
    } else {
        ril_errno = RIL_E_INVALID_STATE;
    }

error:
    response = RfxMclMessage::obtainResponse(msg->getId(), ril_errno,
            RfxVoidData(), msg, false);
    // response to TeleCore
    responseToTelCore(response);
}

void RmcNetworkRequestHandler::requestGetEhrpdInfo(const sp<RfxMclMessage>& msg) {
    int err = 0;
    char* responseStr = NULL;
    sp<RfxMclMessage> response;
    CDMA_CELL_LOCATION_INFO cdma_cell_location = { 0, 0, 0, 0, 0, 0, 0, NULL, NULL };

    ViaBaseHandler *mViaHandler = RfxViaUtils::getViaHandler();
    if (mViaHandler != NULL) {
        err = mViaHandler->getCdmaLocationInfo(this, &cdma_cell_location);
        if (err < 0) goto error;
        if (cdma_cell_location.sector_id != NULL && cdma_cell_location.subnet_mask != NULL) {
            // Skip the first two characters, eg: 0x00a8c0 -> 00a8c0.
            err = asprintf(&responseStr, "%s:%s", cdma_cell_location.sector_id + 2,
                    cdma_cell_location.subnet_mask + 2);
            if (err < 0) goto error;
        } else {
            goto error;
        }
    } else {
        goto error;
    }

    if (responseStr != NULL) {
        response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
                RfxStringData(responseStr, strlen(responseStr)), msg, false);
        responseToTelCore(response);
        free(responseStr);
        responseStr = NULL;
        return;
    } else {
        logE(LOG_TAG, "%s, return because responseStr is NULL.", __FUNCTION__);
    }
error:
    response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
            RfxStringData(), msg, false);
    responseToTelCore(response);
    if (responseStr != NULL) {
        free(responseStr);
        responseStr = NULL;
    }
    return;
}

void RmcNetworkRequestHandler::requestGetSuggestedPlmnList(const sp<RfxMclMessage>& msg) {
    int err, len, list_size, num_filter, i, j;
    int *pInt = (int *)msg->getData()->getData();
    int rat = pInt[0];
    int num = pInt[1];
    int timer = pInt[2];
    char** response = NULL;
    sp<RfxMclMessage> resp;
    sp<RfxAtResponse> p_response;
    RfxAtLine* line;
    char *tmp;

    logE(LOG_TAG, "requestGetSuggestedPlmnList rat=%d, num=%d, timer=%d", rat, num, timer);

    p_response = atSendCommandSingleline(
        String8::format("AT+EPLWSS=%d,%d,%d", rat, num, timer), "+EPLWSS:");
    err = p_response->getError();
    if (err < 0 || p_response->getSuccess() == 0) {
        goto error;
    }

    line = p_response->getIntermediates();

    // count the number of operator
    tmp = line->getLine();
    len = strlen(tmp);
    for(i = 0, list_size = 0, num_filter = 0; i < len ; i++ ) {
        // here we assume that there is no nested ()
        if (tmp[i] == '(') {
            list_size++;
            num_filter++;
        } else if (tmp[i] == ',' && tmp[i+1] == ',') {
            break;
        }
    }

    response = (char**) calloc(1, sizeof(char*) * list_size);
    if (response == NULL) goto error;
    memset(response, 0, sizeof(char*) * list_size);
    // +EPLWSS: (2,"Far EasTone","FET","46601",0),(...),...,,(0, 1, 3),(0-2)
    // go to start position
    line->atTokStart(&err);
    if (err < 0) goto error;

    for (i = 0; i < list_size ; i++) {
        // <state>
        tmp = line->atTokNextstr(&err);
        if (err < 0) goto error;

        // get long name
        tmp = line->atTokNextstr(&err);
        if (err < 0) goto error;

        // get short name
        tmp = line->atTokNextstr(&err);
        if (err < 0) goto error;

        // get <oper> numeric code
        tmp = line->atTokNextstr(&err);
        if (err < 0) goto error;
        asprintf(&(response[i]), "%s", tmp);

        // get <lac> numeric code
        tmp = line->atTokNextstr(&err);
        if (err < 0) goto error;

        // get <AcT>
        tmp = line->atTokNextstr(&err);
        if (err < 0) goto error;
    }

    resp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxStringsData(response, list_size), msg, false);
    responseToTelCore(resp);

    if (response != NULL) {
        for (i = 0; i < list_size ; i++) {
            if (response[i] != NULL) free(response[i]);
        }
        free(response);
    }
    return;
error:
    resp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_MODEM_ERR,
                RfxVoidData(), msg, false);
    responseToTelCore(resp);
    if (response != NULL) {
        for (i = 0; i < list_size ; i++) {
            if (response[i] != NULL) free(response[i]);
        }
        free(response);
    }
}

// NR request
void RmcNetworkRequestHandler::requestConfigA2Offset(const sp<RfxMclMessage>& msg) {
    sp<RfxMclMessage> response;
    response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_REQUEST_NOT_SUPPORTED,
            RfxVoidData(), msg, false);
    responseToTelCore(response);
}

void RmcNetworkRequestHandler::requestConfigB1Offset(const sp<RfxMclMessage>& msg) {
    sp<RfxMclMessage> response;
    response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_REQUEST_NOT_SUPPORTED,
            RfxVoidData(), msg, false);
    responseToTelCore(response);
}

void RmcNetworkRequestHandler::requestEnableSCGFailure(const sp<RfxMclMessage>& msg) {
    sp<RfxMclMessage> response;
    response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_REQUEST_NOT_SUPPORTED,
            RfxVoidData(), msg, false);
    responseToTelCore(response);
}

void RmcNetworkRequestHandler::requestDisableNr(const sp<RfxMclMessage>& msg) {
    sp<RfxMclMessage> response;
    response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_REQUEST_NOT_SUPPORTED,
            RfxVoidData(), msg, false);
    responseToTelCore(response);
}

void RmcNetworkRequestHandler::requestSetTxPower(const sp<RfxMclMessage>& msg) {
    sp<RfxMclMessage> response;
    response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_REQUEST_NOT_SUPPORTED,
            RfxVoidData(), msg, false);
    responseToTelCore(response);
}

void RmcNetworkRequestHandler::requestSearchStoreFrenquencyInfo(const sp<RfxMclMessage>& msg) {
    sp<RfxMclMessage> response;
    response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_REQUEST_NOT_SUPPORTED,
            RfxVoidData(), msg, false);
    responseToTelCore(response);
}

void RmcNetworkRequestHandler::requestSearchRat(const sp<RfxMclMessage>& msg) {
    sp<RfxMclMessage> response;
    response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_REQUEST_NOT_SUPPORTED,
            RfxVoidData(), msg, false);
    responseToTelCore(response);
}

void RmcNetworkRequestHandler::requestSetBackgroundSearchTimer(const sp<RfxMclMessage>& msg) {
    sp<RfxMclMessage> response;
    response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_REQUEST_NOT_SUPPORTED,
            RfxVoidData(), msg, false);
    responseToTelCore(response);
}
