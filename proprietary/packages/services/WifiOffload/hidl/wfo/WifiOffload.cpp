#include "WifiOffload.h"
#include "mal.h"
#include "jni_mal_api.h"

#include <cutils/properties.h>
#include <cutils/log.h>
#include <utils/Mutex.h>

extern JNIMalApi JNI_Mal_Api;

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "WifiOffload"

#ifndef UNUSED
#define UNUSED(param) (void)(param)
#endif

/**
 * Define the values of handover stages.
 */
#define HANDOVER_START  0
#define HANDOVER_END    1
#define HANDOVER_FAILED -1
#ifndef PROPERTY_VALUE_MAX
#define PROPERTY_VALUE_MAX 92
#endif
#ifdef NWMNGR_MAX_SIM
#define MAX_SIM_NUM NWMNGR_MAX_SIM
#else
#define MAX_SIM_NUM 4
#endif

namespace vendor {
namespace mediatek {
namespace hardware {
namespace wfo {
namespace V1_0 {
namespace implementation {

// Used to protect sp: mIWifiOffloadCallback
android::Mutex mLock;
sp<IWifiOffloadCallback> mIWifiOffloadCallback = NULL;
void *g_conn_ptr = NULL;
int sim_pdn_state[MAX_SIM_NUM] = {0};

int handover_callback(rds_ho_status_t *status);
void malReset_callback(void *arg);
int roveout_callback(rds_rvout_alert_t *pralert);
int pdnStateChanged_callback(rds_wifi_pdnact_t *state);
int pdnRanTypeChanged_callback(rds_rb_get_ran_type_rsp_t *pdnType);
int requestSetWiFiDisable_callback(rds_wifi_disi4_t *prDis);
int disconnectNotification_callback(rds_rb_get_last_err_rsp_t *pError);
int requestImsSwitch_callback(rds_ims_status_t *prstatus);
int rssiRequestCallback(rds_wifimon_config_t *prconfig);
int requestLocationInfo_callback(rds_get_location_t* location);
void checkReturnStatus(Return<void>& ret);
int isMultiPsAttachSupport();
int getMainCapabilityPhoneId();

// Methods from ::vendor::mediatek::hardware::wfo::V1_0::IWifiOffload follow.
Return<void> WifiOffload::setWifiOffloadCallback(const sp<IWifiOffloadCallback>& wifiOffloadCallback) {

    ALOGD("setWifiOffloadCallback()");

    ALOGD("call JNI_Mal_Api.init()");
    JNI_Mal_Api.init();

    {
        android::Mutex::Autolock autoLock(mLock);
        if (wifiOffloadCallback != NULL) {
            mIWifiOffloadCallback = wifiOffloadCallback;
            ALOGD("WifiOffload::setWifiOffloadCallback success!");
        } else {
            ALOGE("WifiOffload::setWifiOffloadCallback is NULL!");
        }
    }
    return Void();
}

Return<void> WifiOffload::nativeInit() {
    ALOGD("nativeInit()");
    JNI_Mal_Api.init();
    return Void();
}

Return<void> WifiOffload::nativeClose() {
    ALOGD("nativeClose()");
    if (g_conn_ptr != NULL) {
        ALOGD("release RDS connection pointer");
        epdgs_rds_conn_exit(g_conn_ptr);
        g_conn_ptr = NULL;
        memset(sim_pdn_state, 0, sizeof(sim_pdn_state));
    }
    JNI_Mal_Api.exit();
    return Void();
}

Return<int32_t> WifiOffload::nativeGetRatType(uint8_t simIdx) {
    ALOGD("nativeGetRatType()");
    if (g_conn_ptr == NULL) {
        ALOGE("get_rat_type conn_ptr is NULL");
        return RDS_RAN_UNSPEC;
    }
    if (simIdx < 0) {
        ALOGE("uint8_t conversion overflow! simIdx:%d", simIdx);
    }
    // ToDo: set ucsim_id from ImsService for rds req.
    rds_rb_get_demand_req_t req = {.ucsim_id = simIdx};
    // TODO: set req.u4pdn_id
    rds_rb_get_ran_type_rsp_t rsp =
        {.ucsim_id = simIdx, .i4rds_ret = RDS_FALSE, .u4_ifid = 0, .u4ran_type = RDS_RAN_UNSPEC};
    rds_int32 ret = rds_get_ran_type(&req, &rsp, g_conn_ptr);
    if (ret == RDS_FALSE) {
        rsp.u4ran_type = RDS_RAN_UNSPEC;
    }

    return rsp.u4ran_type;
}

Return<void> WifiOffload::nativeSetWosProfile(uint8_t simId, bool volteEnabled, bool wfcEnabled,
    bool vilteEnabled, const hidl_string& fqdn, bool wifiEnabled, uint8_t wfcMode,
    uint8_t dataRoaming_enabled, bool isAllowTurnOffIms) {
    ALOGD("nativeSetWosProfile()");

    if (g_conn_ptr == NULL) {
        ALOGE("get_rat_type conn_ptr is NULL");
        return Void();
    }
    if ((isMultiPsAttachSupport() == 0) && (simId != getMainCapabilityPhoneId())) {
        ALOGD("MPS not support, ignore settings for simdId:%d, MainCapabilityPhoneId:%d",
            unsigned(simId), getMainCapabilityPhoneId());

        return Void();
    }
    if (simId < 0 || wfcMode < 0 || dataRoaming_enabled < 0) {
        ALOGE("uint8_t conversion overflow! simIdx:%d, wfcMode:%d, dataRoaming_enabled:%d",
            simId, wfcMode, dataRoaming_enabled);

    }
    rds_ru_set_uiparam_req_t req = {
        .ucsim_id = simId,
        .fgimsolte_en = volteEnabled,
        .fgwfc_en = wfcEnabled,
        .omacp_fqdn = {0},
        .fgwifiui_en = wifiEnabled,
        .rdspolicy = wfcMode,
        .fgdata_roaming_ui_en = dataRoaming_enabled,
        .fgimsccp_en = vilteEnabled,
        .fgallow_turnoff_ims = isAllowTurnOffIms,
        .wifioff_reason = 0
    };
    if (!fqdn.empty()) {
        strncpy(req.omacp_fqdn, fqdn.c_str(), RDS_FQDN_LEN - 1);
    }
    rds_set_ui_param(&req, g_conn_ptr);
    return Void();
}

Return<void> WifiOffload::nativeSetWifiStatus(bool wifiConnected, const hidl_string& ifName,
    const hidl_string& ipv4, const hidl_string& ipv6, const hidl_string& mac) {
    ALOGD("nativeSetWifiStatus()");
    if (g_conn_ptr == NULL) {
        ALOGE("set_wifi_state conn_ptr is NULL");
        return Void();
    }
    rds_ru_set_wifistat_req_t req = {
        .fgwifi_en = wifiConnected,
        .szwifi_ifname = {0},
        .szwifi_ipv4addr = {0},
        .szwifi_ipv6addr = {0},
        .szwifi_macaddr = {0}
    };

    if (!ifName.empty()) {
        strncpy(req.szwifi_ifname, ifName.c_str(), RDS_STR_LEN - 1);
    }
    if (!ipv4.empty()) {
        strncpy(req.szwifi_ipv4addr, ipv4.c_str(), 2 * RDS_STR_LEN - 1);
    }
    if (!ipv6.empty()) {
        strncpy(req.szwifi_ipv6addr, ipv6.c_str(), 2 * RDS_STR_LEN - 1);
    }
    if (!mac.empty()) {
        strncpy(req.szwifi_macaddr, mac.c_str(), 2 * RDS_STR_LEN - 1);
    }
    rds_set_wifistat(&req, g_conn_ptr);
    return Void();
}

Return<void> WifiOffload::nativeSetCallState(uint8_t simIdx, int32_t callId, int32_t callType,
    int32_t callState) {
    // TODO implement
    ALOGD("nativeSetCallState()");
    if (g_conn_ptr == NULL) {
        ALOGE("set_call_state conn_ptr is NULL");
        return Void();
    }

    if (simIdx < 0) {
        ALOGE("uint8_t conversion overflow! simIdx: %d", simIdx);
    }
    rds_ru_set_callstat_req_t req = {
        .ucsim_id = simIdx,
        .call_stat = (wos_call_stat)((int)callState),
        .call_type = (wos_call_t)((int)callType),
        .call_id = callId,
    };
    rds_set_callstat(&req, g_conn_ptr);
    return Void();
}

Return<void> WifiOffload::nativeSetServiceState(uint8_t mdIdx, uint8_t simIdx, bool isDataRoaming,
    uint8_t ratType, uint8_t serviceState, const hidl_string& locatedPlmn) {
    ALOGD("nativeSetServiceState()");
    if (g_conn_ptr == NULL) {
        ALOGE("set_Service_state conn_ptr is NULL");
        return Void();
    }

    if (simIdx < 0 || mdIdx < 0 || ratType < 0 || serviceState < 0) {
        ALOGE("uint8_t overflow! simIdx: %d, mdIdx: %d, ratType: %d, serviceState: %d",
            simIdx, mdIdx, ratType, serviceState);
    }
    rds_ru_set_mdstat_req_t req = {
        .md_idx = mdIdx,
        .ucsim_id = simIdx,
        .fgisroaming = isDataRoaming,
        .md_rat = ratType,
        .srv_state = serviceState,
        .plmn_id = {0}
    };

    if (!locatedPlmn.empty()) {
        strncpy(req.plmn_id, locatedPlmn.c_str(), RDS_STR_LEN - 1);
    }

    rds_set_mdstat(&req, g_conn_ptr);
    return Void();
}

Return<void> WifiOffload::nativeSetVoiceState(uint8_t mdIdx, uint8_t simIdx,
            bool isVoiceRoaming, uint8_t voiceRatType, uint8_t voiceRegState) {
    ALOGD("nativeSetVoiceState()");

    if (g_conn_ptr == NULL) {
        ALOGE("set_voice_state conn_ptr is NULL");
        return Void();
    }

    if (simIdx < 0 || mdIdx < 0 || voiceRatType < 0 || voiceRegState < 0) {
        ALOGE("uint8_t overflow! simIdx: %d, mdIdx: %d, voiceRatType: %d, voiceRegState: %d",
            simIdx, mdIdx, voiceRatType, voiceRegState);
    }

    // voice registration state
    rds_ru_set_voice_mdstat_req_t req = {
        .md_idx = mdIdx,
        .ucsim_id = simIdx,
        .fgisroaming = isVoiceRoaming,
        .md_rat = voiceRatType,
        .srv_state = voiceRegState,
    };


    rds_if_req(RDS_USR_WOS, (char *)WOS_RDS_REQ_SET_VOICE_MDSTAT,
        (void *)&req, NULL, g_conn_ptr);
    return Void();
}


Return<void> WifiOffload::nativeSetSimInfo(uint8_t simId, const hidl_string& imei,
    const hidl_string& imsi, const hidl_string& mccMnc, const hidl_string& impi, int32_t simType,
    bool simReady, bool isMainSim) {
    ALOGD("nativeSetSimInfo()");
    if (g_conn_ptr == NULL) {
        ALOGE("setSimInfo conn_ptr is NULL");
        return Void();
    }

    if (simId < 0) {
        ALOGE("uint8_t conversion overflow! simId: %d", simId);
    }

    if (!simReady) {
        ALOGD("setSimInfo: notify SIM reject");
        mal_wo_sim_rejected_notify(mal_once(1, mal_cfg_type_sim_id, simId));
        return Void();
    }

    mal_wo_sim_info_t *simInfo = mal_wo_sim_info_alloc(mal_once(0), NULL);
    if (simInfo == NULL) {
        ALOGE("setSimInfo cannot allocate sim info structure");
        return Void();
    }

    if (!imei.empty()) {
        const char *pcImei = imei.c_str();
        mal_wo_sim_info_set_imei(mal_once(0), simInfo, pcImei, strlen(pcImei));
    }

    if (!imsi.empty()) {
        const char *pcImsi = imsi.c_str();
        mal_wo_sim_info_set_imsi(mal_once(0), simInfo, pcImsi, strlen(pcImsi));
    }

    if (!mccMnc.empty()) {
        const char *pcMccMnc = mccMnc.c_str();
        mal_wo_sim_info_set_operator(mal_once(0), simInfo, pcMccMnc, strlen(pcMccMnc));
    }

    if (!impi.empty()) {
        const char *pcImpi = impi.c_str();
        mal_wo_sim_info_set_impi(mal_once(0), simInfo, pcImpi, strlen(pcImpi));
    }

    mal_wo_sim_info_set_sim_type(mal_once(0), simInfo, (mal_wo_sim_type_t)((int)simType));
    mal_wo_sim_info_set_ps_capability(mal_once(0), simInfo, isMainSim);

    mal_wo_sim_ready_notify(mal_once(1, mal_cfg_type_sim_id, simId), simInfo);
    mal_wo_sim_info_free(mal_once(0), simInfo);

    return Void();
}

Return<void> WifiOffload::nativeSetWifiQuality(uint32_t rssi, uint32_t snr) {
    ALOGD("nativeSetWifiQuality()");
    if (g_conn_ptr == NULL) {
        ALOGE("setWifiQuality conn_ptr is NULL");
        return Void();
    }

    rds_ru_set_wifimon_req_t req = {
        .fgwifi_rssi = rssi,
        .fgwifi_snr = snr
    };
    rds_notify_wifimonitor(&req, g_conn_ptr);
    return Void();
}

Return<void> WifiOffload::nativeSetWfcSupported(uint8_t simId, int32_t isWfcSupported) {
    ALOGD("nativeSetWfcSupported()");
    if (simId < 0) {
        ALOGE("uint8_t conversion overflow! simId: %d", simId);
    }
    mal_nwmgnr_set_wfc_supported(mal_once(1, mal_cfg_type_sim_id, simId), isWfcSupported);
    return Void();
}

Return<void> WifiOffload::nativeSetRadioState(uint8_t simId, uint8_t mdIdx, uint8_t radioState) {
    ALOGD("nativeSetRadioState()");
    if (g_conn_ptr == NULL) {
        ALOGE("setSimInfo conn_ptr is NULL");
        return Void();
    }

    if (simId < 0 || mdIdx < 0) {
        ALOGE("uint8_t overflow! simId: %d, mdIdx: %d", simId, mdIdx);
    }

    rds_ru_set_radiostat_req_t req = {
        .md_idx = mdIdx,
        .ucsim_id = simId,
        .radio = radioState,
    };
    rds_set_radiostat(&req, g_conn_ptr);
    return Void();
}

Return<void> WifiOffload::nativeSetLocationInfo(uint8_t simId, const hidl_string& countryId) {
    ALOGD("nativeSetLocationInfo()");
    if (g_conn_ptr == NULL) {
        ALOGE("setLocationInfo conn_ptr is NULL");
        return Void();
    }

    if (simId < 0) {
        ALOGE("uint8_t conversion overflow! simId: %d", simId);
    }

    rds_set_location_t req = {
        .ucsim_id = simId,
        .country = {0}
    };
    if (!countryId.empty()) {
        strncpy(req.country, countryId.c_str(), RDS_COUNTRY_DATA_LEN - 1);
    }
    rds_set_location(&req);
    return Void();
}

Return<int32_t> WifiOffload::nativeConnectToMal() {
    ALOGD("nativeConnectToMal()");
    memset(sim_pdn_state, 0, sizeof(sim_pdn_state));
    if (g_conn_ptr != NULL) {
        epdgs_rds_conn_exit(g_conn_ptr);
        g_conn_ptr = NULL;
    }

    rds_notify_funp_t rds_fp = {
        .pfrds_ho_status_ind = handover_callback,
        .pfrds_rvout_alert_ind = roveout_callback,
        .pfrds_wifi_pdnact_ind = pdnStateChanged_callback,
        .pfrds_get_rantype_ind = pdnRanTypeChanged_callback,
        .pfrds_wifi_disable_ind = requestSetWiFiDisable_callback,
        .pfrds_get_lasterr_ind = disconnectNotification_callback,
        .pfrds_ims_status_ind = requestImsSwitch_callback,
        .pfrds_wifimon_cfg_ind = rssiRequestCallback,
        .pfrds_get_location_ind = requestLocationInfo_callback,
    };
    g_conn_ptr = epdgs_rds_conn_init(&rds_fp);
    if (g_conn_ptr == NULL) {
        ALOGD("RDS connection pointer is NULL");
        return 0;
    }

    // register reset callback.
    mal_status_reg_restart_cb(mal_once(0), malReset_callback, NULL);
    return 1;
    // return int32_t {};
}

int handover_callback(rds_ho_status_t *status) {


    ALOGD("handover_callback()");
    if (status == NULL) {
        ALOGE("[handover_callback] status is NULL");
        return 0;
    }
    ALOGD("handover_callback() fgho_result:%d, ucsim_id:%d, ucho_status:%d, etarget_ran_type:%d",
        status->fgho_result, status->ucsim_id, status->ucho_status, status->etarget_ran_type);

    uint8_t sim_id = isMultiPsAttachSupport() ? status->ucsim_id : getMainCapabilityPhoneId();

    {
        android::Mutex::Autolock autoLock(mLock);

        if (mIWifiOffloadCallback != NULL) {
            if (status->fgho_result == 0) {
                // HANDOVER_FAILED
                Return<void> retStatus = mIWifiOffloadCallback->onHandover(sim_id,
                        HANDOVER_FAILED, status->etarget_ran_type);
                checkReturnStatus(retStatus);
            } else {
                // HANDOVER_START(0) or HANDOVER_END(1)
                Return<void> retStatus = mIWifiOffloadCallback->onHandover(sim_id,
                        status->ucho_status, status->etarget_ran_type);
                checkReturnStatus(retStatus);
            }
        } else {
            ALOGD("mIWifiOffloadCallback is null!");
        }
    }
    return 0;
}

void malReset_callback(void *arg) {

    UNUSED(arg);
    ALOGD("malReset_callback()");

    {
        android::Mutex::Autolock autoLock(mLock);
        if (mIWifiOffloadCallback != NULL) {
            Return<void> retStatus = mIWifiOffloadCallback->onMalReset();
            checkReturnStatus(retStatus);
        } else {
            ALOGD("mIWifiOffloadCallback is null!");
        }
    }
}

int roveout_callback(rds_rvout_alert_t *pralert) {


    ALOGD("roveout_callback()");
    if (pralert == NULL) {
        ALOGE("[roveout_callback] status is NULL");
        return 0;
    }

    ALOGD("roveout_callback() ucsim_id: %d, rvalert_en: %d, i4wifirssi: %d",
            pralert->ucsim_id, pralert->rvalert_en, pralert->i4wifirssi);

    uint8_t sim_id = isMultiPsAttachSupport() ? pralert->ucsim_id : getMainCapabilityPhoneId();

    {
        android::Mutex::Autolock autoLock(mLock);
        if (mIWifiOffloadCallback != NULL) {
            Return<void> retStatus = mIWifiOffloadCallback->onRoveOut(sim_id,
                    pralert->rvalert_en, pralert->i4wifirssi);
            checkReturnStatus(retStatus);
        } else {
            ALOGD("mIWifiOffloadCallback is null!");
        }
    }
    return 0;
}

int pdnStateChanged_callback(rds_wifi_pdnact_t *state) {

    ALOGD("pdnStateChanged_callback()");
    if (state == NULL) {
        ALOGE("[pdnStateChanged_callback] status is NULL");
        return 0;
    }

    int tempPdnState = sim_pdn_state[state->ucsim_id];
    if (state->pdn_rdy) {
        sim_pdn_state[state->ucsim_id] |= 1 << state->ifid;
    } else {
        sim_pdn_state[state->ucsim_id] &= ~(1 << state->ifid);
    }
    ALOGD("[pdnStateChanged_callback] simId[%d] status: %d, ifid: %d, ifid_state: %d",
            unsigned(state->ucsim_id),
            sim_pdn_state[state->ucsim_id], state->ifid, state->pdn_rdy);

    bool isStateChange = (sim_pdn_state[state->ucsim_id] > 0) == !(tempPdnState > 0);
    if (!isStateChange) {
        ALOGD("[pdnStateChanged_callback] PDN state doesn't change, return directly");
        return 0;
    }

    uint8_t sim_id = isMultiPsAttachSupport() ? state->ucsim_id : getMainCapabilityPhoneId();

    {
        android::Mutex::Autolock autoLock(mLock);
        if (mIWifiOffloadCallback != NULL) {
            Return<void> retStatus =
                    mIWifiOffloadCallback->onPdnStateChanged(sim_id,
                    (sim_pdn_state[state->ucsim_id] > 0));
            checkReturnStatus(retStatus);
        } else {
            ALOGD("mIWifiOffloadCallback is null!");
        }
    }
    return 0;
}

int pdnRanTypeChanged_callback(rds_rb_get_ran_type_rsp_t *pdnType) {

    ALOGD("pdnRanTypeChanged_callback()");
    if (pdnType == NULL) {
        ALOGE("[pdnRanTypeChanged_callback] status is NULL");
        return 0;
    }

    ALOGD("pdnRanTypeChanged_callback(), simIdx: %d, interfaceId: %d, ranType: %d",
        pdnType->ucsim_id, pdnType->u4_ifid, pdnType->u4ran_type);

    uint8_t sim_id = isMultiPsAttachSupport() ? pdnType->ucsim_id : getMainCapabilityPhoneId();

    {
        android::Mutex::Autolock autoLock(mLock);
        if (mIWifiOffloadCallback != NULL) {
            Return<void> retStatus = mIWifiOffloadCallback->onPdnRanTypeChanged(sim_id,
                    pdnType->u4_ifid, pdnType->u4ran_type);
            checkReturnStatus(retStatus);
        } else {
            ALOGD("mIWifiOffloadCallback is null!");
        }
    }
    return 0;
}

int requestSetWiFiDisable_callback(rds_wifi_disi4_t *prDis) {


    ALOGD("requestSetWiFiDisable_callback()");
    if (prDis == NULL) {
        ALOGE("[notifyWiFiDisable_callback] status is NULL");
        return 0;
    }

    {
        android::Mutex::Autolock autoLock(mLock);
        if (mIWifiOffloadCallback != NULL) {
            Return<void> retStatus = mIWifiOffloadCallback->onRequestSetWifiDisabled(prDis->u4pdn_cnt);
            checkReturnStatus(retStatus);
        } else {
            ALOGD("mIWifiOffloadCallback is null!");
        }
    }
    return 0;
}

int disconnectNotification_callback(rds_rb_get_last_err_rsp_t *pError) {

    ALOGD("disconnectNotification_callback()");
    if (pError == NULL) {
        ALOGE("[disconnectNotification_callback] status is NULL");
        return 0;
    }

    ALOGD("disconnectNotification_callback() ucsim_id: %d, i4lasterr: %d, i4lastsuberr: %d",
        pError->ucsim_id, pError->i4lasterr, pError->i4lastsuberr);

    uint8_t sim_id = isMultiPsAttachSupport() ? pError->ucsim_id : getMainCapabilityPhoneId();

    {
        android::Mutex::Autolock autoLock(mLock);
        if (mIWifiOffloadCallback != NULL) {
            ALOGD("[disconnectNotification_callback] lasterr rat = %d", pError->u4lastran);
            Return<void> retStatus = mIWifiOffloadCallback->onDisconnectCauseNotify(sim_id,
                    pError->i4lasterr, pError->i4lastsuberr);
            checkReturnStatus(retStatus);
        } else {
            ALOGD("mIWifiOffloadCallback is null!");
        }
    }
    return 0;
}

int requestImsSwitch_callback(rds_ims_status_t *prstatus) {

    ALOGD("requestImsSwitch_callback()");
    if (prstatus == NULL) {
        ALOGE("[requestImsSwitch_callback] prstatus is NULL");
        return 0;
    }

    ALOGD("requestImsSwitch_callback() sim_idx: %d, fgims_en: %d",
        prstatus->sim_idx, prstatus->fgims_en);

    uint8_t sim_id = isMultiPsAttachSupport() ? prstatus->sim_idx : getMainCapabilityPhoneId();

    {
        android::Mutex::Autolock autoLock(mLock);
        if (mIWifiOffloadCallback != NULL) {
            Return<void> retStatus =
                    mIWifiOffloadCallback->onRequestImsSwitch(sim_id, prstatus->fgims_en);
            checkReturnStatus(retStatus);
        } else {
            ALOGD("mIWifiOffloadCallback is null!");
        }
    }
    return 0;
}

int rssiRequestCallback(rds_wifimon_config_t *prconfig) {

    ALOGD("rssiRequestCallback()");
    if (prconfig == NULL) {
        ALOGE("[rssiRequestCallback] prconfig is NULL");
        return 0;
    }

    int simId = prconfig->sim_id;
    int *list = prconfig->rssi_thrd_list;
    int size = prconfig->rssi_num;
    hidl_vec<int32_t> tempBuf;

    ALOGD("rssiRequestCallback() simId: %d, size: %d", simId, size);

    tempBuf.resize(size);
    for (int i = 0; i < size; i++) {
        tempBuf[i] = (int32_t)list[i];
    }

    uint8_t sim_id = isMultiPsAttachSupport() ? simId : getMainCapabilityPhoneId();

    {
        android::Mutex::Autolock autoLock(mLock);
        if (mIWifiOffloadCallback != NULL) {
            Return<void> retStatus = mIWifiOffloadCallback->onRssiMonitorRequest(sim_id, size, tempBuf);
            checkReturnStatus(retStatus);
        } else {
            ALOGD("mIWifiOffloadCallback is null!");
        }
    }
    return 0;
}

int requestLocationInfo_callback(rds_get_location_t* location) {

    ALOGD("requestLocationInfo_callback()");
    if (location == NULL) {
        ALOGE("[requestLocationInfo_callback] location is NULL");
        return 0;
    }


    ALOGD("requestLocationInfo_callback() ucsim_id:%d", location->ucsim_id);
    uint8_t sim_id = isMultiPsAttachSupport() ? location->ucsim_id : getMainCapabilityPhoneId();

    {
        android::Mutex::Autolock autoLock(mLock);
        if (mIWifiOffloadCallback != NULL) {
            Return<void> retStatus = mIWifiOffloadCallback->onRequestLocationInfo(sim_id);
            checkReturnStatus(retStatus);
        } else {
            ALOGD("mIWifiOffloadCallback is null!");
        }
    }
    return 0;
}

void checkReturnStatus(Return<void>& ret) {


    if (ret.isOk() == false) {
        ALOGE("checkReturnStatus: unable to call response/indication callback");
        mIWifiOffloadCallback = NULL;
    }
}

int isMultiPsAttachSupport() {
    static int support = -1;
    if (support == -1) {
        char value[PROPERTY_VALUE_MAX] = {0};
        property_get("ro.vendor.mtk_data_config", value, "0");

        if ((atoi(value) & 1) == 1) {
            support = 1;
        } else {
            support = 0;
        }
    }
    return support;
}

int getMainCapabilityPhoneId() {
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("persist.vendor.radio.simswitch", property_value, "0");
    return (atoi(property_value) - 1);
}

// Methods from ::android::hidl::base::V1_0::IBase follow.

IWifiOffload* HIDL_FETCH_IWifiOffload(const char* /* name */) {
    return new WifiOffload();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace wfo
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
