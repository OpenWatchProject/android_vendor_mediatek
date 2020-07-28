/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/


#include <stdlib.h>
#include <string.h>
#include "gf_tee_internal_api.h"
#include "gf_error.h"
#include "gf_sensor.h"
#include "cpl_string.h"
#include "cpl_memory.h"
#include "gf_secure_object.h"

#define LOG_TAG "[gf_secure_object] "

#define GF_TOKEN_LEN                          sizeof(uint8_t)
#define GF_AUTH_TOKEN_SECURE_USER_ID_TOKEN    0xF4
#define GF_AUTH_TOKEN_SECURE_USER_ID_LEN      (GF_TOKEN_LEN + sizeof(uint64_t))

#define GF_AUTH_TOKEN_AUTHENTICATOR_ID_TOKEN  0xF5
#define GF_AUTH_TOKEN_AUTHENTICATOR_ID_LEN    (GF_TOKEN_LEN + sizeof(uint64_t))

#define GF_AUTH_TOKEN_LEN                     (GF_AUTH_TOKEN_SECURE_USER_ID_LEN + GF_AUTH_TOKEN_AUTHENTICATOR_ID_LEN)

gf_error_t gf_so_load_persistent_object(int8_t *name, uint8_t **data, uint32_t *data_len) {

    gf_error_t err = GF_SUCCESS;
    GF_TEE_HANDLE so_handle = GF_TEE_HANDLE_NULL;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {

        if ((NULL == name) || (NULL == data) || (NULL == data_len)) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] invalid parameters, name:0x%p, data:%p data_len:0x%p", __func__, name, (void *)data, (void *)data_len);
            break;
        }

        err = gf_tee_open_object((uint8_t*) name, SO_MODE_READ, &so_handle);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] so(%s) not exist, err:%d", __func__, name, err);
            break;
        }

        *data_len = 0;
        err = gf_tee_get_object_size(so_handle, data_len);
        if (GF_SUCCESS != err || ((int32_t)*data_len) <= 0) {
            GF_LOGE(
                    LOG_TAG "[%s] fail to get object size, err:%d, size:%d", __func__, err, *data_len);
            err = GF_ERROR_READ_SECURE_OBJECT_FAILED;
            break;
        }

        *data = (uint8_t *) CPL_MEM_MALLOC(*data_len);
        if (NULL == *data) {
            err = GF_ERROR_OUT_OF_MEMORY;
            GF_LOGE(LOG_TAG "[%s] malloc memory(%d bytes) fail", __func__, *data_len);
            break;
        }

        err = gf_tee_read_object_data(so_handle, *data, *data_len);
        if (GF_SUCCESS != err) {
            CPL_MEM_FREE(*data);
            *data = NULL;
            *data_len = 0;
            GF_LOGE(LOG_TAG "[%s] read so(%s) fail, err:%d", __func__, name, err);
            break;
        }
    } while (0);

    if (GF_TEE_HANDLE_NULL != so_handle) {
        gf_tee_close_object(so_handle);
    }

    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);

    return err;
}

gf_error_t gf_so_save_persistent_object(int8_t *name, uint8_t *data, uint32_t data_len) {

    gf_error_t err = GF_SUCCESS;
    GF_TEE_HANDLE so_handle = GF_TEE_HANDLE_NULL;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        if ((NULL == name) || (NULL == data) || (0 == data_len)) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] invalid parameters, name:0x%p, data:0x%p, data_len:%d", __func__, name, (void*)data, data_len);
            break;
        }

#if (defined __TEE_NUTLET) || (defined __TEE_TRUSTKERNEL)
        gf_tee_delete_object((uint8_t*)name);

        err = gf_tee_write_object_data((uint8_t*)name, &so_handle, data, data_len);
#else
        err = gf_tee_open_object((uint8_t*) name, SO_MODE_WRITE, &so_handle);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] open so(%s) fail", __func__, name);
            break;
        }

        err = gf_tee_write_object_data(so_handle, data, data_len);
#endif
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] write so(%s) fail, err:%d", __func__, name, err);
            break;
        }
    } while (0);

    if (GF_TEE_HANDLE_NULL != so_handle) {
        gf_tee_close_object(so_handle);
    }

    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);

    return err;
}

gf_error_t gf_so_delete_persistent_object(int8_t *name) {

    gf_error_t err = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        if (NULL == name) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad parameter, name:0x%p", __func__, name);
            break;
        }

        err = gf_tee_delete_object((uint8_t*) name);
    } while (0);

    GF_LOGD(LOG_TAG "[%s] err:%d", __func__, err);

    return err;
}

gf_error_t gf_so_get_auth_token_len(uint32_t *len) {
    *len = GF_AUTH_TOKEN_LEN;
    return GF_SUCCESS;
}

gf_error_t gf_so_get_auth_token_pdu(uint8_t *buf, uint32_t buf_len, uint64_t secure_user_id,
        uint64_t authenticator_id) {
    gf_error_t err = GF_SUCCESS;
    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        uint8_t *buffer = buf;

        if (NULL == buf || buf_len < GF_AUTH_TOKEN_LEN) {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        *buffer = GF_AUTH_TOKEN_SECURE_USER_ID_TOKEN;
        buffer++;
        buf_len--;

        cpl_memcpy(buffer, &secure_user_id, sizeof(uint64_t));
        buffer += sizeof(uint64_t);
        buf_len -= sizeof(uint64_t);

        *buffer = GF_AUTH_TOKEN_AUTHENTICATOR_ID_TOKEN;
        buffer++;
        buf_len--;

        cpl_memcpy(buffer, &authenticator_id, sizeof(uint64_t));
        buffer += sizeof(uint64_t);
        buf_len -= sizeof(uint64_t);

    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);
    return err;
}

gf_error_t gf_so_load_auth_token(uint64_t *secure_user_id, uint64_t *authenticator_id,
        uint8_t *buf, uint32_t buf_len) {
    gf_error_t err = GF_SUCCESS;
    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        uint8_t *buffer = buf;

        if (NULL == buf || buf_len < GF_AUTH_TOKEN_LEN) {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (buffer[0] != GF_AUTH_TOKEN_SECURE_USER_ID_TOKEN) {
            GF_LOGE(LOG_TAG "[%s] invalid token , buffer[0]:0x%02X", __func__, buffer[0]);
            err = GF_ERROR_INVALID_DATA;
            break;
        }

        buf_len--;
        buffer++;
        cpl_memcpy(secure_user_id, buffer, sizeof(uint64_t));
        buf_len -= sizeof(uint64_t);
        buffer += sizeof(uint64_t);

        if (buffer[0] != GF_AUTH_TOKEN_AUTHENTICATOR_ID_TOKEN) {
            GF_LOGE(LOG_TAG "[%s] invalid token , buffer[0]:0x%02X", __func__, buffer[0]);
            err = GF_ERROR_INVALID_DATA;
            break;
        }

        buf_len--;
        buffer++;
        cpl_memcpy(authenticator_id, buffer, sizeof(uint64_t));

    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);
    return err;
}
