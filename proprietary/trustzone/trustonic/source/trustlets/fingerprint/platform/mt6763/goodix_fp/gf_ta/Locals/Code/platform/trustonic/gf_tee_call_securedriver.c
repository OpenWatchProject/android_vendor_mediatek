/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#include <taStd.h>
#include <tee_internal_api.h>
#include "gf_tee_internal_api.h"
#include "gf_dr.h"
#include "gf_wechat_api.h"

#define LOG_TAG "[gf_tee_call_securedriver]"

gf_error_t gf_sync_auth_result(uint32_t finger_id, uint64_t time) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();
    err = gf_dr_sync_auth_result(finger_id, time);
    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_sync_auth_result_notify(uint32_t group_id, uint32_t finger_id, uint64_t operation_id,
        uint32_t is_success) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();
    UNUSED_VAR(group_id);
    UNUSED_VAR(finger_id);
    UNUSED_VAR(operation_id);
    UNUSED_VAR(is_success);
    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_secure_share_memory_performance(uint8_t *buf, uint32_t len, uint64_t *time) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();
    err = gf_dr_share_memory_performance(buf, len, time);
    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_sync_finger_list(uint32_t *fingerIDs, uint32_t count) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();
    err = gf_dr_sync_finger_list(fingerIDs, count);
    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_sync_authenticator_version(uint32_t version) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();
    err = gf_dr_sync_authenticator_version(version);
    FUNC_EXIT(err);

    return err;
}
