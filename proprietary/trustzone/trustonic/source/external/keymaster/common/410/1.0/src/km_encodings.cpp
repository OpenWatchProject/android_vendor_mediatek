/*
 * Copyright (c) 2015-2016 TRUSTONIC LIMITED
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

#ifdef SOTER
#include "mtk_keymaster_defs.h"
#else
#include <hardware/keymaster_defs.h>
#endif

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

static keymaster_error_t get_curve(
    ec_curve_t *curve,
    int nid)
{
    switch (nid) {
        case NID_X9_62_prime192v1:
            *curve = ec_curve_nist_p192;
            break;
        case NID_secp224r1:
            *curve = ec_curve_nist_p224;
            break;
        case NID_X9_62_prime256v1:
            *curve = ec_curve_nist_p256;
            break;
        case NID_secp384r1:
            *curve = ec_curve_nist_p384;
            break;
        case NID_secp521r1:
            *curve = ec_curve_nist_p521;
            break;
        default:
            return KM_ERROR_INVALID_KEY_BLOB;
    }

    return KM_ERROR_OK;
}

static keymaster_error_t get_nid(
    int *nid,
    ec_curve_t curve)
{
    assert(nid != NULL);

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

static uint32_t get_bitlen(
    ec_curve_t curve)
{
    switch (curve) {
        case ec_curve_nist_p192:
            return 192;
        case ec_curve_nist_p224:
            return 224;
        case ec_curve_nist_p256:
            return 256;
        case ec_curve_nist_p384:
            return 384;
        case ec_curve_nist_p521:
            return 521;
        default:
            return 0;
    }
}

/**
 * Decode a PKCS8-encoded EC key
 *
 * @param[out] core_key_data buffer for metadata + raw key data
 * @param core_key_data_len length of \p core_key_data buffer
 * @param[in,out] key_size key size in bits if known, otherwise 0 on entry, actual on exit
 * @param pkey key
 *
 * @return KM_ERROR_OK or error
 */
static keymaster_error_t decode_pkcs8_ec_key(
    uint8_t *core_key_data,
    uint32_t core_key_data_len,
    uint32_t *key_size,
    const EVP_PKEY *pkey)
{
    keymaster_error_t ret = KM_ERROR_OK;
    const EC_POINT *pubkey = NULL;
    const EC_GROUP *ec_group = NULL;
    BIGNUM *x_bn = NULL;
    BIGNUM *y_bn = NULL;
    const BIGNUM *d_bn;
    uint32_t x_len, y_len, d_len, curve_len;
    uint8_t *key_metadata;
    uint8_t *raw_key_data;
    uint8_t *pos;
    ec_curve_t curve;
    uint32_t actual_key_size;

    assert(key_size != NULL);

    const EC_KEY *eckey = EVP_PKEY_get1_EC_KEY((EVP_PKEY*)pkey);
    CHECK_TRUE(KM_ERROR_INVALID_KEY_BLOB,
        eckey != NULL);

    pubkey = EC_KEY_get0_public_key(eckey);
    CHECK_TRUE(KM_ERROR_INVALID_KEY_BLOB,
        pubkey != NULL);

    ec_group = EC_KEY_get0_group(eckey);
    CHECK_TRUE(KM_ERROR_INVALID_KEY_BLOB,
        ec_group != NULL);

    CHECK_RESULT_OK(get_curve(&curve, EC_GROUP_get_curve_name(ec_group)));

    /* Set key size, and check consistency with import_key() params. */
    actual_key_size = get_bitlen(curve);
    if (*key_size == 0) {
        *key_size = actual_key_size;
    } else {
        CHECK_TRUE(KM_ERROR_IMPORT_PARAMETER_MISMATCH,
            *key_size == actual_key_size);
    }

    curve_len = BITS_TO_BYTES(actual_key_size);
    CHECK_TRUE(KM_ERROR_IMPORT_PARAMETER_MISMATCH,
        core_key_data_len >= KM_EC_METADATA_SIZE + 3 * curve_len);

    x_bn = BN_new();
    y_bn = BN_new();
    CHECK_TRUE(KM_ERROR_MEMORY_ALLOCATION_FAILED,
        (x_bn != NULL) && (y_bn != NULL));

    CHECK_TRUE(KM_ERROR_INVALID_KEY_BLOB,
        EC_POINT_get_affine_coordinates_GFp(ec_group, pubkey, x_bn, y_bn, NULL));

    d_bn = EC_KEY_get0_private_key(eckey);
    CHECK_TRUE(KM_ERROR_INVALID_KEY_BLOB,
        d_bn != NULL);

    x_len = BN_num_bytes(x_bn);
    y_len = BN_num_bytes(y_bn);
    d_len = BN_num_bytes(d_bn);

    CHECK_TRUE(KM_ERROR_INVALID_KEY_BLOB,
        (x_len <= curve_len) && (y_len <= curve_len) && (d_len <= curve_len));

    key_metadata = core_key_data;
    raw_key_data = key_metadata + KM_EC_METADATA_SIZE;

    /* Some versions of Kinibi require all lengths to match the curve length.
     * Arrange for it to be so by zero-padding.
     */

    pos = key_metadata;
    set_u32_increment_pos(&pos, curve);
    set_u32_increment_pos(&pos, curve_len);
    set_u32_increment_pos(&pos, curve_len);
    set_u32_increment_pos(&pos, curve_len);

    memset(raw_key_data, 0, curve_len - x_len);
    BN_bn2bin(x_bn, raw_key_data + curve_len - x_len);
    memset(raw_key_data + curve_len, 0, curve_len - y_len);
    BN_bn2bin(y_bn, raw_key_data + 2 * curve_len - y_len);
    memset(raw_key_data + 2 * curve_len, 0, curve_len - d_len);
    BN_bn2bin(d_bn, raw_key_data + 3 * curve_len - d_len);

end:
    BN_free(x_bn);
    BN_free(y_bn);
    EC_KEY_free((EC_KEY*)eckey);

    return ret;
}

/**
 * Decode a PKCS8-encoded RSA key.
 *
 * @param[out] core_key_data buffer for metadata + raw key data
 * @param core_key_data_len length of \p core_key_data buffer
 * @param[in,out] key_size key size in bits if known, otherwise 0 on entry, actual on exit
 * @param[in,out] rsa_pubexp RSA public exponent if known, otherwise 0 on entry, actual on exit
 * @param pkey key
 *
 * @return KM_ERROR_OK or error
 */
static keymaster_error_t decode_pkcs8_rsa_key(
    uint8_t *core_key_data,
    uint32_t core_key_data_len,
    uint32_t *key_size,
    uint64_t *rsa_pubexp,
    const EVP_PKEY *pkey)
{
    keymaster_error_t ret = KM_ERROR_OK;
    RSA *rsa = NULL;
    uint32_t n_len, e_len, d_len, p_len, q_len, dp_len, dq_len, qinv_len;
    size_t offset;
    uint8_t *key_metadata;
    uint8_t *raw_key_data;
    uint8_t *pos;
    uint32_t actual_key_size;
    uint64_t actual_rsa_pubexp;

    CHECK_NOT_NULL(key_size);
    CHECK_NOT_NULL(rsa_pubexp);

    rsa = EVP_PKEY_get1_RSA((EVP_PKEY*)pkey);
    CHECK_TRUE(KM_ERROR_INVALID_KEY_BLOB,
        rsa != NULL);

    n_len = BN_num_bytes(rsa->n);
    e_len = BN_num_bytes(rsa->e);
    d_len = BN_num_bytes(rsa->d);
    p_len = BN_num_bytes(rsa->p);
    q_len = BN_num_bytes(rsa->q);
    dp_len = BN_num_bytes(rsa->dmp1);
    dq_len = BN_num_bytes(rsa->dmq1);
    qinv_len = BN_num_bytes(rsa->iqmp);

    CHECK_TRUE(KM_ERROR_IMPORT_PARAMETER_MISMATCH,
        core_key_data_len >= KM_RSA_METADATA_SIZE
            + n_len + e_len + d_len + p_len + q_len + dp_len + dq_len + qinv_len);

    key_metadata = core_key_data;
    raw_key_data = key_metadata + KM_RSA_METADATA_SIZE;

    pos = key_metadata + 4; // leave space for key size
    set_u32_increment_pos(&pos, n_len);
    set_u32_increment_pos(&pos, e_len);
    set_u32_increment_pos(&pos, d_len);
    set_u32_increment_pos(&pos, p_len);
    set_u32_increment_pos(&pos, q_len);
    set_u32_increment_pos(&pos, dp_len);
    set_u32_increment_pos(&pos, dq_len);
    set_u32_increment_pos(&pos, qinv_len);

    offset = 0;
    BN_bn2bin(rsa->n, raw_key_data + offset); offset += n_len;
    BN_bn2bin(rsa->e, raw_key_data + offset); offset += e_len;
    BN_bn2bin(rsa->d, raw_key_data + offset); offset += d_len;
    BN_bn2bin(rsa->p, raw_key_data + offset); offset += p_len;
    BN_bn2bin(rsa->q, raw_key_data + offset); offset += q_len;
    BN_bn2bin(rsa->dmp1, raw_key_data + offset); offset += dp_len;
    BN_bn2bin(rsa->dmq1, raw_key_data + offset); offset += dq_len;
    BN_bn2bin(rsa->iqmp, raw_key_data + offset); // offset += qinv_len;

    actual_key_size = 8 * n_len;
    set_u32(key_metadata, actual_key_size);
    CHECK_TRUE(KM_ERROR_INVALID_ARGUMENT,
        e_len <= 4); // TODO Allow import of RSA keys with 2^32 < e < 2^64
    actual_rsa_pubexp = BN_get_word(rsa->e);

    /* Set key size, or check consistency with import_key() params. */
    if (*key_size == 0) {
        *key_size = actual_key_size;
    } else {
        CHECK_TRUE(KM_ERROR_IMPORT_PARAMETER_MISMATCH,
            *key_size == actual_key_size);
    }

    /* Set RSA public exponent, or check consistency with import_key() params. */
    if (*rsa_pubexp == 0) {
        *rsa_pubexp = actual_rsa_pubexp;
    } else {
        CHECK_TRUE(KM_ERROR_IMPORT_PARAMETER_MISMATCH,
            *rsa_pubexp == actual_rsa_pubexp);
    }

end:
    RSA_free(rsa);

    return ret;
}

keymaster_error_t decode_pkcs8(
    uint8_t *core_key_data,
    uint32_t core_key_data_len,
    uint32_t *key_size,
    uint64_t *rsa_pubexp,
    const keymaster_blob_t *pkcs8_data)
{
    keymaster_error_t ret = KM_ERROR_OK;
    PKCS8_PRIV_KEY_INFO *p8inf = NULL;
    EVP_PKEY *pkey = NULL;

    CHECK_NOT_NULL(key_size);

    /* Parse key data */
    p8inf = d2i_PKCS8_PRIV_KEY_INFO(NULL,
        (const unsigned char**)&pkcs8_data->data, pkcs8_data->data_length);
    CHECK_TRUE(KM_ERROR_INVALID_KEY_BLOB,
        p8inf != NULL);
    pkey = EVP_PKCS82PKEY(p8inf);
    CHECK_TRUE(KM_ERROR_INVALID_KEY_BLOB,
        pkey != NULL);

    switch (EVP_PKEY_type(pkey->type)) {
        case EVP_PKEY_EC:
            ret = decode_pkcs8_ec_key(
                core_key_data, core_key_data_len, key_size, pkey);
            break;
        case EVP_PKEY_RSA:
            ret = decode_pkcs8_rsa_key(
                core_key_data, core_key_data_len, key_size, rsa_pubexp, pkey);
            break;
        default:
            ret = KM_ERROR_INVALID_KEY_BLOB;
    }

end:
    PKCS8_PRIV_KEY_INFO_free(p8inf);
    EVP_PKEY_free(pkey);

    return ret;
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
