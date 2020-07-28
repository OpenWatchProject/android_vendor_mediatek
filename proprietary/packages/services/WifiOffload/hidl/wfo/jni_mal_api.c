#include <utils/Log.h>
#include <dlfcn.h>
#include "jni_mal_api.h"

#ifndef LOG_TAG
#define LOG_TAG "WifiOffloadService:jni_mal_api"
#endif
#ifndef LOG_NDEBUG
#define LOG_NDEBUG 0
#endif
#ifndef UNUSED
#define UNUSED(param) (void)(param)
#endif

#define MAL_LIB_NAME "libmal.so"

#define EPDGS_RDS_CONN_INIT "epdgs_rds_conn_init"
#define EPDGS_RDS_CONN_EXIT "epdgs_rds_conn_exit"
#define MAL_RESET           "mal_reset"
#define MAL_SET             "mal_set"
#define MAL_STATUS_REG_RESTART_CB "mal_status_reg_restart_cb"
#define MAL_NWMGNR_SET_WFC_SUPPORTED "mal_nwmgnr_set_wfc_supported"
#define MAL_WO_SIM_INFO_FREE "mal_wo_sim_info_free"
#define MAL_WO_SIM_INFO_SET_IMSI "mal_wo_sim_info_set_imsi"
#define MAL_WO_SIM_INFO_SET_IMEI "mal_wo_sim_info_set_imei"
#define MAL_WO_SIM_INFO_SET_IMPI "mal_wo_sim_info_set_impi"
#define MAL_WO_SIM_INFO_SET_OPERATOR "mal_wo_sim_info_set_operator"
#define MAL_WO_SIM_INFO_SET_PS_CAPABILITY "mal_wo_sim_info_set_ps_capability"
#define MAL_WO_SIM_INFO_SET_SIM_TYPE "mal_wo_sim_info_set_sim_type"
#define MAL_WO_SIM_READY_NOTIFY "mal_wo_sim_ready_notify"
#define MAL_WO_SIM_REJECTED_NOTIFY "mal_wo_sim_rejected_notify"
#define MAL_WO_SIM_INFO_ALLOC "mal_wo_sim_info_alloc"
#define RDS_GET_LAST_ERR "rds_get_last_err"
#define RDS_GET_RAN_TYPE "rds_get_ran_type"
#define RDS_NOTIFY_WIFIMONITOR "rds_notify_wifimonitor"
#define RDS_SET_CALLSTAT "rds_set_callstat"
#define RDS_SET_LOCATION "rds_set_location"
#define RDS_SET_MDSTAT "rds_set_mdstat"
#define RDS_SET_RADIOSTAT "rds_set_radiostat"
#define RDS_SET_UI_PARAM "rds_set_ui_param"
#define RDS_SET_WIFISTAT "rds_set_wifistat"
#define RDS_IF_REQ "rds_if_req"

typedef void * (* FP_EPDGS_RDS_CONN_INIT)(rds_notify_funp_t *callback_fn);
typedef rds_int32 (* FP_EPDGS_RDS_CONN_EXIT)(void * conn_ptr);
typedef mal_ptr_t (* FP_MAL_RESET)(mal_ptr_t mal_ptr);
typedef mal_ptr_t (* FP_MAL_SET)(mal_ptr_t mal_ptr, unsigned int num_of_cfg, ...);
typedef int (* FP_MAL_STATUS_REG_RESTART_CB)(mal_ptr_t mal_ptr, mal_status_cb_t cb, void *arg);
typedef int (* FP_MAL_NWMGNR_SET_WFC_SUPPORTED)(mal_ptr_t mal_ptr, int is_wfc_supported);
typedef void (* FP_MAL_WO_SIM_INFO_FREE)(mal_ptr_t mal_ptr, mal_wo_sim_info_t* sim_info);
typedef void (* FP_MAL_WO_SIM_INFO_SET_IMSI)(mal_ptr_t mal_ptr, mal_wo_sim_info_t *sim_info,
    const char *imsi, size_t str_len);
typedef void (* FP_MAL_WO_SIM_INFO_SET_IMEI)(mal_ptr_t mal_ptr, mal_wo_sim_info_t *sim_info,
    const char *imei, size_t str_len);
typedef void (* FP_MAL_WO_SIM_INFO_SET_IMPI)(mal_ptr_t mal_ptr, mal_wo_sim_info_t *sim_info,
    const char *impi, size_t str_len);
typedef void (* FP_MAL_WO_SIM_INFO_SET_OPERATOR)(mal_ptr_t mal_ptr, mal_wo_sim_info_t *sim_info,
    const char *op, size_t str_len);
typedef void (* FP_MAL_WO_SIM_INFO_SET_PS_CAPABILITY)(mal_ptr_t mal_ptr,
    mal_wo_sim_info_t *sim_info, bool has_ps_capability);
typedef void (* FP_MAL_WO_SIM_INFO_SET_SIM_TYPE)(mal_ptr_t mal_ptr, mal_wo_sim_info_t *sim_info,
    mal_wo_sim_type_t sim_type);
typedef bool (* FP_MAL_WO_SIM_READY_NOTIFY)(mal_ptr_t mal_ptr, mal_wo_sim_info_t *sim_info);
typedef bool (* FP_MAL_WO_SIM_REJECTED_NOTIFY)(mal_ptr_t mal_ptr);
typedef mal_wo_sim_info_t* (* FP_MAL_WO_SIM_INFO_ALLOC)(mal_ptr_t mal_ptr,
    mal_wo_sim_info_t* sim_info);
typedef rds_int32 (* FP_RDS_GET_LAST_ERR)(rds_rb_get_demand_req_t *req,
    rds_rb_get_last_err_rsp_t *rsp, void * conn_ptr);
typedef rds_int32 (* FP_RDS_GET_RAN_TYPE)(rds_rb_get_demand_req_t *req,
    rds_rb_get_ran_type_rsp_t *rsp, void * conn_ptr);
typedef rds_int32 (* FP_RDS_NOTIFY_WIFIMONITOR)(rds_ru_set_wifimon_req_t *req, void * conn_ptr);
typedef rds_int32 (* FP_RDS_SET_CALLSTAT)(rds_ru_set_callstat_req_t *req, void * conn_ptr);
typedef rds_int32 (* FP_RDS_SET_LOCATION)(rds_set_location_t *loc);
typedef rds_int32 (* FP_RDS_SET_MDSTAT)(rds_ru_set_mdstat_req_t *req, void * conn_ptr);
typedef rds_int32 (* FP_RDS_SET_RADIOSTAT)(rds_ru_set_radiostat_req_t *req, void * conn_ptr);
typedef rds_int32 (* FP_RDS_SET_UI_PARAM)(rds_ru_set_uiparam_req_t *req, void * conn_ptr);
typedef rds_int32 (* FP_RDS_SET_WIFISTAT)(rds_ru_set_wifistat_req_t *req, void * conn_ptr);
typedef rds_int32 (* FP_RDS_IF_REQ)(rds_uint32 u4user, char *szcmd, void *prinput, void *proutput, void * conn_ptr);

static void * jni_mal_epdgs_rds_conn_init(rds_notify_funp_t *callback_fn);
static rds_int32 jni_mal_epdgs_rds_conn_exit(void * conn_ptr);
static void jni_mal_api_init();
static void jni_mal_api_exit();
static mal_ptr_t jni_mal_mal_reset(mal_ptr_t mal_ptr);
static mal_ptr_t jni_mal_mal_set(mal_ptr_t mal_ptr, unsigned int num_of_cfg, ...);
static int jni_mal_mal_status_reg_restart_cb(mal_ptr_t mal_ptr, mal_status_cb_t cb, void *arg);
static int jni_mal_mal_nwmgnr_set_wfc_supported(mal_ptr_t mal_ptr, int is_wfc_supported);
static void jni_mal_mal_wo_sim_info_free(mal_ptr_t mal_ptr, mal_wo_sim_info_t* sim_info);
static void jni_mal_mal_wo_sim_info_set_imsi(mal_ptr_t mal_ptr, mal_wo_sim_info_t *sim_info,
    const char *imsi, size_t str_len);
static void jni_mal_mal_wo_sim_info_set_imei(mal_ptr_t mal_ptr, mal_wo_sim_info_t *sim_info,
    const char *imei, size_t str_len);
static void jni_mal_mal_wo_sim_info_set_impi(mal_ptr_t mal_ptr, mal_wo_sim_info_t *sim_info,
    const char *impi, size_t str_len);
static void jni_mal_mal_wo_sim_info_set_operator(mal_ptr_t mal_ptr, mal_wo_sim_info_t *sim_info,
    const char *op, size_t str_len);
static void jni_mal_mal_wo_sim_info_set_ps_capability(mal_ptr_t mal_ptr,
    mal_wo_sim_info_t *sim_info, bool has_ps_capability);
static void jni_mal_mal_wo_sim_info_set_sim_type(mal_ptr_t mal_ptr, mal_wo_sim_info_t *sim_info,
    mal_wo_sim_type_t sim_type);
static bool jni_mal_mal_wo_sim_ready_notify(mal_ptr_t mal_ptr, mal_wo_sim_info_t *sim_info);
static bool jni_mal_mal_wo_sim_rejected_notify(mal_ptr_t mal_ptr);
static mal_wo_sim_info_t* jni_mal_mal_wo_sim_info_alloc(mal_ptr_t mal_ptr,
    mal_wo_sim_info_t* sim_info);
static rds_int32 jni_mal_rds_get_last_err(rds_rb_get_demand_req_t *req,
    rds_rb_get_last_err_rsp_t *rsp, void * conn_ptr);
static rds_int32 jni_mal_rds_get_ran_type(rds_rb_get_demand_req_t *req,
    rds_rb_get_ran_type_rsp_t *rsp, void * conn_ptr);
static rds_int32 jni_mal_rds_notify_wifimonitor(rds_ru_set_wifimon_req_t *req, void * conn_ptr);
static rds_int32 jni_mal_rds_set_callstat(rds_ru_set_callstat_req_t *req, void * conn_ptr);
static rds_int32 jni_mal_rds_set_location(rds_set_location_t *loc);
static rds_int32 jni_mal_rds_set_mdstat(rds_ru_set_mdstat_req_t *req, void * conn_ptr);
static rds_int32 jni_mal_rds_set_radiostat(rds_ru_set_radiostat_req_t *req, void * conn_ptr);
static rds_int32 jni_mal_rds_set_ui_param(rds_ru_set_uiparam_req_t *req, void * conn_ptr);
static rds_int32 jni_mal_rds_set_wifistat(rds_ru_set_wifistat_req_t *req, void * conn_ptr);

JNIMalApi JNI_Mal_Api = {
    .libmal_version                     = "1.0.0",
    .init                               = jni_mal_api_init,
    .exit                               = jni_mal_api_exit,
};

static void * jni_mal_epdgs_rds_conn_init(rds_notify_funp_t *callback_fn) {
    UNUSED(callback_fn);
    ALOGW("libmal API not found!(%s)", __func__);
    return NULL;
}

static rds_int32 jni_mal_epdgs_rds_conn_exit(void * conn_ptr) {
    UNUSED(conn_ptr);
    rds_int32 ret = 0;
    ALOGW("libmal API not found!(%s)", __func__);
    return ret;
}

static mal_ptr_t jni_mal_mal_reset(mal_ptr_t mal_ptr) {
    UNUSED(mal_ptr);
    ALOGW("libmal API not found!(%s)", __func__);
    return NULL;
}

static mal_ptr_t jni_mal_mal_set(mal_ptr_t mal_ptr, unsigned int num_of_cfg, ...) {
    UNUSED(mal_ptr);
    UNUSED(num_of_cfg);
    ALOGW("libmal API not found!(%s)", __func__);
    return NULL;
}

static int jni_mal_mal_status_reg_restart_cb(mal_ptr_t mal_ptr, mal_status_cb_t cb, void *arg) {
    UNUSED(mal_ptr);
    UNUSED(cb);
    UNUSED(arg);
    ALOGW("libmal API not found!(%s)", __func__);
    return 0;
}

static int jni_mal_mal_nwmgnr_set_wfc_supported(mal_ptr_t mal_ptr, int is_wfc_supported) {
    UNUSED(mal_ptr);
    UNUSED(is_wfc_supported);
    ALOGW("libmal API not found!(%s)", __func__);
    return 0;
}

static void jni_mal_mal_wo_sim_info_free(mal_ptr_t mal_ptr, mal_wo_sim_info_t* sim_info) {
    UNUSED(mal_ptr);
    UNUSED(sim_info);
    ALOGW("libmal API not found!(%s)", __func__);
    return;
}

static void jni_mal_mal_wo_sim_info_set_imsi(mal_ptr_t mal_ptr, mal_wo_sim_info_t *sim_info,
        const char *imsi, size_t str_len) {
    UNUSED(mal_ptr);
    UNUSED(sim_info);
    UNUSED(imsi);
    UNUSED(str_len);
    ALOGW("libmal API not found!(%s)", __func__);
    return;
}

static void jni_mal_mal_wo_sim_info_set_imei(mal_ptr_t mal_ptr, mal_wo_sim_info_t *sim_info,
        const char *imei, size_t str_len) {
    UNUSED(mal_ptr);
    UNUSED(sim_info);
    UNUSED(imei);
    UNUSED(str_len);
    ALOGW("libmal API not found!(%s)", __func__);
    return;
}

static void jni_mal_mal_wo_sim_info_set_impi(mal_ptr_t mal_ptr, mal_wo_sim_info_t *sim_info,
        const char *impi, size_t str_len) {
    UNUSED(mal_ptr);
    UNUSED(sim_info);
    UNUSED(impi);
    UNUSED(str_len);
    ALOGW("libmal API not found!(%s)", __func__);
    return;
}

static void jni_mal_mal_wo_sim_info_set_operator(mal_ptr_t mal_ptr, mal_wo_sim_info_t *sim_info,
        const char *op, size_t str_len) {
    UNUSED(mal_ptr);
    UNUSED(sim_info);
    UNUSED(op);
    UNUSED(str_len);
    ALOGW("libmal API not found!(%s)", __func__);
    return;
}

static void jni_mal_mal_wo_sim_info_set_ps_capability(mal_ptr_t mal_ptr,
        mal_wo_sim_info_t *sim_info, bool has_ps_capability) {
    UNUSED(mal_ptr);
    UNUSED(sim_info);
    UNUSED(has_ps_capability);
    ALOGW("libmal API not found!(%s)", __func__);
    return;
}

static void jni_mal_mal_wo_sim_info_set_sim_type(mal_ptr_t mal_ptr, mal_wo_sim_info_t *sim_info,
        mal_wo_sim_type_t sim_type) {
    UNUSED(mal_ptr);
    UNUSED(sim_info);
    UNUSED(sim_type);
    ALOGW("libmal API not found!(%s)", __func__);
    return;
}

static bool jni_mal_mal_wo_sim_ready_notify(mal_ptr_t mal_ptr, mal_wo_sim_info_t *sim_info) {
    UNUSED(mal_ptr);
    UNUSED(sim_info);
    ALOGW("libmal API not found!(%s)", __func__);
    return false;
}


static bool jni_mal_mal_wo_sim_rejected_notify(mal_ptr_t mal_ptr) {
    UNUSED(mal_ptr);
    ALOGW("libmal API not found!(%s)", __func__);
    return false;
}

static mal_wo_sim_info_t* jni_mal_mal_wo_sim_info_alloc(mal_ptr_t mal_ptr,
        mal_wo_sim_info_t* sim_info) {
    UNUSED(mal_ptr);
    UNUSED(sim_info);
    ALOGW("libmal API not found!(%s)", __func__);
    return NULL;
}

static rds_int32 jni_mal_rds_get_last_err(rds_rb_get_demand_req_t *req,
        rds_rb_get_last_err_rsp_t *rsp, void * conn_ptr) {
    UNUSED(req);
    UNUSED(rsp);
    UNUSED(conn_ptr);
    rds_int32 ret = 0;
    ALOGW("libmal API not found!(%s)", __func__);
    return ret;
}

static rds_int32 jni_mal_rds_get_ran_type(rds_rb_get_demand_req_t *req,
        rds_rb_get_ran_type_rsp_t *rsp, void * conn_ptr) {
    UNUSED(req);
    UNUSED(rsp);
    UNUSED(conn_ptr);
    rds_int32 ret = 0;
    ALOGW("libmal API not found!(%s)", __func__);
    return ret;
}

static rds_int32 jni_mal_rds_notify_wifimonitor(rds_ru_set_wifimon_req_t *req, void * conn_ptr) {
    UNUSED(req);
    UNUSED(conn_ptr);
    rds_int32 ret = 0;
    ALOGW("libmal API not found!(%s)", __func__);
    return ret;
}

static rds_int32 jni_mal_rds_set_callstat(rds_ru_set_callstat_req_t *req, void * conn_ptr) {
    UNUSED(req);
    UNUSED(conn_ptr);
    rds_int32 ret = 0;
    ALOGW("libmal API not found!(%s)", __func__);
    return ret;
}

static rds_int32 jni_mal_rds_set_location(rds_set_location_t *loc) {
    UNUSED(loc);
    rds_int32 ret = 0;
    ALOGW("libmal API not found!(%s)", __func__);
    return ret;
}

static rds_int32 jni_mal_rds_set_mdstat(rds_ru_set_mdstat_req_t *req, void * conn_ptr) {
    UNUSED(req);
    UNUSED(conn_ptr);
    rds_int32 ret = 0;
    ALOGW("libmal API not found!(%s)", __func__);
    return ret;
}

static rds_int32 jni_mal_rds_set_radiostat(rds_ru_set_radiostat_req_t *req, void * conn_ptr) {
    UNUSED(req);
    UNUSED(conn_ptr);
    rds_int32 ret = 0;
    ALOGW("libmal API not found!(%s)", __func__);
    return ret;
}

static rds_int32 jni_mal_rds_set_ui_param(rds_ru_set_uiparam_req_t *req, void * conn_ptr) {
    UNUSED(req);
    UNUSED(conn_ptr);
    rds_int32 ret = 0;
    ALOGW("libmal API not found!(%s)", __func__);
    return ret;
}

static rds_int32 jni_mal_rds_set_wifistat(rds_ru_set_wifistat_req_t *req, void * conn_ptr) {
    UNUSED(req);
    UNUSED(conn_ptr);
    rds_int32 ret = 0;
    ALOGW("libmal API not found!(%s)", __func__);
    return ret;
}

static rds_int32 jni_mal_rds_if_req(rds_uint32 u4user, char *szcmd, void *prinput, void *proutput, void * conn_ptr) {
    UNUSED(u4user);
    UNUSED(szcmd);
    UNUSED(prinput);
    UNUSED(proutput);
    UNUSED(conn_ptr);
    rds_int32 ret = 0;
    ALOGW("libmal API not found!(%s)", __func__);
    return ret;
}

static void jni_mal_api_init() {
    ALOGW("jni_mal_api_init()");
    JNI_Mal_Api.handle = NULL;
    void *func = NULL;

    JNI_Mal_Api.handle = dlopen(MAL_LIB_NAME, RTLD_NOW);
    if (NULL == JNI_Mal_Api.handle) {
        ALOGW("%s, can't load library(%s): %s", __func__, MAL_LIB_NAME, dlerror());
        // Dummy function initialization.
        epdgs_rds_conn_init = jni_mal_epdgs_rds_conn_init;
        epdgs_rds_conn_exit = jni_mal_epdgs_rds_conn_exit;
        mal_reset = jni_mal_mal_reset;
        mal_set = jni_mal_mal_set;
        mal_status_reg_restart_cb = jni_mal_mal_status_reg_restart_cb;
        mal_nwmgnr_set_wfc_supported = jni_mal_mal_nwmgnr_set_wfc_supported;
        mal_wo_sim_info_free = jni_mal_mal_wo_sim_info_free;
        mal_wo_sim_info_set_imsi = jni_mal_mal_wo_sim_info_set_imsi;
        mal_wo_sim_info_set_imei = jni_mal_mal_wo_sim_info_set_imei;
        mal_wo_sim_info_set_impi = jni_mal_mal_wo_sim_info_set_impi;
        mal_wo_sim_info_set_operator = jni_mal_mal_wo_sim_info_set_operator;
        mal_wo_sim_info_set_sim_type = jni_mal_mal_wo_sim_info_set_sim_type;
        mal_wo_sim_ready_notify = jni_mal_mal_wo_sim_ready_notify;
        mal_wo_sim_rejected_notify = jni_mal_mal_wo_sim_rejected_notify;
        mal_wo_sim_info_alloc = jni_mal_mal_wo_sim_info_alloc;
        rds_get_last_err = jni_mal_rds_get_last_err;
        rds_get_ran_type = jni_mal_rds_get_ran_type;
        rds_notify_wifimonitor = jni_mal_rds_notify_wifimonitor;
        rds_set_callstat = jni_mal_rds_set_callstat;
        rds_set_location = jni_mal_rds_set_location;
        rds_set_mdstat = jni_mal_rds_set_mdstat;
        rds_set_radiostat = jni_mal_rds_set_radiostat;
        rds_set_ui_param = jni_mal_rds_set_ui_param;
        rds_set_wifistat = jni_mal_rds_set_wifistat;
        rds_if_req = jni_mal_rds_if_req;
        return;
    }

    func = dlsym(JNI_Mal_Api.handle, EPDGS_RDS_CONN_INIT);
    if (NULL != func) {
        epdgs_rds_conn_init = NULL;
        epdgs_rds_conn_init = (FP_EPDGS_RDS_CONN_INIT)(func);
        if (NULL == epdgs_rds_conn_init) {
            ALOGW("dlsym failed - EPDGS_RDS_CONN_INIT ");
            epdgs_rds_conn_init = jni_mal_epdgs_rds_conn_init;
        }
        func = NULL;
    }

    func = dlsym(JNI_Mal_Api.handle, EPDGS_RDS_CONN_EXIT);
    if (NULL != func) {
        epdgs_rds_conn_exit = NULL;
        epdgs_rds_conn_exit = (FP_EPDGS_RDS_CONN_EXIT)(func);
        if (NULL == epdgs_rds_conn_exit) {
            ALOGW("dlsym failed - EPDGS_RDS_CONN_EXIT ");
            epdgs_rds_conn_exit = jni_mal_epdgs_rds_conn_exit;
        }
        func = NULL;
    }

    func = dlsym(JNI_Mal_Api.handle, MAL_RESET);
    if (NULL != func) {
        mal_reset = NULL;
        mal_reset = (FP_MAL_RESET)(func);
        if (NULL == mal_reset) {
            ALOGW("dlsym failed - MAL_RESET ");
            mal_reset = jni_mal_mal_reset;
        }
        func = NULL;
    }

    func = dlsym(JNI_Mal_Api.handle, MAL_SET);
    if (NULL != func) {
        mal_set = NULL;
        mal_set = (FP_MAL_SET)(func);
        if (NULL == mal_set) {
            ALOGW("dlsym failed - MAL_SET ");
            mal_set = jni_mal_mal_set;
        }
        func = NULL;
    }

    func = dlsym(JNI_Mal_Api.handle, MAL_STATUS_REG_RESTART_CB);
    if (NULL != func) {
        mal_status_reg_restart_cb = NULL;
        mal_status_reg_restart_cb = (FP_MAL_STATUS_REG_RESTART_CB)(func);
        if (NULL == mal_status_reg_restart_cb) {
            ALOGW("dlsym failed - MAL_STATUS_REG_RESTART_CB ");
            mal_status_reg_restart_cb = jni_mal_mal_status_reg_restart_cb;
        }
        func = NULL;
    }

    func = dlsym(JNI_Mal_Api.handle, MAL_NWMGNR_SET_WFC_SUPPORTED);
    if (NULL != func) {
        mal_nwmgnr_set_wfc_supported = NULL;
        mal_nwmgnr_set_wfc_supported = (FP_MAL_NWMGNR_SET_WFC_SUPPORTED)(func);
        if (NULL == mal_nwmgnr_set_wfc_supported) {
            ALOGW("dlsym failed - MAL_NWMGNR_SET_WFC_SUPPORTED ");
            mal_nwmgnr_set_wfc_supported = jni_mal_mal_nwmgnr_set_wfc_supported;
        }
        func = NULL;
    }

    func = dlsym(JNI_Mal_Api.handle, MAL_WO_SIM_INFO_FREE);
    if (NULL != func) {
        mal_wo_sim_info_free = NULL;
        mal_wo_sim_info_free = (FP_MAL_WO_SIM_INFO_FREE)(func);
        if (NULL == mal_wo_sim_info_free) {
            ALOGW("dlsym failed - MAL_WO_SIM_INFO_FREE ");
            mal_wo_sim_info_free = jni_mal_mal_wo_sim_info_free;
        }
        func = NULL;
    }

    func = dlsym(JNI_Mal_Api.handle, MAL_WO_SIM_INFO_SET_IMSI);
    if (NULL != func) {
        mal_wo_sim_info_set_imsi = NULL;
        mal_wo_sim_info_set_imsi = (FP_MAL_WO_SIM_INFO_SET_IMSI)(func);
        if (NULL == mal_wo_sim_info_set_imsi) {
            ALOGW("dlsym failed - MAL_WO_SIM_INFO_SET_IMSI ");
            mal_wo_sim_info_set_imsi = jni_mal_mal_wo_sim_info_set_imsi;
        }
        func = NULL;
    }

    func = dlsym(JNI_Mal_Api.handle, MAL_WO_SIM_INFO_SET_IMEI);
    if (NULL != func) {
        mal_wo_sim_info_set_imei = NULL;
        mal_wo_sim_info_set_imei = (FP_MAL_WO_SIM_INFO_SET_IMEI)(func);
        if (NULL == mal_wo_sim_info_set_imei) {
            ALOGW("dlsym failed - MAL_WO_SIM_INFO_SET_IMEI ");
            mal_wo_sim_info_set_imei = jni_mal_mal_wo_sim_info_set_imei;
        }
        func = NULL;
    }

    func = dlsym(JNI_Mal_Api.handle, MAL_WO_SIM_INFO_SET_IMPI);
    if (NULL != func) {
        mal_wo_sim_info_set_impi = NULL;
        mal_wo_sim_info_set_impi = (FP_MAL_WO_SIM_INFO_SET_IMPI)(func);
        if (NULL == mal_wo_sim_info_set_impi) {
            ALOGW("dlsym failed - MAL_WO_SIM_INFO_SET_IMPI ");
            mal_wo_sim_info_set_impi = jni_mal_mal_wo_sim_info_set_impi;
        }
        func = NULL;
    }

    func = dlsym(JNI_Mal_Api.handle, MAL_WO_SIM_INFO_SET_OPERATOR);
    if (NULL != func) {
        mal_wo_sim_info_set_operator = NULL;
        mal_wo_sim_info_set_operator = (FP_MAL_WO_SIM_INFO_SET_OPERATOR)(func);
        if (NULL == mal_wo_sim_info_set_operator) {
            ALOGW("dlsym failed - MAL_WO_SIM_INFO_SET_OPERATOR ");
            mal_wo_sim_info_set_operator = jni_mal_mal_wo_sim_info_set_operator;
        }
        func = NULL;
    }

    func = dlsym(JNI_Mal_Api.handle, MAL_WO_SIM_INFO_SET_PS_CAPABILITY);
    if (NULL != func) {
        mal_wo_sim_info_set_ps_capability = NULL;
        mal_wo_sim_info_set_ps_capability =
            (FP_MAL_WO_SIM_INFO_SET_PS_CAPABILITY)(func);
        if (NULL == mal_wo_sim_info_set_ps_capability) {
            ALOGW("dlsym failed - MAL_WO_SIM_INFO_SET_PS_CAPABILITY ");
            mal_wo_sim_info_set_ps_capability =
                jni_mal_mal_wo_sim_info_set_ps_capability;
        }
        func = NULL;
    }

    func = dlsym(JNI_Mal_Api.handle, MAL_WO_SIM_INFO_SET_SIM_TYPE);
    if (NULL != func) {
        mal_wo_sim_info_set_sim_type = NULL;
        mal_wo_sim_info_set_sim_type = (FP_MAL_WO_SIM_INFO_SET_SIM_TYPE)(func);
        if (NULL == mal_wo_sim_info_set_sim_type) {
            ALOGW("dlsym failed - MAL_WO_SIM_INFO_SET_SIM_TYPE ");
            mal_wo_sim_info_set_sim_type = jni_mal_mal_wo_sim_info_set_sim_type;
        }
        func = NULL;
    }

    func = dlsym(JNI_Mal_Api.handle, MAL_WO_SIM_READY_NOTIFY);
    if (NULL != func) {
        mal_wo_sim_ready_notify = NULL;
        mal_wo_sim_ready_notify = (FP_MAL_WO_SIM_READY_NOTIFY)(func);
        if (NULL == mal_wo_sim_ready_notify) {
            ALOGW("dlsym failed - MAL_WO_SIM_READY_NOTIFY ");
            mal_wo_sim_ready_notify = jni_mal_mal_wo_sim_ready_notify;
        }
        func = NULL;
    }

    func = dlsym(JNI_Mal_Api.handle, MAL_WO_SIM_REJECTED_NOTIFY);
    if (NULL != func) {
        mal_wo_sim_rejected_notify = NULL;
        mal_wo_sim_rejected_notify = (FP_MAL_WO_SIM_REJECTED_NOTIFY)(func);
        if (NULL == mal_wo_sim_rejected_notify) {
            ALOGW("dlsym failed - MAL_WO_SIM_REJECTED_NOTIFY ");
            mal_wo_sim_rejected_notify = jni_mal_mal_wo_sim_rejected_notify;
        }
        func = NULL;
    }

    func = dlsym(JNI_Mal_Api.handle, MAL_WO_SIM_INFO_ALLOC);
    if (NULL != func) {
        mal_wo_sim_info_alloc = NULL;
        mal_wo_sim_info_alloc = (FP_MAL_WO_SIM_INFO_ALLOC)(func);
        if (NULL == mal_wo_sim_info_alloc) {
            ALOGW("dlsym failed - MAL_WO_SIM_INFO_ALLOC ");
            mal_wo_sim_info_alloc = jni_mal_mal_wo_sim_info_alloc;
        }
        func = NULL;
    }

    func = dlsym(JNI_Mal_Api.handle, RDS_GET_LAST_ERR);
    if (NULL != func) {
        rds_get_last_err = NULL;
        rds_get_last_err = (FP_RDS_GET_LAST_ERR)(func);
        if (NULL == rds_get_last_err) {
            ALOGW("dlsym failed - RDS_GET_LAST_ERR ");
            rds_get_last_err = jni_mal_rds_get_last_err;
        }
        func = NULL;
    }

    func = dlsym(JNI_Mal_Api.handle, RDS_GET_RAN_TYPE);
    if (NULL != func) {
        rds_get_ran_type = NULL;
        rds_get_ran_type = (FP_RDS_GET_RAN_TYPE)(func);
        if (NULL == rds_get_ran_type) {
            ALOGW("dlsym failed - RDS_GET_RAN_TYPE ");
            rds_get_ran_type = jni_mal_rds_get_ran_type;
        }
        func = NULL;
    }

    func = dlsym(JNI_Mal_Api.handle, RDS_NOTIFY_WIFIMONITOR);
    if (NULL != func) {
        rds_notify_wifimonitor = NULL;
        rds_notify_wifimonitor = (FP_RDS_NOTIFY_WIFIMONITOR)(func);
        if (NULL == rds_notify_wifimonitor) {
            ALOGW("dlsym failed - RDS_NOTIFY_WIFIMONITOR ");
            rds_notify_wifimonitor = jni_mal_rds_notify_wifimonitor;
        }
        func = NULL;
    }

    func = dlsym(JNI_Mal_Api.handle, RDS_SET_CALLSTAT);
    if (NULL != func) {
        rds_set_callstat = NULL;
        rds_set_callstat = (FP_RDS_SET_CALLSTAT)(func);
        if (NULL == rds_set_callstat) {
            ALOGW("dlsym failed - RDS_SET_CALLSTAT ");
            rds_set_callstat = jni_mal_rds_set_callstat;
        }
        func = NULL;
    }

    func = dlsym(JNI_Mal_Api.handle, RDS_SET_LOCATION);
    if (NULL != func) {
        rds_set_location = NULL;
        rds_set_location = (FP_RDS_SET_LOCATION)(func);
        if (NULL == rds_set_location) {
            ALOGW("dlsym failed - RDS_SET_LOCATION ");
            rds_set_location = jni_mal_rds_set_location;
        }
        func = NULL;
    }

    func = dlsym(JNI_Mal_Api.handle, RDS_SET_MDSTAT);
    if (NULL != func) {
        rds_set_mdstat = NULL;
        rds_set_mdstat = (FP_RDS_SET_MDSTAT)(func);
        if (NULL == rds_set_mdstat) {
            ALOGW("dlsym failed - RDS_SET_MDSTAT ");
            rds_set_mdstat = jni_mal_rds_set_mdstat;
        }
        func = NULL;
    }

    func = dlsym(JNI_Mal_Api.handle, RDS_SET_RADIOSTAT);
    if (NULL != func) {
        rds_set_radiostat = NULL;
        rds_set_radiostat = (FP_RDS_SET_RADIOSTAT)(func);
        if (NULL == rds_set_radiostat) {
            ALOGW("dlsym failed - RDS_SET_RADIOSTAT ");
            rds_set_radiostat = jni_mal_rds_set_radiostat;
        }
        func = NULL;
    }

    func = dlsym(JNI_Mal_Api.handle, RDS_SET_UI_PARAM);
    if (NULL != func) {
        rds_set_ui_param = NULL;
        rds_set_ui_param = (FP_RDS_SET_UI_PARAM)(func);
        if (NULL == rds_set_ui_param) {
            ALOGW("dlsym failed - RDS_SET_UI_PARAM ");
            rds_set_ui_param = jni_mal_rds_set_ui_param;
        }
        func = NULL;
    }

    func = dlsym(JNI_Mal_Api.handle, RDS_SET_WIFISTAT);
    if (NULL != func) {
        rds_set_wifistat = NULL;
        rds_set_wifistat = (FP_RDS_SET_WIFISTAT)(func);
        if (NULL == rds_set_wifistat) {
            ALOGW("dlsym failed - RDS_SET_WIFISTAT ");
            rds_set_wifistat = jni_mal_rds_set_wifistat;
        }
        func = NULL;
    }

    func = dlsym(JNI_Mal_Api.handle, RDS_IF_REQ);
    if (NULL != func) {
        rds_if_req = NULL;
        rds_if_req = (FP_RDS_IF_REQ)(func);
        if (NULL ==rds_if_req) {
            ALOGW("dlsym failed - FP_RDS_IF_REQ ");
            rds_if_req = jni_mal_rds_if_req;
        }
        func = NULL;
    }
}

static void jni_mal_api_exit() {
    dlclose(JNI_Mal_Api.handle);
    JNI_Mal_Api.handle = NULL;
}
