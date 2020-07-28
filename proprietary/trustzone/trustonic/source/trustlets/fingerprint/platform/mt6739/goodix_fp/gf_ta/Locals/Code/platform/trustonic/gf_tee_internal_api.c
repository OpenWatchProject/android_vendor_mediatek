/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#include <taStd.h>
#include <tee_internal_api.h>
#include "gf_tee_internal_api.h"
#include <buildTag.h>
#include "string.h"
#include "cpl_string.h"
#include "gf_type_define.h"

#define  LOG_TAG  "[gf_tee_internal_api]"

void tlApiLogPrintf(const char *fmt, ...);
extern uint32_t get_hmac_key(uint8_t *hmac_key, uint32_t key_length);
extern gf_error_t gf_generate_hmac_trustonic(void *token);

void tlApiLogPrintf(const char *fmt, ...) {
    UNUSED_VAR(fmt);
    return;
}

gf_error_t gf_generate_random(void *random_buffer, int32_t len) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();
    TEE_GenerateRandom(random_buffer, len);
    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_get_hmac_key(uint8_t *hmac_key, uint32_t key_length) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();
    err = get_hmac_key(hmac_key, key_length);
    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_set_hmac_key(uint8_t* hmac_key, int32_t key_length) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();
    UNUSED_VAR(hmac_key);
    UNUSED_VAR(key_length);
    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_get_tee_version(int8_t *buffer, uint32_t len) {
    uint32_t version_len = strlen(MOBICORE_COMPONENT_BUILD_TAG);
    uint32_t copy_len = version_len > len ? len : version_len;
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();
    memcpy(buffer, MOBICORE_COMPONENT_BUILD_TAG, copy_len);
    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_generate_hmac_enroll(gf_hw_auth_token_t *token) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();
    err = gf_generate_hmac_trustonic(token);
    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_generate_hmac_authenticate(gf_hw_auth_token_t *token) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();
    err = gf_generate_hmac_trustonic(token);
    FUNC_EXIT(err);

    return err;
}

uint32_t gf_tee_log(const char *fmt, ...) {
    va_list valist;

    va_start(valist, fmt);
    TEE_LogvPrintf(fmt, valist);
    va_end(valist);

    return 0;
}

gf_error_t gf_aes_encrypt(uint8_t *in, uint8_t *out,
        uint32_t in_len, uint8_t *aes_key, uint32_t key_len) {
    UNUSED_VAR(aes_key);
    UNUSED_VAR(key_len);

    if (NULL == in || NULL == out || NULL == aes_key) {
        return GF_ERROR_GENERIC;
    }

    cpl_memcpy(out, in, in_len);
    return GF_SUCCESS;
}

gf_error_t gf_aes_decrypt(uint8_t *in, uint8_t *out,
        uint32_t in_len, uint8_t *aes_key, uint32_t key_len) {
    UNUSED_VAR(aes_key);
    UNUSED_VAR(key_len);

    if (NULL == in || NULL == out || NULL == aes_key) {
        return GF_ERROR_GENERIC;
    }

    cpl_memcpy(out, in, in_len);
    return GF_SUCCESS;
}

