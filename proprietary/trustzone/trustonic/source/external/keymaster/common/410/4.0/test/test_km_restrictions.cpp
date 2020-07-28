/*
 * Copyright (c) 2015-2018 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "TrustonicKeymaster4DeviceImpl.h"
#include "test_km_restrictions.h"
#include "test_km_util.h"

#include "log.h"

static keymaster_error_t test_km_restrictions_app_id_and_data(
    TrustonicKeymaster4DeviceImpl *impl)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t key_params[7];
    keymaster_key_param_set_t generate_paramset = {key_params, 0};
    const uint8_t id_bytes[] = {1, 2, 3};
    const uint8_t data_bytes[] = {4, 5, 6, 7};
    keymaster_blob_t application_id = {id_bytes, sizeof(id_bytes)};
    keymaster_blob_t application_data = {data_bytes, sizeof(data_bytes)};
    const uint8_t wrong_id_bytes[] = {3, 2, 1};
    const uint8_t wrong_data_bytes[] = {4, 5, 6, 7, 8};
    keymaster_blob_t wrong_application_id = {wrong_id_bytes, sizeof(wrong_id_bytes)};
    keymaster_blob_t wrong_application_data = {wrong_data_bytes, sizeof(wrong_data_bytes)};
    keymaster_key_blob_t key_blob = {NULL, 0};
    keymaster_key_characteristics_t key_characteristics;
    keymaster_blob_t export_data = {NULL, 0};
    keymaster_operation_handle_t handle;
    keymaster_key_param_t op_params[4];
    keymaster_key_param_set_t begin_paramset = {op_params, 0};

    memset(&key_characteristics, 0, sizeof(keymaster_key_characteristics_t));

    /* generate key */
    key_params[0].tag = KM_TAG_ALGORITHM;
    key_params[0].enumerated = KM_ALGORITHM_EC;
    key_params[1].tag = KM_TAG_KEY_SIZE;
    key_params[1].integer = 256;
    key_params[2].tag = KM_TAG_PURPOSE;
    key_params[2].enumerated = KM_PURPOSE_SIGN;
    key_params[3].tag = KM_TAG_DIGEST;
    key_params[3].enumerated = KM_DIGEST_SHA_2_256;
    key_params[4].tag = KM_TAG_NO_AUTH_REQUIRED;
    key_params[4].boolean = true;
    key_params[5].tag = KM_TAG_APPLICATION_ID;
    key_params[5].blob = application_id;
    key_params[6].tag = KM_TAG_APPLICATION_DATA;
    key_params[6].blob = application_data;
    generate_paramset.length = 7;
    CHECK_RESULT_OK(impl->generate_key(
        &generate_paramset, &key_blob, NULL));
    CHECK_RESULT_OK(save_key_blob("ec-v2+id+data.blob", &key_blob));

    /* get_key_characteristics (good case) */
    CHECK_RESULT_OK(impl->get_key_characteristics(
        &key_blob, &application_id, &application_data, &key_characteristics));

    /* get_key_characteristics (bad cases) */
    keymaster_free_characteristics(&key_characteristics);
    CHECK_RESULT( KM_ERROR_INVALID_KEY_BLOB,
        impl->get_key_characteristics(
            &key_blob, &application_id, NULL, &key_characteristics));
    keymaster_free_characteristics(&key_characteristics);
    CHECK_RESULT( KM_ERROR_INVALID_KEY_BLOB,
        impl->get_key_characteristics(
            &key_blob, NULL, &application_data, &key_characteristics));
    keymaster_free_characteristics(&key_characteristics);
    CHECK_RESULT( KM_ERROR_INVALID_KEY_BLOB,
        impl->get_key_characteristics(
            &key_blob, &application_id, &wrong_application_data, &key_characteristics));
    keymaster_free_characteristics(&key_characteristics);
    CHECK_RESULT( KM_ERROR_INVALID_KEY_BLOB,
        impl->get_key_characteristics(
            &key_blob, &wrong_application_id, &application_data, &key_characteristics));

    /* export_key (good case) */
    CHECK_RESULT_OK(impl->export_key(
        KM_KEY_FORMAT_X509, &key_blob, &application_id, &application_data, &export_data));

    /* export_key (bad cases) */
    km_free_blob(&export_data);
    CHECK_RESULT( KM_ERROR_INVALID_KEY_BLOB,
        impl->export_key(
            KM_KEY_FORMAT_X509, &key_blob, &application_id, NULL, &export_data));
    km_free_blob(&export_data);
    CHECK_RESULT( KM_ERROR_INVALID_KEY_BLOB,
        impl->export_key(
            KM_KEY_FORMAT_X509, &key_blob, NULL, &application_data, &export_data));
    km_free_blob(&export_data);
    CHECK_RESULT( KM_ERROR_INVALID_KEY_BLOB,
        impl->export_key(
            KM_KEY_FORMAT_X509, &key_blob, &application_id, &wrong_application_data, &export_data));
    km_free_blob(&export_data);
    CHECK_RESULT( KM_ERROR_INVALID_KEY_BLOB,
        impl->export_key(
            KM_KEY_FORMAT_X509, &key_blob, &wrong_application_id, &application_data, &export_data));

    /* begin (good case) */
    op_params[0].tag = KM_TAG_ALGORITHM;
    op_params[0].enumerated = KM_ALGORITHM_EC;
    op_params[1].tag = KM_TAG_DIGEST;
    op_params[1].enumerated = KM_DIGEST_SHA_2_256;
    op_params[2].tag = KM_TAG_APPLICATION_ID;
    op_params[2].blob = application_id;
    op_params[3].tag = KM_TAG_APPLICATION_DATA;
    op_params[3].blob = application_data;
    begin_paramset.length = 4;
    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_SIGN, &key_blob, &begin_paramset, NULL, NULL, &handle));
    CHECK_RESULT_OK(impl->abort(
        handle));

    /* begin (bad cases) */
    begin_paramset.length = 3;
    CHECK_RESULT( KM_ERROR_INVALID_KEY_BLOB,
        impl->begin(
            KM_PURPOSE_SIGN, &key_blob, &begin_paramset, NULL, NULL, &handle));
    op_params[2].tag = KM_TAG_APPLICATION_DATA;
    op_params[2].blob = application_data;
    CHECK_RESULT( KM_ERROR_INVALID_KEY_BLOB,
        impl->begin(
            KM_PURPOSE_SIGN, &key_blob, &begin_paramset, NULL, NULL, &handle));
    op_params[3].tag = KM_TAG_APPLICATION_ID;
    op_params[3].blob = wrong_application_id;
    begin_paramset.length = 4;
    CHECK_RESULT( KM_ERROR_INVALID_KEY_BLOB,
        impl->begin(
            KM_PURPOSE_SIGN, &key_blob, &begin_paramset, NULL, NULL, &handle));
    op_params[2].blob = wrong_application_data;
    op_params[3].blob = application_id;
    CHECK_RESULT( KM_ERROR_INVALID_KEY_BLOB,
        impl->begin(
            KM_PURPOSE_SIGN, &key_blob, &begin_paramset, NULL, NULL, &handle));

end:
    km_free_blob(&export_data);
    keymaster_free_characteristics(&key_characteristics);
    km_free_key_blob(&key_blob);

    return res;
}

keymaster_error_t test_km_restrictions(
    TrustonicKeymaster4DeviceImpl *impl)
{
    keymaster_error_t res = KM_ERROR_OK;

    CHECK_RESULT_OK(test_km_restrictions_app_id_and_data(impl));

end:
    return res;
}
