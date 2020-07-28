/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#include "gf_algo.h"
#include "gf_common.h"
#include "gf_error.h"
#include "gf_log.h"
#include "gf_hw_common.h"
#include "gf_state_common.h"
#include "gf_ta_chip_series.h"
#include "gf_sensor.h"
#include "gf_tee_internal_api.h"
#include "gf_nav.h"
#include "gf_config.h"
#include "gf_oswego_m.h"

#define LOG_TAG "[gf_ta_chip_series]"

extern int32_t g_screen_on_flag;
extern uint8_t g_is_normal_cfg;
extern uint32_t g_test_esd_exception_count;

static uint32_t g_esd_exception_count = 0;

void gf_ta_chip_series_init_global_variable(void) {
    VOID_FUNC_ENTER();

    g_esd_exception_count = 0;

    VOID_FUNC_EXIT();
}

static gf_error_t gf_ta_download_fw(gf_cmd_header_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    err = gf_oswego_m_download_fw(&cmd->reset_flag);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_download_cfg(gf_cmd_header_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    UNUSED_VAR(cmd);

    FUNC_ENTER();

    err = gf_oswego_m_download_cfg(CONFIG_NORMAL);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_esd_check(gf_esd_check_t *cmd) {
    gf_error_t err = GF_SUCCESS;
    gf_mode_t cur_mode = MODE_NONE;
    uint8_t result = 0;

    FUNC_ENTER();
    cur_mode = gf_state_get_current_mode();
    do {
        if (0 == g_screen_on_flag || MODE_SLEEP == cur_mode || MODE_FF == cur_mode
                || MODE_DEBUG == cur_mode) {
            GF_LOGE(LOG_TAG "[%s] can't esd check screen_on=%d, mode=%s, mode_code=%d",
                    __func__, g_screen_on_flag, gf_strmode(cur_mode), cur_mode);
            break;
        }

        err = gf_oswego_m_esd_check(&result);
        if (err != GF_SUCCESS) {
            break;
        }

        if (g_test_esd_exception_count > 0) {
            result = 1;
            g_test_esd_exception_count--;
        }

        if (0 == result) {
            g_esd_exception_count = 0;

            cmd->download_fw_flag = 0;
            cmd->download_cfg_flag = 0;
            cmd->cmd_header.reset_flag = 0;
        } else {
            g_esd_exception_count++;

            if (g_esd_exception_count > 3) {
                cmd->download_fw_flag = 1;
                cmd->download_cfg_flag = 1;
            } else {
                cmd->download_fw_flag = 0;
                cmd->download_cfg_flag = 0;
            }

            cmd->cmd_header.reset_flag = 1;
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_nav_init(void) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        err = gf_nav_init(g_config.chip_type,
                g_sensor.nav_row, g_sensor.nav_col, g_sensor.nav_gap);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] navigation init failed, err=%s, errno=%d", __func__,
                    gf_strerror(err), err);
            break;
        }

        err = gf_nav_set_mode(g_config.support_nav_mode);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] navigation set mode failed, err=%s, errno=%d", __func__,
                    gf_strerror(err), err);
            break;
        }

        err = gf_nav_set_config(g_config.nav_config);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] navigation set config failed, err=%s, errno=%d", __func__,
                    gf_strerror(err), err);
            break;
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_init(gf_init_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    GF_LOGD(LOG_TAG "[%s] SOFT VERSION INFO PRINT", __func__);

    GF_LOGD(LOG_TAG "[%s] TARGET_MODE=%s", __func__, GF_TARGET_MODE);

    GF_LOGD(LOG_TAG "[%s] CHIP_TYPE=%s", __func__, gf_strchiptype(g_config.chip_type));

    GF_LOGD(LOG_TAG "[%s] PACKAGE_VERSION_CODE=%s", __func__, GF_PACKAGE_VERSION_CODE);

    GF_LOGD(LOG_TAG "[%s] PACKAGE_VERSION_NAME=%s", __func__, GF_PACKAGE_VERSION_NAME);

    GF_LOGD(LOG_TAG "[%s] GIT_BRANCH=%s", __func__, GF_GIT_BRANCH);

    GF_LOGD(LOG_TAG "[%s] COMMIT_ID=%s", __func__, GF_COMMIT_ID);

    GF_LOGD(LOG_TAG "[%s] BUILD_TIME=%s", __func__, GF_BUILD_TIME);

    do {
        err = gf_oswego_m_fw_cfg_init();

        err = gf_oswego_m_chip_init(&cmd->download_fw_flag, &cmd->download_cfg_flag);
        if (GF_ERROR_SPI_COMMUNICATION == err || GF_ERROR_SENSOR_NOT_AVAILABLE == err
                || GF_ERROR_GET_OTP_INFO_FAILED == err || GF_ERROR_OUT_OF_MEMORY == err) {
            break;
        }

        if (cmd->download_fw_flag > 0) {
            err = gf_oswego_m_download_fw(&cmd->cmd_header.reset_flag);
        }
        gf_hw_get_vendor_id(cmd->vendor_id, GF_VENDOR_ID_LEN);

        cmd->row = g_sensor.row;
        cmd->col = g_sensor.col;
        cmd->nav_row = g_sensor.nav_row;
        cmd->nav_col = g_sensor.nav_col;
        cmd->esd_check_flag = g_sensor.support_esd_check;

        // gf_ta_load_auth_token();

        err = gf_ta_nav_init();
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_navigate_complete(void) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    if (OPERATION_NAV == gf_state_get_current_operation()) {
        err = gf_hw_data_sample_suspend();
    }

    FUNC_EXIT(err);

    return err;
}

void gf_ta_chip_series_cancel_operation(gf_operation_type_t cur_operation) {
    VOID_FUNC_ENTER();

    switch (cur_operation) {
        case OPERATION_TEST_SENSOR_FINE_STEP1:
        case OPERATION_TEST_SENSOR_FINE_STEP2:
            gf_sensor_fine_test_cancel();
            break;

        case OPERATION_TEST_PIXEL_OPEN_STEP1:
        case OPERATION_TEST_PIXEL_OPEN_STEP2:
            gf_pixel_test_cancel();

            if (0 == g_is_normal_cfg) {
                gf_state_execute_operation(OPERATION_TEST_KEY_MODE, NULL);
                gf_oswego_m_download_cfg(CONFIG_NORMAL);
            }
            g_is_normal_cfg = 1;

            break;

        default:
            break;
    }

    VOID_FUNC_EXIT();
}

gf_error_t gf_ta_chip_series_invoke_cmd_entry_point(uint32_t cmd_id, void *buffer) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    if (NULL == buffer) {
        err = GF_ERROR_BAD_PARAMS;
        FUNC_EXIT(err);
        return err;
    }

    switch (cmd_id) {
        case GF_CMD_DOWNLOAD_FW: {
            err = gf_ta_download_fw((gf_cmd_header_t *) buffer);
            break;
        }

        case GF_CMD_DOWNLOAD_CFG: {
            err = gf_ta_download_cfg((gf_cmd_header_t *) buffer);
            break;
        }

        case GF_CMD_ESD_CHECK: {
            err = gf_ta_esd_check((gf_esd_check_t*) buffer);
            break;
        }

        case GF_CMD_INIT: {
            err = gf_ta_init((gf_init_t*) buffer);
            gf_sync_authenticator_version(GF_AUTHENTICATOR_VERSION_CODE);
            break;
        }

        case GF_CMD_NAVIGATE_COMPLETE: {
            err = gf_ta_navigate_complete();
            break;
        }

        default: {
            GF_LOGE(LOG_TAG "[%s] invalid parameters cmd_id=%s, cmd_id_code=%u",
                    __func__, gf_strcmd((gf_cmd_id_t) cmd_id), cmd_id);
            err = GF_ERROR_UNKNOWN_CMD;
            break;
        }
    }

    FUNC_EXIT(err);

    return err;
}

