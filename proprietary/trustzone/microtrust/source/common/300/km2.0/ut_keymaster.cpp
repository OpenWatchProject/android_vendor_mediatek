/*
 * Copyright (C) 2015-2017 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */

#define IMSG_TAG "microkeymaster"

#include <errno.h>
#include <hardware/hardware.h>
#include <hardware/keymaster2.h>
#include <hardware/keymaster_defs.h>
#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <imsg_log.h>

#include "ut_km_def.h"
#include "ut_km_ioctl.h"
#include "ut_km_api.h"

#define RSA_KEY_SO_SIZE 4096

/* Close an opened OpenSSL instance */
static int ut_km_close(hw_device_t* dev) {
    IMSG_ENTER();
    TEE_CLOSE();
    free(dev);
    // delete dev;
    return 0;
}

keymaster_error_t ut_km_get_supported_algorithms(const struct keymaster2_device* dev,
                                                 keymaster_algorithm_t** algorithms,
                                                 size_t* algorithms_length) {
    (void)dev;
    int ret = KM_ERROR_OK;
    ret = ut_get_supported_algorithms(algorithms, algorithms_length);
    return (keymaster_error_t)ret;
}

keymaster_error_t ut_km_get_supported_block_modes(const struct keymaster2_device* dev,
                                                  keymaster_algorithm_t algorithm,
                                                  keymaster_purpose_t purpose,
                                                  keymaster_block_mode_t** modes,
                                                  size_t* modes_length) {
    (void)dev;
    keymaster_error_t ret = KM_ERROR_OK;
    ret = ut_get_supported_block_modes(algorithm, purpose, modes, modes_length);
    return ret;
}

keymaster_error_t ut_km_get_supported_padding_modes(const struct keymaster2_device* dev,
                                                    keymaster_algorithm_t algorithm,
                                                    keymaster_purpose_t purpose,
                                                    keymaster_padding_t** modes,
                                                    size_t* modes_length) {
    (void)dev;
    keymaster_error_t ret = KM_ERROR_OK;
    ret = ut_get_supported_padding_modes(algorithm, purpose, modes, modes_length);
    return ret;
}

keymaster_error_t ut_km_get_supported_digests(const struct keymaster2_device* dev,
                                              keymaster_algorithm_t algorithm,
                                              keymaster_purpose_t purpose,
                                              keymaster_digest_t** digests,
                                              size_t* digests_length) {
    (void)dev;
    keymaster_error_t ret = KM_ERROR_OK;
    ret = ut_get_supported_digests(algorithm, purpose, digests, digests_length);
    return ret;
}

keymaster_error_t ut_km_get_supported_import_formats(const struct keymaster2_device* dev,
                                                     keymaster_algorithm_t algorithm,
                                                     keymaster_key_format_t** formats,
                                                     size_t* formats_length) {
    (void)dev;
    keymaster_error_t ret = KM_ERROR_OK;
    ret = ut_get_supported_import_formats(algorithm, formats, formats_length);
    return ret;
}

// ut_get_supported_export_formats
keymaster_error_t ut_km_get_supported_export_formats(const struct keymaster2_device* dev,
                                                     keymaster_algorithm_t algorithm,
                                                     keymaster_key_format_t** formats,
                                                     size_t* formats_length) {
    (void)dev;
    keymaster_error_t ret = KM_ERROR_OK;
    ret = ut_get_supported_export_formats(algorithm, formats, formats_length);
    return ret;
}

keymaster_error_t ut_km_add_rng_entropy(const struct keymaster2_device* dev, const uint8_t* data,
                                        size_t data_length) {
    (void)dev;
    (void)data;
    (void)data_length;
    return KM_ERROR_OK;
}

keymaster_error_t ut_km_generate_key(const struct keymaster2_device* dev,
                                     const keymaster_key_param_set_t* params,
                                     keymaster_key_blob_t* key_blob,
                                     keymaster_key_characteristics_t* characteristics) {
    (void)dev;
    IMSG_ENTER();
    keymaster_error_t ret = KM_ERROR_OK;

    if (characteristics == NULL) {
        ret = (keymaster_error_t)ut_generate_key(params, key_blob, NULL);
    } else {
        ret = (keymaster_error_t)ut_generate_key(params, key_blob, characteristics);
    }

    IMSG_LEAVE();

    return (keymaster_error_t)ret;
}

keymaster_error_t ut_km_get_key_characteristics(const struct keymaster2_device* dev,
                                                const keymaster_key_blob_t* key_blob,
                                                const keymaster_blob_t* client_id,
                                                const keymaster_blob_t* app_data,
                                                keymaster_key_characteristics_t* characteristics) {
    (void)dev;
    IMSG_ENTER();
    keymaster_error_t ret = KM_ERROR_OK;

    if (characteristics == NULL) {
        IMSG_ERROR("CHARACTER GET IS NULL, do not need to return");
        return KM_ERROR_UNKNOWN_ERROR;
    }

    ret = ut_get_key_characteristics((keymaster_key_blob_t*)key_blob, (keymaster_blob_t*)client_id,
                                     (keymaster_blob_t*)app_data, characteristics);

    IMSG_LEAVE();
    return ret;
}

keymaster_error_t ut_km_import_key(const struct keymaster2_device* dev,
                                   const keymaster_key_param_set_t* params,
                                   keymaster_key_format_t key_format,
                                   const keymaster_blob_t* key_data, keymaster_key_blob_t* key_blob,
                                   keymaster_key_characteristics_t* characteristics) {
    (void)dev;
    keymaster_error_t ret = KM_ERROR_OK;
    IMSG_ENTER();

    // now we alloc 50 nums keymaster_key_characteristics_t
    if (characteristics == NULL) {
        IMSG_ERROR("CHARACTER IMPORT IS NULL, do not need to return");
        ret = ut_import_key(params, key_format, key_data, key_blob, NULL);
    } else {
        ret = ut_import_key(params, key_format, key_data, key_blob, characteristics);
    }

    IMSG_LEAVE();

    //*characteristics = character.release();
    return ret;
}

keymaster_error_t ut_km_delete_key(const struct keymaster2_device* dev,
                                   const keymaster_key_blob_t* key) {
    (void)dev;
    (void)key;
    IMSG_ENTER();
    keymaster_error_t ret = KM_ERROR_OK;
    IMSG_LEAVE();
    //ret = ut_delete_key(key);
    return ret;
}

keymaster_error_t
ut_km_export_key(const struct keymaster2_device* dev, keymaster_key_format_t export_format,
                 const keymaster_key_blob_t* key_to_export, const keymaster_blob_t* client_id,
                 const keymaster_blob_t* app_data, keymaster_blob_t* export_data) {
    (void)dev;
    IMSG_ENTER();
    keymaster_error_t ret = KM_ERROR_OK;
    ret = ut_export_key(export_format, key_to_export, client_id, app_data, export_data);
    IMSG_LEAVE();
    return ret;
}

keymaster_error_t ut_km_begin(const struct keymaster2_device* dev, keymaster_purpose_t purpose,
                              const keymaster_key_blob_t* key,
                              const keymaster_key_param_set_t* in_params,
                              keymaster_key_param_set_t* out_params,
                              keymaster_operation_handle_t* operation_handle) {
    (void)dev;
    IMSG_ENTER();
    keymaster_error_t ret = KM_ERROR_OK;
    ret = (keymaster_error_t)ut_begin(purpose, key, in_params, out_params, operation_handle);
    IMSG_LEAVE();
    return ret;
}

keymaster_error_t ut_km_update(const struct keymaster2_device* dev,
                               keymaster_operation_handle_t operation_handle,
                               const keymaster_key_param_set_t* in_params,
                               const keymaster_blob_t* input, size_t* input_consumed,
                               keymaster_key_param_set_t* out_params, keymaster_blob_t* output) {
    (void)dev;
    IMSG_ENTER();
    keymaster_error_t ret = KM_ERROR_OK;

    if (in_params == NULL && out_params == NULL && output == NULL) {
        ret =
            (keymaster_error_t)ut_update(operation_handle, NULL, input, input_consumed, NULL, NULL);
    } else
        ret = (keymaster_error_t)ut_update(operation_handle, in_params, input, input_consumed,
                                           out_params, output);

    IMSG_LEAVE();
    return ret;
}


keymaster_error_t ut_km_finish(const struct keymaster2_device* dev,
                               keymaster_operation_handle_t operation_handle,
                               const keymaster_key_param_set_t* in_params,
                               const keymaster_blob_t* input, const keymaster_blob_t* signature,
                               keymaster_key_param_set_t* out_params, keymaster_blob_t* output) {
    (void)dev;
    IMSG_ENTER();
    keymaster_error_t ret = KM_ERROR_OK;

	if(input == NULL)
		IMSG_INFO("finish input is null");

    if (in_params == NULL && signature == NULL && out_params == NULL) {
        ret = (keymaster_error_t)ut_finish(operation_handle, NULL, NULL, NULL, output, input);
    } else
        ret = (keymaster_error_t)ut_finish(operation_handle, in_params, signature, out_params,
                                           output, input);

    IMSG_LEAVE();
    return ret;
}

keymaster_error_t ut_km_abort(const struct keymaster2_device* dev,
                              keymaster_operation_handle_t operation_handle) {
    (void)dev;
    IMSG_ENTER();
    keymaster_error_t ret = KM_ERROR_OK;
    // TODO return result
    ret = ut_abort(operation_handle);
    IMSG_LEAVE();
    return ret;
}

keymaster_error_t ut_km_configure(const struct keymaster2_device* dev,
                                   const keymaster_key_param_set_t* params) {
    (void)dev;
    IMSG_ENTER();
    keymaster_error_t ret = KM_ERROR_OK;
	//TODO need to realize
    ret = ut_configure(params);
    IMSG_LEAVE();
    return ret;
}

keymaster_error_t ut_km_attest_key(const struct keymaster2_device* dev,
                                    const keymaster_key_blob_t* key_to_attest,
                                    const keymaster_key_param_set_t* attest_params,
                                    keymaster_cert_chain_t* cert_chain) {
    (void)dev;
    IMSG_ENTER();
    keymaster_error_t ret = KM_ERROR_OK;
	//TODO need to realize
    ret = ut_attest_key(key_to_attest, attest_params, cert_chain);
    IMSG_LEAVE();
    return ret;
}

keymaster_error_t ut_km_upgrade_key(const struct keymaster2_device* dev,
                             const keymaster_key_blob_t* key_to_upgrade,
                             const keymaster_key_param_set_t* upgrade_params,
                             keymaster_key_blob_t* upgraded_key) {
    (void)dev;
    IMSG_ENTER();
    keymaster_error_t ret = KM_ERROR_OK;
	//TODO need to realize
    ret = ut_upgrade_key(key_to_upgrade, upgrade_params, upgraded_key);
    IMSG_LEAVE();
    return ret;
}

keymaster_error_t ut_km_delete_allkeys(const struct keymaster2_device* dev) {
    (void)dev;
    IMSG_ENTER();
    keymaster_error_t ret = KM_ERROR_OK;
    // TODO return result
    // ut_abort(operation_handle);
    return ret;
}

#ifdef SOTER
keymaster_error_t ut_km_generate_attk(const struct keymaster2_device* dev,
                                      const unsigned char copy_num) {
    (void)dev;
    IMSG_INFO("[%s][%d] start , copy_num = %d", __func__, __LINE__, copy_num);
    keymaster_error_t ret = KM_ERROR_OK;
    ret = ut_generate_attk(copy_num);
    IMSG_INFO("ut km generate attk end , ret = %d", ret);
    return ret;
}

keymaster_error_t ut_km_verify_attk(const struct keymaster2_device* dev) {
    (void)dev;
    IMSG_INFO("[%s][%d] start", __func__, __LINE__);
    keymaster_error_t ret = KM_ERROR_OK;
    ret = ut_verify_attk();
    IMSG_INFO("[%s][%d] end , ret = %d", __func__, __LINE__, ret);

    return ret;
}

keymaster_error_t ut_km_export_attk(const struct keymaster2_device* dev,
                                    const unsigned char** key_data, size_t* key_datalen) {
    (void)dev;
    IMSG_INFO("[%s][%d] start", __func__, __LINE__);
    keymaster_error_t ret = KM_ERROR_OK;
    ret = ut_export_attk(key_data, key_datalen);
    IMSG_INFO("[%s][%d] end , ret = %d , *key_datalen = %u", __func__, __LINE__, ret,
          (unsigned int)(*key_datalen));
    return ret;
}

keymaster_error_t ut_km_get_device_id(const struct keymaster2_device* dev,
                                      const unsigned char** device_id, size_t* device_idlen) {
    (void)dev;
    IMSG_INFO("[%s][%d] start", __func__, __LINE__);
    keymaster_error_t ret = KM_ERROR_OK;
    ret = ut_get_device_id(device_id, device_idlen);
    IMSG_INFO("[%s][%d] end , ret = %d ,  *device_idlen = %d", __func__, __LINE__, ret,
          (unsigned int)(*device_idlen));
    return ret;
}
#endif  // endi of SOTER

extern int km_version;
extern int android_version;

static int ut_km_open(const hw_module_t* module, const char* name, hw_device_t** device) {
    IMSG_ENTER();

    if (strcmp(name, KEYSTORE_KEYMASTER) != 0) {
        IMSG_ERROR("km open error, name=%s", name);
        return -EINVAL;
    }

    int ret = TEE_OPEN();

    if (ret != 0) {
        IMSG_ERROR("keymaster open device node failed, errno=%d", errno);
        return -ENOMEM;
    }

    keymaster2_device_t* dev = NULL;
    dev = (keymaster2_device_t*)malloc(sizeof(keymaster2_device_t));
    // memset(dev, 0, sizeof(keymaster2_device_t));

    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 2;
    dev->common.module = (struct hw_module_t*)module;
    dev->common.close = ut_km_close;
    dev->flags = 0;

    dev->add_rng_entropy = ut_km_add_rng_entropy;
    dev->generate_key = ut_km_generate_key;
    dev->get_key_characteristics = ut_km_get_key_characteristics;
    dev->import_key = ut_km_import_key;
    dev->export_key = ut_km_export_key;
    dev->delete_key = ut_km_delete_key;
    dev->delete_all_keys = ut_km_delete_allkeys;
    dev->begin = ut_km_begin;
    dev->update = ut_km_update;
    dev->finish = ut_km_finish;
    dev->abort = ut_km_abort;

	dev->configure = ut_km_configure;
	dev->attest_key = ut_km_attest_key;
	dev->upgrade_key = ut_km_upgrade_key;
#if SOTER
    dev->generate_attk_key_pair = ut_km_generate_attk;
    dev->verify_attk_key_pair = ut_km_verify_attk;
    dev->export_attk_public_key = ut_km_export_attk;
    dev->get_device_id = ut_km_get_device_id;
#endif

    *device = (hw_device_t*)dev;

    km_version = dev->common.version;
    android_version = PLATFORM_VERSION_MAJOR;

    return 0;
}

static struct hw_module_methods_t keystore_module_methods = {
    .open = ut_km_open,
};

struct keystore_module HAL_MODULE_INFO_SYM __attribute__((visibility("default"))) = {
    .common =
        {
            .tag = HARDWARE_MODULE_TAG,
            .module_api_version = KEYMASTER_MODULE_API_VERSION_2_0,
            //.module_api_version = KEYMASTER_MODULE_API_VERSION_0_2,
            .hal_api_version = HARDWARE_HAL_API_VERSION,
            .id = KEYSTORE_HARDWARE_MODULE_ID,
            .name = "Keymaster UT HAL",
            .author = "The Microtrust Keymaster Source Project",
            .methods = &keystore_module_methods,
            .dso = 0,
            .reserved = {},
        },
};
