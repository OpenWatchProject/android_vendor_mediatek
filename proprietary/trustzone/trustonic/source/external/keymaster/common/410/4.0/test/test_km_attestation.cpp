/*
 * Copyright (c) 2016-2018 TRUSTONIC LIMITED
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
#include "test_km_attestation.h"
#include "test_km_util.h"
#ifdef DEBUG_CERT
#include "stdio.h"
#endif

#include "log.h"

/* HACK */
#define KM_TAG_ATTESTATION_APPLICATION_ID (keymaster_tag_t)(KM_BYTES | 709)

#ifdef DEBUG_CERT
/* Write generated certs here */
#define CERT_DIR "/data/app"

static keymaster_error_t write_cert(
    keymaster_cert_chain_t *cert_chain,
    const char *filename)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_blob_t *cert;
    FILE *f;
    size_t n_bytes;
    int err;

    CHECK_TRUE(cert_chain->entry_count == 3);
    cert = &cert_chain->entries[0];
    f = fopen(filename, "wb");
    CHECK_TRUE(f != NULL);
    n_bytes = fwrite(cert->data, 1, cert->data_length, f);
    CHECK_TRUE(n_bytes == cert->data_length);
    err = fclose(f);
    CHECK_TRUE(err == 0);

end:
    return res;
}
#endif

keymaster_error_t test_km_attestation(
    TrustonicKeymaster4DeviceImpl *impl)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t key_param[11];
    keymaster_key_param_set_t key_paramset = {key_param, 11};
    keymaster_key_blob_t key_blob = {NULL, 0};
    keymaster_key_param_t attest_param[3];
    keymaster_key_param_set_t attest_paramset = {attest_param, 0};
    keymaster_cert_chain_t cert_chain = {NULL, 0};
    uint8_t challenge_bytes[] = {1,2,3};
    keymaster_blob_t challenge = {challenge_bytes, sizeof(challenge_bytes)};
    uint8_t attestation_id_bytes[] = {4,5,6};
    keymaster_blob_t attestation_id = {attestation_id_bytes, sizeof(attestation_id_bytes)};

    LOG_I("Generate EC key ...");
    key_param[0].tag = KM_TAG_ALGORITHM;
    key_param[0].enumerated = KM_ALGORITHM_EC;
    key_param[1].tag = KM_TAG_KEY_SIZE;
    key_param[1].integer = 256;
    key_param[2].tag = KM_TAG_NO_AUTH_REQUIRED;
    key_param[2].boolean = true;
    key_param[3].tag = KM_TAG_ACTIVE_DATETIME;
    key_param[3].date_time = 10000;
    key_param[4].tag = KM_TAG_USAGE_EXPIRE_DATETIME;
    key_param[4].date_time = 1000000;
    key_param[5].tag = KM_TAG_INCLUDE_UNIQUE_ID;
    key_param[5].boolean = true;
    key_paramset.length = 6;
    CHECK_RESULT_OK(impl->generate_key(
        &key_paramset, &key_blob, NULL));

    LOG_I("Attest to EC key...");
    attest_param[0].tag = KM_TAG_ATTESTATION_CHALLENGE;
    attest_param[0].blob = challenge;
    attest_param[1].tag = KM_TAG_RESET_SINCE_ID_ROTATION;
    attest_param[1].boolean = true;
    attest_param[2].tag = KM_TAG_ATTESTATION_APPLICATION_ID;
    attest_param[2].blob = attestation_id;
    attest_paramset.length = 3;
    CHECK_RESULT_OK(impl->attest_key(
        &key_blob, &attest_paramset, &cert_chain));

#ifdef DEBUG_CERT
    CHECK_RESULT_OK(write_cert(&cert_chain, CERT_DIR "/ec"));
#endif

    LOG_I("Generate RSA key (1)...");
    km_free_key_blob(&key_blob);
    key_param[0].enumerated = KM_ALGORITHM_RSA;
    key_param[1].integer = 1024;
    CHECK_RESULT_OK(impl->generate_key(
        &key_paramset, &key_blob, NULL));

    LOG_I("Attest to RSA key (1)...");
    keymaster_free_cert_chain(&cert_chain);
    CHECK_RESULT_OK(impl->attest_key(
        &key_blob, &attest_paramset, &cert_chain));

#ifdef DEBUG_CERT
    CHECK_RESULT_OK(write_cert(&cert_chain, CERT_DIR "/rsa1"));
#endif

    LOG_I("Generate RSA key (2)...");
    km_free_key_blob(&key_blob);
    key_param[1].integer = 512;
    key_param[5].boolean = false; // KM_TAG_INCLUDE_UNIQUE_ID
    key_param[6].tag = KM_TAG_PURPOSE;
    key_param[6].enumerated = KM_PURPOSE_SIGN;
    key_param[7].tag = KM_TAG_PURPOSE;
    key_param[7].enumerated = KM_PURPOSE_VERIFY;
    key_param[8].tag = KM_TAG_PADDING;
    key_param[8].enumerated = KM_PAD_RSA_PKCS1_1_5_SIGN;
    key_param[9].tag = KM_TAG_DIGEST;
    key_param[9].enumerated = KM_DIGEST_SHA_2_512;
    key_param[10].tag = KM_TAG_ORIGINATION_EXPIRE_DATETIME;
    key_param[10].date_time = 100000;
    key_paramset.length = 11;
    CHECK_RESULT_OK(impl->generate_key(
        &key_paramset, &key_blob, NULL));

    LOG_I("Attest to RSA key (2)...");
    keymaster_free_cert_chain(&cert_chain);
    CHECK_RESULT_OK(impl->attest_key(
        &key_blob, &attest_paramset, &cert_chain));

#ifdef DEBUG_CERT
    CHECK_RESULT_OK(write_cert(&cert_chain, CERT_DIR "/rsa2"));
#endif

    /* Other key types should fail with INCOMPATIBLE_ALGORITHM. */
    LOG_I("Unsupported key type...");
    km_free_key_blob(&key_blob);
    key_param[0].enumerated = KM_ALGORITHM_AES;
    key_param[1].integer = 128;
    CHECK_RESULT_OK(impl->generate_key(
        &key_paramset, &key_blob, NULL));
    keymaster_free_cert_chain(&cert_chain);
    CHECK_RESULT(KM_ERROR_INCOMPATIBLE_ALGORITHM, impl->attest_key(
        &key_blob, &attest_paramset, &cert_chain));

end:
    km_free_key_blob(&key_blob);
    keymaster_free_cert_chain(&cert_chain);
    return res;
}
