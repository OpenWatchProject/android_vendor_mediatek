/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __GF_MEMMGR_INNER_H__
#define __GF_MEMMGR_INNER_H__

#ifdef __cplusplus
extern "C" {
#endif

#define CALL_STACK_FORMAT_STR                   "[%s::%s:%d]"
#define CALL_STACK_FORMAT_STR_LEN               5
#define CALL_STACK_MAX_BYTES                    128
#define CALL_STACK_LINE_NUMBER_MAX_BYTES        16

#define CALL_STACK_MINIMUM_BYTES                22
#define CALL_STACK_VALID_BYTES                  (CALL_STACK_MAX_BYTES - CALL_STACK_MINIMUM_BYTES)

/**
 * Define some macros for the start tokens of memory pool.
 */
#define MEM_BLOCK_START_TOKEN_BYTES_COUNT 4

#define MEM_BLOCK_START_FIRST_TOKEN          0xEC
#define MEM_BLOCK_START_SECOND_TOKEN         0xED
#define MEM_BLOCK_START_THIRD_TOKEN          0xEE
#define MEM_BLOCK_START_FOUTH_TOKEN          0xEF

/**
 * Define some macros for the end tokens of memory pool.
 */
#define MEM_BLOCK_END_TOKEN_BYTES_COUNT 4

#define MEM_BLOCK_END_FIRST_TOKEN            0xEF
#define MEM_BLOCK_END_SECOND_TOKEN           0xEE
#define MEM_BLOCK_END_THIRD_TOKEN            0xED
#define MEM_BLOCK_END_FOUTH_TOKEN            0xEC

typedef struct {
    uint8_t memmgr_enable;
    uint8_t debug_enable;
    uint8_t match_best_mem_pool_enable;
    uint8_t erase_mem_pool_when_free;
    uint8_t dump_time_enable;
    uint32_t memmgr_pool_size;
    uint8_t enable_memmgr_next_reboot;
} gf_mem_config_t;

/**
 * Define enumerable type to identify memory pool's status.
 */
typedef enum {
    MEM_BLOCK_FLAG_FREE = 0,
    MEM_BLOCK_FLAG_USED,
} gf_memmgr_node_flag_t;

/**
 * The structure for memory pool node.
 */

typedef struct gf_memmgr_node {
    void *base;                           // The address of this memory block
    struct gf_memmgr_node *prev;        // Point to the previous memory pool node
    struct gf_memmgr_node *next;        // Point to the next memory pool node

    struct gf_memmgr_node *prev_free;   // Point to the previous memory pool node
    struct gf_memmgr_node *next_free;   // Point to the next memory pool node

    int32_t size;                         // The available size of this block
    int32_t used_size;
    int32_t flag;                         // Flags for this memory
    uint64_t time;                        // The malloc time
} gf_memmgr_node_t;

#define MEM_BLOCK_HEAD_LEN       (int32_t)(sizeof(gf_memmgr_node_t) \
                                           + MEM_BLOCK_START_TOKEN_BYTES_COUNT)
#define MEM_BLOCK_TAIL_LEN       (MEM_BLOCK_END_TOKEN_BYTES_COUNT + CALL_STACK_MAX_BYTES)

#define MEM_BLOCK_MINIMUM_LEN    (MEM_BLOCK_HEAD_LEN + MEM_BLOCK_TAIL_LEN)
#define MEM_BLOCK_LEN(POOL_SIZE) (MEM_BLOCK_MINIMUM_LEN + (POOL_SIZE))

#ifdef __cplusplus
}
#endif

#endif  // __GF_MEMMGR_INNER_H__
