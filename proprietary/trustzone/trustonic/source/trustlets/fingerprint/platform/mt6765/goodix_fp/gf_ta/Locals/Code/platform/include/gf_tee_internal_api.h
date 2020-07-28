/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/
#ifndef __GF_TEE_INTERNAL_API_H__
#define __GF_TEE_INTERNAL_API_H__

#include <stdint.h>
#include "gf_error.h"
#include "gf_tee_storage.h"
#include "gf_log.h"
#include "gf_type_define.h"
#include "gf_tee_stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NULL
#define NULL 0
#endif

#define UNUSED_VAR(X)   (void)(X)

//memory
void *gf_malloc(int32_t size);
void gf_free(void *buffer);
void *gf_malloc_with_default_value(int32_t size, uint8_t default_value);
void* gf_realloc(void* buf, uint32_t buf_len);

//time
void gf_sleep(uint32_t milliseconds);
gf_error_t gf_get_timestamp(uint64_t *timestamp);

//secure driver
gf_error_t gf_sync_auth_result(uint32_t finger_id, uint64_t time);
gf_error_t gf_wechat_api_get_session_id(uint64_t *session_id);
gf_error_t gf_wechat_api_set_session_id(uint64_t session_id);
gf_error_t gf_secure_share_memory_performance(uint8_t *buf, uint32_t len, uint64_t *time);
gf_error_t gf_sync_finger_list(uint32_t *fingerIDs, uint32_t count) ;
gf_error_t gf_sync_authenticator_version( uint32_t version);
gf_error_t gf_sync_auth_result_notify(uint32_t group_id, uint32_t finger_id, uint32_t is_success);

//other
gf_error_t gf_generate_random(void *random_buffer, int32_t len);
gf_error_t gf_get_tee_version(int8_t *buffer,uint32_t len);

//key master
gf_error_t gf_get_hmac_key(uint8_t *hmac_key, uint32_t key_length);
gf_error_t gf_generate_hmac_enroll(gf_hw_auth_token_t *token);
gf_error_t gf_generate_hmac_authenticate(gf_hw_auth_token_t *token);

#ifdef __cplusplus
}
#endif

#endif // __GF_TEE_INTERNAL_API_H__
