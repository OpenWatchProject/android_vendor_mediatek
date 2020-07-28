/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#include "gf_irq.h"

#include <stdint.h>
#include "gf_common.h"
#include "gf_error.h"
#include "gf_spi.h"
#include "gf_tee_internal_api.h"
#include "gf_log.h"
#include "gf_algo.h"
#include "gf_hbd_test.h"
#include "gf_heart_beat.h"
#include "gf_base.h"
#include "cpl_string.h"
#include "gf_sensor.h"
#include "gf_utils.h"
#include "gf_hw_common.h"
#include "gf_state_common.h"
#include "gf_irq_common.h"
#include "gf_nav.h"
#include "gf_bad_point_test.h"
#include "gf_fido.h"
#include "gf_config.h"
#include "gf_state.h"
#include "gf_milan_a_series_common.h"
#include "gf_milan_a_series.h"

#define LOG_TAG "[gf_irq]"

// below should be refactor
static uint32_t g_best_quality_for_authenticate = 0;
static uint8_t g_report_authenticate_result_flag = 0;

extern uint32_t g_image_count;

extern gf_test_performance_t g_dump_performance;

extern uint32_t g_ignore_irq_mask;
extern uint8_t g_is_normal_cfg;
extern uint64_t g_operation_id;
extern uint64_t g_user_id;
extern uint64_t g_authenticator_id;
extern gf_authenticate_fido_t g_authenticator_fido;

extern gf_image_data_t *g_image_data;
extern gf_nav_data_t *g_nav_data;
extern gf_hbd_data_t *g_hbd_data;

extern gf_image_origin_data_t *g_image_origin_data;
extern gf_nav_origin_data_t *g_nav_origin_data;

extern gf_base_data_t *g_base_data;
extern uint8_t g_last_authenticate_result;

static uint32_t g_authenticate_fail_count = 0;
static uint32_t g_study_enable_flag = 1;

static gf_error_t get_live_data(int32_t* data, uint32_t data_len);
static void dump_live_data(void);
static gf_error_t irq_hbd(gf_irq_t *irq);
static gf_error_t irq_enroll(gf_irq_t *irq);
static gf_error_t irq_authenticate(gf_irq_t *irq);
static gf_error_t irq_test_frr_far_record_enroll(gf_irq_t *irq);
static gf_error_t irq_test_frr_far_record_authenticate(gf_irq_t *irq);
static gf_error_t irq_finger_base(gf_irq_t *irq);
static gf_error_t irq_key(gf_irq_t *irq);
static gf_error_t irq_nav_base(gf_irq_t *irq);
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

    FUNC_ENTER();

    // irq process by generally
    if (irq->irq_type & (GF_IRQ_FW_ERR_MASK | GF_IRQ_CFG_ERR_MASK | GF_IRQ_ESD_ERR_MASK)) {
        GF_LOGI(LOG_TAG "[%s] fw,cfg,esd error, ready reset chip", __func__);

        irq->cmd_header.reset_flag = 1;
    }

    if (irq->irq_type & GF_IRQ_UPDATE_BASE_MASK) {
        GF_LOGI(LOG_TAG "[%s] temperature changed, ready update base", __func__);

        err = gf_base_on_temperature_change_irq(&irq->cmd_header.reset_flag);
    }

    if (irq->irq_type & GF_IRQ_RESET_MASK) {
        gf_milan_a_series_update_temprature_dac();
        gf_irq_common_reset(irq->irq_type);
    }

    if (irq->irq_type & GF_IRQ_FINGER_DOWN_MASK) {
        g_image_count = 0;
    }

    if (irq->irq_type & GF_IRQ_IMAGE_MASK) {
        g_image_count++;
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

        case OPERATION_HEARTBEAT_KEY:
        case OPERATION_TEST_BIO_CALIBRATION:
        case OPERATION_TEST_HBD_CALIBRATION:
            err = irq_hbd(irq);
            break;

        case OPERATION_HOME_KEY:
        case OPERATION_CAMERA_KEY:
        case OPERATION_POWER_KEY:
            err = irq_key(irq);
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

static gf_error_t get_live_data(int32_t* data, uint32_t data_len) {
    gf_error_t err = GF_SUCCESS;
    int32_t i = 0;
    uint32_t offset = 0;

    FUNC_ENTER();

    do {
        uint8_t* tmp = NULL;
        if (data == NULL || data_len < GF_LIVE_DATA_LEN) {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        tmp = g_image_data->gsc_untouch_data;
        data[offset++] = *((uint16_t*) ((void*) tmp));
        for (i = GSC_UNTOUCH_LEN - 1; i >= 0; i--) {
            data[offset++] = *((uint16_t*) ((void*) tmp) + i + 1);
        }

        tmp = g_image_data->gsc_touch_data;
        data[offset++] = *((uint16_t*) ((void*) tmp));
        for (i = GSC_TOUCH_LEN - 1; i >= 0; i--) {
            data[offset++] = *((uint16_t*) ((void*) tmp) + i + 1);
        }

        data[offset++] = g_image_data->gsc_base;

        // modify untouch_data last 2 bytes
        offset = 1 + GSC_UNTOUCH_LEN - 2;
        data[offset++] = g_milan_a_series_fw_cfg.hbd_version;
        data[offset++] = g_milan_a_series_fw_cfg.hbd_otp_base;
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static void dump_live_data(void) {
    uint32_t i = 0;
    uint16_t untouch_data[GSC_UNTOUCH_LEN + 1] = { 0 };
    uint16_t touch_data[GSC_TOUCH_LEN + 1] = { 0 };

    VOID_FUNC_ENTER();

    // +1 for bio assay data length.
    cpl_memcpy(untouch_data, g_image_data->gsc_untouch_data,
            (GSC_UNTOUCH_LEN + 1) * sizeof(uint16_t));
    cpl_memcpy(touch_data, g_image_data->gsc_touch_data, (GSC_TOUCH_LEN + 1) * sizeof(uint16_t));

    GF_LOGD(LOG_TAG "[%s] unTouchLen=%u, touchLen=%u", __func__, untouch_data[0], touch_data[0]);
    for (i = 0; i < untouch_data[0]; i++) {
        GF_LOGI(LOG_TAG "[%s] unTouchData[%u]=%u", __func__, i, untouch_data[i]);
    }

    for (i = 0; i < touch_data[0]; i++) {
        GF_LOGI(LOG_TAG "[%s] TouchData[%u]=%u", __func__, i, touch_data[i]);
    }

    GF_LOGI(LOG_TAG "[%s] gsc_base=%u", __func__, g_image_data->gsc_base);

    VOID_FUNC_EXIT();
}

static gf_error_t irq_hbd(gf_irq_t *irq) {
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
            gf_heart_beat_init();
        }

        if (irq->irq_type & GF_IRQ_HBD_MASK) {
            if (g_config.support_bio_assay > 0) {
                switch (irq->operation) {
                    case OPERATION_HEARTBEAT_KEY:
                        err = gf_heart_beat(g_hbd_data->hbd_data,
                                HBD_BUFFER_LEN, &irq->heart_beat);
                        if (err != GF_SUCCESS) {
                            GF_LOGE(LOG_TAG "[%s] gf_heart_beat failed", __func__);
                            irq->irq_type &= (~GF_IRQ_HBD_MASK);
                        }
                        cpl_memcpy(irq->heart_beat.raw_data, g_hbd_data->hbd_data,
                                HBD_BUFFER_LEN);
                        break;

                    case OPERATION_TEST_BIO_CALIBRATION:

                        err = gf_milan_a_series_get_hbd_base(&irq->test_hdb_feature.hbd_base);
                        if (GF_SUCCESS != err) {
                            GF_LOGE(LOG_TAG "[%s] get hdb base fail", __func__);
                            break;
                        }

                        err = gf_calutate_hbd_avg(&irq->test_hdb_feature.hbd_avg,
                                g_hbd_data->hbd_data,
                                HBD_BUFFER_LEN);
                        if (GF_SUCCESS != err) {
                            GF_LOGE(LOG_TAG "[%s] gf_calutate_hbd_avg", __func__);
                            break;
                        }

                        err = gf_state_execute_operation(OPERATION_TEST_IDLE_MODE,
                                &irq->cmd_header.reset_flag);

                        cpl_memcpy(&irq->test_hdb_feature.hdb_data, g_hbd_data->hbd_data,
                                HBD_BUFFER_LEN);
                        irq->test_hdb_feature.hbd_data_len = HBD_BUFFER_LEN;
                        break;

                    case OPERATION_TEST_HBD_CALIBRATION:

                        err = gf_milan_a_series_get_hbd_base(&irq->test_hdb_feature.hbd_base);
                        if (GF_SUCCESS != err) {
                            GF_LOGE(LOG_TAG "[%s] get hdb base fail", __func__);
                            break;
                        }

                        err = gf_calutate_hbd_avg(&irq->test_hdb_feature.hbd_avg,
                                g_hbd_data->hbd_data,
                                g_hbd_data->hbd_data_len);
                        if (GF_SUCCESS != err || g_hbd_data->hbd_data_len < 24) {
                            /* electricity_value at 23th & 24th byte*/
                            GF_LOGE(LOG_TAG "[%s] gf_calutate_hbd_avg", __func__);
                            break;
                        }

                        irq->test_hdb_feature.electricity_value = (g_hbd_data->hbd_data[23] << 8)
                                | g_hbd_data->hbd_data[22];

                        gf_milan_a_series_download_cfg(CONFIG_NORMAL);
                        g_is_normal_cfg = 1;
                        err = gf_state_execute_operation(OPERATION_TEST_IDLE_MODE,
                                &irq->cmd_header.reset_flag);

                        cpl_memcpy(&irq->test_hdb_feature.hdb_data, g_hbd_data->hbd_data,
                                HBD_BUFFER_LEN);
                        irq->test_hdb_feature.hbd_data_len = HBD_BUFFER_LEN;
                        break;

                    default:
                        err = GF_ERROR_GENERIC;
                        GF_LOGE(LOG_TAG "[%s] in HBD irq no such a operation=%s, operation_code=%d",
                                __func__, gf_stroperation(irq->operation), irq->operation);
                        break;
                }
            } else {
                GF_LOGI(LOG_TAG "[%s] irq hbd, but flag support_bio_assay=%u.", __func__,
                        g_config.support_bio_assay);
                irq->irq_type &= (~GF_IRQ_HBD_MASK);
            }
        }

        if (irq->irq_type & GF_IRQ_FINGER_UP_MASK) {
            // do nothing
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
            uint8_t fw_version[10] = { 0 };
            uint32_t offset = 7;

            irq->test_spi_performance.get_image_time = gf_get_time_delta();
            irq->test_spi_performance.raw_data_len = g_image_origin_data->data_len;

            gf_get_time_delta();
            irq->test_spi_performance.get_dr_timestamp_time = gf_get_time_delta();

            err = gf_hw_get_mode(&test_mode);
            irq->test_spi_performance.get_mode_time = gf_get_time_delta();

            err = gf_milan_a_series_get_fw_version(fw_version, 10);
            irq->test_spi_performance.get_fw_version_time = gf_get_time_delta();

            cpl_memcpy(irq->test_spi_performance.fw_version, fw_version, 7);

            irq->test_spi_performance.fw_version[offset++] = '0' + (fw_version[7] & 0x0f);
            irq->test_spi_performance.fw_version[offset++] = '.';
            irq->test_spi_performance.fw_version[offset++] = '0' + ((fw_version[8] >> 4) & 0x0f);
            irq->test_spi_performance.fw_version[offset++] = '0' + (fw_version[8] & 0x0f);
            irq->test_spi_performance.fw_version[offset++] = '.';
            irq->test_spi_performance.fw_version[offset++] = '0' + ((fw_version[9] >> 4) & 0x0f);
            irq->test_spi_performance.fw_version[offset++] = '0' + (fw_version[9] & 0x0f);
            irq->test_spi_performance.fw_version[offset++] = '\0';

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
            err = gf_hw_read_register(0x0AA0, irq->test_real_time_data.fpc_key_data,
                    FPC_KEY_DATA_LEN);
            if (err != GF_SUCCESS) {
                GF_LOGE(LOG_TAG "[%s] fpc key data read failed. err=%s, errno=%d",
                        __func__, gf_strerror(err), err);
                break;
            }

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
            // TODO(goodix):
        }

        if (irq->irq_type & GF_IRQ_GSC_MASK) {
            // do nothing
        }

        if (irq->irq_type & GF_IRQ_IMAGE_MASK) {
            irq->test_performance.get_raw_data_time = gf_get_time_delta();
            if (g_config.support_bio_assay > 0) {
                GF_LOGE(LOG_TAG "[%s] OPERATION_TEST_PERFORMANCE, but gf_bio_assay err.", __func__);
                err = GF_ERROR_BIO_ASSAY_FAIL;
            } else {
                err = gf_algo_test_performance(g_image_data->raw_data, g_image_data->raw_data_len,
                        &irq->test_performance);
            }
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
                gf_milan_a_series_update_dac_for_sensor_pixel_open_test(0, 1);
                g_is_normal_cfg = 0;
                err = gf_state_execute_operation(OPERATION_TEST_PIXEL_OPEN_STEP2, NULL);
            } else if (OPERATION_TEST_PIXEL_OPEN_STEP2 == irq->operation) {
                irq->image_count = g_image_count;
                gf_pixel_test_step2(g_image_data->raw_data, g_image_data->raw_data_len);
                gf_pixel_test(&(irq->bad_pixel_num));
                err = gf_milan_a_series_update_dac_for_sensor_pixel_open_test(1, 0);
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
            // TODO(goodix):
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
            gf_nav_code_t nav_code = GF_NAV_NONE;
            if (g_config.require_down_and_up_in_pairs_for_nav_mode > 0) {
                gf_state_require_down_and_up_in_pairs_begin();
            }
            /*if (GF_NAV_EVENT_DETECT_METHOD_POLLING == g_config.support_nav_get_data_method) {
                gf_hw_data_sample_resume();
            }*/
            gf_nav_detect(NULL, 0, 1, 0, &nav_code);

            // gf_nav_stop();
        }

        if (irq->irq_type & GF_IRQ_NAV_MASK) {
            gf_hw_data_sample_suspend();
        }

        if (irq->irq_type & GF_IRQ_FINGER_UP_MASK) {
            gf_hw_data_sample_resume();
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
            GF_LOGE(LOG_TAG "[%s] bad paramter, irq is NULL.", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        GF_LOGD(LOG_TAG "[%s] irq_type=%s, irq_type_code=0x%x",
                __func__, gf_strirq(irq->irq_type), irq->irq_type);

        if (irq->irq_type & GF_IRQ_NAV_MASK) {
            err = gf_base_update_nav_base(g_nav_data->raw_data[0], g_nav_data->raw_data_len,
                    &irq->cmd_header.reset_flag);
        } else if (irq->irq_type & GF_IRQ_FINGER_DOWN_MASK) {
            gf_hw_data_sample_suspend();
            gf_state_operation_done(&irq->cmd_header.reset_flag);
        } else if (irq->irq_type & GF_IRQ_IMAGE_MASK) {
            gf_hw_data_sample_resume();
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
        if ((irq->irq_type & GF_IRQ_FINGER_DOWN_MASK)
            || (irq->irq_type & GF_IRQ_FINGER_UP_MASK)
            || (irq->irq_type & GF_IRQ_UPDATE_BASE_MASK)
            || (irq->irq_type & GF_IRQ_FDT_REVERSE_MASK))
        {
            if (OPERATION_FINGER_BASE == irq->operation)
            {
                gf_hw_data_sample_resume();
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
            gf_hw_data_sample_resume();
        }

        if (irq->irq_type & GF_IRQ_GSC_MASK) {
            // do nothing
        }

        if (irq->irq_type & GF_IRQ_IMAGE_MASK) {
            int32_t image_quality = 0;
            int32_t valid_area = 0;
            int32_t live_data[GF_LIVE_DATA_LEN] = { 0 };

            GF_GET_TIME_DELTA_X(g_dump_performance.get_raw_data_time);

            err = gf_algo_get_acquired_info(g_image_data->raw_data, g_image_data->raw_data_len, 1,
                    NULL,
                    irq->operation, &image_quality, &valid_area);
            GF_GET_TIME_DELTA_X(g_dump_performance.preprocess_time);
            if (err != GF_SUCCESS) {
                int32_t raw_data_mean = gf_algo_get_raw_data_mean(g_image_data->raw_data,
                        g_sensor.row, g_sensor.col);
                if (raw_data_mean < 1000) {
                    irq->cmd_header.reset_flag = 1;
                }
                // consider base is invalid if preprocess fail count >= 5
                if (GF_ERROR_PREPROCESS_FAILED == err
                        && (gf_algo_get_preprocess_fail_times() >= 5)) {
                    gf_base_reset_base_status();
                }
                break;
            }

            err = gf_milan_a_series_judge_finger_stable(0);
            if (err != GF_SUCCESS) {
                g_image_count = 0;
                gf_algo_backup_kr();
                GF_LOGE(LOG_TAG "[%s]Milan_a_series judge stable failed", __func__);
                break;
            }

            err = get_live_data(live_data, GF_LIVE_DATA_LEN);
            if (err != GF_SUCCESS) {
                break;
            }

            irq->samples_remaining = g_config.enrolling_min_templates;
            if (OPERATION_ENROLL == irq->operation) {
                err = gf_algo_enroll(live_data, GF_LIVE_DATA_LEN, &irq->group_id, &irq->finger_id,
                        &irq->samples_remaining, &irq->duplicate_finger_id);
            } else {
                // TODO(goodix): test enroll need check live data
                err = gf_algo_test_enroll(&irq->group_id, &irq->finger_id, &irq->samples_remaining,
                        &irq->duplicate_finger_id);
            }
            if (GF_ERROR_BIO_ASSAY_FAIL == err) {
                dump_live_data();
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
            g_report_authenticate_result_flag = 0;
            g_study_enable_flag = 1;

            if (((OPERATION_AUTHENTICATE_IMAGE == irq->operation)
                    && (g_config.require_down_and_up_in_pairs_for_image_mode > 0))
                    || ((OPERATION_AUTHENTICATE_FF == irq->operation)
                            && (g_config.require_down_and_up_in_pairs_for_ff_mode > 0))) {
                gf_state_require_down_and_up_in_pairs_begin();
            }
            gf_hw_data_sample_resume();
        }

        if (irq->irq_type & GF_IRQ_IMAGE_MASK) {
            uint64_t timestamp = 0;
            int32_t image_quality = 0;
            int32_t valid_area = 0;
            int32_t match_score = 0;
            int32_t live_data[GF_LIVE_DATA_LEN] = { 0 };

            GF_GET_TIME_DELTA_X(g_dump_performance.get_raw_data_time);

            err = gf_algo_get_acquired_info(g_image_data->raw_data, g_image_data->raw_data_len, 1,
                    NULL, irq->operation, &image_quality, &valid_area);
            GF_GET_TIME_DELTA_X(g_dump_performance.preprocess_time);

            if (err != GF_SUCCESS) {
                int32_t raw_data_mean = gf_algo_get_raw_data_mean(g_image_data->raw_data,
                        g_sensor.row, g_sensor.col);
                if (raw_data_mean < 1000) {
                    irq->cmd_header.reset_flag = 1;
                }
                // consider base is invalid if preprocess fail count >= 5
                if (GF_ERROR_PREPROCESS_FAILED == err
                        && (gf_algo_get_preprocess_fail_times() >= 5)) {
                    //  gf_base_reset_base_status();
                }
                break;
            }

            err = gf_milan_a_series_judge_finger_stable(0);
            if (err != GF_SUCCESS) {
                g_study_enable_flag = 0;
                gf_algo_backup_kr();
                GF_LOGE(LOG_TAG "[%s]Milan_a_series judge stable failed", __func__);
            }

            /*err = get_live_data(live_data, GF_LIVE_DATA_LEN);
            if (err != GF_SUCCESS) {
                break;
            }*/

            if (OPERATION_TEST_UNTRUSTED_AUTHENTICATE == irq->operation) {
                err = gf_algo_test_authenticate(1, &irq->group_id, &irq->finger_id,
                        &image_quality, &valid_area, &match_score);
            } else {
                err = gf_algo_authenticate(live_data, GF_LIVE_DATA_LEN, g_study_enable_flag,
                        &irq->group_id, &irq->finger_id, &irq->save_flag, &irq->update_stitch_flag,
                        &image_quality, &valid_area, &match_score);
            }

            if ((uint32_t) image_quality > g_best_quality_for_authenticate) {
                g_best_quality_for_authenticate = (uint32_t) image_quality;
            }

            if (GF_ERROR_BIO_ASSAY_FAIL == err) {
                dump_live_data();
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
                    err = GF_ERROR_MATCH_FAIL_AND_RETRY;
                    GF_LOGI(LOG_TAG
                            "[%s] authenticate failed, need retry. g_authenticate_fail_count=%u",
                            __func__, g_authenticate_fail_count);
                    break;
                } else {
                    gf_hw_data_sample_suspend();

                    g_report_authenticate_result_flag = 1;

                    // touch by mistake
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
            g_report_authenticate_result_flag = 1;

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

            if (0 == g_report_authenticate_result_flag) {
                uint32_t valid_touch_frame_threshold = 0;
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
                    GF_LOGI(LOG_TAG "[%s] touch by mistake, g_best_quality_for_authenticate=%u",
                            __func__, g_best_quality_for_authenticate);
                    irq->mistake_touch_flag = 1;
                } else {
                    irq->report_authenticate_fail_flag = 1;
                }
                // authenticate failed
                gf_sync_auth_result_notify(irq->group_id, irq->finger_id, g_operation_id, err);
                if (GF_PERFORMANCE_DUMP_ENABLED) {
                    cpl_memcpy(&irq->dump_performance, &g_dump_performance,
                            sizeof(gf_test_performance_t));
                }
                GF_LOGI(LOG_TAG "[%s] authenticate failed", __func__);
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
            // TODO(goodix):
        }

        if (irq->irq_type & GF_IRQ_GSC_MASK) {
            // do nothing
        }

        if (irq->irq_type & GF_IRQ_IMAGE_MASK) {
            err = get_live_data(irq->test_frr_far.gsc_data, GF_LIVE_DATA_LEN);
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
            // TODO(goodix):
        }

        if (irq->irq_type & GF_IRQ_GSC_MASK) {
            // do nothing
        }

        if (irq->irq_type & GF_IRQ_IMAGE_MASK) {
            get_live_data(irq->test_frr_far.gsc_data, GF_LIVE_DATA_LEN);
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

    if (NULL == cmd) {
        err = GF_ERROR_BAD_PARAMS;
        FUNC_EXIT(err);
        return err;
    }

    GF_LOGD(LOG_TAG "[%s] step=%d", __func__, cmd->step);

    cmd->operation = gf_state_get_current_operation();
    cmd->mode = gf_state_get_current_mode();

    do {
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
            } while (polling_count < 100);

            if (err != GF_SUCCESS || 100 == polling_count) {
                break;
            }

            GF_LOGD(LOG_TAG "[%s] get_irq_type, irq_type=%s, irq_type=0x%08X",
                    __func__, gf_strirq(irq_type), irq_type);
            cmd->irq_type = irq_type;

            if (cmd->irq_type & GF_IRQ_HBD_MASK) {
                err = gf_milan_a_series_get_hbd(g_hbd_data->hbd_data, &g_hbd_data->hbd_data_len);
                if (err != GF_SUCCESS) {
                    cmd->irq_type &= (~GF_IRQ_HBD_MASK);
                    break;
                }
            }

            if (cmd->irq_type & (GF_IRQ_NAV_MASK | GF_IRQ_IMAGE_MASK)) {
                if (OPERATION_TEST_INTERRUPT_PIN == cmd->operation) {
                    /*
                     * For interrupt pin test, chip mode is debug.
                     * So debug mode should return image irq
                     */
                    GF_LOGD(LOG_TAG "[%s] debug mode should return image irq", __func__);
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
                // TODO(goodix):
            } else if ((OPERATION_NAV_BASE == cmd->operation)
                    && (cmd->irq_type & GF_IRQ_NAV_MASK)) {
                cpl_memset(g_nav_data, 0, sizeof(gf_nav_data_t));
            } else {
                gf_get_time_delta();
                // remove cpl_memset since it erases gsc data when doing enroll and authen
                // cpl_memset(g_image_data, 0, sizeof(gf_image_data_t));
                err = gf_milan_a_series_pre_get_image();
            }

            if (err != GF_SUCCESS) {
                break;
            }

            cmd->step = GF_IRQ_STEP_GET_IMAGE;
            if (0 == g_config.support_set_spi_speed_in_tee) {
                gf_milan_a_series_get_spi_speed(cmd->irq_type, &cmd->speed);
                break;
            }
        }

        if (GF_IRQ_STEP_GET_IMAGE == cmd->step) {
            GF_LOGD(LOG_TAG "[%s] GF_IRQ_STEP_GET_IMAGE", __func__);

            if (OPERATION_NAV == cmd->operation) {
                // TODO(goodix):
            } else if (OPERATION_NAV_BASE == cmd->operation && (cmd->irq_type & GF_IRQ_NAV_MASK)) {
                err = gf_milan_a_series_get_nav_base(g_nav_origin_data->data[0],
                        &g_nav_origin_data->data_len);
            } else {
                err = gf_milan_a_series_get_image(g_image_origin_data->data,
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
                // TODO(goodix):
            } else if (OPERATION_NAV_BASE == cmd->operation && (cmd->irq_type & GF_IRQ_NAV_MASK)) {
                err = gf_milan_a_series_post_get_nav_base(g_nav_data->raw_data[0],
                        &g_nav_data->raw_data_len);
                gf_hw_data_sample_suspend();
                if (err != GF_SUCCESS) {
                    break;
                }
            } else {
                if (MODE_FINGER_BASE == cmd->mode) {
                    gf_base_pre_get_image();
                }

                err = gf_milan_a_series_post_get_image(g_image_data->raw_data,
                        &g_image_data->raw_data_len);
                if (err != GF_SUCCESS) {
                    break;
                }
                /*gsc irq and image irq merged,read gsc data after get image*/
                if ((MODE_IMAGE == cmd->mode || MODE_FF == cmd->mode)
                        && g_config.support_bio_assay > 0
                        && g_config.support_merged_gsc > 0) {
                    err = gf_milan_a_series_get_gsc(g_image_data->gsc_untouch_data,
                            &g_image_data->gsc_untouch_data_len, g_image_data->gsc_touch_data,
                            &g_image_data->gsc_touch_data_len, &g_image_data->gsc_base);
                    if (err != GF_SUCCESS) {
                        break;
                    }
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

        if (cmd->irq_type & GF_IRQ_NAV_MASK) {
            cmd->irq_type &= (~GF_IRQ_NAV_MASK);
        }

        if (gf_state_is_base_operation(cmd->operation)) {
            gf_state_operation_done(&cmd->cmd_header.reset_flag);
        }
    }

    FUNC_EXIT(err);

    return err;
}
