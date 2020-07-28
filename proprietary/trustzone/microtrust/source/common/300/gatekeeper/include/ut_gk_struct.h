/*
 * Copyright (c) 2015-2016 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */

#ifndef __UT_GK_STRUCT_H__
#define __UT_GK_STRUCT_H__
#include <stdint.h>

typedef uint64_t secure_id_t;
typedef uint64_t salt_t;
struct __attribute__((__packed__)) password_handle_t {
    uint8_t version;
    secure_id_t user_id;
    uint64_t flags;

    salt_t salt;
    uint8_t signature[32];

    bool hardware_backed;
};

typedef struct __attribute__((__packed__)) {
    uint8_t version;  // Current version is 0
    uint64_t challenge;
    uint64_t user_id;             // secure user ID, not Android user ID
    uint64_t authenticator_id;    // secure authenticator ID
    uint32_t authenticator_type;  // hw_authenticator_type_t, in network order
    uint64_t timestamp;           // in network order
    uint8_t hmac[32];
} hw_auth_token_t;

typedef enum {
    ERROR_NONE = 0,
    ERROR_INVALID,
    ERROR_RETRY,
    ERROR_UNKNOWN,
    ERROR_PASSWORD_WRONG,
    ERROR_UID_NOTFOUND,
    ERROR_CMD_WRONG,
} gatekeeper_error_t;

typedef struct __attribute__((__packed__)) {
    gatekeeper_error_t error;
    uint32_t enrolled_password_handle_length;
    uint32_t retry_timeout;
} ut_gk_enroll_out_t;

typedef struct __attribute__((__packed__)) {
    gatekeeper_error_t error;
    uint32_t auth_token_length;
    bool request_reenroll;
    uint32_t retry_timeout;
} ut_gk_verify_out_t;

typedef struct __attribute__((__packed__)) {
    gatekeeper_error_t error;
} ut_gk_del_user_out_t;

typedef struct __attribute__((__packed__)) {
    gatekeeper_error_t error;
} ut_gk_base_out_t;

typedef struct __attribute__((__packed__)) {
    uint32_t current_password_handle_length;
    uint32_t current_password_length;
    uint32_t desired_password_length;
} ut_gk_enroll_t;

typedef struct __attribute__((__packed__)) {
    uint64_t challenge;
    uint32_t enrolled_password_handle_length;
    uint32_t provided_password_length;
} ut_gk_verify_t;

typedef struct __attribute__((__packed__)) {
    uint8_t cmd;
    uint32_t uid;
    union {
        ut_gk_enroll_t ut_gk_enroll;
        ut_gk_verify_t ut_gk_verify;
    };
} ut_gk_struct_t;

#endif
