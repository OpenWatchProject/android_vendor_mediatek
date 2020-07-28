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

#define LOG_TAG "[gf_alipay_api] "

TEE_Result gf_alipay_api_get_auth_id(int32_t* finger_id) {

    TEE_Result err = TEE_SUCCESS;
    do {
        if (NULL == finger_id) {
            err = TEE_ERROR_BAD_PARAMETERS;
            break;
        }

        gf_dr_cmd_t cmd = { 0 };
        cmd.cmd_id = GF_DR_CMD_ALIPAY_API_GET_AUTH_ID;

        tlApiResult_t ret = tlApi_callDriver(DRIVER_ID, &cmd);
        if (ret != TLAPI_OK) {
            break;
        }

        uint64_t current_time = 0;
        gf_dr_get_timestamp(&current_time);
        if (current_time - cmd.data.get_auth_id.time < 1000 * 1000) {
            *finger_id = cmd.data.get_auth_id.finger_id;
        } else {
            err = TEE_ERROR_TIMEOUT;
        }

    } while (0);

    return err;
}
