/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#include "gf_irq.h"

#include <stdint.h>
#include <string.h>

#include "gf_common.h"
#include "gf_error.h"
#include "gf_spi.h"
#include "gf_tee_internal_api.h"
#include "gf_log.h"
#include "gf_algo.h"
#include "gf_base.h"
#include "gf_broken.h"
#include "cpl_string.h"
#include "gf_sensor.h"
#include "gf_utils.h"
#include "gf_hw_common.h"
#include "gf_state_common.h"
#include "gf_irq_common.h"
#include "gf_nav.h"
#include "gf_bad_point_test.h"
#include "gf_config.h"
#include "gf_state.h"
#include "gf_fido.h"
#include "gf_hw.h"
#include "cpl_memory.h"

#define LOG_TAG "[gf_irq]"

extern gf_test_performance_t g_dump_performance;
extern uint32_t g_image_count;

extern uint32_t g_ignore_irq_mask;
extern uint64_t g_operation_id;
extern uint64_t g_user_id;
extern uint64_t g_authenticator_id;
extern gf_authenticate_fido_t g_authenticator_fido;
extern uint8_t g_last_authenticate_result;

extern gf_image_data_t *g_image_data;
extern gf_nav_data_t *g_nav_data;
extern gf_base_data_t *g_base_data;

extern gf_image_origin_data_t *g_image_origin_data;
extern gf_nav_origin_data_t *g_nav_origin_data;

static uint32_t g_authenticate_fail_count = 0;
static uint32_t g_best_quality_for_authenticate = 0;
static uint32_t g_study_enable_flag = 1;
extern uint8_t g_enable_sensor_broken_check;

static gf_error_t irq_enroll(gf_irq_t *irq);
static gf_error_t irq_authenticate(gf_irq_t *irq);
static gf_error_t irq_test_frr_far_record_enroll(gf_irq_t *irq);
static gf_error_t irq_test_frr_far_record_authenticate(gf_irq_t *irq);
static gf_error_t irq_finger_base(gf_irq_t *irq);
static gf_error_t irq_nav_base(gf_irq_t *irq);
static gf_error_t irq_key(gf_irq_t *irq);
static gf_error_t irq_nav(gf_irq_t *irq);
static gf_error_t irq_test_bad_point(gf_irq_t *irq);
static gf_error_t irq_test_pixel_open(gf_irq_t *irq);
static gf_error_t irq_test_performance(gf_irq_t *irq);
static gf_error_t irq_test_spi_transfer(gf_irq_t *irq);
static gf_error_t irq_test_spi_performance(gf_irq_t *irq);
static gf_error_t irq_test_real_time_data(gf_irq_t *irq);
static gf_error_t irq_test_bmp_data(gf_irq_t *irq);
static gf_error_t irq_test_interrupt_pin(gf_irq_t *irq);
static gf_error_t irq_broken_check(gf_irq_t *irq);

void gf_irq_init_global_variable(void) {
    VOID_FUNC_ENTER();

    g_authenticate_fail_count = 0;
    g_study_enable_flag = 1;

    VOID_FUNC_EXIT();
}

static gf_error_t irq_process(gf_irq_t *irq) {
    gf_error_t err = GF_SUCCESS;
    uint8_t reset_flag = 0;

    FUNC_ENTER();

    // irq process by generally
    if (irq->irq_type & GF_IRQ_FINGER_DOWN_MASK) {
        GF_LOGI(LOG_TAG "[%s] receive IRQ_FINGER_DOWN", __func__);
        g_image_count = 0;
    }

    if (irq->irq_type & GF_IRQ_RESET_MASK) {
        GF_LOGI(LOG_TAG "[%s] receive IRQ_RESET", __func__);
        gf_irq_common_reset(irq->irq_type);
    }

    if (irq->irq_type & GF_IRQ_TEMPERATURE_CHANGE_MASK) {
        GF_LOGI(LOG_TAG "[%s] receive IRQ_TEMP_CHANGE", __func__);
        gf_base_on_temperature_change_irq(&reset_flag);
        GF_LOGI(LOG_TAG "[%s] reset_flag=%d", __func__, reset_flag);
    }

    if (irq->irq_type & GF_IRQ_IMAGE_MASK) {
        g_image_count++;

        if (!gf_state_is_authenticate(irq->operation) && (irq->mode != MODE_TEST_BAD_POINT)) {
            gf_hw_data_sample_suspend();
        }
    }

    // irq process by operation
    switch (irq->operation) {
        case OPERATION_ENROLL:
            err = irq_enroll(irq);
            if (gf_algo_get_preprocess_times() >= UPDATE_BASE_THRESHOLD
                    && gf_base_is_finger_base_vaild()) {
                gf_base_save_calibration();
            }
            break;

        case OPERATION_TEST_UNTRUSTED_ENROLL:
            err = irq_enroll(irq);
            break;

        case OPERATION_TEST_FRR_FAR_RECORD_ENROLL:
            err = irq_test_frr_far_record_enroll(irq);
            break;

        case OPERATION_AUTHENTICATE_FF:
        case OPERATION_AUTHENTICATE_IMAGE:
        case OPERATION_AUTHENTICATE_FIDO:
            err = irq_authenticate(irq);
            // TODO(goodix): base handle
            if (gf_algo_get_preprocess_times() >= UPDATE_BASE_THRESHOLD
                    && gf_base_is_finger_base_vaild()) {
                gf_base_save_calibration();
            }
            break;

        case OPERATION_TEST_UNTRUSTED_AUTHENTICATE:
            err = irq_authenticate(irq);
            break;

        case OPERATION_TEST_FRR_FAR_RECORD_AUTHENTICATE:
            err = irq_test_frr_far_record_authenticate(irq);
            break;

        case OPERATION_FINGER_BASE:
        case OPERATION_TEST_FRR_FAR_RECORD_BASE_FRAME:
        case OPERATION_TEST_BAD_POINT_RECODE_BASE:
            err = irq_finger_base(irq);
            break;

        case OPERATION_NAV_BASE:
            err = irq_nav_base(irq);
            break;

        case OPERATION_NAV:
            err = irq_nav(irq);
            break;

        case OPERATION_TEST_BAD_POINT:
            err = irq_test_bad_point(irq);
            break;

        case OPERATION_TEST_PIXEL_OPEN_STEP1:
        case OPERATION_TEST_PIXEL_OPEN_STEP2:
            err = irq_test_pixel_open(irq);
            break;

        case OPERATION_TEST_PERFORMANCE:
            err = irq_test_performance(irq);
            break;

        case OPERATION_TEST_SPI_TRANSFER:
            err = irq_test_spi_transfer(irq);
            break;

        case OPERATION_TEST_SPI_PERFORMANCE:
            err = irq_test_spi_performance(irq);
            break;

        case OPERATION_TEST_REAL_TIME_DATA:
            err = irq_test_real_time_data(irq);
            break;

        case OPERATION_TEST_BMP_DATA:
            err = irq_test_bmp_data(irq);
            break;

        case OPERATION_TEST_DATA_NOISE:
            err = irq_test_data_noise(irq);
            break;

        case OPERATION_TEST_INTERRUPT_PIN:
            err = irq_test_interrupt_pin(irq);
            break;

        case OPERATION_HOME_KEY:
        case OPERATION_CAMERA_KEY:
        case OPERATION_POWER_KEY:
            err = irq_key(irq);
            break;

        case OPERATION_TEST_SENSOR_VALIDITY:
            err = gf_hw_test_sensor_validity();
            if (GF_SUCCESS == err) {
                GF_LOGD(LOG_TAG "[%s] sensor check pass", __func__);
                irq->test_sensor_validity.is_passed = 1;
            } else {
                GF_LOGD(LOG_TAG "[%s] sensor check failed", __func__);
                irq->test_sensor_validity.is_passed = 0;
            }
            err = gf_state_cancel_prior_test_operation(&irq->cmd_header.reset_flag);
            break;

        case OPERATION_BROKEN_CHECK_DEFAULT:
        case OPERATION_BROKEN_CHECK_NEGATIVE:
        case OPERATION_BROKEN_CHECK_POSITIVE:
            irq_broken_check(irq);
            break;

        case OPERATION_NONE:
            GF_LOGI(LOG_TAG "[%s] irq_type=%s, irq_type=0x%08X when OPERATION_NONE", __func__,
                    gf_strirq(irq->irq_type), irq->irq_type);
            break;

        default:
            err = GF_ERROR_GENERIC;
            GF_LOGE(LOG_TAG "[%s] irq_type=%s, irq_type=0x%08x when operation=%s, operation=%d",
                    __func__, gf_strirq(irq->irq_type), irq->irq_type,
                    gf_stroperation(irq->operation), irq->operation);
            break;
    }

    FUNC_EXIT(err);

    return err;
}

static gf_error_t irq_test_interrupt_pin(gf_irq_t *irq) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == irq) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad paramter. err=%s, errno=%d", __func__, gf_strerror(err), err);
            break;
        }

        if (irq->irq_type & (GF_IRQ_NAV_MASK | GF_IRQ_IMAGE_MASK)) {
            err = gf_state_cancel_test_operation(&irq->cmd_header.reset_flag);
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t irq_broken_check(gf_irq_t *irq) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == irq) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad paramter. err=%s, errno=%d", __func__, gf_strerror(err), err);
            break;
        }

        if (irq->irq_type & GF_IRQ_FINGER_UP_MASK) {
            gf_state_operation_done(&irq->cmd_header.reset_flag);
            gf_base_on_up_irq(&irq->cmd_header.reset_flag);
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t irq_key(gf_irq_t *irq) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == irq) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad paramter. err=%s, errno=%d", __func__, gf_strerror(err), err);
            break;
        }

        GF_LOGD(LOG_TAG "[%s] irq_type=%s, irq_type_code=0x%08X",
                __func__, gf_strirq(irq->irq_type), irq->irq_type);

        if (irq->irq_type & GF_IRQ_FINGER_DOWN_MASK) {
            if (g_config.require_down_and_up_in_pairs_for_key_mode > 0) {
                gf_state_require_down_and_up_in_pairs_begin();
            }
        }
        if (irq->irq_type & GF_IRQ_FINGER_UP_MASK) {
            if (g_config.require_down_and_up_in_pairs_for_key_mode > 0) {
                gf_state_require_down_and_up_in_pairs_end(&irq->cmd_header.reset_flag);
            }
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t irq_test_spi_performance(gf_irq_t *irq) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == irq) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad paramter. err=%s, errno=%d", __func__, gf_strerror(err), err);
            break;
        }

        GF_LOGD(LOG_TAG "[%s] irq_type=%s, irq_type_code=0x%08X",
                __func__, gf_strirq(irq->irq_type), irq->irq_type);

        if (irq->irq_type & GF_IRQ_IMAGE_MASK) {
            gf_mode_t test_mode = MODE_NONE;

            irq->test_spi_performance.get_image_time = gf_get_time_delta();
            irq->test_spi_performance.raw_data_len = g_image_origin_data->data_len;

            gf_get_time_delta();
            irq->test_spi_performance.get_dr_timestamp_time = gf_get_time_delta();

            err = gf_hw_get_mode(&test_mode);
            irq->test_spi_performance.get_mode_time = gf_get_time_delta();

            // autocancel
            err = gf_state_cancel_test_operation(&irq->cmd_header.reset_flag);
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t irq_test_real_time_data(gf_irq_t *irq) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == irq) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad paramter. err=%s, errno=%d", __func__, gf_strerror(err), err);
            break;
        }

        GF_LOGD(LOG_TAG "[%s] irq_type=%s, irq_type_code=0x%08X",
                __func__, gf_strirq(irq->irq_type), irq->irq_type);

        if (irq->irq_type & GF_IRQ_IMAGE_MASK) {
            err = gf_algo_test_real_time_data(g_image_data->raw_data, g_image_data->raw_data_len,
                    &(irq->test_real_time_data));

            if (GF_SUCCESS == err) {
                cpl_memcpy(irq->test_real_time_data.raw_data, g_image_data->raw_data,
                        g_image_data->raw_data_len * sizeof(uint16_t));
                cpl_memcpy(irq->test_real_time_data.base_data, g_base_data->finger_base_rawdata,
                        g_base_data->finger_base_rawdata_len * sizeof(uint16_t));
            } else {
                gf_hw_data_sample_resume();
            }
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t irq_test_bmp_data(gf_irq_t *irq) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == irq) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad paramter. err=%s, errno=%d", __func__, gf_strerror(err), err);
            break;
        }

        GF_LOGD(LOG_TAG "[%s] irq_type=%s, irq_type_code=0x%08X",
                __func__, gf_strirq(irq->irq_type), irq->irq_type);

        if (irq->irq_type & GF_IRQ_IMAGE_MASK) {
            err = gf_algo_test_bmp_data(g_image_data->raw_data, g_image_data->raw_data_len,
                    &(irq->test_bmp_data));

            if (GF_SUCCESS != err) {
                gf_hw_data_sample_resume();
            }
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t irq_test_spi_transfer(gf_irq_t *irq) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == irq) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad paramter. err=%s, errno=%d", __func__, gf_strerror(err), err);
            break;
        }

        GF_LOGD(LOG_TAG "[%s] irq_type=%s, irq_type_code=0x%08X",
                __func__, gf_strirq(irq->irq_type), irq->irq_type);

        if (irq->irq_type & GF_IRQ_IMAGE_MASK) {
            gf_hw_data_sample_resume();
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t irq_test_performance(gf_irq_t *irq) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == irq) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad paramter. err=%s, errno=%d", __func__, gf_strerror(err), err);
            break;
        }

        GF_LOGD(LOG_TAG "[%s] irq_type=%s, irq_type_code=0x%08X",
                __func__, gf_strirq(irq->irq_type), irq->irq_type);

        if (irq->irq_type & GF_IRQ_FINGER_DOWN_MASK) {
            // do nothing
        }

        if (irq->irq_type & GF_IRQ_IMAGE_MASK) {
            irq->test_performance.get_raw_data_time = gf_get_time_delta();
            err = gf_algo_test_performance(g_image_data->raw_data, g_image_data->raw_data_len,
                    &irq->test_performance);
        }

        if (irq->irq_type & GF_IRQ_FINGER_UP_MASK) {
            // do nothing
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t irq_test_pixel_open(gf_irq_t *irq) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == irq) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad paramter. err=%s, errno=%d", __func__, gf_strerror(err), err);
            break;
        }

        GF_LOGD(LOG_TAG "[%s] irq_type=%s, irq_type_code=0x%08X",
                __func__, gf_strirq(irq->irq_type), irq->irq_type);

        if (irq->irq_type & GF_IRQ_IMAGE_MASK) {
            if (OPERATION_TEST_PIXEL_OPEN_STEP1 == irq->operation) {
                irq->image_count = g_image_count;
                gf_pixel_test_step1(g_image_data->raw_data, g_image_data->raw_data_len);
                err = gf_state_execute_operation(OPERATION_TEST_PIXEL_OPEN_STEP2,
                        &irq->cmd_header.reset_flag);
            } else if (OPERATION_TEST_PIXEL_OPEN_STEP2 == irq->operation) {
                irq->image_count = g_image_count;
                gf_pixel_test_step2(g_image_data->raw_data, g_image_data->raw_data_len);
                gf_pixel_test(&(irq->bad_pixel_num));
            } else {
                GF_LOGE(LOG_TAG "[%s] no such a operation=%s, operation=%d", __func__,
                        gf_stroperation(irq->operation), irq->operation);
            }
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t irq_test_bad_point(gf_irq_t *irq) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == irq) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad paramter. err=%s, errno=%d", __func__, gf_strerror(err), err);
            break;
        }

        GF_LOGD(LOG_TAG "[%s] irq_type=%s, irq_type_code=0x%08X",
                __func__, gf_strirq(irq->irq_type), irq->irq_type);

        if (irq->irq_type & GF_IRQ_FINGER_DOWN_MASK) {
            // do nothing
        }

        if (irq->irq_type & GF_IRQ_IMAGE_MASK) {
            g_image_count++;

            gf_bad_point_test_on_image_irq(g_image_data->raw_data,
                    g_image_data->raw_data_len * sizeof(uint16_t), irq);
            if (irq->test_bad_point.algo_processed_flag > 0) {
                err = gf_state_cancel_test_operation(&irq->cmd_header.reset_flag);
            }
        }

        if (irq->irq_type & GF_IRQ_FINGER_UP_MASK) {
            gf_base_on_up_irq(&irq->cmd_header.reset_flag);
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t irq_nav(gf_irq_t *irq) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == irq) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad paramter. err=%s, errno=%d", __func__, gf_strerror(err), err);
            break;
        }

        GF_LOGD(LOG_TAG "[%s] irq_type=%s, irq_type_code=0x%08X",
                __func__, gf_strirq(irq->irq_type), irq->irq_type);

        if (irq->irq_type & GF_IRQ_FINGER_DOWN_MASK) {
            if (g_config.require_down_and_up_in_pairs_for_nav_mode > 0) {
                gf_state_require_down_and_up_in_pairs_begin();
            }
        }

        if (irq->irq_type & GF_IRQ_NAV_MASK) {
            // nothing to do
        }

        if (irq->irq_type & GF_IRQ_FINGER_UP_MASK) {
            if (g_config.require_down_and_up_in_pairs_for_nav_mode > 0) {
                gf_state_require_down_and_up_in_pairs_end(&irq->cmd_header.reset_flag);
            }
            gf_base_on_up_irq(&irq->cmd_header.reset_flag);
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t irq_nav_base(gf_irq_t *irq) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == irq) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad paramter. err=%s, errno=%d", __func__, gf_strerror(err), err);
            break;
        }

        GF_LOGD(LOG_TAG "[%s] irq_type=%s, irq_type_code=0x%08X",
                __func__, gf_strirq(irq->irq_type), irq->irq_type);

        if (irq->irq_type & GF_IRQ_NAV_MASK) {
            err = gf_base_update_nav_base(g_nav_data->raw_data[0], g_nav_data->raw_data_len,
                    &irq->cmd_header.reset_flag);
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t irq_finger_base(gf_irq_t *irq) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == irq) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad paramter. err=%s, errno=%d", __func__, gf_strerror(err), err);
            break;
        }

        GF_LOGD(LOG_TAG "[%s] irq_type=%s, irq_type_code=0x%08X",
                __func__, gf_strirq(irq->irq_type), irq->irq_type);

        if (irq->irq_type & GF_IRQ_IMAGE_MASK) {
            if (OPERATION_FINGER_BASE == irq->operation) {
                // TODO(goodix): base handle
                err = gf_base_update_finger_base(g_image_data->raw_data, g_image_data->raw_data_len,
                        &irq->cmd_header.reset_flag);
            } else if (OPERATION_TEST_FRR_FAR_RECORD_BASE_FRAME == irq->operation) {
                GF_LOGD(LOG_TAG "[%s] OPERATION_TEST_FRR_FAR_RECORD_BASE_FRAME", __func__);
                err = gf_frr_far_calibration(g_image_data->raw_data, g_image_data->raw_data_len);
                if (GF_SUCCESS == err) {
                    cpl_memcpy(irq->test_frr_far.raw_data, g_image_data->raw_data,
                            g_image_data->raw_data_len * sizeof(uint16_t));
                    err = gf_state_execute_operation(gf_state_get_default_operation(),
                            &irq->cmd_header.reset_flag);
                } else {
                    gf_hw_data_sample_resume();
                }
            } else if (OPERATION_TEST_BAD_POINT_RECODE_BASE == irq->operation) {
                GF_LOGD(LOG_TAG "[%s] OPERATION_TEST_BAD_POINT_RECODE_BASE irq", __func__);
                err = gf_bad_point_test_save_base_frame_data(g_image_data->raw_data,
                                       g_image_data->raw_data_len);

                if (GF_SUCCESS != err) {
                    GF_LOGD(LOG_TAG "[%s] RECODE_BASE error", __func__);
                    gf_bad_point_test_destroy();
                    break;
                }
                err = gf_state_execute_operation(OPERATION_TEST_BAD_POINT,
                          &(irq->cmd_header.reset_flag));

            } else {
                GF_LOGE(LOG_TAG "[%s] did not process such a operation=%s, operation_code=%d",
                        __func__, gf_stroperation(irq->operation), irq->operation);
            }
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t irq_enroll(gf_irq_t *irq) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == irq) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad paramter. err=%s, errno=%d", __func__, gf_strerror(err), err);
            break;
        }

        GF_LOGD(LOG_TAG "[%s] irq_type=%s, irq_type_code=0x%08X",
                __func__, gf_strirq(irq->irq_type), irq->irq_type);

        if (irq->irq_type & GF_IRQ_FINGER_DOWN_MASK) {
            if (g_config.require_down_and_up_in_pairs_for_image_mode > 0) {
                gf_state_require_down_and_up_in_pairs_begin();
            }
        }

        if (irq->irq_type & GF_IRQ_IMAGE_MASK) {
            int32_t image_quality = 0;
            int32_t valid_area = 0;
            int32_t select_index = -1;

            GF_GET_TIME_DELTA_X(g_dump_performance.get_raw_data_time);

            GF_LOGD(LOG_TAG "g_enable_sensor_broken_check=%d", __func__,
                    g_enable_sensor_broken_check);
            if ((g_config.support_sensor_broken_check > 0) &&
                    (irq->operation != OPERATION_TEST_UNTRUSTED_ENROLL)) {
                if (1 == g_enable_sensor_broken_check) {
                    err = gf_broken_check(&irq->broken_check,
                        &irq->cmd_header.reset_flag, irq->operation);
                    if (err != GF_SUCCESS) {
                        break;
                    }
                    if (0 == irq->broken_check.disable_sensor) {
                        g_enable_sensor_broken_check = 0;
                    } else {
                        g_enable_sensor_broken_check = 1;
                    }
                }
                GF_GET_TIME_DELTA_X(g_dump_performance.broken_check_time);
            }

            if (irq->broken_check.disable_sensor > 0) {
                err = GF_ERROR_SENSOR_IS_BROKEN;
                GF_LOGE(LOG_TAG "[%s] sensor is broken. err=%s, errno=%d", __func__,
                        gf_strerror(err), err);
                break;
            }

            /* algo chooses the better frame for enrolling */
            err = gf_algo_get_acquired_info(g_image_data->raw_data, g_image_data->raw_data_len,
                    MAX_CONTINUE_FRAME_NUM,
                    &select_index, irq->operation, &image_quality, &valid_area);

            GF_GET_TIME_DELTA_X(g_dump_performance.preprocess_time);
            if (err != GF_SUCCESS) {
                break;
            }

            GF_LOGI(LOG_TAG "[%s] select_index=%d", __func__, select_index);
            g_image_data->enroll_select_index = select_index;

            irq->samples_remaining = g_config.enrolling_min_templates;
            if (OPERATION_ENROLL == irq->operation) {
                err = gf_algo_enroll(NULL, 0, &irq->group_id, &irq->finger_id,
                        &irq->samples_remaining, &irq->duplicate_finger_id);
            } else if (OPERATION_TEST_UNTRUSTED_ENROLL == irq->operation) {
                err = gf_algo_test_enroll(&irq->group_id, &irq->finger_id, &irq->samples_remaining,
                        &irq->duplicate_finger_id);
            }

            if (err != GF_SUCCESS) {
                break;
            }

            // enroll complete
            if (0 == irq->samples_remaining) {
                if (OPERATION_ENROLL == irq->operation) {
                    gf_state_cancel_operation(&irq->cmd_header.reset_flag);
                } else if (OPERATION_TEST_UNTRUSTED_ENROLL == irq->operation) {
                    gf_state_cancel_test_operation(&irq->cmd_header.reset_flag);
                }
            }

            if (GF_PERFORMANCE_DUMP_ENABLED) {
                cpl_memcpy(&irq->dump_performance, &g_dump_performance,
                        sizeof(gf_test_performance_t));
            }
        }

        if (irq->irq_type & GF_IRQ_FINGER_UP_MASK) {
            if (g_config.require_down_and_up_in_pairs_for_image_mode > 0) {
                gf_state_require_down_and_up_in_pairs_end(&irq->cmd_header.reset_flag);
            }

            if (0 == g_image_count) {
                irq->too_fast_flag = 1;
            } else {
                irq->too_fast_flag = 0;
            }
            // TODO(goodix): base handle
            gf_base_on_up_irq(&irq->cmd_header.reset_flag);
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t irq_authenticate(gf_irq_t *irq) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == irq) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad paramter. err=%s, errno=%d", __func__, gf_strerror(err), err);
            break;
        }

        GF_LOGD(LOG_TAG "[%s] irq_type=%s, irq_type_code=0x%08X",
                __func__, gf_strirq(irq->irq_type), irq->irq_type);

        if (irq->irq_type & GF_IRQ_FINGER_DOWN_MASK) {
            g_authenticate_fail_count = 0;
            g_best_quality_for_authenticate = 0;
            g_study_enable_flag = 1;
            if (((OPERATION_AUTHENTICATE_IMAGE == irq->operation)
                    && (g_config.require_down_and_up_in_pairs_for_image_mode > 0))
                    || ((OPERATION_AUTHENTICATE_FF == irq->operation)
                            && (g_config.require_down_and_up_in_pairs_for_ff_mode > 0))) {
                gf_state_require_down_and_up_in_pairs_begin();
            }
        }

        if (irq->irq_type & GF_IRQ_IMAGE_MASK) {
            uint64_t timestamp = 0;
            int32_t image_quality = 0;
            int32_t valid_area = 0;
            int32_t match_score = 0;
            uint32_t is_finger_touched = 1;

            GF_GET_TIME_DELTA_X(g_dump_performance.get_raw_data_time);

            gf_hw_check_finger_touched(&is_finger_touched);
            if (0 == is_finger_touched) {
                uint32_t valid_touch_frame_threshold = 0;

                GF_LOGI(LOG_TAG "[%s] finger is up, so don't need to retry", __func__);

                // press too fast
                if (OPERATION_AUTHENTICATE_IMAGE == irq->operation
                        || OPERATION_AUTHENTICATE_FIDO == irq->operation) {
                    valid_touch_frame_threshold = g_config.screen_on_valid_touch_frame_threshold;
                } else if (OPERATION_AUTHENTICATE_FF == irq->operation) {
                    valid_touch_frame_threshold = g_config.screen_off_valid_touch_frame_threshold;
                }

                if (g_authenticate_fail_count < valid_touch_frame_threshold) {
                    GF_LOGI(LOG_TAG "[%s] press too fast", __func__);
                    irq->too_fast_flag = 1;
                } else if (g_best_quality_for_authenticate
                        < g_config.image_quality_threshold_for_mistake_touch) {
                    // touch by mistake
                    GF_LOGI(LOG_TAG "[%s] touch by mistake", __func__);
                    irq->mistake_touch_flag = 1;
                }

                err = GF_ERROR_NOT_MATCH;
                // authenticate failed
                gf_sync_auth_result_notify(irq->group_id, irq->finger_id, g_operation_id, err);
                if (GF_PERFORMANCE_DUMP_ENABLED) {
                    cpl_memcpy(&irq->dump_performance, &g_dump_performance,
                            sizeof(gf_test_performance_t));
                }
                gf_hw_data_sample_suspend();
                g_authenticate_fail_count = 0;
                break;
            }

            err = gf_algo_get_acquired_info(g_image_data->raw_data, g_image_data->raw_data_len, 1,
                    NULL, irq->operation, &image_quality, &valid_area);

            GF_GET_TIME_DELTA_X(g_dump_performance.preprocess_time);
            if (err == GF_SUCCESS) {
                if (OPERATION_TEST_UNTRUSTED_AUTHENTICATE == irq->operation) {
                    err = gf_algo_test_authenticate(1, &irq->group_id, &irq->finger_id,
                            &image_quality, &valid_area, &match_score);
                } else {
                    err = gf_algo_authenticate(NULL, 0, g_study_enable_flag, &irq->group_id,
                            &irq->finger_id, &irq->save_flag, &irq->update_stitch_flag,
                            &image_quality, &valid_area, &match_score);
                }
            }

            /* retry logic */
            if (err != GF_SUCCESS) {
                uint32_t max_authenticate_failed_retry_count = 0;

                if (gf_algo_get_safe_class() >= GF_SAFE_CLASS_MEDIUM) {
                    if (OPERATION_AUTHENTICATE_IMAGE == irq->operation
                            || OPERATION_AUTHENTICATE_FIDO == irq->operation) {
                        max_authenticate_failed_retry_count =
                                g_config.screen_on_authenticate_fail_retry_count;
                    } else if (OPERATION_AUTHENTICATE_FF == irq->operation) {
                        max_authenticate_failed_retry_count =
                                g_config.screen_off_authenticate_fail_retry_count;
                    }
                }

                g_authenticate_fail_count++;

                if (g_authenticate_fail_count <= max_authenticate_failed_retry_count) {
                    gf_hw_data_sample_resume();
                    err = GF_ERROR_MATCH_FAIL_AND_RETRY;
                    GF_LOGI(LOG_TAG "[%s] authenticate failed, need retry. "
                            "g_authenticate_fail_count=%u", __func__, g_authenticate_fail_count);
                    break;
                } else {
                    gf_hw_data_sample_suspend();
                    if (g_best_quality_for_authenticate
                            < g_config.image_quality_threshold_for_mistake_touch) {
                        GF_LOGI(LOG_TAG "[%s] touch by mistake, g_best_quality_for_authenticate=%u",
                                __func__, g_best_quality_for_authenticate);
                        irq->mistake_touch_flag = 1;
                    }
                    // authenticate failed
                    gf_sync_auth_result_notify(irq->group_id, irq->finger_id, g_operation_id, err);
                    if (GF_PERFORMANCE_DUMP_ENABLED) {
                        cpl_memcpy(&irq->dump_performance, &g_dump_performance,
                                sizeof(gf_test_performance_t));
                    }
                }

                break;
            }

            g_last_authenticate_result = 1;

            // authenticate success
            if ((OPERATION_AUTHENTICATE_IMAGE == irq->operation)
                    || (OPERATION_AUTHENTICATE_FF == irq->operation)) {
                gf_state_cancel_operation(&irq->cmd_header.reset_flag);
            } else {
                gf_state_cancel_test_operation(&irq->cmd_header.reset_flag);
            }

            gf_sync_auth_result_notify(irq->group_id, irq->finger_id, g_operation_id, err);

            if (GF_PERFORMANCE_DUMP_ENABLED) {
                cpl_memcpy(&irq->dump_performance, &g_dump_performance,
                        sizeof(gf_test_performance_t));
            }

            gf_get_timestamp(&timestamp);

            if (OPERATION_AUTHENTICATE_IMAGE == irq->operation
                    || OPERATION_AUTHENTICATE_FF == irq->operation) {
                irq->auth_token.challenge = g_operation_id;
                irq->auth_token.authenticator_id = g_authenticator_id;
                irq->auth_token.user_id = g_user_id;
                irq->auth_token.timestamp = timestamp;

                gf_generate_hmac_authenticate(&irq->auth_token);
            } else if (OPERATION_AUTHENTICATE_FIDO == irq->operation) {
                gf_verify_t verify = { 0 };
                verify.result = 1;
                verify.final_challenge_len = g_authenticator_fido.final_challenge_len;
                cpl_memcpy(verify.final_challenge, g_authenticator_fido.final_challenge,
                        MAX_FINAL_CHALLENGE_LEN);
                verify.aaid_len = g_authenticator_fido.aaid_len;
                cpl_memcpy(verify.aaid, g_authenticator_fido.aaid, MAX_AAID_LEN);
                verify.user_id = (uint32_t) g_user_id;
                verify.timestamp = (uint32_t) timestamp;
                verify.user_verification_index = irq->finger_id;
                verify.match_score = (uint8_t) (match_score * 100 / 256);

                fido_generate_uvt(verify, irq->uvt.uvt_buf, &irq->uvt.uvt_len);
            }

            gf_hw_data_sample_suspend();
        }

        if (irq->irq_type & GF_IRQ_FINGER_UP_MASK) {
            if (((OPERATION_AUTHENTICATE_IMAGE == irq->operation)
                    && (g_config.require_down_and_up_in_pairs_for_image_mode > 0))
                    || ((OPERATION_AUTHENTICATE_FF == irq->operation)
                            && (g_config.require_down_and_up_in_pairs_for_ff_mode > 0))) {
                gf_state_require_down_and_up_in_pairs_end(&irq->cmd_header.reset_flag);
            }

            gf_base_on_up_irq(&irq->cmd_header.reset_flag);
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t irq_test_frr_far_record_enroll(gf_irq_t *irq) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == irq) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad paramter. err=%s, errno=%d", __func__, gf_strerror(err), err);
            break;
        }

        GF_LOGD(LOG_TAG "[%s] irq_type=%s, irq_type_code=0x%08X",
                __func__, gf_strirq(irq->irq_type), irq->irq_type);

        if (irq->irq_type & GF_IRQ_FINGER_DOWN_MASK) {
            // do nothing
        }

        if (irq->irq_type & GF_IRQ_IMAGE_MASK) {
            err = gf_frr_far_enroll(g_image_data->raw_data, g_image_data->raw_data_len,
                    &irq->test_frr_far);
            if (GF_SUCCESS == err) {
                cpl_memcpy(irq->test_frr_far.raw_data, g_image_data->raw_data,
                        g_image_data->raw_data_len * sizeof(uint16_t));

                err = gf_frr_far_dump_info(&(irq->test_frr_far));
            }
        }

        if (irq->irq_type & GF_IRQ_FINGER_UP_MASK) {
            // TODO(goodix): base handle
            gf_base_on_up_irq(&irq->cmd_header.reset_flag);
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t irq_test_frr_far_record_authenticate(gf_irq_t *irq) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == irq) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad paramter. err=%s, errno=%d", __func__, gf_strerror(err), err);
            break;
        }

        GF_LOGD(LOG_TAG "[%s] irq_type=%s, irq_type_code=0x%08X",
                __func__, gf_strirq(irq->irq_type), irq->irq_type);

        if (irq->irq_type & GF_IRQ_FINGER_DOWN_MASK) {
            // do nothing
        }

        if (irq->irq_type & GF_IRQ_IMAGE_MASK) {
            err = gf_frr_far_authenticate_with_greedy_strategy(g_image_data->raw_data,
                    g_image_data->raw_data_len, &irq->test_frr_far);
            if (GF_SUCCESS == err) {
                cpl_memcpy(irq->test_frr_far.raw_data, g_image_data->raw_data,
                        g_image_data->raw_data_len * sizeof(uint16_t));
                err = gf_frr_far_dump_info(&(irq->test_frr_far));
            }
        }

        if (irq->irq_type & GF_IRQ_FINGER_UP_MASK) {
            gf_base_on_up_irq(&irq->cmd_header.reset_flag);
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_irq(gf_irq_t *cmd) {
    gf_error_t err = GF_SUCCESS;
    uint32_t irq_type = 0;
    uint32_t polling_count = 0;

    FUNC_ENTER();

    do {
        if (NULL == cmd) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad paramter. err=%s, errno=%d", __func__, gf_strerror(err), err);
            break;
        }

        GF_LOGD(LOG_TAG "[%s] step=%d", __func__, cmd->step);

        cmd->operation = gf_state_get_current_operation();
        cmd->mode = gf_state_get_current_mode();

        if ((GF_IRQ_STEP_GET_IRQ_TYPE == cmd->step) || (GF_IRQ_STEP_POLLING == cmd->step)) {
            do {
                err = gf_hw_get_irq_type(&irq_type);
                if (err != GF_SUCCESS) {
                    break;
                }

                if (GF_IRQ_STEP_GET_IRQ_TYPE == cmd->step) {
                    GF_LOGD(LOG_TAG "[%s] GF_IRQ_STEP_GET_IRQ_TYPE", __func__);
                    break;
                } else {
                    GF_LOGD(LOG_TAG "[%s] GF_IRQ_STEP_POLLING polling_count=%d",
                            __func__, polling_count);
                    if (irq_type > 0) {
                        break;
                    }

                    gf_sleep(1);
                    polling_count++;
                }
            } while (polling_count < 1000);

            if (err != GF_SUCCESS) {
                break;
            }

            GF_LOGD(LOG_TAG "[%s] get_irq_type, irq_type=%s, irq_type=0x%08X",
                    __func__, gf_strirq(irq_type), irq_type);
            cmd->irq_type = irq_type;

            if (0 == irq_type) {
                FUNC_EXIT(err);
                return err;
            }

            if (cmd->irq_type & (GF_IRQ_NAV_MASK | GF_IRQ_IMAGE_MASK)) {
                if (OPERATION_TEST_INTERRUPT_PIN == cmd->operation) {
                    GF_LOGD(LOG_TAG "[%s] For interrupt pin test, chip mode is idle and host set "
                            "image irq. So idle mode should return image irq", __func__);
                    cmd->step = GF_IRQ_STEP_PROCESS;
                } else {
                    cmd->step = GF_IRQ_STEP_GET_IMAGE;
                }
            } else {
                cmd->step = GF_IRQ_STEP_CLEAR_IRQ;
            }
        }

        if (GF_IRQ_STEP_GET_IMAGE == cmd->step) {
            GF_LOGD(LOG_TAG "[%s] GF_IRQ_STEP_GET_IMAGE", __func__);

            if (OPERATION_NAV_BASE == cmd->operation) {
                cpl_memset(g_nav_data, 0, sizeof(gf_nav_data_t));
                err = gf_hw_get_nav_base_image(g_nav_origin_data->data[0],
                        &g_nav_origin_data->data_len,
                        g_nav_data->raw_data[0], &g_nav_data->raw_data_len);
            } else if (OPERATION_ENROLL == cmd->operation
                    || OPERATION_TEST_UNTRUSTED_ENROLL == cmd->operation) {
                gf_get_time_delta();
                cpl_memset(g_image_data, 0, sizeof(gf_image_data_t));
                g_image_data->enroll_select_index = -1;

                err = gf_hw_get_image(g_image_origin_data->data, &g_image_origin_data->data_len,
                        g_image_data->raw_data, &g_image_data->raw_data_len,
                        MAX_CONTINUE_FRAME_NUM);
            } else {
                gf_get_time_delta();
                cpl_memset(g_image_data, 0, sizeof(gf_image_data_t));

                err = gf_hw_get_image(g_image_origin_data->data, &g_image_origin_data->data_len,
                        g_image_data->raw_data, &g_image_data->raw_data_len, 1);
            }

            if (GF_SUCCESS != err) {
                GF_LOGE(LOG_TAG "[%s] image get failed", __func__);
                break;
            }

            cmd->step = GF_IRQ_STEP_CLEAR_IRQ;
        }

        if (GF_IRQ_STEP_CLEAR_IRQ == cmd->step) {
            GF_LOGD(LOG_TAG "[%s] GF_IRQ_STEP_CLEAR_IRQ", __func__);

            if (irq_type & GF_IRQ_FDT_REVERSE_MASK) {
                gf_hw_clear_irq(irq_type);
            }

            // ingore irq, only for test irq report times & interval, this test won't clear irq
            if (0 == (cmd->irq_type & g_ignore_irq_mask)) {
                // note: cmd->irq_type may be changed after irq preprocess
                err = gf_hw_irq_preprocess(&cmd->irq_type);
                if (GF_SUCCESS != err) {
                    GF_LOGE(LOG_TAG "[%s] irq preprocess failed", __func__);
                    break;
                }
                gf_hw_switch_mode(irq_type);
            }

            cmd->step = GF_IRQ_STEP_PROCESS;
        }
    } while (0);

    if (GF_SUCCESS == err) {
        if (GF_IRQ_STEP_PROCESS == cmd->step) {
            GF_LOGD(LOG_TAG "[%s] GF_IRQ_STEP_PROCESS", __func__);

            err = irq_process(cmd);
        }
    } else {
        if (cmd->step <= GF_IRQ_STEP_CLEAR_IRQ) {
            gf_hw_irq_preprocess(&cmd->irq_type);
            gf_hw_switch_mode(irq_type);
        }
        cmd->step = GF_IRQ_STEP_IDLE;
        cmd->speed = GF_SPI_SPEED_LOW;

        if (cmd->irq_type & (GF_IRQ_NAV_MASK | GF_IRQ_IMAGE_MASK)) {
            gf_hw_data_sample_suspend();
            cmd->irq_type &= (~(GF_IRQ_NAV_MASK | GF_IRQ_IMAGE_MASK));
        }

        if (gf_state_is_base_operation(cmd->operation)) {
            gf_state_operation_done(&cmd->cmd_header.reset_flag);
        }
    }

    FUNC_EXIT(err);

    return err;
}
