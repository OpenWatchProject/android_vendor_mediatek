/*
 * Copyright (c) 2019, MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws.
 * The information contained herein is confidential and proprietary to
 * MediaTek Inc. and/or its licensors.
 * Except as otherwise provided in the applicable licensing terms with
 * MediaTek Inc. and/or its licensors, any reproduction, modification, use or
 * disclosure of MediaTek Software, and information contained herein, in whole
 * or in part, shall be strictly prohibited.
*/

#include <assert.h>
#include <err.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <trusty_std.h>

#include <openssl/engine.h>
#include <openssl/digest.h>
#include <openssl/cipher.h>
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/ec_key.h>
#include <openssl/ecdsa.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/mem.h>

#define LOG_TAG "KM_SETKEY_LIB"
#define TLOGE(fmt, ...) \
	fprintf(stderr, "%s: %s: %d: " fmt, LOG_TAG, __FUNCTION__, __LINE__,  ## __VA_ARGS__)

#define TLOGP(fmt, ...) \
	fprintf(stderr, fmt,  ## __VA_ARGS__)

#define RSA_PKCS_15 1
#define DATA_DUMP 0

#if 0
int32_t tries, rand_tries;

static int summary(int event, int n, BN_GENCB *arg)
{
	if (event == 0)
		rand_tries = n;
	else if (event == 2)
		tries = n;
	return 1;
}
#endif

#define data_size 0x100000
#define hash_size 32
#define aes_128_size 16
#define aes_256_size 32
#define log_size 16

uint8_t in[data_size], out[data_size], key[aes_256_size], iv[aes_256_size];
uint32_t in_len, out_len;

RSA *rsakey = NULL;
EC_KEY *ecckey = NULL;

void init_data(uint8_t seed_in, uint8_t seed_out, uint8_t seed_key, uint8_t seed_iv)
{
	for (in_len = 0; in_len < data_size; ++in_len) {
		in[in_len % data_size] = (seed_in + in_len) % 256;
		out[in_len % data_size] = (seed_out + in_len) % 256;
		key[in_len % aes_256_size] = (seed_key + in_len) % 256;
		iv[in_len % aes_256_size] = (seed_iv + in_len) % 256;
	}
}

int32_t generate_rsa_key(void)
{
	int32_t rc = 1;
	//BN_GENCB cb;

	if (rsakey)
		RSA_free(rsakey);

	rsakey = RSA_new();
	if (rsakey == NULL) {
		TLOGE("Allocation RSA key failed!\n");
		return -1;
	}

	//BN_GENCB_set(&cb, &summary, NULL);
	//tries = 0;
	//rand_tries = 0;
	//rc = RSA_generate_key_fips(rsakey, 2048, &cb);

	rc = RSA_generate_key_fips(rsakey, 2048, NULL);
	if (rc != 1) {
		TLOGE("RSA_generate_key_fips failed (%d)!\n", rc);
		rc = -2;
		goto exit;
	}
#if DATA_DUMP
	BIGNUM *n, *e, *d, *p, *q, *dmp1, *dmq1, *iqmp;
	char *str;

	RSA_get0_key(rsakey, &n, &e, &d);
	RSA_get0_factors(rsakey, &p, &q);
	RSA_get0_crt_params(rsakey, &dmp1, &dmq1, &iqmp);

	str = BN_bn2hex(n);
	TLOGE("RSA->n:\n");
	TLOGP(" 0x%s\n", str);
	OPENSSL_free(str);

	str = BN_bn2hex(e);
	TLOGE("RSA->e:\n");
	TLOGP(" 0x%s\n", str);
	OPENSSL_free(str);

	str = BN_bn2hex(d);
	TLOGE("RSA->d:\n");
	TLOGP(" 0x%s\n", str);
	OPENSSL_free(str);

	str = BN_bn2hex(p);
	TLOGE("RSA->p:\n");
	TLOGP(" 0x%s\n", str);
	OPENSSL_free(str);

	str = BN_bn2hex(q);
	TLOGE("RSA->q:\n");
	TLOGP(" 0x%s\n", str);
	OPENSSL_free(str);

	str = BN_bn2hex(dmp1);
	TLOGE("RSA->dmp1:\n");
	TLOGP(" 0x%s\n", str);
	OPENSSL_free(str);

	str = BN_bn2hex(dmq1);
	TLOGE("RSA->dmq1:\n");
	TLOGP(" 0x%s\n", str);
	OPENSSL_free(str);

	str = BN_bn2hex(iqmp);
	TLOGE("RSA->iqmp:\n");
	TLOGP(" 0x%s\n", str);
	OPENSSL_free(str);
#endif
exit:
	return rc < 0 ? rc : 0;
}

int32_t generate_rsa_signature(void)
{
	int32_t rc = 1;

	in_len = hash_size;
	out_len = data_size;
#if RSA_PKCS_15
	rc = RSA_sign(NID_sha256, in, in_len, out, &out_len, rsakey);
	if (rc != 1) {
		TLOGE("RSA_sign failed (%d)!\n", rc);
		rc = -1;
		goto exit;
	}
#else
	rc = RSA_sign_pss_mgf1(rsakey, &out_len, out, data_size, in, in_len, EVP_sha256(), NULL, -1);
	if (rc != 1) {
		TLOGE("RSA_sign_pss_mgf1 failed (%d)!\n", rc);
		rc = -1;
		goto exit;
	}
#endif

#if DATA_DUMP
	TLOGE("RSA signature:");
	for (rc = 0; rc < out_len; ++rc) {
		if (rc % log_size == 0)
			TLOGP("\n");
		TLOGP(" 0x%02X", out[rc]);
	}
	TLOGP("\n");
	TLOGE("RSA message:");
	for (rc = 0; rc < hash_size; ++rc) {
		if (rc % log_size == 0)
			TLOGP("\n");
		TLOGP(" 0x%02X", in[rc]);
	}
	TLOGP("\n");
#endif
exit:
	return rc < 0 ? rc : 0;
}

int32_t verify_rsa_signature(void)
{
	int32_t rc = 1;
#if RSA_PKCS_15
	rc = RSA_verify(NID_sha256, in, in_len, out, out_len, rsakey);
	if (rc != 1) {
		TLOGE("RSA_verify (%d)!\n", rc);
		rc = -1;
		goto exit;
	}
#else
	rc = RSA_verify_pss_mgf1(rsakey, in, in_len, EVP_sha256(), NULL, -1, out, out_len);
	if (rc != 1) {
		TLOGE("RSA_verify_pss_mgf1 (%d)!\n", rc);
		rc = -1;
		goto exit;
	}
#endif
exit:
	return rc < 0 ? rc : 0;
}

int32_t generate_ecc_key(void)
{
	int32_t rc = 1;

	if (ecckey)
		EC_KEY_free(ecckey);

	ecckey = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
	if (ecckey == NULL) {
		TLOGE("Allocation ECC key failed!\n");
		return -1;
	}

	rc = EC_KEY_generate_key_fips(ecckey);
	if (rc != 1) {
		TLOGE("EC_KEY_generate_key_fips failed (%d)!\n", rc);
		rc = -2;
		goto exit;
	}
#if DATA_DUMP
	EC_GROUP *group = EC_KEY_get0_group(ecckey);
	int nid = EC_GROUP_get_curve_name(group);
	TLOGE("EC_KEY->group:\n");
	TLOGP(" %d\n", nid);

	BIGNUM *sk = EC_KEY_get0_private_key(ecckey);
	char *str = BN_bn2hex(sk);
	TLOGE("EC_KEY->priv_key:\n");
	TLOGP(" 0x%s\n", str);
	OPENSSL_free(str);

	EC_POINT *pk = EC_KEY_get0_public_key(ecckey);
	BIGNUM *x = BN_new(), *y = BN_new();
	rc = EC_POINT_get_affine_coordinates_GFp(group, pk, x, y, NULL);
	if (rc != 1) {
		TLOGE("Convert EC_KEY->pub_key failed: %d\n", rc);
		BN_free(x);
		BN_free(y);
		return -3;
	}
	str = BN_bn2hex(x);
	TLOGE("EC_KEY->pub_key(x):\n");
	TLOGP(" 0x%s\n", str);
	OPENSSL_free(str);
	BN_free(x);
	str = BN_bn2hex(y);
	TLOGE("EC_KEY->pub_key(y):\n");
	TLOGP(" 0x%s\n", str);
	OPENSSL_free(str);
	BN_free(y);

	unsigned int flag = EC_KEY_get_enc_flags(ecckey);
	TLOGE("EC_KEY->enc_flag:\n");
	TLOGP(" %u\n", flag);

	point_conversion_form_t form = EC_KEY_get_conv_form(ecckey);
	TLOGE("EC_KEY->conv_form:\n");
	TLOGP(" %u\n", form);
#endif
exit:
	return rc < 0 ? rc : 0;
}

int32_t generate_ecc_signature(void)
{
	int32_t rc = 1;

	in_len = hash_size;
	out_len = data_size;

	rc = ECDSA_sign(0, in, in_len, out, &out_len, ecckey);
	if (rc != 1) {
		TLOGE("ECDSA_sign failed (%d)!\n", rc);
		rc = -1;
		goto exit;
	}
#if DATA_DUMP
	TLOGE("ECC signature:");
	for (rc = 0; rc < out_len; ++rc) {
		if (rc % log_size == 0)
			TLOGP("\n");
		TLOGP(" 0x%02X", out[rc]);
	}
	TLOGP("\n");
	TLOGE("ECC message:");
	for (rc = 0; rc < hash_size; ++rc) {
		if (rc % log_size == 0)
			TLOGP("\n");
		TLOGP(" 0x%02X", in[rc]);
	}
	TLOGP("\n");
#endif
exit:
	return rc < 0 ? rc : 0;
}

int32_t verify_ecc_signature(void)
{
	int32_t rc = 1;

	rc = ECDSA_verify(0, in, in_len, out, out_len, ecckey);
	if (rc != 1) {
		TLOGE("ECDSA_verify failed (%d)!\n", rc);
		rc = -1;
		goto exit;
	}

exit:
	return rc < 0 ? rc : 0;
}

int32_t perform_aes_128_enc(void)
{
	int32_t rc;

	EVP_CIPHER_CTX *cipher_ctx = EVP_CIPHER_CTX_new();
	if (cipher_ctx == NULL) {
		TLOGE("Allocation cipher context failed!\n");
		return -1;
	}

	rc = EVP_CipherInit_ex(cipher_ctx, EVP_aes_128_cbc(), NULL, key, iv, 1);
	if (rc != 1) {
		TLOGE("Initialize cipher failed!\n");
		rc = -2;
		goto exit;
	}

	rc = EVP_Cipher(cipher_ctx, out, in, data_size);
	if (rc != 1) {
		TLOGE("cipher failed!\n");
		rc = -3;
		goto exit;
	}
#if DATA_DUMP
	TLOGE("AES-128 key:");
	for (rc = 0; rc < aes_128_size; ++rc) {
		if (rc % log_size == 0)
			TLOGP("\n");
		TLOGP(" 0x%02X", key[rc]);
	}
	TLOGP("\n");
	TLOGE("AES-128 IV:");
	for (rc = 0; rc < aes_128_size; ++rc) {
		if (rc % log_size == 0)
			TLOGP("\n");
		TLOGP(" 0x%02X", iv[rc]);
	}
	TLOGP("\n");
	TLOGE("AES-128 ciphertext:");
	for (rc = 0; rc < data_size; ++rc) {
		if (rc % log_size == 0)
			TLOGP("\n");
		TLOGP(" 0x%02X", out[rc]);
	}
	TLOGP("\n");
	TLOGE("AES-128 plaintext:");
	for (rc = 0; rc < data_size; ++rc) {
		if (rc % log_size == 0)
			TLOGP("\n");
		TLOGP(" 0x%02X", in[rc]);
	}
	TLOGP("\n");
#endif
exit:
	EVP_CIPHER_CTX_free(cipher_ctx);
	return rc < 0 ? rc : 0;
}

int32_t perform_aes_256_enc(void)
{
	int32_t rc;

	EVP_CIPHER_CTX *cipher_ctx = EVP_CIPHER_CTX_new();
	if (cipher_ctx == NULL) {
		TLOGE("Allocation cipher context failed!\n");
		return -1;
	}

	rc = EVP_CipherInit_ex(cipher_ctx, EVP_aes_256_cbc(), NULL, key, iv, 1);
	if (rc != 1) {
		TLOGE("Initialize cipher failed!\n");
		rc = -2;
		goto exit;
	}

	rc = EVP_Cipher(cipher_ctx, out, in, data_size);
	if (rc != 1) {
		TLOGE("cipher failed!\n");
		rc = -3;
		goto exit;
	}
#if DATA_DUMP
	TLOGE("AES-256 key:");
	for (rc = 0; rc < aes_256_size; ++rc) {
		if (rc % log_size == 0)
			TLOGP("\n");
		TLOGP(" 0x%02X", key[rc]);
	}
	TLOGP("\n");
	TLOGE("AES-256 IV:");
	for (rc = 0; rc < aes_256_size; ++rc) {
		if (rc % log_size == 0)
			TLOGP("\n");
		TLOGP(" 0x%02X", iv[rc]);
	}
	TLOGP("\n");
	TLOGE("AES-256 ciphertext:");
	for (rc = 0; rc < data_size; ++rc) {
		if (rc % log_size == 0)
			TLOGP("\n");
		TLOGP(" 0x%02X", out[rc]);
	}
	TLOGP("\n");
	TLOGE("AES-256 plaintext:");
	for (rc = 0; rc < data_size; ++rc) {
		if (rc % log_size == 0)
			TLOGP("\n");
		TLOGP(" 0x%02X", in[rc]);
	}
	TLOGP("\n");
#endif
exit:
	EVP_CIPHER_CTX_free(cipher_ctx);
	return rc < 0 ? rc : 0;
}

int32_t perform_sha_hash(void)
{
	int32_t rc;

	out_len = data_size;

	rc = EVP_Digest(in, data_size, out, &out_len, EVP_sha256(), NULL);
	if (rc != 1) {
		TLOGE("Digest failed!\n");
		return -1;
	}

	if (out_len != EVP_MD_size(EVP_sha256())) {
		TLOGE("Wrong digest length: %u!\n", out_len);
		return -2;
	}
#if DATA_DUMP
	TLOGE("SHA-256 digest:");
	for (rc = 0; rc < out_len; ++rc) {
		if (rc % log_size == 0)
			TLOGP("\n");
		TLOGP(" 0x%02X", out[rc]);
	}
	TLOGP("\n");
	TLOGE("SHA-256 MESSAGE:");
	for (rc = 0; rc < data_size; ++rc) {
		if (rc % log_size == 0)
			TLOGP("\n");
		TLOGP(" 0x%02X", in[rc]);
	}
	TLOGP("\n");
#endif
	return rc < 0 ? rc : 0;
}

