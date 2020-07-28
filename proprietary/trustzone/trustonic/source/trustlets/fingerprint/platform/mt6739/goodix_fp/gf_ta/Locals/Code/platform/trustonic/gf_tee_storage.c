/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#include "taStd.h"
#include "tee_internal_api.h"
#include "stdio.h"
#include "gf_tee_internal_api.h"

#define LOG_TAG "[gf_tee_storage]"

gf_error_t gf_tee_object_exist(uint8_t *object_id) {
    gf_error_t err = GF_SUCCESS;
    TEE_Result ret = TEE_SUCCESS;
    FUNC_ENTER();

    do {
        TEE_ObjectHandle object_handle = TEE_HANDLE_NULL;

        if (NULL == object_id) {
            GF_LOGE(LOG_TAG "[%s] invalid parameter", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        ret = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE, object_id,
                strlen((const char*) object_id),
                TEE_DATA_FLAG_ACCESS_READ, &object_handle);
        if (TEE_SUCCESS != ret) {
            GF_LOGI(LOG_TAG "[%s] open so(%s) failed. ret=0x%08X", __func__, object_id, ret);
            err = GF_ERROR_SECURE_OBJECT_NOT_EXIST;
            break;
        }

        TEE_CloseObject(object_handle);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_tee_open_object(uint8_t *object_id, gf_tee_mode_t mode, GF_TEE_HANDLE *so_handle) {
    gf_error_t err = GF_SUCCESS;
    uint32_t flags = 0; /* flags for open mode */
    TEE_Result ret = TEE_SUCCESS;

    FUNC_ENTER();

    do {
        if ((NULL == object_id) || (NULL == so_handle)
                || ((SO_MODE_READ != mode) && (SO_MODE_WRITE != mode))) {
            GF_LOGE(LOG_TAG "[%s] invalid parameter, object_id=%p, mode=%d, so_handle=%p", __func__,
                    (void*)object_id, mode, (void*)so_handle);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        /* set the flags based on operation mode */
        if (SO_MODE_READ == mode) {
            flags = TEE_DATA_FLAG_ACCESS_READ;
        } else if (SO_MODE_WRITE == mode) {
            flags = TEE_DATA_FLAG_ACCESS_WRITE;
        }

        ret = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE, object_id,
                strlen((const char*) object_id),
                flags, (TEE_ObjectHandle *) so_handle);
        if (TEE_SUCCESS == ret) {
            GF_LOGI(LOG_TAG "[%s] open so(%s) success", __func__, object_id);

            if (SO_MODE_WRITE == mode) {
                ret = TEE_TruncateObjectData((TEE_ObjectHandle) *so_handle, 0);
                if (TEE_SUCCESS != ret) {
                    GF_LOGE(LOG_TAG "[%s] truncate so failed. ret=0x%08X", __func__, ret);
                    err = GF_ERROR_OPEN_SECURE_OBJECT_FAILED;
                    TEE_CloseObject((TEE_ObjectHandle) *so_handle);
                    *so_handle = TEE_HANDLE_NULL;
                    break;
                }

                GF_LOGD(LOG_TAG "[%s] truncate so to zero success", __func__);
            }

            break;
        }

        GF_LOGI(LOG_TAG "[%s] so(%s) not exist, ret=0x%08X", __func__, object_id, ret);

        if (SO_MODE_WRITE == mode) {
            ret = TEE_CreatePersistentObject(TEE_STORAGE_PRIVATE, object_id,
                    strlen((const char*) object_id),
                    flags, TEE_HANDLE_NULL, NULL, 0,
                    (TEE_ObjectHandle *) so_handle);

            if (TEE_SUCCESS == ret) {
                GF_LOGI(LOG_TAG "[%s] create new so(%s) success", __func__, object_id);
                break;
            }
            GF_LOGE(LOG_TAG "[%s] create new so(%s) failed. ret=0x%08X", __func__, object_id, ret);
        }

        err = GF_ERROR_OPEN_SECURE_OBJECT_FAILED;
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_tee_get_object_size(GF_TEE_HANDLE so_handle, uint32_t *object_size) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        TEE_ObjectInfo object_info = { 0 };

        if ((NULL == so_handle) || (NULL == object_size)) {
            GF_LOGE(LOG_TAG "[%s] invalid parameter, so_handle=%p, object_size=%p", __func__,
                    (void*)so_handle, (void*)object_size);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        TEE_GetObjectInfo(so_handle, &object_info);
        *object_size = object_info.dataSize;
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_tee_read_object_data(GF_TEE_HANDLE so_handle, uint8_t *buffer, uint32_t size) {
    gf_error_t err = GF_SUCCESS;
    TEE_Result ret = TEE_SUCCESS;

    FUNC_ENTER();

    do {
        uint32_t read_bytes = 0;

        if ((NULL == so_handle) || (NULL == buffer)) {
            GF_LOGE(LOG_TAG "[%s] invalid parameter, so_handle=%p, buffer=%p", __func__,
                    (void*)so_handle, (void*)buffer);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        ret = TEE_ReadObjectData((TEE_ObjectHandle) so_handle, (void *) buffer, (size_t) size,
                &read_bytes);
        if (TEE_SUCCESS != ret) {
            GF_LOGE(LOG_TAG "[%s] read so failed, ret=0x%08X", __func__, ret);
            err = GF_ERROR_READ_SECURE_OBJECT_FAILED;
            break;
        }
        if (size != read_bytes) {
            GF_LOGE(LOG_TAG "[%s] read so failed, size=%u, read_bytes=%u", __func__, size,
                    read_bytes);
            err = GF_ERROR_READ_SECURE_OBJECT_FAILED;
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_tee_write_object_data(GF_TEE_HANDLE so_handle, uint8_t *buffer, uint32_t size) {
    gf_error_t err = GF_SUCCESS;
    TEE_Result ret = TEE_SUCCESS;

    FUNC_ENTER();

    do {
        if ((NULL == so_handle) || (NULL == buffer) || (size == 0)) {
            GF_LOGE(LOG_TAG "[%s] invalid parameter, so_handle=%p, buffer=%p, size=%u", __func__,
                    (void*)so_handle, (void*)buffer, size);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        ret = TEE_WriteObjectData((TEE_ObjectHandle) so_handle, (void *) buffer, (size_t) size);
        if (TEE_SUCCESS != ret) {
            GF_LOGE(LOG_TAG "[%s] write so failed. ret=0x%08X", __func__, ret);
            err = GF_ERROR_WRITE_SECURE_OBJECT_FAILED;
            break;
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_tee_delete_object(uint8_t *object_id) {
    gf_error_t err = GF_SUCCESS;
    TEE_Result ret = TEE_SUCCESS;

    FUNC_ENTER();

    do {
        TEE_ObjectHandle object_handle = TEE_HANDLE_NULL;

        if (NULL == object_id) {
            GF_LOGE(LOG_TAG "[%s] invalid parameter, object_id=%p", __func__, (void*)object_id);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        ret = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE, object_id,
                strlen((const char*) object_id), TEE_DATA_FLAG_ACCESS_WRITE_META, &object_handle);
        if (TEE_SUCCESS == ret) {
            TEE_CloseAndDeletePersistentObject(object_handle);
            GF_LOGD(LOG_TAG "[%s] delete so(%s) success", __func__, object_id);
        } else {
            GF_LOGD(LOG_TAG "[%s] so(%s) not exist", __func__, object_id);
        }

        err = GF_SUCCESS;
    } while (0);

    FUNC_EXIT(err);

    return err;
}

void gf_tee_close_object(GF_TEE_HANDLE so_handle) {
    VOID_FUNC_ENTER();

    do {
        if (NULL == so_handle) {
            GF_LOGE(LOG_TAG "[%s] invalid parameter", __func__);
            break;
        }

        TEE_CloseObject((TEE_ObjectHandle) so_handle);
    } while (0);

    VOID_FUNC_EXIT();
}
