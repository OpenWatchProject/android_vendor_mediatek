/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#include "cpl_string.h"
#include "gf_algo.h"
#include "gf_bad_point_test.h"
#include "gf_common.h"
#include "gf_error.h"
#include "gf_log.h"
#include "gf_hw_common.h"
#include "gf_state_common.h"
#include "gf_tee_internal_api.h"
#include "gf_ta_test_chip_series.h"
#include "gf_milan.h"

#define LOG_TAG "[gf_ta_test_chip_series]"

extern uint8_t g_is_normal_cfg;
extern gf_base_data_t *g_base_data;

uint32_t g_image_count = 0;

static uint8_t g_sensor_engineer_available = 1;

void gf_ta_test_chip_series_init_global_variable(void) {
    VOID_FUNC_ENTER();

    g_image_count = 0;
    g_sensor_engineer_available = 1;

    VOID_FUNC_EXIT();
}

gf_error_t gf_ta_test_performance(gf_cmd_header_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == cmd) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE("[%s] bad parameter", __func__);
            break;
        }

        // if engineer mode otp crc check failed, finger factory test can't available
        if (0 == g_sensor_engineer_available) {
            GF_LOGD(LOG_TAG "[%s] engineer mode break", __func__);
            err = GF_ERROR_SENSOR_TEST_FAILED;
            break;
        }

        err = gf_state_execute_operation(OPERATION_TEST_PERFORMANCE, &cmd->reset_flag);
        gf_hw_data_sample_resume();
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_ta_test_spi(gf_test_spi_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == cmd) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE("[%s] bad parameter", __func__);
            break;
        }

        err = gf_hw_get_chip_id(cmd->chip_id, GF_CHIP_ID_LEN);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] get chip_id fail", __func__);
        }
    } while (0);

    gf_state_cancel_prior_test_operation(&cmd->cmd_header.reset_flag);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_ta_test_interrupt_pin(gf_cmd_header_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == cmd) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE("[%s] bad parameter", __func__);
            break;
        }

        err = gf_state_execute_operation(OPERATION_TEST_INTERRUPT_PIN, &cmd->reset_flag);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] exit, set mode fail", __func__);
            break;
        }
        err = gf_milan_host_irq_set();
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_ta_test_pre_get_version(gf_cmd_header_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == cmd) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE("[%s] bad parameter", __func__);
            break;
        }
        err = gf_state_execute_operation(OPERATION_TEST_IDLE_MODE, &cmd->reset_flag);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_ta_test_get_version(gf_test_get_version_t *cmd) {
    gf_error_t err = GF_SUCCESS;
    uint8_t chip_id[GF_CHIP_ID_LEN] = { 0 };
    uint8_t vendor_id[64] = { 0 };
    uint32_t irq_type = 0;

    FUNC_ENTER();

    do {
        if (NULL == cmd) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE("[%s] bad parameter", __func__);
            break;
        }

        gf_hw_get_irq_type(&irq_type);
        gf_hw_clear_irq(irq_type);

        err = gf_algo_test_get_algo_version(cmd->algo_version, cmd->preprocess_version);
        if (err != GF_SUCCESS) {
            break;
        }

        gf_get_tee_version(cmd->tee_version, TEE_VERSION_INFO_LEN);
        cpl_memcpy(cmd->ta_version, GF_TA_VERSION, cpl_strlen((const int8_t*) GF_TA_VERSION));

        err = gf_hw_get_chip_id((uint8_t *) chip_id, GF_CHIP_ID_LEN);
        if (err != GF_SUCCESS) {
            break;
        }
        GF_LOGD(LOG_TAG "[%s] chip_id=0x%02X, 0x%02X, 0x%02X, 0x%02X", __func__, chip_id[0],
                chip_id[1], chip_id[2], chip_id[3]);
        cpl_memcpy(cmd->chip_id, chip_id, GF_CHIP_ID_LEN);

        gf_hw_get_vendor_id(vendor_id, 64);
        GF_LOGD(LOG_TAG "[%s] vendor_id=0x%02X, 0x%02X, 0x%02X, 0x%02X", __func__, vendor_id[0],
                vendor_id[1], vendor_id[2], vendor_id[3]);
        cpl_memcpy(cmd->vendor_id, vendor_id, 4);

        GF_LOGD(LOG_TAG "[%s] production_date=0x%02X, 0x%02X, 0x%02X, 0x%02X", __func__,
                vendor_id[0x38], vendor_id[0x39], vendor_id[0x3a], vendor_id[0x3b]);
        cpl_memcpy(cmd->production_date, &(vendor_id[0x38]), 4);

        gf_state_cancel_test_operation(&cmd->cmd_header.reset_flag);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_ta_test_pixel_open_step1(gf_cmd_header_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == cmd) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE("[%s] bad parameter", __func__);
            break;
        }

        // if engineer mode otp crc check failed, finger factory test can't available
        if (0 == g_sensor_engineer_available) {
            GF_LOGD(LOG_TAG "[%s] engineer mode break", __func__);
            err = GF_ERROR_SENSOR_TEST_FAILED;
            break;
        }

        g_image_count = 0;

        err = gf_state_execute_operation(OPERATION_TEST_PIXEL_OPEN_STEP1, &cmd->reset_flag);
        if (err != GF_SUCCESS) {
            break;
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_ta_test_pixel_open_finish(gf_cmd_header_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == cmd) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE("[%s] bad parameter", __func__);
            break;
        }

        // if engineer mode otp crc check failed, finger factory test can't available
        if (0 == g_sensor_engineer_available) {
            GF_LOGD(LOG_TAG "[%s] engineer mode break", __func__);
            err = GF_ERROR_SENSOR_TEST_FAILED;
            break;
        }

        g_image_count = 0;

        g_is_normal_cfg = 1;

        gf_state_cancel_test_operation(&cmd->reset_flag);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_ta_test_bad_point(gf_cmd_header_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == cmd) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE("[%s] bad parameter", __func__);
            break;
        }

        // if engineer mode otp crc check failed, finger factory test can't available
        if (0 == g_sensor_engineer_available) {
            GF_LOGD(LOG_TAG "[%s] engineer mode break", __func__);
            err = GF_ERROR_SENSOR_TEST_FAILED;
            break;
        }

        err = gf_bad_point_test_init();
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] bad point init fail, err=%s, errno=%d",
                    __func__, gf_strerror(err), err);
            break;
        }

        if (0 == g_base_data->finger_base_rawdata_len) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGD(LOG_TAG "[%s] get finger base rawdata fail", __func__);
            break;
        }

        err = gf_bad_point_test_save_base_frame_data(g_base_data->finger_base_rawdata,
                g_base_data->finger_base_rawdata_len);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] save base data fail, err=%s, errno=%d",
                    __func__, gf_strerror(err), err);
            break;
        }

        err = gf_state_execute_operation(OPERATION_TEST_BAD_POINT, &cmd->reset_flag);
    } while (0);

    if (GF_SUCCESS != err) {
        gf_bad_point_test_destroy();
    }

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_test_sensor_validity(gf_test_sensor_validity_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == cmd) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE("[%s] bad parameter", __func__);
            break;
        }

        err = gf_state_execute_operation(OPERATION_TEST_SENSOR_VALIDITY,
                &cmd->cmd_header.reset_flag);

        if (0 == cmd->cmd_header.reset_flag) {
            err = gf_milan_test_sensor_validity();
            if (GF_SUCCESS == err) {
                cmd->is_passed = 1;
                g_sensor_engineer_available = 1;
            } else {
                cmd->is_passed = 0;
                g_sensor_engineer_available = 0;
            }

            gf_state_cancel_prior_test_operation(&cmd->cmd_header.reset_flag);
        } else {
            err = GF_ERROR_GENERIC;
            GF_LOGE(LOG_TAG "[%s] need reset to switch to idle mode", __func__);
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_ta_test_chip_series_invoke_cmd_entry_point(uint32_t cmd_id, void *buffer) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    if (NULL == buffer) {
        err = GF_ERROR_BAD_PARAMS;
        FUNC_EXIT(err);
        return err;
    }

    switch (cmd_id) {
        case GF_CMD_TEST_SENSOR_VALIDITY: {
            err = gf_ta_test_sensor_validity((gf_test_sensor_validity_t*) buffer);
            break;
        }

        default: {
            GF_LOGE(LOG_TAG "[%s] invalid parameters. cmd_id=%s, cmd_id_code=%u",
                    __func__, gf_strcmd((gf_cmd_id_t) cmd_id), cmd_id);
            err = GF_ERROR_UNKNOWN_CMD;
            break;
        }
    }

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_ta_test_chip_series_driver_cmd(gf_test_driver_cmd_t *cmd) {
    UNUSED_VAR(cmd);
    return GF_SUCCESS;
}
