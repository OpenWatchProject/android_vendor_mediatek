#include "tlStd.h"
#include "TlApiDriver.h"
#include "TlApi/TlApiCommon.h"

#include "gf_error.h"
#include "gf_dr_common.h"
#include "gf_dr.h"

typedef uint32_t TEE_Result;
#define TEE_SUCCESS                      ((TEE_Result)0x00000000)
#define TEE_ERROR_BAD_PARAMETERS         ((TEE_Result)0xFFFF0006)
#define TEE_ERROR_TIMEOUT                ((TEE_Result)0xFFFF3001)

#define LOG_TAG "[gf_wechat_api] "

TEE_Result gf_dr_wechat_api_get_session_id(uint64_t* session_id) {

    TEE_Result err = TEE_SUCCESS;
    do {
        if (NULL == session_id) {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        gf_dr_cmd_t cmd = { 0 };
        cmd.cmd_id = GF_DR_CMD_WECHAT_API_GET_SESSION_ID;

        tlApiResult_t ret = tlApi_callDriver(DRIVER_ID, &cmd);
        if (ret != TLAPI_OK) {
            break;
        } else {
            *session_id = cmd.data.get_session_id.session_id;
        }
    } while (0);

    return err;
}

TEE_Result gf_dr_wechat_api_set_session_id(uint64_t session_id) {

    TEE_Result err = TEE_SUCCESS;
    do {

        gf_dr_cmd_t cmd = { 0 };
        cmd.cmd_id = GF_DR_CMD_WECHAT_API_SET_SESSION_ID;
        cmd.data.set_session_id.session_id = session_id;
        tlApiResult_t ret = tlApi_callDriver(DRIVER_ID, &cmd);
        if (ret != TLAPI_OK) {
            break;
        }

    } while (0);

    return err;
}

TEE_Result tlApiFpGetFingerId(int32_t *finger_id) {
    TEE_Result err = TEE_SUCCESS;
    uint64_t current_time = 0;

    do {
        if (NULL == finger_id) {
            err = TEE_ERROR_BAD_PARAMETERS;
            break;
        }

        gf_dr_cmd_t cmd = { 0 };
        cmd.cmd_id = GF_DR_CMD_WECHAT_API_GET_AUTH_ID;
        tlApiResult_t ret = tlApi_callDriver(DRIVER_ID, &cmd);
        if (ret != TLAPI_OK) {
            break;
        }

        gf_dr_get_timestamp(&current_time);
        if (current_time - cmd.data.wechat_get_auth_id.time > 3000 * 1000) {
            err = TEE_ERROR_TIMEOUT;
            break;
        }

        *finger_id = cmd.data.wechat_get_auth_id.finger_id;

    } while(0);

    return err;
}

