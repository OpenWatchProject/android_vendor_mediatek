/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#include "cpl_memory.h"
#include "cpl_string.h"
#include "gf_algo.h"
#include "gf_broken.h"
#include "gf_common.h"
#include "gf_config.h"
#include "gf_error.h"
#include "gf_sensor.h"
#include "gf_hw_common.h"
#include "gf_state_common.h"
#include "gf_ta_test.h"

#include "gf_ta_test_chip_series.h"
#include "gf_tee_internal_api.h"

#define LOG_TAG "[gf_ta_test]"

#define MILAN_GET_RESET_IRQ_MAX_COUNT (20)

uint32_t g_ignore_irq_mask = 0;
uint32_t g_max_frame_num = 0;

extern uint8_t g_last_authenticate_result;

static gf_error_t gf_ta_test_cancel(gf_cancel_t *cmd);

void gf_ta_test_init_global_variable(void) {
    VOID_FUNC_ENTER();

    g_ignore_irq_mask = 0;
    g_max_frame_num = 0;

    VOID_FUNC_EXIT();
}

static gf_error_t gf_ta_test_untrusted_enroll(gf_cmd_header_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == cmd) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE("[%s] bad parameter", __func__);
            break;
        }

        err = gf_algo_test_pre_enroll();
        if (GF_SUCCESS != err) {
            break;
        }

        err = gf_state_execute_operation(OPERATION_TEST_UNTRUSTED_ENROLL, &cmd->reset_flag);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_test_untrusted_authenticate(gf_authenticate_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    g_last_authenticate_result = 0;

    do {
        if (NULL == cmd) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE("[%s] bad parameter", __func__);
            break;
        }

        GF_LOGD(LOG_TAG "[%s] group_id=%u", __func__, cmd->group_id);

        err = gf_state_execute_operation(OPERATION_TEST_UNTRUSTED_AUTHENTICATE,
                &cmd->cmd_header.reset_flag);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_test_untrusted_remove(void) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    gf_algo_test_remove(0, 0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_test_spi_performance(gf_cmd_header_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == cmd) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE("[%s] bad parameter", __func__);
            break;
        }

        err = gf_state_execute_operation(OPERATION_TEST_SPI_PERFORMANCE, &cmd->reset_flag);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_test_spi_transfer(gf_cmd_header_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == cmd) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE("[%s] bad parameter", __func__);
            break;
        }

        err = gf_state_execute_operation(OPERATION_TEST_SPI_TRANSFER, &cmd->reset_flag);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_test_pre_spi(gf_cmd_header_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == cmd) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE("[%s] bad parameter", __func__);
            break;
        }

        err = gf_state_execute_operation(OPERATION_TEST_PRE_SPI, &cmd->reset_flag);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_test_spi_rw(gf_test_spi_rw_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (cmd == NULL || cmd->rw_len == 0) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad parameter", __func__);
            break;
        }

        if (cmd->spi_cmd == TEST_SPI_READ) {
            err = gf_hw_read_register(cmd->start_addr, cmd->rw_content, cmd->rw_len);
        } else {
            err = gf_hw_write_register(cmd->start_addr, cmd->rw_content, cmd->rw_len);
        }

        // notify error
        cmd->cmd_header.result = err;

        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] test spi read/write failed", __func__);
            break;
        }
    } while (0);

    gf_state_cancel_prior_test_operation(&cmd->cmd_header.reset_flag);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_test_reset_pin1(gf_cmd_header_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == cmd) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE("[%s] bad parameter", __func__);
            break;
        }

        err = gf_state_execute_operation(OPERATION_TEST_RESET_PIN, &cmd->reset_flag);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_test_reset_pin2(gf_cmd_header_t *cmd) {
    gf_error_t err = GF_ERROR_TEST_RESET_PIN;
    uint32_t irq_type = 0;
    uint16_t count = 0;

    FUNC_ENTER();

    do {
        if (NULL == cmd) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE("[%s] bad parameter", __func__);
            break;
        }

        for (count = 0; count < MILAN_GET_RESET_IRQ_MAX_COUNT; count++) {
            gf_hw_get_irq_type(&irq_type);

            GF_LOGD(LOG_TAG "[%s] irq_type=0x%x", __func__, irq_type);

            if (irq_type & (GF_IRQ_RESET_MASK)) {
                err = GF_SUCCESS;
                break;
            }

            gf_sleep(5);
        }

        gf_hw_clear_irq(irq_type);
        gf_state_cancel_test_operation(&cmd->reset_flag);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_test_frr_far_init(gf_test_frr_far_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == cmd) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE("[%s] bad parameter", __func__);
            break;
        }

        err = gf_frr_far_init(cmd->safe_class, cmd->template_count, cmd->support_bio_assay);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_test_frr_far_record_base_frame(gf_cmd_header_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == cmd) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE("[%s] bad parameter", __func__);
            break;
        }

        err = gf_state_execute_operation(OPERATION_TEST_FRR_FAR_RECORD_BASE_FRAME,
                &cmd->reset_flag);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_test_frr_far_record_enroll(gf_cmd_header_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == cmd) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE("[%s] bad parameter", __func__);
            break;
        }

        err = gf_state_execute_operation(OPERATION_TEST_FRR_FAR_RECORD_ENROLL, &cmd->reset_flag);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_test_frr_far_record_authenticate(gf_cmd_header_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == cmd) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE("[%s] bad parameter", __func__);
            break;
        }

        err = gf_state_execute_operation(OPERATION_TEST_FRR_FAR_RECORD_AUTHENTICATE,
                &cmd->reset_flag);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_test_frr_far_record_authenticate_finish(gf_cmd_header_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == cmd) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE("[%s] bad parameter", __func__);
            break;
        }

        err = gf_state_cancel_test_operation(&cmd->reset_flag);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_test_frr_far_play_base_frame(gf_test_frr_far_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == cmd) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE("[%s] bad parameter", __func__);
            break;
        }

        err = gf_frr_far_calibration(cmd->raw_data, g_sensor.raw_data_len);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_test_frr_far_play_enroll(gf_test_frr_far_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == cmd) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE("[%s] bad parameter", __func__);
            break;
        }

        cmd->check_flag = 1;
        err = gf_frr_far_play_enroll(cmd);

        if (cmd->data_type == (uint32_t)TEST_TOKEN_RAW_DATA) {
            gf_frr_far_dump_info(cmd);
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_test_frr_far_play_authenticate(gf_test_frr_far_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == cmd) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE("[%s] bad parameter", __func__);
            break;
        }

        err = gf_frr_far_authenticate(cmd->raw_data, g_sensor.raw_data_len, cmd);

        if (cmd->data_type == (uint32_t)TEST_TOKEN_RAW_DATA) {
            gf_frr_far_dump_info(cmd);
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_test_frr_far_enroll_finish(gf_cmd_header_t *cmd) {
    gf_error_t err = GF_SUCCESS;
    gf_operation_type_t cur_operation = OPERATION_NONE;

    FUNC_ENTER();

    do {
        if (NULL == cmd) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE("[%s] bad parameter", __func__);
            break;
        }

        cur_operation = gf_state_get_current_operation();

        if (cur_operation == OPERATION_TEST_FRR_FAR_RECORD_ENROLL) {
            gf_state_execute_operation(OPERATION_TEST_IMAGE_MODE, &cmd->reset_flag);
        }

        gf_frr_far_delete_tran();
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_test_frr_far_cancel(gf_cancel_t *cmd) {
    gf_error_t err = GF_SUCCESS;
    gf_operation_type_t cur_operation = OPERATION_NONE;

    FUNC_ENTER();

    do {
        if (NULL == cmd) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE("[%s] bad parameter", __func__);
            break;
        }

        cur_operation = gf_state_get_current_operation();

        if (cur_operation == OPERATION_TEST_FRR_FAR_RECORD_BASE_FRAME
                || cur_operation == OPERATION_TEST_FRR_FAR_RECORD_ENROLL
                || cur_operation == OPERATION_TEST_FRR_FAR_RECORD_AUTHENTICATE) {
            err = gf_ta_test_cancel(cmd);
        } else {
            gf_frr_far_cancel();
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_test_set_config(gf_test_set_config_t *cmd) {
    gf_error_t err = GF_SUCCESS;
    gf_algo_basic_config_t basic_config = { 0 };

    FUNC_ENTER();

    if (NULL == cmd) {
        err = GF_ERROR_BAD_PARAMS;
        GF_LOGE("[%s] bad parameter", __func__);
        FUNC_EXIT(err);
        return err;
    }

    cpl_memcpy(&g_config, &cmd->config, sizeof(gf_config_t));

    basic_config.max_fingers = g_config.max_fingers;
    basic_config.max_fingers_per_user = g_config.max_fingers_per_user;

    basic_config.support_bio_assay = g_config.support_bio_assay;

    basic_config.enrolling_min_templates = g_config.enrolling_min_templates;

    basic_config.valid_image_quality_threshold =
            g_config.valid_image_quality_threshold;
    basic_config.valid_image_area_threshold =
            g_config.valid_image_area_threshold;
    basic_config.duplicate_finger_overlay_score =
            g_config.duplicate_finger_overlay_score;
    basic_config.increase_rate_between_stitch_info =
            g_config.increase_rate_between_stitch_info;

    basic_config.forbidden_enroll_duplicate_fingers =
            g_config.forbidden_enroll_duplicate_fingers;

    basic_config.authenticate_order = g_config.authenticate_order;

    /*check template_update_save_threshold can not be less than GF_MIN_TEMPLATE_UPDATE_COUNT_SAVE_THRESHOLD
     * and can not be greater than GF_MAX_TEMPLATE_UPDATE_COUNT_SAVE_THRESHOLD
     */
    if (GF_MIN_TEMPLATE_UPDATE_COUNT_SAVE_THRESHOLD > g_config.template_update_save_threshold) {
        g_config.template_update_save_threshold = GF_MIN_TEMPLATE_UPDATE_COUNT_SAVE_THRESHOLD;
    } else if (GF_MAX_TEMPLATE_UPDATE_COUNT_SAVE_THRESHOLD
            < g_config.template_update_save_threshold) {
        g_config.template_update_save_threshold = GF_MAX_TEMPLATE_UPDATE_COUNT_SAVE_THRESHOLD;
    }
    basic_config.template_update_save_threshold = g_config.template_update_save_threshold;

    gf_algo_update_config(&basic_config);

    cpl_memcpy(&cmd->config, &g_config, sizeof(gf_config_t));

    switch (cmd->token) {
        case TEST_TOKEN_SUPPORT_KEY_MODE:
        case TEST_TOKEN_SUPPORT_NAV_MODE:
            gf_state_cancel_test_operation(&cmd->cmd_header.reset_flag);
            break;

        case TEST_TOKEN_SUPPORT_SENSOR_BROKEN_CHECK: {
            if (g_config.support_sensor_broken_check > 0) {
                err = gf_broken_check_init();
                if (err != GF_SUCCESS) {
                    break;
                }
            } else {
                err = gf_broken_check_destroy();
                if (err != GF_SUCCESS) {
                    break;
                }
            }
            break;
        }

        default:
            break;
    }

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_test_real_time_data(gf_cmd_header_t *cmd) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_state_execute_operation(OPERATION_TEST_REAL_TIME_DATA, &cmd->reset_flag);

    gf_hw_data_sample_resume();

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_test_bmp_data(gf_cmd_header_t *cmd) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_state_execute_operation(OPERATION_TEST_BMP_DATA, &cmd->reset_flag);
    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_test_data_noise(gf_test_data_noise_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == cmd) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE("[%s] bad parameter", __func__);
            break;
        }

        g_max_frame_num = cmd->max_frame_num;
        GF_LOGD(LOG_TAG "[%s] g_max_frame_num=%u" , __func__, g_max_frame_num);

        err = gf_state_execute_operation(OPERATION_TEST_DATA_NOISE, &(cmd->cmd_header.reset_flag));
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_test_pixel_open(gf_cmd_header_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == cmd) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE("[%s] bad parameter", __func__);
            break;
        }

        err = gf_state_execute_operation(OPERATION_TEST_KEY_MODE, &cmd->reset_flag);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

/*
 * driver test command
 * version xx
 * mode xx (value == 0xff means get chip mode)
 * debug xx(debug case)
 * timer xx(print timestamp)
 */
static gf_error_t gf_ta_test_driver_cmd(gf_test_driver_cmd_t *cmd) {
    gf_error_t err = GF_SUCCESS;
    uint64_t cur_timestamp = 0;
    uint32_t i = 0;

    FUNC_ENTER();

    if (NULL == cmd) {
        err = GF_ERROR_BAD_PARAMS;
        GF_LOGE("[%s] bad parameter", __func__);
        FUNC_EXIT(err);
        return err;
    }

    if (0 == cpl_strncmp((const int8_t*) cmd->cmd_buf, (const int8_t*) "get_mode",
            cpl_strlen((const int8_t*) "get_mode"))) {
        gf_mode_t mode = MODE_NONE;
        err = gf_hw_get_mode(&mode);
        cmd->mode = mode;
    } else if (0 == cpl_strncmp((const int8_t*) cmd->cmd_buf, (const int8_t*) "set_mode",
            cpl_strlen((const int8_t*) "set_mode"))) {
        gf_operation_type_t operation = OPERATION_NONE;
        switch (cmd->mode) {
            case MODE_IMAGE:
                operation = OPERATION_TEST_IMAGE_MODE;
                break;
            case MODE_FF:
                operation = OPERATION_TEST_FF_MODE;
                break;
            case MODE_SLEEP:
                operation = OPERATION_TEST_SLEEP_MODE;
                break;
            case MODE_KEY:
                operation = OPERATION_TEST_KEY_MODE;
                break;
            case MODE_NAV:
                operation = OPERATION_TEST_NAV_MODE;
                break;
            case MODE_FINGER_BASE:
                operation = OPERATION_TEST_FINGER_BASE_MODE;
                break;
            case MODE_HBD:
                operation = OPERATION_TEST_HBD_MODE;
                break;
            case MODE_DEBUG:
                operation = OPERATION_TEST_DEBUG_MODE;
                break;
            case MODE_HBD_DEBUG:
                operation = OPERATION_TEST_HBD_DEBUG_MODE;
                break;
            case MODE_IDLE:
                operation = OPERATION_TEST_IDLE_MODE;
                break;
            default:
                operation = OPERATION_NONE;
                break;
        }
        gf_state_execute_operation(operation, NULL);
    } else if (0 == cpl_strncmp((const int8_t*) cmd->cmd_buf,
            (const int8_t*) "timer", cpl_strlen((const int8_t*) "timer"))) {
        GF_LOGI(LOG_TAG "[%s] recevied command time", __func__);

        gf_get_timestamp(&cur_timestamp);
        GF_LOGI(LOG_TAG "[%s] get timestamp 0x%08X%08X",
                __func__, (uint32_t)((cur_timestamp >> 32) & 0xFFFFFFFF),
                (uint32_t)(cur_timestamp & 0xFFFFFFFF));

        gf_sleep(10);

        gf_get_timestamp(&cur_timestamp);
        GF_LOGI(LOG_TAG "[%s] get timestamp 0x%08X%08X",
                __func__, (uint32_t)((cur_timestamp >> 32) & 0xFFFFFFFF),
                (uint32_t)(cur_timestamp & 0xFFFFFFFF));
    } else if (0 == cpl_strncmp((const int8_t*) cmd->cmd_buf,
            (const int8_t*) "clear_storage", cpl_strlen((const int8_t*) "clear_storage"))) {
        // TODO(goodix)
    } else if (0 == cpl_strncmp((const int8_t*) cmd->cmd_buf,
            (const int8_t*) "secure_share_memory_performance",
            cpl_strlen((const int8_t*) "secure_share_memory_performance"))) {
        uint8_t *share_buf = CPL_MEM_MALLOC(102400);
        if (NULL == share_buf) {
            err = GF_ERROR_OUT_OF_MEMORY;
            GF_LOGE(LOG_TAG "[%s] malloc failed err=%s, errno=%d", __func__, gf_strerror(err), err);
        } else {
            uint32_t size_buf[] = { 0, 1, 8, 32, 1024, 10240, 51200, 102400 };

            uint32_t count = sizeof(size_buf) / sizeof(uint32_t);

            for (i = 0; i < count; i++) {
                cmd->secure_share_memory_size[i] = size_buf[i];
                gf_secure_share_memory_performance(share_buf, cmd->secure_share_memory_size[i],
                        &cmd->secure_share_memory_time[i]);
            }

            cmd->secure_share_memory_count = count;

            CPL_MEM_FREE(share_buf);
        }

    } else if (0 == cpl_strncmp((const int8_t*) cmd->cmd_buf, (const int8_t*) "ta_crash",
            cpl_strlen((const int8_t*) "ta_crash"))) {
        uint32_t *addr = NULL;
        GF_LOGD(LOG_TAG "[%s] ta_crash", __func__);
        // cppcheck-suppress nullPointer
        *addr = 0x1234;
    } else if (0 == cpl_strncmp((const int8_t*) cmd->cmd_buf, (const int8_t*) "write",
            cpl_strlen((const int8_t*) "write"))) {
        GF_LOGD(LOG_TAG "[%s] spi write address=0x%04X, val=0x%02X",
                __func__, cmd->address, cmd->value);
        err = gf_hw_write_register(cmd->address, &(cmd->value), 1);
    } else if (0 == cpl_strncmp((const int8_t*) cmd->cmd_buf, (const int8_t*) "read",
            cpl_strlen((const int8_t*) "read"))) {
        GF_LOGD(LOG_TAG "[%s] spi read address=0x%04X", __func__, cmd->address);
        err = gf_hw_read_register(cmd->address, &(cmd->value), 1);
    } else if (0 == cpl_strncmp((const int8_t*) cmd->cmd_buf, (const int8_t*) "ignore",
            cpl_strlen((const int8_t*) "ignore"))) {
        GF_LOGD(LOG_TAG "[%s] ignore irq type=0x%X", __func__, cmd->ignore_irq_type);

        if (cmd->ignore_irq_type <= 0) {
            g_ignore_irq_mask = 0;
        } else {
            g_ignore_irq_mask = 1 << (cmd->ignore_irq_type);
        }
    } else {
        err = gf_ta_test_chip_series_driver_cmd(cmd);
    }

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_test_cancel(gf_cancel_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == cmd) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE("[%s] bad parameter", __func__);
            break;
        }

        cmd->operation = gf_state_get_current_operation();
        /*
         * there is a trap:
         * when the function is called, #gf_ta_cancel_operation release some resource;
         * but #gf_ta_pre_set_mode maybe can not change the mode immediately,such as cancel but sensor is pressed, it will be set mode next;
         * in this condition, attention the released resource is NULL;
         */
        err = gf_state_cancel_test_operation(&cmd->cmd_header.reset_flag);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_test_prior_cancel(gf_cancel_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == cmd) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE("[%s] bad parameter", __func__);
            break;
        }

        cmd->operation = gf_state_get_current_operation();
        /*
         * there is a trap:
         * when the function is called, #gf_ta_cancel_operation release some resource;
         * but #gf_ta_pre_set_mode maybe can not change the mode immediately,such as cancel but sensor is pressed, it will be set mode next;
         * in this condition, attention the released resource is NULL;
         */
        err = gf_state_cancel_prior_test_operation(&cmd->cmd_header.reset_flag);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_ta_invoke_test_cmd_entry_point(uint32_t cmd_id, void *buffer, uint32_t buffer_len) {
    gf_error_t err = GF_SUCCESS;
    gf_cmd_header_t *cmd = NULL;
    gf_operation_type_t cur_operation = OPERATION_NONE;
    gf_mode_t cur_mode = MODE_NONE;

    FUNC_ENTER();

    if (NULL == buffer || buffer_len < sizeof(gf_cmd_header_t)) {
        err = GF_ERROR_BAD_PARAMS;
        FUNC_EXIT(err);
        return err;
    }

    cmd = (gf_cmd_header_t *) buffer;

    GF_LOGD(LOG_TAG "[%s] cmd_id=%s, cmd_id_code=%u",
            __func__, gf_strcmd((gf_cmd_id_t) cmd_id), cmd_id);

    switch (cmd_id) {
        case GF_CMD_TEST_UNTRUSTED_ENROLL:
            err = gf_ta_test_untrusted_enroll((gf_cmd_header_t *) buffer);
            break;

        case GF_CMD_TEST_UNTRUSTED_AUTHENTICATE:
            err = gf_ta_test_untrusted_authenticate((gf_authenticate_t*) buffer);
            break;

        case GF_CMD_TEST_DELETE_UNTRUSTED_ENROLLED_FINGER:
            err = gf_ta_test_untrusted_remove();
            break;

        case GF_CMD_TEST_PERFORMANCE:
            err = gf_ta_test_performance((gf_cmd_header_t *) buffer);
            break;

        case GF_CMD_TEST_SPI_PERFORMANCE:
            err = gf_ta_test_spi_performance((gf_cmd_header_t *) buffer);
            break;

        case GF_CMD_TEST_SPI_TRANSFER:
            err = gf_ta_test_spi_transfer((gf_cmd_header_t *) buffer);
            break;

        case GF_CMD_TEST_PRE_SPI:
            err = gf_ta_test_pre_spi((gf_cmd_header_t *) buffer);
            break;

        case GF_CMD_TEST_SPI:
            err = gf_ta_test_spi((gf_test_spi_t*) buffer);
            break;

        case GF_CMD_TEST_SPI_RW:
            err = gf_ta_test_spi_rw((gf_test_spi_rw_t*) buffer);
            break;

        case GF_CMD_TEST_RESET_PIN1:
            err = gf_ta_test_reset_pin1((gf_cmd_header_t *) buffer);
            break;

        case GF_CMD_TEST_RESET_PIN2:
            err = gf_ta_test_reset_pin2((gf_cmd_header_t *) buffer);
            break;

        case GF_CMD_TEST_INTERRUPT_PIN:
            err = gf_ta_test_interrupt_pin((gf_cmd_header_t *) buffer);
            break;

        case GF_CMD_TEST_FRR_FAR_INIT:
            err = gf_ta_test_frr_far_init((gf_test_frr_far_t*) buffer);
            break;

        case GF_CMD_TEST_FRR_FAR_RECORD_BASE_FRAME:
            err = gf_ta_test_frr_far_record_base_frame((gf_cmd_header_t *) buffer);
            break;

        case GF_CMD_TEST_FRR_FAR_RECORD_ENROLL:
            err = gf_ta_test_frr_far_record_enroll((gf_cmd_header_t *) buffer);
            break;

        case GF_CMD_TEST_FRR_FAR_RECORD_AUTHENTICATE:
            err = gf_ta_test_frr_far_record_authenticate((gf_cmd_header_t *) buffer);
            break;

        case GF_CMD_TEST_FRR_FAR_RECORD_AUTHENTICATE_FINISH:
            err = gf_ta_test_frr_far_record_authenticate_finish((gf_cmd_header_t *) buffer);
            break;

        case GF_CMD_TEST_FRR_FAR_PLAY_BASE_FRAME:
            err = gf_ta_test_frr_far_play_base_frame((gf_test_frr_far_t*) buffer);
            break;

        case GF_CMD_TEST_FRR_FAR_PLAY_ENROLL:
            err = gf_ta_test_frr_far_play_enroll((gf_test_frr_far_t*) buffer);
            break;

        case GF_CMD_TEST_FRR_FAR_PLAY_AUTHENTICATE:
            err = gf_ta_test_frr_far_play_authenticate((gf_test_frr_far_t*) buffer);
            break;

        case GF_CMD_TEST_FRR_FAR_ENROLL_FINISH:
            err = gf_ta_test_frr_far_enroll_finish((gf_cmd_header_t *) buffer);
            break;

        case GF_CMD_TEST_FRR_FAR_CANCEL:
            err = gf_ta_test_frr_far_cancel((gf_cancel_t*) buffer);
            break;

        case GF_CMD_TEST_PRE_GET_VERSION:
            err = gf_ta_test_pre_get_version((gf_cmd_header_t *) buffer);
            break;

        case GF_CMD_TEST_GET_VERSION:
            err = gf_ta_test_get_version((gf_test_get_version_t*) buffer);
            break;

        case GF_CMD_TEST_SET_CONFIG:
            err = gf_ta_test_set_config((gf_test_set_config_t*) buffer);
            break;

        case GF_CMD_TEST_REAL_TIME_DATA:
            err = gf_ta_test_real_time_data((gf_cmd_header_t*) buffer);
            break;

        case GF_CMD_TEST_BMP_DATA:
            err = gf_ta_test_bmp_data((gf_cmd_header_t*) buffer);
            break;

        case GF_CMD_TEST_NOISE:
            err = gf_ta_test_data_noise((gf_test_data_noise_t*) buffer);
            break;

        case GF_CMD_TEST_PIXEL_OPEN:
            err = gf_ta_test_pixel_open((gf_cmd_header_t *) buffer);
            break;

        case GF_CMD_TEST_PIXEL_OPEN_STEP1:
            err = gf_ta_test_pixel_open_step1((gf_cmd_header_t *) buffer);
            break;

        case GF_CMD_TEST_PIXEL_OPEN_FINISH:
            err = gf_ta_test_pixel_open_finish((gf_cmd_header_t *) buffer);
            break;

        case GF_CMD_TEST_DRIVER_CMD:
            err = gf_ta_test_driver_cmd((gf_test_driver_cmd_t*) buffer);
            break;

        case GF_CMD_TEST_BAD_POINT:
            err = gf_ta_test_bad_point((gf_cmd_header_t *) buffer);
            break;

        case GF_CMD_TEST_CANCEL:
            err = gf_ta_test_cancel((gf_cancel_t*) buffer);
            break;

        case GF_CMD_TEST_PRIOR_CANCEL:
            err = gf_ta_test_prior_cancel((gf_cancel_t*) buffer);
            break;

        default: {
            err = gf_ta_test_chip_series_invoke_cmd_entry_point(cmd_id, buffer);
            break;
        }
    }

    cur_operation = gf_state_get_current_operation();
    cur_mode = gf_state_get_current_mode();

    cmd->mode = cur_mode;
    cmd->operation = cur_operation;

    /*compatible with different TEE*/
    cmd->result = err;

    gf_state_get_operation_array(cmd->operation_array, sizeof(cmd->operation_array));

    GF_LOGD(LOG_TAG "[%s] mode=%s, mode_code=%d", __func__, gf_strmode(cur_mode), cur_mode);

    GF_LOGD(LOG_TAG "[%s] operation=%s, operation_code=%d",
            __func__, gf_stroperation(cur_operation), cur_operation);

    FUNC_EXIT(err);

    return err;
}

