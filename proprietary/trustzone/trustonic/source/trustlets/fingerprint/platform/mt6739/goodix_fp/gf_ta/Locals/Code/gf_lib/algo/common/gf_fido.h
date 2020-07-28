/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __GF_FIDO_H__
#define __GF_FIDO_H__

#include <gf_error.h>

// below is defined for fido, UVT means User Verify Token.
#define MAX_AAID_LEN             32
#define MAX_FINAL_CHALLENGE_LEN  32
#define MAX_UVT_LEN              512  // the size which fido suggest is 8k

// the define of gf_verify_t is accord to Android AK SDK document.
typedef struct {
    uint8_t result;

    uint16_t final_challenge_len;
    uint8_t final_challenge[MAX_FINAL_CHALLENGE_LEN];

    uint16_t aaid_len;
    uint8_t aaid[MAX_AAID_LEN];

    uint32_t user_id;
    uint32_t user_verification_index;  // finger print id

    uint32_t timestamp;

    uint8_t match_score;  // a number from 0 to 100;
} gf_verify_t;

/**
 * @brief
 *  generate UVT(User Verification Token) according to gf_verify_t data
 * @param[in]  verify  original data
 * @param[out]  buf     UVT data buffer
 * @param[out]  buf_len UVT data buffer length
 * @return         0 means success
 */
gf_error_t fido_generate_uvt(gf_verify_t verify, uint8_t *buf, uint32_t *buf_len);

#endif  // __GF_FIDO_H__
