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
#include "gf_fdt.h"
#include "gf_config.h"
#include "gf_state.h"
#include "gf_fido.h"
#include "gf_milan.h"

#define LOG_TAG "[gf_irq]"

// below should be refactor
static uint32_t g_best_quality_for_authenticate = 0;

extern uint32_t g_image_count;
extern gf_test_performance_t g_dump_performance;

extern uint32_t g_ignore_irq_mask;
extern uint8_t g_is_normal_cfg;
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
static uint32_t g_study_enable_flag = 1;

static gf_error_t irq_process_reverse(gf_irq_t *irq);
static gf_error_t irq_check_finger_long_press(gf_irq_t *irq);
static gf_error_t irq_enroll(gf_irq_t *irq);
static gf_error_t irq_authenticate(gf_irq_t *irq, uint32_t *switch_mode_flag);
static gf_error_t irq_test_frr_far_record_enroll(gf_irq_t *irq);
static gf_error_t irq_test_frr_far_record_authenticate(gf_irq_t *irq, uint32_t *switch_mode_flag);
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

void gf_irq_init_global_variable(void) {
    VOID_FUNC_ENTER();

    g_authenticate_fail_count = 0;
    g_study_enable_flag = 1;

    VOID_FUNC_EXIT();
}

static gf_error_t irq_process(gf_irq_t *irq) {
    gf_error_t err = GF_SUCCESS;
    uint32_t do_mode_switch_flag = 1;

    FUNC_ENTER();

    // irq process by generally
    if (irq->irq_type & GF_IRQ_FDT_REVERSE_MASK) {
        GF_LOGI(LOG_TAG "[%s] receive GF_IRQ_FDT_REVERSE", __func__);

        err = irq_process_reverse(irq);
    }

    if (irq->irq_type & GF_IRQ_RESET_MASK) {
        gf_irq_common_reset(irq->irq_type);
    }

    if (irq->irq_type & GF_IRQ_FINGER_DOWN_MASK) {
        g_image_count = 0;

        gf_base_on_down_irq();
    }

    if (irq->irq_type & GF_IRQ_IMAGE_MASK) {
        g_image_count++;

        if (!gf_state_is_authenticate(irq->operation)) {
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
            err = irq_authenticate(irq, &do_mode_switch_flag);
            if (gf_algo_get_preprocess_times() >= UPDATE_BASE_THRESHOLD
                    && gf_base_is_finger_base_vaild()) {
                gf_base_save_calibration();
            }
            break;

        case OPERATION_TEST_UNTRUSTED_AUTHENTICATE:
            err = irq_authenticate(irq, &do_mode_switch_flag);
            break;

        case OPERATION_TEST_FRR_FAR_RECORD_AUTHENTICATE:
            err = irq_test_frr_far_record_authenticate(irq, &do_mode_switch_flag);
            break;

        case OPERATION_FINGER_BASE:
        case OPERATION_TEST_FRR_FAR_RECORD_BASE_FRAME:
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
            gf_hw_clear_irq(irq->irq_type);
            err = gf_milan_test_sensor_validity();
            if (GF_SUCCESS == err) {
                GF_LOGD(LOG_TAG "[%s] sensor check pass", __func__);
                irq->test_sensor_validity.is_passed = 1;
            } else {
                GF_LOGD(LOG_TAG "[%s] sensor check failed", __func__);
                irq->test_sensor_validity.is_passed = 0;
            }
            err = gf_state_cancel_prior_test_operation(&irq->cmd_header.reset_flag);
            break;

        case OPERATION_CHECK_FINGER_LONG_PRESS:
            err = irq_check_finger_long_press(irq);
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

    if (1 == do_mode_switch_flag) {
        if (GF_SUCCESS != gf_milan_switch_mode()) {
            irq->cmd_header.reset_flag = 1;
            GF_LOGE(LOG_TAG "[%s] auto switch mode failed, so need to reset chip", __func__);
        }
    }

    FUNC_EXIT(err);

    return err;
}

static gf_error_t irq_process_reverse(gf_irq_t *irq) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    /* Del with reverse interrupt */
    if (0 == ((GF_IRQ_FINGER_DOWN_MASK | GF_IRQ_FINGER_UP_MASK) & irq->irq_type)) {
        GF_LOGI(LOG_TAG "[%s] receive FDT reverse IRQ", __func__);
        err = gf_base_on_fdt_reverse_irq(&irq->cmd_header.reset_flag);
    } else {
        /* clear the reverse interrupt when recive the mix interrupt */
        GF_LOGI(LOG_TAG "[%s] receive mix interrupt, firstly clear the reverse interrupt. ",
                __func__);
        irq->irq_type &= (~GF_IRQ_FDT_REVERSE_MASK);
    }

    FUNC_EXIT(err);

    return err;
}

static gf_error_t irq_check_finger_long_press(gf_irq_t *irq) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == irq) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad paramter. err=%s, errno=%d", __func__, gf_strerror(err), err);
            break;
        }

        GF_LOGD(LOG_TAG "[%s] irq_type=%s, irq_type_code=0x%x",
                __func__, gf_strirq(irq->irq_type), irq->irq_type);

        if (irq->irq_type & GF_IRQ_IMAGE_MASK) {
            err = gf_base_do_finger_long_press_check(g_image_data->raw_data,
                    g_image_data->raw_data_len, &irq->cmd_header.reset_flag);
            if (GF_SUCCESS == err) {
                GF_LOGI(LOG_TAG "[%s] valid finger long press", __func__);
                gf_milan_detect_finger_up();
            }
        }
    } while (0);

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

        if (irq->irq_type & GF_IRQ_IMAGE_MASK) {
            err = gf_state_cancel_test_operation(&irq->cmd_header.reset_flag);
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
                gf_milan_update_dac_for_sensor_pixel_open_test(0, 1);
                g_is_normal_cfg = 0;
                err = gf_state_execute_operation(OPERATION_TEST_PIXEL_OPEN_STEP2, NULL);
            } else if (OPERATION_TEST_PIXEL_OPEN_STEP2 == irq->operation) {
                irq->image_count = g_image_count;
                gf_pixel_test_step2(g_image_data->raw_data, g_image_data->raw_data_len);
                gf_pixel_test(&(irq->bad_pixel_num));
                err = gf_milan_update_dac_for_sensor_pixel_open_test(1, 0);
                g_is_normal_cfg = 1;
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
                // gf_ta_cancel_operation();
                // err = gf_state_execute_operation(gf_state_get_default_operation(),
                //            &irq->cmd_header.reset_flag);
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
            gf_hw_data_sample_resume();

            gf_nav_stop();
        }

        if (irq->irq_type & GF_IRQ_NAV_MASK) {
            err = gf_nav_detect(g_nav_data->raw_data[0], g_nav_data->raw_data_len,
                    irq->nav_result.finger_up_flag, g_nav_data->frame_num[0],
                    &irq->nav_result.nav_code);

            if (irq->nav_result.finger_up_flag > 0 || irq->nav_result.nav_code != GF_NAV_NONE) {
                gf_milan_nav_complete();
            }
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

            GF_GET_TIME_DELTA_X(g_dump_performance.get_raw_data_time);

            err = gf_base_on_image_irq(g_image_data->raw_data, g_image_data->raw_data_len,
                    &irq->cmd_header.reset_flag);
            if (err != GF_SUCCESS) {
                GF_LOGI(LOG_TAG "[%s] FDT INT caused by temperature", __func__);
                break;
            }

            if ((g_config.support_sensor_broken_check > 0)
                    && (irq->operation != OPERATION_TEST_UNTRUSTED_ENROLL)) {
                gf_broken_check_on_image_irq(g_image_data->raw_data, g_image_data->raw_data_len,
                        irq, irq->operation);
                GF_GET_TIME_DELTA_X(g_dump_performance.broken_check_time);
            }

            if (irq->broken_check.disable_sensor > 0) {
                err = GF_ERROR_SENSOR_IS_BROKEN;
                GF_LOGE(LOG_TAG "[%s] sensor is broken. err=%s, errno=%d", __func__,
                        gf_strerror(err), err);
                break;
            }

            err = gf_algo_get_acquired_info(g_image_data->raw_data, g_image_data->raw_data_len, 1,
                    NULL, irq->operation, &image_quality, &valid_area);
            GF_GET_TIME_DELTA_X(g_dump_performance.preprocess_time);

            if (err != GF_SUCCESS) {
                break;
            }

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
            gf_base_on_up_irq(&irq->cmd_header.reset_flag);
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t irq_authenticate(gf_irq_t *irq, uint32_t *switch_mode_flag) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == irq) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad paramter. err=%s, errno=%d", __func__, gf_strerror(err), err);
            break;
        }

        if (NULL == switch_mode_flag) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad paramter, switch_mode_flag is NULL.", __func__);
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
            gf_fdt_ret_t fdt_ret = GF_FDT_RET_TEMP;

            GF_GET_TIME_DELTA_X(g_dump_performance.get_raw_data_time);

            err = gf_base_on_image_irq(g_image_data->raw_data, g_image_data->raw_data_len,
                    &irq->cmd_header.reset_flag);
            if (err != GF_SUCCESS) {
                GF_LOGI(LOG_TAG "[%s] FDT INT caused by temperature", __func__);
                break;
            }

            gf_fdt_get_event_type(GF_FDT_IMAGE, g_image_data->raw_data, &fdt_ret);

            if (fdt_ret != GF_FDT_RET_FINGER) {
                uint32_t valid_touch_frame_threshold = 0;

                GF_LOGI(LOG_TAG "[%s] finger is up, so don't need to retry. fdt_ret=%d",
                        __func__, fdt_ret);

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
                } else if (g_best_quality_for_authenticate <
                        g_config.image_quality_threshold_for_mistake_touch) {
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
                break;
            }

            if ((g_config.support_sensor_broken_check > 0)
                    && (0 == g_authenticate_fail_count)
                    && (irq->operation != OPERATION_TEST_UNTRUSTED_AUTHENTICATE)) {
                gf_broken_check_on_image_irq(g_image_data->raw_data, g_image_data->raw_data_len,
                        irq, irq->operation);
                g_study_enable_flag = (!irq->broken_check.disable_study);
                GF_GET_TIME_DELTA_X(g_dump_performance.broken_check_time);
            }

            err = gf_algo_get_acquired_info(g_image_data->raw_data, g_image_data->raw_data_len, 1,
                    NULL, irq->operation, &image_quality, &valid_area);
            GF_GET_TIME_DELTA_X(g_dump_performance.preprocess_time);
            if (err != GF_SUCCESS) {
                break;
            }

            if (OPERATION_TEST_UNTRUSTED_AUTHENTICATE == irq->operation) {
                err = gf_algo_test_authenticate(1, &irq->group_id, &irq->finger_id,
                        &image_quality, &valid_area, &match_score);
            } else {
                err = gf_algo_authenticate(NULL, 0, g_study_enable_flag, &irq->group_id,
                        &irq->finger_id, &irq->save_flag, &irq->update_stitch_flag,
                        &image_quality, &valid_area, &match_score);
            }

            if ((uint32_t)image_quality > g_best_quality_for_authenticate) {
                g_best_quality_for_authenticate = (uint32_t)image_quality;
            }

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

                    *switch_mode_flag = 0;
                    err = GF_ERROR_MATCH_FAIL_AND_RETRY;

                    GF_LOGI(LOG_TAG "[%s] authenticate failed, need retry. "
                            "g_authenticate_fail_count=%u", __func__, g_authenticate_fail_count);
                    break;
                }
                gf_hw_data_sample_suspend();
                // touch by mistake
                if (g_best_quality_for_authenticate <
                        g_config.image_quality_threshold_for_mistake_touch) {
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
            gf_base_on_up_irq(&irq->cmd_header.reset_flag);
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t irq_test_frr_far_record_authenticate(gf_irq_t *irq, uint32_t *switch_mode_flag) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == irq) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad paramter. err=%s, errno=%d", __func__, gf_strerror(err), err);
            break;
        }

        if (NULL == switch_mode_flag) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad paramter, switch_mode_flag is NULL.", __func__);
            break;
        }

        GF_LOGD(LOG_TAG "[%s] irq_type=%s, irq_type_code=0x%08X",
                __func__, gf_strirq(irq->irq_type), irq->irq_type);

        if (irq->irq_type & GF_IRQ_FINGER_DOWN_MASK) {
            // do nothing
        }

        if (irq->irq_type & GF_IRQ_IMAGE_MASK) {
            err = gf_frr_far_authenticate_without_greedy_strategy(g_image_data->raw_data,
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

            if (cmd->irq_type & (GF_IRQ_NAV_MASK | GF_IRQ_IMAGE_MASK)) {
                if (OPERATION_TEST_INTERRUPT_PIN == cmd->operation) {
                    GF_LOGD(LOG_TAG "[%s] For interrupt pin test, chip mode is idle, and host set "
                            "image irq. So idle mode should return image irq", __func__);
                    gf_hw_clear_irq(irq_type);
                    cmd->step = GF_IRQ_STEP_PROCESS;
                } else {
                    cmd->step = GF_IRQ_STEP_PRE_GET_IMAGE;
                }
            } else {
                cmd->step = GF_IRQ_STEP_CLEAR_IRQ;
            }
        }

        if (GF_IRQ_STEP_PRE_GET_IMAGE == cmd->step) {
            GF_LOGD(LOG_TAG "[%s] GF_IRQ_STEP_PRE_GET_IMAGE", __func__);

            if (OPERATION_NAV == cmd->operation) {
                cpl_memset(g_nav_data, 0, sizeof(gf_nav_data_t));
                err = gf_milan_pre_get_nav_image();
            } else if (OPERATION_NAV_BASE == cmd->operation) {
                cpl_memset(g_nav_data, 0, sizeof(gf_nav_data_t));
                err = gf_milan_pre_get_nav_base();
            } else {
                gf_get_time_delta();
                cpl_memset(g_image_data, 0, sizeof(gf_image_data_t));
                err = gf_milan_pre_get_image();
            }

            if (err != GF_SUCCESS) {
                break;
            }

            cmd->step = GF_IRQ_STEP_GET_IMAGE;
            if (0 == g_config.support_set_spi_speed_in_tee) {
                gf_milan_get_spi_speed(cmd->irq_type, &cmd->speed);
                break;
            }
        }

        if (GF_IRQ_STEP_GET_IMAGE == cmd->step) {
            GF_LOGD(LOG_TAG "[%s] GF_IRQ_STEP_GET_IMAGE", __func__);

            if (OPERATION_NAV == cmd->operation) {
                err = gf_milan_get_nav_image(g_nav_origin_data->data[0],
                        &g_nav_origin_data->data_len);
            } else if (OPERATION_NAV_BASE == cmd->operation) {
                err = gf_milan_get_nav_base(g_nav_origin_data->data[0],
                        &g_nav_origin_data->data_len);
            } else {
                err = gf_milan_get_image(g_image_origin_data->data,
                        &g_image_origin_data->data_len);
            }

            if (err != GF_SUCCESS) {
                break;
            }

            cmd->step = GF_IRQ_STEP_POST_GET_IMAGE;
            if (0 == g_config.support_set_spi_speed_in_tee) {
                cmd->speed = GF_SPI_SPEED_LOW;
                break;
            }
        }

        if (GF_IRQ_STEP_POST_GET_IMAGE == cmd->step) {
            GF_LOGD(LOG_TAG "[%s] GF_IRQ_STEP_POST_GET_IMAGE", __func__);

            if (OPERATION_NAV == cmd->operation) {
                err = gf_milan_post_get_nav_image(g_nav_data->raw_data[0],
                        &g_nav_data->raw_data_len, &g_nav_data->frame_num[0],
                        &cmd->nav_result.finger_up_flag);
                if (err != GF_SUCCESS) {
                    break;
                }
                g_nav_data->nav_frame_count = 1;

            } else if (OPERATION_NAV_BASE == cmd->operation) {
                err = gf_milan_post_get_nav_base(g_nav_data->raw_data[0],
                        &g_nav_data->raw_data_len);
                if (err != GF_SUCCESS) {
                    break;
                }

            } else {
                err = gf_milan_post_get_image(g_image_data->raw_data,
                        &g_image_data->raw_data_len);
                if (err != GF_SUCCESS) {
                    break;
                }
            }

            cmd->step = GF_IRQ_STEP_CLEAR_IRQ;
        }

        if (GF_IRQ_STEP_CLEAR_IRQ == cmd->step) {
            GF_LOGD(LOG_TAG "[%s] GF_IRQ_STEP_CLEAR_IRQ", __func__);

            // ingore irq, only for test irq report times & interval, this test won't clear irq
            if (0 == (cmd->irq_type & g_ignore_irq_mask)) {
                gf_hw_clear_irq(cmd->irq_type);
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
            gf_hw_clear_irq(irq_type);
        }
        cmd->step = GF_IRQ_STEP_IDLE;
        cmd->speed = GF_SPI_SPEED_LOW;

        if (cmd->irq_type & (GF_IRQ_NAV_MASK | GF_IRQ_IMAGE_MASK)) {
            gf_hw_data_sample_suspend();
            cmd->irq_type &= (~(GF_IRQ_NAV_MASK | GF_IRQ_IMAGE_MASK));
        }

        gf_milan_switch_mode();

        if (gf_state_is_base_operation(cmd->operation)) {
            gf_state_operation_done(&cmd->cmd_header.reset_flag);
        }
    }

    FUNC_EXIT(err);

    return err;
}
