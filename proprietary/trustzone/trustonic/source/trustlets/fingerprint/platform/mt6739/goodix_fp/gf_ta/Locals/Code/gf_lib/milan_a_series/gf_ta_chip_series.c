/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#include "cpl_string.h"
#include "gf_algo.h"
#include "gf_common.h"
#include "gf_config.h"
#include "gf_error.h"
#include "gf_heart_beat.h"
#include "gf_log.h"
#include "gf_nav.h"
#include "gf_hw_common.h"
#include "gf_state_common.h"
#include "gf_ta_chip_series.h"
#include "gf_sensor.h"
#include "gf_tee_internal_api.h"
#include "gf_milan_a_series_common.h"
#include "gf_milan_a_series.h"

#define LOG_TAG "[gf_ta_chip_series]"

extern int32_t g_screen_on_flag;
extern uint8_t g_is_normal_cfg;
extern uint32_t g_test_esd_exception_count;
extern gf_nav_data_t *g_nav_data;
extern gf_nav_origin_data_t *g_nav_origin_data;
static uint32_t g_esd_exception_count = 0;

gf_milan_a_series_fw_cfg_t g_milan_a_series_fw_cfg;

void gf_ta_chip_series_init_global_variable(void) {
}

static gf_error_t gf_ta_download_fw(gf_cmd_header_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    err = gf_milan_a_series_download_fw(&cmd->reset_flag);

    if (GF_SUCCESS == err) {
        gf_mode_t cur_mode;
        cur_mode = gf_state_get_current_mode();
        if (MODE_NONE != cur_mode) {
            gf_hw_set_mode(cur_mode);
        }
    }

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_download_cfg(gf_cmd_header_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    UNUSED_VAR(cmd);

    FUNC_ENTER();

    err = gf_milan_a_series_download_cfg(CONFIG_NORMAL);

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
            GF_LOGE(
                    LOG_TAG "[%s] can't esd check screen_on=%d, mode=%s, mode_code=%d",
                    __func__, g_screen_on_flag, gf_strmode(cur_mode), cur_mode);
            break;
        }

        err = gf_milan_a_series_esd_check(&result);
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
                cmd->cmd_header.reset_flag = 1;
            } else {
                cmd->download_fw_flag = 0;
                cmd->download_cfg_flag = 0;
            }
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_start_hbd(gf_cmd_header_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    UNUSED_VAR(cmd);

    FUNC_ENTER();

    do {
        if (g_config.support_hbd <= 0) {
            GF_LOGE(LOG_TAG "[%s] chip do not support hbd mode!", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
        gf_heart_beat_init();
        err = gf_state_execute_operation(OPERATION_HEARTBEAT_KEY, &cmd->reset_flag);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_detect_nav_event(gf_detect_nav_event_t *cmd) {
    gf_error_t err = GF_SUCCESS;
    gf_nav_code_t nav_code = GF_NAV_NONE;
    int32_t total_frame_num = 0;
    uint8_t frame_num = 0;
    uint32_t i = 0;
    uint8_t finger_up_flag = 0;

    FUNC_ENTER();

    cpl_memset(g_nav_data, 0, sizeof(gf_nav_data_t));
    do {
        if (g_config.nav_config.maxNvgFrameNum > NAV_MAX_FRAMES) {
            g_config.nav_config.maxNvgFrameNum = NAV_MAX_FRAMES;
        }
        for (; total_frame_num < g_config.nav_config.maxNvgFrameNum;) {
            err = gf_milan_a_series_get_nav_image(g_nav_origin_data->data[0],
                    &g_nav_origin_data->data_len);
            if (GF_SUCCESS != err) {
                break;
            }

            err = gf_milan_a_series_post_get_nav_image(g_nav_data->raw_data[total_frame_num],
                    &g_nav_data->raw_data_len, &frame_num, &finger_up_flag);
            if (GF_SUCCESS != err) {
                break;
            }

            total_frame_num += frame_num;

            if (finger_up_flag > 0) {
                break;
            }
        }

        if (total_frame_num > 0) {
            if (total_frame_num < g_config.nav_config.maxNvgFrameNum) {
                g_nav_data->finger_up_flag[total_frame_num - 1] = 2;
            } else {
                g_nav_data->finger_up_flag[total_frame_num - 1] = finger_up_flag;
            }
        }

        g_nav_data->nav_frame_count = total_frame_num;
        for (i = 0; i < g_nav_data->nav_frame_count; i++) {
            g_nav_data->frame_num[i] = 1;
            err = gf_nav_detect(g_nav_data->raw_data[i], g_nav_data->raw_data_len,
                    g_nav_data->finger_up_flag[i], 1, &nav_code);

            if (GF_SUCCESS != err) {
                break;
            }
            if (nav_code != GF_NAV_NONE) {
                break;
            }
        }
    } while (0);

    cmd->nav_result.finger_up_flag = finger_up_flag;
    cmd->nav_result.nav_code = nav_code;

    if (0 == finger_up_flag) {
        gf_hw_data_sample_suspend();
    } else {
        if (g_config.require_down_and_up_in_pairs_for_nav_mode > 0) {
            gf_state_require_down_and_up_in_pairs_end(&(cmd->cmd_header.reset_flag));
        }
    }

    GF_LOGI(LOG_TAG "[%s] nav_code=%d, total_frame_num=%d, finger_up_flag=%u",
            __func__, nav_code, total_frame_num, finger_up_flag);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_nav_init(void) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        err = gf_nav_init(g_config.chip_type, g_sensor.nav_row, g_sensor.nav_col, g_sensor.nav_gap);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] navigation init failed, err=%s, errno=%d",
                    __func__, gf_strerror(err), err);
            break;
        }

        err = gf_nav_set_mode(g_config.support_nav_mode);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] navigation set mode failed, err=%s, errno=%d",
                    __func__, gf_strerror(err), err);
            break;
        }

        err = gf_nav_set_config(g_config.nav_config);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] navigation set config failed, err=%s, errno=%d",
                    __func__, gf_strerror(err), err);
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
        err = gf_milan_a_series_chip_init(&cmd->download_fw_flag, &cmd->download_cfg_flag);
        if (GF_ERROR_SPI_COMMUNICATION == err || GF_ERROR_SENSOR_NOT_AVAILABLE == err
                || GF_ERROR_GET_OTP_INFO_FAILED == err || GF_ERROR_OUT_OF_MEMORY == err) {
            break;
        }

        if (cmd->download_fw_flag > 0) {
            err = gf_milan_a_series_download_fw(&cmd->cmd_header.reset_flag);
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
        case OPERATION_TEST_PIXEL_OPEN_STEP1:
        case OPERATION_TEST_PIXEL_OPEN_STEP2:
            gf_pixel_test_cancel();

            /* no break here, directly go down to <case OPERATION_TEST_HBD_CALIBRATION> */

        case OPERATION_TEST_HBD_CALIBRATION:
            if (0 == g_is_normal_cfg) {
                gf_milan_a_series_download_cfg(CONFIG_NORMAL);
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

        case GF_CMD_DETECT_NAV_EVENT: {
            err = gf_ta_detect_nav_event((gf_detect_nav_event_t *) buffer);
            break;
        }

        case GF_CMD_ESD_CHECK: {
            err = gf_ta_esd_check((gf_esd_check_t*) buffer);
            break;
        }

        case GF_CMD_START_HBD: {
            err = gf_ta_start_hbd((gf_cmd_header_t *) buffer);
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

