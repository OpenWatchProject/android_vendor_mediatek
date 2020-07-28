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

#include "RmcNetworkHandler.h"
#include <math.h>                          /* log10 */
#include "RfxViaUtils.h"
#include "ViaBaseHandler.h"
#include <libmtkrilutils.h>
#include "utils/Timers.h"

static const char PROPERTY_NW_LTE_SIGNAL[MAX_SIM_COUNT][MAX_PROP_CHARS] = {
    "vendor.ril.nw.signalstrength.lte.1",
    "vendor.ril.nw.signalstrength.lte.2",
    "vendor.ril.nw.signalstrength.lte.3",
    "vendor.ril.nw.signalstrength.lte.4",
};

char const *sOp12Plmn[] = {
    "311278", "311483", "310004", "311283", "311488",
    "310890", "311272", "311288", "311277", "311482",
    "311282", "311487", "310590", "311287", "311271",
    "311276", "311481", "311281", "311486", "310013",
    "311286", "311270", "311275", "311480", "311280",
    "311485", "310012", "311285", "311110", "311274",
    "311390", "311279", "311484", "310010", "311284",
    "311489", "310910", "311273", "311289"
};

static const char PROPERTY_RIL_TEST_SIM[4][35] = {
    "vendor.gsm.sim.ril.testsim",
    "vendor.gsm.sim.ril.testsim.2",
    "vendor.gsm.sim.ril.testsim.3",
    "vendor.gsm.sim.ril.testsim.4",
};

const static SpnTable s_mtk_spn_table[] = {
    #include "mtk_spn_table.h"
};

const static SpnTable s_mtk_ts25_table[] = {
    #include "mtk_ts25_table.h"
};

const static SpnTable s_mtk_plmn_table[] = {
    #include "mtk_plmn_table.h"
};

int RmcNetworkHandler::ECELLext3ext4Support = 1;
pthread_mutex_t RmcNetworkHandler::ril_handler_init_mutex[MAX_SIM_COUNT];
bool RmcNetworkHandler::nwHandlerInit[MAX_SIM_COUNT] = {false};
pthread_mutex_t RmcNetworkHandler::ril_nw_femtoCell_mutex;
RIL_FEMTO_CELL_CACHE* RmcNetworkHandler::femto_cell_cache[MAX_SIM_COUNT];
pthread_mutex_t RmcNetworkHandler::s_signalStrengthMutex[MAX_SIM_COUNT];
RIL_SIGNAL_STRENGTH_CACHE* RmcNetworkHandler::signal_strength_cache[MAX_SIM_COUNT];
pthread_mutex_t RmcNetworkHandler::s_voiceRegStateMutex[MAX_SIM_COUNT];
RIL_VOICE_REG_STATE_CACHE* RmcNetworkHandler::voice_reg_state_cache[MAX_SIM_COUNT];
RIL_DATA_REG_STATE_CACHE* RmcNetworkHandler::data_reg_state_cache[MAX_SIM_COUNT];
pthread_mutex_t RmcNetworkHandler::s_urc_voiceRegStateMutex[MAX_SIM_COUNT];
RIL_VOICE_REG_STATE_CACHE* RmcNetworkHandler::urc_voice_reg_state_cache[MAX_SIM_COUNT];
pthread_mutex_t RmcNetworkHandler::s_urc_dataRegStateMutex[MAX_SIM_COUNT];
RIL_DATA_REG_STATE_CACHE* RmcNetworkHandler::urc_data_reg_state_cache[MAX_SIM_COUNT];
pthread_mutex_t RmcNetworkHandler::s_caMutex[MAX_SIM_COUNT];
RIL_CA_CACHE* RmcNetworkHandler::ca_cache[MAX_SIM_COUNT];
pthread_mutex_t RmcNetworkHandler::s_wfcRegStatusMutex[MAX_SIM_COUNT];
RIL_OPERATOR_INFO_CACHE* RmcNetworkHandler::op_info_cache[MAX_SIM_COUNT];
int RmcNetworkHandler::ril_wfc_reg_status[MAX_SIM_COUNT];
bool RmcNetworkHandler::mIsNeedNotifyState[MAX_SIM_COUNT];
RIL_EonsNetworkFeatureInfo RmcNetworkHandler::eons_info[MAX_SIM_COUNT];
int RmcNetworkHandler::mPlmnListOngoing = 0;
int RmcNetworkHandler::mPlmnListAbort = 0;
String8 RmcNetworkHandler::mCurrentLteSignal[MAX_SIM_COUNT];
bool RmcNetworkHandler::physicalConfigSwitch[MAX_SIM_COUNT];
int RmcNetworkHandler::physicalConfig_cache[MAX_SIM_COUNT][4];

pthread_mutex_t RmcNetworkHandler::ril_nw_nitzName_mutex[MAX_SIM_COUNT];
char RmcNetworkHandler::m_ril_nw_nitz_oper_code[MAX_SIM_COUNT][MAX_OPER_NAME_LENGTH];
char RmcNetworkHandler::m_ril_nw_nitz_oper_lname[MAX_SIM_COUNT][MAX_OPER_NAME_LENGTH];
char RmcNetworkHandler::m_ril_nw_nitz_oper_sname[MAX_SIM_COUNT][MAX_OPER_NAME_LENGTH];

char RmcNetworkHandler::last_valid_plmn[MAX_SIM_COUNT][8];
nsecs_t RmcNetworkHandler::last_valid_plmn_time[MAX_SIM_COUNT];

SPNTABLE RmcNetworkHandler::cache_spn_table[MAX_SIM_COUNT];

bool RmcNetworkHandler::m_dc_support[MAX_SIM_COUNT];
bool RmcNetworkHandler::ims_ecc_state[MAX_SIM_COUNT];
bool RmcNetworkHandler::ca_filter_switch[MAX_SIM_COUNT];

int RmcNetworkHandler::nr_support;
int RmcNetworkHandler::data_context_ids[MAX_SIM_COUNT][16];

MD_EREG* RmcNetworkHandler::mdEreg[MAX_SIM_COUNT];
pthread_mutex_t RmcNetworkHandler::mdEregMutex[MAX_SIM_COUNT];
MD_EGREG* RmcNetworkHandler::mdEgreg[MAX_SIM_COUNT];
pthread_mutex_t RmcNetworkHandler::mdEgregMutex[MAX_SIM_COUNT];
MD_EOPS* RmcNetworkHandler::mdEops[MAX_SIM_COUNT];
pthread_mutex_t RmcNetworkHandler::mdEopsMutex[MAX_SIM_COUNT];

MD_ECELL* RmcNetworkHandler::mdEcell[MAX_SIM_COUNT];
pthread_mutex_t RmcNetworkHandler::mdEcellMutex[MAX_SIM_COUNT];

int RmcNetworkHandler::mLastSignalBand[MAX_SIM_COUNT];

RmcNetworkHandler::RmcNetworkHandler(int slot_id, int channel_id) :
        RfxBaseHandler(slot_id, channel_id) {

    pthread_mutex_lock(&ril_handler_init_mutex[m_slot_id]);
    logV(LOG_TAG, "RmcNetworkHandler[%d] start", m_slot_id);
    if (nwHandlerInit[m_slot_id] == false) {
        nwHandlerInit[m_slot_id] = true;
        // request cache
        voice_reg_state_cache[m_slot_id] = (RIL_VOICE_REG_STATE_CACHE*)calloc(1, sizeof(RIL_VOICE_REG_STATE_CACHE));
        data_reg_state_cache[m_slot_id] = (RIL_DATA_REG_STATE_CACHE*)calloc(1, sizeof(RIL_DATA_REG_STATE_CACHE));
        signal_strength_cache[m_slot_id] = (RIL_SIGNAL_STRENGTH_CACHE*)calloc(1, sizeof(RIL_SIGNAL_STRENGTH_CACHE));
        // urc cache
        urc_voice_reg_state_cache[m_slot_id] = (RIL_VOICE_REG_STATE_CACHE*)calloc(1, sizeof(RIL_VOICE_REG_STATE_CACHE));
        urc_data_reg_state_cache[m_slot_id] = (RIL_DATA_REG_STATE_CACHE*)calloc(1, sizeof(RIL_DATA_REG_STATE_CACHE));
        // nw info cache
        op_info_cache[m_slot_id] = (RIL_OPERATOR_INFO_CACHE*)calloc(1, sizeof(RIL_OPERATOR_INFO_CACHE));
        ca_cache[m_slot_id] = (RIL_CA_CACHE*)calloc(1, sizeof(RIL_CA_CACHE));
        // femto cell cache
        femto_cell_cache[m_slot_id] = (RIL_FEMTO_CELL_CACHE*)calloc(1, sizeof(RIL_FEMTO_CELL_CACHE));

        pthread_mutex_init(&s_signalStrengthMutex[m_slot_id], NULL);
        pthread_mutex_init(&s_voiceRegStateMutex[m_slot_id], NULL);
        pthread_mutex_init(&s_urc_voiceRegStateMutex[m_slot_id], NULL);
        pthread_mutex_init(&s_urc_dataRegStateMutex[m_slot_id], NULL);
        pthread_mutex_init(&s_wfcRegStatusMutex[m_slot_id], NULL);
        pthread_mutex_init(&s_caMutex[m_slot_id], NULL);
        eons_info[m_slot_id] = {EONS_INFO_NOT_RECEIVED, 0};
        mCurrentLteSignal[m_slot_id] = "";
        mIsNeedNotifyState[m_slot_id] = true;

        memset(last_valid_plmn[m_slot_id], 0, 8);
        last_valid_plmn_time[m_slot_id] = -1;

        m_dc_support[m_slot_id] = false;

        ca_filter_switch[m_slot_id] = false;

        physicalConfigSwitch[m_slot_id] = true;
        for (int i = 0; i < 4; i++)
        {
            physicalConfig_cache[m_slot_id][i] = 0;
        }

        for (int i = 0; i < 16; i++) data_context_ids[m_slot_id][i] = -1;

        ims_ecc_state[m_slot_id] = false;

        // AT cache
        mdEreg[m_slot_id] = (MD_EREG*) calloc(1, sizeof(MD_EREG));
        memset(mdEreg[m_slot_id], 0, sizeof(MD_EREG));
        pthread_mutex_init(&mdEregMutex[m_slot_id], NULL);
        mdEgreg[m_slot_id] = (MD_EGREG*) calloc(1, sizeof(MD_EGREG));
        memset(mdEgreg[m_slot_id], 0, sizeof(MD_EGREG));
        pthread_mutex_init(&mdEgregMutex[m_slot_id], NULL);
        mdEops[m_slot_id] = (MD_EOPS*) calloc(1, sizeof(MD_EOPS));
        memset(mdEops[m_slot_id], 0, sizeof(MD_EOPS));
        pthread_mutex_init(&mdEopsMutex[m_slot_id], NULL);

        mdEcell[m_slot_id] = (MD_ECELL*) calloc(1, sizeof(MD_ECELL));
        memset(mdEcell[m_slot_id], 0, sizeof(MD_ECELL));
        pthread_mutex_init(&mdEcellMutex[m_slot_id], NULL);
        // reset signal band
        mLastSignalBand[m_slot_id] = 0;
        setMSimProperty(m_slot_id, (char *)PROPERTY_MTK_SIGNAL_BAND, (char*)"0");
    }
    nr_support = -1;
    pthread_mutex_unlock(&ril_handler_init_mutex[m_slot_id]);
}

/* Android framework expect spec 27.007  AT+CSQ <rssi> 0~31 format when handling 3G signal strength.
   So we convert 3G signal to <rssi> in RILD */
int RmcNetworkHandler::convert3GRssiValue(int rscp_in_dbm)
{
    int rssi = 0;
    int INVALID = 0x7FFFFFFF;

    if (rscp_in_dbm == INVALID) {
        return rssi;
    }

    // logD(LOG_TAG, "convert3GRssiValue rscp_in_dbm = %d", rscp_in_dbm);

    rssi = (rscp_in_dbm + 113) / 2;

    if (rssi > 31) {
        rssi = 31;
    } else if (rssi < 0) {
        rssi = 0;
    }

    return rssi;
}

int RmcNetworkHandler::getSignalStrength(RfxAtLine *line)
{
    int err;
    MD_SIGNAL_STRENGTH ecsq;

    //Use int max, as -1 is a valid value in signal strength
    int INVALID = 0x7FFFFFFF;

    // 93 modem
    // +ECSQ: <sig1>,<sig2>,<rssi_in_qdbm>,<rscp_in_qdbm>,<ecn0_in_qdbm>,<rsrq_in_qdbm>,<rsrp_in_qdbm>,<Act>,<sig3>,<serv_band>
    // +ECSQ: <sig1>,<sig2>,<rssi_in_qdbm>,<rscp_in_qdbm>,<ecn0_in_qdbm>,<rsrq_in_qdbm>,<rsrp_in_qdbm>,<256>,<sig3>,<serv_band>
    // +ECSQ: <sig1>,<sig2>,<rssi_in_qdbm>,<rscp_in_qdbm>,<ecn0_in_qdbm>,<rsrq_in_qdbm>,<rsrp_in_qdbm>,<Act_EvDo>,<sig3>,<serv_band>
    // +ECSQ: <sig1>,<sig2>,<rssi_in_qdbm>,<rscp_in_qdbm>,<ecn0_in_qdbm>,<ssRsrq_in_qdb>,<ssRsrp_in_qdbm>
    //                  ,<NR>,<ssSINR_in_qdb>,<serv_band>[,<csiRsrq_in_qdb>,<csiRsrp_in_qdbm>,<csiSinr_in_qdb>]

    float sinr_db;
    float temp;
    int cdma_dbm = CELLINFO_INVALID;
    int cdma_ecio = CELLINFO_INVALID;
    int evdo_dbm = CELLINFO_INVALID;
    int evdo_ecio = CELLINFO_INVALID;

    ViaBaseHandler *mViaHandler = RfxViaUtils::getViaHandler();

    // go to start position
    line->atTokStart(&err);
    if (err < 0) return -1;

    ecsq.sig1 = line->atTokNextint(&err);
    if (err < 0) return -1;

    ecsq.sig2 = line->atTokNextint(&err);
    if (err < 0) return -1;

    ecsq.rssi_in_qdbm = line->atTokNextint(&err);
    if (err < 0) return -1;

    ecsq.rscp_in_qdbm = line->atTokNextint(&err);
    if (err < 0) return -1;

    ecsq.ecn0_in_qdbm = line->atTokNextint(&err);
    if (err < 0) return -1;

    // for LTE
    ecsq.rsrq_in_qdbm = line->atTokNextint(&err);
    if (err < 0) return -1;

    ecsq.rsrp_in_qdbm = line->atTokNextint(&err);
    if (err < 0) return -1;

    ecsq.act = line->atTokNextint(&err);
    if (err < 0) return -1;

    ecsq.sig3 = line->atTokNextint(&err);
    if (err < 0) return -1;

    ecsq.serv_band = line->atTokNextint(&err);
    if (err < 0) return -1;

    if (ecsq.act == 0x8000) {
        // NR
        ecsq.csiRsrq_in_qdb = line->atTokNextint(&err);
        if (err < 0) return -1;
        ecsq.csiRsrp_in_qdbm = line->atTokNextint(&err);
        if (err < 0) return -1;
        ecsq.csiSinr_in_qdb = line->atTokNextint(&err);
        if (err < 0) return -1;
    }

    // CS Reg GSM, or No Reg Cdma, has Reg GSM but not 4G, clear legacy CDMA signal
    MD_EREG *mMdEreg = mdEreg[m_slot_id];
    MD_EGREG *mMdEgreg = mdEgreg[m_slot_id];
    int voiceRegState = convertRegState(mMdEreg->stat, true);
    int voiceRadioTech = convertCSNetworkType(mMdEreg->eAct);
    int dataRegState = convertRegState(mMdEgreg->stat, false);
    int dataRadioTech = convertPSNetworkType(mMdEgreg->eAct);

    bool isCsRegGsm = isInService(voiceRegState) && RfxNwServiceState::isGsmGroup(voiceRadioTech);
    bool isPsRegGsm = isInService(dataRegState) && RfxNwServiceState::isGsmGroup(dataRadioTech);
    bool isRegCdma = (isInService(voiceRegState) && RfxNwServiceState::isCdmaGroup(voiceRadioTech))
            || (isInService(dataRegState) && RfxNwServiceState::isCdmaGroup(dataRadioTech));
    bool isReg4G = (isInService(voiceRegState)
            && (voiceRadioTech == RADIO_TECH_LTE || voiceRadioTech == RADIO_TECH_LTE_CA))
            || (isInService(dataRegState)
            && (dataRadioTech == RADIO_TECH_LTE || dataRadioTech == RADIO_TECH_LTE_CA));

    if (isCsRegGsm || (isPsRegGsm && !isRegCdma && !isReg4G)) {
        resetSignalStrengthCache(signal_strength_cache[m_slot_id], CACHE_GROUP_C2K);
        // Skip For 1xRTT & EVDO
        if (ecsq.act == 0x0100 || ecsq.act == 0x0200 || ecsq.act == 0x0400) {
            return -1;
        }
    } else if (isRegCdma && !isReg4G) {
        resetSignalStrengthCache(signal_strength_cache[m_slot_id], CACHE_GROUP_GSM);
        // Skip For LTE
        if (ecsq.act == 0x1000 || ecsq.act == 0x2000) {
            return -1;
        }
    }

    // write band to property if changes
    if (mLastSignalBand[m_slot_id] != ecsq.serv_band) {
        char* serv_band_str = NULL;
        asprintf(&serv_band_str, "%d", ecsq.serv_band);
        if (serv_band_str != NULL) {
            setMSimProperty(m_slot_id, (char *)PROPERTY_MTK_SIGNAL_BAND, serv_band_str);
            logV(LOG_TAG, "write band %s", serv_band_str);
            free(serv_band_str);
        }
        mLastSignalBand[m_slot_id] = ecsq.serv_band;
    }

    // validate information for each AcT
    if ((ecsq.act == 0x0001) || (ecsq.act == 0x0002)) {  // for GSM
        if (ecsq.sig1 < 0  || ecsq.sig1 > 63) {
            logE(LOG_TAG, "Recevice an invalid <rssi> value from modem");
            return -1;
        }
        // It's normal that <ber> is 99 with GSM sometimes.
        if ((ecsq.sig2 < 0 || ecsq.sig2 > 7) && ecsq.sig2 != 99) {
            logE(LOG_TAG, "Recevice an invalid <ber> value from modem");
            return -1;
        }
        resetSignalStrengthCache(signal_strength_cache[m_slot_id], CACHE_GROUP_GSM);
        signal_strength_cache[m_slot_id]->gsm_signal_strength = ((ecsq.rssi_in_qdbm >> 2)+113) >> 1;
        signal_strength_cache[m_slot_id]->gsm_bit_error_rate = ecsq.sig2;
        // range check
        if (signal_strength_cache[m_slot_id]->gsm_signal_strength < 0)
            signal_strength_cache[m_slot_id]->gsm_signal_strength = 0;
        if (signal_strength_cache[m_slot_id]->gsm_signal_strength > 31)
            signal_strength_cache[m_slot_id]->gsm_signal_strength = 31;
    } else if ((ecsq.act >= 0x0004) && (ecsq.act <= 0x00e0)) { // for UMTS
        if (ecsq.sig1 < 0  || ecsq.sig1 > 96) {
            logE(LOG_TAG, "Recevice an invalid <rscp> value from modem");
            return -1;
        }
        if (ecsq.sig2 < 0 || ecsq.sig2 > 49) {
            logE(LOG_TAG, "Recevice an invalid <ecn0> value from modem");
            return -1;
        }
        resetSignalStrengthCache(signal_strength_cache[m_slot_id], CACHE_GROUP_GSM);

        int rscp_in_dbm = 0 - (ecsq.rscp_in_qdbm >> 2);
        if (rscp_in_dbm > 120) {
            rscp_in_dbm = 120;
        } else if (rscp_in_dbm < 25) {
            rscp_in_dbm = 25;
        }
        int rssi = convert3GRssiValue(0 - rscp_in_dbm);
        if (!isTdd3G()) {
            signal_strength_cache[m_slot_id]->wcdma_signal_strength = rssi;
            signal_strength_cache[m_slot_id]->wcdma_scdma_rscp= ecsq.sig1;
            signal_strength_cache[m_slot_id]->wcdma_ecno = ecsq.sig2;
        } else {
            signal_strength_cache[m_slot_id]->tdscdma_signal_strength = rssi;
            signal_strength_cache[m_slot_id]->tdscdma_rscp= ecsq.sig1;
        }
    } else if (ecsq.act == 0x0100) {  // for 1xRTT
        resetSignalStrengthCache(signal_strength_cache[m_slot_id], CACHE_GROUP_1XRTT);
        if (ecsq.sig1 < 0  || ecsq.sig1 > 31) {
            logE(LOG_TAG, "Recevice an invalid <rssi> value from modem");
            return -1;
        }
        if (ecsq.sig2 < -128 || ecsq.sig2 > 0) {
            // logE(LOG_TAG, "Recevice an invalid <ec/io> value from modem");
            return -1;
        }

        if (mViaHandler != NULL) {
            cdma_dbm = mViaHandler->convertCdmaEvdoSig(ecsq.sig1, SIGNAL_CDMA_DBM);
            cdma_ecio = mViaHandler->convertCdmaEvdoSig(ecsq.sig2, SIGNAL_CDMA_ECIO);
        }
        signal_strength_cache[m_slot_id]->cdma_dbm = cdma_dbm;
        // logD(LOG_TAG, "signal_strength_cache.cdma_dbm = %d",
        //         signal_strength_cache[m_slot_id]->cdma_dbm);
        // -5 => -10 / 2 (-10 follow AOSP ril define, 2 is to normalize MD3 ecio data)
        signal_strength_cache[m_slot_id]->cdma_ecio = cdma_ecio;
        // logD(LOG_TAG, "signal_strength_cache.cdma_ecio = %d",
        //         signal_strength_cache[m_slot_id]->cdma_ecio);
    } else if ((ecsq.act == 0x0200) || (ecsq.act == 0x0400)) {  // for EVDO
        resetSignalStrengthCache(signal_strength_cache[m_slot_id], CACHE_GROUP_EVDO);
        if (ecsq.sig1 < 0  || ecsq.sig1 > 31) {
            logE(LOG_TAG, "Recevice an invalid <rssi> value from modem");
            return -1;
        }
        if (ecsq.sig2 < -512 || ecsq.sig2 > 0) {
            // logE(LOG_TAG, "Recevice an invalid <ec/io> value from modem");
            return -1;
        }
        if (ecsq.sig1 == 0 && ecsq.sig2 == -512) {
            // logE(LOG_TAG, "Recevice an invalid <ec/io> value from modem");
            return -1;
        }

        if (mViaHandler != NULL) {
            evdo_dbm = mViaHandler->convertCdmaEvdoSig(ecsq.sig1, SIGNAL_EVDO_DBM);
            evdo_ecio = mViaHandler->convertCdmaEvdoSig(ecsq.sig2, SIGNAL_EVDO_ECIO);
        }
        signal_strength_cache[m_slot_id]->evdo_dbm = evdo_dbm;
        // logD(LOG_TAG, "signal_strength_cache.evdo_dbm = %d",
        //         signal_strength_cache[m_slot_id]->evdo_dbm);
        signal_strength_cache[m_slot_id]->evdo_ecio = evdo_ecio;
        // logD(LOG_TAG, "signal_strength_cache.evdo_ecio = %d",
        //         signal_strength_cache[m_slot_id]->evdo_ecio);
        temp = (double)ecsq.sig3 / 512;
        sinr_db = 100 * log10(temp);
        // logD(LOG_TAG, "sinr:%d, sinr_dB:%f", ecsq.sig3, sinr_db);
        if (sinr_db >= 100) {
            signal_strength_cache[m_slot_id]->evdo_snr = 8;
        } else if (sinr_db >= 70) {
            signal_strength_cache[m_slot_id]->evdo_snr = 7;
        } else if (sinr_db >= 50) {
            signal_strength_cache[m_slot_id]->evdo_snr = 6;
        } else if (sinr_db >= 30) {
            signal_strength_cache[m_slot_id]->evdo_snr = 5;
        } else if (sinr_db >= -20) {
            signal_strength_cache[m_slot_id]->evdo_snr = 4;
        } else if (sinr_db >= -45) {
            signal_strength_cache[m_slot_id]->evdo_snr = 3;
        } else if (sinr_db >= -90) {
            signal_strength_cache[m_slot_id]->evdo_snr = 2;
        } else if (sinr_db > -120) {
            signal_strength_cache[m_slot_id]->evdo_snr = 1;
        } else {
            signal_strength_cache[m_slot_id]->evdo_snr = 0;
        }
    } else if ((ecsq.act == 0x1000) || (ecsq.act == 0x2000)) {  // for LTE
        if (ecsq.sig1 < 0  || ecsq.sig1 > 34) {
            return -1;
        }
        if (ecsq.sig2 < 0 || ecsq.sig2 > 97) {
            logE(LOG_TAG, "Recevice an invalid <rsrp> value from modem");
            return -1;
        }
        resetSignalStrengthCache(signal_strength_cache[m_slot_id], CACHE_GROUP_GSM);
        signal_strength_cache[m_slot_id]->lte_signal_strength = 99; //(0~63, 99) Unknown for now
        int rsrp_in_dbm = 0 - (ecsq.rsrp_in_qdbm >> 2);
        if (rsrp_in_dbm > 140) rsrp_in_dbm = 140;
        else if (rsrp_in_dbm < 44) rsrp_in_dbm = 44;
        signal_strength_cache[m_slot_id]->lte_rsrp = rsrp_in_dbm;
        int rsrq_in_dbm = 0 - (ecsq.rsrq_in_qdbm >> 2);
        if (rsrq_in_dbm > 20) rsrq_in_dbm = 20;
        else if (rsrq_in_dbm < 3) rsrq_in_dbm = 3;
        signal_strength_cache[m_slot_id]->lte_rsrq = rsrq_in_dbm;

        // from (snr in qdbm) to (snr in 0.1dbm)
        int rssnr = ecsq.sig3;
        if (rssnr != 0x7FFF) {
            rssnr = (rssnr * 10) >> 2;
            if (rssnr > 300) {
                rssnr = 300;
            } else if (rssnr < -200) {
                rssnr = -200;
            }
        }
        signal_strength_cache[m_slot_id]->lte_rssnr = rssnr; // unit: 0.1 dB
        signal_strength_cache[m_slot_id]->lte_cqi = 0;
        signal_strength_cache[m_slot_id]->lte_timing_advance = 0;
        updateSignalStrengthProperty();
    } else if (ecsq.act == 0x8000) {  // NR
        // TODO: filter invalid ecsq, may not need to do filter
        resetSignalStrengthCache(signal_strength_cache[m_slot_id], CACHE_GROUP_NR);
        signal_strength_cache[m_slot_id]->ssRsrp = 0 - (ecsq.rsrp_in_qdbm >> 2);
        signal_strength_cache[m_slot_id]->ssRsrq = 0 - (ecsq.rsrq_in_qdbm >> 2);
        signal_strength_cache[m_slot_id]->ssSinr = (ecsq.sig3 >> 2);  // sig3 = ssSINR_in_qdb
        signal_strength_cache[m_slot_id]->csiRsrp = 0 - (ecsq.csiRsrp_in_qdbm >> 2);
        signal_strength_cache[m_slot_id]->csiRsrq = 0 - (ecsq.csiRsrq_in_qdb >> 2);
        signal_strength_cache[m_slot_id]->csiSinr = (ecsq.csiSinr_in_qdb >> 2);
        // range check
        if (signal_strength_cache[m_slot_id]->ssRsrp < 44)
            signal_strength_cache[m_slot_id]->ssRsrp = 44;
        if (signal_strength_cache[m_slot_id]->ssRsrp > 140)
            signal_strength_cache[m_slot_id]->ssRsrp = 140;
        if (signal_strength_cache[m_slot_id]->ssRsrq < 3)
            signal_strength_cache[m_slot_id]->ssRsrq = 3;
        if (signal_strength_cache[m_slot_id]->ssRsrq > 20)
            signal_strength_cache[m_slot_id]->ssRsrq = 20;
        if (signal_strength_cache[m_slot_id]->ssSinr < -23)
            signal_strength_cache[m_slot_id]->ssSinr = -23;
        if (signal_strength_cache[m_slot_id]->ssSinr > 40)
            signal_strength_cache[m_slot_id]->ssSinr = 40;
        if (signal_strength_cache[m_slot_id]->csiRsrp < 44)
            signal_strength_cache[m_slot_id]->csiRsrp = 44;
        if (signal_strength_cache[m_slot_id]->csiRsrp > 140)
            signal_strength_cache[m_slot_id]->csiRsrp = 140;
        if (signal_strength_cache[m_slot_id]->csiRsrq < 3)
            signal_strength_cache[m_slot_id]->csiRsrq = 3;
        if (signal_strength_cache[m_slot_id]->csiRsrq > 20)
            signal_strength_cache[m_slot_id]->csiRsrq = 20;
        if (signal_strength_cache[m_slot_id]->csiSinr < -23)
            signal_strength_cache[m_slot_id]->csiSinr = -23;
        if (signal_strength_cache[m_slot_id]->csiSinr > 40)
            signal_strength_cache[m_slot_id]->csiSinr = 40;
    } else {
        // logE(LOG_TAG, "Recevice an invalid <eAcT> value from modem");
        return -1;
    }
    return 0;
}

unsigned int RmcNetworkHandler::convertRegState(unsigned int uiRegState, bool isVoiceState)
{
    unsigned int uiRet = 0;

    switch (uiRegState)
    {
        case 6:         // Registered for "SMS only", home network
            uiRet = 1;  // Registered
            break;
        case 7:         // Registered for "SMS only", roaming
            uiRet = 5;  // roaming
            break;
        case 8:         // attached for emergency bearer service only
            uiRet = 0;  // not registered
            break;
        case 9:         // registered for "CSFB not prefereed", home network
            uiRet = 1;  // Registered
            break;
        case 10:        // registered for "CSFB not prefereed", roaming
            uiRet = 5;  // roaming
            break;
        case 101:       // no NW, but need to find NW
            if (isVoiceState) {
                uiRet = 1;  // 1xRTT normal service, Mapping '+VSER:0'
            } else {
                uiRet = 0;  // not registered
            }
            break;
        case 102:       // not registered, but MT find 1X NW existence
            if (isVoiceState) {
                uiRet = 2; // 1x is searching
            } else {
                uiRet = 0; // not registered
            }
            break;
        case 103:       // not registered, but MT find Do NW existence
            uiRet = 0;  // not registered
            break;
        case 104:       // not registered, but MT find Do&1X NW existence
            uiRet = 0;  // not registered
            break;
        default:
            uiRet = uiRegState;
            break;
    }

    return uiRet;
}

void RmcNetworkHandler::resetSignalStrengthCache(RIL_SIGNAL_STRENGTH_CACHE *sigCache, RIL_CACHE_GROUP source) {
    if (source == CACHE_GROUP_GSM) {
        (*sigCache).gsm_signal_strength = 99;
        (*sigCache).gsm_bit_error_rate = 99;
        (*sigCache).gsm_timing_advance = 0x7FFFFFFF;
        (*sigCache).lte_signal_strength = 99;
        (*sigCache).lte_rsrp = 0x7FFFFFFF;
        (*sigCache).lte_rsrq = 0x7FFFFFFF;
        (*sigCache).lte_rssnr = 0x7FFFFFFF;
        (*sigCache).lte_cqi = 0x7FFFFFFF;
        (*sigCache).lte_timing_advance = 0x7FFFFFFF;
        (*sigCache).tdscdma_signal_strength = 99;
        (*sigCache).tdscdma_bit_error_rate = 99;
        (*sigCache).tdscdma_rscp = 255;
        (*sigCache).wcdma_signal_strength = 99;
        (*sigCache).wcdma_bit_error_rate = 99;
        (*sigCache).wcdma_scdma_rscp = 255;
        (*sigCache).wcdma_ecno = 255;
        updateSignalStrengthProperty();
    } else if (source == CACHE_GROUP_C2K) {
        (*sigCache).cdma_dbm = CELLINFO_INVALID;
        (*sigCache).cdma_ecio = CELLINFO_INVALID;
        (*sigCache).evdo_dbm = CELLINFO_INVALID;
        (*sigCache).evdo_ecio = CELLINFO_INVALID;
        (*sigCache).evdo_snr = CELLINFO_INVALID;
    } else if (source == CACHE_GROUP_1XRTT) {
        (*sigCache).cdma_dbm = CELLINFO_INVALID;
        (*sigCache).cdma_ecio = CELLINFO_INVALID;
    } else if (source == CACHE_GROUP_EVDO) {
        (*sigCache).evdo_dbm = CELLINFO_INVALID;
        (*sigCache).evdo_ecio = CELLINFO_INVALID;
        (*sigCache).evdo_snr = CELLINFO_INVALID;
    } else if (source == CACHE_GROUP_ALL) {
        (*sigCache).gsm_signal_strength = 99;
        (*sigCache).gsm_bit_error_rate = 99;
        (*sigCache).gsm_timing_advance = 0x7FFFFFFF;
        (*sigCache).cdma_dbm = CELLINFO_INVALID;
        (*sigCache).cdma_ecio = CELLINFO_INVALID;
        (*sigCache).evdo_dbm = CELLINFO_INVALID;
        (*sigCache).evdo_ecio = CELLINFO_INVALID;
        (*sigCache).evdo_snr = CELLINFO_INVALID;
        (*sigCache).lte_signal_strength = 99;
        (*sigCache).lte_rsrp = 0x7FFFFFFF;
        (*sigCache).lte_rsrq = 0x7FFFFFFF;
        (*sigCache).lte_rssnr = 0x7FFFFFFF;
        (*sigCache).lte_cqi = 0x7FFFFFFF;
        (*sigCache).lte_timing_advance = 0x7FFFFFFF;
        (*sigCache).tdscdma_signal_strength = 99;
        (*sigCache).tdscdma_bit_error_rate = 99;
        (*sigCache).tdscdma_rscp = 255;
        (*sigCache).wcdma_signal_strength = 99;
        (*sigCache).wcdma_bit_error_rate = 99;
        (*sigCache).wcdma_scdma_rscp = 255;
        (*sigCache).wcdma_ecno = 255;
        (*sigCache).ssRsrp = 0x7FFFFFFF;
        (*sigCache).ssRsrq = 0x7FFFFFFF;
        (*sigCache).ssSinr = 0x7FFFFFFF;
        (*sigCache).csiRsrp = 0x7FFFFFFF;
        (*sigCache).csiRsrq = 0x7FFFFFFF;
        (*sigCache).csiSinr = 0x7FFFFFFF;
        updateSignalStrengthProperty();
    } else if (source == CACHE_GROUP_NR) {
        (*sigCache).ssRsrp = 0x7FFFFFFF;
        (*sigCache).ssRsrq = 0x7FFFFFFF;
        (*sigCache).ssSinr = 0x7FFFFFFF;
        (*sigCache).csiRsrp = 0x7FFFFFFF;
        (*sigCache).csiRsrq = 0x7FFFFFFF;
        (*sigCache).csiSinr = 0x7FFFFFFF;
    } else {
        // source type invalid
    }
}

bool RmcNetworkHandler::isTdd3G() {
    bool isTdd3G = false;
    char worldMode_prop[RFX_PROPERTY_VALUE_MAX] = {0};
    int worldMode = 0;

    rfx_property_get("vendor.ril.nw.worldmode.activemode", worldMode_prop, "1");
    worldMode = atoi(worldMode_prop);
    if (worldMode == 2) {
        isTdd3G = true;
    }
    return isTdd3G;
}

int RmcNetworkHandler::isFemtocellSupport() {
    int isFemtocellSupport = 0;
    char optr[RFX_PROPERTY_VALUE_MAX] = {0};
    rfx_property_get("ro.vendor.mtk_femto_cell_support", optr, "0");
    isFemtocellSupport = atoi(optr);
#ifdef MTK_TC1_COMMON_SERVICE
    isFemtocellSupport = 1;
#endif
    if (isMatchOpid("OP12") == true ||
            isMatchOpid("OP07") == true) {
        isFemtocellSupport = 1;
    }
    return isFemtocellSupport;
}

String8 RmcNetworkHandler::getCurrentLteSignal(int slotId) {
    if (mCurrentLteSignal[slotId] == "") {
        char tempstr[RFX_PROPERTY_VALUE_MAX];
        memset(tempstr, 0, sizeof(tempstr));
        rfx_property_get(PROPERTY_NW_LTE_SIGNAL[slotId], tempstr, "");
        mCurrentLteSignal[slotId] = String8(tempstr);
    }
    return mCurrentLteSignal[slotId];
}

void RmcNetworkHandler::updateSignalStrengthProperty() {
    int rsrp = 0x7FFFFFFF;
    String8 propString;
    if (signal_strength_cache[m_slot_id]->lte_rsrp != 0x7FFFFFFF) {
        rsrp = signal_strength_cache[m_slot_id]->lte_rsrp * (-1);
    }
    propString = String8::format("%d,%d", rsrp, signal_strength_cache[m_slot_id]->lte_rssnr/10);

    if (getCurrentLteSignal(m_slot_id) != propString) {
        rfx_property_set(PROPERTY_NW_LTE_SIGNAL[m_slot_id], propString.string());
        mCurrentLteSignal[m_slot_id] = propString;
    }
}

int RmcNetworkHandler::isOp12Plmn(const char* plmn) {
    unsigned long i = 0;
    if (plmn != NULL) {
        for (i = 0 ; i < (sizeof(sOp12Plmn)/sizeof(sOp12Plmn[0])) ; i++) {
            if (strcmp(plmn, sOp12Plmn[i]) == 0) {
                //logD(LOG_TAG, "[isOp12Plmn] plmn:%s", plmn);
                return true;
            }
        }
    }
    return false;
}

bool RmcNetworkHandler::isMatchOpid(const char* opid) {
    char optr[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get("persist.vendor.operator.optr", optr, "0");

    if (strcmp(optr, opid) == 0) {
        return true;
    }
    return false;
}

char const *RmcNetworkHandler::sourceToString(int srcId) {
    switch (srcId) {
        case CACHE_GROUP_GSM:
            return "GSM";
        case CACHE_GROUP_C2K:
            return "C2K";
        case CACHE_GROUP_1XRTT:
            return "1XRTT";
        case CACHE_GROUP_EVDO:
            return "EVDO";
        case CACHE_GROUP_ALL:
            return "ALL";
        default:
            return "INVALID SRC";
    }
}

int RmcNetworkHandler::getCellInfoListV12(RfxAtLine* line, int num,
        RIL_CellInfo_v12 * response) {
    int INVALID = INT_MAX; // 0x7FFFFFFF;
    int err=0, i=0,act=0 ,cid=0,mcc=0,mnc=0,lacTac=0,pscPci=0, mnc_len=0;
    int sig1=0,sig2=0,rsrp=0,rsrq=0,rssnr=0,cqi=0,timingAdvance=0;
    int bsic=0;
    int arfcn=INVALID;
    /* C2K related cell info. */
    int nid = 0;
    int sid = 0;
    int base_station_id = 0;
    int base_station_longitude = 0;
    int base_station_latitude = 0;
    int cdma_dbm = INVALID;
    int cdma_ecio = INVALID;
    int evdo_dbm = INVALID;
    int evdo_ecio = INVALID;
    int snr = 0;
    int evdo_snr = INVALID;
    // for NR
    int nr_cid = 0, nr_tac = 0, nr_mcc = 0, nr_mnc = 0, nr_pci = 0, nr_bsic = 0, nr_arfcn = 0;
    int nr_ta = 0;
    int nr_ssrsrp_qdbm = 0, nr_ssrsrq_qdbm = 0, nr_sssinr = 0;
    int nr_csirsrp_qdbm = 0, nr_csirsrq_qdbm = 0, nr_csisinr = 0;
    // for Operator name
    char longname[MAX_OPER_NAME_LENGTH], shortname[MAX_OPER_NAME_LENGTH];
    char* mccmnc = NULL;
    char* mMnc = NULL;

     /* +ECELL: <num_of_cell>[,<act>,<cid>,<lac_or_tac>,<mcc>,<mnc>,
             <psc_or_pci>, <sig1>,<sig2>,<sig1_in_dbm>,<sig2_in_dbm>,<ta>,
             <ext1>,<ext2>,]
             [<ext3>,<ext4>]
             [,...]
             [,<Act>,<nid>,<sid>,<bs_id>,<bs_long>,<bs_lat>,<1xRTT_rssi>,
             <1xRTT_ec/io>,<EVDO_rssi>,<EVDO_ec/io>,<EVDO_snr>]
    */
     // ext3 is for gsm bsic
     // ext4 is for arfcn, uarfcn, earfch
    int ext3_ext4_enabled = ECELLext3ext4Support; //set to 1 if modem support

    for (i = 0 ; i < num ; i++) {
        /* Registered field is used to tell serving cell or neighboring cell.
           The first cell info returned is the serving cell,
           others are neighboring cell */
        if(i == 0 &&
                (isInService(voice_reg_state_cache[m_slot_id]->registration_state) ||
                isInService(data_reg_state_cache[m_slot_id]->registration_state))) {
            response[i].registered = 1;
            response[i].connectionStatus = PRIMARY_SERVING;
        } else {
            response[i].registered = 0;
            response[i].connectionStatus = NONE_SERVING;
        }
        act = line->atTokNextint(&err);
        if (err < 0) goto error;

        if (act == 0 || act == 2 || act == 7) {
            cid = line->atTokNexthexint(&err);
            if (err < 0)
                goto error;

            lacTac = line->atTokNexthexint(&err);
            if (err < 0) {
                lacTac = INVALID;
            }
            mcc = line->atTokNextint(&err);
            if (err < 0) {
                mcc = INVALID;
            }
            mMnc= line->atTokNextstr(&err);
            if (err < 0) {
                mnc = INVALID;
            } else {
                mnc = atoi(mMnc);
                mnc_len = strlen(mMnc) > 2 ? 3 : 2;
            }
            pscPci = line->atTokNextint(&err);
            if (err < 0) {
                pscPci = INVALID;
            }
            sig1 = line->atTokNextint(&err);
            if (err < 0) {
                sig1 = INVALID;
            }
            sig2 = line->atTokNextint(&err);
            if (err < 0) {
                sig2 = INVALID;
            }
            rsrp = line->atTokNextint(&err);
            if (err < 0) {
                rsrp = INVALID;
            }
            rsrq = line->atTokNextint(&err);
            if (err < 0) {
                rsrq = INVALID;
            }
            timingAdvance = line->atTokNextint(&err);
            if (err < 0) {
                timingAdvance = INVALID;
            }
            rssnr = line->atTokNextint(&err);
            if (err < 0) {
                rssnr = INVALID;
            }
            cqi = line->atTokNextint(&err);
            if (err < 0) {
                cqi = INVALID;
            }
            if (ext3_ext4_enabled) {
                bsic = line->atTokNextint(&err);
                if (err < 0) {
                    bsic = 0;
                }
                arfcn = line->atTokNextint(&err);
                if (err < 0) {
                    arfcn = INVALID;
                }
            }
            char* cid_s = getMask(cid);
            char* lac_s = getMask(lacTac);
            char* pci_s = getMask(pscPci);
            if (!cid_s || !lac_s || !pci_s) {
                logE(LOG_TAG, "[%s] can not get memory to print log", __func__);
            } else {
                logV(LOG_TAG, "act=%d,cid=%s,mcc=%d,mnc=%d,mnc_len=%d,lacTac=%s,"
                        "pscPci=%s,sig1=%d,sig2=%d,sig1_dbm=%d,sig1_dbm=%d,ta=%d,"
                        "rssnr=%d,cqi=%d,bsic=%d,arfcn=%d",
                        act, cid_s, mcc, mnc, mnc_len, lac_s, pci_s, sig1, sig2, rsrp,
                        rsrq, timingAdvance, rssnr, cqi, bsic, arfcn);
            }
            if (cid_s) free(cid_s);
            if (lac_s) free(lac_s);
            if (pci_s) free(pci_s);
            // keep first cell info
            if (i == 0) {
                mdEcell[m_slot_id]->act = act;
                mdEcell[m_slot_id]->cid = cid;
                mdEcell[m_slot_id]->mcc = mcc;
                mdEcell[m_slot_id]->mnc = mnc;
                mdEcell[m_slot_id]->lacTac = lacTac;
                mdEcell[m_slot_id]->pscPci = pscPci;
                mdEcell[m_slot_id]->sig1 = sig1;
                mdEcell[m_slot_id]->sig2 = sig2;
                mdEcell[m_slot_id]->rsrp = rsrp;
                mdEcell[m_slot_id]->rsrq = rsrq;
                mdEcell[m_slot_id]->timingAdvance = timingAdvance;
                mdEcell[m_slot_id]->rssnr = rssnr;
                mdEcell[m_slot_id]->cqi = cqi;
                mdEcell[m_slot_id]->bsic = bsic;
                mdEcell[m_slot_id]->arfcn = arfcn;
            }
            // Skip cell information if signal is invalid in serving cell
            if (i == 0 && sig1 == 255 && sig2 == 255) {
                goto error;
            }
        } else if (act == 256) {
            response[i].registered = 1;
            response[i].connectionStatus = PRIMARY_SERVING;
            nid = line->atTokNextint(&err);
            if (err < 0) {
                nid = INVALID;
            }
            sid = line->atTokNextint(&err);
            if (err < 0) {
                sid = INVALID;
            }
            base_station_id = line->atTokNextint(&err);
            if (err < 0) {
                base_station_id = INVALID;
            }
            base_station_longitude = line->atTokNextint(&err);
            if (err < 0) {
                base_station_longitude = INVALID;
            }
            base_station_latitude = line->atTokNextint(&err);
            if (err < 0) {
                base_station_latitude = INVALID;
            }
            cdma_dbm = line->atTokNextint(&err);
            if (err < 0 || cdma_dbm < 0  || cdma_dbm > 31) {
                cdma_dbm = INVALID;
            }
            cdma_ecio = line->atTokNextint(&err);
            if (err < 0 || cdma_ecio < -128 || cdma_ecio > 0) {
                cdma_ecio = INVALID;
            }
            evdo_dbm = line->atTokNextint(&err);
            if (err < 0 || evdo_dbm < 0  || evdo_dbm > 31) {
                evdo_dbm = INVALID;
            }
            evdo_ecio = line->atTokNextint(&err);
            if (err < 0 || evdo_ecio < -512 || evdo_ecio > 0) {
                evdo_ecio = INVALID;
            }
            snr = line->atTokNextint(&err);
            if (err < 0) {
                snr = INVALID;
            }
            if (snr != INVALID) {
                float temp = (double) snr / 512;
                float snr_db = 100 * log10(temp);
                if (snr_db >= 100) {
                    evdo_snr = 8;
                } else if (snr_db >= 70) {
                    evdo_snr = 7;
                } else if (snr_db >= 50) {
                    evdo_snr = 6;
                } else if (snr_db >= 30) {
                    evdo_snr = 5;
                } else if (snr_db >= -20) {
                    evdo_snr = 4;
                } else if (snr_db >= -45) {
                    evdo_snr = 3;
                } else if (snr_db >= -90) {
                    evdo_snr = 2;
                } else if (snr_db > -120) {
                    evdo_snr = 1;
                } else {
                    evdo_snr = 0;
                }
            }

            // Print cell info log
            char* nid_s = getMask(nid);
            char* sid_s = getMask(sid);
            char* base_station_id_s = getMask(base_station_id);
            char* base_station_longitude_s = getMask(base_station_longitude);
            char* base_station_latitude_s = getMask(base_station_latitude);
            if (!nid_s || !sid_s || !base_station_id_s || !base_station_longitude_s
                    || !base_station_latitude_s) {
                logE(LOG_TAG, "[%s] can not get memory to print log", __func__);
            } else {
                logD(LOG_TAG, "nid=%s,sid=%s,base_station_id=%s,"
                        "base_station_longitude=%s,base_station_latitude=%s,"
                        "cdma_dbm=%d,cdma_ecio=%d,evdo_dbm=%d,evdo_ecio=%d,"
                        "snr=%d,evdo_snr=%d",
                        nid_s, sid_s, base_station_id_s, base_station_longitude_s,
                        base_station_latitude_s, cdma_dbm, cdma_ecio, evdo_dbm,
                        evdo_ecio, snr, evdo_snr);
            }
            if (nid_s) free(nid_s);
            if (sid_s) free(sid_s);
            if (base_station_id_s) free(base_station_id_s);
            if (base_station_longitude_s) free(base_station_longitude_s);
            if (base_station_latitude_s) free(base_station_latitude_s);
        } else if (act == 11) { // NR
            nr_cid = line->atTokNextint(&err);
            if (err < 0) nr_cid = INVALID;
            nr_tac = line->atTokNextint(&err);
            if (err < 0) nr_tac = INVALID;
            nr_mcc = line->atTokNextint(&err);
            if (err < 0) nr_mcc = INVALID;
            nr_mnc = line->atTokNextint(&err);
            if (err < 0) nr_mnc = INVALID;
            nr_pci = line->atTokNextint(&err);
            if (err < 0) nr_pci = INVALID;
            nr_ssrsrp_qdbm= line->atTokNextint(&err);
            if (err < 0) nr_ssrsrp_qdbm = INVALID;
            nr_ssrsrq_qdbm = line->atTokNextint(&err);
            if (err < 0) nr_ssrsrq_qdbm = INVALID;
            nr_csirsrp_qdbm= line->atTokNextint(&err);
            if (err < 0) nr_csirsrp_qdbm = INVALID;
            nr_csirsrq_qdbm = line->atTokNextint(&err);
            if (err < 0) nr_csirsrq_qdbm = INVALID;
            nr_ta = line->atTokNextint(&err);
            if (err < 0) nr_ta = INVALID;
            nr_sssinr = line->atTokNextint(&err);
            if (err < 0) nr_sssinr = INVALID;
            nr_csisinr = line->atTokNextint(&err);
            if (err < 0) nr_csisinr = INVALID;
            nr_bsic= line->atTokNextint(&err);
            if (err < 0) nr_bsic = INVALID;
            nr_arfcn = line->atTokNextint(&err);
            if (err < 0) nr_arfcn = INVALID;
        } else {
            logD(LOG_TAG, "RIL_CELL_INFO_TYPE invalid act=%d", act);
        }

        /* <Act>  0: GSM , 2: UMTS , 7: LTE, 256: 1x */
        if(act == 7) {
            if (mnc_len == 3) asprintf(&mccmnc, "%d%03d", mcc, mnc);
            else if (mnc_len == 2) asprintf(&mccmnc, "%d%02d", mcc, mnc);
            else asprintf(&mccmnc, "%d%d", mcc, mnc);
            getPLMNNameFromNumeric(mccmnc, longname, shortname, MAX_OPER_NAME_LENGTH);
            free(mccmnc);
            response[i].cellInfoType = RIL_CELL_INFO_TYPE_LTE;
            response[i].CellInfo.lte.cellIdentityLte.ci = cid;
            response[i].CellInfo.lte.cellIdentityLte.mcc = mcc;
            response[i].CellInfo.lte.cellIdentityLte.mnc = mnc;
            response[i].CellInfo.lte.cellIdentityLte.mnc_len= mnc_len;
            response[i].CellInfo.lte.cellIdentityLte.tac = lacTac;
            response[i].CellInfo.lte.cellIdentityLte.pci = pscPci;
            response[i].CellInfo.lte.cellIdentityLte.earfcn = arfcn;
            asprintf(&(response[i].CellInfo.lte.cellIdentityLte.operName.long_name),
                    "%s", longname);
            asprintf(&(response[i].CellInfo.lte.cellIdentityLte.operName.short_name),
                    "%s", shortname);
            response[i].CellInfo.lte.signalStrengthLte.signalStrength = sig1;

            int aosp_rsrp = -rsrp / 4;
            if (aosp_rsrp > 140) aosp_rsrp = 140;
            else if (aosp_rsrp < 44) aosp_rsrp = 44;
            response[i].CellInfo.lte.signalStrengthLte.rsrp = aosp_rsrp;

            int aosp_rsrq = rsrq != INVALID ? -rsrq / 4 : INVALID;
            if (aosp_rsrq > 35 && aosp_rsrq != INVALID) aosp_rsrq = 35;
            else if (aosp_rsrq < 3 && aosp_rsrq != INVALID) aosp_rsrq = 3;
            response[i].CellInfo.lte.signalStrengthLte.rsrq = aosp_rsrq;

            int aosp_ta = timingAdvance != INVALID ? timingAdvance : INVALID;
            if (aosp_ta < 0 && aosp_ta != INVALID) aosp_ta = 0;
            else if (aosp_ta > 1282 && aosp_ta != INVALID) aosp_ta = 1282;
            response[i].CellInfo.lte.signalStrengthLte.timingAdvance = aosp_ta;

            if (rssnr < 0 && rssnr != INVALID) rssnr = 0;
            else if (rssnr > 50 && rssnr != INVALID) rssnr = 50;
            response[i].CellInfo.lte.signalStrengthLte.rssnr = rssnr;

            if (cqi < 0 && cqi != INVALID) cqi = 0;
            else if (cqi > 30 && cqi != INVALID) cqi = 30;
            response[i].CellInfo.lte.signalStrengthLte.cqi = cqi;
        } else if(act == 2) {
            if (mnc_len == 3) asprintf(&mccmnc, "%d%03d", mcc, mnc);
            else if (mnc_len == 2) asprintf(&mccmnc, "%d%02d", mcc, mnc);
            else asprintf(&mccmnc, "%d%d", mcc, mnc);
            getPLMNNameFromNumeric(mccmnc, longname, shortname, MAX_OPER_NAME_LENGTH);
            free(mccmnc);
            response[i].cellInfoType = RIL_CELL_INFO_TYPE_WCDMA;
            response[i].CellInfo.wcdma.cellIdentityWcdma.cid = cid;
            response[i].CellInfo.wcdma.cellIdentityWcdma.mcc = mcc;
            response[i].CellInfo.wcdma.cellIdentityWcdma.mnc = mnc;
            response[i].CellInfo.wcdma.cellIdentityWcdma.mnc_len= mnc_len;
            response[i].CellInfo.wcdma.cellIdentityWcdma.lac = lacTac;
            response[i].CellInfo.wcdma.cellIdentityWcdma.psc = pscPci;
            response[i].CellInfo.wcdma.cellIdentityWcdma.uarfcn = arfcn;
            asprintf(&(response[i].CellInfo.wcdma.cellIdentityWcdma.operName.long_name),
                    "%s", longname);
            asprintf(&(response[i].CellInfo.wcdma.cellIdentityWcdma.operName.short_name),
                    "%s", shortname);
            response[i].CellInfo.wcdma.signalStrengthWcdma.signalStrength =
                    convert3GRssiValue(sig1-120);
            response[i].CellInfo.wcdma.signalStrengthWcdma.bitErrorRate = sig2;
        } else if (act == 0) {
            if (mnc_len == 3) asprintf(&mccmnc, "%d%03d", mcc, mnc);
            else if (mnc_len == 2) asprintf(&mccmnc, "%d%02d", mcc, mnc);
            else asprintf(&mccmnc, "%d%d", mcc, mnc);
            getPLMNNameFromNumeric(mccmnc, longname, shortname, MAX_OPER_NAME_LENGTH);
            free(mccmnc);
            response[i].cellInfoType = RIL_CELL_INFO_TYPE_GSM;
            response[i].CellInfo.gsm.cellIdentityGsm.cid = cid;
            response[i].CellInfo.gsm.cellIdentityGsm.mcc = mcc;
            response[i].CellInfo.gsm.cellIdentityGsm.mnc = mnc;
            response[i].CellInfo.gsm.cellIdentityGsm.mnc_len= mnc_len;
            response[i].CellInfo.gsm.cellIdentityGsm.lac = lacTac;
            response[i].CellInfo.gsm.cellIdentityGsm.arfcn = arfcn;
            response[i].CellInfo.gsm.cellIdentityGsm.bsic = bsic;
            asprintf(&(response[i].CellInfo.gsm.cellIdentityGsm.operName.long_name),
                    "%s", longname);
            asprintf(&(response[i].CellInfo.gsm.cellIdentityGsm.operName.short_name),
                    "%s", shortname);
            int aosp_level = (sig1 + 2) / 2;
            if (aosp_level > 31) aosp_level = 31;
            else if (aosp_level < 0) aosp_level = 0;
            response[i].CellInfo.gsm.signalStrengthGsm.signalStrength = aosp_level;
            response[i].CellInfo.gsm.signalStrengthGsm.bitErrorRate = sig2;

            if (timingAdvance < 0 && timingAdvance != INVALID) timingAdvance = 0;
            else if (timingAdvance > 219 && timingAdvance != INVALID) timingAdvance = 219;
            response[i].CellInfo.gsm.signalStrengthGsm.timingAdvance = timingAdvance;
        } else if (act == 256) {
            response[i].cellInfoType = RIL_CELL_INFO_TYPE_CDMA;
            response[i].CellInfo.cdma.cellIdentityCdma.networkId = nid;
            response[i].CellInfo.cdma.cellIdentityCdma.systemId = sid;
            response[i].CellInfo.cdma.cellIdentityCdma.basestationId = base_station_id;
            response[i].CellInfo.cdma.cellIdentityCdma.longitude = base_station_longitude;
            response[i].CellInfo.cdma.cellIdentityCdma.latitude = base_station_latitude;
            response[i].CellInfo.cdma.cellIdentityCdma.operName.long_name = NULL;  // unknown
            response[i].CellInfo.cdma.cellIdentityCdma.operName.short_name = NULL;  // unknown
            ViaBaseHandler *mViaHandler = RfxViaUtils::getViaHandler();
            if (mViaHandler != NULL && cdma_dbm != INVALID) {
                response[i].CellInfo.cdma.signalStrengthCdma.dbm =
                        mViaHandler->convertCdmaEvdoSig(cdma_dbm,
                                SIGNAL_CDMA_DBM);
            } else {
                response[i].CellInfo.cdma.signalStrengthCdma.dbm = INVALID;
            }
            if (mViaHandler != NULL && cdma_ecio != INVALID) {
                response[i].CellInfo.cdma.signalStrengthCdma.ecio =
                        mViaHandler->convertCdmaEvdoSig(cdma_ecio,
                                SIGNAL_CDMA_ECIO);
            } else {
                response[i].CellInfo.cdma.signalStrengthCdma.ecio = INVALID;
            }
            if (mViaHandler != NULL && evdo_dbm != INVALID) {
                response[i].CellInfo.cdma.signalStrengthEvdo.dbm =
                        mViaHandler->convertCdmaEvdoSig(evdo_dbm,
                                SIGNAL_EVDO_DBM);
            } else {
                response[i].CellInfo.cdma.signalStrengthEvdo.dbm = INVALID;

            }
            if (mViaHandler != NULL && evdo_ecio != INVALID) {
                response[i].CellInfo.cdma.signalStrengthEvdo.ecio =
                        mViaHandler->convertCdmaEvdoSig(evdo_ecio,
                                SIGNAL_EVDO_ECIO);
            } else {
                response[i].CellInfo.cdma.signalStrengthEvdo.ecio = INVALID;

            }
            response[i].CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio = evdo_snr;
            logD(LOG_TAG, "RIL_CELL_INFO_TYPE_C2K act=%d, cdma_dbm=%d, "
                    "cdma_ecio=%d, evdo_dbm=%d, evdo_ecio=%d, evdo_snr=%d ",
                    act, response[i].CellInfo.cdma.signalStrengthCdma.dbm,
                    response[i].CellInfo.cdma.signalStrengthCdma.ecio,
                    response[i].CellInfo.cdma.signalStrengthEvdo.dbm,
                    response[i].CellInfo.cdma.signalStrengthEvdo.ecio,
                    response[i].CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio);
        } else if (act == 11) {
            if (nr_mcc == INVALID || nr_mnc == INVALID) asprintf(&mccmnc, "00000");
            else if (nr_mnc > 99) asprintf(&mccmnc, "%d%03d", nr_mcc, nr_mnc);
            else asprintf(&mccmnc, "%d%02d", nr_mcc, nr_mnc);
            getPLMNNameFromNumeric(mccmnc, longname, shortname, MAX_OPER_NAME_LENGTH);
            free(mccmnc);
            response[i].cellInfoType = RIL_CELL_INFO_TYPE_NR;
            response[i].CellInfo.nr.cellidentity.mcc = nr_mcc;
            response[i].CellInfo.nr.cellidentity.mnc = nr_mnc;
            response[i].CellInfo.nr.cellidentity.nci = nr_cid;
            response[i].CellInfo.nr.cellidentity.nrarfcn = nr_arfcn;
            response[i].CellInfo.nr.cellidentity.pci = nr_pci;
            response[i].CellInfo.nr.cellidentity.tac = nr_tac;
            asprintf(&(response[i].CellInfo.nr.cellidentity.operName.long_name), "%s", longname);
            asprintf(&(response[i].CellInfo.nr.cellidentity.operName.short_name), "%s", shortname);
            response[i].CellInfo.nr.signalStrength.ssRsrp = nr_ssrsrp_qdbm;
            response[i].CellInfo.nr.signalStrength.ssRsrq = nr_ssrsrq_qdbm;
            response[i].CellInfo.nr.signalStrength.ssSinr = nr_sssinr;
            response[i].CellInfo.nr.signalStrength.csiRsrp = nr_csirsrp_qdbm;
            response[i].CellInfo.nr.signalStrength.csiRsrq = nr_csirsrq_qdbm;
            response[i].CellInfo.nr.signalStrength.csiSinr = nr_csisinr;
        } else {
            logD(LOG_TAG, "RIL_CELL_INFO_TYPE invalid act=%d", act);
        }
    }
    return 0;
error:
    return -1;
}

int RmcNetworkHandler::convertToModUtf8Encode(int src) {
    int rlt = src;
    int byte1 = 0;
    int byte2 = 0;
    int byte3 = 0;

    if (src > 0x7FF) {
        byte1 = (src >> 12) | 0xE0;
        byte2 = ((src >> 6) & 0x3F) | 0x80;
        byte3 = (src & 0x3F) | 0x80;
        rlt = (byte1 << 16) | (byte2 << 8) | byte3;
    } else if (src > 0x7F) {
        byte1 = (src >> 6) | 0xC0;
        byte2 = (src & 0x3F) | 0x80;
        rlt = (byte1 << 8) | byte2;
    }
    return rlt;
}

void RmcNetworkHandler::convertToUtf8String(char *src) {
    int i;
    int idx;
    int len;
    int cvtCode;
    char temp_oper_name[MAX_OPER_NAME_LENGTH] = {0};

    idx = 0;
    len = strlen(src);
    memset(temp_oper_name, 0, sizeof(char) * MAX_OPER_NAME_LENGTH);
    for (i = 0; i < len; i++) {
        cvtCode = convertToModUtf8Encode(src[i]);
        // logD(LOG_TAG, "cvtUTF8 %x", cvtCode);
        if ((cvtCode & 0xFF0000) > 0) {
            if (idx >= MAX_OPER_NAME_LENGTH - 3) {
                break;
            }
            temp_oper_name[idx++] = (cvtCode & 0xFF0000) >> 16;
            temp_oper_name[idx++] = (cvtCode & 0xFF00) >> 8;
            temp_oper_name[idx++] = cvtCode & 0xFF;
        } else if ((cvtCode & 0xFF00) > 0) {
            if (idx >= MAX_OPER_NAME_LENGTH - 2) {
                break;
            }
            temp_oper_name[idx++] = (cvtCode & 0xFF00) >> 8;
            temp_oper_name[idx++] = cvtCode & 0xFF;
        } else {
            temp_oper_name[idx++] = cvtCode;
        }
        if (idx == MAX_OPER_NAME_LENGTH) {
            break;
        }
    }
    temp_oper_name[MAX_OPER_NAME_LENGTH - 1] = '\0';
    strncpy(src, temp_oper_name, MAX_OPER_NAME_LENGTH);
    // logD(LOG_TAG, "convertToUtf8String %s", src);
}

unsigned int RmcNetworkHandler::convertCSNetworkType(unsigned int uiResponse)
{
    RIL_RadioTechnology uiRet = RADIO_TECH_UNKNOWN;

    /* mapping */
    switch(uiResponse)
    {
        case 0x0001:     // GPRS
        case 0x0002:     // EDGE
            uiRet = RADIO_TECH_GSM;        // GSM
            break;
        case 0x0004:     // UMTS
        case 0x0008:     // HSDPA
        case 0x0010:     // HSUPA
        case 0x0018:     // HSDPA_UPA
        case 0x0020:     // HSDPAP
        case 0x0030:     // HSDPAP_UPA
        case 0x0040:     // HSUPAP
        case 0x0048:     // HSUPAP_DPA
        case 0x0060:     // HSPAP
        case 0x0088:     // DC_DPA
        case 0x0098:     // DC_DPA_UPA
        case 0x00a0:     // DC_HSDPAP
        case 0x00b0:     // DC_HSDPAP_UPA
        case 0x00c8:     // DC_HSUPAP_DPA
        case 0x00e0:     // DC_HSPAP
            uiRet = RADIO_TECH_UMTS;        // UMTS
            break;
        // for C2K
        case 0x0100:     // 1xRTT
            uiRet = RADIO_TECH_1xRTT;        // 1xRTT
            break;
        case 0x0200:     // HRPD
            uiRet = RADIO_TECH_EVDO_A;        // EVDO_A
            break;
        case 0x0400:     // EHRPD
            uiRet = RADIO_TECH_EHRPD;         // EHRPD
            break;
        //for LTE
        case 0x1000:     // LTE
        case 0x2000:     // LTE_CA
        case 0x4000:
            uiRet = RADIO_TECH_LTE;       // LTE
            break;
        case 0x8000:
            uiRet = RADIO_TECH_NR;
            break;
        default:
            uiRet = RADIO_TECH_UNKNOWN;        // UNKNOWN
            break;
    }

    return (unsigned int)uiRet;
}

unsigned int RmcNetworkHandler::convertPSNetworkType(unsigned int uiResponse)
{
    unsigned int uiRet = 0;

    /* mapping */
    switch(uiResponse)
    {
        case 0x0001:     // GPRS
            uiRet = 1;        // GPRS
            break;
        case 0x0002:     // EDGE
            uiRet = 2;        // EDGE
            break;
        case 0x0004:     // UMTS
            uiRet = 3;        // UMTS
            break;
        case 0x0008:     // HSDPA
            uiRet = 9;        // HSDPA
            break;
        case 0x0010:     // HSUPA
            uiRet = 10;        // HSUPA
            break;
        case 0x0018:     // HSDPA_UPA
            uiRet = 11;       // HSPA
            break;
        case 0x0020:     // HSDPAP
        case 0x0030:     // HSDPAP_UPA
        case 0x0040:     // HSUPAP
        case 0x0048:     // HSUPAP_DPA
        case 0x0060:     // HSPAP
        case 0x0088:     // DC_DPA
        case 0x0098:     // DC_DPA_UPA
        case 0x00a0:     // DC_HSDPAP
        case 0x00b0:     // DC_HSDPAP_UPA
        case 0x00c8:     // DC_HSUPAP_DPA
        case 0x00e0:     // DC_HSPAP
            uiRet = 15;       // HSPAP
            break;
        // for C2K
        case 0x0100:     // 1xRTT
            uiRet = 6;        // 1xRTT
            break;
        case 0x0200:     // HRPD
            uiRet = 8;        // EVDO_A
            break;
        case 0x0400:     // EHRPD
            uiRet = 13;         // EHRPD
            break;
        //for LTE
        case 0x1000:     // LTE
            uiRet = 14;       // LTE
            break;
        case 0x2000:     // LTE_CA
        case 0x4000:
            uiRet = 19;       // LTE
            break;
        case 0x8000:
            uiRet = 20;   // NR
            break;
        default:
            uiRet = 0;        // UNKNOWN
            break;
    }

    return uiRet;
}

void RmcNetworkHandler::updateWfcState(int status) {
    logV(LOG_TAG, "%s, WFC state of slot%d:%d->%d", __FUNCTION__,
            m_slot_id, ril_wfc_reg_status[m_slot_id], status);
    pthread_mutex_lock(&s_wfcRegStatusMutex[m_slot_id]);
    ril_wfc_reg_status[m_slot_id] = status;  // cache wfc status
    getMclStatusManager()->setIntValue(RFX_STATUS_KEY_WFC_STATE,
            ril_wfc_reg_status[m_slot_id]);
    if (status == 1) setMSimProperty(m_slot_id, (char *)PROPERTY_WFC_STATE, (char*)"1");
    else setMSimProperty(m_slot_id, (char *)PROPERTY_WFC_STATE, (char*)"0");
    pthread_mutex_unlock(&s_wfcRegStatusMutex[m_slot_id]);
}

int RmcNetworkHandler::getOperatorNamesFromNumericCode(
    char *code,
    char *longname,
    char *shortname,
    int max_length) {

    char nitz[RFX_PROPERTY_VALUE_MAX];
    char oper_file_path[RFX_PROPERTY_VALUE_MAX];
    char oper[128], name[MAX_OPER_NAME_LENGTH];
    int err;
    char *oper_code, *oper_lname, *oper_sname;

    if (max_length > MAX_OPER_NAME_LENGTH) {
        logE(LOG_TAG, "The buffer size %d is not valid. We only accept the length less than or equal to %d",
                 max_length, MAX_OPER_NAME_LENGTH);
        return -1;
    }
    oper_code = m_ril_nw_nitz_oper_code[m_slot_id];
    oper_lname = m_ril_nw_nitz_oper_lname[m_slot_id];
    oper_sname = m_ril_nw_nitz_oper_sname[m_slot_id];

    longname[0] = '\0';
    shortname[0] = '\0';

    pthread_mutex_lock(&ril_nw_nitzName_mutex[m_slot_id]);
    // logD(LOG_TAG, "Get ril_nw_nitzName_mutex in the getOperatorNamesFromNumericCode");
    // logD(LOG_TAG, "getOperatorNamesFromNumericCode code:%s oper_code:%s", code, oper_code);

    /* Check if there is a NITZ name*/
    /* compare if the operator code is the same with <oper>*/
    if(strcmp(code, oper_code) == 0) {
        /* there is a NITZ Operator Name */
        /* get operator code and name */
        /* set short name with long name when short name is null and long name isn't, and vice versa */
        int nlnamelen = strlen(oper_lname);
        int nsnamelen = strlen(oper_sname);
        if(nlnamelen != 0 && nsnamelen != 0) {
            strncpy(longname,oper_lname, max_length);
            strncpy(shortname, oper_sname, max_length);
        } else if(strlen(oper_sname) != 0) {
            strncpy(longname, oper_sname, max_length);
            strncpy(shortname, oper_sname, max_length);
        } else if(strlen(oper_lname) != 0) {
            strncpy(longname, oper_lname, max_length);
            strncpy(shortname, oper_lname, max_length);
        }
        logD(LOG_TAG, "Return NITZ Operator Name: %s %s %s, lname length: %d, sname length: %d",
                oper_code, oper_lname, oper_sname, nlnamelen, nsnamelen);
    } else {
        //strcpy(longname, code);
        //strcpy(shortname, code);
        getPLMNNameFromNumeric(code, longname, shortname, max_length);
    }
     pthread_mutex_unlock(&ril_nw_nitzName_mutex[m_slot_id]);
    return 0;
}

int RmcNetworkHandler::getEonsNamesFromNumericCode(
        char *code,
        unsigned int lac,
        char *longname,
        char *shortname,
        int max_length)
{
    int err;
    int skip;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> resp;
    RfxAtLine* line;
    char *oper_lname = NULL, *oper_sname = NULL;
    int dcs_long = 0;
    int dcs_short = 0;

    if (eons_info[m_slot_id].eons_status == EONS_INFO_RECEIVED_ENABLED &&
        ((lac >= 0) && (lac < 0xFFFF))) {
        p_response = atSendCommandSingleline(String8::format("AT+EONS=2,\"%s\",%d", code, lac), "+EONS");
        err = p_response->getError();
        if (err != 0 || p_response->getSuccess() == 0 ||
                p_response->getIntermediates() == NULL) {
            logE(LOG_TAG, "EONS got error response");
            goto error;
        } else {
            line = p_response->getIntermediates();
            line->atTokStart(&err);
            if (err >= 0) {
                skip = line->atTokNextint(&err);
                /* <Result> */
                if (err >= 0 && skip == 1) {
                    /* <dcs_long> */
                    dcs_long = line->atTokNextint(&err);
                    if (err < 0) goto error;

                    /* <long_oper> */
                    oper_lname = line->atTokNextstr(&err);
                    logD(LOG_TAG, "EONS Get operator long %s", longname);
                    if (err < 0) goto error;

                    /* <dcs_short> */
                    dcs_short = line->atTokNextint(&err);
                    if (err < 0) goto error;

                    /* <short_oper> */
                    oper_sname = line->atTokNextstr(&err);
                    logD(LOG_TAG, "EONS Get operator short %s", shortname);
                    if (err < 0) goto error;
                    // check long_oper and short_oper are valid.
                    if((strcmp(oper_lname, "") == 0)
                            && (strcmp(oper_sname, "") == 0)){
                        logD(LOG_TAG, "EONS Get invalid operator name");
                        goto error;
                    }
                } else {
                    goto error;
                }
                /* Copy operator name */
                strncpy(longname, oper_lname, max_length-1);
                strncpy(shortname, oper_sname, max_length-1);
            }
        }

        logD(LOG_TAG, "Return EONS Operator Name: %s %s %s",
                code,
                oper_lname,
                oper_sname);
        return 0;
    }
error:
    return -1;
}

int RmcNetworkHandler::getPLMNNameFromNumeric(char *numeric, char *longname, char *shortname, int max_length) {
    int i = 0;
    int length_ts25 = sizeof(s_mtk_ts25_table)/sizeof(s_mtk_ts25_table[0]);
    int length_plmn = sizeof(s_mtk_plmn_table)/sizeof(s_mtk_plmn_table[0]);
    const SpnTable* spn = NULL;
    longname[0] = '\0';
    shortname[0] = '\0';

    // use the previous one
    if (strcmp(cache_spn_table[m_slot_id].mccMnc, numeric) == 0) {
        strncpy(longname, cache_spn_table[m_slot_id].spn, max_length);
        strncpy(shortname, cache_spn_table[m_slot_id].short_name, max_length);
        return 0;
    }

    // TS.25 table
    for (i=0; i < length_ts25; i++) {
        if (0 == strcmp(numeric, s_mtk_ts25_table[i].mccMnc))
        {
            spn = &s_mtk_ts25_table[i];
        }
    }

    // MTK customization PLMN name
    for (i=0; i < length_plmn; i++) {
        if (0 == strcmp(numeric, s_mtk_plmn_table[i].mccMnc))
        {
            spn = &s_mtk_plmn_table[i];
        }
    }

    if (spn != NULL) {
        strncpy(longname, spn->spn, max_length);
        strncpy(shortname, spn->short_name, max_length);

        // backup the spn for next use
        strncpy(cache_spn_table[m_slot_id].mccMnc, spn->mccMnc, 8);
        strncpy(cache_spn_table[m_slot_id].spn, spn->spn, MAX_OPER_NAME_LENGTH+2);
        strncpy(cache_spn_table[m_slot_id].short_name, spn->short_name, MAX_OPER_NAME_LENGTH+2);
        return 0;
    }
    strncpy(longname, numeric, max_length);
    strncpy(shortname, numeric, max_length);
    logV(LOG_TAG, "getPLMNNameFromNumeric: s_mtk_spn_table not found %s", numeric);
    return -1;
}

int RmcNetworkHandler::getOperatorNamesFromNumericCode(
    char *code,
    unsigned int lac,
    char *longname,
    char *shortname,
    int max_length)
{
    getOperatorNamesFromNumericCodeByDisplay(code, lac,
            longname, shortname, max_length, DISPLAY_ALL);
    return 0;
}

int RmcNetworkHandler::getOperatorNamesFromNumericCodeByDisplay(
    char *code,
    unsigned int lac,
    char *longname,
    char *shortname,
    int max_length,
    int display)
{
    char *line, *tmp;
    FILE *list;
    int err;
    char *oper_code, *oper_lname, *oper_sname;

    if (max_length > MAX_OPER_NAME_LENGTH) {
        logE(LOG_TAG, "The buffer size %d is not valid. We only accept the length less than or equal to %d",
             max_length, MAX_OPER_NAME_LENGTH);
        return -1;
    }

    longname[0] = '\0';
    shortname[0] = '\0';

    /* To support return EONS if available in RIL_REQUEST_OPERATOR START */
    if ((display & DISPLAY_EONS) == DISPLAY_EONS &&
            eons_info[m_slot_id].eons_status == EONS_INFO_RECEIVED_ENABLED) {
        err = getEonsNamesFromNumericCode(
                code, lac, longname, shortname, max_length);
        if (err == 0) {
            logD(LOG_TAG, "Get EONS %s, %s", longname, shortname);
            return 0;
        }
    }
    /* To support return EONS if available in RIL_REQUEST_OPERATOR END */

    if ((display & DISPLAY_NITZ) == DISPLAY_NITZ) {
        int hasNitz = 0;
        oper_code = m_ril_nw_nitz_oper_code[m_slot_id];
        oper_lname = m_ril_nw_nitz_oper_lname[m_slot_id];
        oper_sname = m_ril_nw_nitz_oper_sname[m_slot_id];

        pthread_mutex_lock(&ril_nw_nitzName_mutex[m_slot_id]);

        /* Check if there is a NITZ name*/
        /* compare if the operator code is the same with <oper>*/
        if(strcmp(code, oper_code) == 0) {
            hasNitz = 1;
            /* there is a NITZ Operator Name*/
            /*get operator code and name*/
            /*set short name with long name when short name is null and long name isn't, and vice versa*/
            int nlnamelen = strlen(oper_lname);
            int nsnamelen = strlen(oper_sname);
            if (nlnamelen != 0 && nsnamelen != 0) {
                strncpy(longname,oper_lname, max_length);
                strncpy(shortname, oper_sname, max_length);
            } else if (strlen(oper_sname) != 0) {
                strncpy(longname, oper_sname, max_length);
                strncpy(shortname, oper_sname, max_length);
            } else if (strlen(oper_lname) != 0) {
                strncpy(longname, oper_lname, max_length);
                strncpy(shortname, oper_lname, max_length);
            }
            logD(LOG_TAG, "Return NITZ Operator Name: %s %s %s, lname length: %d, sname length: %d", oper_code,
                                                                                            oper_lname,
                                                                                            oper_sname,
                                                                                            nlnamelen,
                                                                                            nsnamelen);
        }
        pthread_mutex_unlock(&ril_nw_nitzName_mutex[m_slot_id]);
        if (hasNitz == 1) return 0;
    }

    if ((display & DISPLAY_TS25) == DISPLAY_TS25) {
        getPLMNNameFromNumeric(code, longname, shortname, max_length);
    }
    return 0;
}

void RmcNetworkHandler::isFemtoCell(int regState, int cid, int act) {
    int isFemtocell = 0;
    int eNBid = 0;
    String8 oper(getMclStatusManager()->getString8Value(RFX_STATUS_KEY_OPERATOR));
    eNBid = cid >> 8;
    if (isOp12Plmn(oper.string()) == true
            && act == 14  // LTE
            && regState == 1  // in home service
            && eNBid >= 1024000 && eNBid <= 1048575) { // OP12: 0xFA000 - 0xFFFFF
        isFemtocell = 2;
    }
    pthread_mutex_lock(&ril_nw_femtoCell_mutex);
    if (isFemtocell != femto_cell_cache[m_slot_id]->is_femtocell) {
        femto_cell_cache[m_slot_id]->is_femtocell = isFemtocell;
#ifndef MTK_TC1_COMMON_SERVICE
        pthread_mutex_unlock(&ril_nw_femtoCell_mutex);
        updateFemtoCellInfo();
        pthread_mutex_lock(&ril_nw_femtoCell_mutex);
#endif
    }
    pthread_mutex_unlock(&ril_nw_femtoCell_mutex);
}

void RmcNetworkHandler::updateFemtoCellInfo() {
#ifdef MTK_TC1_COMMON_SERVICE
    if (femto_cell_cache[m_slot_id]->is_csg_cell == 1) { // for GSM femtocell
        // update 99 for femtocell.
        data_reg_state_cache[m_slot_id]->roaming_indicator = 99;
    } else {
        // reset for femtocell.
        data_reg_state_cache[m_slot_id]->roaming_indicator = 0;
    }
    return;
#else
    int err;
    unsigned long i;
    char *responseStr[10] = { 0 };
    char *shortOperName = (char *) calloc(1, sizeof(char)*MAX_OPER_NAME_LENGTH);
    char *longOperName = (char *) calloc(1, sizeof(char)*MAX_OPER_NAME_LENGTH);

    if (shortOperName == NULL || longOperName == NULL) {
        logE(LOG_TAG, "updateFemtoCellInfo calloc fail");
        if (longOperName != NULL) free(longOperName);
        if (shortOperName != NULL) free(shortOperName);
        return;
    }
    pthread_mutex_lock(&ril_nw_femtoCell_mutex);
    asprintf(&responseStr[0], "%d", femto_cell_cache[m_slot_id]->domain);
    asprintf(&responseStr[1], "%d", femto_cell_cache[m_slot_id]->state);
    asprintf(&responseStr[3], "%d", femto_cell_cache[m_slot_id]->plmn_id);

    err = getOperatorNamesFromNumericCode(responseStr[3], longOperName, shortOperName, MAX_OPER_NAME_LENGTH);
    if(err < 0) {
        asprintf(&responseStr[2], "");
    } else {
        asprintf(&responseStr[2], "%s", longOperName);
    }
    free(shortOperName);
    free(longOperName);

    asprintf(&responseStr[4], "%d", femto_cell_cache[m_slot_id]->act);

    if (femto_cell_cache[m_slot_id]->is_femtocell == 2) { // for LTE/C2K femtocell
        asprintf(&responseStr[5], "%d", femto_cell_cache[m_slot_id]->is_femtocell);
    } else if (femto_cell_cache[m_slot_id]->is_csg_cell == 1) { // for GSM femtocell
        asprintf(&responseStr[5], "%d", femto_cell_cache[m_slot_id]->is_csg_cell);
    } else {
        asprintf(&responseStr[5], "0");
    }
    asprintf(&responseStr[6], "%d", femto_cell_cache[m_slot_id]->csg_id);
    asprintf(&responseStr[7], "%d", femto_cell_cache[m_slot_id]->csg_icon_type);
    asprintf(&responseStr[8], "%s", femto_cell_cache[m_slot_id]->hnbName);
    asprintf(&responseStr[9], "%d", femto_cell_cache[m_slot_id]->cause);
    pthread_mutex_unlock(&ril_nw_femtoCell_mutex);

    // <domain>,<state>,<lognname>,<plmn_id>,<act>,<is_csg_cell/is_femto_cell>,<csg_id>,<csg_icon_type>,<hnb_name>,<cause> */
    sp<RfxMclMessage> urc = RfxMclMessage::obtainUrc(RFX_MSG_URC_FEMTOCELL_INFO,
            m_slot_id, RfxStringsData(responseStr, 10));
    responseToTelCore(urc);
    // free memory
    for (i=0 ; i<(sizeof(responseStr)/sizeof(char*)) ; i++) {
        if (responseStr[i]) {
            // logD(LOG_TAG, "free responseStr[%d]=%s", i, responseStr[i]);
            free(responseStr[i]);
            responseStr[i] = NULL;
        }
    }
    return;
#endif
}

void RmcNetworkHandler::printSignalStrengthCache(char* source) {
    logD(LOG_TAG,
        "%s,"
        " gsm_signal_strength: %d,"
        " gsm_bit_error_rate: %d,"
        " gsm_timing_advance: %d,"
        " cdma_dbm: %d,"
        " cdma_ecio: %d,"
        " evdo_dbm: %d,"
        " evdo_ecio: %d,"
        " evdo_snr: %d,"
        " lte_signal_strength: %d,"
        " lte_rsrp: %d,"
        " lte_rsrq: %d,"
        " lte_rssnr: %d,"
        " lte_cqi: %d,"
        " lte_timing_advance: %d,"
        " tdscdma_signal_strength: %d,"
        " tdscdma_bit_error_rate: %d,"
        " tdscdma_rscp: %d,"
        " wcdma_signal_strength: %d,"
        " wcdma_bit_error_rate: %d,"
        " wcdma_scdma_rscp: %d,"
        " wcdma_ecno: %d,"
        " ssRsrp: %d,"
        " ssRsrq: %d,"
        " ssSinr: %d,"
        " csiRsrp: %d,"
        " csiRsrq: %d,"
        " csiSinr: %d",
        source,
        signal_strength_cache[m_slot_id]-> gsm_signal_strength,
        signal_strength_cache[m_slot_id]-> gsm_bit_error_rate,
        signal_strength_cache[m_slot_id]-> gsm_timing_advance,
        signal_strength_cache[m_slot_id]-> cdma_dbm,
        signal_strength_cache[m_slot_id]-> cdma_ecio,
        signal_strength_cache[m_slot_id]-> evdo_dbm,
        signal_strength_cache[m_slot_id]-> evdo_ecio,
        signal_strength_cache[m_slot_id]-> evdo_snr,
        signal_strength_cache[m_slot_id]-> lte_signal_strength,
        signal_strength_cache[m_slot_id]-> lte_rsrp,
        signal_strength_cache[m_slot_id]-> lte_rsrq,
        signal_strength_cache[m_slot_id]-> lte_rssnr,
        signal_strength_cache[m_slot_id]-> lte_cqi,
        signal_strength_cache[m_slot_id]-> lte_timing_advance,
        signal_strength_cache[m_slot_id]-> tdscdma_signal_strength,
        signal_strength_cache[m_slot_id]-> tdscdma_bit_error_rate,
        signal_strength_cache[m_slot_id]-> tdscdma_rscp,
        signal_strength_cache[m_slot_id]-> wcdma_signal_strength,
        signal_strength_cache[m_slot_id]-> wcdma_bit_error_rate,
        signal_strength_cache[m_slot_id]-> wcdma_scdma_rscp,
        signal_strength_cache[m_slot_id]-> wcdma_ecno,
        signal_strength_cache[m_slot_id]-> ssRsrp,
        signal_strength_cache[m_slot_id]-> ssRsrq,
        signal_strength_cache[m_slot_id]-> ssSinr,
        signal_strength_cache[m_slot_id]-> csiRsrp,
        signal_strength_cache[m_slot_id]-> csiRsrq,
        signal_strength_cache[m_slot_id]-> csiSinr);
}

RIL_RAT_GROUP RmcNetworkHandler::getVoiceRatGroup(int act) {
    RIL_RAT_GROUP result = RAT_GROUP_UNKNOWN;
    /* mapping */
    switch(act)
    {
        case 0x0001:     // GPRS
        case 0x0002:     // EDGE
            result = RAT_GROUP_2G;
            break;
        case 0x0004:     // UMTS
        case 0x0008:     // HSDPA
        case 0x0010:     // HSUPA
        case 0x0018:     // HSDPA_UPA
        case 0x0020:     // HSDPAP
        case 0x0030:     // HSDPAP_UPA
        case 0x0040:     // HSUPAP
        case 0x0048:     // HSUPAP_DPA
        case 0x0060:     // HSPAP
        case 0x0088:     // DC_DPA
        case 0x0098:     // DC_DPA_UPA
        case 0x00a0:     // DC_HSDPAP
        case 0x00b0:     // DC_HSDPAP_UPA
        case 0x00c8:     // DC_HSUPAP_DPA
        case 0x00e0:     // DC_HSPAP
            result = RAT_GROUP_3G;
            break;
        // for C2K
        case 0x0100:     // 1xRTT
        case 0x0200:     // HRPD
        case 0x0400:     // EHRPD
            result = RAT_GROUP_UNKNOWN;
            break;
        //for LTE
        case 0x1000:     // LTE
        case 0x2000:     // LTE_CA
            result = RAT_GROUP_4G;
            break;
        default:
            result = RAT_GROUP_UNKNOWN;
            break;
    }
    return result;
}

RIL_RAT_GROUP RmcNetworkHandler::getDataRatGroup(int act) {
    RIL_RAT_GROUP result = RAT_GROUP_UNKNOWN;
    /* mapping */
    switch(act)
    {
        case 0x0001:     // GPRS
        case 0x0002:     // EDGE
            result = RAT_GROUP_2G;
            break;
        case 0x0004:     // UMTS
        case 0x0008:     // HSDPA
        case 0x0010:     // HSUPA
        case 0x0018:     // HSDPA_UPA
        case 0x0020:     // HSDPAP
        case 0x0030:     // HSDPAP_UPA
        case 0x0040:     // HSUPAP
        case 0x0048:     // HSUPAP_DPA
        case 0x0060:     // HSPAP
        case 0x0088:     // DC_DPA
        case 0x0098:     // DC_DPA_UPA
        case 0x00a0:     // DC_HSDPAP
        case 0x00b0:     // DC_HSDPAP_UPA
        case 0x00c8:     // DC_HSUPAP_DPA
        case 0x00e0:     // DC_HSPAP
            result = RAT_GROUP_3G;
            break;
        // for C2K
        case 0x0100:     // 1xRTT
        case 0x0200:     // HRPD
        case 0x0400:     // EHRPD
            result = RAT_GROUP_UNKNOWN;
            break;
        //for LTE
        case 0x1000:     // LTE
        case 0x2000:     // LTE_CA
            result = RAT_GROUP_4G;
            break;
        default:
            result = RAT_GROUP_UNKNOWN;
            break;
    }
    return result;
}

void RmcNetworkHandler::isNeedNotifyStateChanged() {
    logV(LOG_TAG, "isNeedNotifyStateChanged: %d", mIsNeedNotifyState[m_slot_id]);
    if (mIsNeedNotifyState[m_slot_id] == true) {
        sp<RfxMclMessage> urc;
        mIsNeedNotifyState[m_slot_id] = false;
        urc = RfxMclMessage::obtainUrc(RFX_MSG_URC_RESPONSE_VOICE_NETWORK_STATE_CHANGED,
                m_slot_id, RfxVoidData());
        // response to TeleCore
        responseToTelCore(urc);
    }
}

void RmcNetworkHandler::setLastValidPlmn(const char* plmn) {
    if (plmn != NULL &&
            (strncmp(plmn, "000000", 6) != 0) &&
            (plmn[0] >= '0' && plmn[0] <= '9')) {
        last_valid_plmn_time[m_slot_id] = systemTime(SYSTEM_TIME_MONOTONIC);
        strncpy(last_valid_plmn[m_slot_id], plmn, 7);
        last_valid_plmn[m_slot_id][7] = '\0';
        // logD(LOG_TAG, "setLastValidPlmn: %ld, %s",
        //         (last_valid_plmn_time[m_slot_id]/1000000000), last_valid_plmn[m_slot_id]);
    }
}

bool RmcNetworkHandler::getLastValidPlmn(char* plmn, int size) {
    nsecs_t current_time = systemTime(SYSTEM_TIME_MONOTONIC);
    // logD(LOG_TAG, "getLastValidPlmn: %ld, %s",
    //         (current_time/10000000000), last_valid_plmn[m_slot_id]);
    if (plmn != NULL && size >= 8
            && last_valid_plmn[m_slot_id][0] != '\0'
            && strlen(last_valid_plmn[m_slot_id]) >= 5) {
        plmn[0] = '\0';
        if (current_time - last_valid_plmn_time[m_slot_id] < LIFE_VALID_PLMN) {
            strncpy(plmn, last_valid_plmn[m_slot_id], 7);
            plmn[7] = '\0';
            return true;
        }
    }
    return false;
}

bool RmcNetworkHandler::isRilTestMode() {
    char property_value[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get("persist.vendor.ril.test_mode", property_value, "0");
    return strcmp(property_value, "1") == 0;
}

bool RmcNetworkHandler::supportMccMncUrc() {
    bool support = false;
    if (isRilTestMode()) {
        support = true;
    }
    return support;
}

bool RmcNetworkHandler::isNrSupported() {
    /* check if nr is supported */
    char NR_VER[] = "5G_SUPPORTED";
    if (nr_support < 0) {
        nr_support = getFeatureVersion(NR_VER, 0);
    }
    return (nr_support == 1) ? true: false;
}


int RmcNetworkHandler::isGcfTestMode() {
    int isTestMode = 0;
    char prop[RFX_PROPERTY_VALUE_MAX] = {0};

    memset(prop,0,sizeof(prop));
    rfx_property_get("vendor.gsm.gcf.testmode", prop, "0");
    isTestMode = atoi(prop);
    return isTestMode;
}

int RmcNetworkHandler::isTestSim(int slotId) {
    int ret = 0;
    char prop[RFX_PROPERTY_VALUE_MAX] = {0};

    rfx_property_get(PROPERTY_RIL_TEST_SIM[slotId], prop, "0");
    ret = atoi(prop);
    return ret;
}

bool RmcNetworkHandler::isAPInCall() {
    bool ret = false;

    for (int slotId = RFX_SLOT_ID_0; slotId < RFX_SLOT_COUNT; slotId++) {
        if (getMclStatusManager(slotId)->getIntValue(RFX_STATUS_KEY_AP_VOICE_CALL_COUNT, 0) > 0) {
            ret = true;
        }
    }
    return ret;
}

void RmcNetworkHandler::refreshEcellCache() {
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

    response = (RIL_CellInfo_v12 *) alloca(num * sizeof(RIL_CellInfo_v12));
    memset(response, 0, num * sizeof(RIL_CellInfo_v12));

    err = getCellInfoListV12(line, num, response);
    freeOperatorNameOfCellInfo(num, response);
error:
    MD_ECELL *mMdEcell = mdEcell[m_slot_id];
    dumpEcellCache(__func__, mMdEcell);
    return;
}

void RmcNetworkHandler::freeOperatorNameOfCellInfo(int num, RIL_CellInfo_v12 *response) {
    if (response == NULL) return;
    for (int i = 0; i < num; i++) {
        switch (response[i].cellInfoType) {
            case RIL_CELL_INFO_TYPE_GSM: {
                if (response[i].CellInfo.gsm.cellIdentityGsm.operName.long_name)
                    free(response[i].CellInfo.gsm.cellIdentityGsm.operName.long_name);
                if (response[i].CellInfo.gsm.cellIdentityGsm.operName.short_name)
                    free(response[i].CellInfo.gsm.cellIdentityGsm.operName.short_name);
                break;
            }

            case RIL_CELL_INFO_TYPE_WCDMA: {
                if (response[i].CellInfo.wcdma.cellIdentityWcdma.operName.long_name)
                    free(response[i].CellInfo.wcdma.cellIdentityWcdma.operName.long_name);
                if (response[i].CellInfo.wcdma.cellIdentityWcdma.operName.short_name)
                    free(response[i].CellInfo.wcdma.cellIdentityWcdma.operName.short_name);
                break;
            }

            case RIL_CELL_INFO_TYPE_CDMA: {
                // It's NULL now.
                break;
            }

            case RIL_CELL_INFO_TYPE_LTE: {
                if (response[i].CellInfo.lte.cellIdentityLte.operName.long_name)
                    free(response[i].CellInfo.lte.cellIdentityLte.operName.long_name);
                if (response[i].CellInfo.lte.cellIdentityLte.operName.short_name)
                    free(response[i].CellInfo.lte.cellIdentityLte.operName.short_name);
                break;
            }

            case RIL_CELL_INFO_TYPE_TD_SCDMA: {
                if (response[i].CellInfo.tdscdma.cellIdentityTdscdma.operName.long_name)
                    free(response[i].CellInfo.tdscdma.cellIdentityTdscdma.operName.long_name);
                if (response[i].CellInfo.tdscdma.cellIdentityTdscdma.operName.short_name)
                    free(response[i].CellInfo.tdscdma.cellIdentityTdscdma.operName.short_name);
                break;
            }

            case RIL_CELL_INFO_TYPE_NR: {
                if (response[i].CellInfo.nr.cellidentity.operName.long_name)
                    free(response[i].CellInfo.nr.cellidentity.operName.long_name);
                if (response[i].CellInfo.nr.cellidentity.operName.short_name)
                    free(response[i].CellInfo.nr.cellidentity.operName.short_name);
                break;
            }
            case RIL_CELL_INFO_TYPE_NONE:
                break;
        }
    }
    return;
}

void RmcNetworkHandler::dumpEcellCache(const char* fun, MD_ECELL *mMdEcell) {
    char* ecell_cid_s = getMask(mMdEcell->cid);
    char* ecell_tac_s = getMask(mMdEcell->lacTac);
    char* ecell_pci_s = getMask(mMdEcell->pscPci);
    if (!ecell_cid_s || !ecell_tac_s || !ecell_pci_s) {
        logE(LOG_TAG, "[%s] can not getMask due to low memory", fun);
        goto error;
    }
    logD(LOG_TAG, "mMdEcell act=%d, cid=%s, mcc=%d, mnc=%d, lacTac=%s, pscPci=%s,"
        " sig1=%d, sig2=%d, rsrp=%d, rsrq=%d, timingAdvance=%d,"
        " rssnr=%d, cqi=%d, bsic=%d, arfcn=%d",
        mMdEcell->act, ecell_cid_s, mMdEcell->mcc, mMdEcell->mnc, ecell_tac_s,
        ecell_pci_s, mMdEcell->sig1, mMdEcell->sig2, mMdEcell->rsrp, mMdEcell->rsrq,
        mMdEcell->timingAdvance, mMdEcell->rssnr, mMdEcell->cqi, mMdEcell->bsic,
        mMdEcell->arfcn);
error:
    if (ecell_cid_s) free(ecell_cid_s);
    if (ecell_tac_s) free(ecell_tac_s);
    if (ecell_pci_s) free(ecell_pci_s);
}

void RmcNetworkHandler::dumpEregCache(const char* fun, MD_EREG *mMdEreg, MD_ECELL *mMdEcell) {
    dumpEregCache(fun, mMdEreg, mMdEcell, false);
}

void RmcNetworkHandler::dumpEregCache(const char* fun, MD_EREG *mMdEreg, MD_ECELL *mMdEcell, bool debugV) {
    char* lac_s = getMask(mMdEreg->lac);
    char* cid_s = getMask(mMdEreg->ci);
    char* ecell_cid_s = getMask((int) mMdEcell->cid);
    char* ecell_tac_s = getMask(mMdEcell->lacTac);
    char* ecell_pci_s = getMask(mMdEcell->pscPci);
    if (!lac_s || !cid_s || !ecell_cid_s || !ecell_tac_s || !ecell_pci_s) {
        logE(LOG_TAG, "[%s] can not getMask due to low memory", fun);
        goto error;
    }
    if (debugV) {
        logV(LOG_TAG, "[%s] mMdEreg n=%d, stat=%d, lac=%s, ci=%s, eAct=%d"
                ", nwk_existence=%d, roam_indicator=%d, cause_type=%d, reject_cause=%d"
                " mMdEcell act=%d, cid=%s, mcc=%d, mnc=%d, lacTac=%s, pscPci=%s,"
                " sig1=%d, sig2=%d, rsrp=%d, rsrq=%d, timingAdvance=%d,"
                " rssnr=%d, cqi=%d, bsic=%d, arfcn=%d",
                fun,
                mMdEreg->n, mMdEreg->stat, lac_s, cid_s, mMdEreg->eAct,
                mMdEreg->nwk_existence, mMdEreg->roam_indicator, mMdEreg->cause_type,
                mMdEreg->reject_cause,
                mMdEcell->act, ecell_cid_s, mMdEcell->mcc, mMdEcell->mnc, ecell_tac_s,
                ecell_pci_s, mMdEcell->sig1, mMdEcell->sig2, mMdEcell->rsrp, mMdEcell->rsrq,
                mMdEcell->timingAdvance, mMdEcell->rssnr, mMdEcell->cqi, mMdEcell->bsic,
                mMdEcell->arfcn);
    } else {
        logD(LOG_TAG, "[%s] mMdEreg n=%d, stat=%d, lac=%s, ci=%s, eAct=%d"
                ", nwk_existence=%d, roam_indicator=%d, cause_type=%d, reject_cause=%d"
                " mMdEcell act=%d, cid=%s, mcc=%d, mnc=%d, lacTac=%s, pscPci=%s,"
                " sig1=%d, sig2=%d, rsrp=%d, rsrq=%d, timingAdvance=%d,"
                " rssnr=%d, cqi=%d, bsic=%d, arfcn=%d",
                fun,
                mMdEreg->n, mMdEreg->stat, lac_s, cid_s, mMdEreg->eAct,
                mMdEreg->nwk_existence, mMdEreg->roam_indicator, mMdEreg->cause_type,
                mMdEreg->reject_cause,
                mMdEcell->act, ecell_cid_s, mMdEcell->mcc, mMdEcell->mnc, ecell_tac_s,
                ecell_pci_s, mMdEcell->sig1, mMdEcell->sig2, mMdEcell->rsrp, mMdEcell->rsrq,
                mMdEcell->timingAdvance, mMdEcell->rssnr, mMdEcell->cqi, mMdEcell->bsic,
                mMdEcell->arfcn);
    }
error:
    if (lac_s) free(lac_s);
    if (cid_s) free(cid_s);
    if (ecell_cid_s) free(ecell_cid_s);
    if (ecell_tac_s) free(ecell_tac_s);
    if (ecell_pci_s) free(ecell_pci_s);
}

void RmcNetworkHandler::dumpEgregCache(const char* fun, MD_EGREG *mMdEgreg, MD_ECELL *mMdEcell) {
     dumpEgregCache(fun, mMdEgreg, mMdEcell, false);
}

void RmcNetworkHandler::dumpEgregCache(const char* fun, MD_EGREG *mMdEgreg, MD_ECELL *mMdEcell, bool debugV) {
    char* lac_s = getMask(mMdEgreg->lac);
    char* ci_s = getMask(mMdEgreg->ci);
    char* ecell_cid_s = getMask((int) mMdEcell->cid);
    char* ecell_lac_s = getMask(mMdEcell->lacTac);
    char* ecell_pci_s = getMask(mMdEcell->pscPci);
    if (!lac_s || !ci_s || !ecell_cid_s || !ecell_lac_s || !ecell_pci_s) {
        logE(LOG_TAG, "[%s] can not getMask due to low memory", fun);
        goto error;
    }
    if (debugV) {
        logV(LOG_TAG, "[%s] mMdEgreg n=%d, stat=%d, lac=%s, ci=%s, eAct=%d, rac=%d"
                ", nwk_existence=%d, roam_indicator=%d, cause_type=%d,"
                " reject_cause=%d, dcnr_restricted=%d, endc_sib=%d, endc_available=%d.  "
                " mMdEcell act=%d, cid=%s, mcc=%d, mnc=%d, lacTac=%s, pscPci=%s,"
                " sig1=%d, sig2=%d, rsrp=%d, rsrq=%d, timingAdvance=%d,"
                " rssnr=%d, cqi=%d, bsic=%d, arfcn=%d",
                fun,
                mMdEgreg->n, mMdEgreg->stat, lac_s, ci_s, mMdEgreg->eAct,
                mMdEgreg->rac,mMdEgreg->nwk_existence, mMdEgreg->roam_indicator,
                mMdEgreg->cause_type, mMdEgreg->reject_cause, mMdEgreg->dcnr_restricted,
                mMdEgreg->endc_sib, mMdEgreg->endc_available,
                mMdEcell->act, ecell_cid_s, mMdEcell->mcc, mMdEcell->mnc, ecell_lac_s,
                ecell_pci_s, mMdEcell->sig1, mMdEcell->sig2, mMdEcell->rsrp, mMdEcell->rsrq,
                mMdEcell->timingAdvance, mMdEcell->rssnr, mMdEcell->cqi, mMdEcell->bsic,
                mMdEcell->arfcn);
    } else {
        logD(LOG_TAG, "[%s] mMdEgreg n=%d, stat=%d, lac=%s, ci=%s, eAct=%d, rac=%d"
                ", nwk_existence=%d, roam_indicator=%d, cause_type=%d,"
                " reject_cause=%d, dcnr_restricted=%d, endc_sib=%d, endc_available=%d.  "
                " mMdEcell act=%d, cid=%s, mcc=%d, mnc=%d, lacTac=%s, pscPci=%s,"
                " sig1=%d, sig2=%d, rsrp=%d, rsrq=%d, timingAdvance=%d,"
                " rssnr=%d, cqi=%d, bsic=%d, arfcn=%d",
                fun,
                mMdEgreg->n, mMdEgreg->stat, lac_s, ci_s, mMdEgreg->eAct,
                mMdEgreg->rac,mMdEgreg->nwk_existence, mMdEgreg->roam_indicator,
                mMdEgreg->cause_type, mMdEgreg->reject_cause, mMdEgreg->dcnr_restricted,
                mMdEgreg->endc_sib, mMdEgreg->endc_available,
                mMdEcell->act, ecell_cid_s, mMdEcell->mcc, mMdEcell->mnc, ecell_lac_s,
                ecell_pci_s, mMdEcell->sig1, mMdEcell->sig2, mMdEcell->rsrp, mMdEcell->rsrq,
                mMdEcell->timingAdvance, mMdEcell->rssnr, mMdEcell->cqi, mMdEcell->bsic,
                mMdEcell->arfcn);
    }
error:
    if (lac_s) free(lac_s);
    if (ci_s) free(ci_s);
    if (ecell_cid_s) free(ecell_cid_s);
    if (ecell_lac_s) free(ecell_lac_s);
    if (ecell_pci_s) free(ecell_pci_s);
}

// Have to call free() after getting the resp
char* RmcNetworkHandler::getMask(int i) {
    char* d = NULL;
    asprintf(&d, "%X", i);
    if (d == NULL) {
        logE(LOG_TAG, "getMask error");
        return NULL;
    }
    size_t size_ = strlen(d);
    if (size_ == 1 || size_ == 2) {
        d[0] = '*';
        return d;
    }
    for (int i = 0; i < (size_/2); i++) {
        d[i] = '*';
    }
    return d;
}
