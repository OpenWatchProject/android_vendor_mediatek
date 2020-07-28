/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#include "string.h"
#include "cpl_stdio.h"
#include "cpl_assert.h"
#include "cpl_thread.h"
#include "cpl_string.h"
#include "gf_memmgr.h"
#include "gf_log.h"
#include "gf_user_type_define.h"
#include "gf_tee_internal_api.h"

#define LOG_TAG "[gf_memmgr_manager]"

#define MEMMGR_TOKEN_LENGTH        sizeof(uint8_t)

#define GF_MEMMGR_CONFIG_ID "gf_memmgr_config.so"
#define GF_MEMMGR_TOKEN_MEMMGR_ENABLE         0XF0
#define GF_MEMMGR_TOKEN_DEBUG_ENABLE          0XF1
#define GF_MEMMGR_TOKEN_MATCH_BEST_ENABLE     0XF2
#define GF_MEMMGR_TOKEN_ERASE_FREE_ENABLE     0XF3
#define GF_MEMMGR_TOKEN_DUMP_TIME_ENABLE      0XF4
#define GF_MEMMGR_TOKEN_MEMMGR_POOL_SIZE      0XFA

#define GF_MEMMGR_INFO_SIZE_PER_NODE          \
        (4 * sizeof(uint32_t) + 2 * sizeof(uint64_t) + CALL_STACK_MAX_BYTES)

gf_mem_config_t g_mem_config = { 0 };

static gf_memmgr_node_t *g_mem_pool_head = NULL;
static gf_memmgr_node_t *g_free_mem_pool_head = NULL;

static void *g_memmgr_start_sentinel = NULL;
static void *g_memmgr_end_sentinel = NULL;

static uint32_t g_cur_used_block_count = 0;
static uint32_t g_max_used_block_count = 0;
static uint32_t g_cur_used_mem_size = 0;
static uint32_t g_maxs_used_mem_size = 0;
static uint32_t g_total_node_count = 0;

static uint64_t g_memmgr_start_time = 0;

static gf_error_t gf_memmgr_init(void);

gf_error_t  gf_memmgr_create_entry_point(void) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    // load config to g_mem_config
    gf_memmgr_load_config();

    // delete mem_config file ensure use normal malloc mode next reboot
    gf_memmgr_remove_config();

    if (g_mem_config.memmgr_enable > 0) {
        err = gf_memmgr_init();
        if (err != GF_SUCCESS) {
            g_mem_config.memmgr_enable = 0;
        }
    }

    FUNC_EXIT(err);
    return err;
}

#ifdef __SUPPORT_GF_MEMMGR
static void gf_memmgr_save_call_stack(gf_memmgr_node_t *node, const int8_t *file_name,
        const int8_t *func_name, const int32_t line) {
    VOID_FUNC_ENTER();

    do {
        uint32_t file_name_len = 0;
        uint32_t func_name_len = 0;
        int8_t *call_stack = NULL;
        if (NULL == node || NULL == file_name || NULL == func_name) {
            GF_LOGE(LOG_TAG "[%s] node=%p, file_name=%p, func_name=%p", __func__, (void*)node,
                    (void*)file_name, (void*)func_name);
            break;
        }

        if (g_mem_config.dump_time_enable > 0) {
            uint64_t time = 0;
            gf_get_timestamp(&time);
            node->time = time - g_memmgr_start_time;
        } else {
            node->time = 0;
        }

        file_name_len = cpl_strlen(file_name);
        func_name_len = cpl_strlen(func_name);

        call_stack = (int8_t*) node->base + node->size + MEM_BLOCK_END_TOKEN_BYTES_COUNT;

        if (file_name_len + func_name_len <= CALL_STACK_VALID_BYTES) {
            cpl_sprintf(call_stack, (const int8_t *) CALL_STACK_FORMAT_STR, file_name, func_name,
                    line);
        } else if (func_name_len <= CALL_STACK_VALID_BYTES) {
            uint32_t remained_len = CALL_STACK_VALID_BYTES - func_name_len;
            cpl_sprintf(call_stack, (const int8_t *) CALL_STACK_FORMAT_STR,
                    file_name + (file_name_len - remained_len), func_name, line);
        } else {
            cpl_sprintf(call_stack, (const int8_t *) CALL_STACK_FORMAT_STR, "",
                    (func_name + func_name_len - CALL_STACK_VALID_BYTES), line);
        }
    } while (0);

    VOID_FUNC_EXIT();
}

static void gf_memmgr_dump_corrupt_block(const void *addr) {
    uint32_t len = 0;
    uint8_t *tmp = (uint8_t *) addr;

    VOID_FUNC_ENTER();
    if (addr < g_memmgr_end_sentinel && addr > g_memmgr_start_sentinel) {
        len = (uint8_t *) addr - (uint8_t *) g_memmgr_start_sentinel;

        if (len > MEM_BLOCK_HEAD_LEN) {
            len = MEM_BLOCK_HEAD_LEN;
        }
    }

    GF_LOGI(LOG_TAG "[%s] invalid memory pool, this=%p", __func__, addr);
    GF_LOGI(LOG_TAG "[%s] begin addr=%p", __func__, (tmp - len));
    for (; len > 0; len--) {
        GF_LOGI(LOG_TAG "[%s] 0x%X", __func__, *(tmp - len));
    }
    GF_LOGI(LOG_TAG "[%s] end addr=%p", __func__, addr);

    VOID_FUNC_EXIT();
}

static gf_memmgr_node_t* gf_memmgr_get_node_by_addr(const void *mem_addr) {
    uint8_t *addr = (uint8_t *) mem_addr;
    gf_memmgr_node_t *result = NULL;
    uint8_t *first_start_token_addr = NULL;
    uint8_t *first_end_token_addr = NULL;
    gf_memmgr_node_t *node = NULL;

    VOID_FUNC_ENTER();
    do {
        if (NULL == addr) {
            GF_LOGE(LOG_TAG "[%s] input addr is NULL", __func__);
            break;
        }

        node = (gf_memmgr_node_t *) (const void *) (addr - MEM_BLOCK_HEAD_LEN);

        if (g_mem_config.debug_enable > 0) {
            if ((const void *) (addr + MEM_BLOCK_TAIL_LEN) > g_memmgr_end_sentinel
                    || (const void *) (addr - MEM_BLOCK_HEAD_LEN) < g_memmgr_start_sentinel) {
                GF_LOGE(LOG_TAG "[%s] reason [1] invalid memory addr, addr=%p", __func__, addr);
                break;
            }

            // invalid memory address or this memory block has been damaged.
            if (mem_addr != node->base) {
                GF_LOGE(LOG_TAG "[%s] reason [2] invalid memory addr, addr=%p", __func__, addr);
                break;
            }

            if (node->used_size > node->size) {
                GF_LOGE(LOG_TAG "[%s] used_size bigger than size, used_size=%d, size=%d", __func__,
                        node->used_size, node->size);
                break;
            }

            // check the memory address's validity
            first_start_token_addr = addr - MEM_BLOCK_START_TOKEN_BYTES_COUNT;
            if ((*first_start_token_addr != MEM_BLOCK_START_FIRST_TOKEN)
                    || (*(first_start_token_addr + 1) != MEM_BLOCK_START_SECOND_TOKEN)
                    || (*(first_start_token_addr + 2) != MEM_BLOCK_START_THIRD_TOKEN)
                    || (*(first_start_token_addr + 3) != MEM_BLOCK_START_FOUTH_TOKEN)) {
                GF_LOGE(LOG_TAG "[%s] reason [3] invalid memory addr, addr=%p", __func__, addr);
                break;  // this memory address is invalidity
            }

            first_end_token_addr = addr + node->size;
            if (((void *) (first_end_token_addr + MEM_BLOCK_TAIL_LEN) > g_memmgr_end_sentinel)
                    || (*first_end_token_addr != MEM_BLOCK_END_FIRST_TOKEN)
                    || (*(first_end_token_addr + 1) != MEM_BLOCK_END_SECOND_TOKEN)
                    || (*(first_end_token_addr + 2) != MEM_BLOCK_END_THIRD_TOKEN)
                    || (*(first_end_token_addr + 3) != MEM_BLOCK_END_FOUTH_TOKEN)) {
                GF_LOGE(LOG_TAG "[%s] reason [4] invalid memory addr, addr=%p", __func__, addr);
                break;  // this memory address is invalidity
            }
        }

        result = node;
    } while (0);

    VOID_FUNC_EXIT();
    return result;
}

static void *gf_memmgr_memset(void *buff, uint8_t value, int32_t count) {
    uint8_t *tmp = (uint8_t *) buff;
    VOID_FUNC_ENTER();
    do {
        if (NULL == buff) {
            GF_LOGE(LOG_TAG "[%s] buff is NULL", __func__);
            break;
        }

        while (count-- > 0) {
            *tmp++ = value;
        }
    } while (0);

    VOID_FUNC_EXIT();
    return buff;
}

static void *gf_memmgr_memcpy(void *dest, void *src, int32_t count) {
    int32_t i = 0;
    int32_t dest_used_size = 0;
    int32_t src_used_size = 0;
    uint8_t *tmp_dest = (uint8_t *) dest;
    uint8_t *tmp_src = (uint8_t *) src;
    gf_memmgr_node_t *node_dest =
            (gf_memmgr_node_t *) (const void *) (tmp_dest - MEM_BLOCK_HEAD_LEN);
    gf_memmgr_node_t *node_src =
            (gf_memmgr_node_t *) (const void *) (tmp_src - MEM_BLOCK_HEAD_LEN);

    VOID_FUNC_ENTER();
    do {
        int32_t tmp_min = 0;
        if (NULL == tmp_dest || NULL == tmp_src) {
            GF_LOGE(LOG_TAG "[%s] NULL pointer parameter, tmp_dest=%p, tmp_src=%p", __func__,
                    tmp_dest, tmp_src);
            break;
        }

        if (g_mem_config.debug_enable > 0) {
            dest_used_size = node_dest->used_size;
            src_used_size = node_src->used_size;
        } else {
            dest_used_size = node_dest->size;
            src_used_size = node_src->size;
        }

        tmp_min = count < dest_used_size ? count : dest_used_size;
        tmp_min = tmp_min < src_used_size ? tmp_min : src_used_size;
        for (i = 0; i < tmp_min; i++) {
            *(tmp_dest + i) = *(tmp_src + i);
        }
    } while (0);

    VOID_FUNC_EXIT();
    return dest;
}

static gf_memmgr_node_t *gf_memmgr_search_free_block(int32_t size) {
    gf_memmgr_node_t *node = g_free_mem_pool_head;
    gf_memmgr_node_t *result = NULL;
    gf_memmgr_node_t *best_match_node = NULL;

    VOID_FUNC_ENTER();
    do {
        if (NULL == node) {
            GF_LOGE(LOG_TAG "[%s] node is NULL", __func__);
            break;
        }

        if (g_mem_config.debug_enable > 0) {
            if (NULL == gf_memmgr_get_node_by_addr(node->base)) {
                gf_memmgr_dump_corrupt_block(node->base);
                break;
            }
        }

        if (node->size >= size) {  // find out the right of free memory pool.
            if (g_mem_config.match_best_mem_pool_enable > 0) {
                if (node->size - size < MEM_BLOCK_MINIMUM_LEN) {
                    result = node;
                    break;
                }

                if (NULL == best_match_node) {
                    best_match_node = node;
                } else if (best_match_node->size > node->size) {
                    best_match_node = node;
                }

                result = best_match_node;
            } else {
                result = node;
                break;
            }
        }

        node = node->next_free;
    } while (g_free_mem_pool_head != node);

    VOID_FUNC_EXIT();
    return result;
}
#endif

/******************************* interface below **************************/
gf_error_t gf_memmgr_set_config(gf_memmgr_config_t *mem_config) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
#ifdef __SUPPORT_GF_MEMMGR
    do {
        if (NULL == mem_config) {
            GF_LOGE("[%s] bad parameter, mem_config is NULL", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        /* Notice:
         * [memmgr_enable][memmgr_pool_size] can't be changed during test
         * */
        g_mem_config.debug_enable = mem_config->debug_enable;
        g_mem_config.match_best_mem_pool_enable = mem_config->match_best_mem_pool_enable;
        g_mem_config.erase_mem_pool_when_free = mem_config->erase_mem_pool_when_free;
        g_mem_config.dump_time_enable = mem_config->dump_time_enable;
    } while (0);
#else
    UNUSED_VAR(mem_config);
#endif
    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_memmgr_save_config(gf_memmgr_config_t *mem_config) {
    gf_error_t err = GF_SUCCESS;
#ifdef __SUPPORT_GF_MEMMGR
    GF_TEE_HANDLE so_handle = GF_TEE_HANDLE_NULL;
    uint32_t len = 0;
    uint8_t buf[32] = { 0 };  // this arr do not malloc
    uint8_t *tmp = buf;
#endif
    FUNC_ENTER();

#ifdef __SUPPORT_GF_MEMMGR
    do {
        if (NULL == mem_config) {
            GF_LOGE("[%s] bad parameter, mem_config is NULL", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        // token + memmgr_enable
        len += MEMMGR_TOKEN_LENGTH + sizeof(uint8_t);
        // token + debug_enable
        len += MEMMGR_TOKEN_LENGTH + sizeof(uint8_t);
        // token + match_best_mem_pool_enable
        len += MEMMGR_TOKEN_LENGTH + sizeof(uint8_t);
        // token + erase_mem_pool_when_free
        len += MEMMGR_TOKEN_LENGTH + sizeof(uint8_t);
        // token + dump_time_enable
        len += MEMMGR_TOKEN_LENGTH + sizeof(uint8_t);
        // token + memmgr_pool_size
        len += MEMMGR_TOKEN_LENGTH + sizeof(uint32_t);

        *tmp++ = GF_MEMMGR_TOKEN_MEMMGR_ENABLE;
        *tmp++ = mem_config->memmgr_enable;

        *tmp++ = GF_MEMMGR_TOKEN_DEBUG_ENABLE;
        *tmp++ = mem_config->debug_enable;

        *tmp++ = GF_MEMMGR_TOKEN_MATCH_BEST_ENABLE;
        *tmp++ = mem_config->match_best_mem_pool_enable;

        *tmp++ = GF_MEMMGR_TOKEN_ERASE_FREE_ENABLE;
        *tmp++ = mem_config->erase_mem_pool_when_free;

        *tmp++ = GF_MEMMGR_TOKEN_DUMP_TIME_ENABLE;
        *tmp++ = mem_config->dump_time_enable;

        *tmp++ = GF_MEMMGR_TOKEN_MEMMGR_POOL_SIZE;
        *tmp++ = (mem_config->memmgr_pool_size >> 24) & 0xFF;
        *tmp++ = (mem_config->memmgr_pool_size >> 16) & 0xFF;
        *tmp++ = (mem_config->memmgr_pool_size >> 8) & 0xFF;
        *tmp = mem_config->memmgr_pool_size & 0xFF;

#if (defined __TEE_NUTLET) || (defined __TEE_TRUSTKERNEL)
        gf_tee_delete_object((uint8_t*) GF_MEMMGR_CONFIG_ID);

        err = gf_tee_write_object_data((uint8_t*) GF_MEMMGR_CONFIG_ID, &so_handle, buf, len);
#else
        err = gf_tee_open_object((uint8_t*) GF_MEMMGR_CONFIG_ID, SO_MODE_WRITE, &so_handle);
        if (GF_SUCCESS != err) {
            break;
        }

        err = gf_tee_write_object_data(so_handle, buf, len);
#endif

        g_mem_config.enable_memmgr_next_reboot = mem_config->memmgr_enable;
    } while (0);

    if (GF_TEE_HANDLE_NULL != so_handle) {
        gf_tee_close_object(so_handle);
    }
#else
    UNUSED_VAR(mem_config);
#endif
    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_memmgr_remove_config(void) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
#ifdef __SUPPORT_GF_MEMMGR
    err = gf_tee_delete_object((uint8_t*) GF_MEMMGR_CONFIG_ID);
    if (err != GF_SUCCESS) {
        GF_LOGE(LOG_TAG "[%s] delete so=%s failed", __func__, GF_MEMMGR_CONFIG_ID);
    }

    g_mem_config.enable_memmgr_next_reboot = 0;
#endif
    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_memmgr_load_config(void) {
    gf_error_t err = GF_SUCCESS;

#ifdef __SUPPORT_GF_MEMMGR
    GF_TEE_HANDLE so_handle = GF_TEE_HANDLE_NULL;
    uint8_t *data = NULL;
    uint32_t data_len = 0;
    uint32_t size = 0;
#endif
    FUNC_ENTER();

#ifdef __SUPPORT_GF_MEMMGR
    do {
        uint8_t *buf = NULL;
        err = gf_tee_open_object((uint8_t*) GF_MEMMGR_CONFIG_ID, SO_MODE_READ, &so_handle);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] not found config file, so=%s ", __func__,
                    (uint8_t*) GF_MEMMGR_CONFIG_ID);
            break;
        }

        err = gf_tee_get_object_size(so_handle, &data_len);
        if (GF_SUCCESS != err || ((int32_t) data_len) <= 0) {
            GF_LOGE(LOG_TAG "[%s] config file object corrupt. size=%u", __func__, data_len);
            err = GF_ERROR_READ_SECURE_OBJECT_FAILED;
            break;
        }

        data = (uint8_t *) gf_malloc(data_len);
        if (NULL == data) {
            err = GF_ERROR_OUT_OF_MEMORY;
            GF_LOGE(LOG_TAG "[%s] malloc memory(%u bytes) failed", __func__, data_len);
            break;
        }

        err = gf_tee_read_object_data(so_handle, data, data_len);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] read config file(%s) failed", __func__,
                    (uint8_t*) GF_MEMMGR_CONFIG_ID);
            break;
        }

        buf = data;
        // memmgr_enable
        if (GF_MEMMGR_TOKEN_MEMMGR_ENABLE != *buf++) {
            GF_LOGE(LOG_TAG "[%s] token GF_MEMMGR_TOKEN_MEMMGR_ENABLE error", __func__);
            err = GF_ERROR_INVALID_DATA;
            break;
        }
        g_mem_config.memmgr_enable = *buf++;

        // debug_enable
        if (GF_MEMMGR_TOKEN_DEBUG_ENABLE != *buf++) {
            GF_LOGE(LOG_TAG "[%s] token GF_MEMMGR_TOKEN_DEBUG_ENABLE error", __func__);
            err = GF_ERROR_INVALID_DATA;
            break;
        }
        g_mem_config.debug_enable = *buf++;

        // match_best_mem_pool_enable
        if (GF_MEMMGR_TOKEN_MATCH_BEST_ENABLE != *buf++) {
            GF_LOGE(LOG_TAG "[%s] token GF_MEMMGR_TOKEN_MATCH_BEST_ENABLE error", __func__);
            err = GF_ERROR_INVALID_DATA;
            break;
        }
        g_mem_config.match_best_mem_pool_enable = *buf++;

        // erase_mem_pool_when_free
        if (GF_MEMMGR_TOKEN_ERASE_FREE_ENABLE != *buf++) {
            GF_LOGE(LOG_TAG "[%s] token GF_MEMMGR_TOKEN_ERASE_FREE_ENABLE error", __func__);
            err = GF_ERROR_INVALID_DATA;
            break;
        }
        g_mem_config.erase_mem_pool_when_free = *buf++;

        // dump_time_enable
        if (GF_MEMMGR_TOKEN_DUMP_TIME_ENABLE != *buf++) {
            GF_LOGE(LOG_TAG "[%s] token GF_MEMMGR_TOKEN_DUMP_TIME_ENABLE error", __func__);
            err = GF_ERROR_INVALID_DATA;
            break;
        }
        g_mem_config.dump_time_enable = *buf++;

        // dump_time_enable
        if (GF_MEMMGR_TOKEN_MEMMGR_POOL_SIZE != *buf++) {
            GF_LOGE(LOG_TAG "[%s] token GF_MEMMGR_TOKEN_MEMMGR_POOL_SIZE error", __func__);
            err = GF_ERROR_INVALID_DATA;
            break;
        }
        size |= *buf++ << 24;
        size |= *buf++ << 16;
        size |= *buf++ << 8;
        size |= *buf;
        g_mem_config.memmgr_pool_size = size;
    } while (0);

    if (GF_SUCCESS != err) {
        // ta run as normal mode...
        g_mem_config.memmgr_enable = 0;
        g_mem_config.debug_enable = 0;
        g_mem_config.match_best_mem_pool_enable = 0;
        g_mem_config.erase_mem_pool_when_free = 0;
        g_mem_config.dump_time_enable = 0;
        g_mem_config.memmgr_pool_size = 0;
        g_mem_config.enable_memmgr_next_reboot = 0;
    }

    GF_LOGI(LOG_TAG "[%s] g_mem_config:               memmgr_enable=%d", __func__,
            g_mem_config.memmgr_enable);
    GF_LOGI(LOG_TAG "[%s] g_mem_config:                debug_enable=%d", __func__,
            g_mem_config.debug_enable);
    GF_LOGI(LOG_TAG "[%s] g_mem_config:  match_best_mem_pool_enable=%d", __func__,
            g_mem_config.match_best_mem_pool_enable);
    GF_LOGI(LOG_TAG "[%s] g_mem_config:    erase_mem_pool_when_free=%d", __func__,
            g_mem_config.erase_mem_pool_when_free);
    GF_LOGI(LOG_TAG "[%s] g_mem_config:            dump_time_enable=%d", __func__,
            g_mem_config.dump_time_enable);
    GF_LOGI(LOG_TAG "[%s] g_mem_config:            memmgr_pool_size=%d", __func__,
            g_mem_config.memmgr_pool_size);
    GF_LOGI(LOG_TAG "[%s] g_mem_config:   enable_memmgr_next_reboot=%d", __func__,
            g_mem_config.enable_memmgr_next_reboot);

    if (data != NULL) {
        gf_free(data);
    }

    if (GF_TEE_HANDLE_NULL != so_handle) {
        gf_tee_close_object(so_handle);
    }
#endif
    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_memmgr_get_manager_info(gf_memmgr_info_t *memmgr_info) {
    gf_error_t err = GF_SUCCESS;
#ifdef __SUPPORT_GF_MEMMGR
    gf_memmgr_node_t *tmp_node = NULL;
    uint8_t *tmp_callstack = NULL;
    uint32_t callstack_len = 0;
    uint32_t transport_node_count = 0;
    uint32_t max_transport_node_count = DUMP_MEM_NODE_BUF_LEN / GF_MEMMGR_INFO_SIZE_PER_NODE;
    uint32_t i = 0;
    uint64_t tmp_node_base = 0;
#endif
    FUNC_ENTER();

#ifdef __SUPPORT_GF_MEMMGR
    do {
        uint8_t *tmp_info = NULL;
        if (NULL == memmgr_info) {
            GF_LOGE(LOG_TAG "[%s] memmgr_info is NULL", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        tmp_info = memmgr_info->mem_pool_node_data;

        // copy mem_pool used info
        if (g_mem_config.memmgr_enable > 0) {
            memmgr_info->cur_used_block_count = g_cur_used_block_count;
            memmgr_info->max_used_block_count = g_max_used_block_count;
            memmgr_info->cur_used_mem_size = g_cur_used_mem_size;
            memmgr_info->maxs_used_mem_size = g_maxs_used_mem_size;
            memmgr_info->mem_pool_start_addr = (long) g_memmgr_start_sentinel;  // NOLINT
            memmgr_info->mem_pool_end_addr = (long) g_memmgr_end_sentinel;  // NOLINT
        } else {
            GF_LOGE(LOG_TAG "[%s] memmgr_enable is not enable", __func__);
            err = GF_ERROR_GENERIC;
            break;
        }

        // copy node info to mem_pool_node_data
        memmgr_info->total_node_count = g_total_node_count;
        memmgr_info->node_info_size = GF_MEMMGR_INFO_SIZE_PER_NODE;
        if (NULL == g_mem_pool_head) {
            GF_LOGE(LOG_TAG "[%s] g_mem_pool_head is NULL", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        GF_LOGD(LOG_TAG "[%s] next_start_node = %d", __func__, memmgr_info->next_start_node);

        tmp_node = g_mem_pool_head;

        // Consider compatible with transport that can't be done one time in the future test
        for (i = 0; i < memmgr_info->next_start_node; i++) {
            tmp_node = tmp_node->next;
        }

        do {
            tmp_node_base = (long) tmp_node->base;  // NOLINT
            cpl_memcpy(tmp_info, &tmp_node_base, sizeof(tmp_node_base));
            tmp_info += sizeof(tmp_node_base);

            cpl_memcpy(tmp_info, &tmp_node->size, sizeof(tmp_node->size));
            tmp_info += sizeof(tmp_node->size);

            cpl_memcpy(tmp_info, &tmp_node->used_size, sizeof(tmp_node->used_size));
            tmp_info += sizeof(tmp_node->used_size);

            cpl_memcpy(tmp_info, &tmp_node->flag, sizeof(tmp_node->flag));
            tmp_info += sizeof(tmp_node->flag);

            cpl_memcpy(tmp_info, &tmp_node->time, sizeof(tmp_node->time));
            tmp_info += sizeof(tmp_node->time);

            tmp_callstack = (uint8_t*) tmp_node->base + tmp_node->size
                    + MEM_BLOCK_END_TOKEN_BYTES_COUNT;

            callstack_len = cpl_strlen((void*) tmp_callstack);
            cpl_memcpy(tmp_info, &callstack_len, sizeof(callstack_len));
            tmp_info += sizeof(callstack_len);

            cpl_memcpy(tmp_info, tmp_callstack, CALL_STACK_MAX_BYTES);
            tmp_info += CALL_STACK_MAX_BYTES;

            tmp_node = tmp_node->next;
            transport_node_count++;
        } while ((transport_node_count < max_transport_node_count)
                && (tmp_node != g_mem_pool_head));

        memmgr_info->transport_node_count = transport_node_count;
        // Consider compatible with transport that can't be done one time in the future test
        if (tmp_node == g_mem_pool_head) {  // finish transport
            memmgr_info->next_start_node = 0;
        } else {
            memmgr_info->next_start_node += transport_node_count;
        }

        GF_LOGD(LOG_TAG "[%s] transport_node_count = %d",
                __func__, memmgr_info->transport_node_count);
    } while (0);
#else
    UNUSED_VAR(memmgr_info);
#endif
    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_memmgr_dump_mem_pool(gf_memmgr_pool_dump_t *pool_dump) {
    gf_error_t err = GF_SUCCESS;

#ifndef __SUPPORT_GF_MEMMGR
    UNUSED_VAR(pool_dump);
#endif

    FUNC_ENTER();

#ifdef __SUPPORT_GF_MEMMGR
    do {
        uint8_t *src_data = NULL;
        uint8_t *dest_data = NULL;
        uint32_t cur_dump_len = 0;
        if (NULL == pool_dump) {
            GF_LOGE(LOG_TAG "[%s] pool_dump is NULL", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        dest_data = pool_dump->pool_arr;
        src_data = (uint8_t *) g_memmgr_start_sentinel + pool_dump->offset;

        while (cur_dump_len < DUMP_MEM_POOL_BUF_LEN
                && src_data < (uint8_t*) g_memmgr_end_sentinel) {
            *dest_data++ = *src_data++;
            cur_dump_len++;
        }

        if (src_data >= (uint8_t*) g_memmgr_end_sentinel) {
            pool_dump->finished = 1;
        }

        pool_dump->dump_len = cur_dump_len;
    } while (0);
#endif

    FUNC_EXIT(err);
    return err;
}

static gf_error_t gf_memmgr_init(void) {
    gf_error_t err = GF_SUCCESS;
    uint64_t time = 0;
    FUNC_ENTER();

    do {
        if (g_mem_config.memmgr_pool_size <= MEM_BLOCK_MINIMUM_LEN) {
            GF_LOGE(LOG_TAG "[%s] mem_len is invalid, mem_len=%d",
                    __func__, g_mem_config.memmgr_pool_size);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (NULL != g_mem_pool_head) {
            GF_LOGE(LOG_TAG "[%s] g_mem_pool_head is not NULL", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        g_memmgr_start_sentinel = (void *) gf_malloc(g_mem_config.memmgr_pool_size);
        if (NULL == g_memmgr_start_sentinel) {
            err = GF_ERROR_OUT_OF_MEMORY;
            GF_LOGE(LOG_TAG "[%s] malloc failed err=%s, errno=%d", __func__, gf_strerror(err), err);
            break;
        }
        g_memmgr_end_sentinel = (void *) ((uint8_t *) g_memmgr_start_sentinel
                + g_mem_config.memmgr_pool_size);

        gf_get_timestamp(&time);
        g_memmgr_start_time = time;

        /**
         * Initialized global variable #g_mem_pool_head. Divided the whole memory
         * into a whole memory block.
         */
        g_mem_pool_head = (gf_memmgr_node_t *) g_memmgr_start_sentinel;
        g_mem_pool_head->flag = MEM_BLOCK_FLAG_FREE;
        g_mem_pool_head->base =
                (void *) ((uint8_t *) g_memmgr_start_sentinel + MEM_BLOCK_HEAD_LEN);
        g_mem_pool_head->next = g_mem_pool_head;
        g_mem_pool_head->prev = g_mem_pool_head;

        // init free memory pool liked list
        g_free_mem_pool_head = g_mem_pool_head;
        g_free_mem_pool_head->next_free = g_free_mem_pool_head;
        g_free_mem_pool_head->prev_free = g_free_mem_pool_head;

        g_mem_pool_head->size = g_mem_config.memmgr_pool_size - MEM_BLOCK_MINIMUM_LEN;

        if (g_mem_config.debug_enable > 0) {
            // initialize the start token
            uint8_t *tmp_addr = g_mem_pool_head->base;

            *(--tmp_addr) = MEM_BLOCK_START_FOUTH_TOKEN;
            *(--tmp_addr) = MEM_BLOCK_START_THIRD_TOKEN;
            *(--tmp_addr) = MEM_BLOCK_START_SECOND_TOKEN;
            *(--tmp_addr) = MEM_BLOCK_START_FIRST_TOKEN;

            // initialize the end token
            tmp_addr = (uint8_t *) (g_mem_pool_head->base) + g_mem_pool_head->size;
            *(tmp_addr++) = MEM_BLOCK_END_FIRST_TOKEN;
            *(tmp_addr++) = MEM_BLOCK_END_SECOND_TOKEN;
            *(tmp_addr++) = MEM_BLOCK_END_THIRD_TOKEN;
            *(tmp_addr++) = MEM_BLOCK_END_FOUTH_TOKEN;

            g_mem_pool_head->used_size = 0;

            g_cur_used_block_count = 0;
            g_max_used_block_count = 0;
            g_cur_used_mem_size = 0;
            g_maxs_used_mem_size = 0;
        }

        g_total_node_count = 1;
    } while (0);

    if (GF_SUCCESS != err) {
        gf_memmgr_destroy_entry_point();
    }

    FUNC_EXIT(err);
    return err;
}

void gf_memmgr_destroy_entry_point(void) {
    VOID_FUNC_ENTER();

    g_mem_pool_head = NULL;
    gf_free(g_memmgr_start_sentinel);
    g_memmgr_start_sentinel = NULL;
    g_memmgr_end_sentinel = NULL;
    g_free_mem_pool_head = NULL;

    if (g_mem_config.debug_enable > 0) {
        g_cur_used_block_count = 0;
        g_max_used_block_count = 0;
        g_cur_used_mem_size = 0;
        g_maxs_used_mem_size = 0;
    }

    g_total_node_count = 0;

    VOID_FUNC_EXIT();
}

void *gf_memmgr_malloc(int32_t size, uint8_t default_value, const int8_t *file_name,
        const int8_t *func_name, int32_t line) {
    void *result = NULL;

#ifdef __SUPPORT_GF_MEMMGR
    gf_memmgr_node_t *node = NULL;
    uint8_t *tmp_addr = NULL;
#else
    UNUSED_VAR(size);
    UNUSED_VAR(default_value);
    UNUSED_VAR(file_name);
    UNUSED_VAR(func_name);
    UNUSED_VAR(line);
#endif

    VOID_FUNC_ENTER();

#ifdef __SUPPORT_GF_MEMMGR
    do {
        int32_t remained_len = 0;
        if (size <= 0) {
            GF_LOGE(LOG_TAG "[%s] without free memory pool block. filename=%s, func=%s, line=%d",
                    __func__, file_name, func_name, line);
            break;
        }

#ifdef MEM_IS_16BIT_CPU
        size = (size + 1) & ~1;
#else
        size = (size + 3) & ~3;
#endif

        node = gf_memmgr_search_free_block(size);
        if (NULL == node) {
            GF_LOGE(LOG_TAG "[%s] search free block failed", __func__);
            break;
        }

        remained_len = node->size - size;
        if (remained_len > (int32_t) MEM_BLOCK_MINIMUM_LEN) {
            // this block can be divided into two blocks
            gf_memmgr_node_t *new_node = NULL;

            new_node = (gf_memmgr_node_t *) (const void *) ((int8_t *) node + MEM_BLOCK_LEN(size));
            new_node->flag = MEM_BLOCK_FLAG_FREE;
            new_node->base = (void *) ((int8_t *) new_node + MEM_BLOCK_HEAD_LEN);
            new_node->next = node->next;
            new_node->prev = node;
            new_node->size = remained_len - MEM_BLOCK_MINIMUM_LEN;
            node->next = new_node;
            node->size = size;

            if (g_mem_config.debug_enable > 0) {
                new_node->used_size = 0;
                // initialize start token
                tmp_addr = new_node->base;
                *(--tmp_addr) = MEM_BLOCK_START_FOUTH_TOKEN;
                *(--tmp_addr) = MEM_BLOCK_START_THIRD_TOKEN;
                *(--tmp_addr) = MEM_BLOCK_START_SECOND_TOKEN;
                *(--tmp_addr) = MEM_BLOCK_START_FIRST_TOKEN;

                node->used_size = size;
                // offset to the first bytes of end token.
                tmp_addr = (uint8_t *) node->base + size;
                *(tmp_addr++) = MEM_BLOCK_END_FIRST_TOKEN;
                *(tmp_addr++) = MEM_BLOCK_END_SECOND_TOKEN;
                *(tmp_addr++) = MEM_BLOCK_END_THIRD_TOKEN;
                *(tmp_addr++) = MEM_BLOCK_END_FOUTH_TOKEN;
            }

            new_node->next->prev = new_node;
            if (node == node->next_free) {
                // only one free memory pool node
                g_free_mem_pool_head = new_node->next_free = new_node->prev_free = new_node;
            } else {
                new_node->next_free = node->next_free;
                new_node->prev_free = node->prev_free;
                node->prev_free->next_free = new_node;
                node->next_free->prev_free = new_node;

                if (node == g_free_mem_pool_head) {
                    g_free_mem_pool_head = new_node;
                }
            }

            g_total_node_count++;
        } else {
            if (node == node->next_free) {
                // only one free memory pool node
                g_free_mem_pool_head = NULL;
            } else {
                node->prev_free->next_free = node->next_free;
                node->next_free->prev_free = node->prev_free;

                if (node == g_free_mem_pool_head) {
                    g_free_mem_pool_head = node->next_free;
                }
            }

            if (g_mem_config.debug_enable > 0) {
                node->used_size = size;
            }
        }

        node->flag = MEM_BLOCK_FLAG_USED;
        gf_memmgr_memset(node->base, default_value, node->size);

        if (g_mem_config.debug_enable > 0) {
            gf_memmgr_save_call_stack(node, file_name, func_name, line);

            g_cur_used_block_count++;
            g_cur_used_mem_size += MEM_BLOCK_LEN(node->size);

            if (g_cur_used_mem_size > g_maxs_used_mem_size) {
                g_maxs_used_mem_size = g_cur_used_mem_size;
                g_max_used_block_count = g_cur_used_block_count;
            }
        }

        result = node->base;
    }while(0);
#endif

    VOID_FUNC_EXIT();
    return result;
}

void *gf_memmgr_realloc(void *src, int32_t size, const int8_t *file_name, const int8_t *func_name,
        int32_t line) {
    void *new_buff = NULL;
#ifndef __SUPPORT_GF_MEMMGR
    UNUSED_VAR(src);
    UNUSED_VAR(size);
    UNUSED_VAR(file_name);
    UNUSED_VAR(func_name);
    UNUSED_VAR(line);
#endif
    VOID_FUNC_ENTER();

#ifdef __SUPPORT_GF_MEMMGR
    do {
        gf_memmgr_node_t *node = NULL;
        // allocates a new block of size bytes
        if (NULL == src) {
            new_buff = gf_memmgr_malloc(size, 0, file_name, func_name, line);
            break;
        }

        if (0 == size) {
            GF_LOGE(LOG_TAG "[%s] realloc size is 0", __func__);
            break;
        }

#ifdef MEM_IS_16BIT_CPU
        size = (size + 1) & ~1;
#else
        size = (size + 3) & ~3;
#endif

        node = (gf_memmgr_node_t *) (const void *) ((int8_t *) src - MEM_BLOCK_HEAD_LEN);

        /**
         * Check the current block of memory is large enough for new size, If it is enough,
         * then return the same address, don't need to allocate new memory block.
         */
        if (node->size >= size) {
            if (g_mem_config.debug_enable > 0) {
                node->used_size = size;
                gf_memmgr_save_call_stack(node, file_name, func_name, line);
            }

            new_buff = src;
            break;
        }

        /**
         *  Allocate new memory block, If success, then copy original memory block into new
         *  memory block and release the original block. otherwise do nothing.
         */
        new_buff = gf_memmgr_malloc(size, 0, file_name, func_name, line);
        if (NULL != new_buff) {
            if (g_mem_config.debug_enable > 0) {
                gf_memmgr_memcpy(new_buff, src, node->used_size);
            } else {
                gf_memmgr_memcpy(new_buff, src, node->size);
            }
            gf_memmgr_free(src);
        }
    } while (0);
#endif

    VOID_FUNC_EXIT();
    return new_buff;
}

void gf_memmgr_free(void *mem_addr) {
#ifdef __SUPPORT_GF_MEMMGR
    gf_memmgr_node_t *node = NULL;
    gf_memmgr_node_t *prev = NULL;
    gf_memmgr_node_t *next = NULL;
#else
    UNUSED_VAR(mem_addr);
#endif
    VOID_FUNC_ENTER();

#ifdef __SUPPORT_GF_MEMMGR
    do {
        if (NULL == mem_addr) {
            // nothing to do
            break;
        }

        node = gf_memmgr_get_node_by_addr(mem_addr);
        if (NULL == node) {  // the current memory address is not validity
            GF_LOGE(LOG_TAG "[%s] node is NULL", __func__);
            break;
        }

        if (MEM_BLOCK_FLAG_FREE == node->flag) {
            break;
        }

        // only one memory pool node is exist
        if (node == node->next) {
            node->flag = MEM_BLOCK_FLAG_FREE;
            if (g_mem_config.debug_enable > 0) {
                node->used_size = 0;
                g_cur_used_block_count--;
                g_cur_used_mem_size -= MEM_BLOCK_LEN(node->size);
            }

            g_free_mem_pool_head = node->next_free = node->prev_free = node;

            if (g_mem_config.erase_mem_pool_when_free > 0) {
                gf_memmgr_memset(node->base, 0, node->size);
            }

            break;
        }

        /**
         * Check the previous and next pool node, judge if need to reconstruct the pool node
         */
        prev = node->prev;
        next = node->next;
        if ((MEM_BLOCK_FLAG_FREE == prev->flag) || (MEM_BLOCK_FLAG_FREE == next->flag)) {
            if (g_mem_pool_head == node) {  // node is the first node
                if (g_mem_config.debug_enable > 0) {
                    g_cur_used_block_count--;
                    g_cur_used_mem_size -= MEM_BLOCK_LEN(node->size);
                    node->used_size = 0;
                }

                node->flag = MEM_BLOCK_FLAG_FREE;

                // and the next node is free, so merge into one block.
                if (MEM_BLOCK_FLAG_FREE == next->flag) {
                    node->next = next->next;
                    next->next->prev = node;
                    node->size += MEM_BLOCK_LEN(next->size);

                    if (next == next->next_free) {  // only one free node left.
                        node->next_free = node->prev_free = node;
                    } else {
                        node->next_free = next->next_free;
                        next->next_free->prev_free = node;
                        next->prev_free->next_free = node;
                        node->prev_free = next->prev_free;
                    }

                    g_total_node_count--;
                } else {  // and the next node is not free.
                    if (NULL != g_free_mem_pool_head) {
                        node->prev_free = g_free_mem_pool_head->prev_free;
                        g_free_mem_pool_head->prev_free->next_free = node;

                        node->next_free = g_free_mem_pool_head;
                        g_free_mem_pool_head->prev_free = node;
                    } else {
                        node->next_free = node->prev_free = node;
                    }
                }

                if (g_mem_config.erase_mem_pool_when_free > 0) {
                    gf_memmgr_memset(node->base, 0, node->size);
                }

                g_free_mem_pool_head = node;
                break;
            }

            if (g_mem_pool_head->prev == node) {  // node is the last node
                if (g_mem_config.debug_enable > 0) {
                    g_cur_used_block_count--;
                    g_cur_used_mem_size -= MEM_BLOCK_LEN(node->size);
                }

                // node is the last node and the previous node is free, so merge
                if (MEM_BLOCK_FLAG_FREE == prev->flag) {
                    prev->next = node->next;
                    node->next->prev = prev;
                    prev->size += MEM_BLOCK_LEN(node->size);

                    if (g_mem_config.erase_mem_pool_when_free > 0) {
                        gf_memmgr_memset(prev->base, 0, prev->size);
                    }

                    g_total_node_count--;
                } else {
                    node->flag = MEM_BLOCK_FLAG_FREE;

                    if (g_mem_config.debug_enable > 0) {
                        node->used_size = 0;
                    }

                    if (NULL != g_free_mem_pool_head) {
                        node->prev_free = g_free_mem_pool_head->prev_free;
                        g_free_mem_pool_head->prev_free->next_free = node;

                        node->next_free = g_free_mem_pool_head;
                        g_free_mem_pool_head->prev_free = node;
                    } else {
                        g_free_mem_pool_head = node;
                    }

                    if (g_mem_config.erase_mem_pool_when_free > 0) {
                        gf_memmgr_memset(node->base, 0, node->size);
                    }
                }
                break;
            }

            // node is the middle node
            if ((MEM_BLOCK_FLAG_FREE == prev->flag) && (MEM_BLOCK_FLAG_FREE == next->flag)) {
                if (g_mem_config.debug_enable > 0) {
                    g_cur_used_block_count--;
                    g_cur_used_mem_size -= MEM_BLOCK_LEN(node->size);
                }

                prev->next = next->next;
                next->next->prev = prev;
                prev->size += MEM_BLOCK_LEN(node->size) + MEM_BLOCK_LEN(next->size);
                prev->next_free = next->next_free;
                next->next_free->prev_free = prev;

                if (g_mem_config.erase_mem_pool_when_free > 0) {
                    gf_memmgr_memset(prev->base, 0, prev->size);
                }

                g_total_node_count -= 2;
                break;
            }

            if (MEM_BLOCK_FLAG_FREE == prev->flag) {  // prev is free and next is not free
                if (g_mem_config.debug_enable > 0) {
                    g_cur_used_block_count--;
                    g_cur_used_mem_size -= MEM_BLOCK_LEN(node->size);
                }

                prev->next = next;
                next->prev = prev;
                prev->size += MEM_BLOCK_LEN(node->size);

                if (g_mem_config.erase_mem_pool_when_free > 0) {
                    gf_memmgr_memset(prev->base, 0, prev->size);
                }

                g_total_node_count--;
                break;
            }

            // prev is not free and next is free
            if (g_mem_config.debug_enable > 0) {
                g_cur_used_block_count--;
                g_cur_used_mem_size -= MEM_BLOCK_LEN(node->size);
                node->used_size = 0;
            }

            next->next->prev = node;
            node->next = next->next;
            node->size += MEM_BLOCK_LEN(next->size);
            node->flag = MEM_BLOCK_FLAG_FREE;
            if (next->next_free == next) {  // next is the last free block
                g_free_mem_pool_head = node->next_free = node->prev_free = node;
            } else {
                node->next_free = next->next_free;
                next->next_free->prev_free = node;
                next->prev_free->next_free = node;
                node->prev_free = next->prev_free;

                if (g_free_mem_pool_head == next) {
                    g_free_mem_pool_head = node;
                }
            }

            if (g_mem_config.erase_mem_pool_when_free > 0) {
                gf_memmgr_memset(node->base, 0, node->size);
            }

            g_total_node_count--;
            break;
        }

        // the previous and next node both are used
        node->flag = MEM_BLOCK_FLAG_FREE;

        if (NULL == g_free_mem_pool_head) {
            node->next_free = node->prev_free = g_free_mem_pool_head = node;
        } else {
            gf_memmgr_node_t *tmp = g_free_mem_pool_head;

            while ((tmp < node) && (tmp->next_free != g_free_mem_pool_head)) {
                tmp = tmp->next_free;
            }

            node->prev_free = tmp->prev_free;
            tmp->prev_free->next_free = node;

            node->next_free = tmp;
            tmp->prev_free = node;

            if (g_free_mem_pool_head > node) {
                g_free_mem_pool_head = node;
            }
        }

        if (g_mem_config.debug_enable > 0) {
            g_cur_used_block_count--;
            g_cur_used_mem_size -= MEM_BLOCK_LEN(node->size);
            node->used_size = 0;
        }

        if (g_mem_config.erase_mem_pool_when_free > 0) {
            gf_memmgr_memset(node->base, 0, node->size);
        }
    } while (0);
#endif

    VOID_FUNC_EXIT();
}

gf_error_t gf_memmgr_check_addr_validity(const void *mem_addr) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
#ifdef __SUPPORT_GF_MEMMGR
    do {
        gf_memmgr_node_t *node = gf_memmgr_get_node_by_addr(mem_addr);

        if (NULL == node) {
            err = GF_ERROR_INVALID_DATA;
            break;
        }

        if (MEM_BLOCK_FLAG_USED != node->flag) {
            err = GF_ERROR_INVALID_DATA;
            break;
        }
    } while (0);
#else
    UNUSED_VAR(mem_addr);
#endif
    FUNC_EXIT(err);
    return err;
}

void gf_memmgr_dump_node(const void *mem_addr) {
    VOID_FUNC_ENTER();
#ifdef __SUPPORT_GF_MEMMGR
    if (GF_SUCCESS == gf_memmgr_check_addr_validity(mem_addr)) {  // this memory is valid
        uint8_t *tmp_addr = (uint8_t *) ((const uint8_t *) mem_addr
                - MEM_BLOCK_START_TOKEN_BYTES_COUNT);
        gf_memmgr_node_t *node = (gf_memmgr_node_t *) (const void *) ((const uint8_t *) mem_addr
                - MEM_BLOCK_HEAD_LEN);

        GF_LOGI(LOG_TAG "[%s] Valid Memory Pool, this=%p", __func__, (void *) node);
        GF_LOGI(LOG_TAG "[%s] Valid Memory Pool, base=%p", __func__, (void *) (node->base));
        GF_LOGI(LOG_TAG "[%s] Valid Memory Pool, prev=%p", __func__, (void *) (node->prev));
        GF_LOGI(LOG_TAG "[%s] Valid Memory Pool, next=%p", __func__, (void *) (node->next));
        GF_LOGI(LOG_TAG "[%s] Valid Memory Pool, size=%d", __func__, node->size);
        GF_LOGI(LOG_TAG "[%s] Valid Memory Pool, used=%d", __func__, node->used_size);
        GF_LOGI(LOG_TAG "[%s] Valid Memory Pool, flag=%d", __func__, node->flag);
        GF_LOGI(LOG_TAG "[%s] Valid Memory Pool, start token=0x%X,0x%X,0x%X,0x%X", __func__,
                *tmp_addr, *(tmp_addr + 1), *(tmp_addr + 2), *(tmp_addr + 3));

        tmp_addr = (uint8_t *) ((const uint8_t *) mem_addr + node->size);
        GF_LOGI(LOG_TAG "[%s] Valid Memory Pool, end token=0x%X,0x%X,0x%X,0x%X", __func__,
                *tmp_addr, *(tmp_addr + 1), *(tmp_addr + 2), *(tmp_addr + 3));
        GF_LOGI(LOG_TAG "[%s] Valid Memory Pool, call stack=%s", __func__,
                tmp_addr + MEM_BLOCK_END_TOKEN_BYTES_COUNT);
    } else {
        GF_LOGI(LOG_TAG "[%s] Invalid Memory Pool, this=%p", __func__, mem_addr);
    }
#else
    UNUSED_VAR(mem_addr);
#endif
    VOID_FUNC_EXIT();
}

void gf_memmgr_dump_leak_info(void) {
#ifdef __SUPPORT_GF_MEMMGR
    int32_t has_damaged_pool = 0;
    uint32_t leak_mem_pool_count = 0;
    gf_memmgr_node_t *node = NULL;
    gf_memmgr_node_t *last_valid_node = NULL;

// sequential search
    node = g_mem_pool_head;
#endif

    VOID_FUNC_ENTER();

#ifdef __SUPPORT_GF_MEMMGR
    do {
        if (NULL != gf_memmgr_get_node_by_addr(node->base)) {
            if (MEM_BLOCK_FLAG_USED == node->flag) {
                gf_memmgr_dump_node(node->base);

                leak_mem_pool_count++;
            }
            last_valid_node = node;
            node = node->next;
        } else {
            has_damaged_pool = 1;
            break;
        }
    } while (node != g_mem_pool_head);

    GF_LOGI(LOG_TAG "[%s] memory pools are leak , leak_mem_pool_count=%d", __func__,
            leak_mem_pool_count);

    if (has_damaged_pool) {  // Didn't check all memory pools and find damaged memory pool
        if (NULL != last_valid_node) {
            gf_memmgr_dump_corrupt_block(last_valid_node->next + MEM_BLOCK_HEAD_LEN);
        } else {
            gf_memmgr_dump_corrupt_block(node->base);
        }
    }
#endif

    VOID_FUNC_EXIT();
}

void gf_memmgr_dump_usage_info(void) {
#ifdef __SUPPORT_GF_MEMMGR
    int32_t total = (int32_t) ((const uint8_t *) g_memmgr_end_sentinel
            - (const uint8_t *) g_memmgr_start_sentinel);
#endif

    VOID_FUNC_ENTER();
#ifdef __SUPPORT_GF_MEMMGR
    GF_LOGI(LOG_TAG "[%s] Memory Usage Information,                       total=%d", __func__,
            total);
    GF_LOGI(LOG_TAG "[%s] Memory Usage Information,        max_used_memory_size=%d", __func__,
            g_maxs_used_mem_size);
    GF_LOGI(LOG_TAG "[%s] Memory Usage Information,         max_used_pool_count=%d", __func__,
            g_max_used_block_count);
    GF_LOGI(LOG_TAG "[%s] Memory Usage Information,        cur_used_memory_size=%d", __func__,
            g_cur_used_mem_size);
    GF_LOGI(LOG_TAG "[%s] Memory Usage Information, cur_used_memory_block_count=%d", __func__,
            g_cur_used_block_count);
#endif

    VOID_FUNC_EXIT();
}
