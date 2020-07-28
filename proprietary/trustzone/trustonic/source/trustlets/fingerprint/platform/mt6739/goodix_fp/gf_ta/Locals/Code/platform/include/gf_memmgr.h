/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __GF_MEMMGR_H__
#define __GF_MEMMGR_H__

#include "gf_error.h"
#include "gf_memmgr_inner.h"
#include "gf_tee_storage.h"
#include "gf_user_type_define.h"

#ifdef __cplusplus
extern "C" {
#endif

gf_error_t gf_memmgr_set_config(gf_memmgr_config_t *mem_config);

gf_error_t gf_memmgr_save_config(gf_memmgr_config_t *mem_config);

gf_error_t gf_memmgr_remove_config(void);

gf_error_t gf_memmgr_load_config(void);

gf_error_t gf_memmgr_get_manager_info(gf_memmgr_info_t *node_info);

gf_error_t gf_memmgr_dump_mem_pool(gf_memmgr_pool_dump_t *pool_dump);

/**
 * @brief: called before gf_ta_create_entry_point.
 */
gf_error_t gf_memmgr_create_entry_point(void);

/**
 * @brief: Uninitialized memory management module. called after gf_ta_destory_entry_point
 */
void gf_memmgr_destroy_entry_point(void);

/**
 * @brief: Apply for memory from memory management module.
 */
void *gf_memmgr_malloc(int32_t size, uint8_t default_value, const int8_t *file_name,
        const int8_t *func_name, int32_t line);

/**
 * @brief: Reallocate memory blocks from memory management module.
 */
void *gf_memmgr_realloc(void *src, int32_t size, const int8_t *file_name, const int8_t *func_name,
        int32_t line);

/**
 * @brief: Free the applied memory.
 */
void gf_memmgr_free(void *mem_addr);

/**
 * @brief: Check the validity of memory
 */
gf_error_t gf_memmgr_check_addr_validity(const void *mem_addr);

/**
 * @brief: Dump the memory pool information of the memory address.
 */
void gf_memmgr_dump_node(const void *mem_addr);

/**
 * @brief: Print all unreleased memory pool's information.
 */
void gf_memmgr_dump_leak_info(void);

/**
 * @brief: Dump the current memory usage information.
 */
void gf_memmgr_dump_usage_info(void);

#ifdef __cplusplus
}
#endif
#endif  // __GF_MEMMGR_H__
