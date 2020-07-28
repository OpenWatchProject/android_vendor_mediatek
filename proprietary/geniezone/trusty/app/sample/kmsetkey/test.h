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

#ifndef __TEST_H__
#define __TEST_H__

int32_t generate_rsa_key_test(void);
int32_t generate_rsa_signature_test(void);
int32_t verify_rsa_signature_test(void);
int32_t generate_ecc_key_test(void);
int32_t generate_ecc_signature_test(void);
int32_t verify_ecc_signature_test(void);
int32_t perform_aes_128_enc_test(void);
int32_t perform_aes_128_dec_test(void);
int32_t perform_aes_256_enc_test(void);
int32_t perform_aes_256_dec_test(void);
int32_t perform_sha_hash_test(void);

#endif
