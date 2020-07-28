#ifndef __GF_DR_COMMON_H__
#define __GF_DR_COMMON_H__

typedef enum {
    GF_DR_CMD_GET_TIMESTAMP = 2000, //
    GF_DR_CMD_SLEEP,
    GF_DR_CMD_SYNC_AUTH_RESULT,
    GF_DR_CMD_SHARE_MEMORY_PREFORMANCE,
    GF_DR_CMD_ALIPAY_API_GET_AUTH_ID,
    GF_DR_CMD_WECHAT_API_GET_SESSION_ID,
    GF_DR_CMD_WECHAT_API_SET_SESSION_ID,
    GF_DR_CMD_WECHAT_API_GET_AUTH_ID,
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
    uint64_t session_id;
} gf_wechat_api_get_session_id_t;

typedef struct {
    uint64_t session_id;
} gf_wechat_api_set_session_id_t;

typedef struct {
    uint32_t finger_id;
    uint64_t time;
} gf_wechat_api_get_auth_id_t;

typedef struct {
    gf_dr_cmd_id_t cmd_id;
    union {
        gf_dr_get_timestamp_t get_timestamp;
        gf_dr_sleep_t sleep;
        gf_dr_sync_auth_result_t auth_result;
        gf_dr_share_memory_performance_t share_memory_performance;
        gf_alipay_api_get_auth_id_t get_auth_id;
        gf_wechat_api_get_session_id_t get_session_id;
        gf_wechat_api_set_session_id_t set_session_id;
        gf_wechat_api_get_auth_id_t wechat_get_auth_id;
    } data;
} gf_dr_cmd_t;

#endif // __GF_DR_COMMON_H__
