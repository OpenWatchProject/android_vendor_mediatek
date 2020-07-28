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

#include "RmcNetworkRealTimeRequestHandler.h"
#include "RfxVoiceRegStateData.h"
#include "RfxDataRegStateData.h"
#include "rfx_properties.h"
#include "ViaBaseHandler.h"
#include "RfxViaUtils.h"
#include <libmtkrilutils.h>

static const int request[] = {
    RFX_MSG_REQUEST_VOICE_REGISTRATION_STATE,
    RFX_MSG_REQUEST_DATA_REGISTRATION_STATE,
    RFX_MSG_REQUEST_OPERATOR,
    RFX_MSG_REQUEST_QUERY_NETWORK_SELECTION_MODE
};

// register data
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxVoiceRegStateData, RFX_MSG_REQUEST_VOICE_REGISTRATION_STATE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxDataRegStateData, RFX_MSG_REQUEST_DATA_REGISTRATION_STATE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxStringsData, RFX_MSG_REQUEST_OPERATOR);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxIntsData, RFX_MSG_REQUEST_QUERY_NETWORK_SELECTION_MODE);

// register handler to channel
RFX_IMPLEMENT_HANDLER_CLASS(RmcNetworkRealTimeRequestHandler, RIL_CMD_PROXY_7);

RmcNetworkRealTimeRequestHandler::RmcNetworkRealTimeRequestHandler(int slot_id, int channel_id) :
        RmcNetworkHandler(slot_id, channel_id),
        m_emergency_only(0) {
    int err;
    sp<RfxAtResponse> p_response;
    logV(LOG_TAG, "%s[%d] start", __FUNCTION__, slot_id);

    m_slot_id = slot_id;
    m_channel_id = channel_id;

    registerToHandleRequest(request, sizeof(request)/sizeof(int));
    resetVoiceRegStateCache(voice_reg_state_cache[m_slot_id], CACHE_GROUP_ALL);
    resetDataRegStateCache(data_reg_state_cache[m_slot_id], CACHE_GROUP_ALL);
    resetVoiceRegStateCache(urc_voice_reg_state_cache[m_slot_id], CACHE_GROUP_ALL);
    resetDataRegStateCache(urc_data_reg_state_cache[m_slot_id], CACHE_GROUP_ALL);
    resetCaCache(ca_cache[m_slot_id]);
    updateServiceStateValue();
}

RmcNetworkRealTimeRequestHandler::~RmcNetworkRealTimeRequestHandler() {
}

void RmcNetworkRealTimeRequestHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {
    int request = msg->getId();
    switch (request) {
        case RFX_MSG_REQUEST_VOICE_REGISTRATION_STATE:
            requestVoiceRegistrationState(msg);
            break;
        case RFX_MSG_REQUEST_DATA_REGISTRATION_STATE:
            requestDataRegistrationState(msg);
            break;
        case RFX_MSG_REQUEST_OPERATOR:
            requestOperator(msg);
            mIsNeedNotifyState[m_slot_id] = true;
            break;
        case RFX_MSG_REQUEST_QUERY_NETWORK_SELECTION_MODE:
            requestQueryNetworkSelectionMode(msg);
            break;
        default:
            logE(LOG_TAG, "Should not be here");
            break;
    }
}

static RIL_CellInfoType getCellInfoTypeRadioTechnology(int rat) {
    switch(rat) {
        case RADIO_TECH_GPRS:
        case RADIO_TECH_EDGE:
        case RADIO_TECH_GSM: {
            return RIL_CELL_INFO_TYPE_GSM;
        }
        case RADIO_TECH_UMTS:
        case RADIO_TECH_HSDPA:
        case RADIO_TECH_HSUPA:
        case RADIO_TECH_HSPA:
        case RADIO_TECH_HSPAP: {
            return RIL_CELL_INFO_TYPE_WCDMA;
        }
        case RADIO_TECH_IS95A:
        case RADIO_TECH_IS95B:
        case RADIO_TECH_1xRTT:
        case RADIO_TECH_EVDO_0:
        case RADIO_TECH_EVDO_A:
        case RADIO_TECH_EVDO_B:
        case RADIO_TECH_EHRPD: {
            return RIL_CELL_INFO_TYPE_CDMA;
        }
        case RADIO_TECH_LTE:
        case RADIO_TECH_LTE_CA: {
            return RIL_CELL_INFO_TYPE_LTE;
        }
        case RADIO_TECH_TD_SCDMA: {
            return RIL_CELL_INFO_TYPE_TD_SCDMA;
        }
        default: {
            break;
        }
    }
    return RIL_CELL_INFO_TYPE_NONE;
}

void RmcNetworkRealTimeRequestHandler::sendVoiceRegResponse(const sp<RfxMclMessage>& msg) {
    RIL_VoiceRegistrationStateResponse *pData = (RIL_VoiceRegistrationStateResponse *)
            alloca(sizeof(RIL_VoiceRegistrationStateResponse));
    int err = 0;
    int mcc = 0;
    int mnc = 0;
    int mnc_len = 0;
    char longname[MAX_OPER_NAME_LENGTH], shortname[MAX_OPER_NAME_LENGTH];

    pthread_mutex_lock(&s_voiceRegStateMutex[m_slot_id]);
    pData->regState = (RIL_RegState)voice_reg_state_cache[m_slot_id]->registration_state;
    pData->rat = (RIL_RadioTechnology)voice_reg_state_cache[m_slot_id]->radio_technology;
    pData->cssSupported = voice_reg_state_cache[m_slot_id]->css;
    pData->roamingIndicator = voice_reg_state_cache[m_slot_id]->roaming_indicator;
    pData->systemIsInPrl = voice_reg_state_cache[m_slot_id]->is_in_prl;
    pData->defaultRoamingIndicator = voice_reg_state_cache[m_slot_id]->default_roaming_indicator;
    if ((urc_voice_reg_state_cache[m_slot_id]->denied_reason > 0) &&
        (urc_voice_reg_state_cache[m_slot_id]->lac == voice_reg_state_cache[m_slot_id]->lac) &&
        (urc_voice_reg_state_cache[m_slot_id]->cid == voice_reg_state_cache[m_slot_id]->cid)) {
         // we have reject cause with the same lac/cid, we use it.
        pData->reasonForDenial = urc_voice_reg_state_cache[m_slot_id]->denied_reason;
    } else {
        pData->reasonForDenial = voice_reg_state_cache[m_slot_id]->denied_reason;
    }
    pthread_mutex_unlock(&s_voiceRegStateMutex[m_slot_id]);

#ifdef MTK_TC1_COMMON_SERVICE
    if (isFemtocellSupport() && isCdmaVoiceInFemtocell()) {
        // Support the Femtocell network: The response values of Roaming Indicator
        // should be the value "99" while connected to the Femtocell network.
        pData->roamingIndicator = 99;
    } else {
        pData->roamingIndicator = voice_reg_state_cache[m_slot_id]->roaming_indicator;
    }
#endif

    if (strlen(op_info_cache[m_slot_id]->mccMnc) == 5 || strlen(op_info_cache[m_slot_id]->mccMnc) == 6) {
        if (strlen(op_info_cache[m_slot_id]->mccMnc) == 6) mnc_len = 3;
        else mnc_len = 2;
        char mcc_str[4] = {0};
        char mnc_str[4] = {0};
        strncpy(mcc_str, op_info_cache[m_slot_id]->mccMnc, 3);
        strncpy(mnc_str, (op_info_cache[m_slot_id]->mccMnc+3), (strlen(op_info_cache[m_slot_id]->mccMnc)-3));
        mcc = atoi(mcc_str);
        mnc = atoi(mnc_str);
        err = getOperatorNamesFromNumericCode(
                op_info_cache[m_slot_id]->mccMnc, longname, shortname, MAX_OPER_NAME_LENGTH);
        if (err < 0) {
            memset(longname, 0, MAX_OPER_NAME_LENGTH);
            memset(shortname, 0, MAX_OPER_NAME_LENGTH);
        }
    } else {
        mcc = INT_MAX;
        mnc = INT_MAX;
        // reset longname/shortname because mccmnc may be empty and these are empty too.
        // It happends when the MD is in service right after poll_operator and before poll_voice/data
        memset(longname, 0, MAX_OPER_NAME_LENGTH);
        memset(shortname, 0, MAX_OPER_NAME_LENGTH);
    }

    pthread_mutex_lock(&s_voiceRegStateMutex[m_slot_id]);
    if (isInService(voice_reg_state_cache[m_slot_id]->registration_state)) {
        // unlock mutex before using any AT cmd
        pthread_mutex_unlock(&s_voiceRegStateMutex[m_slot_id]);
        refreshEcellCache();
        pthread_mutex_lock(&s_voiceRegStateMutex[m_slot_id]);
        // if AT+ECELL provides old data
        if (voice_reg_state_cache[m_slot_id]->lac != mdEcell[m_slot_id]->lacTac ||
                voice_reg_state_cache[m_slot_id]->cid != mdEcell[m_slot_id]->cid) {
            // reset
            memset(mdEcell[m_slot_id], 0, sizeof(MD_ECELL));
            mdEcell[m_slot_id]->bsic = 0xFF;
            mdEcell[m_slot_id]->arfcn = INT_MAX;
            mdEcell[m_slot_id]->pscPci = INT_MAX;
        }
    }

    pData->cellIdentity.cellInfoType =
            getCellInfoTypeRadioTechnology(voice_reg_state_cache[m_slot_id]->radio_technology);
    switch(pData->cellIdentity.cellInfoType) {
        case RIL_CELL_INFO_TYPE_GSM: {
            pData->cellIdentity.cellIdentityGsm.mcc = mcc;
            pData->cellIdentity.cellIdentityGsm.mnc = mnc;
            pData->cellIdentity.cellIdentityGsm.mnc_len= mnc_len;
            pData->cellIdentity.cellIdentityGsm.lac = voice_reg_state_cache[m_slot_id]->lac;
            pData->cellIdentity.cellIdentityGsm.cid = (int) voice_reg_state_cache[m_slot_id]->cid;
            pData->cellIdentity.cellIdentityGsm.arfcn = mdEcell[m_slot_id]->arfcn;
            pData->cellIdentity.cellIdentityGsm.bsic = mdEcell[m_slot_id]->bsic;
            pData->cellIdentity.cellIdentityGsm.operName.long_name = longname;
            pData->cellIdentity.cellIdentityGsm.operName.short_name = shortname;
            break;
        }

        case RIL_CELL_INFO_TYPE_WCDMA: {
            pData->cellIdentity.cellIdentityWcdma.mcc = mcc;
            pData->cellIdentity.cellIdentityWcdma.mnc = mnc;
            pData->cellIdentity.cellIdentityWcdma.mnc_len= mnc_len;
            pData->cellIdentity.cellIdentityWcdma.lac = voice_reg_state_cache[m_slot_id]->lac;
            pData->cellIdentity.cellIdentityWcdma.cid = (int) voice_reg_state_cache[m_slot_id]->cid;
            pData->cellIdentity.cellIdentityWcdma.psc = voice_reg_state_cache[m_slot_id]->psc;
            pData->cellIdentity.cellIdentityWcdma.uarfcn = mdEcell[m_slot_id]->arfcn;
            pData->cellIdentity.cellIdentityWcdma.operName.long_name = longname;
            pData->cellIdentity.cellIdentityWcdma.operName.short_name = shortname;
            break;
        }

        case RIL_CELL_INFO_TYPE_CDMA: {
            pData->cellIdentity.cellIdentityCdma.networkId = voice_reg_state_cache[m_slot_id]->network_id;
            pData->cellIdentity.cellIdentityCdma.systemId = voice_reg_state_cache[m_slot_id]->system_id;
            pData->cellIdentity.cellIdentityCdma.basestationId = voice_reg_state_cache[m_slot_id]->base_station_id;
            pData->cellIdentity.cellIdentityCdma.longitude = voice_reg_state_cache[m_slot_id]->base_station_longitude;
            pData->cellIdentity.cellIdentityCdma.latitude = voice_reg_state_cache[m_slot_id]->base_station_latitude;
            pData->cellIdentity.cellIdentityCdma.operName.long_name = longname;
            pData->cellIdentity.cellIdentityCdma.operName.short_name = shortname;
            break;
        }

        case RIL_CELL_INFO_TYPE_LTE: {
            pData->cellIdentity.cellIdentityLte.mcc = mcc;
            pData->cellIdentity.cellIdentityLte.mnc = mnc;
            pData->cellIdentity.cellIdentityLte.mnc_len= mnc_len;
            pData->cellIdentity.cellIdentityLte.ci = (int) voice_reg_state_cache[m_slot_id]->cid;
            pData->cellIdentity.cellIdentityLte.pci = mdEcell[m_slot_id]->pscPci;
            pData->cellIdentity.cellIdentityLte.tac = voice_reg_state_cache[m_slot_id]->lac;
            pData->cellIdentity.cellIdentityLte.earfcn = mdEcell[m_slot_id]->arfcn;
            pData->cellIdentity.cellIdentityLte.operName.long_name = longname;
            pData->cellIdentity.cellIdentityLte.operName.short_name = shortname;
            pData->cellIdentity.cellIdentityLte.bandwidth = 100 * ca_cache[m_slot_id]->pcell_bw;
            break;
        }

        case RIL_CELL_INFO_TYPE_TD_SCDMA: {
            pData->cellIdentity.cellIdentityTdscdma.mcc = mcc;
            pData->cellIdentity.cellIdentityTdscdma.mnc = mnc;
            pData->cellIdentity.cellIdentityTdscdma.mnc_len = mnc_len;
            pData->cellIdentity.cellIdentityTdscdma.lac = voice_reg_state_cache[m_slot_id]->lac;
            pData->cellIdentity.cellIdentityTdscdma.cid = (int) voice_reg_state_cache[m_slot_id]->cid;
            pData->cellIdentity.cellIdentityTdscdma.cpid = INT_MAX;
            pData->cellIdentity.cellIdentityTdscdma.operName.long_name = longname;
            pData->cellIdentity.cellIdentityTdscdma.operName.short_name = shortname;
            break;
        }

        case RIL_CELL_INFO_TYPE_NR:
            pData->cellIdentity.cellIdentityNr.mcc = mcc;
            pData->cellIdentity.cellIdentityNr.mnc = mnc;
            pData->cellIdentity.cellIdentityNr.tac = voice_reg_state_cache[m_slot_id]->lac;
            pData->cellIdentity.cellIdentityNr.nci = voice_reg_state_cache[m_slot_id]->cid;
            pData->cellIdentity.cellIdentityNr.pci = INT_MAX;
            pData->cellIdentity.cellIdentityNr.operName.long_name = longname;
            pData->cellIdentity.cellIdentityNr.operName.short_name = shortname;
            break;
        default: {
            break;
        }
    }
    pthread_mutex_unlock(&s_voiceRegStateMutex[m_slot_id]);

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxVoiceRegStateData(pData, sizeof(RIL_VoiceRegistrationStateResponse)), msg, false);

    // response to TeleCore
    responseToTelCore(response);
}

void RmcNetworkRealTimeRequestHandler::combineVoiceRegState(const sp<RfxMclMessage>& msg) {
    sendVoiceRegResponse(msg);
    updateServiceStateValue();
}

void RmcNetworkRealTimeRequestHandler::printVoiceCache(RIL_VOICE_REG_STATE_CACHE cache) {
    char* lac_s = getMask(cache.lac);
    char* cid_s = getMask(cache.cid);
    char* bsla_s = getMask(cache.base_station_latitude);
    char* bslo_s = getMask(cache.base_station_longitude);
    char* bsid_s = getMask(cache.base_station_id);
    char* sid_s = getMask(cache.system_id);
    char* nid_s = getMask(cache.network_id);
    if (!lac_s || !cid_s || !bsla_s || !bslo_s || !bsid_s || !sid_s || !nid_s) {
        logE(LOG_TAG, "[%s] can not get memeory to print log", __func__);
    } else {
        logV(LOG_TAG, " VoiceCache: "
            "registration_state=%d lac=%s cid=%s radio_technology=%d base_station_id=%s "
            "base_station_latitude=%s base_station_longitude=%s css=%d system_id=%s "
            "network_id=%s roaming_indicator=%d is_in_prl=%d default_roaming_indicator=%d "
            "denied_reason=%d psc=%d network_exist=%d", cache.registration_state, lac_s,
            cid_s, cache.radio_technology, bsid_s, bsla_s,
            bslo_s, cache.css, sid_s, nid_s,
            cache.roaming_indicator, cache.is_in_prl, cache.default_roaming_indicator,
            cache.denied_reason, cache.psc, cache.network_exist);
    }
    if (lac_s) free(lac_s);
    if (cid_s) free(cid_s);
    if (bsla_s) free(bsla_s);
    if (bslo_s) free(bslo_s);
    if (bsid_s) free(bsid_s);
    if (sid_s) free(sid_s);
    if (nid_s) free(nid_s);
}

int RmcNetworkRealTimeRequestHandler::convertToAndroidRegState(unsigned int uiRegState) {
    unsigned int uiRet = 0;

    logD(LOG_TAG, "convertToAndroidRegState s_emergencly_only=%d"
        ", m_cdma_emergency_only=%d"
        ", m_ims_ecc_only=%d",
        m_emergency_only,
        m_cdma_emergency_only,
        m_ims_ecc_only);
    /* Ref RIL_RegState in RIL.h*/
    if (m_emergency_only == 1 ||
            m_cdma_emergency_only == 1 ||
            m_ims_ecc_only == 1) {  // ims ecc only is true only if ps is out of service
        switch (uiRegState) {
            case 0:
                uiRet = 10;
                break;
            case 2:
                uiRet = 12;
                break;
            case 3:
                uiRet = 13;
                break;
            case 4:
                uiRet = 14;
                break;
            default:
                uiRet = uiRegState;
                break;
        }
    } else {
        uiRet = uiRegState;
    }
    return uiRet;
}

void RmcNetworkRealTimeRequestHandler::requestVoiceRegistrationState(const sp<RfxMclMessage>& msg) {
    int exist = 0;
    int err;
    int i = 0;
    sp<RfxAtResponse> p_response;
    RfxAtLine* line;
    int skip;
    int count = 3, cntCdma = 0;
    int cause_type;
    bool m4gRat = false;
    bool m5gRat = false;

    int cdma_network_exist = 0;
    // if support IMS ECC, this will be 1
    int ims_emergency_only = 0;
    /*
    int ecc_bearer_support =
            getMclStatusManager()->getIntValue(RFX_STATUS_KEY_ECC_BEARER_SUPPORT, 0);
    int ims_registerred =
            getMclStatusManager()->getIntValue(RFX_STATUS_KEY_IMS_REGISTRATION_STATE, 0);
    */
    int ims_ecc_support = ims_ecc_state[m_slot_id];
    int radio_state = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_RADIO_STATE, 0);
    MD_EREG *mMdEreg = mdEreg[m_slot_id];
    MD_ECELL *mMdEcell = mdEcell[m_slot_id];

    resetVoiceRegStateCache(voice_reg_state_cache[m_slot_id], CACHE_GROUP_COMMON_REQ);

    /* +EREG: <n>,<stat> */
    /* +EREG: <n>,<stat>,<lac>,<ci>,<eAct>,<nwk_existence >,<roam_indicator> */
    /* +EREG: <n>,<stat>,<lac>,<ci>,<eAct>,<nwk_existence >,<roam_indicator>,<cause_type>,<reject_cause> */

    dumpEregCache(__func__, mMdEreg, mMdEcell, true);

    pthread_mutex_lock(&mdEregMutex[m_slot_id]);
    pthread_mutex_lock(&s_voiceRegStateMutex[m_slot_id]);

    /* <stat> */
    voice_reg_state_cache[m_slot_id]->registration_state = mMdEreg->stat;
    if (voice_reg_state_cache[m_slot_id]->registration_state > 10 )  // for LTE
    {
        // TODO: add C2K flag
        if (voice_reg_state_cache[m_slot_id]->registration_state < 101 ||
                voice_reg_state_cache[m_slot_id]->registration_state > 104) {// for C2K
            logE(LOG_TAG, "The value in the field <stat> is not valid: %d",
                 voice_reg_state_cache[m_slot_id]->registration_state);
            goto error;
        }
    }

    // For LTE and C2K
    voice_reg_state_cache[m_slot_id]->registration_state =
            convertRegState(voice_reg_state_cache[m_slot_id]->registration_state, true);

    /* <lac> */
    voice_reg_state_cache[m_slot_id]->lac = mMdEreg->lac;
    if (voice_reg_state_cache[m_slot_id]->lac > 0xffff &&
                voice_reg_state_cache[m_slot_id]->lac != 0xffffffff ) {
        logE(LOG_TAG, "The value in the field <lac> or <stat> is not valid. <stat>:%d, <lac>:%d",
             voice_reg_state_cache[m_slot_id]->registration_state,
             voice_reg_state_cache[m_slot_id]->lac);
        goto error;
    }

    /* <ci> */
    voice_reg_state_cache[m_slot_id]->cid = mMdEreg->ci;
    if ((mMdEreg->eAct != 0x8000 &&  // 2/3/4G
            voice_reg_state_cache[m_slot_id]->cid > 0x0fffffff &&
            voice_reg_state_cache[m_slot_id]->cid != 0xffffffff) ||
            (mMdEreg->eAct == 0x8000 &&  // 5G
            voice_reg_state_cache[m_slot_id]->cid > 0x0fffffffff &&
            voice_reg_state_cache[m_slot_id]->cid != 0xffffffffff)) {
        logE(LOG_TAG, "The value in the field <ci> is not valid: %llX",
                voice_reg_state_cache[m_slot_id]->cid);
        goto error;
    }

    /* <eAct> */
    voice_reg_state_cache[m_slot_id]->radio_technology = mMdEreg->eAct;

    count = 4;
    if (voice_reg_state_cache[m_slot_id]->radio_technology > 0x8000)  // LTE-CA is 0x2000
    {
        logE(LOG_TAG, "The value in the eAct is not valid: %d",
             voice_reg_state_cache[m_slot_id]->radio_technology);
        goto error;
    }

    /* <eAct> mapping */
    if (!isInService(voice_reg_state_cache[m_slot_id]->registration_state))
    {
        if (convertCSNetworkType(voice_reg_state_cache[m_slot_id]->radio_technology) == 14)
        {
            m4gRat = true;
        }
        if (convertCSNetworkType(voice_reg_state_cache[m_slot_id]->radio_technology)
                == RADIO_TECH_NR)
        {
            m5gRat = true;
        }
        voice_reg_state_cache[m_slot_id]->radio_technology = 0;
    }
    else
    {
        voice_reg_state_cache[m_slot_id]->radio_technology =
                convertCSNetworkType(voice_reg_state_cache[m_slot_id]->radio_technology);
    }

    voice_reg_state_cache[m_slot_id]->network_exist = mMdEreg->nwk_existence;

    voice_reg_state_cache[m_slot_id]->roaming_indicator = mMdEreg->roam_indicator;

    if (!isInService(voice_reg_state_cache[m_slot_id]->registration_state)) {
        voice_reg_state_cache[m_slot_id]->roaming_indicator = m_ps_roaming_ind;
    }

    if (voice_reg_state_cache[m_slot_id]->roaming_indicator >= 255) {
        voice_reg_state_cache[m_slot_id]->roaming_indicator = 1;
    }

    /* <cause_type> */
    cause_type = mMdEreg->cause_type;
    if (cause_type != 0)
    {
        logE(LOG_TAG, "The value in the field <cause_type> is not valid: %d", cause_type);
        goto error;
    }

    /* <reject_cause> */
    voice_reg_state_cache[m_slot_id]->denied_reason = mMdEreg->reject_cause;
    count = 14;
    pthread_mutex_unlock(&mdEregMutex[m_slot_id]);

#ifdef MTK_TC1_COMMON_SERVICE
    if(!isInService(voice_reg_state_cache[m_slot_id]->registration_state)) {
        int reg_info = 0;
        int ext_info = 0;
        p_response = NULL;

        // +CIREG: <n>,<reg_info>[,<ext_info>]
        p_response = atSendCommandSingleline("AT+CIREG?", "+CIREG:");

        // check error
        err = p_response->getError();
        if (err != 0 ||
                p_response == NULL ||
                p_response->getSuccess() == 0 ||
                p_response->getIntermediates() == NULL) goto done;

        // handle intermediate
        line = p_response->getIntermediates();

        // go to start position
        line->atTokStart(&err);
        if (err < 0) goto done;

        /* Enables or disables reporting of changes */
        skip = line->atTokNextint(&err);
        if (err < 0) goto done;

        /* Indicates the IMS registration status */
        reg_info = line->atTokNextint(&err);
        if (err < 0) goto done;

        if (reg_info == 1) {
            ext_info = line->atTokNextint(&err);
            if (err < 0) goto done;

            if ((ext_info & 0x1) == 1) {
                // VoLTE attach, query CGREG state and return
                p_response = NULL;
                p_response = atSendCommandSingleline("AT+EGREG?", "+EGREG:");

                // check error
                err = p_response->getError();
                if (err != 0 ||
                        p_response == NULL ||
                        p_response->getSuccess() == 0 ||
                        p_response->getIntermediates() == NULL) goto done;

                // handle intermediate
                line = p_response->getIntermediates();

                // go to start position
                line->atTokStart(&err);
                if (err < 0) goto done;

                /* <n> */
                skip = line->atTokNextint(&err);
                if (err < 0) goto done;

                /* <stat> */
                reg_info = line->atTokNextint(&err);
                if (err < 0 || reg_info > 10) {
                    logE(LOG_TAG, "The value in the field <stat> is not valid: %d", reg_info);
                    goto done;
                }
                voice_reg_state_cache[m_slot_id]->registration_state
                        = convertRegState(reg_info, true);

                if (line->atTokHasmore()) {
                    /* <lac> */
                    voice_reg_state_cache[m_slot_id]->lac = line->atTokNexthexint(&err);
                    if (err < 0 ||
                            (voice_reg_state_cache[m_slot_id]->lac > 0xffff &&
                            voice_reg_state_cache[m_slot_id]->lac != 0xffffffff)) {
                        logE(LOG_TAG, "The value in the field <lac> is not valid: %d",
                                voice_reg_state_cache[m_slot_id]->lac);
                        goto done;
                    }

                    /* <cid> */
                    voice_reg_state_cache[m_slot_id]->cid = line->atTokNexthexint(&err);
                    if (err < 0 ||
                            (voice_reg_state_cache[m_slot_id]->cid > 0x0fffffff &&
                            voice_reg_state_cache[m_slot_id]->cid != 0xffffffff)) {
                        logE(LOG_TAG, "The value in the field <cid> is not valid: %llX",
                                voice_reg_state_cache[m_slot_id]->cid);
                        goto done;
                    }

                    /* <eAct> */
                    voice_reg_state_cache[m_slot_id]->radio_technology = line->atTokNextint(&err);
                    if (err < 0) {
                        logE(LOG_TAG, "No eAct in command");
                        goto done;
                    }
                }
            }
        }
    }
done:
#endif

    // ECC support - START
    // IMS ecc
    // if(ims_registerred == 0 && (ims_ecc_support > 0 || ecc_bearer_support > 0)) {
    if (ims_ecc_support > 0) {
        ims_emergency_only = 1;
    }
    if(!isInService(voice_reg_state_cache[m_slot_id]->registration_state) &&
            !isInService(data_reg_state_cache[m_slot_id]->registration_state) &&
            (ims_emergency_only == 1)) {
        m_ims_ecc_only = 1;
    } else {
        m_ims_ecc_only = 0;
    }
    // CDMA ECC
    if (voice_reg_state_cache[m_slot_id]->network_exist == 1) {
        cdma_network_exist = 1;
    } else {
        if (((voice_reg_state_cache[m_slot_id]->registration_state == 0) ||
                    (voice_reg_state_cache[m_slot_id]->registration_state == 2) ||
                    (voice_reg_state_cache[m_slot_id]->registration_state == 3)) &&
                    ((voice_reg_state_cache[m_slot_id]->cid != 0x0fffffff) &&
                    (voice_reg_state_cache[m_slot_id]->lac != 0xffff) &&
                    (voice_reg_state_cache[m_slot_id]->cid != (unsigned int)-1) &&
                    (voice_reg_state_cache[m_slot_id]->lac != (unsigned int)-1)) &&
                    (!m4gRat) &&
                    (!m5gRat)) {
                cdma_network_exist = 1;
            }
    }
    if ((cdma_network_exist == 1) && (radio_state == RADIO_STATE_ON) &&
            !isInService(voice_reg_state_cache[m_slot_id]->registration_state) &&
            !isInService(data_reg_state_cache[m_slot_id]->registration_state)) {
        m_cdma_emergency_only = 1;
    } else {
        m_cdma_emergency_only = 0;
    }
    // TODO: restricted state for dual SIM
    if(voice_reg_state_cache[m_slot_id]->registration_state == 4) {
        /* registration_state(4) is 'Unknown' */
        // logD(LOG_TAG, "No valid info to distinguish limited service and no service");
    } else {
        // if cid is 0x0fffffff means it is invalid
        // if lac is 0xffff means it is invalid
        if (((voice_reg_state_cache[m_slot_id]->registration_state == 0) ||
                (voice_reg_state_cache[m_slot_id]->registration_state == 2) ||
                (voice_reg_state_cache[m_slot_id]->registration_state == 3)) &&
                ((voice_reg_state_cache[m_slot_id]->cid != 0x0fffffff) &&
                (voice_reg_state_cache[m_slot_id]->lac != 0xffff) &&
                (voice_reg_state_cache[m_slot_id]->cid != (unsigned int)-1) &&
                (voice_reg_state_cache[m_slot_id]->lac != (unsigned int)-1)) &&
                // do not set ECC when it is LTE/NR. ECC depends IMS.
                (!m4gRat) &&
                (!m5gRat)) {
            if (m_emergency_only == 0) {
                m_emergency_only = 1;
                // logD(LOG_TAG, "Set slot%d m_emergencly_only to true", m_slot_id);
            }
        } else {
            if(m_emergency_only == 1) {
                m_emergency_only = 0;
                // logD(LOG_TAG, "Set slot%d s_emergencly_only to false", m_slot_id);
            }
        }
    }

    voice_reg_state_cache[m_slot_id]->registration_state =
            convertToAndroidRegState(voice_reg_state_cache[m_slot_id]->registration_state);
    // ECC support -END

    pthread_mutex_unlock(&s_voiceRegStateMutex[m_slot_id]);

    // TODO: only query cdma state if C2K support and on cdma slot
    cntCdma = requestVoiceRegistrationStateCdma(msg);
    if (cntCdma == 0) {
        // response contains error
        resetVoiceRegStateCache(voice_reg_state_cache[m_slot_id], CACHE_GROUP_C2K);
        // logD(LOG_TAG, "C2K voice response error");
    } else if (cntCdma > count) {
        count = cntCdma;
    }
    combineVoiceRegState(msg);

    return;

error:
    pthread_mutex_unlock(&mdEregMutex[m_slot_id]);
    pthread_mutex_unlock(&s_voiceRegStateMutex[m_slot_id]);
    logE(LOG_TAG, "requestVoiceRegistrationState must never return an error when radio is on");
    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
            RfxStringsData(), msg, false);
    // response to TeleCore
    responseToTelCore(response);
    return;
}

int RmcNetworkRealTimeRequestHandler::requestVoiceRegistrationStateCdma(const sp<RfxMclMessage>& msg) {
    RFX_UNUSED(msg);
    int err = 0;
    ViaBaseHandler *mViaHandler = RfxViaUtils::getViaHandler();
    pthread_mutex_lock(&s_voiceRegStateMutex[m_slot_id]);
    if (mViaHandler != NULL) {
        err = mViaHandler->getCdmaLocationInfo(this, voice_reg_state_cache[m_slot_id]);
        if (err < 0) {
            goto error;
        }
    } else goto error;
    pthread_mutex_unlock(&s_voiceRegStateMutex[m_slot_id]);
    return 16;

error:
    pthread_mutex_unlock(&s_voiceRegStateMutex[m_slot_id]);
    // logE(LOG_TAG, "AT+VLOCINFO? response error");
    return 0;
}

static unsigned int convertCellSpeedSupport(unsigned int uiResponse)
{
    // Cell speed support is bitwise value of cell capability:
    // bit7 0x80  bit6 0x40  bit5 0x20  bit4 0x10  bit3 0x08  bit2 0x04  bit1 0x02  bit0 0x01
    // Dual-Cell  HSUPA+     HSDPA+     HSUPA      HSDPA      UMTS       EDGE       GPRS
    unsigned int uiRet = 0;

    unsigned int RIL_RADIO_TECHNOLOGY_MTK = 128;

    if ((uiResponse & 0x2000) != 0) {
        //support carrier aggregation (LTEA)
        uiRet = 19; // ServiceState.RIL_RADIO_TECHNOLOGY_LTE_CA
    } else if ((uiResponse & 0x1000) != 0) {
        uiRet = 14; // ServiceState.RIL_RADIO_TECHNOLOGY_LTE
    } else if ((uiResponse & 0x80) != 0 ||
            (uiResponse & 0x40) != 0 ||
            (uiResponse & 0x20) != 0) {
        uiRet = 15; // ServiceState.RIL_RADIO_TECHNOLOGY_HSPAP
    } else if ((uiResponse & 0x10) != 0 &&
            (uiResponse & 0x08) != 0) {
        uiRet = 11; // ServiceState.RIL_RADIO_TECHNOLOGY_HSPA
    } else if ((uiResponse & 0x10) != 0) {
        uiRet = 10; // ServiceState.RIL_RADIO_TECHNOLOGY_HSUPA
    } else if ((uiResponse & 0x08) != 0) {
        uiRet = 9;  // ServiceState.RIL_RADIO_TECHNOLOGY_HSDPA
    } else if ((uiResponse & 0x04) != 0) {
        uiRet = 3;  // ServiceState.RIL_RADIO_TECHNOLOGY_UMTS
    } else if ((uiResponse & 0x02) != 0) {
        uiRet = 2;  // ServiceState.RIL_RADIO_TECHNOLOGY_EDGE
    } else if ((uiResponse & 0x01) != 0) {
        uiRet = 1;  // ServiceState.RIL_RADIO_TECHNOLOGY_GPRS
    } else {
        uiRet = 0;  // ServiceState.RIL_RADIO_TECHNOLOGY_UNKNOWN
    }

    return uiRet;
}

static unsigned int convertPSBearerCapability(unsigned int uiResponse)
{
    /*
     *typedef enum
     *{
     *    L4C_NONE_ACTIVATE = 0,
     *    L4C_GPRS_CAPABILITY,
     *    L4C_EDGE_CAPABILITY,
     *    L4C_UMTS_CAPABILITY,
     *    L4C_HSDPA_CAPABILITY, //mac-hs
     *    L4C_HSUPA_CAPABILITY, //mac-e/es
     *    L4C_HSDPA_HSUPA_CAPABILITY, //mac-hs + mac-e/es
     *
     *    L4C_HSDPAP_CAPABILITY, //mac-ehs
     *    L4C_HSDPAP_UPA_CAPABILITY, //mac-ehs + mac-e/es
     *    L4C_HSUPAP_CAPABILITY, //mac-i/is
     *    L4C_HSUPAP_DPA_CAPABILITY, //mac-hs + mac-i/is
     *    L4C_HSPAP_CAPABILITY, //mac-ehs + mac-i/is
     *    L4C_DC_DPA_CAPABILITY, //(DC) mac-hs
     *    L4C_DC_DPA_UPA_CAPABILITY, //(DC) mac-hs + mac-e/es
     *    L4C_DC_HSDPAP_CAPABILITY, //(DC) mac-ehs
     *    L4C_DC_HSDPAP_UPA_CAPABILITY, //(DC) mac-ehs + mac-e/es
     *    L4C_DC_HSUPAP_DPA_CAPABILITY, //(DC) mac-hs + mac-i/is
     *    L4C_DC_HSPAP_CAPABILITY, //(DC) mac-ehs + mac-i/is
     *    L4C_LTE_CAPABILITY
     *} l4c_data_bearer_capablility_enum;
     */

    unsigned int uiRet = 0;

    unsigned int RIL_RADIO_TECHNOLOGY_MTK = 128;

    switch (uiResponse)
    {
        case 1:
            uiRet = 1;                           // ServiceState.RIL_RADIO_TECHNOLOGY_GPRS
            break;
        case 2:
            uiRet = 2;                           // ServiceState.RIL_RADIO_TECHNOLOGY_EDGE
            break;
        case 3:
            uiRet = 3;                           // ServiceState.RIL_RADIO_TECHNOLOGY_UMTS
            break;
        case 4:
            uiRet = 9;                           // ServiceState.RIL_RADIO_TECHNOLOGY_HSDPA
            break;
        case 5:
            uiRet = 10;                          // ServiceState.RIL_RADIO_TECHNOLOGY_HSUPA
            break;
        case 6:
            uiRet = 11;                          // ServiceState.RIL_RADIO_TECHNOLOGY_HSPA
            break;
        case 7:
            uiRet = RIL_RADIO_TECHNOLOGY_MTK+1;  // ServiceState.RIL_RADIO_TECHNOLOGY_HSDPAP
            break;
        case 8:
            uiRet = RIL_RADIO_TECHNOLOGY_MTK+2;  // ServiceState.RIL_RADIO_TECHNOLOGY_HSDPAP_UPA
            break;
        case 9:
            uiRet = RIL_RADIO_TECHNOLOGY_MTK+3;  // ServiceState.RIL_RADIO_TECHNOLOGY_HSUPAP
            break;
        case 10:
            uiRet = RIL_RADIO_TECHNOLOGY_MTK+4;  // ServiceState.RIL_RADIO_TECHNOLOGY_HSUPAP_DPA
            break;
        case 11:
            uiRet = 15;                          // ServiceState.RIL_RADIO_TECHNOLOGY_HSPAP
            break;
        case 12:
            uiRet = RIL_RADIO_TECHNOLOGY_MTK+5;  // ServiceState.RIL_RADIO_TECHNOLOGY_DC_DPA
            break;
        case 13:
            uiRet = RIL_RADIO_TECHNOLOGY_MTK+6;  // ServiceState.RIL_RADIO_TECHNOLOGY_DC_UPA
            break;
        case 14:
            uiRet = RIL_RADIO_TECHNOLOGY_MTK+7;  // ServiceState.RIL_RADIO_TECHNOLOGY_DC_HSDPAP
            break;
        case 15:
            uiRet = RIL_RADIO_TECHNOLOGY_MTK+8;  // ServiceState.RIL_RADIO_TECHNOLOGY_DC_HSDPAP_UPA
            break;
        case 16:
            uiRet = RIL_RADIO_TECHNOLOGY_MTK+9;  // ServiceState.RIL_RADIO_TECHNOLOGY_DC_HSDPAP_DPA
            break;
        case 17:
            uiRet = RIL_RADIO_TECHNOLOGY_MTK+10; // ServiceState.RIL_RADIO_TECHNOLOGY_DC_HSPAP
            break;
        case 18:
            uiRet = 14;                          // ServiceState.RIL_RADIO_TECHNOLOGY_LTE
            break;
        default:
            uiRet = 0;                           // ServiceState.RIL_RADIO_TECHNOLOGY_UNKNOWN
            break;
    }

    return uiRet;
}

void RmcNetworkRealTimeRequestHandler::sendDataRegResponse(const sp<RfxMclMessage>& msg) {
    RIL_DataRegistrationStateResponse *pData = (RIL_DataRegistrationStateResponse *)
            alloca(sizeof(RIL_DataRegistrationStateResponse));
    int err = 0;
    int mcc = 0;
    int mnc = 0;
    int mnc_len = 0;
    char longname[MAX_OPER_NAME_LENGTH], shortname[MAX_OPER_NAME_LENGTH];

    memset(pData, 0, sizeof(RIL_DataRegistrationStateResponse));
    pData->regState = (RIL_RegState)data_reg_state_cache[m_slot_id]->registration_state;
    pData->rat = (RIL_RadioTechnology)data_reg_state_cache[m_slot_id]->radio_technology;
    if ((urc_data_reg_state_cache[m_slot_id]->denied_reason > 0) &&
        (urc_data_reg_state_cache[m_slot_id]->lac == data_reg_state_cache[m_slot_id]->lac) &&
        (urc_data_reg_state_cache[m_slot_id]->cid == data_reg_state_cache[m_slot_id]->cid)) {
         // we have reject cause with the same lac/cid, we use it.
        pData->reasonDataDenied = urc_data_reg_state_cache[m_slot_id]->denied_reason;
    } else {
        pData->reasonDataDenied = data_reg_state_cache[m_slot_id]->denied_reason;
    }
    pData->maxDataCalls = data_reg_state_cache[m_slot_id]->max_simultaneous_data_call;
    // Add for MTK_TC1_FEATURE - ERI
#ifdef MTK_TC1_COMMON_SERVICE
    if (RfxNwServiceState::isInService(data_reg_state_cache[m_slot_id]->registration_state)
            && RfxNwServiceState::isCdmaGroup(data_reg_state_cache[m_slot_id]->radio_technology)) {
        if (isFemtocellSupport() && isCdmaDataInFemtocell()) {
            // Support the Femtocell network: The response values of Roaming Indicator
            // should be the value "99" while connected to the Femtocell network.
            pData->dataRoamingIndicator = 99;
        } else {
            pData->dataRoamingIndicator = data_reg_state_cache[m_slot_id]->roaming_indicator;
        }
    } else {
        pData->dataDefaultRoamingIndicator = -1;
        if (RfxNwServiceState::isInService(data_reg_state_cache[m_slot_id]->registration_state)
                && (data_reg_state_cache[m_slot_id]->radio_technology == RADIO_TECH_LTE
                || data_reg_state_cache[m_slot_id]->radio_technology == RADIO_TECH_LTE_CA)) {
            if (femto_cell_cache[m_slot_id]->is_femtocell != 0 ||
                    femto_cell_cache[m_slot_id]->is_csg_cell == 1) {
                pData->dataRoamingIndicator = 99;
            }
        }
    }
#endif

    if (strlen(op_info_cache[m_slot_id]->mccMnc) == 5 || strlen(op_info_cache[m_slot_id]->mccMnc) == 6) {
        if (strlen(op_info_cache[m_slot_id]->mccMnc) == 6) mnc_len = 3;
        else mnc_len = 2;
        char mcc_str[4] = {0};
        char mnc_str[4] = {0};
        strncpy(mcc_str, op_info_cache[m_slot_id]->mccMnc, 3);
        strncpy(mnc_str, (op_info_cache[m_slot_id]->mccMnc+3), (strlen(op_info_cache[m_slot_id]->mccMnc)-3));
        mcc = atoi(mcc_str);
        mnc = atoi(mnc_str);
        err = getOperatorNamesFromNumericCode(
                op_info_cache[m_slot_id]->mccMnc, longname, shortname, MAX_OPER_NAME_LENGTH);
        if (err < 0) {
            memset(longname, 0, MAX_OPER_NAME_LENGTH);
            memset(shortname, 0, MAX_OPER_NAME_LENGTH);
        }
    } else {
        mcc = INT_MAX;
        mnc = INT_MAX;
        // reset longname/shortname because mccmnc may be empty and these are empty too.
        // It happends when the MD is in service right after poll_operator and before poll_voice/data
        memset(longname, 0, MAX_OPER_NAME_LENGTH);
        memset(shortname, 0, MAX_OPER_NAME_LENGTH);
    }

    if (isInService(data_reg_state_cache[m_slot_id]->registration_state)) {
        refreshEcellCache();
        // if AT+ECELL provides old data
        if (data_reg_state_cache[m_slot_id]->lac != mdEcell[m_slot_id]->lacTac ||
                data_reg_state_cache[m_slot_id]->cid != mdEcell[m_slot_id]->cid) {
            // reset
            memset(mdEcell[m_slot_id], 0, sizeof(MD_ECELL));
            mdEcell[m_slot_id]->bsic = 0xFF;
            mdEcell[m_slot_id]->arfcn = INT_MAX;
            mdEcell[m_slot_id]->pscPci = INT_MAX;
        }
    }

    pData->cellIdentity.cellInfoType =
            getCellInfoTypeRadioTechnology(data_reg_state_cache[m_slot_id]->radio_technology);
    switch(pData->cellIdentity.cellInfoType) {
        case RIL_CELL_INFO_TYPE_GSM: {
            pData->cellIdentity.cellIdentityGsm.mcc = mcc;
            pData->cellIdentity.cellIdentityGsm.mnc = mnc;
            pData->cellIdentity.cellIdentityGsm.mnc_len = mnc_len;
            pData->cellIdentity.cellIdentityGsm.lac = data_reg_state_cache[m_slot_id]->lac;
            pData->cellIdentity.cellIdentityGsm.cid = (int) data_reg_state_cache[m_slot_id]->cid;
            pData->cellIdentity.cellIdentityGsm.arfcn = mdEcell[m_slot_id]->arfcn;
            pData->cellIdentity.cellIdentityGsm.bsic = 0xFF;
            pData->cellIdentity.cellIdentityGsm.operName.long_name = longname;
            pData->cellIdentity.cellIdentityGsm.operName.short_name = shortname;
            break;
        }

        case RIL_CELL_INFO_TYPE_WCDMA: {
            pData->cellIdentity.cellIdentityWcdma.mcc = mcc;
            pData->cellIdentity.cellIdentityWcdma.mnc = mnc;
            pData->cellIdentity.cellIdentityWcdma.mnc_len= mnc_len;
            pData->cellIdentity.cellIdentityWcdma.lac = data_reg_state_cache[m_slot_id]->lac;
            pData->cellIdentity.cellIdentityWcdma.cid = (int) data_reg_state_cache[m_slot_id]->cid;
            pData->cellIdentity.cellIdentityWcdma.psc = voice_reg_state_cache[m_slot_id]->psc;
            pData->cellIdentity.cellIdentityWcdma.uarfcn = mdEcell[m_slot_id]->arfcn;
            pData->cellIdentity.cellIdentityWcdma.operName.long_name = longname;
            pData->cellIdentity.cellIdentityWcdma.operName.short_name = shortname;
            break;
        }

        case RIL_CELL_INFO_TYPE_CDMA: {
            pData->cellIdentity.cellIdentityCdma.networkId = voice_reg_state_cache[m_slot_id]->network_id;
            pData->cellIdentity.cellIdentityCdma.systemId = voice_reg_state_cache[m_slot_id]->system_id;
            pData->cellIdentity.cellIdentityCdma.basestationId = voice_reg_state_cache[m_slot_id]->base_station_id;
            pData->cellIdentity.cellIdentityCdma.longitude = voice_reg_state_cache[m_slot_id]->base_station_longitude;
            pData->cellIdentity.cellIdentityCdma.latitude = voice_reg_state_cache[m_slot_id]->base_station_latitude;
            pData->cellIdentity.cellIdentityCdma.operName.long_name = longname;
            pData->cellIdentity.cellIdentityCdma.operName.short_name = shortname;
            break;
        }

        case RIL_CELL_INFO_TYPE_LTE: {
            pData->cellIdentity.cellIdentityLte.mcc = mcc;
            pData->cellIdentity.cellIdentityLte.mnc = mnc;
            pData->cellIdentity.cellIdentityLte.mnc_len= mnc_len;
            pData->cellIdentity.cellIdentityLte.ci = (int) data_reg_state_cache[m_slot_id]->cid;
            pData->cellIdentity.cellIdentityLte.pci = mdEcell[m_slot_id]->pscPci;
            pData->cellIdentity.cellIdentityLte.tac = data_reg_state_cache[m_slot_id]->lac; //RILD doesn't use Tac.
            pData->cellIdentity.cellIdentityLte.earfcn = mdEcell[m_slot_id]->arfcn;
            pData->cellIdentity.cellIdentityLte.operName.long_name = longname;
            pData->cellIdentity.cellIdentityLte.operName.short_name = shortname;
            pData->cellIdentity.cellIdentityLte.bandwidth = 100 * ca_cache[m_slot_id]->pcell_bw;
            // lte vops info
            pData->lteVopsInfo.isVopsSupported = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_VOPS);
            pData->lteVopsInfo.isEmcBearerSupported = ims_ecc_state[m_slot_id] ? 1 : 0;
            // NR non-standalone
            pData->nrIndicators.isDcNrRestricted = data_reg_state_cache[m_slot_id]->dcnr_restricted;
            pData->nrIndicators.isNrAvailable = data_reg_state_cache[m_slot_id]->endc_sib;
            pData->nrIndicators.isEndcAvailable = data_reg_state_cache[m_slot_id]->endc_available;
            break;
        }

        case RIL_CELL_INFO_TYPE_TD_SCDMA: {
            pData->cellIdentity.cellIdentityTdscdma.mcc = mcc;
            pData->cellIdentity.cellIdentityTdscdma.mnc = mnc;
            pData->cellIdentity.cellIdentityTdscdma.mnc_len = mnc_len;
            pData->cellIdentity.cellIdentityTdscdma.lac = data_reg_state_cache[m_slot_id]->lac;
            pData->cellIdentity.cellIdentityTdscdma.cid = (int) data_reg_state_cache[m_slot_id]->cid;
            pData->cellIdentity.cellIdentityTdscdma.cpid = INT_MAX;
            pData->cellIdentity.cellIdentityTdscdma.operName.long_name = longname;
            pData->cellIdentity.cellIdentityTdscdma.operName.short_name = shortname;
            break;
        }

        case RIL_CELL_INFO_TYPE_NR:
            pData->cellIdentity.cellIdentityNr.mcc = mcc;
            pData->cellIdentity.cellIdentityNr.mnc = mnc;
            pData->cellIdentity.cellIdentityNr.tac = data_reg_state_cache[m_slot_id]->lac;
            pData->cellIdentity.cellIdentityNr.nci = data_reg_state_cache[m_slot_id]->cid;
            pData->cellIdentity.cellIdentityNr.pci = INT_MAX;
            pData->cellIdentity.cellIdentityNr.operName.long_name = longname;
            pData->cellIdentity.cellIdentityNr.operName.short_name = shortname;
            break;
        default: {
            break;
        }
    }

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxDataRegStateData(pData, sizeof(RIL_DataRegistrationStateResponse)), msg, false);
    // response to TeleCore
    responseToTelCore(response);

}

void RmcNetworkRealTimeRequestHandler::combineDataRegState(const sp<RfxMclMessage>& msg) {
    updateServiceStateValue();
    sendDataRegResponse(msg);
}

void RmcNetworkRealTimeRequestHandler::printDataCache() {
    char* lac_s = getMask(data_reg_state_cache[m_slot_id]->lac);
    char* cid_s = getMask((int) data_reg_state_cache[m_slot_id]->cid);
    if (!lac_s || !cid_s) {
        logE(LOG_TAG, "[%s] can not get memeory to print log", __func__);
    } else {
        logV(LOG_TAG, "printDataCache: reg=%d, lac=%s, cid=%s, rat=%d, reason=%d",
                 data_reg_state_cache[m_slot_id]->registration_state,
                 lac_s,
                 cid_s,
                 data_reg_state_cache[m_slot_id]->radio_technology,
                 data_reg_state_cache[m_slot_id]->denied_reason);
    }
    if (lac_s) free(lac_s);
    if (cid_s) free(cid_s);
}

void RmcNetworkRealTimeRequestHandler::requestDataRegistrationState(const sp<RfxMclMessage>& msg)
{
    /* +EGREG: <n>, <stat>, <lac>, <cid>, <eAct>, <rac>,
            <nwk_existence>, <roam_indicator>, <cause_type>, <reject_cause> */

    int exist = 0;
    int err;
    sp<RfxAtResponse> p_response;
    RfxAtLine* line;
    int skip;
    int count = 3;
    int cause_type;
    MD_EGREG *mMdEgreg = mdEgreg[m_slot_id];
    MD_ECELL *mMdEcell = mdEcell[m_slot_id];
    pthread_mutex_lock(&mdEgregMutex[m_slot_id]);

    resetDataRegStateCache(data_reg_state_cache[m_slot_id], CACHE_GROUP_COMMON_REQ);

    dumpEgregCache(__func__, mMdEgreg, mMdEcell, true);

    /* <stat> */
    data_reg_state_cache[m_slot_id]->registration_state = mMdEgreg->stat;

    if (data_reg_state_cache[m_slot_id]->registration_state > 10 )  //for LTE
    {
        // TODO: add C2K flag
        if (data_reg_state_cache[m_slot_id]->registration_state < 101 ||
                data_reg_state_cache[m_slot_id]->registration_state > 104)  // for C2K
        {
            logE(LOG_TAG, "The value in the field <stat> is not valid: %d",
                 data_reg_state_cache[m_slot_id]->registration_state);
            goto error;
        }
    }

    //For LTE and C2K
    data_reg_state_cache[m_slot_id]->registration_state =
            convertRegState(data_reg_state_cache[m_slot_id]->registration_state, false);

    /* <lac> */
    data_reg_state_cache[m_slot_id]->lac = mMdEgreg->lac;
    if (data_reg_state_cache[m_slot_id]->lac > 0xffff &&
            data_reg_state_cache[m_slot_id]->lac != 0xffffffff) {
        logE(LOG_TAG, "The value in the field <lac> or <stat> is not valid. <stat>:%d, <lac>:%d",
                 data_reg_state_cache[m_slot_id]->registration_state,
                 data_reg_state_cache[m_slot_id]->lac);
        goto error;
    }

    /* <ci> */
    data_reg_state_cache[m_slot_id]->cid = mMdEgreg->ci;
    if ((mMdEgreg->eAct != 0x8000 &&  // 2/3/4g
            data_reg_state_cache[m_slot_id]->cid > 0x0fffffff &&
            data_reg_state_cache[m_slot_id]->cid != 0xffffffff) ||
            (mMdEgreg->eAct == 0x8000 &&  // 5g
            data_reg_state_cache[m_slot_id]->cid > 0x0fffffffff &&
            data_reg_state_cache[m_slot_id]->cid != 0xffffffffff)) {
        logE(LOG_TAG, "The value in the field <ci> is not valid: %llX",
                data_reg_state_cache[m_slot_id]->cid);
        goto error;
    }

    /* <eAct> */
    data_reg_state_cache[m_slot_id]->radio_technology = mMdEgreg->eAct;
    count = 4;
    if (data_reg_state_cache[m_slot_id]->radio_technology > 0x8000)  // NR is 0x8000
    {
        logE(LOG_TAG, "The value in the eAct is not valid: %d",
             data_reg_state_cache[m_slot_id]->radio_technology);
        goto error;
    }

    /* <eAct> mapping */
    if (!isInService(data_reg_state_cache[m_slot_id]->registration_state))
    {
        data_reg_state_cache[m_slot_id]->lac = -1;
        data_reg_state_cache[m_slot_id]->cid = -1;
        data_reg_state_cache[m_slot_id]->radio_technology = 0;
    }
    else
    {
        data_reg_state_cache[m_slot_id]->radio_technology =
                convertPSNetworkType(data_reg_state_cache[m_slot_id]->radio_technology);
    }

    exist = mMdEgreg->nwk_existence;

    m_ps_roaming_ind = mMdEgreg->roam_indicator;

    // if NR support, [<dcnr_restricted>],[<endc_sib>],[<endc_available>]
    if (isNrSupported()) {
        data_reg_state_cache[m_slot_id]->dcnr_restricted = mMdEgreg->dcnr_restricted;

        data_reg_state_cache[m_slot_id]->endc_sib = mMdEgreg->endc_sib == 2 ? 1 : 0;

        data_reg_state_cache[m_slot_id]->endc_available = mMdEgreg->endc_available;
    }

    // if eact is 4000, we can force it.
    if (mMdEgreg->eAct == 0x4000) {
        data_reg_state_cache[m_slot_id]->dcnr_restricted = 0;
        data_reg_state_cache[m_slot_id]->endc_sib = 1;
        data_reg_state_cache[m_slot_id]->endc_available = 1;
    }

    /* <cause_type> */
    cause_type = mMdEgreg->cause_type;
    if (cause_type != 0)
    {
        logE(LOG_TAG, "The value in the field <cause_type> is not valid: %d", cause_type);
        goto error;
    }
    if (isFemtocellSupport()) {
        isFemtoCell(data_reg_state_cache[m_slot_id]->registration_state,
                (int) data_reg_state_cache[m_slot_id]->cid,
                data_reg_state_cache[m_slot_id]->radio_technology);
    }

    /* <reject_cause> */
    data_reg_state_cache[m_slot_id]->denied_reason = mMdEgreg->reject_cause;
    count = 5;

    pthread_mutex_unlock(&mdEgregMutex[m_slot_id]);
    // Query PSBEARER when PS registered
    if (isInService(data_reg_state_cache[m_slot_id]->registration_state)
               && data_reg_state_cache[m_slot_id]->radio_technology != 6
               && data_reg_state_cache[m_slot_id]->radio_technology != 8
               && data_reg_state_cache[m_slot_id]->radio_technology != 13) {
        requestDataRegistrationStateGsm();
        int bw_sum = 0;
        // check ca is actived or config
        // if nw support ca but not use ca, ca_band[0] should be 3 (deactive)
        if (ca_cache[m_slot_id]->ca_info == 2 || ca_cache[m_slot_id]->ca_info == 0) {
            bw_sum = ca_cache[m_slot_id]->pcell_bw +
                ca_cache[m_slot_id]->scell_bw1 +
                ca_cache[m_slot_id]->scell_bw2 +
                ca_cache[m_slot_id]->scell_bw3 +
                ca_cache[m_slot_id]->scell_bw4;
        }
        if (ca_filter_switch[m_slot_id]) {
            if (data_reg_state_cache[m_slot_id]->radio_technology == 19 &&
                bw_sum <= 200) {
                // downgrade LTE_CA to LTE when sum of bands is 20Mhz or less
                logD(LOG_TAG, "Downgrade LTE_CA to LTE when ca_info:%d, bw_sum:%d MHZ",
                        ca_cache[m_slot_id]->ca_info, bw_sum/10);
                data_reg_state_cache[m_slot_id]->radio_technology = 14;
            }
        }
    } else {
        resetCaCache(ca_cache[m_slot_id]);
        m_dc_support[m_slot_id] = false;
    }
    printDataCache();
    //TODO: restricted state for dual SIM

    // After data_reg & ca_cache is updated, send PhysicalChannelConfigs URC
    sendEvent(RFX_MSG_EVENT_RERESH_PHYSICAL_CONFIG, RfxVoidData(),
            RIL_CMD_PROXY_3, m_slot_id);

    combineDataRegState(msg);

    return;

error:
    pthread_mutex_unlock(&mdEgregMutex[m_slot_id]);
    logE(LOG_TAG, "requestDataRegistrationState must never return an error when radio is on");
    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
            RfxStringsData(), msg, false);
    // response to TeleCore
    responseToTelCore(response);
    return;
}

void RmcNetworkRealTimeRequestHandler::requestDataRegistrationStateGsm()
{
    int err;
    sp<RfxAtResponse> p_response;
    RfxAtLine* line;
    int psBearerCount = 0;
    int cell_data_speed = 0;
    int max_data_bearer = 0;

    //support carrier aggregation (LTEA)
    int ignoreMaxDataBearerCapability = 0;

    // send AT command
    p_response = atSendCommandSingleline("AT+PSBEARER?", "+PSBEARER:");

    // check error
    err = p_response->getError();
    if (err != 0 ||
            p_response == NULL ||
            p_response->getSuccess() == 0 ||
            p_response->getIntermediates() == NULL) goto skipR7R8;

    // handle intermediate
    line = p_response->getIntermediates();

    // go to start position
    line->atTokStart(&err);
    if (err < 0) goto skipR7R8;

    /* <cell_data_speed_support> */
    cell_data_speed = line->atTokNextint(&err);
    if (err < 0) goto skipR7R8;
    psBearerCount++;

    // <max_data_bearer_capability> is only support on 3G
    if (cell_data_speed >= 0x1000){
        ignoreMaxDataBearerCapability = 1;
    }

    cell_data_speed = convertCellSpeedSupport(cell_data_speed);

    /* <max_data_bearer_capability> */
    max_data_bearer = line->atTokNextint(&err);
    if (err < 0) goto skipR7R8;
    psBearerCount++;

    if (!ignoreMaxDataBearerCapability) {
        max_data_bearer = convertPSBearerCapability(max_data_bearer);
    } else {
        max_data_bearer = 0;  // ServiceState.RIL_RADIO_TECHNOLOGY_UNKNOWN
    }

    updateDcStatus(cell_data_speed, max_data_bearer);

skipR7R8:

    if (psBearerCount == 2) {
        if (cell_data_speed == 0 && max_data_bearer == 0) {
            return;
        } else if (data_reg_state_cache[m_slot_id]->radio_technology != 20) {  // skip NR
            data_reg_state_cache[m_slot_id]->radio_technology =
                    (cell_data_speed > max_data_bearer) ? cell_data_speed : max_data_bearer;
            // Always return aosp's network type when it is aosp's function.
            if (data_reg_state_cache[m_slot_id]->radio_technology >= 128) {
                // Save mtk_data_type to PROPERTY_MTK_DATA_TYPE
                char* mtk_data_type = NULL;
                asprintf(&mtk_data_type, "%d", data_reg_state_cache[m_slot_id]->radio_technology);
                setMSimProperty(m_slot_id, (char *)PROPERTY_MTK_DATA_TYPE, mtk_data_type);
                if (mtk_data_type) free(mtk_data_type);
                // RIL_RADIO_TECHNOLOGY_HSPAP = 15
                data_reg_state_cache[m_slot_id]->radio_technology = 15;
            } else {
                // clear it if it is not mtk's type
                setMSimProperty(m_slot_id, (char *)PROPERTY_MTK_DATA_TYPE, (char*)"0");
            }
        }
    }
    // update CA band info
    updateCaBandInfo();
    return;

}

void RmcNetworkRealTimeRequestHandler::requestOperator(const sp<RfxMclMessage>& msg)
{
    int err;
    int skip;
    char *resp[3];
    sp<RfxMclMessage> response;
    sp<RfxMclMessage> urc;
    RfxAtLine* line;
    char nemric[MAX_OPER_NAME_LENGTH];
    char longname[MAX_OPER_NAME_LENGTH], shortname[MAX_OPER_NAME_LENGTH];
    sp<RfxAtResponse> p_response;
    MD_EOPS *mMdEops = mdEops[m_slot_id];

    memset(resp, 0, sizeof(resp));

    /* Format should be set during initialization */
    p_response = atSendCommandSingleline("AT+EOPS?", "+EOPS:");

    pthread_mutex_lock(&mdEopsMutex[m_slot_id]);
    memset(mdEops[m_slot_id], 0, sizeof(MD_EOPS));

    // check error
    err = p_response->getError();
    if (err != 0 ||
            p_response == NULL ||
            p_response->getSuccess() == 0 ||
            p_response->getIntermediates() == NULL) {
        logE(LOG_TAG, "EOPS got error response");
        goto error;
    } else {
        // handle intermediate
        line = p_response->getIntermediates();

        // go to start position
        line->atTokStart(&err);
        if (err >= 0) {
            /* <mode> */
            skip = line->atTokNextint(&err);
            if ((err >= 0) &&( skip >= 0 && skip <= 4 && skip != 2)) {
                mMdEops->mode = skip;
                // a "+EOPS: 0" response is possible
                if (line->atTokHasmore()) {
                    /* <format> */
                    skip = line->atTokNextint(&err);
                    if (err >= 0 && skip == 2)
                    {
                        mMdEops->format = skip;
                        /* <oper> */
                        resp[2] = line->atTokNextstr(&err);
                        strncpy(mMdEops->oper, resp[2], strlen(resp[2]) > 9 ? 9 : (1+strlen(resp[2])));
                        /* Modem might response invalid PLMN ex: "", "000000" , "??????", ignore it */
                        // request_operator returns valid plmn when in service.
                        setLastValidPlmn(resp[2]);
                        bool hasService =
                            isInService(urc_data_reg_state_cache[m_slot_id]->registration_state) ||
                            isInService(urc_voice_reg_state_cache[m_slot_id]->registration_state);
                        if ((!hasService) ||
                                (!((*resp[2] >= '0') && (*resp[2] <= '9'))) ||
                                (0 == strcmp(resp[2], "000000"))) {
                            memset(resp, 0, sizeof(resp));
                            if (!hasService) {
                                // clear key/cache when we are not in service
                                // keep the previous value when MD report invalid.
                                getMclStatusManager()->setString8Value(RFX_STATUS_KEY_OPERATOR,
                                        String8(""));
                                memset(op_info_cache[m_slot_id]->mccMnc, 0, 8);
                                memset(op_info_cache[m_slot_id]->longName, 0, MAX_OPER_NAME_LENGTH);
                                memset(op_info_cache[m_slot_id]->shortName, 0, MAX_OPER_NAME_LENGTH);
                            }
                            pthread_mutex_unlock(&mdEopsMutex[m_slot_id]);
                            response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
                                    RfxStringsData(resp, 3), msg, false);
                            // response to TeleCore
                            responseToTelCore(response);
                            return;
                        }

                        err = getOperatorNamesFromNumericCode(
                                resp[2], eons_info[m_slot_id].lac, longname, shortname, MAX_OPER_NAME_LENGTH);
                        String8 oper(resp[2]);
                        getMclStatusManager()->setString8Value(RFX_STATUS_KEY_OPERATOR, oper);

                        if(err >= 0)
                        {
                            resp[0] = longname;
                            resp[1] = shortname;
                        }
                        strncpy(op_info_cache[m_slot_id]->mccMnc, resp[2], strlen(resp[2]));
                        strncpy(op_info_cache[m_slot_id]->longName, longname, strlen(longname));
                        strncpy(op_info_cache[m_slot_id]->shortName, shortname, strlen(shortname));
                        pthread_mutex_unlock(&mdEopsMutex[m_slot_id]);
                        response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
                                RfxStringsData(resp, 3), msg, false);
                        // response to TeleCore
                        responseToTelCore(response);
                        return;
                    }
                }
            }
        }
    }
    pthread_mutex_unlock(&mdEopsMutex[m_slot_id]);
    response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxStringsData(resp, 3), msg, false);
    // response to TeleCore
    responseToTelCore(response);

    return;
error:
    pthread_mutex_unlock(&mdEopsMutex[m_slot_id]);
    logE(LOG_TAG, "requestOperator must never return error when radio is on");
    response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
            RfxStringsData(), msg, false);
    // response to TeleCore
    responseToTelCore(response);
}

void RmcNetworkRealTimeRequestHandler::requestQueryNetworkSelectionMode(const sp<RfxMclMessage>& msg) {
    int err;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> response;
    int resp = 0;
    RfxAtLine* line;

    resp = mdEops[m_slot_id]->mode;
    if (resp < 0 || resp > 4 || resp == 3) {
        goto error;
    }

    logD(LOG_TAG, "requestQueryNetworkSelectionMode return %d", mdEops[m_slot_id]->mode);
    response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxIntsData(&resp, 1), msg, false);
    // response to TeleCore
    responseToTelCore(response);

    return;
error:
    logE(LOG_TAG, "requestQueryNetworkSelectionMode must never return error when radio is on");
    response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
            RfxVoidData(), msg, false);
    // response to TeleCore
    responseToTelCore(response);
}

void RmcNetworkRealTimeRequestHandler::resetVoiceRegStateCache(RIL_VOICE_REG_STATE_CACHE *voiceCache, RIL_CACHE_GROUP source) {
    pthread_mutex_lock(&s_voiceRegStateMutex[m_slot_id]);
    if (source == CACHE_GROUP_ALL) {
        (*voiceCache).registration_state = 0;
        (*voiceCache).lac = 0xffffffff;
        (*voiceCache).cid = 0x0fffffff;
        (*voiceCache).radio_technology = 0;
        (*voiceCache).base_station_id = 0;
        (*voiceCache).base_station_latitude = 0;
        (*voiceCache).base_station_longitude = 0;
        (*voiceCache).css = 0;
        (*voiceCache).system_id = 0;
        (*voiceCache).network_id = 0;
        (*voiceCache).roaming_indicator = -1;
        (*voiceCache).is_in_prl = -1;
        (*voiceCache).default_roaming_indicator = -1;
        (*voiceCache).denied_reason = 0;
        (*voiceCache).psc = -1;
        (*voiceCache).network_exist = 0;
        asprintf(&((*voiceCache).sector_id), "%s", "");
        asprintf(&((*voiceCache).subnet_mask), "%s", "");
    } else if (source == CACHE_GROUP_COMMON_REQ) {
        (*voiceCache).registration_state = 0;
        (*voiceCache).lac = 0xffffffff;
        (*voiceCache).cid = 0x0fffffff;
        (*voiceCache).radio_technology = 0;
        (*voiceCache).roaming_indicator = 0;
        (*voiceCache).denied_reason = 0;
        (*voiceCache).network_exist = 0;
    } else if (source == CACHE_GROUP_C2K) {
        (*voiceCache).base_station_id = 0;
        (*voiceCache).base_station_latitude = 0;
        (*voiceCache).base_station_longitude = 0;
        (*voiceCache).css = 0;
        (*voiceCache).system_id = 0;
        (*voiceCache).network_id = 0;
        asprintf(&((*voiceCache).sector_id), "%s", "");
        asprintf(&((*voiceCache).subnet_mask), "%s", "");
    } else {
        // source type invalid!!!
        logD(LOG_TAG, "updateVoiceRegStateCache(): source type invalid!!!");
    }
    pthread_mutex_unlock(&s_voiceRegStateMutex[m_slot_id]);
}

void RmcNetworkRealTimeRequestHandler::resetDataRegStateCache(RIL_DATA_REG_STATE_CACHE *dataCache,
        RIL_CACHE_GROUP source) {
    (*dataCache).registration_state = 0;
    (*dataCache).lac = 0xffffffff;
    (*dataCache).cid = 0x0fffffff;
    (*dataCache).radio_technology = 0;
    (*dataCache).denied_reason = 0;
    (*dataCache).max_simultaneous_data_call = 1;
    (*dataCache).tac = 0;
    (*dataCache).physical_cid = 0;
    (*dataCache).eci = 0;
    (*dataCache).csgid = 0;
    (*dataCache).tadv = 0;
    // NR none-standalone
    (*dataCache).dcnr_restricted = 0;
    (*dataCache).endc_sib = 0;
    (*dataCache).endc_available = 0;

    // Add for MTK_TC1_FEATURE - ERI
    if (source == CACHE_GROUP_COMMON_REQ) {
        (*dataCache).roaming_indicator = 1;
    } else {
        (*dataCache).roaming_indicator = 1;
        (*dataCache).is_in_prl = 0;
        (*dataCache).default_roaming_indicator = 0;
    }
}

void RmcNetworkRealTimeRequestHandler::updateServiceStateValue() {
    pthread_mutex_lock(&s_voiceRegStateMutex[m_slot_id]);
    getMclStatusManager()->setServiceStateValue(RFX_STATUS_KEY_SERVICE_STATE,
            RfxNwServiceState(
                    voice_reg_state_cache[m_slot_id]->registration_state,
                    data_reg_state_cache[m_slot_id]->registration_state,
                    voice_reg_state_cache[m_slot_id]->radio_technology,
                    data_reg_state_cache[m_slot_id]->radio_technology,
                    ril_wfc_reg_status[m_slot_id]));
    pthread_mutex_unlock(&s_voiceRegStateMutex[m_slot_id]);
}

void RmcNetworkRealTimeRequestHandler::resetCaCache(RIL_CA_CACHE* cache) {
    pthread_mutex_lock(&s_caMutex[m_slot_id]);
    cache->ca_info = 1;
    cache->pcell_bw = 0;
    cache->scell_bw1 = 0;
    cache->scell_bw2 = 0;
    cache->scell_bw3 = 0;
    cache->scell_bw4 = 0;
    pthread_mutex_unlock(&s_caMutex[m_slot_id]);
}

// get Ca Band info, Unit is 0.1MHz
void RmcNetworkRealTimeRequestHandler::updateCaBandInfo() {
    int err;
    sp<RfxAtResponse> p_response;
    int resp = 0;
    RfxAtLine* line;

    resetCaCache(ca_cache[m_slot_id]);

    int *ca_info = &(ca_cache[m_slot_id]->ca_info);
    int *pcell_bw = &(ca_cache[m_slot_id]->pcell_bw);
    int *scell_bw1 = &(ca_cache[m_slot_id]->scell_bw1);
    int *scell_bw2 = &(ca_cache[m_slot_id]->scell_bw2);
    int *scell_bw3 = &(ca_cache[m_slot_id]->scell_bw3);
    int *scell_bw4 = &(ca_cache[m_slot_id]->scell_bw4);

    pthread_mutex_lock(&s_caMutex[m_slot_id]);

    // ca_info- CA_CONFIGURED(0)/CA_NOT_CONFIGURATED(1)/CA_ACTIVATED(2)/CA_DEACTIVATED(3)
    // +ECAINFO: <ca_info>,<pcell_bw>,<scell_bw1>,<scell_bw2>,<scell_bw3>,<scell_bw4>
    p_response = atSendCommandSingleline("AT+ECAINFO?", "+ECAINFO:");

    // check error
    err = p_response->getError();
    if (err != 0 ||
            p_response == NULL ||
            p_response->getSuccess() == 0 ||
            p_response->getIntermediates() == NULL) goto error;

    // handle intermediate
    line = p_response->getIntermediates();

    // go to start position
    line->atTokStart(&err);
    if (err < 0) goto error;

    *ca_info = line->atTokNextint(&err);
    if (err < 0) goto error;

    *pcell_bw = line->atTokNextint(&err);
    if (err < 0) goto error;

    *scell_bw1 = line->atTokNextint(&err);
    if (err < 0) goto error;

    *scell_bw2 = line->atTokNextint(&err);
    if (err < 0) goto error;

    *scell_bw3 = line->atTokNextint(&err);
    if (err < 0) goto error;

    *scell_bw4 = line->atTokNextint(&err);
    if (err < 0) goto error;

    // get LTE band when pcell_bw is 0 and it's LTE in service now.
    if (*pcell_bw == 0) {
        if (isInService(urc_data_reg_state_cache[m_slot_id]->registration_state) ||
            urc_data_reg_state_cache[m_slot_id]->radio_technology == 14) {  // 14:LTE, 19:LTE_CA
            // When we query LTE bw info, the only way is to turn on the URC.
            p_response = atSendCommandSingleline("AT+ELTEBWINFO=1", "+ELTEBWINFO:");
            // check error
            err = p_response->getError();
            if (err != 0 ||
                    p_response == NULL ||
                    p_response->getSuccess() == 0 ||
                    p_response->getIntermediates() == NULL) goto error;

            // handle intermediate
            line = p_response->getIntermediates();

            // go to start position
            line->atTokStart(&err);
            if (err < 0) goto error;

            *pcell_bw = line->atTokNextint(&err);
            if (err < 0) goto error;

            // we turn off the URC right after we got lte bw info
            atSendCommand("AT+ELTEBWINFO=0");
        }
    }

    pthread_mutex_unlock(&s_caMutex[m_slot_id]);
    return;
error:
    pthread_mutex_unlock(&s_caMutex[m_slot_id]);
    resetCaCache(ca_cache[m_slot_id]);
    return;
}

void RmcNetworkRealTimeRequestHandler::updateDcStatus(int cell, int max) {
    if ((cell & 0x80) == 0x80) m_dc_support[m_slot_id] = true;
    else if (max >= 12 && max <= 17) m_dc_support[m_slot_id] = true;
    else m_dc_support[m_slot_id] = false;
}

int RmcNetworkRealTimeRequestHandler::isCdmaVoiceInFemtocell() {
    int isFemtocell = 0;
    if (RfxNwServiceState::isInService(voice_reg_state_cache[m_slot_id]->registration_state)
            && RfxNwServiceState::isCdmaGroup(voice_reg_state_cache[m_slot_id]->radio_technology)) {
        if (femto_cell_cache[m_slot_id]->is_1x_femtocell == 1) {
            isFemtocell = 1;
        } else {
            isFemtocell = isInFemtocell(voice_reg_state_cache[m_slot_id]->sector_id,
                    voice_reg_state_cache[m_slot_id]->subnet_mask,
                    voice_reg_state_cache[m_slot_id]->network_id,
                    voice_reg_state_cache[m_slot_id]->radio_technology);
            logD(LOG_TAG, "isCdmaVoiceInFemtocell(): CDMA CS in femtocell ? %d", isFemtocell);
        }
    }
    return isFemtocell;
}

int RmcNetworkRealTimeRequestHandler::isCdmaDataInFemtocell() {
    int isFemtocell = 0;
    if (RfxNwServiceState::isInService(data_reg_state_cache[m_slot_id]->registration_state)
            && RfxNwServiceState::isCdmaGroup(data_reg_state_cache[m_slot_id]->radio_technology)) {
        if (femto_cell_cache[m_slot_id]->is_1x_femtocell == 1
                || femto_cell_cache[m_slot_id]->is_evdo_femtocell == 1) {
            isFemtocell = 1;
        } else {
            isFemtocell = isInFemtocell(voice_reg_state_cache[m_slot_id]->sector_id,
                    voice_reg_state_cache[m_slot_id]->subnet_mask,
                    voice_reg_state_cache[m_slot_id]->network_id,
                    data_reg_state_cache[m_slot_id]->radio_technology);
            logD(LOG_TAG, "isCdmaDataInFemtocell(): CDMA PS in femtocell ? %d", isFemtocell);
        }
    }
    return isFemtocell;
}

int RmcNetworkRealTimeRequestHandler::isInFemtocell(char *sector_id, char *subnet_mask,
        int network_id, int radio_technology) {
    char oxtet7;
    int subnet_mask_value = 0;
    if (strlen(subnet_mask) > 0) {
        subnet_mask_value = strtol(subnet_mask, NULL, 16);
    }
    logD(LOG_TAG, "isInFemtocell, sector_id:%s, subnet_mask:%x, network_id:%d, radio_tech:%d",
            sector_id, subnet_mask_value, network_id, radio_technology);
    switch (radio_technology) {
        case 6:  // 1xRTT
            // 1X Femtocell Visual Indicator VZ_REQ_1XEV1XRTT_8757:
            // When a 1xRTT-capable device is attached to a Verizon Wireless network
            // with a NID whose value is within the range 250 ~ 254, the device
            // shall display the 1X femtocell visual indicator.
            if (network_id >= 250 && network_id <= 254) {
                return 1;
            }
            break;
        case 8:   // EVDO
        case 13:  // eHRPD
            if (subnet_mask_value > 48) {
                return 0;
            }
            if (strlen(sector_id) < 13) {
                return 0;
            }
            // EVDO Femtocell Visual Indicator VZ_REQ_1XEV1XRTT_8758:
            // When a 1xEVDO-capable device is attached to a Verizon Wireless
            // network whose Subnet ID bits 48 and 49 (the two most significant bits
            // in octet number 7) are both set to 1, the device shall display the
            // EVDO femtocell visual indicator.
            oxtet7 = tolower(sector_id[strlen(sector_id) - 13]);
            if (oxtet7 == '3' || oxtet7 == '7' || oxtet7 == 'b' || oxtet7 == 'f') {
                logD(LOG_TAG, "isInFemtocell, oxtet7:%c", oxtet7);
                return 1;
            }
            break;
        default:
            logD(LOG_TAG, "[isInFemtocell] Unknown radioTech");
            break;
    }
    return 0;
}

