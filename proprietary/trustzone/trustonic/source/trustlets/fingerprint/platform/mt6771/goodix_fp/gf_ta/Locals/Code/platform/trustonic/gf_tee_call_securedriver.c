/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/


#include <taStd.h>
#include <tee_internal_api.h>
#include "gf_tee_internal_api.h"
#include "gf_dr.h"
#include "gf_wechat_api.h"

#define LOG_TAG "[gf_tee_call_securedriver]"


gf_error_t gf_sync_auth_result(uint32_t finger_id, uint64_t time) {
    return gf_dr_sync_auth_result(finger_id, time);
}

gf_error_t gf_sync_auth_result_notify(uint32_t group_id, uint32_t finger_id, uint32_t is_success) {
    UNUSED_VAR(group_id);
    UNUSED_VAR(finger_id);
    UNUSED_VAR(is_success);
    return GF_SUCCESS;
}

gf_error_t gf_secure_share_memory_performance(uint8_t *buf, uint32_t len, uint64_t *time) {
    return gf_dr_share_memory_performance(buf, len, time);
}

gf_error_t gf_wechat_api_get_session_id(uint64_t *session_id)
{
    return gf_dr_wechat_api_get_session_id(session_id);
}

gf_error_t gf_wechat_api_set_session_id(uint64_t session_id)
{
    return gf_dr_wechat_api_set_session_id(session_id);
}

gf_error_t gf_sync_finger_list(uint32_t *fingerIDs, uint32_t count) {
    return gf_dr_sync_finger_list(fingerIDs, count);
}

gf_error_t gf_sync_authenticator_version( uint32_t version) {
    return gf_dr_sync_authenticator_version(version);
}

