#include "tlStd.h"
#include "TlApiDriver.h"
#include "TlApi/TlApiCommon.h"

#include "gf_error.h"

#include "gf_dr_common.h"

#define LOG_TAG "[gf_dr] "

gf_error_t gf_dr_get_timestamp(uint64_t *timestamp) {

    gf_error_t err = GF_SUCCESS;
    do {
        if (NULL == timestamp) {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        gf_dr_cmd_t cmd = { 0 };
        cmd.cmd_id = GF_DR_CMD_GET_TIMESTAMP;

        tlApiResult_t ret = tlApi_callDriver(DRIVER_ID, &cmd);
        if (ret != TLAPI_OK) {
            break;
        }

        *timestamp = cmd.data.get_timestamp.timestamp;
    } while (0);

    return err;
}

gf_error_t gf_dr_sleep(uint32_t miliseconds) {

    gf_error_t err = GF_SUCCESS;
    do {
        gf_dr_cmd_t cmd = { 0 };
        cmd.cmd_id = GF_DR_CMD_SLEEP;
        cmd.data.sleep.miliseconds = miliseconds;

        tlApiResult_t ret = tlApi_callDriver(DRIVER_ID, &cmd);
        if (ret != TLAPI_OK) {
            break;
        }
    } while (0);

    return err;
}

gf_error_t gf_dr_sync_auth_result(uint32_t finger_id, uint64_t time) {
    gf_error_t err = GF_SUCCESS;
    do {
        gf_dr_cmd_t cmd = { 0 };
        cmd.cmd_id = GF_DR_CMD_SYNC_AUTH_RESULT;
        cmd.data.auth_result.finger_id = finger_id;
        cmd.data.auth_result.time = time;

        tlApiResult_t ret = tlApi_callDriver(DRIVER_ID, &cmd);
        if (ret != TLAPI_OK) {
            break;
        }
    } while (0);

    return err;
}

gf_error_t gf_dr_share_memory_performance(uint8_t *buf, uint32_t len, uint64_t *time) {
    gf_error_t err = GF_SUCCESS;
    do {
        gf_dr_cmd_t cmd = { 0 };
        cmd.cmd_id = GF_DR_CMD_SHARE_MEMORY_PREFORMANCE;
        cmd.data.share_memory_performance.buf = buf;
        cmd.data.share_memory_performance.len = len;

        tlApiResult_t ret = tlApi_callDriver(DRIVER_ID, &cmd);
        if (ret != TLAPI_OK) {
            break;
        }

        *time = cmd.data.share_memory_performance.time;
    } while (0);

    return err;
}
