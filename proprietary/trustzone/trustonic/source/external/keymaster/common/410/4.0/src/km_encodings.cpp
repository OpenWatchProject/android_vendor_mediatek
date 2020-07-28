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

#include "keymaster_ta_defs.h"
#include <openssl/x509.h>

#include "km_encodings.h"
#include "tlTeeKeymaster_Api.h"
#include "km_shared_util.h"
#include "km_util.h"
#include <assert.h>

/**
 * Supported ECC curve types (from ectypes.h)
 */
typedef enum {
    ec_curve_nist_p192 = 1,
    ec_curve_nist_p224 = 2,
    ec_curve_nist_p256 = 3,
    ec_curve_nist_p384 = 4,
    ec_curve_nist_p521 = 5
} ec_curve_t;

static keymaster_error_t get_nid(
    int *nid,
    ec_curve_t curve)
{
    switch (curve) {
        case ec_curve_nist_p192:
            *nid = NID_X9_62_prime192v1;
            break;
        case ec_curve_nist_p224:
            *nid = NID_secp224r1;
            break;
        case ec_curve_nist_p256:
            *nid = NID_X9_62_prime256v1;
            break;
        case ec_curve_nist_p384:
            *nid = NID_secp384r1;
            break;
        case ec_curve_nist_p521:
            *nid = NID_secp521r1;
            break;
        default:
            return KM_ERROR_INVALID_KEY_BLOB;
    }

    return KM_ERROR_OK;
}

static keymaster_error_t encode_x509_rsa_pub(
    keymaster_blob_t *export_data,
    uint32_t key_size,
    const uint8_t *core_pub_data,
    uint32_t core_pub_data_len)
{
    keymaster_error_t ret = KM_ERROR_OK;
    EVP_PKEY *pkey = NULL;
    RSA *rsa = NULL;
    uint32_t n_len, e_len;
    int encoded_len;
    unsigned char *pos = NULL;

    rsa = RSA_new();
    CHECK_TRUE(KM_ERROR_MEMORY_ALLOCATION_FAILED,
        rsa != NULL);

    /* Read metadata */
    CHECK_TRUE(KM_ERROR_INVALID_KEY_BLOB,
        core_pub_data_len >= 12);

    CHECK_TRUE(KM_ERROR_INVALID_KEY_BLOB,
        key_size == get_u32(core_pub_data));
    n_len = get_u32(core_pub_data + 4);
    e_len = get_u32(core_pub_data + 8);

    CHECK_TRUE(KM_ERROR_INVALID_KEY_BLOB,
        (n_len <= BITS_TO_BYTES(key_size)) &&
        (e_len <= BITS_TO_BYTES(key_size)));

    /* Read bignums and populate key */
    CHECK_TRUE(KM_ERROR_INVALID_KEY_BLOB,
        core_pub_data_len >= 12 + n_len + e_len);

    rsa->n = BN_bin2bn(core_pub_data + 12, n_len, NULL);
    rsa->e = BN_bin2bn(core_pub_data + 12 + n_len, e_len, NULL);
    CHECK_TRUE(KM_ERROR_MEMORY_ALLOCATION_FAILED,
        (rsa->n != NULL) && (rsa->e != NULL));

    /* Set the EVP_PKEY */
    pkey = EVP_PKEY_new();
    CHECK_TRUE(KM_ERROR_MEMORY_ALLOCATION_FAILED,
        pkey != NULL);
    CHECK_TRUE(KM_ERROR_UNKNOWN_ERROR,
        EVP_PKEY_set1_RSA(pkey, rsa));

    /* Allocate and populate export_data */
    encoded_len = i2d_PUBKEY(pkey, NULL);
    CHECK_TRUE(KM_ERROR_INVALID_KEY_BLOB,
        encoded_len > 0);
    export_data->data = (uint8_t*)malloc(encoded_len);
    CHECK_TRUE(KM_ERROR_MEMORY_ALLOCATION_FAILED,
        export_data->data != NULL);
    pos = (unsigned char*)export_data->data;
    CHECK_TRUE(KM_ERROR_UNKNOWN_ERROR,
        i2d_PUBKEY(pkey, &pos) == encoded_len);
    export_data->data_length = encoded_len;

end:
    EVP_PKEY_free(pkey);
    RSA_free(rsa);
    if (ret != KM_ERROR_OK) {
        free((void*)export_data->data);
        export_data->data = NULL;
        export_data->data_length = 0;
    }

    return ret;
}

static keymaster_error_t encode_x509_ec_pub(
    keymaster_blob_t *export_data,
    uint32_t key_size,
    const uint8_t *core_pub_data,
    uint32_t core_pub_data_len)
{
    keymaster_error_t ret = KM_ERROR_OK;
    EVP_PKEY *pkey = NULL;
    EC_GROUP *ecgroup = NULL;
    EC_KEY *eckey = NULL;
    EC_POINT *ecpoint = NULL;
    BIGNUM *x = NULL;
    BIGNUM *y = NULL;
    ec_curve_t curve;
    uint32_t x_len, y_len;
    int encoded_len;
    int nid;
    unsigned char *pos = NULL;

    /* Read metadata */
    CHECK_TRUE(KM_ERROR_INVALID_KEY_BLOB,
        core_pub_data_len >= 12);
    curve = (ec_curve_t)get_u32(core_pub_data);
    x_len = get_u32(core_pub_data + 4);
    y_len = get_u32(core_pub_data + 8);
    CHECK_TRUE(KM_ERROR_INVALID_KEY_BLOB,
        (x_len <= BITS_TO_BYTES(key_size)) &&
        (y_len <= BITS_TO_BYTES(key_size)));

    /* Construct EC group and key */
    CHECK_RESULT_OK(get_nid(&nid, curve));
    ecgroup = EC_GROUP_new_by_curve_name(nid);
    CHECK_TRUE(KM_ERROR_MEMORY_ALLOCATION_FAILED,
        ecgroup != NULL);
    eckey = EC_KEY_new_by_curve_name(nid);
    CHECK_TRUE(KM_ERROR_MEMORY_ALLOCATION_FAILED,
        eckey != NULL);

    /* Read bignums */
    x = BN_bin2bn(core_pub_data + 12, x_len, NULL);
    y = BN_bin2bn(core_pub_data + 12 + x_len, y_len, NULL);
    CHECK_TRUE(KM_ERROR_MEMORY_ALLOCATION_FAILED,
        (x != NULL) && y != NULL);

    /* Set ecpoint */
    ecpoint = EC_POINT_new(ecgroup);
    CHECK_TRUE(KM_ERROR_MEMORY_ALLOCATION_FAILED,
        ecpoint != NULL);
    CHECK_TRUE(KM_ERROR_UNKNOWN_ERROR,
        EC_POINT_set_affine_coordinates_GFp(ecgroup, ecpoint, x, y, NULL));

    /* Set eckey */
    CHECK_TRUE(KM_ERROR_UNKNOWN_ERROR,
        EC_KEY_set_public_key(eckey, ecpoint));

    /* Set the EVP_PKEY */
    pkey = EVP_PKEY_new();
    CHECK_TRUE(KM_ERROR_MEMORY_ALLOCATION_FAILED,
        pkey != NULL);
    CHECK_TRUE(KM_ERROR_UNKNOWN_ERROR,
        EVP_PKEY_set1_EC_KEY(pkey, eckey));

    /* Allocate and populate export_data */
    encoded_len = i2d_PUBKEY(pkey, NULL);
    CHECK_TRUE(KM_ERROR_INVALID_KEY_BLOB,
        encoded_len > 0);
    export_data->data = (uint8_t*)malloc(encoded_len);
    CHECK_TRUE(KM_ERROR_MEMORY_ALLOCATION_FAILED,
        export_data->data != NULL);
    pos = (unsigned char*)export_data->data;
    CHECK_TRUE(KM_ERROR_UNKNOWN_ERROR,
        i2d_PUBKEY(pkey, &pos) == encoded_len);
    export_data->data_length = encoded_len;

end:
    EVP_PKEY_free(pkey);
    EC_POINT_free(ecpoint);
    BN_free(x);
    BN_free(y);
    EC_KEY_free(eckey);
    EC_GROUP_free(ecgroup);
    if (ret != KM_ERROR_OK) {
        free((void*)export_data->data);
        export_data->data = NULL;
        export_data->data_length = 0;
    }

    return ret;
}

keymaster_error_t encode_x509(
    keymaster_blob_t *export_data,
    keymaster_algorithm_t key_type,
    uint32_t key_size,
    const uint8_t *core_pub_data,
    uint32_t core_pub_data_len)
{
    switch (key_type) {
        case KM_ALGORITHM_RSA:
            return encode_x509_rsa_pub(export_data, key_size,
                core_pub_data, core_pub_data_len);
        case KM_ALGORITHM_EC:
            return encode_x509_ec_pub(export_data, key_size,
                core_pub_data, core_pub_data_len);
        default:
            return KM_ERROR_INCOMPATIBLE_ALGORITHM;
    }
}
