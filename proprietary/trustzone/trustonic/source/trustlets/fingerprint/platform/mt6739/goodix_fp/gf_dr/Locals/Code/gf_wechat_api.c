/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#include "drStd.h"
#include "tlStd.h"
#include "TlApiDriver.h"

#include "gf_error.h"
#include "gf_dr_common.h"
#include "gf_dr.h"

#define LOG_TAG "[gf_wechat_api] "


typedef uint32_t TEE_Result;
#define TEE_SUCCESS                      ((TEE_Result)0x00000000)
#define TEE_ERROR_BAD_PARAMETERS         ((TEE_Result)0xFFFF0006)
#define TEE_ERROR_TIMEOUT                ((TEE_Result)0xFFFF3001)

#define LOG_TAG "[gf_wechat_api]"

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
            drDbgPrintLnf(LOG_TAG "Line=%d: tlApi_callDriver fails, error=0x%X\n", __LINE__, ret);
            err = TEE_ERROR_TIMEOUT;
            break;
        }

        gf_dr_get_timestamp(&current_time);
        if (current_time - cmd.data.wechat_get_auth_id.time > 3000 * 1000) {
            drDbgPrintLnf(LOG_TAG "Line=%d: time check fails, curr_time=0x%X, last_time=0x%x\n",
                    __LINE__, current_time, cmd.data.wechat_get_auth_id.time);
            err = TEE_ERROR_TIMEOUT;
            break;
        }

        *finger_id = cmd.data.wechat_get_auth_id.finger_id;
    } while (0);

    return err;
}

