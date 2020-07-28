/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#include "cpl_string.h"
#include "gf_algo.h"
#include "gf_base.h"
#include "gf_common.h"
#include "gf_config.h"
#include "gf_sensor.h"
#include "gf_error.h"
#include "gf_log.h"
#include "gf_nav.h"
#include "gf_hw_common.h"
#include "gf_state_common.h"
#include "gf_tee_internal_api.h"
#include "gf_ta_chip_series.h"
#include "gf_secure_object.h"
#include "cpl_memory.h"
#include "gf_hw.h"

#define LOG_TAG "[gf_ta_chip_series]"

extern uint8_t g_is_normal_cfg;

extern gf_nav_data_t *g_nav_data;
extern gf_nav_origin_data_t *g_nav_origin_data;

void gf_ta_chip_series_init_global_variable(void) {
    // do nothing
}

static gf_error_t gf_ta_load_otp_info(uint8_t *otp_buf, uint32_t *otp_buf_len) {
    gf_error_t err = GF_SUCCESS;
    uint8_t *buf = NULL;
    uint32_t buf_len = 0;
    FUNC_ENTER();

    do {
        err = gf_so_load_persistent_object((int8_t*) GF_OTP_INFO_SECURE_ID, &buf, &buf_len);
        if (err != GF_SUCCESS) {
            GF_LOGI(LOG_TAG "[%s] load so failed, so=%s", __func__, GF_OTP_INFO_SECURE_ID);
            break;
        }

        cpl_memcpy(otp_buf, (void*) buf, buf_len);
        *otp_buf_len = buf_len;
    } while (0);

    CPL_MEM_FREE(buf);

    FUNC_EXIT(err);
    return err;
}

static gf_error_t gf_ta_save_otp_info(uint8_t *buf, uint32_t buf_len) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    err = gf_so_save_persistent_object((int8_t*) GF_OTP_INFO_SECURE_ID, buf, buf_len);
    if (err != GF_SUCCESS) {
        GF_LOGI(LOG_TAG "[%s] save so=%s failed", __func__, GF_OTP_INFO_SECURE_ID);
    }

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
        uint8_t tee_otp_buf[GF_SENSOR_OTP_INFO_LEN] = { 0 };
        uint8_t tee_otp_buf_tmp[GF_SENSOR_OTP_INFO_LEN] = { 0 };
        uint32_t tee_otp_buf_len = 0;

        err = gf_ta_load_otp_info(tee_otp_buf, &tee_otp_buf_len);
        if (GF_SUCCESS == err) {
            cpl_memcpy(tee_otp_buf_tmp, tee_otp_buf, tee_otp_buf_len);
        }

        err = gf_hw_chip_init(cmd->otp_info, &(cmd->otp_info_len), tee_otp_buf, &tee_otp_buf_len);
        if (GF_ERROR_SPI_COMMUNICATION == err || GF_ERROR_SENSOR_NOT_AVAILABLE == err
                || GF_ERROR_GET_OTP_INFO_FAILED == err || GF_ERROR_OUT_OF_MEMORY == err) {
            break;
        }

        if (cpl_memcmp(tee_otp_buf, tee_otp_buf_tmp, GF_SENSOR_OTP_INFO_LEN) != 0) {
            gf_so_delete_persistent_object((int8_t*) GF_OTP_INFO_SECURE_ID);
            gf_ta_save_otp_info(tee_otp_buf, GF_SENSOR_OTP_INFO_LEN);
            GF_LOGI(LOG_TAG "[%s] update new otp data to TEE storage", __func__);
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

static gf_error_t gf_ta_detect_nav_event(gf_detect_nav_event_t *cmd) {
    gf_error_t err = GF_SUCCESS;
    gf_nav_code_t nav_code = GF_NAV_NONE;
    uint8_t frame_num = 0;
    uint8_t finger_up_flag = 0;
    uint32_t i = 0;

    FUNC_ENTER();

    do {
        if (g_config.nav_config.maxNvgFrameNum > NAV_MAX_FRAMES) {
            g_config.nav_config.maxNvgFrameNum = NAV_MAX_FRAMES;
        }

        cpl_memset(g_nav_data, 0, sizeof(gf_nav_data_t));
        err = gf_hw_get_nav_image(g_nav_origin_data->data[0], &g_nav_origin_data->data_len,
                                          g_nav_data->raw_data[0], &g_nav_data->raw_data_len,
                                          &frame_num, &finger_up_flag);

        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] get nav image fail, err=%d", __func__, err);
            break;
        }

        if (0 == frame_num) {
            GF_LOGE(LOG_TAG "[%s] get nav image fail, frame num=%d",
                    __func__, g_nav_data->frame_num[0]);
            break;
        }

        if (frame_num < g_config.nav_config.maxNvgFrameNum) {
            g_nav_data->finger_up_flag[frame_num - 1] = 1;
        } else {
            g_nav_data->finger_up_flag[frame_num - 1] = finger_up_flag;
        }

        g_nav_data->nav_frame_count = frame_num;
        for (i = 0; i < frame_num; i++) {
            g_nav_data->frame_num[i] = 1;
            err = gf_nav_detect(g_nav_data->raw_data[i], g_nav_data->raw_data_len,
                    g_nav_data->finger_up_flag[i], 1, &nav_code);

            if (GF_SUCCESS != err) {
                GF_LOGE(LOG_TAG "[%s] nav detect fail, err=%d", __func__, err);
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
        err = gf_hw_nav_complete();
    } else {
        if (0 < g_config.require_down_and_up_in_pairs_for_nav_mode) {
            gf_state_require_down_and_up_in_pairs_end(&(cmd->cmd_header.reset_flag));
        }
    }

    GF_LOGI(LOG_TAG "[%s] nav_code = %d, frame_num=%d, finger_up_flag=%d",
            __func__, nav_code, frame_num, finger_up_flag);

    FUNC_EXIT(err);

    return err;
}

void gf_ta_chip_series_cancel_operation(gf_operation_type_t cur_operation) {
    VOID_FUNC_ENTER();

    switch (cur_operation) {
        case OPERATION_TEST_PIXEL_OPEN_STEP1:
        case OPERATION_TEST_PIXEL_OPEN_STEP2:
            gf_pixel_test_cancel();

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
        case GF_CMD_INIT: {
            err = gf_ta_init((gf_init_t*) buffer);
            gf_sync_authenticator_version(GF_AUTHENTICATOR_VERSION_CODE);
            break;
        }

        case GF_CMD_DETECT_NAV_EVENT: {
            err = gf_ta_detect_nav_event((gf_detect_nav_event_t *) buffer);
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

