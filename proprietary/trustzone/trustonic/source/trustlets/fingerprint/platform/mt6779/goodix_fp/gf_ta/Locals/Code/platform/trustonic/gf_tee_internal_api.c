/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/

#include <taStd.h>
#include <tee_internal_api.h>
#include "gf_tee_internal_api.h"
#include <buildTag.h>
#include "string.h"
#include "gf_type_define.h"

#define  LOG_TAG  "[gf_tee_internal_api]"


extern uint32_t get_hmac_key(uint8_t *hmac_key, uint32_t key_length);
extern gf_error_t gf_generate_hmac_trustonic(void *token);
//multiple definition of `tlApiLogPrintf'
//void tlApiLogPrintf(const char *fmt, ...);
//void tlApiLogPrintf(const char *fmt, ...) {
//   UNUSED_VAR(fmt);
//    return;
//}

gf_error_t gf_generate_random(void *random_buffer, int32_t len) {
    TEE_GenerateRandom(random_buffer, len);
    return GF_SUCCESS;
}

gf_error_t gf_get_hmac_key(uint8_t *hmac_key, uint32_t key_length) {
    return get_hmac_key(hmac_key, key_length);
}

gf_error_t gf_get_tee_version(int8_t *buffer,uint32_t len){
    uint32_t version_len = strlen(MOBICORE_COMPONENT_BUILD_TAG);
    uint32_t copy_len = version_len > len ? len : version_len ;
    memcpy(buffer, MOBICORE_COMPONENT_BUILD_TAG, copy_len);
    return GF_SUCCESS;
}

gf_error_t gf_generate_hmac_enroll(gf_hw_auth_token_t *token){
    return gf_generate_hmac_trustonic(token);
}

gf_error_t gf_generate_hmac_authenticate(gf_hw_auth_token_t *token){
    return gf_generate_hmac_trustonic(token);
}
