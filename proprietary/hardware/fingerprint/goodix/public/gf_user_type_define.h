/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#ifndef __GF_USER_TYPE_DEFINE_H__
#define __GF_USER_TYPE_DEFINE_H__

#include <stdint.h>
#include "gf_type_define.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TEST_SHARE_MEMORY_CHECK_LEN 1024
// (65536 * 4 = 256 * 1024) the size to dump memmgr node one time
#define DUMP_MEM_NODE_BUF_LEN    262144
#define DUMP_MEM_POOL_BUF_LEN    262144
#define QSEE_HMAC_KEY_MAX_LEN    256

typedef enum {
    GF_USER_CMD_TEST_SHARE_MEMORY_PERFORMANCE = 100,
    GF_USER_CMD_GET_LAST_IDENTIFY_ID,
    GF_USER_CMD_GET_AUTHENTICATOR_VERSION,
    GF_USER_CMD_ENUMERATE,
    GF_USER_CMD_TEST_SHARE_MEMORY_CHECK,
    GF_USER_CMD_TEST_SET_MEMMGR_CONFIG,
    GF_USER_CMD_TEST_GET_MEMMGR_CONFIG,
    GF_USER_CMD_TEST_GET_MEMMGR_INFO,
    GF_USER_CMD_TEST_DUMP_MEMMGR_POOL,
    GF_USER_CMD_SET_HMAC_KEY,
} gf_user_cmd_id_t;

typedef struct {
    uint32_t version;
} gf_user_authenticator_version_t;

typedef struct {
    uint32_t last_identify_id;
} gf_user_last_identify_id_t;

typedef struct {
    uint32_t size;
    uint32_t group_ids[MAX_FINGERS_PER_USER];
    uint32_t finger_ids[MAX_FINGERS_PER_USER];
} gf_user_enumerate_t;

typedef struct {
    uint8_t memmgr_enable;
    uint8_t debug_enable;
    uint8_t match_best_mem_pool_enable;
    uint8_t erase_mem_pool_when_free;
    uint8_t dump_time_enable;
    uint32_t memmgr_pool_size;
    // prompt whether to start mem manager next reboot
    uint8_t enable_memmgr_next_reboot;
} gf_memmgr_config_t;

typedef struct {
    uint64_t mem_pool_start_addr;
    uint64_t mem_pool_end_addr;
    uint32_t cur_used_block_count;
    uint32_t max_used_block_count;
    uint32_t cur_used_mem_size;
    uint32_t maxs_used_mem_size;

    uint32_t total_node_count;
    uint32_t transport_node_count;
    uint32_t next_start_node;
    uint32_t node_info_size;
    uint8_t mem_pool_node_data[DUMP_MEM_NODE_BUF_LEN];
} gf_memmgr_info_t;

typedef struct {
    uint32_t offset;
    uint32_t dump_len;
    uint8_t finished;
    uint8_t pool_arr[DUMP_MEM_POOL_BUF_LEN];
} gf_memmgr_pool_dump_t;

typedef struct {
    uint8_t hmac_key[QSEE_HMAC_KEY_MAX_LEN];
} gf_hmac_key_t;

#ifdef __cplusplus
}
#endif

#endif  // __GF_USER_TYPE_DEFINE_H__
