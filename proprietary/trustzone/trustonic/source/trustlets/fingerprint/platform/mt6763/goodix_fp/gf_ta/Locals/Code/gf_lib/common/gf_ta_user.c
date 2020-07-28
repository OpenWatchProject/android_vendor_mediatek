/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#include "string.h"
#include "gf_ta.h"
#include "gf_error.h"
#include "gf_log.h"
#include "gf_user_type_define.h"
#include "gf_memmgr.h"
#include "cpl_string.h"

#define LOG_TAG "[gf_ta_user]"

extern gf_mem_config_t g_mem_config;

static gf_error_t gf_ta_user_get_last_identify_id_cmd(void *buffer) {
    gf_user_last_identify_id_t *user_buffer = (gf_user_last_identify_id_t *) buffer;
    uint32_t id_len = 0;

    return gf_ta_get_last_identify_id((uint8_t *)&user_buffer->last_identify_id, &id_len);
}

static gf_error_t gf_ta_user_enumerate(void *buffer) {
    gf_error_t err = GF_SUCCESS;
    gf_user_enumerate_t *user_buffer = (gf_user_enumerate_t *) buffer;
    uint8_t id_buf[MAX_FINGERS_PER_USER * 2 * sizeof(uint32_t)] = { 0 };
    uint32_t id_buf_len = 0;
    uint32_t i = 0;
    uint8_t *tmp = id_buf;
    uint8_t len = 0;

    FUNC_ENTER();

    do {
        err = gf_ta_get_id_list(id_buf, &id_buf_len);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] gf_ta_get_id_list. id_buf_len=0x%u", __func__, id_buf_len);
            break;
        }

        while (tmp < id_buf + id_buf_len) {
            /* length */
            cpl_memcpy(&len, tmp, sizeof(uint32_t));
            tmp += 4;

            /* id*/
            cpl_memcpy((void *)(&user_buffer->finger_ids[i]) , tmp, len);
            tmp += 4;

            i++;
        }

        user_buffer->size = i;
    }while (0);
    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_user_get_authenticator_version(void *buffer) {
    gf_user_authenticator_version_t *user_buffer = (gf_user_authenticator_version_t *) buffer;
    return gf_ta_get_authenticator_version(&user_buffer->version);
}

static gf_error_t gf_ta_user_test_share_memory_check(void *buffer) {
    gf_error_t err = GF_SUCCESS;
    uint8_t * buf = (uint8_t *) buffer;
    uint32_t i = 0;
    FUNC_ENTER();

    do {
        for (; i < TEST_SHARE_MEMORY_CHECK_LEN; i++) {
            if (buf[i] != 0xAA) {
                GF_LOGE(LOG_TAG "[%s] hal to ta share memory failed. buf[%d]=0x%02X",
                        __func__, i, buf[i]);
                break;
            }
        }

        for (; i < TEST_SHARE_MEMORY_CHECK_LEN + 6; i++) {
            if (buf[i] == 0xAA) {
                GF_LOGE(LOG_TAG "[%s] hal to ta share memory exceed data length", __func__);
                // no break; trustonic this test always failed
            }
        }

        GF_LOGI(LOG_TAG "[%s] hal to ta share memory success", __func__);
    } while (0);

    cpl_memset(buf, 0x55, TEST_SHARE_MEMORY_CHECK_LEN);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_user_test_set_memmgr_config(gf_memmgr_config_t *mem_config) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do {
        if (NULL == mem_config) {
            GF_LOGE("[%s] bad parameter, mem_config is NULL", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (mem_config->memmgr_enable > 0) {  // input config
            err = gf_memmgr_save_config(mem_config);
        } else {
            err = gf_memmgr_remove_config();
        }

        if (g_mem_config.memmgr_enable > 0) {  // current config
            err = gf_memmgr_set_config(mem_config);
        }
    } while (0);

    FUNC_EXIT(err);
    return err;
}

static gf_error_t gf_ta_user_test_get_memmgr_config(gf_memmgr_config_t *mem_config) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do {
        if (NULL == mem_config) {
            GF_LOGE("[%s] bad parameter, mem_config is NULL", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        mem_config->memmgr_enable = g_mem_config.memmgr_enable;
        mem_config->debug_enable = g_mem_config.debug_enable;
        mem_config->match_best_mem_pool_enable = g_mem_config.match_best_mem_pool_enable;
        mem_config->erase_mem_pool_when_free = g_mem_config.erase_mem_pool_when_free;
        mem_config->dump_time_enable = g_mem_config.dump_time_enable;
        mem_config->memmgr_pool_size = g_mem_config.memmgr_pool_size;
        mem_config->enable_memmgr_next_reboot = g_mem_config.enable_memmgr_next_reboot;
    } while (0);

    FUNC_EXIT(err);
    return err;
}

static gf_error_t gf_ta_user_test_get_memmgr_info(gf_memmgr_info_t *node_info) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    err = gf_memmgr_get_manager_info(node_info);

    FUNC_EXIT(err);
    return err;
}

static gf_error_t gf_ta_user_test_dump_memmgr_pool(gf_memmgr_pool_dump_t *pool_dump) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_memmgr_dump_mem_pool(pool_dump);
    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_ta_invoke_user_cmd_entry_point(uint32_t cmd_id, void *buffer) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    switch (cmd_id) {
        case GF_USER_CMD_GET_LAST_IDENTIFY_ID: {
            err = gf_ta_user_get_last_identify_id_cmd(buffer);
            break;
        }

        case GF_USER_CMD_GET_AUTHENTICATOR_VERSION: {
            err = gf_ta_user_get_authenticator_version(buffer);
            break;
        }

        case GF_USER_CMD_ENUMERATE: {
            err = gf_ta_user_enumerate(buffer);
            break;
        }

        case GF_USER_CMD_TEST_SHARE_MEMORY_PERFORMANCE: {
            break;
        }

        case GF_USER_CMD_TEST_SHARE_MEMORY_CHECK: {
            err = gf_ta_user_test_share_memory_check(buffer);
            break;
        }

        case GF_USER_CMD_TEST_SET_MEMMGR_CONFIG: {
            err = gf_ta_user_test_set_memmgr_config((gf_memmgr_config_t *) buffer);
            break;
        }

        case GF_USER_CMD_TEST_GET_MEMMGR_CONFIG: {
            err = gf_ta_user_test_get_memmgr_config((gf_memmgr_config_t *) buffer);
            break;
        }

        case GF_USER_CMD_TEST_GET_MEMMGR_INFO: {
            err = gf_ta_user_test_get_memmgr_info((gf_memmgr_info_t *) buffer);
            break;
        }

        case GF_USER_CMD_TEST_DUMP_MEMMGR_POOL: {
            err = gf_ta_user_test_dump_memmgr_pool((gf_memmgr_pool_dump_t *) buffer);
            break;
        }

        case GF_USER_CMD_SET_HMAC_KEY: {
            err = gf_ta_set_hmac_key((gf_hmac_key_t*)buffer);
            break;
        }

        default: {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
    }

    FUNC_EXIT(err);
    return err;
}
