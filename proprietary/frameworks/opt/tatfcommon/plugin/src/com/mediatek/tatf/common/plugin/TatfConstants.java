package com.mediatek.tatf.common.plugin;

import android.text.TextUtils;

public class TatfConstants {
    public static final String AT_ERROR_NO_AVAILABLE_IMS_PHONE =
            "not insert available sim support ims feature.";

    public static final String AT_ERROR_NO_AVAILABLE_SIM =
            "without any valid SIM inserted.";

    public static final int MOCALL_RESULT_SUCCESS = 0;
    // Testcase error, need case designer check.
    public static final int MOCALL_RESULT_ERROR = 10;
    // Tatf internal error, need tatf owner check.
    public static final int MOCALL_RESULT_ERROR_INTERNAL = 20;
    // Test environment error, need AT check.
    // No available phone.
    public static final int MOCALL_RESULT_ERROR_NOAVAILAIBLE_PHONE = 30;
    // Framework error, need case owner check.
    public static final int MOCALL_RESULT_CALLFAILED = 40;
    // WFC error, need case owner check
    public static final int MOCALL_RESULT_WFC_ERROR = 41;

    // MoCall type.
    public static final int MOCALL_TYPE_CS = 0;
    public static final int MOCALL_TYPE_VOLTE = 1;
    public static final int MOCALL_TYPE_VIDEO = 2;
    public static final int MOCALL_TYPE_WFC = 3;
    public static final int MOCALL_TYPE_VOLTE_DIGITS = 4;
    public static final int MOCALL_TYPE_WFC_DIGITS = 5;

    // AP Security type for WIFI
    public static final int SECURITY_NONE = 0;
    public static final int SECURITY_WEP = 1;
    public static final int SECURITY_PSK = 2;
    public static final int SECURITY_EAP = 5;

    // configuration

    public static final String CMD_INCALLUI_MOCALL = "incallui_mo_call";
    public static final String CMD_INCALLUI_ENDCALL = "incallui_end_call";
    public static final String CMD_INCALLUI_CALL_STATE = "incallui_call_state";

    /**
     * Error handle
     */
    public static final int GENERIC_ERROR = -1;

    /// timeout for some long time operation, e.g Search PLAN.
    public static final int COST_TIME_OPERATION_TIMEOUT = 120;
    public static final int DEFAULT_OPERATION_TIMEOUT = 60;
    public static final int MAX_TIME_OPERATION_TIMEOUT = 5 * 60;
    public static final int DEFAULT_CALL_DURATION = 10;

    // send command to Tatf, need recevied a mt call.
    public static final String CMD_FWK_REQUEST_MT_CALL = "tatf_fwk_mt_request";
    public static final String CMD_FWK_ANSWER_CALL = "tatf_fwk_answer_call";
    public static final String CMD_FWK_MO_CALL = "tatf_fwk_mo_call_without_hangup";
    public static final String CMD_FWK_END_CALL = "tatf_fwk_end_call";
    public static final String CMD_FWK_SWITCH_HOLD_CALL = "tatf_fwk_hold_call";
    public static final String CMD_FWK_SWITCH_RESUME_CALL = "tatf_fwk_resume_call";
    public static final String CMD_FWK_DOWNGRADE_CALL="tatf_fwk_downgrade_call";
    public static final String CMD_FWK_UPGRADE_CALL = "tatf_fwk_upgrade_call";
    // check if current call is VoLte/ViLte/WFC call.
    public static final String CMD_FWK_CHECK_CALL = "tatf_fwk_check_call";
    public static final String CMD_FWK_CHECK_CALL_HOLD = "tatf_fwk_check_call_hold";
    public static final String CMD_FWK_IS_VIDEO_CALL = "tatf_fwk_is_video_call";
    // TK/BSK support command.
    public static final String CMD_FWK_GET_PHONE_TYPE = "tatf_fwk_get_phone_type";
    // ECC Call
    public static final String CMD_FWK_REQUEST_ECC_CALL = "tatf_fwk_request_ecc_all";
    // IMS SS
    public static final String CMD_FWK_GET_CALL_FORWARD = "tatf_fwk_get_call_forward";
    public static final String CMD_FWK_SET_CALL_FORWARD = "tatf_fwk_set_call_forward";
    public static final String CMD_FWK_GET_CALL_WAITING = "tatf_fwk_get_call_waiting";
    public static final String CMD_FWK_SET_CALL_WAITING = "tatf_fwk_set_call_waiting";
    public static final String CMD_FWK_GET_CALL_IDENTITY = "tatf_fwk_get_call_identity";
    public static final String CMD_FWK_SET_CALL_IDENTITY = "tatf_fwk_set_call_identity";
    public static final String CMD_FWK_GET_CALL_BARRING = "tatf_fwk_get_call_barring";
    public static final String CMD_FWK_SET_CALL_BARRING = "tatf_fwk_set_call_barring";
    // Common DATA
    public static final String CMD_FWK_DATA_SWITCH_NO_WAIT = "tatf_fwk_data_switch_no_wait";
    public static final String CMD_FWK_GET_DATA_SS = "tatf_fwk_get_data_service_state";
    public static final String CMD_FWK_IS_DATA_CONNECTED = "tatf_fwk_is_data_connected";

    public static final String CMD_FWK_GET_SERVICE_STATE = "tatf_fwk_get_service_state";
    public static final String CMD_FWK_GET_SIGNAL_STRENGTH = "tatf_fwk_get_signal_strength";
    public static final String CMD_FWK_GET_ICC_LOCK_ENABLE = "tatf_fwk_get_icc_lock_enable";
    public static final String CMD_FWK_SET_ICC_LOCK_ENABLE = "tatf_fwk_set_icc_lock_enable";
    public static final String CMD_FWK_GET_RADIO_STATE = "tatf_fwk_get_radio_state";
    public static final String CMD_FWK_SET_TRM = "tatf_fwk_get_set_trm";
    public static final String CMD_FWK_RESET_RADIO = "tatf_fwk_reset_radio";

    // Common parameter key
    public static final String PARAM_PHONE_ID = "tatf_param_phoneId";
    public static final String PARAM_RESULT = "tatf_param_result";
    public static final String PARAM_ENABLE = "tatf_param_enable";
    public static final String PARAM_PASSWORD = "tatf_param_password";

    // mtcall constants
    public static final String PARAM_CALL_NUMBER = "tatf_param_call_number";
    public static final String PARAM_CALL_TYPE = "tatf_param_call_type";
    public static final String PARAM_CALL_RESULT = "tatf_param_call_result";
    public static final String PARAM_CALL_PHONEID = "tatf_param_call_phoneId";
    public static final String PARAM_CALL_STATE_RESULT = "tatf_param_call_state_result";
    public static final String PARAM_CALL_CERTIFICATION = "tatf_param_certification";
    public static final String PARAM_PHONE_TYPE = "tatf_param_phone_type";
    // IMS SS
    public static final String PARAM_CALL_FORWARD_TYPE = "tatf_param_call_forward_type";
    public static final String PARAM_CALL_BARRING_TYPE = "tatf_param_call_barring_type";
    public static final String PARAM_ACTION = "tatf_param_action";
    public static final String PARAM_PASSWARD = "tatf_param_passward";
    public static final String PARAM_FACILITY = "tatf_param_facility";

    // TRM mode
    public static final String PARAM_TRM_MODE = "tatf_param_trm_mode";

    public static final int MTCALL_RESULT_SUCCESS = 0;
    public static final int MTCALL_RESULT_BUSY = 1;
    // Internal state, for call is processing, warning status code.
    public static final int MTCALL_RESULT_CHECKING = 2;
    // failed for MTCall env, need Tatf check it.
    public static final int MTCALL_RESULT_ENV_ERROR = 10;
    public static final int MTCALL_RESULT_IO_ERROR = 11;
    // failed for Testcase error, need testcase owner check.
    public static final int MTCALL_RESULT_PARAMS_ERROR = 20;
    public static final int MTCALL_RESULT_INVALID_APIKEY = 21;

    public static final String MTCALL_HOST = "localhost";
    public static final int MTCALL_PORT = 4445;
    // MTCallServer should return 200 OK in the http content.
    public static final String MTCALL_STATE_OK = "200 OK";
    public static final String MTCALL_STATE_BUSY = "500 BUSY";
    public static final String MTCALL_STATE_UNAUTHOR = "403 Unauthorized";

    public static String getCallTypeName(int type) {
        switch (type) {
            case MOCALL_TYPE_CS:
                return "call";
            case MOCALL_TYPE_VIDEO:
                return "vicall";
            case MOCALL_TYPE_VOLTE:
            case MOCALL_TYPE_VOLTE_DIGITS:
                return "vocall";
            case MOCALL_TYPE_WFC:
            case MOCALL_TYPE_WFC_DIGITS:
                return "wfcall";
            default:
                return "";
        }
    }

    public static String getCallURL(int type, String callee, String certification) {
        //http://10.19.22.33:4446/call/1008611?api_key=123456789
        String url = "http://" + MTCALL_HOST + ":" + MTCALL_PORT + "/" + getCallTypeName(type)
                + "/" + callee;
        if (!TextUtils.isEmpty(certification)) {
            url = url + "?api_key=" + certification;
        }
        return url;
    }

    public static String resultMo(int code) {
        switch (code) {
            case MOCALL_RESULT_ERROR: {
                return "case error";
            }
            case MOCALL_RESULT_ERROR_INTERNAL: {
                return "case error internal";
            }
            case MOCALL_RESULT_ERROR_NOAVAILAIBLE_PHONE: {
                return "case error no available phone";
            }
            case MOCALL_RESULT_CALLFAILED: {
                return "case fail call failed";
            }
            case MOCALL_RESULT_WFC_ERROR: {
                return "case fail for wfc";
            }
            default:
                return "case success";
        }
    }

    public static String resultMt(int code) {
        switch (code) {
            case MTCALL_RESULT_BUSY: {
                return "case error server busy";
            }
            case MTCALL_RESULT_ENV_ERROR: {
                return "case error internal";
            }
            case MTCALL_RESULT_INVALID_APIKEY: {
                return "case error invalid apikey";
            }
            case MTCALL_RESULT_IO_ERROR: {
                return "case error client not run";
            }
            case MTCALL_RESULT_PARAMS_ERROR: {
                return "case error invalid parameters";
            }
            default:
                return "case success";
        }
    }

    public enum ServiceNumberType {
        CS(0), VILTE(1), VOLTE(2), WFC(3);
        private int value;

        ServiceNumberType(int value) {
            this.value = value;
        }

        public int value() {
            return this.value;
        }

    }
}
