/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/


#include "taStd.h"
#include "tee_internal_api.h"
#include "stdio.h"
#include "gf_tee_internal_api.h"

#define LOG_TAG "[gf_tee_storage]"

gf_error_t gf_tee_object_exist(uint8_t *object_id) {

    gf_error_t err = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        TEE_ObjectHandle object_handle = TEE_HANDLE_NULL;

        if (NULL == object_id) {
            GF_LOGE(LOG_TAG "[%s] invalid parameter", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        err = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE, object_id, strlen((const char*)object_id),
                TEE_DATA_FLAG_ACCESS_READ, &object_handle);
        if (TEE_SUCCESS != err) {
            GF_LOGI( LOG_TAG "[%s] open so(%s) fail:0x%08X", __func__, object_id, err);
            err = GF_ERROR_SECURE_OBJECT_NOT_EXIST;
            break;
        }

        TEE_CloseObject(object_handle);
    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit, err:0x%d", __func__, err);

    return err;
}

gf_error_t gf_tee_open_object(uint8_t *object_id, gf_tee_mode_t mode, GF_TEE_HANDLE *so_handle) {

    gf_error_t err = GF_SUCCESS;
    uint32_t flags = 0;  /* flags for open mode */

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {

        if ((NULL == object_id) || (NULL == so_handle)
                || ((SO_MODE_READ != mode) && (SO_MODE_WRITE != mode))) {
            GF_LOGE(
                    LOG_TAG "[%s] invalid parameter, object_id:0x%p, mode:%d, so_handle:0x%p", __func__, (void*)object_id, mode, (void*)so_handle);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        /* set the flags based on operation mode */
        if( SO_MODE_READ == mode ){
            flags = TEE_DATA_FLAG_ACCESS_READ;
        } else if( SO_MODE_WRITE  == mode ){
            flags = TEE_DATA_FLAG_ACCESS_WRITE;
        }

        err = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE, object_id, strlen((const char*)object_id),
                flags, (TEE_ObjectHandle *) so_handle);
        if (TEE_SUCCESS == err) {

            GF_LOGI(LOG_TAG "[%s] open so(%s) success", __func__, object_id);

            if (SO_MODE_WRITE == mode) {

                err = TEE_TruncateObjectData((TEE_ObjectHandle) *so_handle, 0);
                if (TEE_SUCCESS != err) {

                    GF_LOGE(LOG_TAG "[%s] truncate so fail:0x%08X", __func__, err);
                    err = GF_ERROR_OPEN_SECURE_OBJECT_FAILED;
                    TEE_CloseObject((TEE_ObjectHandle) *so_handle);
                    *so_handle = TEE_HANDLE_NULL;
                    break;
                }

                GF_LOGD(LOG_TAG "[%s] truncate so to zero success", __func__);
            }

            break;
        }

        GF_LOGI(LOG_TAG "[%s] so(%s) not exist, ret:0x%08X", __func__, object_id, err);

        if (SO_MODE_WRITE == mode) {
            err = TEE_CreatePersistentObject(TEE_STORAGE_PRIVATE, object_id, strlen((const char*)object_id),
                    flags, TEE_HANDLE_NULL, NULL, 0,
                    (TEE_ObjectHandle *) so_handle);

            if (TEE_SUCCESS == err) {
                GF_LOGI(LOG_TAG "[%s] create new so(%s) success", __func__, object_id);
                break;
            }

            GF_LOGE(LOG_TAG "[%s] create new so(%s) fail:0x%08X", __func__, object_id, err);
        }

        err = GF_ERROR_OPEN_SECURE_OBJECT_FAILED;
    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit, err:0x%d", __func__, err);

    return err;
}

gf_error_t gf_tee_get_object_size(GF_TEE_HANDLE so_handle, uint32_t *object_size) {

    gf_error_t err = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {

        TEE_ObjectInfo object_info = { 0 };

        if ((NULL == so_handle) || (NULL == object_size)) {
            GF_LOGE(
                    LOG_TAG "[%s] invalid parameter, so_handle:0x%p, object_size:0x%p", __func__, (void*)so_handle,(void*)object_size);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        TEE_GetObjectInfo(so_handle, &object_info);
        *object_size = object_info.dataSize;
    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit, err:0x%d", __func__, err);

    return err;
}

gf_error_t gf_tee_read_object_data(GF_TEE_HANDLE so_handle, uint8_t *buffer, uint32_t size) {

    gf_error_t err = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        uint32_t read_bytes = 0;

        if ((NULL == so_handle) || (NULL == buffer)) {
            GF_LOGE(
                    LOG_TAG "[%s] invalid parameter, so_handle:0x%p, buffer:0x%p", __func__, (void*)so_handle, (void*)buffer);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        err = TEE_ReadObjectData((TEE_ObjectHandle) so_handle, (void *) buffer, (size_t) size,
                &read_bytes);
        if ((TEE_SUCCESS != err) || (size != read_bytes)) {
            GF_LOGE(
                    LOG_TAG "[%s] read so fail, ret:0x%08X, size:%d, read_bytes:%d", __func__, err, size, read_bytes);
            err = GF_ERROR_READ_SECURE_OBJECT_FAILED;
        }

    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit, err:0x%d", __func__, err);

    return err;
}

gf_error_t gf_tee_write_object_data(GF_TEE_HANDLE so_handle, uint8_t *buffer, uint32_t size) {

    gf_error_t err = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        if ((NULL == so_handle) || (NULL == buffer) || (size == 0)) {
            GF_LOGE(
                    LOG_TAG "[%s] invalid parameter, so_handle:0x%p, buffer:0x%p, size:%d", __func__, (void*)so_handle, (void*)buffer, size);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        err = TEE_WriteObjectData((TEE_ObjectHandle) so_handle, (void *) buffer, (size_t) size);
        if (TEE_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] write so fail:0x%08X", __func__, err);
            err = GF_ERROR_WRITE_SECURE_OBJECT_FAILED;
            break;
        }
    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit, err:0x%d", __func__, err);

    return err;
}

gf_error_t gf_tee_delete_object(uint8_t *object_id) {

    gf_error_t err = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        TEE_ObjectHandle object_handle = TEE_HANDLE_NULL;

        if (NULL == object_id) {
            GF_LOGE( LOG_TAG "[%s] invalid parameter, object_id:0x%p", __func__, (void*)object_id);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        err = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE, object_id, strlen((const char*)object_id),
                TEE_DATA_FLAG_ACCESS_WRITE_META, &object_handle);
        if (TEE_SUCCESS == err) {
            TEE_CloseAndDeletePersistentObject(object_handle);
            GF_LOGD(LOG_TAG "[%s] delete so(%s) success", __func__, object_id);
        } else {
            GF_LOGD(LOG_TAG "[%s] so(%s) not exist", __func__, object_id);
        }

        err = GF_SUCCESS;
    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit, err:0x%d", __func__, err);

    return err;
}

void gf_tee_close_object(GF_TEE_HANDLE so_handle) {

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        if (NULL == so_handle) {
            GF_LOGE( LOG_TAG "[%s] invalid parameter", __func__);
            break;
        }

        TEE_CloseObject((TEE_ObjectHandle) so_handle);
    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit", __func__);
}
