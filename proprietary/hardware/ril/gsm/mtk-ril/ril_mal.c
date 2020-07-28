/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

#include <telephony/mtk_ril.h>
#include <libmtkrilutils.h>
#include "atchannels.h"
#include <ril_callbacks.h>
#include <mal.h>
#include "ril_mal.h"
#include <dlfcn.h>

#ifdef LOG_TAG /* in log.h */
#undef LOG_TAG
#endif

#define LOG_TAG "RIL-MAL"


const struct timeval TIMEVAL_RDS_CONN_RETRY = {0, 50000};
int requestedTlvMemoryOfMalApi[MALRIL_API_INDEX_MAX];
void *dlopenHandlerForMalApi;

void registerMalRestartCallbacks() {
    if (mal_datamngr_get_api_tlv_buff_len != NULL) {
        mal_datamngr_get_api_tlv_buff_len(mal_once(0), sizeof(requestedTlvMemoryOfMalApi), requestedTlvMemoryOfMalApi);
        RLOGD("[%s] Call mal_datamngr_get_api_tlv_buff_len success", __FUNCTION__);
        for (int i = 0; i < MALRIL_API_INDEX_MAX; ++i) {
            RLOGD("requestedTlvMemoryOfMalApi[%d] is %d", i, requestedTlvMemoryOfMalApi[i]);
        }
    } else {
        RLOGE("[%s] mal_datamngr_get_api_tlv_buff_len is null", __FUNCTION__);
    }
    if (!isEpdgSupport()) return;
    RLOGD("registerMalRestartCallbacks()");
    // register for ril-mal restart callback
    if (mal_status_reg_restart_cb != NULL &&
        mal_set != NULL && mal_reset != NULL) {

        mal_status_reg_restart_cb(mal_once(0), rilMalRestartCallback, NULL);

    } else {
        RLOGD("ERROR !! MAL API is NULL !! %d %d %d",
              (mal_status_reg_restart_cb == NULL) ? 0 : 1,
              (mal_set == NULL) ? 0 : 1, (mal_reset == NULL) ? 0 : 1);
    }

    // register for rds socket handle
    if (rild_rds_conn_init != NULL) {
        g_rds_conn_ptr = rild_rds_conn_init(NULL);
        RLOGD("[%s] Call rild_rds_conn_init success", __FUNCTION__);
    } else {
        RLOGE("[%s] rild_rds_conn_init is null", __FUNCTION__);
    }
    if (g_rds_conn_ptr != NULL) {
        RLOGD("rds socket init success");
    }
}

void rilMalRestartCallback(void *arg) {
    // Fix build warning
    (void *)arg;

    clearRilDataMalCache();
    clearRilNWMalCache();
    clearRdsSocketHandle();

    // re-register
    registerMalRestartCallbacks();
    if (g_rds_conn_ptr == NULL) {
        initRdsConnection(NULL);
    }
}

void initRdsConnection(void *param) {
    // Fix build warning
    (void *)param;

    g_rds_conn_ptr = rild_rds_conn_init(NULL);
    if (g_rds_conn_ptr == NULL) {
        // RLOGD("retry mal socket connection");
        RIL_requestTimedCallback(initRdsConnection, NULL, &TIMEVAL_RDS_CONN_RETRY);
    } else {
        RLOGD("rds socket init success");
    }
}

void *getDlopenHandler(const char *libPath) {
    void *dlHandler;
    dlHandler = dlopen(libPath, RTLD_NOW);
    if (dlHandler == NULL) {
        LOGE("dlopen failed: %s", dlerror());
    }
    return dlHandler;
}


void *getAPIbyDlopenHandler(void *dlHandler, const char *apiName){
    if (dlHandler == NULL) {
        LOGE("dlopen Handler is null");
        return NULL;
    }
    void *outputFun;
    outputFun = dlsym(dlHandler, apiName);
    if (outputFun == NULL) {
        LOGE("%s is not defined or exported: %s", apiName, dlerror());
    } else {
        LOGD("Dlopen api success: %s", apiName);
    }
    return outputFun;
}


void initMalApi() {
    LOGD("[%s] Dlopen mal APIs start",__FUNCTION__);
    dlopenHandlerForMalApi = getDlopenHandler("libmal.so");
    if (dlopenHandlerForMalApi != NULL) {
        mal_status_reg_restart_cb = (int (*)(mal_ptr_t mal_ptr, mal_status_cb_t cb, void *arg))getAPIbyDlopenHandler(dlopenHandlerForMalApi, "mal_status_reg_restart_cb");
        mal_datamngr_get_ims_info = (int (*)(mal_ptr_t mal_ptr, mal_datamngr_ims_info_req_ptr_t req_ptr, mal_datamngr_ims_info_rsp_ptr_t rsp_ptr))getAPIbyDlopenHandler(dlopenHandlerForMalApi, "mal_datamngr_get_ims_info");
        mal_datamngr_set_data_call_info = (int (*)(mal_ptr_t mal_ptr, mal_datamngr_data_call_info_req_ptr_t req_ptr, mal_datamngr_data_call_info_rsp_ptr_t rsp_ptr))getAPIbyDlopenHandler(dlopenHandlerForMalApi, "mal_datamngr_set_data_call_info");
        mal_mdmngr_send_urc = (int (*)(mal_ptr_t mal_ptr, const char *urc))getAPIbyDlopenHandler(dlopenHandlerForMalApi, "mal_mdmngr_send_urc");
        mal_set = (mal_ptr_t (*)(mal_ptr_t mal_ptr, unsigned int num_of_cfg, ...))getAPIbyDlopenHandler(dlopenHandlerForMalApi, "mal_set");
        mal_reset = (mal_ptr_t (*)(mal_ptr_t mal_ptr))getAPIbyDlopenHandler(dlopenHandlerForMalApi, "mal_reset");

        mal_datamngr_get_api_tlv_buff_len = (int (*)(mal_ptr_t mal_ptr, int size, int *tlv_buff_len))getAPIbyDlopenHandler(dlopenHandlerForMalApi, "mal_datamngr_get_api_tlv_buff_len");
        mal_datamngr_get_ims_info_tlv = (int (*)(mal_ptr_t mal_ptr, mal_datamngr_ims_info_req_ptr_t req_ptr, mal_datamngr_ims_info_rsp_ptr_t rsp_ptr,
                int num_req_tlv, void *req_tlv, int *num_rsp_tlv, void *rsp_tlv))getAPIbyDlopenHandler(dlopenHandlerForMalApi, "mal_datamngr_get_ims_info_tlv");
        mal_datamngr_set_data_call_info_tlv = (int (*)(mal_ptr_t mal_ptr, mal_datamngr_data_call_info_req_ptr_t req_ptr, mal_datamngr_data_call_info_rsp_ptr_t rsp_ptr,
                int num_req_tlv, void *req_tlv, int *num_rsp_tlv, void *rsp_tlv))getAPIbyDlopenHandler(dlopenHandlerForMalApi, "mal_datamngr_set_data_call_info_tlv");
        mal_datamngr_notify_data_call_list_tlv = (int (*)(mal_ptr_t mal_ptr, int num_pdn, mal_datamngr_data_call_info_req_ptr_t req_ptr[], mal_datamngr_data_call_info_rsp_ptr_t rsp_ptr[],
                int num_req_tlv[], void *req_tlv[], int *num_rsp_tlv[], void *rsp_tlv[]))getAPIbyDlopenHandler(dlopenHandlerForMalApi, "mal_datamngr_notify_data_call_list_tlv");
        mal_datamngr_notify_dedicated_bearer_act_tlv = (int (*)(mal_ptr_t mal_ptr, MAL_Dedicate_Data_Call_Struct *req_ptr, mal_datamngr_notify_dedicated_bearer_act_rsp_ptr_t rsp_ptr,
                int num_req_tlv, void *req_tlv, int *num_rsp_tlv, void *rsp_tlv))getAPIbyDlopenHandler(dlopenHandlerForMalApi, "mal_datamngr_notify_dedicated_bearer_act_tlv");
        mal_datamngr_notify_dedicated_bearer_modify_tlv = (int (*)(mal_ptr_t mal_ptr, MAL_Dedicate_Data_Call_Struct *req_ptr, mal_datamngr_notify_dedicated_bearer_modify_rsp_ptr_t rsp_ptr,
                int num_req_tlv, void *req_tlv, int *num_rsp_tlv, void *rsp_tlv))getAPIbyDlopenHandler(dlopenHandlerForMalApi, "mal_datamngr_notify_dedicated_bearer_modify_tlv");
        mal_datamngr_notify_dedicated_bearer_deact_tlv = (int (*)(mal_ptr_t mal_ptr, int cid, mal_datamngr_notify_dedicated_bearer_deact_rsp_ptr_t rsp_ptr,
                int num_req_tlv, void *req_tlv, int *num_rsp_tlv, void *rsp_tlv))getAPIbyDlopenHandler(dlopenHandlerForMalApi, "mal_datamngr_notify_dedicated_bearer_deact_tlv");
        mal_rilproxy_resp_data_call_info_tlv = (int (*)(mal_ptr_t mal_ptr, int serial_no, mal_datamngr_data_call_info_req_ptr_t req_ptr, mal_datamngr_data_call_info_rsp_ptr_t rsp_ptr,
                int num_req_tlv, void *req_tlv, int *num_rsp_tlv, void *rsp_tlv))getAPIbyDlopenHandler(dlopenHandlerForMalApi, "mal_rilproxy_resp_data_call_info_tlv");
        mal_rilproxy_resp_pcscf_pco_tlv = (int (*)(mal_ptr_t mal_ptr, int serial_no, char *pcscf_str, mal_rilproxy_pcscf_pco_rsp_ptr_t rsp_ptr,
                int num_req_tlv, void *req_tlv, int *num_rsp_tlv, void *rsp_tlv))getAPIbyDlopenHandler(dlopenHandlerForMalApi, "mal_rilproxy_resp_pcscf_pco_tlv");
        mal_rilproxy_resp_last_fail_cause_tlv = (int (*)(mal_ptr_t mal_ptr, int serial_no, int fail_cause, mal_rilproxy_last_fail_cause_rsp_ptr_t rsp_ptr,
                int num_req_tlv, void *req_tlv, int *num_rsp_tlv, void *rsp_tlv))getAPIbyDlopenHandler(dlopenHandlerForMalApi, "mal_rilproxy_resp_last_fail_cause_tlv");

        rild_rds_conn_init = (void *(*)(rds_notify_funp_t *callback_fn))getAPIbyDlopenHandler(dlopenHandlerForMalApi, "rild_rds_conn_init");
        rild_rds_conn_exit = (rds_int32 (*)(void * conn_ptr))getAPIbyDlopenHandler(dlopenHandlerForMalApi, "rild_rds_conn_exit");
        rild_rds_sdc_req = (rds_int32(*)(dm_req_setup_data_call_t *req, dm_resp_setup_data_call_t *cnf))getAPIbyDlopenHandler(dlopenHandlerForMalApi, "rild_rds_sdc_req");
        rild_rds_ddc_req = (rds_int32(*)(dm_req_t *req, rr_ddc_cnf_t *cnf))getAPIbyDlopenHandler(dlopenHandlerForMalApi, "rild_rds_ddc_req");
        rds_rild_get_ho_status = (rds_int32(*)(rds_rb_get_ho_status_t *rsp, rds_rb_get_ho_status_req_t *req, void * conn_ptr))getAPIbyDlopenHandler(dlopenHandlerForMalApi, "rds_rild_get_ho_status");
        rild_rds_set_apncfg = (rds_int32(*)(rds_apn_cfg_req_t *req))getAPIbyDlopenHandler(dlopenHandlerForMalApi, "rild_rds_set_apncfg");
        rds_if_req = (rds_int32 (*)(rds_uint32 u4user, char *szcmd, void *prinput, void *proutput, void * conn_ptr))getAPIbyDlopenHandler(dlopenHandlerForMalApi, "rds_if_req");
        LOGD("[%s] Dlopen mal APIs success",__FUNCTION__);
    } else {
        LOGE("[%s] Dlopen mal APIs failed: dlopenHandlerForMalApi is null",__FUNCTION__);
    }
}