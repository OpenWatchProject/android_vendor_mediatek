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
#include <imsg_log.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <cutils/properties.h>

#include "ut_km_def.h"
#include "ut_km_ioctl.h"
#include "ut_km_api.h"

unsigned int km_version = 0;
unsigned int android_version = 0;

uint32_t ut_append_decode_parameters(uint32_t count, void* from,
                                     keymaster_key_param_set_t* to_buffer);
uint32_t ut_append_encode_parameters(const keymaster_key_param_set_t* from, void* to_buffer);
int ut_set_param_value(keymaster_tag_t tag, size_t size, void* from, keymaster_key_param_t* param);

void ut_append_data(const uint8_t* from_buffer, void* to_buffer, uint32_t size);

/**
 * Gets algorithms supported.
 *
 * \param[out] algorithms Array of algorithms supported.  The caller takes ownership of the
 * array and must free() it.
 *
 * \param[out] algorithms_length Length of \p algorithms.
 */
keymaster_error_t ut_get_supported_algorithms(keymaster_algorithm_t** algorithms,
                                              size_t* algorithms_length) {

    int ret = 0;
    ut_message_t keymaster_msg;

    if (TEE_Alloc_KM_Buffer_And_Reset() != 0) {
        IMSG_ERROR("%s: malloc km buffer failed", __FUNCTION__);
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;
    }

    /*this command should be the supportted algorithms.*/
    keymaster_msg.command.commandId = CMD_ID_TEE_GET_SUPPORTED_ALGO;
    memcpy(keymaster_buffer, &keymaster_msg, sizeof(ut_message_t));

    ret = TEE_InvokeCommand();

    if (ret != 0) {
        IMSG_ERROR("TEE invoke command err %d", ret);
        return KM_ERROR_UNKNOWN_ERROR;
    }

    memcpy(&keymaster_msg, keymaster_buffer, sizeof(ut_message_t));

    if (keymaster_msg.response.error != KM_ERROR_OK) {
        IMSG_ERROR("TEE soter get support algo err %d", keymaster_msg.response.error);
        return keymaster_msg.response.error;
    }

    /*if the algo list is null, directly return ok.*/
    if (keymaster_msg.get_supported_algo.algorithms_length == 0) {
        *algorithms_length = keymaster_msg.get_supported_algo.algorithms_length;
        return KM_ERROR_OK;
    }

    IMSG_INFO("ut_get_supported_algorithms, %u", (unsigned int)(*algorithms_length));
    *algorithms_length = keymaster_msg.get_supported_algo.algorithms_length;
    *algorithms = malloc(*algorithms_length * sizeof(keymaster_algorithm_t));
    memcpy(*algorithms, keymaster_buffer + sizeof(ut_message_t),
           keymaster_msg.get_supported_algo.algorithms_length * sizeof(keymaster_algorithm_t));
    return KM_ERROR_OK;
}

/**
 * Gets the block modes supported for the specified algorithm.
 *
 * \param[in] dev The keymaster device structure.
 *
 * \param[in] algorithm The algorithm for which supported modes will be returned.
 *
 * \param[out] modes Array of modes supported.  The caller takes ownership of the array and must
 * free() it.
 *
 * \param[out] modes_length Length of \p modes.
 */
keymaster_error_t ut_get_supported_block_modes(keymaster_algorithm_t algorithm,
                                               keymaster_purpose_t purpose,
                                               keymaster_block_mode_t** modes,
                                               size_t* modes_length) {
    int ret = 0;
    ut_message_t keymaster_msg;

    IMSG_ENTER();

    if (TEE_Alloc_KM_Buffer_And_Reset() != 0) {
        IMSG_ERROR("%s: malloc km buffer failed", __FUNCTION__);
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;
    }

    /*this command should be the supportted algorithms.*/
    keymaster_msg.command.commandId = CMD_ID_TEE_GET_SUPPORTED_BLOCK;
    keymaster_msg.get_supported_block.algo = algorithm;
    keymaster_msg.get_supported_block.purpose = purpose;
    memcpy(keymaster_buffer, &keymaster_msg, sizeof(ut_message_t));

    ret = TEE_InvokeCommand();

    if (ret != 0) {
        IMSG_ERROR("TEE invoke command err %d", ret);
        return KM_ERROR_UNKNOWN_ERROR;
    }

    memcpy(&keymaster_msg, keymaster_buffer, sizeof(ut_message_t));

    if (keymaster_msg.response.error != KM_ERROR_OK) {
        IMSG_ERROR("TEE soter get support algo err %d", keymaster_msg.response.error);
        return keymaster_msg.response.error;
    }

    /*if the algo list is null, directly return ok.*/
    if (keymaster_msg.get_supported_block.modes_length == 0) {
        *modes_length = keymaster_msg.get_supported_block.modes_length;
        return KM_ERROR_OK;
    }

    *modes_length = keymaster_msg.get_supported_block.modes_length;
    *modes = malloc(*modes_length * sizeof(keymaster_block_mode_t));
    memcpy(*modes, keymaster_buffer + sizeof(ut_message_t),
           keymaster_msg.get_supported_block.modes_length * sizeof(keymaster_block_mode_t));
    return ret;
}

/**
 * Gets the padding modes supported for the specified algorithm.  Caller assumes ownership of
 * the allocated array.
 *
 * \param[in] algorithm The algorithm for which supported padding modes will be returned.
 *
 * \param[out] modes Array of padding modes supported.  The caller takes ownership of the
 * array
 * and must free() it.
 *
 * \param[out] modes_length Length of \p modes.
 */
keymaster_error_t ut_get_supported_padding_modes(keymaster_algorithm_t algorithm,
                                                 keymaster_purpose_t purpose,
                                                 keymaster_padding_t** modes,
                                                 size_t* modes_length) {

    int ret = 0;
    ut_message_t keymaster_msg;

    IMSG_ENTER();

    if (TEE_Alloc_KM_Buffer_And_Reset() != 0) {
        IMSG_ERROR("%s: malloc km buffer failed", __FUNCTION__);
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;
    }

    /*this command should be the supportted algorithms.*/
    keymaster_msg.command.commandId = CMD_ID_TEE_GET_SUPPORTED_PADDING;
    keymaster_msg.get_supported_padding.algo = algorithm;
    keymaster_msg.get_supported_padding.purpose = purpose;
    memcpy(keymaster_buffer, &keymaster_msg, sizeof(ut_message_t));

    ret = TEE_InvokeCommand();

    if (ret != 0) {
        IMSG_ERROR("TEE invoke command err %d", ret);
        return KM_ERROR_UNKNOWN_ERROR;
    }

    memcpy(&keymaster_msg, keymaster_buffer, sizeof(ut_message_t));

    if (keymaster_msg.response.error != KM_ERROR_OK) {
        IMSG_ERROR("TEE soter get support algo err %d", keymaster_msg.response.error);
        return keymaster_msg.response.error;
    }

    /*if the algo list is null, directly return ok.*/
    if (keymaster_msg.get_supported_padding.modes_length == 0) {
        *modes_length = keymaster_msg.get_supported_padding.modes_length;
        return KM_ERROR_OK;
    }

    *modes_length = keymaster_msg.get_supported_padding.modes_length;
    *modes = malloc(*modes_length * sizeof(keymaster_padding_t));
    memcpy(*modes, keymaster_buffer + sizeof(ut_message_t),
           keymaster_msg.get_supported_padding.modes_length * sizeof(keymaster_padding_t));
    return ret;
}

/**
 * Gets the digests supported for the specified algorithm.  Caller assumes ownership of the
 * allocated array.
 *
 * \param[in] dev The keymaster device structure.
 *
 * \param[in] algorithm The algorithm for which supported digests will be returned.
 *
 * \param[out] digests Array of digests supported.  The caller takes ownership of the array and
 * must free() it.
 *
 * \param[out] digests_length Length of \p digests.
 */
keymaster_error_t ut_get_supported_digests(keymaster_algorithm_t algorithm,
                                           keymaster_purpose_t purpose,
                                           keymaster_digest_t** digests, size_t* digests_length) {
    int ret = 0;
    keymaster_digest_t digest_mode[] = {
        KM_DIGEST_NONE,      KM_DIGEST_MD5,       KM_DIGEST_SHA1,     KM_DIGEST_SHA_2_224,
        KM_DIGEST_SHA_2_256, KM_DIGEST_SHA_2_384, KM_DIGEST_SHA_2_512};

    if (purpose == KM_PURPOSE_ENCRYPT || purpose == KM_PURPOSE_DECRYPT ||
        purpose == KM_PURPOSE_SIGN || purpose == KM_PURPOSE_VERIFY) {
        if (algorithm == KM_ALGORITHM_AES) {
            *digests_length = 0;
            return 0;
        } else {
            *digests_length = sizeof(digest_mode);
            *digests = (keymaster_digest_t*)malloc(sizeof(digest_mode));
            memcpy(*digests, digest_mode, sizeof(digest_mode));
            return 0;
        }
    } else
        return KM_ERROR_INCOMPATIBLE_PURPOSE;
}

/**
 * Gets the key import formats supported for keys of the specified algorithm.  Caller assumes
 * ownership of the allocated array.
 *
 * \param[in] algorithm The algorithm for which supported formats will be returned.
 *
 * \param[out] formats Array of formats supported.  The caller takes ownership of the array and
 * must free() it.
 *
 * \param[out] formats_length Length of \p formats.
 */
keymaster_error_t ut_get_supported_import_formats(keymaster_algorithm_t algorithm,
                                                  keymaster_key_format_t** formats,
                                                  size_t* formats_length) {
    int ret = 0;
    ut_message_t keymaster_msg;

    IMSG_ENTER();

    if (TEE_Alloc_KM_Buffer_And_Reset() != 0) {
        IMSG_ERROR("%s: malloc km buffer failed", __FUNCTION__);
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;
    }

    /*this command should be the supportted algorithms.*/
    keymaster_msg.command.commandId = CMD_ID_TEE_GET_SUPPORTED_IMPORT_FORMATS;
    keymaster_msg.get_import_format.algo = algorithm;
    memcpy(keymaster_buffer, &keymaster_msg, sizeof(ut_message_t));
    ret = TEE_InvokeCommand();

    if (ret != 0) {
        IMSG_ERROR("TEE invoke command err %d", ret);
        return KM_ERROR_UNKNOWN_ERROR;
    }

    memcpy(&keymaster_msg, keymaster_buffer, sizeof(ut_message_t));

    if (keymaster_msg.response.error != KM_ERROR_OK) {
        IMSG_ERROR("TEE soter get support algo err %d", keymaster_msg.response.error);
        return keymaster_msg.response.error;
    }

    if (keymaster_msg.get_import_format.formats_length == 0) {
        *formats_length = keymaster_msg.get_import_format.formats_length;
        return KM_ERROR_OK;
    }

    *formats_length = keymaster_msg.get_import_format.formats_length;
    *formats = malloc(*formats_length * sizeof(keymaster_key_format_t));
    memcpy(*formats, keymaster_buffer + sizeof(ut_message_t),
           keymaster_msg.get_import_format.formats_length * sizeof(keymaster_key_format_t));
    return ret;
}

keymaster_error_t ut_get_supported_export_formats(keymaster_algorithm_t algorithm,
                                                  keymaster_key_format_t** formats,
                                                  size_t* formats_length) {

    int ret = 0;
    ut_message_t keymaster_msg;

    IMSG_ENTER();

    if (TEE_Alloc_KM_Buffer_And_Reset() != 0) {
        IMSG_ERROR("%s: malloc km buffer failed", __FUNCTION__);
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;
    }

    /*this command should be the supportted algorithms.*/
    keymaster_msg.command.commandId = CMD_ID_TEE_GET_SUPPORTED_EXPORT_FORMATS;
    keymaster_msg.get_export_format.algo = algorithm;

    memcpy(keymaster_buffer, &keymaster_msg, sizeof(ut_message_t));

    ret = TEE_InvokeCommand();

    if (ret != 0) {
        IMSG_ERROR("TEE invoke command err %d", ret);
        return KM_ERROR_UNKNOWN_ERROR;
    }

    memcpy(&keymaster_msg, keymaster_buffer, sizeof(ut_message_t));

    if (keymaster_msg.response.error != KM_ERROR_OK) {
        IMSG_ERROR("TEE soter get support algo err %d", keymaster_msg.response.error);
        return keymaster_msg.response.error;
    }

    if (keymaster_msg.get_export_format.formats_length == 0) {
        *formats_length = keymaster_msg.get_export_format.formats_length;
        return KM_ERROR_OK;
    }

    *formats_length = keymaster_msg.get_export_format.formats_length;
    *formats = malloc(*formats_length * sizeof(keymaster_key_format_t));
    memcpy(*formats, keymaster_buffer + sizeof(ut_message_t),
           keymaster_msg.get_export_format.formats_length * sizeof(keymaster_key_format_t));
    return ret;
}
#ifdef SOTER

keymaster_error_t ut_generate_attk(const unsigned char copy_num) {
    uint32_t retVal = 0;
    ut_message_t kmstr_msg;

    if (TEE_Alloc_KM_Buffer_And_Reset() != 0) {
        IMSG_ERROR("%s: malloc km buffer failed", __FUNCTION__);
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;
    }

    memset(&kmstr_msg, 0, sizeof(ut_message_t));
    kmstr_msg.command.commandId = CMD_ID_TEE_GENERATE_ATTK;
    kmstr_msg.generate_attk.copy_num = copy_num;

    memcpy(keymaster_buffer, &kmstr_msg, sizeof(ut_message_t));

    retVal = TEE_InvokeCommand();

    if (retVal != 0) {
        IMSG_ERROR("TEE invoke command err %d", retVal);
        retVal = -1;
        goto end;
    }

    memcpy(&kmstr_msg, keymaster_buffer, sizeof(ut_message_t));

    if (kmstr_msg.response.error != KM_ERROR_OK) {
        IMSG_ERROR("keymaster generate attk failed %d", kmstr_msg.response.error);
        retVal = -1;
        goto end;
    }

    retVal = KM_ERROR_OK;
end:
    return retVal;
}

keymaster_error_t ut_verify_attk(void) {
    uint32_t retVal = 0;
    ut_message_t kmstr_msg;

    if (TEE_Alloc_KM_Buffer_And_Reset() != 0) {
        IMSG_ERROR("%s: malloc km buffer failed", __FUNCTION__);
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;
    }

    memset(&kmstr_msg, 0, sizeof(ut_message_t));
    kmstr_msg.command.commandId = CMD_ID_TEE_VERIFY_ATTK;
    memcpy(keymaster_buffer, &kmstr_msg, sizeof(ut_message_t));

    retVal = TEE_InvokeCommand();

    if (retVal != 0) {
        retVal = -1;
        IMSG_ERROR("TEE invoke command err %d", retVal);
        goto end;
    }

    memcpy(&kmstr_msg, keymaster_buffer, sizeof(ut_message_t));

    if (kmstr_msg.response.error != KM_ERROR_OK) {
        retVal = -1;
        IMSG_ERROR("keymaster verify attk failed %d", kmstr_msg.response.error);
        goto end;
    }

    retVal = KM_ERROR_OK;

end:
    return (keymaster_error_t)retVal;
}

keymaster_error_t ut_export_attk(const unsigned char** key_data, size_t* key_datalen) {
    int ret = 0;
    ut_message_t keymaster_msg;

    if (TEE_Alloc_KM_Buffer_And_Reset() != 0) {
        IMSG_ERROR("%s: malloc km buffer failed", __FUNCTION__);
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;
    }

    keymaster_msg.command.commandId = CMD_ID_TEE_EXPORT_ATTK;
    memcpy(keymaster_buffer, &keymaster_msg, sizeof(ut_message_t));

    ret = TEE_InvokeCommand();

    if (ret != 0) {
        IMSG_ERROR("TEE invoke command err %d", ret);
        ret = -1;
        goto end;
    }

    memcpy(&keymaster_msg, keymaster_buffer, sizeof(ut_message_t));

    if (keymaster_msg.response.error != KM_ERROR_OK) {
        ret = -1;
        IMSG_ERROR("[%s]TEE_InvokeCommand response failed , error=%d", __func__,
              keymaster_msg.response.error);
        goto end;
    }

    *key_datalen = keymaster_msg.export_attk.attklen;
    *key_data = malloc(*key_datalen);
    memset(key_data, 0, *key_datalen);
    memcpy((unsigned char*)(*key_data), keymaster_buffer + sizeof(ut_message_t), *key_datalen);
    ret = KM_ERROR_OK;
end:
    return ret;
}

keymaster_error_t ut_get_device_id(const unsigned char** device_id, size_t* device_idlen) {
    int ret = 0;
    ut_message_t keymaster_msg;

    if (TEE_Alloc_KM_Buffer_And_Reset() != 0) {
        IMSG_ERROR("%s: malloc km buffer failed", __FUNCTION__);
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;
    }

    keymaster_msg.command.commandId = CMD_ID_TEE_GET_DEVICE_ID;
    memcpy(keymaster_buffer, &keymaster_msg, sizeof(ut_message_t));

    ret = TEE_InvokeCommand();

    if (ret != 0) {

        IMSG_ERROR("TEE invoke command err %d", ret);
        ret = -1;
        goto end;
    }

    memcpy(&keymaster_msg, keymaster_buffer, sizeof(ut_message_t));

    if (keymaster_msg.response.error != KM_ERROR_OK) {
        ret = -1;
        IMSG_ERROR("[%s]TEE_InvokeCommand response failed ,  error=%d", __func__,
              keymaster_msg.response.error);
        goto end;
    }

    *device_idlen = keymaster_msg.get_device_id.idlen;
    *device_id = malloc(*device_idlen);
    memset(device_id, 0, *device_idlen);
    memcpy((unsigned char*)(*device_id), keymaster_buffer + sizeof(ut_message_t), *device_idlen);
    ret = KM_ERROR_OK;

end:
    return ret;
}

#endif

int ut_generate_key(const keymaster_key_param_set_t* params, keymaster_key_blob_t* key_blob,
                    keymaster_key_characteristics_t* characteristics)

{
    IMSG_ENTER();
    uint32_t retVal = 0;
    int isOpen = 0;
    uint8_t* p_data_buff = 0;
    uint32_t in_params_size = 0;
    ut_message_t kmstr_msg;

    if (TEE_Alloc_KM_Buffer_And_Reset() != 0) {
        IMSG_ERROR("%s: malloc km buffer failed", __FUNCTION__);
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;
    }

    memset(&kmstr_msg, 0, sizeof(ut_message_t));
    p_data_buff = keymaster_buffer + sizeof(ut_message_t);
    kmstr_msg.command.commandId = CMD_ID_TEE_GENERATE_KEY;
    kmstr_msg.generate_key.android_version = android_version;
    kmstr_msg.generate_key.km_version = km_version;
    kmstr_msg.generate_key.in_params_length = params->length;

    in_params_size = ut_append_encode_parameters(params, p_data_buff);
    kmstr_msg.generate_key.all_block_length = in_params_size;
    memcpy(keymaster_buffer, &kmstr_msg, sizeof(ut_message_t));
    retVal = TEE_InvokeCommand();

    if (retVal != 0) {
        IMSG_ERROR("TEE invoke command err %d", retVal);
        return -1;
    }

    memcpy(&kmstr_msg, keymaster_buffer, sizeof(ut_message_t));

    if (kmstr_msg.response.error != KM_ERROR_OK) {
        IMSG_ERROR("keymaster generate key failed %d", kmstr_msg.response.error);
        return kmstr_msg.response.error;
    }

    p_data_buff = keymaster_buffer + sizeof(ut_message_t);

    if (characteristics == NULL) {
        key_blob->key_material_size = kmstr_msg.generate_key.key_material_size;
        key_blob->key_material = (uint8_t*)malloc(key_blob->key_material_size * sizeof(uint8_t));
        memcpy((unsigned char*)(key_blob->key_material), keymaster_buffer + sizeof(ut_message_t),
               key_blob->key_material_size * sizeof(uint8_t));
        return retVal;
    }

    key_blob->key_material_size = kmstr_msg.generate_key.key_material_size;
    characteristics->hw_enforced.length = kmstr_msg.generate_key.hw_params_length;
    characteristics->sw_enforced.length = kmstr_msg.generate_key.sw_params_length;
    /*malloc a space to save the key_material*/
    key_blob->key_material = (uint8_t*)malloc(key_blob->key_material_size * sizeof(uint8_t));
    characteristics->hw_enforced.params = (keymaster_key_param_t*)malloc(
        characteristics->hw_enforced.length * sizeof(keymaster_key_param_t));
    characteristics->sw_enforced.params = (keymaster_key_param_t*)malloc(
        characteristics->sw_enforced.length * sizeof(keymaster_key_param_t));

    memcpy((unsigned char*)(key_blob->key_material), keymaster_buffer + sizeof(ut_message_t),
           key_blob->key_material_size * sizeof(uint8_t));
    p_data_buff += kmstr_msg.generate_key.key_material_size;

    in_params_size = ut_append_decode_parameters(characteristics->hw_enforced.length, p_data_buff,
                                                 &characteristics->hw_enforced);
    p_data_buff += in_params_size;

    in_params_size = ut_append_decode_parameters(characteristics->sw_enforced.length, p_data_buff,
                                                 &characteristics->sw_enforced);
    retVal = kmstr_msg.response.error;
    return retVal;
}

keymaster_error_t ut_configure(const keymaster_key_param_set_t* params)
{
    IMSG_ENTER();
    uint32_t retVal = 0;
    uint8_t* p_data_buff = 0;
    uint32_t in_params_size = 0;
    key_status_t key_status;
    ut_message_t kmstr_msg;

    if (TEE_Alloc_KM_Buffer_And_Reset() != 0) {
        IMSG_ERROR("%s: malloc km buffer failed", __FUNCTION__);
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;
    }

    memset(&key_status, 0, sizeof(key_status_t));
    memset(&kmstr_msg, 0, sizeof(ut_message_t));
    p_data_buff = keymaster_buffer + sizeof(ut_message_t);
    kmstr_msg.command.commandId = CMD_ID_TEE_CONFIGURE;
    kmstr_msg.configure.android_version = android_version;
    kmstr_msg.configure.km_version = km_version;
    kmstr_msg.configure.key_material_size = params->length;

    in_params_size = ut_append_encode_parameters(params, p_data_buff);
    memcpy(keymaster_buffer, &kmstr_msg, sizeof(ut_message_t));
    retVal = TEE_InvokeCommand();

    if (retVal != 0) {
        IMSG_ERROR("TEE invoke command err %d", retVal);
        return -1;
    }

    memcpy(&kmstr_msg, keymaster_buffer, sizeof(ut_message_t));
    memcpy(&key_status, keymaster_buffer + sizeof(ut_message_t),
          sizeof(key_status_t));

    IMSG_INFO("km magic=%x", key_status.google_key_staus);
    if(key_status.google_key_staus == KM_MAGIC)
      property_set("vendor.soter.teei.googlekey.status", "OK");
    else
      property_set("vendor.soter.teei.googlekey.status", "FAIL");

    if (kmstr_msg.response.error != KM_ERROR_OK) {
        IMSG_ERROR("keymaster configure failed %d", kmstr_msg.response.error);
        return kmstr_msg.response.error;
    }

    retVal = kmstr_msg.response.error;
    return retVal;
}

keymaster_error_t ut_attest_key(const keymaster_key_blob_t* key_to_attest,
                                const keymaster_key_param_set_t* attest_params,
                                keymaster_cert_chain_t* cert_chain)
{
    int ret = 0;
    uint8_t* p_data_buff = 0;
    uint32_t in_params_size = 0;
    ut_message_t keymaster_msg;

    if (TEE_Alloc_KM_Buffer_And_Reset() != 0) {
        IMSG_ERROR("%s: malloc km buffer failed", __FUNCTION__);
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;
    }

	memset(&keymaster_msg, 0, sizeof(ut_message_t));
    /*this command should be the supportted algorithms. */
    keymaster_msg.command.commandId = CMD_ID_TEE_ATTEST_KEY;
    keymaster_msg.attest_key.android_version = android_version;
    keymaster_msg.attest_key.km_version = km_version;
    keymaster_msg.attest_key.key_material_size = key_to_attest->key_material_size;
    keymaster_msg.attest_key.in_params_length = attest_params->length;
    /* IMSG_ERROR("key_data->data_length=%d", key_data->data_length); */
    memcpy(keymaster_buffer, &keymaster_msg, sizeof(ut_message_t));

    p_data_buff = (uint8_t*)keymaster_buffer + sizeof(ut_message_t);
	memcpy(p_data_buff, key_to_attest->key_material, key_to_attest->key_material_size);
	p_data_buff = p_data_buff + key_to_attest->key_material_size;
    in_params_size = ut_append_encode_parameters(attest_params, p_data_buff);

    ret = TEE_InvokeCommand();

    if (ret != 0) {
        IMSG_ERROR("attest_key cmd err %d", ret);
        return KM_ERROR_UNKNOWN_ERROR;
    }

    memcpy(&keymaster_msg, keymaster_buffer, sizeof(ut_message_t));

    if (keymaster_msg.response.error != 0) {
        IMSG_ERROR("attest_key  err %d", keymaster_msg.response.error);
        return keymaster_msg.response.error;
    }

    if (cert_chain != NULL) {
		p_data_buff = (uint8_t*)keymaster_buffer + sizeof(ut_message_t);
		cert_chain->entry_count = keymaster_msg.attest_key.out_entry_count;
		cert_chain->entries = malloc((sizeof(keymaster_blob_t)) * cert_chain->entry_count);
		for(unsigned int i = 0; i < cert_chain->entry_count; i++) {
			cert_chain->entries[i].data_length = keymaster_msg.attest_key.datalen[i];
			cert_chain->entries[i].data = malloc(cert_chain->entries[i].data_length);
			memcpy((unsigned char *)cert_chain->entries[i].data,
				p_data_buff,
				cert_chain->entries[i].data_length);
			p_data_buff += cert_chain->entries[i].data_length;
		}
    }

    return ret;
}

keymaster_error_t ut_upgrade_key(const keymaster_key_blob_t* key_to_upgrade,
                             const keymaster_key_param_set_t* upgrade_params,
                             keymaster_key_blob_t* upgraded_key)
{
    int ret = 0;
    uint8_t* p_data_buff = 0;
    uint32_t in_params_size = 0;
    ut_message_t keymaster_msg;

    if (TEE_Alloc_KM_Buffer_And_Reset() != 0) {
        IMSG_ERROR("%s: malloc km buffer failed", __FUNCTION__);
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;
    }

    /*this command should be the supportted algorithms. */
    keymaster_msg.command.commandId = CMD_ID_TEE_UPGRADE_KEY;
    keymaster_msg.upgrade_key.android_version = android_version;
    keymaster_msg.upgrade_key.km_version = km_version;
    keymaster_msg.upgrade_key.key_material_size = key_to_upgrade->key_material_size;
    keymaster_msg.upgrade_key.in_params_length = upgrade_params->length;
    /* IMSG_ERROR("key_data->data_length=%d", key_data->data_length); */
    memcpy(keymaster_buffer, &keymaster_msg, sizeof(ut_message_t));
    p_data_buff = (uint8_t*)keymaster_buffer + sizeof(ut_message_t);
	memcpy(p_data_buff, key_to_upgrade->key_material, key_to_upgrade->key_material_size);
	p_data_buff += key_to_upgrade->key_material_size;
    in_params_size = ut_append_encode_parameters(upgrade_params, p_data_buff);
    p_data_buff = (uint8_t*)keymaster_buffer + sizeof(ut_message_t) + in_params_size;

    ret = TEE_InvokeCommand();

    if (ret != 0) {
        IMSG_ERROR("import_key cmd err %d", ret);
        return KM_ERROR_UNKNOWN_ERROR;
    }

    memcpy(&keymaster_msg, keymaster_buffer, sizeof(ut_message_t));

    if (keymaster_msg.response.error != 0) {
        IMSG_ERROR("import_key  err %d", keymaster_msg.response.error);
        return keymaster_msg.response.error;
    }

	if (upgraded_key != NULL) {
		p_data_buff = (uint8_t*)keymaster_buffer + sizeof(ut_message_t);
		upgraded_key->key_material_size = keymaster_msg.upgrade_key.out_keyname_length;
		upgraded_key->key_material = malloc(keymaster_msg.upgrade_key.out_keyname_length);
		memcpy((unsigned char *)(upgraded_key->key_material), p_data_buff, upgraded_key->key_material_size);
	}

    return ret;

}


keymaster_error_t ut_get_key_characteristics(keymaster_key_blob_t* key_blob,
                                             keymaster_blob_t* client_id,
                                             keymaster_blob_t* app_data,
                                             keymaster_key_characteristics_t* characteristics) {
    int ret = 0;
    unsigned char* p_data_buff;
    int in_params_size;
    ut_message_t keymaster_msg;

    if (TEE_Alloc_KM_Buffer_And_Reset() != 0) {
        IMSG_ERROR("%s: malloc km buffer failed", __FUNCTION__);
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;
    }

    /* this command should be the supportted algorithms. */
    memset(&keymaster_msg, 0, sizeof(ut_message_t));
    keymaster_msg.command.commandId = CMD_ID_TEE_GET_KEY_CHARACTERISTICS;
    keymaster_msg.get_key_character.android_version = android_version;
    keymaster_msg.get_key_character.km_version = km_version;
    keymaster_msg.get_key_character.key_material_size = key_blob->key_material_size;

    memcpy(keymaster_buffer + sizeof(ut_message_t), key_blob->key_material,
           key_blob->key_material_size * sizeof(uint8_t));

    if (client_id != NULL && app_data != NULL) {
        IMSG_INFO("get character need to transfer client data and id");
        keymaster_msg.get_key_character.client_id_data_length = client_id->data_length;
        keymaster_msg.get_key_character.app_data_length = app_data->data_length;
        memcpy(keymaster_buffer + sizeof(ut_message_t) +
                   key_blob->key_material_size * sizeof(uint8_t),
               client_id->data, client_id->data_length * sizeof(uint8_t));

        memcpy(keymaster_buffer + sizeof(ut_message_t) + key_blob->key_material_size +
                   client_id->data_length,
               app_data->data, app_data->data_length * sizeof(uint8_t));

    } else {
        keymaster_msg.get_key_character.client_id_data_length = 0;
        keymaster_msg.get_key_character.app_data_length = 0;
    }

    memcpy(keymaster_buffer, &keymaster_msg, sizeof(ut_message_t));
    ret = TEE_InvokeCommand();
    p_data_buff = keymaster_buffer + sizeof(ut_message_t);

    if (ret != 0) {

        IMSG_ERROR("ut_get_key_characteristics cmd err %d", ret);
        return KM_ERROR_UNKNOWN_ERROR;
    }

    memcpy(&keymaster_msg, keymaster_buffer, sizeof(ut_message_t));

    if (keymaster_msg.response.error != 0) {
        IMSG_ERROR("get characteristics  err %d", keymaster_msg.response.error);
        return keymaster_msg.response.error;
    }


    characteristics->hw_enforced.length = keymaster_msg.get_key_character.hw_length;
    characteristics->sw_enforced.length = keymaster_msg.get_key_character.sw_length;

    characteristics->hw_enforced.params = (keymaster_key_param_t*)malloc(
        characteristics->hw_enforced.length * sizeof(keymaster_key_param_t));
    characteristics->sw_enforced.params = (keymaster_key_param_t*)malloc(
        characteristics->sw_enforced.length * sizeof(keymaster_key_param_t));

    in_params_size = ut_append_decode_parameters(characteristics->hw_enforced.length, p_data_buff,
                                                 &characteristics->hw_enforced);
    p_data_buff += in_params_size;

    in_params_size = ut_append_decode_parameters(characteristics->sw_enforced.length, p_data_buff,
                                                 &characteristics->sw_enforced);
    return ret;
}

keymaster_error_t ut_import_key(const keymaster_key_param_set_t* params,
                                keymaster_key_format_t key_format, const keymaster_blob_t* key_data,
                                keymaster_key_blob_t* key_blob,
                                keymaster_key_characteristics_t* characteristics) {

    int ret = 0;
    uint8_t* p_data_buff = 0;
    uint32_t in_params_size = 0;
    ut_message_t keymaster_msg;

    if (TEE_Alloc_KM_Buffer_And_Reset() != 0) {
        IMSG_ERROR("%s: malloc km buffer failed", __FUNCTION__);
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;
    }

    /*this command should be the supportted algorithms. */
    memset(&keymaster_msg, 0, sizeof(ut_message_t));
    keymaster_msg.command.commandId = CMD_ID_TEE_IMPORT_KEY;
    keymaster_msg.import_key.android_version = android_version;
    keymaster_msg.import_key.km_version = km_version;
    keymaster_msg.import_key.params_length = params->length;
    keymaster_msg.import_key.key_format = key_format;
    keymaster_msg.import_key.key_data_length = key_data->data_length;
    /* IMSG_ERROR("key_data->data_length=%d", key_data->data_length); */
    memcpy(keymaster_buffer, &keymaster_msg, sizeof(ut_message_t));
    p_data_buff = (uint8_t*)keymaster_buffer + sizeof(ut_message_t);
    in_params_size = ut_append_encode_parameters(params, p_data_buff);
    p_data_buff = (uint8_t*)keymaster_buffer + sizeof(ut_message_t) + in_params_size;
    memcpy(p_data_buff, key_data->data, key_data->data_length);

    ret = TEE_InvokeCommand();

    if (ret != 0) {
        IMSG_ERROR("import_key cmd err %d", ret);
        return KM_ERROR_UNKNOWN_ERROR;
    }

    memcpy(&keymaster_msg, keymaster_buffer, sizeof(ut_message_t));

    if (keymaster_msg.response.error != 0) {
        IMSG_ERROR("import_key  err %d", keymaster_msg.response.error);
        return keymaster_msg.response.error;
    }

    key_blob->key_material_size = keymaster_msg.import_key.key_material_size;
    /*malloc a space to save the key_material, does the key_material is free on the up layer */
    key_blob->key_material = (uint8_t*)malloc(key_blob->key_material_size * sizeof(uint8_t));
    memcpy((unsigned char*)key_blob->key_material, keymaster_buffer + sizeof(ut_message_t),
           key_blob->key_material_size * sizeof(uint8_t));

    if (characteristics != NULL) {
        characteristics->hw_enforced.length = keymaster_msg.import_key.hw_length;
        characteristics->sw_enforced.length = keymaster_msg.import_key.sw_length;
        characteristics->hw_enforced.params = (keymaster_key_param_t*)malloc(
            characteristics->hw_enforced.length * sizeof(keymaster_key_param_t));
        characteristics->sw_enforced.params = (keymaster_key_param_t*)malloc(
            characteristics->sw_enforced.length * sizeof(keymaster_key_param_t));
        p_data_buff = keymaster_buffer + sizeof(ut_message_t) + key_blob->key_material_size;
        in_params_size = ut_append_decode_parameters(characteristics->hw_enforced.length,
                                                     p_data_buff, &characteristics->hw_enforced);
        p_data_buff += in_params_size;
        in_params_size = ut_append_decode_parameters(characteristics->sw_enforced.length,
                                                     p_data_buff, &characteristics->sw_enforced);
    }

    return ret;
}

keymaster_error_t ut_export_key(keymaster_key_format_t export_format,
                                const keymaster_key_blob_t* key_to_export,
                                const keymaster_blob_t* client_id, const keymaster_blob_t* app_data,
                                keymaster_blob_t* export_data)

{
    int ret = 0;
    ut_message_t keymaster_msg;

    IMSG_ENTER();

    if (TEE_Alloc_KM_Buffer_And_Reset() != 0) {
        IMSG_ERROR("%s: malloc km buffer failed", __FUNCTION__);
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;
    }

    /* this command should be the supportted algorithms. */
    keymaster_msg.command.commandId = CMD_ID_TEE_EXPORT_KEY;
    keymaster_msg.export_key.android_version = android_version;
    keymaster_msg.export_key.km_version = km_version;
    keymaster_msg.export_key.export_format = export_format;

    keymaster_msg.export_key.key_material_size = key_to_export->key_material_size;
    memcpy(keymaster_buffer + sizeof(ut_message_t), key_to_export->key_material,
           key_to_export->key_material_size * sizeof(uint8_t));

    if (client_id != NULL && app_data != NULL) {
        keymaster_msg.export_key.client_id_data_length = client_id->data_length;
        keymaster_msg.export_key.app_data_length = app_data->data_length;
        if (client_id->data_length != 0)
            memcpy(keymaster_buffer + sizeof(ut_message_t) +
                   key_to_export->key_material_size * sizeof(uint8_t),
               client_id->data, client_id->data_length * sizeof(uint8_t));
        if( app_data->data_length != 0)
            memcpy(keymaster_buffer + sizeof(ut_message_t) + key_to_export->key_material_size +
                   client_id->data_length,
               app_data->data, app_data->data_length * sizeof(uint8_t));
    } else {
        keymaster_msg.export_key.client_id_data_length = 0;
        keymaster_msg.export_key.app_data_length = 0;
    }
    memcpy(keymaster_buffer, &keymaster_msg, sizeof(ut_message_t));

    ret = TEE_InvokeCommand();

    if (ret != 0) {
        IMSG_ERROR("ut_get_key_characteristics cmd err %d", ret);
        return KM_ERROR_UNKNOWN_ERROR;
    }

    memcpy(&keymaster_msg, keymaster_buffer, sizeof(ut_message_t));

    if (keymaster_msg.response.error != 0) {
        IMSG_ERROR("export_key  err %d", keymaster_msg.response.error);
        return keymaster_msg.response.error;
    }

    export_data->data_length = keymaster_msg.export_key.export_data_length;
    export_data->data = (uint8_t*)malloc(export_data->data_length * sizeof(uint8_t));
    memcpy((unsigned char*)export_data->data, keymaster_buffer + sizeof(ut_message_t),
           export_data->data_length * sizeof(uint8_t));
    return ret;
}
/**
 * ut_delete_key
 *
 *
 *
 *
 * @param  key              [in]
 */
keymaster_error_t ut_delete_key(const keymaster_key_blob_t* key) {
    int ret;
    ut_message_t kmstr_msg;

    if (TEE_Alloc_KM_Buffer_And_Reset() != 0) {
        IMSG_ERROR("%s: malloc km buffer failed", __FUNCTION__);
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;
    }

    memset(&kmstr_msg, 0, sizeof(ut_message_t));
    kmstr_msg.delete_key.key_length = key->key_material_size;
    kmstr_msg.command.commandId = CMD_ID_TEE_DELETE_KEY;
    kmstr_msg.delete_key.android_version = android_version;
    kmstr_msg.delete_key.km_version = km_version;
    memcpy(keymaster_buffer, &kmstr_msg, sizeof(ut_message_t));
    memcpy(keymaster_buffer + sizeof(ut_message_t), key->key_material, key->key_material_size);
    /* IMSG_ERROR("ut_delete_key key=%s,key_length=%d",key->key_material,key->key_material_size); */
    ret = TEE_InvokeCommand();

    if (ret != 0) {
        IMSG_ERROR("ut_delete_key cmd err %d", ret);
        return KM_ERROR_UNKNOWN_ERROR;
    }

    memcpy(&kmstr_msg, keymaster_buffer, sizeof(ut_message_t));
    return kmstr_msg.response.error;
}

/**
 * ut_begin
 *
 * @param  purpose          [in]
 * @param  key              [in]
 * @param  in_params        [in]
 * @param  out_params       [out]
 * @param  operation_handle [out]
 */
keymaster_error_t ut_begin(keymaster_purpose_t purpose, const keymaster_key_blob_t* key,
                           const keymaster_key_param_set_t* in_params,
                           keymaster_key_param_set_t* out_params,
                           keymaster_operation_handle_t* operation_handle) {
    uint32_t retVal = 0;
    uint8_t* p_data_buff = 0;
    uint32_t in_params_size = 0;
    ut_message_t kmstr_msg;

    if (TEE_Alloc_KM_Buffer_And_Reset() != 0) {
        IMSG_ERROR("%s: malloc km buffer failed", __FUNCTION__);
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;
    }

    memset(&kmstr_msg, 0, sizeof(ut_message_t));
    p_data_buff = keymaster_buffer + sizeof(ut_message_t);
    kmstr_msg.command.commandId = CMD_ID_TEE_BEGIN;
    kmstr_msg.begin.android_version = android_version;
    kmstr_msg.begin.km_version = km_version;
    kmstr_msg.begin.purpose = purpose;
    kmstr_msg.begin.key_size = key->key_material_size;
    ut_append_data(key->key_material, p_data_buff, key->key_material_size);
    p_data_buff += kmstr_msg.begin.key_size;
    kmstr_msg.begin.in_params_count = in_params->length;
    in_params_size = ut_append_encode_parameters(in_params, p_data_buff);
    memcpy(keymaster_buffer, &kmstr_msg, sizeof(ut_message_t));
    retVal = TEE_InvokeCommand();

    if (retVal != 0) {
        IMSG_ERROR("begin cmd err %d", retVal);
        return KM_ERROR_UNKNOWN_ERROR;
    }

    memcpy(&kmstr_msg, keymaster_buffer, sizeof(ut_message_t));
    p_data_buff = keymaster_buffer + sizeof(ut_message_t);

    if (kmstr_msg.response.error < 0) {
        IMSG_ERROR("keymaster begin error %d", kmstr_msg.response.error);
        return kmstr_msg.response.error;
    }

    *operation_handle = kmstr_msg.begin.operation_handle;

    if (out_params == NULL)
        IMSG_INFO("begin not has out params1");

    if (out_params != NULL && kmstr_msg.begin.out_params_count != 0) {
        out_params->params = (keymaster_key_param_t*)malloc(sizeof(keymaster_key_param_t));
        out_params->length = 1;
        out_params->params->tag = KM_TAG_NONCE;
        out_params->params->blob.data = malloc(kmstr_msg.begin.out_params_count);
        out_params->params->blob.data_length = kmstr_msg.begin.out_params_count;
        memcpy((unsigned char*)(out_params->params->blob.data), p_data_buff,
               kmstr_msg.begin.out_params_count);
    }

    return retVal;
}

/**
 * ut_update
 *
 * @param  operation_handle [in]
 * @param  in_params        [in]
 * @param  input            [in]
 * @param  input_consumed   [in]
 * @param  out_params       [out]
 * @param  output           [out]
 */
keymaster_error_t ut_update(keymaster_operation_handle_t operation_handle,
                            const keymaster_key_param_set_t* in_params,
                            const keymaster_blob_t* input, size_t* input_consumed,
                            keymaster_key_param_set_t* out_params, keymaster_blob_t* output) {
    (void)out_params;
    uint32_t retVal = 0;
    uint8_t* p_data_buff = NULL;
    uint32_t in_params_size = 0;
    ut_message_t kmstr_msg;

    if (TEE_Alloc_KM_Buffer_And_Reset() != 0) {
        IMSG_ERROR("%s: malloc km buffer failed", __FUNCTION__);
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;
    }

    memset(&kmstr_msg, 0, sizeof(ut_message_t));
    p_data_buff = keymaster_buffer + sizeof(ut_message_t);
    kmstr_msg.command.commandId = CMD_ID_TEE_UPDATE;
    kmstr_msg.update.android_version = android_version;
    kmstr_msg.update.km_version = km_version;
    kmstr_msg.update.operation_handle = operation_handle;

    if (in_params != NULL) {
        kmstr_msg.update.in_params_count = in_params->length;
        in_params_size = ut_append_encode_parameters(in_params, p_data_buff);
    }

    p_data_buff += in_params_size;
    kmstr_msg.update.input_size = input->data_length;
    ut_append_data(input->data, p_data_buff, input->data_length);
    memcpy(keymaster_buffer, &kmstr_msg, sizeof(ut_message_t));
    retVal = TEE_InvokeCommand();

    if (retVal != 0) {
        IMSG_ERROR("update cmd err %d", retVal);
        *input_consumed = 0;
        return KM_ERROR_UNKNOWN_ERROR;
    }

    memcpy(&kmstr_msg, keymaster_buffer, sizeof(ut_message_t));
    p_data_buff = keymaster_buffer + sizeof(ut_message_t);

    if (kmstr_msg.response.error != 0) {
        IMSG_ERROR("keymaster consumed=%d ", kmstr_msg.update.input_consumed);
        IMSG_ERROR("keymaster update error=%d ", kmstr_msg.response.error);
        *input_consumed = kmstr_msg.update.input_consumed;
        return kmstr_msg.response.error;
    }

    *input_consumed = kmstr_msg.update.input_consumed;

    if (output != NULL) {
        if (kmstr_msg.update.output_size != 0) {
            /* IMSG_ERROR("kmstr_msg.update.output_size=%d",kmstr_msg.update.output_size); */
            output->data = malloc(kmstr_msg.update.output_size);
            output->data_length = kmstr_msg.update.output_size;
            ut_append_data(p_data_buff, (unsigned char*)output->data, kmstr_msg.update.output_size);
        }
    }

    return retVal;
}

/**
 * ut_finish
 *
 *
 *
 *
 * @param   operation_handle[in]
 * @param   in_params       [in]
 * @param   signature       [in]
 * @param   out_params      [out]
 * @param   output          [out]
 */
keymaster_error_t ut_finish(keymaster_operation_handle_t operation_handle,
                            const keymaster_key_param_set_t* in_params,
                            const keymaster_blob_t* signature,
                            keymaster_key_param_set_t* out_params, keymaster_blob_t* output,
                            const keymaster_blob_t* input) {
    (void)out_params;
    uint32_t retVal = 0;
    uint8_t* p_data_buff = NULL;
    ut_message_t kmstr_msg;

    if (TEE_Alloc_KM_Buffer_And_Reset() != 0) {
        IMSG_ERROR("%s: malloc km buffer failed", __FUNCTION__);
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;
    }

    memset(&kmstr_msg, 0, sizeof(ut_message_t));
    p_data_buff = keymaster_buffer + sizeof(ut_message_t);
    kmstr_msg.command.commandId = CMD_ID_TEE_FINISH;
    kmstr_msg.finisth.android_version = android_version;
    kmstr_msg.finisth.km_version = km_version;
    kmstr_msg.finisth.operation_handle = operation_handle;

	if(input != NULL) {
		kmstr_msg.finisth.input_size = input->data_length;
		if(input->data_length != 0)
			ut_append_data(input->data, p_data_buff, input->data_length);
		p_data_buff += input->data_length;
	}

    if (in_params != NULL) {
        kmstr_msg.finisth.in_params_count = in_params->length;
        retVal = ut_append_encode_parameters(in_params, p_data_buff);
    }

    if (signature != NULL) {
        kmstr_msg.finisth.signature_size = signature->data_length;
        p_data_buff += retVal;
        ut_append_data(signature->data, p_data_buff, signature->data_length);
    } else {
        kmstr_msg.finisth.signature_size = 0;
    }

    memcpy(keymaster_buffer, &kmstr_msg, sizeof(ut_message_t));

    retVal = TEE_InvokeCommand();

    if (retVal != 0) {
        IMSG_ERROR("finish cmd err %d", retVal);
        return KM_ERROR_UNKNOWN_ERROR;
    }

    memcpy(&kmstr_msg, keymaster_buffer, sizeof(ut_message_t));
    p_data_buff = keymaster_buffer + sizeof(ut_message_t);

    if (kmstr_msg.response.error != 0) {
        IMSG_ERROR("keymaster finish err=%d", kmstr_msg.response.error);
        return kmstr_msg.response.error;
    }

    if (kmstr_msg.finisth.output_size != 0) {
        output->data = malloc(kmstr_msg.finisth.output_size);
        output->data_length = kmstr_msg.finisth.output_size;
        ut_append_data(p_data_buff, (unsigned char*)(output->data), kmstr_msg.finisth.output_size);
    }

    return retVal;
}

/**
 * ut_abort
 * @param   operation_handle[in]
 */
keymaster_error_t ut_abort(keymaster_operation_handle_t operation_handle) {
    uint32_t retVal = 0;
    uint8_t* p_data_buff = NULL;
    ut_message_t kmstr_msg;

    IMSG_ENTER();

    if (TEE_Alloc_KM_Buffer_And_Reset() != 0) {
        IMSG_ERROR("%s: malloc km buffer failed", __FUNCTION__);
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;
    }

    memset(&kmstr_msg, 0, sizeof(ut_message_t));
    p_data_buff = keymaster_buffer + sizeof(ut_message_t);
    kmstr_msg.command.commandId = CMD_ID_TEE_ABORT;
    kmstr_msg.abort.operation_handle = operation_handle;
    kmstr_msg.abort.android_version = android_version;
    kmstr_msg.abort.km_version = km_version;
    memcpy(keymaster_buffer, &kmstr_msg, sizeof(ut_message_t));
    retVal = TEE_InvokeCommand();

    if (retVal != 0) {
        IMSG_ERROR("abort cmd err %d", retVal);
        return KM_ERROR_UNKNOWN_ERROR;
    }

    memcpy(&kmstr_msg, keymaster_buffer, sizeof(ut_message_t));

    /*HidlHalGTest#SigningOperationsTest.RsaAbort_32bit*/
    if (kmstr_msg.response.error != 0) {
        IMSG_ERROR("keymaster abort err=%d", kmstr_msg.response.error);
        return kmstr_msg.response.error;
    }

    return retVal;
}
/**
 * inner ut_append_encode_parameters
 *
 * count    [in]
 * data     [in]
 * buffer   [out]
 * size     [out]
 */
uint32_t ut_append_encode_parameters(const keymaster_key_param_set_t* from, void* to_buffer) {
    uint8_t* p = (uint8_t*)to_buffer;
    int retVal = 0;
    size_t i = 0;
    unsigned int data_length;

    for (i = 0; i < from->length; i++) {
        memcpy(p + retVal, &from->params[i].tag, sizeof(uint32_t));
        retVal += sizeof(uint32_t);

        switch (keymaster_tag_get_type(from->params[i].tag)) {
        case KM_ENUM:
        case KM_ENUM_REP:
            data_length = sizeof(uint32_t);
            memcpy(p + retVal, &data_length, sizeof(uint32_t));
            retVal += sizeof(uint32_t);
            memcpy(p + retVal, &from->params[i].enumerated, sizeof(uint32_t));
            retVal += sizeof(uint32_t);
            break;

        case KM_BOOL:
            data_length = sizeof(uint32_t);
            memcpy(p + retVal, &data_length, sizeof(uint32_t));
            retVal += sizeof(uint32_t);
            memcpy(p + retVal, &from->params[i].boolean, sizeof(uint32_t));
            retVal += sizeof(uint32_t);
            break;

        case KM_UINT:
        case KM_UINT_REP:
            data_length = sizeof(uint32_t);
            memcpy(p + retVal, &data_length, sizeof(uint32_t));
            retVal += sizeof(uint32_t);
            memcpy(p + retVal, &from->params[i].integer, sizeof(uint32_t));
            retVal += sizeof(uint32_t);
            break;

        case KM_ULONG_REP:
        case KM_ULONG:
            data_length = sizeof(uint64_t);
            memcpy(p + retVal, &data_length, sizeof(uint32_t));
            retVal += sizeof(uint32_t);
            memcpy(p + retVal, &from->params[i].long_integer, sizeof(uint64_t));
            retVal += sizeof(uint64_t);
            break;

        case KM_DATE:
            data_length = sizeof(uint64_t);
            memcpy(p + retVal, &data_length, sizeof(uint32_t));
            retVal += sizeof(uint32_t);
            memcpy(p + retVal, &from->params[i].date_time, sizeof(uint64_t));
            retVal += sizeof(uint64_t);
            break;

        case KM_BIGNUM:
        case KM_BYTES:
            data_length = from->params[i].blob.data_length;
            memcpy(p + retVal, &data_length, sizeof(uint32_t));
            retVal += sizeof(uint32_t);
            memcpy(p + retVal, from->params[i].blob.data, from->params[i].blob.data_length);
            retVal += from->params[i].blob.data_length;
            break;

        default:
            data_length = 0;
            memcpy(p + retVal, &data_length, sizeof(uint32_t));
            retVal += sizeof(uint32_t);
            break;
        }
    }

    return retVal;
}

uint32_t ut_append_decode_parameters(uint32_t count, void* from,
                                     keymaster_key_param_set_t* to_buffer) {
    int err = 0;
    unsigned int i = 0;
    int retVal = 0;
    uint8_t* p = (uint8_t*)from;
    keymaster_tag_t tag;
    uint32_t size = 0;

    to_buffer->length = count;

    for (i = 0; i < count; i++) {
        memcpy(&tag, p + retVal, sizeof(uint32_t));
        to_buffer->params[i].tag = tag;
        retVal += sizeof(uint32_t);
        memcpy(&size, p + retVal, sizeof(uint32_t));
        retVal += sizeof(uint32_t);
        err = ut_set_param_value(to_buffer->params[i].tag, size, p + retVal, &to_buffer->params[i]);
        retVal += size;
    }

    return retVal;
}

int ut_set_param_value(keymaster_tag_t tag, size_t size, void* from, keymaster_key_param_t* param) {
    int retVal = 0;

    param->tag = tag;

    switch (keymaster_tag_get_type(param->tag)) {
    case KM_ENUM:
    case KM_ENUM_REP:
        memcpy(&param->enumerated, from, size);
        break;

    case KM_BOOL:
        memcpy(&param->boolean, from, size);
        break;

    case KM_UINT:
    case KM_UINT_REP:
        memcpy(&param->integer, from, size);
        break;

    case KM_ULONG_REP:
    case KM_ULONG:
        memcpy(&param->long_integer, from, size);
        break;

    case KM_DATE:
        memcpy(&param->date_time, from, size);
        break;

    case KM_BIGNUM:
    case KM_BYTES:
        param->blob.data = (uint8_t*)malloc(size);
        memcpy((unsigned char*)(param->blob.data), from, size);
        param->blob.data_length = size;
        break;

    default:
        break;
    }

    return retVal;
}

void ut_append_data(const uint8_t* from_buffer, void* to_buffer, uint32_t size) {
    memcpy(to_buffer, from_buffer, size);
}
