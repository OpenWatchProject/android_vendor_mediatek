/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#ifndef __GF_DR_COMMON_H__
#define __GF_DR_COMMON_H__

#define MAX_FINGER_COUNT 32

typedef enum {
    GF_DR_CMD_GET_TIMESTAMP = 2000,
    GF_DR_CMD_SLEEP,
    GF_DR_CMD_SYNC_AUTH_RESULT,
    GF_DR_CMD_SHARE_MEMORY_PREFORMANCE,
    GF_DR_CMD_ALIPAY_API_GET_AUTH_ID,
    GF_DR_CMD_WECHAT_API_GET_AUTH_ID,

    GF_DR_CMD_SYNC_FINGER_LIST,
    GF_DR_CMD_SYNC_AUTHENTICATOR_VERSION,
    GF_DR_CMD_IFAA_GET_FINGER_LIST,
    GF_DR_CMD_IFAA_GET_AUTHENTICATOR_VERSION
} gf_dr_cmd_id_t;

typedef struct {
    uint64_t timestamp;
} gf_dr_get_timestamp_t;

typedef struct {
    uint64_t miliseconds;
} gf_dr_sleep_t;

typedef struct {
    uint32_t finger_id;
    uint64_t time;
} gf_dr_sync_auth_result_t;

typedef struct {
    uint32_t len;
    uint8_t *buf;
    uint64_t time;
} gf_dr_share_memory_performance_t;

typedef struct {
    uint32_t finger_id;
    uint64_t time;
} gf_alipay_api_get_auth_id_t;

typedef struct {
    uint32_t finger_id;
    uint64_t time;
} gf_wechat_api_get_auth_id_t;

typedef struct {
    uint32_t ids[MAX_FINGER_COUNT];
    uint32_t count;
} gf_dr_sync_finger_list_t;

typedef struct {
    uint32_t version;
} gf_dr_sync_authenticator_version_t;

typedef struct {
    uint32_t ids[MAX_FINGER_COUNT];
    uint32_t count;
} gf_ifaa_api_get_get_finger_list_t;

typedef struct {
    uint32_t version;
} gf_ifaa_api_get_authenticator_version_t;

typedef struct {
    gf_dr_cmd_id_t cmd_id;
    union {
        gf_dr_get_timestamp_t get_timestamp;
        gf_dr_sleep_t sleep;
        gf_dr_sync_auth_result_t auth_result;
        gf_dr_share_memory_performance_t share_memory_performance;

        gf_dr_sync_finger_list_t sync_finger_list;
        gf_dr_sync_authenticator_version_t sync_authenticator_version;

        gf_alipay_api_get_auth_id_t get_auth_id;
        gf_wechat_api_get_auth_id_t wechat_get_auth_id;

        gf_ifaa_api_get_get_finger_list_t get_finger_list;
        gf_ifaa_api_get_authenticator_version_t get_authenticator_version;
    } data;
} gf_dr_cmd_t;

#endif  // __GF_DR_COMMON_H__
