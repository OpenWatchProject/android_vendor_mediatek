/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/


#include <stdlib.h>
#include "string.h"
#include "gf_error.h"
#include "gf_common.h"
#include "gf_user_type_define.h"
#include "gf_algo.h"
#include "gf_secure_object.h"
#include "cpl_string.h"
#include "gf_tee_internal_api.h"
#include "gf_spi_common.h"
#include "gf_ta.h"
#include "gf_sensor.h"
#include "gf_utils.h"
#include "gf_status.h"
#include "cpl_memory.h"
#include "cpl_stdio.h"
#include "cpl_math.h"
#include "cpl_string.h"
#include "gf_bio_assay.h"
#include "gf_heart_beat.h"
#include "gf_algo_crc32.h"
#include "gf_hbd_test.h"
#include "gf_base.h"
#include "gf_fdt.h"
#include "gf_broken.h"
#include "gf_frr_far_encoder.h"

#ifdef GF_MEMORY_DEBUG_ENABLE
#include "mem_manager.h"

#define GF_MEMORY_MANAGER_TOTAL_SIZE    (3 * 1024 * 1024)
void *g_ta_memory_start = NULL;
#endif

#define ENDIAN_SWAP_U64(val) ((uint64_t) (\
    (((uint64_t) (val) & (uint64_t) 0x00000000000000ff) << 56) | \
    (((uint64_t) (val) & (uint64_t) 0x000000000000ff00) << 40) | \
    (((uint64_t) (val) & (uint64_t) 0x0000000000ff0000) << 24) | \
    (((uint64_t) (val) & (uint64_t) 0x00000000ff000000) << 8) |  \
    (((uint64_t) (val) & (uint64_t) 0x000000ff00000000) >> 8) |  \
    (((uint64_t) (val) & (uint64_t) 0x0000ff0000000000) >> 24) | \
    (((uint64_t) (val) & (uint64_t) 0x00ff000000000000) >> 40) | \
    (((uint64_t) (val) & (uint64_t) 0xff00000000000000) >> 56)))
#define LOG_TAG "[gf_ta] "
#define SENSOR_BROKEN_CHECK_FLOW_VERSION "v1.0"
#define GF_FDT_BASE_DATA_LEN    (28)    // CRC32(4 bytes) + 12 blocks * sizeof(uint16_t)
#define GSC_RAW_DATA_BUFFER_LEN (24)
#define MILAN_GET_RESET_IRQ_MAX_COUNT (20)
static gf_mode_t g_cur_mode = MODE_NONE;
static gf_mode_t g_next_mode = MODE_NONE;

gf_operation_type_t g_operation = OPERATION_NONE;
static gf_operation_type_t g_next_operation = OPERATION_NONE;
gf_spi_function_t g_spi_function;

static uint8_t g_finger_pressed_flag = 0;
static uint8_t g_authenticate_is_cancel = 0;

static uint32_t g_image_count = 0;
static uint32_t g_authenticate_fail_count = 0;
static uint32_t g_retry_flag = 0;
static uint32_t g_bad_point_test_image_count = 0;

static uint32_t g_test_esd_exception_count = 0;
static uint32_t g_esd_exception_count = 0;
static int32_t g_screen_on_flag = 1;

static uint32_t *g_finger_index_hash_table = NULL;

static uint8_t g_set_active_group_flag = 0;

static uint16_t *g_fp_raw_data = NULL;
static uint32_t g_fp_raw_data_len = 0;
static uint8_t *g_origin_raw_data = NULL;
static uint32_t g_origin_raw_data_len = 0;

static uint32_t g_study_enable_flag = 1;

static uint64_t g_operation_id = 0;
static uint64_t g_authenticator_id = 0xdeadbeef;
static uint64_t g_user_id = 0;
static uint64_t g_challenge = 0;
static uint32_t g_sensor_engineer_available = 1;

gf_test_performance_t g_dump_performance = { 0 };
uint8_t g_sensor_otp_info[GF_SENSOR_OTP_INFO_LEN] = { 0 };

static uint32_t g_ignore_irq_mask = 0;

static uint8_t g_is_normal_cfg = 1;

extern gf_test_frr_far_finger_list_t g_frr_far_finger_list;

static gf_error_t gf_ta_test_get_version(gf_test_get_version_t *cmd);
static void gf_ta_cancel_operation(void);
static gf_error_t gf_ta_switch_fingerprint_user(uint32_t group_id);

gf_error_t gf_ta_pre_set_mode(gf_mode_t mode, gf_operation_type_t operation,
        uint8_t *reset_flag);
gf_error_t gf_ta_set_next_mode(uint8_t *reset_flag);
static gf_error_t gf_ta_set_mode(gf_mode_t mode, gf_operation_type_t operation);
static gf_error_t gf_ta_set_normal_mode(uint8_t *reset_flag);

gf_error_t gf_ta_pre_enroll(gf_pre_enroll_t *cmd);
gf_error_t gf_ta_enroll(gf_enroll_t *cmd);
gf_error_t gf_ta_post_enroll(void);
gf_error_t gf_ta_cancel(gf_cancel_t *cmd);
gf_error_t gf_ta_authenticate(gf_authenticate_t *cmd);
gf_error_t gf_ta_get_auth_id(gf_get_auth_id_t *cmd);
gf_error_t gf_ta_remove(gf_remove_t *cmd);
gf_error_t gf_ta_set_active_group(gf_set_active_group_t *cmd);
gf_error_t gf_ta_enumerate(gf_enumerate_t *cmd);
static gf_error_t gf_ta_image_irq_algo(gf_irq_t *cmd);

static void gf_ta_dump_mode(const char *func_name, gf_mode_t mode) {
    char mode_str[][64] = { /**/
    "MODE_IMAGE", /**/
    "MODE_KEY", /**/
    "MODE_SLEEP", /**/
    "MODE_FF", /**/
    "MODE_NAV", /**/
    "MODE_NAV_BASE", /**/
    "MODE_DEBUG", /**/
    "MODE_FINGER_BASE", /**/
    "MODE_IDLE", /**/
    "MODE_HBD", /**/
    "MODE_HBD_DEBUG", /**/
    };
    if (mode < MODE_MAX) {
        GF_LOGI(LOG_TAG "[%s] mode = %s", func_name, mode_str[mode]);
    }
}

static void gf_ta_dump_operation(const char *func_name, gf_operation_type_t operation) {
    char operation_str[][64] = { /**/
    "OPERATION_AUTHENTICATE", /**/
    "OPERATION_ENROLL", /**/
    "OPERATION_FINGER_BASE", /**/
    "OPERATION_TEST_SENSOR_STEP1", /**/
    "OPERATION_TEST_SENSOR_STEP2", /**/
    "OPERATION_TEST_BAD_POINT", /**/
    "OPERATION_TEST_PERFORMANCE", /**/
    "OPERATION_TEST_SPI_PERFORMANCE", /**/
    "OPERATION_TEST_SPI_TRANSFER", /**/
    "OPERATION_TEST_FRR_FAR_RECORD_BASE_FRAME", /**/
    "OPERATION_TEST_FRR_FAR_RECORD_ENROLL", /**/
    "OPERATION_TEST_FRR_FAR_RECORD_AUTHENTICATE", /**/
    "OPERATION_TEST_UNTRUSTED_ENROLL", /**/
    "OPERATION_TEST_UNTRUSTED_AUTHENTICATE", /**/
    "OPERATION_TEST_CHECK_FINGER_EVENT",
    "OPERATION_TEST_BIO_CALIBRATION",
    "OPERATION_TEST_HBD_CALIBRATION",
    "OPERATION_TEST_FPC_KEY_DETECT",
    "OPERATION_NAV", /**/
    "OPERATION_NAV_BASE", /**/
    "OPERATION_CHECK_FINGER_LONG_PRESS", /**/
    "OPERATION_SENSOR_CHECK", /**/
    "OPERATION_TEST_RESET_PIN", /**/
    "OPERATION_HOME_KEY", /**/
    "OPERATION_POWER_KEY", /**/
    "OPERATION_CAMERA_KEY", /**/
    "OPERATION_HEARTBEAT_KEY", /**/
    "OPERATION_NONE", /**/
    };

    if (operation < OPERATION_MAX) {
        GF_LOGI(LOG_TAG "[%s] operation = %s", func_name, operation_str[operation]);
    }
}

static gf_error_t gf_ta_save_auth_token(void) {
    gf_error_t err = GF_SUCCESS;
    uint8_t *buf = NULL;
    uint32_t buf_len = 0;

    do {
        gf_so_get_auth_token_len(&buf_len);
        buf = (uint8_t *) CPL_MEM_MALLOC(buf_len);
        if (NULL == buf) {
            GF_LOGE(LOG_TAG "[%s] finger out of memory", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        cpl_memset(buf, 0, buf_len);
        err = gf_so_get_auth_token_pdu(buf, buf_len, g_user_id, g_authenticator_id);
        if (err != GF_SUCCESS) {
            break;
        }

        err = gf_so_save_persistent_object((int8_t*) GF_FINGER_SECURE_ID, buf, buf_len);
    } while (0);

    if (buf != NULL) {
        CPL_MEM_FREE(buf);
        buf = NULL;
    }

    return err;
}

static gf_error_t gf_ta_load_auth_token(void) {
    gf_error_t err = GF_SUCCESS;
    uint8_t *buf = NULL;
    uint32_t buf_len = 0;

    do {
        err = gf_so_load_persistent_object((int8_t*) GF_FINGER_SECURE_ID, &buf, &buf_len);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] Load secure id failed", __func__);
            break;
        }

        err = gf_so_load_auth_token(&g_user_id, &g_authenticator_id, buf, buf_len);

    } while (0);

    if (buf != NULL) {
        CPL_MEM_FREE(buf);
        buf = NULL;
    }

    return err;
}

static gf_error_t gf_ta_set_mode(gf_mode_t mode, gf_operation_type_t operation) {
    gf_error_t err = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] enter, mode:%d, operation:%d", __func__, mode, operation);

    do {
        if (mode == MODE_NONE) {
            GF_LOGE(LOG_TAG "[%s] mode is MODE_NONE, nothing to do.", __func__);
            break;
        }

        g_cur_mode = mode;
        g_operation = operation;

        err = g_spi_function.set_mode(mode);

        if (OPERATION_AUTHENTICATE == g_operation) {
            g_authenticate_is_cancel = 0;
        }

        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] set mode(%d) fail, err:%d", __func__, mode, err);
            break;
        }

        if ((MODE_FINGER_BASE == g_cur_mode) || (MODE_NAV_BASE == g_cur_mode || MODE_DEBUG == g_cur_mode)) {
            g_spi_function.data_sample_resume();
        }

    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);
    return err;
}

static gf_error_t gf_ta_set_normal_mode(uint8_t *reset_flag) {
    gf_mode_t mode = MODE_KEY;
    gf_operation_type_t operation = OPERATION_NONE;

    if (g_sensor.config.support_nav_mode > GF_NAV_MODE_NONE) {
        mode = MODE_NAV;
        operation = OPERATION_NAV;
    } else if (g_sensor.config.support_key_mode > 0) {
        mode = MODE_KEY;
        operation = OPERATION_HOME_KEY;
    } else {
        mode = MODE_SLEEP;
        operation = OPERATION_NONE;
    }

    return gf_ta_pre_set_mode(mode, operation, reset_flag);
}

gf_error_t gf_ta_pre_set_mode(gf_mode_t mode, gf_operation_type_t operation,
        uint8_t *reset_flag) {

    gf_error_t err = GF_SUCCESS;
    uint8_t touch_status = 0;
    GF_LOGD(LOG_TAG "[%s] enter, mode = %d, operation = %d, next_mode = %d", __func__, mode, operation, g_next_mode);

    do {
        if (OPERATION_FINGER_BASE == operation || OPERATION_NAV_BASE == operation
                       || OPERATION_CHECK_FINGER_LONG_PRESS == operation) {
            if (MODE_NAV_BASE == g_cur_mode || MODE_FINGER_BASE == g_cur_mode) {
                GF_LOGI(LOG_TAG "[%s] mode is same debug,don't switch", __func__);
                break;
            }
        }
        if (MODE_NONE == g_next_mode) {
            if (OPERATION_FINGER_BASE == operation || OPERATION_NAV_BASE == operation
                       || OPERATION_CHECK_FINGER_LONG_PRESS == operation) {
                g_next_mode = g_cur_mode;
                g_next_operation = g_operation;
            }
        } else {
            if (g_finger_pressed_flag > 0) {
                err = g_spi_function.get_key_status(&touch_status);
                if (0 == touch_status) {
                    g_finger_pressed_flag = 0;
                    g_next_mode = mode;
                    g_next_operation = operation;
                    gf_ta_set_next_mode(reset_flag);
                    break;
                }
                if (MODE_IMAGE == mode || MODE_KEY == mode || MODE_SLEEP == mode || MODE_NAV == mode
                        || MODE_FF == mode) {
                    g_next_mode = mode;
                    g_next_operation = operation;
                    break;
                } else {
                    g_finger_pressed_flag = 0;
                }
            } else if (MODE_NAV_BASE == g_cur_mode || MODE_FINGER_BASE == g_cur_mode) {
                g_next_mode = mode;
                g_next_operation = operation;
                GF_LOGD(LOG_TAG "[%s] update finger_base or update nav_base, shouldn't switch mode", __func__);
                break;
            }
        }

        GF_LOGD(LOG_TAG "[%s] g_finger_pressed_flag:%d, mode:%d, g_cur_mode:%d", __func__,
                g_finger_pressed_flag, mode, g_cur_mode);

        if (mode == g_cur_mode) {

            GF_LOGE(LOG_TAG "[%s] current mode is the same, mode = %d", __func__, mode);

            if (operation != g_operation) {
                GF_LOGE(LOG_TAG "[%s] operation is different, g_operation:%d, operation:%d",
                        __func__, g_operation, operation);
                gf_ta_cancel_operation();
            }
        }

        err = gf_ta_set_mode(mode, operation);
        if (err != GF_SUCCESS) {
            *reset_flag = 1;
        } else {
            if (MODE_IMAGE == mode
                    && (OPERATION_ENROLL == operation
                            || OPERATION_TEST_FRR_FAR_RECORD_ENROLL == operation
                            || OPERATION_TEST_UNTRUSTED_ENROLL == operation)) {
                if (IS_MILAN_A_SERIES_BY_CHIP_TYPE(g_sensor.chip_type)) {
                    g_spi_function.set_config_for_stable_acquire_image(1);
                    g_is_normal_cfg = 0;
                }
            }
        }
    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit", __func__);
    // always success
    return GF_SUCCESS;
}

gf_error_t gf_ta_set_next_mode(uint8_t *reset_flag) {
    gf_error_t err = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        GF_LOGD(LOG_TAG "[%s] g_next_mode:%d, g_next_operation:%d", __func__, g_next_mode,
                g_next_operation);

        err = gf_ta_set_mode(g_next_mode, g_next_operation);
        if (GF_SUCCESS == err) {
            *reset_flag = 0;
            g_next_mode = MODE_NONE;
            g_next_operation = OPERATION_NONE;
        } else {
            *reset_flag = 1;
            GF_LOGD(LOG_TAG "[%s] gf_ta_set_mode, err:%d, set reset flag.", __func__, err);
        }
    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);

    return err;
}

static gf_error_t gf_ta_init_algo_module(void) {

    gf_error_t err = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        gf_algo_config_t algo_config = { 0 };

        err = g_spi_function.get_sensor_id(algo_config.sensor_id, GF_SENSOR_ID_LEN);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] get sensor_id fail:%d", __func__, err);
        }

        err = g_spi_function.get_sensor_otp_info(algo_config.otp_info, GF_SENSOR_OTP_INFO_LEN);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] get sensor otp info fail:%d", __func__, err);
        }

        cpl_memcpy(g_sensor_otp_info, algo_config.otp_info, GF_SENSOR_OTP_INFO_LEN);

        err = g_spi_function.get_vendor_id(algo_config.vendor_id, GF_VENDOR_ID_LEN);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] get vendor_id fail:%d", __func__, err);
        }

        err = g_spi_function.get_chip_id(algo_config.chip_id, GF_CHIP_ID_LEN);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] get chip_id fail:%d", __func__, err);
        }

        algo_config.chip_type = g_sensor.chip_type;
        algo_config.basic_config.duplicate_finger_overlay_score =
                g_sensor.config.duplicate_finger_overlay_score;
        algo_config.basic_config.enrolling_min_templates = g_sensor.config.enrolling_min_templates;
        algo_config.flag_chip_info = g_sensor.flag_chip_info;
        algo_config.basic_config.forbidden_enroll_duplicate_fingers =
                g_sensor.config.forbidden_enroll_duplicate_fingers;
        algo_config.basic_config.increase_rate_between_stitch_info =
                g_sensor.config.increase_rate_between_stitch_info;
        algo_config.basic_config.max_fingers_per_user = g_sensor.config.max_fingers_per_user;
        algo_config.thr_select_bmp = g_sensor.thr_select_bmp;
        algo_config.basic_config.valid_image_area_threshold =
                g_sensor.config.valid_image_area_threshold;
        algo_config.basic_config.valid_image_quality_threshold =
                g_sensor.config.valid_image_quality_threshold;
        algo_config.basic_config.support_bio_assay = g_sensor.config.support_bio_assay;
        algo_config.basic_config.support_cover_broken_check = g_sensor.config.support_cover_broken_check;

        err = gf_algo_init(&algo_config);

        /* reset group id and reload fingerprint list,because group id cleared in gf_algo_init */
        g_set_active_group_flag = 0;
    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);

    return err;
}

static gf_error_t gf_ta_detect_sensor(gf_detect_sensor_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        //samsung exynos8890 should init spi first.
        #if defined(CONFIG_EXYNOS8890)
        err = gf_spi_init();
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] :spi init failed, err:%d.", __func__, err);
            break;
        }
        GF_LOGD(LOG_TAG "[%s] : exynos8890 spi init success.", __func__);
        #endif

        err = g_spi_function.select_chip_otp(cmd);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] get otp failed, err:%d.", __func__, err);
            break;
        }

        err = g_spi_function.detect_sensor();
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] detect sensor failed, err:%d.", __func__, err);
            break;
        }

        cpl_memcpy(&cmd->config, &g_sensor.config, sizeof(gf_config_t));
    } while(0);

    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);

    return err;
}

static gf_error_t gf_ta_init(gf_init_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    GF_LOGD(LOG_TAG "SOFT VERSION INFO");

    GF_LOGD(LOG_TAG "TARGET_MODE          : %s ", GF_TARGET_MODE);

    GF_LOGD(LOG_TAG "CHIP_TYPE            : %s ", GF_CHIP_TYPE);

    GF_LOGD(LOG_TAG "PACKAGE_VERSION_CODE : %s ", GF_PACKAGE_VERSION_CODE);

    GF_LOGD(LOG_TAG "PACKAGE_VERSION_NAME : %s ", GF_PACKAGE_VERSION_NAME);

    GF_LOGD(LOG_TAG "GIT_BRANCH           : %s ", GF_GIT_BRANCH);

    GF_LOGD(LOG_TAG "COMMIT_ID            : %s ", GF_COMMIT_ID);

    GF_LOGD(LOG_TAG "BUILD_TIME           : %s ", GF_BUILD_TIME);

    GF_LOGD(LOG_TAG "sizeof(gf_init_t)    : %d", (uint32_t)sizeof(gf_init_t));

    do {
        err = g_spi_function.chip_init(&cmd->download_fw_flag, &cmd->download_cfg_flag);

        if (GF_ERROR_SPI_COMMUNICATION == err || GF_ERROR_SENSOR_NOT_AVAILABLE == err || GF_ERROR_GET_OTP_INFO_FAILED == err) {
             break;
         } else {
             err = GF_SUCCESS;
         }

        err = gf_spi_create();
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] gf_spi_create fail, err:%d", __func__, err);
            break;
        }

        if (cmd->download_fw_flag > 0) {
            err = g_spi_function.download_fw(&cmd->cmd_header.reset_flag);
        }
        g_spi_function.get_vendor_id(cmd->vendor_id, GF_VENDOR_ID_LEN);

        cmd->row = g_sensor.row;
        cmd->col = g_sensor.col;
        cmd->nav_row = g_sensor.nav_row;
        cmd->nav_col = g_sensor.nav_col;
        cmd->nav_gap = g_sensor.nav_gap;
        cmd->esd_check_flag = g_sensor.support_esd_check;

        gf_ta_load_auth_token();

    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);

    return err;
}

static gf_error_t gf_ta_download_fw(gf_cmd_header_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    err = g_spi_function.download_fw(&cmd->reset_flag);

    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);

    return err;
}

static gf_error_t gf_ta_download_cfg(gf_cmd_header_t *cmd) {
    UNUSED_VAR(cmd);

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    return g_spi_function.download_cfg(CONFIG_NORMAL);
}

static gf_error_t gf_ta_enable_reissue_key_down(uint8_t flag) {
    gf_error_t err = GF_SUCCESS;
    GF_LOGD(LOG_TAG "[%s] enter", __func__);
    //flag = 0 for FF mode & flag = 1 for image mode
    do {
        if (MODE_SLEEP == g_cur_mode) {
            GF_LOGE(LOG_TAG "[%s] can't enable reissue key down in sleep mode", __func__);
            break;
        }
        if(0 == flag){
            err = g_spi_function.enable_reissue_key_down(MODE_FF);
        }else{
            err = g_spi_function.enable_reissue_key_down(MODE_IMAGE);
        }

        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] enable reissue key down failed, err = %d", __func__, err);
            break;
        }
    } while (0);
    return err;
}

static gf_error_t gf_ta_init_finger_index_hash_table(void) {

    gf_error_t err = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);
    do {
        // init fingerprint index hash table.
        uint32_t index_hash_table_len = g_sensor.config.max_fingers_per_user * sizeof(uint32_t);

        if (NULL != g_finger_index_hash_table) {
            CPL_MEM_FREE(g_finger_index_hash_table);
        }

        g_finger_index_hash_table = (uint32_t *) CPL_MEM_MALLOC(index_hash_table_len);
        if (NULL == g_finger_index_hash_table) {
            err = GF_ERROR_OUT_OF_MEMORY;
            GF_LOGE(LOG_TAG "[%s] out of memory for finger index hash table", __func__);
            break;
        }

        cpl_memset(g_finger_index_hash_table, 0, index_hash_table_len);
    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);

    return err;
}

static gf_error_t gf_ta_init_finished(gf_init_finished_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        err = gf_ta_init_finger_index_hash_table();
        if (GF_SUCCESS != err) {
            GF_LOGD(LOG_TAG "[%s] init finger index hash table failed.", __func__);
            break;
        }

        // init chip
        err = g_spi_function.chip_config_init();
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] chip config init failed.", __func__);
        }

        // init algorithm module
        err = gf_ta_init_algo_module();
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] init algorithm module fail:%d", __func__, err);
            break;
        }

        err = g_spi_function.get_orientation(&cmd->orientation,
                &cmd->facing);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] get orientation fail:%d", __func__, err);
        }

        err = g_spi_function.aes_set();
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] set ase key error, err:%d", __func__, err);
            break;
        }

        if (g_sensor.config.support_sensor_broken_check) {
            err = gf_broken_check_init(&g_spi_function, &g_operation);
            if (err != GF_SUCCESS) {
                GF_LOGE(LOG_TAG "[%s] init sensor broken check error, err:%d", __func__, err);
                break;
            }
            err = g_spi_function.start_sensor_broken_check();
            if (err != GF_SUCCESS) {
                GF_LOGE(LOG_TAG "[%s] start broken check error, err:%d", __func__, err);
                break;
            }
        }

        if (g_sensor.config.support_nav_mode > GF_NAV_MODE_NONE) {
            g_cur_mode = MODE_NAV;
            g_operation = OPERATION_NAV;
        } else if (g_sensor.config.support_key_mode > 0) {
            g_cur_mode = MODE_KEY;
            g_operation = OPERATION_HOME_KEY;
        } else {
            g_cur_mode = MODE_SLEEP;
            g_operation = OPERATION_NONE;
        }

        err = gf_base_init_finished(&cmd->cmd_header.reset_flag);
        //gf_ta_generate_authenticator_id(&g_authenticator_id);

        if (1 == gf_algo_get_calibration_state()) {
            uint8_t *base = NULL;
            uint32_t base_len = 0;

            err = gf_algo_get_base_rawdata((uint16_t**) &base, &base_len);
            if (GF_SUCCESS == err) {
                cpl_memcpy((void *) cmd->finger_base, base,
                        base_len * sizeof(uint16_t));
                cmd->finger_base_len = base_len;
            }
        }

        if (IS_MILAN_F_SERIES_BY_CHIP_TYPE(g_sensor.chip_type) ||
            (IS_MILAN_A_SERIES_BY_CHIP_TYPE(g_sensor.chip_type) &&
                0 == g_sensor.config.support_fw_navigation)) {
            uint8_t *base = NULL;
            uint32_t base_len = 0;

            if (GF_SUCCESS == gf_algo_get_nav_base((uint16_t**) &base, &base_len)) {
                cpl_memcpy((void *) cmd->nav_base,
                        (void *) base, base_len * sizeof(uint16_t));
                cmd->nav_base_len = base_len;
            }
        }

        g_spi_function.init_finished();
    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);

    return err;
}

gf_error_t gf_ta_pre_enroll(gf_pre_enroll_t *cmd) {
    g_challenge = cmd->challenge;
    gf_base_on_pre_enroll(&(cmd->cmd_header.reset_flag));
    return GF_SUCCESS;
}

gf_error_t gf_ta_enroll(gf_enroll_t *cmd) {
    gf_error_t err = GF_SUCCESS;
    uint32_t group_id = cmd->group_id;
    gf_hw_auth_token_t *token = &cmd->hat;
    gf_hw_auth_token_t hat = { 0 };
    uint32_t hmac_len = sizeof(hat.hmac);
    GF_LOGD(LOG_TAG "[%s] enter, group_id:%d", __func__, group_id);

    do {
        if (token && token->challenge == g_challenge) {
            g_user_id = token->user_id;
        } else {
            GF_LOGE(LOG_TAG "[%s] invalid or null auth token", __func__);
        }

        if (g_sensor.config.forbidden_untrusted_enroll > 0) {
            if (token && token->version != cmd->system_auth_token_version) {
                GF_LOGE(LOG_TAG "[%s] invalid hat version code detected", __func__);
                err = GF_ERROR_INVALID_HAT_VERSION;
                break;
            }

            if (token && token->challenge != g_challenge
                    && (token->authenticator_type & GF_HW_AUTH_FINGERPRINT)) {
                GF_LOGE(LOG_TAG "[%s] invalid challenge detected", __func__);
                err = GF_ERROR_INVALID_CHALLENGE;
                break;
            }

            cpl_memcpy(&hat, token, sizeof(gf_hw_auth_token_t));
            cpl_memset(&(hat.hmac), 0, hmac_len);
            gf_generate_hmac_enroll(&hat);

            if (0 != cpl_memcmp(hat.hmac, token->hmac, hmac_len)) {
                GF_LOGE(LOG_TAG "[%s] token authenticate failed", __func__);
                err = GF_ERROR_UNTRUSTED_ENROLL;
                break;
            }
        }

        if (0 == g_set_active_group_flag) {
            gf_ta_switch_fingerprint_user(group_id);
        }

        err = gf_algo_fingers_limit_check();
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] reach to the max count, err:%d", __func__, err);
            break;
        }

        err = gf_algo_pre_enroll(group_id);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] gf_algo_pre_enroll return:%d", __func__, err);
            break;
        }

        err = gf_ta_pre_set_mode(MODE_IMAGE, OPERATION_ENROLL, &cmd->cmd_header.reset_flag);
    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);

    return err;
}

gf_error_t gf_ta_post_enroll(void) {
    GF_LOGD(LOG_TAG "[%s] enter", __func__);
    return GF_SUCCESS;
}

static void gf_ta_cancel_operation(void) {
    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    switch (g_operation) {
        case OPERATION_ENROLL:
            gf_algo_cancel_enroll();
            if (IS_MILAN_A_SERIES_BY_CHIP_TYPE(g_sensor.chip_type) && (0 == g_is_normal_cfg)) {
                g_spi_function.set_config_for_stable_acquire_image(0);
                g_is_normal_cfg = 1;
            }
            break;

        case OPERATION_AUTHENTICATE:
            gf_algo_cancel_authenticate();
            g_authenticate_fail_count = 0;
            g_authenticate_is_cancel = 1;
            break;

        case OPERATION_TEST_SENSOR_STEP1:
        case OPERATION_TEST_SENSOR_STEP2:
            gf_pixel_test_cancel();

            if (0 == g_is_normal_cfg) {

                if (IS_MILAN_A_SERIES_BY_CHIP_TYPE(g_sensor.chip_type)) {
                    g_spi_function.download_cfg(CONFIG_NORMAL);
                } else if (IS_MILAN_F_SERIES_BY_CHIP_TYPE(g_sensor.chip_type)) {
                    g_spi_function.update_dac_for_sensor_pixel_open_test(1, 0);
                } else {
                    gf_ta_set_mode(MODE_KEY, OPERATION_NONE);
                    g_spi_function.download_cfg(CONFIG_NORMAL);
                }

            }

            g_is_normal_cfg = 1;
            break;

        case OPERATION_TEST_HBD_CALIBRATION:
            if (0 == g_is_normal_cfg) {

                if (IS_MILAN_A_SERIES_BY_CHIP_TYPE(g_sensor.chip_type)) {
                    g_spi_function.download_cfg(CONFIG_NORMAL);
                } else if (IS_MILAN_F_SERIES_BY_CHIP_TYPE(g_sensor.chip_type)) {
                    g_spi_function.update_dac_for_sensor_pixel_open_test(1, 0);
                } else {
                    gf_ta_set_mode(MODE_KEY, OPERATION_NONE);
                    g_spi_function.download_cfg(CONFIG_NORMAL);
                }

            }
            g_is_normal_cfg = 1;
            break;

        case OPERATION_TEST_PERFORMANCE:
            gf_algo_cancel_test_performance();
            break;

        case OPERATION_TEST_FRR_FAR_RECORD_BASE_FRAME:
        case OPERATION_TEST_FRR_FAR_RECORD_ENROLL:
        case OPERATION_TEST_FRR_FAR_RECORD_AUTHENTICATE:
            //gf_frr_far_cancel();
            break;

        case OPERATION_TEST_UNTRUSTED_ENROLL:
            gf_algo_cancel_test_enroll();
            break;

        case OPERATION_TEST_UNTRUSTED_AUTHENTICATE:
            gf_algo_cancel_test_authenticate();
            break;
        case OPERATION_TEST_BAD_POINT: {
            g_bad_point_test_image_count = 0;
            break;
        }

        case OPERATION_FINGER_BASE:
        case OPERATION_NAV_BASE:
        case OPERATION_NAV: {
            gf_base_stop_navigation();
            break;
        }
        case OPERATION_TEST_FPC_KEY_DETECT:
            gf_ta_set_mode(MODE_KEY, OPERATION_NONE);
            break;
        default:
            break;
    }

    GF_LOGD(LOG_TAG "[%s] exit", __func__);
}

gf_error_t gf_ta_cancel(gf_cancel_t *cmd) {

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    cmd->operation = g_operation;

    gf_ta_cancel_operation();
    /*
     * there is a trap:
     * when the function is called, #gf_ta_cancel_operation release some resource;
     * but #gf_ta_pre_set_mode maybe can not change the mode immediately,such as cancel but sensor is pressed, it will be set mode next;
     * in this condition, attention the released resource is NULL;
     */
    return gf_ta_set_normal_mode(&cmd->cmd_header.reset_flag);
}

static gf_error_t gf_ta_auto_cancel(gf_cmd_header_t *cmd) {

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    gf_ta_cancel_operation();

    return gf_ta_set_normal_mode(&cmd->reset_flag);
}

gf_error_t gf_ta_authenticate(gf_authenticate_t *cmd) {

    gf_mode_t mode = MODE_IMAGE;

    GF_LOGD(LOG_TAG "[%s] enter, group_id = %d", __func__, cmd->group_id);
    g_operation_id = cmd->operation_id;

    if (0 == g_set_active_group_flag) {
        gf_ta_switch_fingerprint_user(cmd->group_id);
    }

    if (-1 != cmd->screen_on_flag) {
        g_screen_on_flag = cmd->screen_on_flag;
    }

    if (0 == g_screen_on_flag) {
        if (g_sensor.config.support_ff_mode > 0) {
            mode = MODE_FF;
        } else {
            mode = MODE_SLEEP;
        }
    }

    g_authenticate_fail_count = 0;

    return gf_ta_pre_set_mode(mode, OPERATION_AUTHENTICATE, &cmd->cmd_header.reset_flag);
}

gf_error_t gf_ta_get_auth_id(gf_get_auth_id_t *cmd) {
    cmd->auth_id = g_authenticator_id;
    return GF_SUCCESS;
}

static gf_error_t gf_ta_user_get_last_identify_id_cmd(void *buffer) {
    gf_user_last_identify_id_t *user_buffer = (gf_user_last_identify_id_t *)buffer;
    return gf_algo_get_last_identify_id(&(user_buffer->last_identify_id));
}

gf_error_t gf_ta_get_last_identify_id(uint8_t *pIdBuf, uint32_t *pBufLen) {
    gf_error_t ret = GF_SUCCESS;
    uint32_t id = 0;

    GF_LOGD(LOG_TAG "%s enter", __func__);

    do {
        if (NULL == pIdBuf || NULL == pBufLen) {
            ret = GF_ERROR_BAD_PARAMS;
            break;
        }

        ret = gf_algo_get_last_identify_id(&id);
        if (GF_SUCCESS != ret) {
            break;
        }

        *pBufLen = sizeof(uint32_t);
        cpl_memcpy(pIdBuf, &id, *pBufLen);
    } while (0);

    return ret;
}

gf_error_t gf_ta_get_id_list(uint8_t *pIdBuf, uint32_t *pBufLen) {
    gf_error_t ret = GF_SUCCESS;
    gf_enumerate_t enumerate = {.cmd_header ={0}, 0 };
    uint32_t len = 4;
    uint32_t i = 0;

    GF_LOGD(LOG_TAG "%s enter", __func__);

    do {
        if (NULL == pIdBuf || NULL == pBufLen) {
            ret = GF_ERROR_BAD_PARAMS;
            break;
        }

        ret = gf_algo_enumerate(enumerate.group_ids, enumerate.finger_ids, &(enumerate.size));
        if (GF_SUCCESS != ret) {
            break;
        }

        while (i < enumerate.size) {
            /* length */
            cpl_memcpy(pIdBuf, &len, sizeof(uint32_t));
            pIdBuf += 4;

            /* id*/
            cpl_memcpy(pIdBuf, &enumerate.finger_ids[i], sizeof(uint32_t));
            pIdBuf += 4;

            i++;
        }

        *pBufLen = enumerate.size * sizeof(uint32_t) * 2;
    } while (0);

    return ret;
}

gf_error_t gf_get_authenticator_version(uint32_t *pVersion) {
    gf_error_t ret = GF_SUCCESS;

    GF_LOGD(LOG_TAG "%s enter", __func__);

    do {
        if (NULL == pVersion) {
            ret = GF_ERROR_BAD_PARAMS;
            break;
        }

        *pVersion = (uint32_t)GF_AUTHENTICATOR_VERSION_CODE;
    } while (0);

    return ret;
}

static gf_error_t gf_ta_find_slot_for_finger(gf_finger_status_t status, uint32_t finger_id,
        uint32_t *slot) {

    gf_error_t err = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        uint32_t i = 0;

        if (NULL == slot) {
            GF_LOGE(LOG_TAG "[%s] bad parameter", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        for (; i < g_sensor.config.max_fingers_per_user; i++) {
            if (((FINGER_STATUS_ENROLLING == status) && (g_finger_index_hash_table[i] == 0))
                    || (g_finger_index_hash_table[i] == finger_id)) {
                *slot = i;
                GF_LOGI(LOG_TAG "[%s] find slot(%d) for finger(%u)", __func__, i, finger_id);
                break;
            }
        }

        if (i == g_sensor.config.max_fingers_per_user) {
            err = GF_ERROR_REACH_FINGERS_UPLIMIT;
            GF_LOGE(LOG_TAG "[%s] can not find the slot for finger(%u)", __func__, finger_id);
            break;
        }
    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);

    return err;
}

static gf_error_t gf_ta_save(gf_save_t *cmd) {

    uint8_t *buf = NULL;
    gf_error_t err = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        uint32_t buf_len = 0;
        uint32_t group_id = 0;
        uint32_t finger_id = 0;
        uint32_t index = 0;
        uint8_t name[GF_FINGER_OBJECT_ID_MAX_LEN] = { 0 };
        gf_finger_status_t status = FINGER_STATUS_IDLE;

        if (NULL == cmd) {
            GF_LOGE(LOG_TAG "[%s] bad parameter", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (cmd->authenticator_id != 0) {
            g_authenticator_id = cmd->authenticator_id;
            gf_ta_save_auth_token();
        }

        group_id = cmd->group_id;
        finger_id = cmd->finger_id;
        GF_LOGD(LOG_TAG "[%s] group_id:%u, finger_id:%u", __func__, group_id, finger_id);

        err = gf_algo_get_finger_status(group_id, finger_id, &status);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] group_id(%d), finger(%u) not exist, err:%d", __func__, group_id,
                    finger_id, err);
            break;
        }

        err = gf_ta_find_slot_for_finger(status, finger_id, &index);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] can not find the slot to save finger(%u)", __func__, finger_id);
            break;
        }

        cpl_sprintf((int8_t*) name, (const int8_t*) GF_FINGER_OBJECT_ID, group_id, index);

        err = gf_algo_get_fingerTempInfo_size(finger_id, &buf_len);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] get finger(%u) size fail:%d", __func__, finger_id, err);
            break;
        }

        buf = (uint8_t *) CPL_MEM_MALLOC(buf_len);
        if (NULL == buf) {
            GF_LOGE(LOG_TAG "[%s] buf out of memory", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }
        err = gf_algo_get_finger_pdu(group_id, finger_id, buf, buf_len);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] fail to get data,err:%d", __func__, err);
            break;
        }

        err = gf_so_save_persistent_object((int8_t*) name, buf, buf_len);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] fail to save finger(%u)'s so(%s), err:%d", __func__, finger_id,
                    name, err);
            break;
        }

        // NOTE: here it must be success. so don't care error code
        gf_algo_set_finger_status(group_id, finger_id, FINGER_STATUS_READY);
        g_finger_index_hash_table[index] = finger_id;

        if (FINGER_STATUS_ENROLLING == status) {
            gf_error_t tmp_err = GF_SUCCESS;

            cpl_sprintf((int8_t*) name, (const int8_t*) GF_FINGER_OBJECT_ID_BAK, group_id, index);

            tmp_err = gf_so_save_persistent_object((int8_t*) name, buf, buf_len);
            if (GF_SUCCESS != tmp_err) {
                GF_LOGE(LOG_TAG "[%s] fail to save finger(%u)'s so(%s), tmp_err:%d", __func__,
                        finger_id, name, tmp_err);
            } else {
                GF_LOGI(LOG_TAG "[%s] success to save finger(%u)'s so(%s)", __func__, finger_id,
                        name);
            }
        }
    } while (0);

    if (buf != NULL) {
        CPL_MEM_FREE(buf);
    }

    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);

    return err;
}

gf_error_t gf_ta_remove(gf_remove_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        uint32_t slot = 0;
        uint32_t group_id = cmd->group_id;
        uint32_t finger_id = cmd->finger_id;
        uint8_t name[GF_FINGER_OBJECT_ID_MAX_LEN] = { 0 };
        uint32_t finger_ids[MAX_FINGERS_PER_USER] = { 0 };
        uint32_t group_ids[MAX_FINGERS_PER_USER] = { 0 };
        uint32_t finger_count = 0;
        uint32_t removing_templates = 0;
        uint32_t i = 0;

        GF_LOGD(LOG_TAG "[%s] group_id:%u, finger_id:%u", __func__, group_id, finger_id);

        if (0 == g_set_active_group_flag) {
            gf_ta_switch_fingerprint_user(group_id);
        }

        if (0 != finger_id) {
            finger_ids[0] = finger_id;
            group_ids[0] = group_id;
            finger_count = 1;
        } else {
            gf_algo_enumerate(group_ids, finger_ids, &finger_count);
        }

        removing_templates = finger_count;
        for (i = 0; i < finger_count; i++) {
            err = gf_ta_find_slot_for_finger(FINGER_STATUS_READY, finger_ids[i], &slot);
            if (GF_SUCCESS != err) {
                GF_LOGE(LOG_TAG "[%s] can not find the slot for finger(%d), err:%d", __func__,
                        finger_ids[i], err);
                err = GF_SUCCESS; // NOTE: because google's application's limit, so should return success here
                cmd->deleted_fids[i] = finger_ids[i];
                cmd->deleted_gids[i] = group_ids[i];
                continue;
            }

            cpl_sprintf((int8_t*) name, (const int8_t*) GF_FINGER_OBJECT_ID, group_id, slot);

            err = gf_so_delete_persistent_object((int8_t*) name);
            if (GF_SUCCESS != err) {
                removing_templates--;
                GF_LOGE(LOG_TAG "[%s] fail to delete so(%s) for finger(%d), err:%d", __func__, name,
                        finger_ids[i], err);
                continue;
            }

            cpl_sprintf((int8_t*) name, (const int8_t*) GF_FINGER_OBJECT_ID_BAK, group_id, slot);

            err = gf_so_delete_persistent_object((int8_t*) name);
            if (GF_SUCCESS != err) {
                removing_templates--;
                GF_LOGE(LOG_TAG "[%s] fail to delete so(%s) for finger(%d), err:%d", __func__, name,
                        finger_ids[i], err);
                continue;
            }

            g_finger_index_hash_table[slot] = 0;
            err = gf_algo_remove(group_id, finger_ids[i]);

            cmd->deleted_fids[i] = finger_ids[i];
            cmd->deleted_gids[i] = group_ids[i];
        }
        cmd->removing_templates = removing_templates;
    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);

    return err;
}

static gf_error_t gf_ta_load_finger(uint32_t group_id, uint32_t index, uint32_t *finger_id) {

    gf_error_t err = GF_SUCCESS;
    uint8_t *buf = NULL;
    uint32_t buf_len = 0;
    uint8_t name[GF_FINGER_OBJECT_ID_MAX_LEN] = { 0 };

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {

        if (NULL == finger_id) {
            GF_LOGE(LOG_TAG "[%s] bad parameters", __func__);
            break;
        }

        cpl_sprintf((int8_t*) name, (const int8_t*) GF_FINGER_OBJECT_ID, group_id, index);

        err = gf_so_load_persistent_object((int8_t*) name, &buf, &buf_len);

        if (GF_SUCCESS == err) {

            GF_LOGD(LOG_TAG "[%s] load so(%s) success", __func__, name);

            err = gf_algo_add(buf, buf_len, group_id, finger_id);
            if (GF_SUCCESS == err) {

                gf_error_t tmp_err = GF_SUCCESS;

                GF_LOGD(LOG_TAG "[%s] add finger(%d) success", __func__, *finger_id);

                cpl_sprintf((int8_t*) name, (const int8_t*) GF_FINGER_OBJECT_ID_BAK, group_id,
                        index);
                tmp_err = gf_so_save_persistent_object((int8_t*) name, buf, buf_len);
                if (GF_SUCCESS == tmp_err) {
                    GF_LOGI(LOG_TAG "[%s] sync data to backup object(%s) success", __func__, name);
                } else {
                    GF_LOGE(LOG_TAG "[%s] sync data to backup object(%s) fail,err:%d", __func__,
                            name, tmp_err);
                }

                break;
            }

            if (GF_ERROR_REACH_FINGERS_UPLIMIT == err) {
                GF_LOGE(LOG_TAG "[%s] reach to the up limit when add so(%s)", __func__, name);
                break;
            }

            GF_LOGE(LOG_TAG "[%s] fail to add finger(%s), err:%d", __func__, name, err);
        } else {
            GF_LOGD(LOG_TAG "[%s] load so(%s) fail, err:%d", __func__, name, err);
        }

        // load the finger from the backup object
        cpl_sprintf((int8_t*) name, (const int8_t*) GF_FINGER_OBJECT_ID_BAK, group_id, index);
        if (buf != NULL) {
            CPL_MEM_FREE(buf);
            buf = NULL;
        }

        err = gf_so_load_persistent_object((int8_t*) name, &buf, &buf_len);
        if (GF_SUCCESS == err) {

            GF_LOGD(LOG_TAG "[%s] load so(%s) success", __func__, name);

            err = gf_algo_add(buf, buf_len, group_id, finger_id);
            if (GF_SUCCESS == err) {
                GF_LOGI(LOG_TAG "[%s] load finger(%d) success", __func__, *finger_id);
            } else {
                GF_LOGE(LOG_TAG "[%s] load finger(%s) fail, err:%d", __func__, name, err);
            }
        } else {
            GF_LOGD(LOG_TAG "[%s] load so(%s) fail, err:%d", __func__, name, err);
        }
    } while (0);

    if (buf != NULL) {
        CPL_MEM_FREE(buf);
        buf = NULL;
    }

    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);

    return err;
}

static gf_error_t gf_ta_switch_fingerprint_user(uint32_t group_id) {

    gf_error_t err = GF_SUCCESS;
    uint32_t i;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        g_set_active_group_flag = 1;
        GF_LOGD(LOG_TAG "[%s] group_id = %d", __func__, group_id);

        err = gf_algo_set_active_group(group_id);

        cpl_memset(g_finger_index_hash_table, 0,
                g_sensor.config.max_fingers_per_user * sizeof(uint32_t));

        for (i = 0; i < g_sensor.config.max_fingers_per_user; i++) {

            uint32_t finger_id = 0;

            err = gf_ta_load_finger(group_id, i, &finger_id);
            if (GF_SUCCESS == err) {
                GF_LOGI(LOG_TAG "[%s] load group_id(%d), index(%d) success", __func__, group_id, i);
                g_finger_index_hash_table[i] = finger_id;
            } else if (GF_ERROR_REACH_FINGERS_UPLIMIT != err) {
                GF_LOGI(LOG_TAG "[%s] load group_id(%d), index(%d) fail,err(%d)", __func__,
                        group_id, i, err);
                err = GF_SUCCESS;
            } else {
                GF_LOGE(LOG_TAG "[%s] reach to the up limit", __func__);
                err = GF_SUCCESS;
                break;
            }
        }
    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);

    return err;
}

gf_error_t gf_ta_set_active_group(gf_set_active_group_t *cmd) {

    gf_error_t err = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        uint32_t group_id = cmd->group_id;

        if (NULL == cmd) {
            GF_LOGE(LOG_TAG "[%s] bad parameters, cmd:0x%p", __func__, (void *)cmd);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        err = gf_ta_switch_fingerprint_user(group_id);

    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);

    return err;
}

static gf_error_t gf_ta_set_safe_class(gf_set_safe_class_t *cmd) {

    gf_error_t err = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        if (NULL == cmd) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad parameter", __func__);
            break;
        }

        err = gf_algo_set_safe_class(cmd->safe_class);
        g_authenticate_fail_count = 0;
    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);

    return err;
}

gf_error_t gf_ta_enumerate(gf_enumerate_t *cmd) {
    return gf_algo_enumerate(cmd->group_ids, cmd->finger_ids,
            &(cmd->size));
}

static gf_error_t gf_ta_user_enumerate(void *buffer) {
    gf_user_enumerate_t *user_buffer = (gf_user_enumerate_t *) buffer;
    return gf_algo_enumerate(user_buffer->group_ids, user_buffer->finger_ids, &(user_buffer->size));
}

static gf_error_t gf_ta_navigate(gf_cmd_header_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    err = gf_ta_pre_set_mode(MODE_NAV, OPERATION_NAV, &cmd->reset_flag);

    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);

    return err;
}

static gf_error_t gf_ta_get_navigation_data(gf_irq_t *cmd) {

    gf_error_t err = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        uint8_t frame_num = 0;
        uint8_t over_flag = 0;
        uint32_t time_counter = 0;
        UNUSED_VAR(time_counter);

        if ((MODE_NAV != g_cur_mode) || (OPERATION_NAV != g_operation)) {
            GF_LOGE(
                    LOG_TAG "[%s] current mode is not navigation mode, g_cur_mode:%d, g_operation:%d",
                    __func__, g_cur_mode, g_operation);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        time_counter = gf_get_time_delta();

        err = g_spi_function.get_nav_image(&g_fp_raw_data, &g_fp_raw_data_len, &frame_num,
                &over_flag);

        time_counter = gf_get_time_delta();
        GF_LOGD(LOG_TAG "[%s] get_nav_image time = %dms", __func__, time_counter / 1000);

        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] get_nav_image failed err = %d", __func__, err);
            break;
        }

        cpl_memcpy(cmd->navigation.raw_data, g_fp_raw_data,
                g_fp_raw_data_len * sizeof(uint16_t));

        cmd->navigation.raw_data_len = g_fp_raw_data_len;
        cmd->navigation.frame_num = frame_num;
        cmd->navigation.finish_flag = over_flag;

        cmd->operation = g_operation;

        GF_LOGD(LOG_TAG "[%s] got (%d) word navigation data", __func__, g_fp_raw_data_len);

    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);

    return err;
}

static gf_error_t gf_ta_navigate_complete(gf_cmd_header_t *cmd) {

    gf_error_t err = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);
    UNUSED_VAR(cmd);

    if (MODE_NAV == g_cur_mode) {
        err = g_spi_function.nav_complete();
    }

    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);

    return err;
}

static gf_error_t gf_ta_sensor_check(gf_test_check_sensor_test_info_t *cmd) {

    gf_error_t err = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        if (!IS_MILAN_F_SERIES_BY_CHIP_TYPE(g_sensor.chip_type)) {
            cmd->is_pass_sensor_test = 1;
            break;
        }

        err = gf_ta_pre_set_mode(MODE_IDLE, OPERATION_SENSOR_CHECK, &cmd->cmd_header.reset_flag);

        if (0 == cmd->cmd_header.reset_flag) {
            err = g_spi_function.check_sensor();
            if (GF_SUCCESS == err) {
                GF_LOGD(LOG_TAG "[%s] sensor check pass", __func__);
                cmd->is_pass_sensor_test = 1;
                g_sensor_engineer_available = 1;
            } else {
                GF_LOGD(LOG_TAG "[%s] sensor check fail", __func__);
                cmd->is_pass_sensor_test = 0;
                g_sensor_engineer_available = 0;
            }

            gf_ta_auto_cancel(&cmd->cmd_header);
        } else {
            err = GF_ERROR_GENERIC;
            GF_LOGD(LOG_TAG "[%s] need reset to switch to idle mode", __func__);
        }
    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit, err: %d", __func__, err);

    return err;
}

static gf_error_t gf_ta_test_untrusted_enroll(gf_cmd_header_t *cmd) {

    gf_error_t err = GF_SUCCESS;
    GF_LOGD(LOG_TAG "[%s] enter", __func__);
    do {
        err = gf_algo_test_pre_enroll();
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] gf_algo_test_pre_enroll return:%d", __func__, err);
            break;
        }

        err = gf_ta_pre_set_mode(MODE_IMAGE, OPERATION_TEST_UNTRUSTED_ENROLL, &cmd->reset_flag);
    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit, err: %d", __func__, err);
    return err;
}

static gf_error_t gf_ta_bio_check_finger_event(gf_irq_t *cmd) {

    gf_error_t err = GF_SUCCESS;
    uint8_t touch = 0;
    uint8_t temperature = 0;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    err = gf_ta_pre_set_mode(MODE_KEY, OPERATION_TEST_CHECK_FINGER_EVENT,
            &cmd->cmd_header.reset_flag);
    if (GF_SUCCESS == err) {
        gf_sleep(60);
        err = g_spi_function.get_sensor_status(&touch, &temperature);
        if (touch == 1) {
            cmd->irq_type = GF_IRQ_HOMEKEY_DOWN_MASK;
        }
    }

    GF_LOGD(LOG_TAG "[%s] exit", __func__);

    return err;
}

static gf_error_t gf_ta_test_bio_calibration(gf_cmd_header_t *cmd) {

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    return gf_ta_pre_set_mode(MODE_HBD_DEBUG, OPERATION_TEST_BIO_CALIBRATION, &cmd->reset_flag);
}

/* test heartrate */
static gf_error_t gf_ta_test_hbd_calibration(gf_cmd_header_t *cmd) {

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    g_spi_function.download_cfg(CONFIG_TEST_HBD);
    g_is_normal_cfg = 0;

    return gf_ta_pre_set_mode(MODE_HBD_DEBUG, OPERATION_TEST_HBD_CALIBRATION, &cmd->reset_flag);
}

static gf_error_t gf_ta_test_fpc_key_detect(gf_test_fpc_key_data_t* data) {
    gf_error_t err = GF_SUCCESS;
    uint8_t fpc_key_en;
    uint16_t en_addr = 0x0852;  //FPC key enable status address

    GF_LOGD(LOG_TAG "[%s] enter", __func__);
    err = gf_ta_pre_set_mode(MODE_KEY, OPERATION_TEST_FPC_KEY_DETECT, &data->cmd_header.reset_flag);
    g_spi_function.spi_read_byte(en_addr, &fpc_key_en);
    data->fpc_key_en = fpc_key_en;

    //TODO: read fpc rawdata
    //TODO: read fpc cancel register
    GF_LOGD(LOG_TAG "[%s] exit", __func__);
    return err;
}

static gf_error_t gf_ta_test_frr_far_reset_calibration(void) {
    gf_error_t err = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);
    err = gf_frr_far_reset_calibration(0);
    GF_LOGD(LOG_TAG "[%s] exit, err = %d", __func__, err);
    return err;
}

static gf_error_t gf_ta_test_cancel(gf_cancel_t *cmd) {

    gf_error_t err = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    err = gf_ta_cancel(cmd);

    return err;
}

static gf_error_t gf_ta_test_untrusted_authenticate(gf_authenticate_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] enter, group_id = %d", __func__, cmd->group_id);

    do {
        g_operation_id = cmd->operation_id;

        err = gf_ta_pre_set_mode(MODE_IMAGE, OPERATION_TEST_UNTRUSTED_AUTHENTICATE, &cmd->cmd_header.reset_flag);
    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit, err: %d", __func__, err);

    return err;
}

static gf_error_t gf_ta_irq(gf_irq_t *cmd) {

    gf_error_t err = GF_SUCCESS;
    uint32_t irq_type = 0;
    uint8_t frame_num = 0;
    uint8_t over_flag = 0;
    uint32_t do_mode_switch_flag = 1;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        err = g_spi_function.get_irq_type(&irq_type, g_cur_mode);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] get irq type fail:%d", __func__, err);
            break;
        }

        GF_LOGD(LOG_TAG "[%s], g_cur_mode=%d, irq_type=0x%x, operation=%d", __func__, g_cur_mode,
                irq_type, g_operation);

        gf_ta_dump_mode(__func__, g_cur_mode);
        gf_ta_dump_operation(__func__, g_operation);

        cmd->irq_type = irq_type;
        cmd->operation = g_operation;
        cmd->mode = g_cur_mode;

        //ignore irq type
        if (g_ignore_irq_mask & cmd->irq_type) {
            GF_LOGD(LOG_TAG "[%s] ---ignore irq mask[0x%x]!", __func__, g_ignore_irq_mask);
            break;
        }

        if (cmd->irq_type & GF_IRQ_NAV_MASK) {
            if (g_cur_mode == MODE_NAV) {
                err = g_spi_function.get_nav_image(&g_fp_raw_data, &g_fp_raw_data_len, &frame_num,
                        &over_flag);
                if (err != GF_SUCCESS) {
                    GF_LOGE(LOG_TAG "[%s] get_nav_image failed err = %d", __func__, err);
                    cmd->irq_type &= (~GF_IRQ_NAV_MASK);
                } else {
                    if (cmd->dump_data_flag > 0) {
                        cpl_memcpy(cmd->dump_data.raw_data, g_fp_raw_data,
                                g_fp_raw_data_len * sizeof(uint16_t));
                    }
                }

            } else if (g_cur_mode == MODE_NAV_BASE) {
                err = g_spi_function.get_nav_base(&g_fp_raw_data, &g_fp_raw_data_len);
                if (err != GF_SUCCESS) {
                    GF_LOGE(LOG_TAG "[%s] get_nav_base failed err = %d", __func__, err);
                    cmd->irq_type &= (~GF_IRQ_NAV_MASK);
                    g_spi_function.data_sample_resume();
                } else {
                    g_spi_function.data_sample_suspend();

                    if (cmd->dump_data_flag > 0) {
                        cpl_memcpy(cmd->dump_data.raw_data, g_fp_raw_data,
                                g_fp_raw_data_len * sizeof(uint16_t));
                    }
                }
            } else {
                GF_LOGE(LOG_TAG "[%s] Not support this mode %d", __func__, g_cur_mode);
                cmd->irq_type &= (~GF_IRQ_NAV_MASK);
            }
        }

        if (cmd->irq_type & GF_IRQ_IMAGE_MASK) {
            if ((MODE_IMAGE != g_cur_mode) && (MODE_FINGER_BASE != g_cur_mode)
                    && (MODE_FF != g_cur_mode) && (MODE_DEBUG != g_cur_mode)
                    && (MODE_NAV != g_cur_mode) && (MODE_NAV_BASE != g_cur_mode )) {
                GF_LOGE(LOG_TAG "[%s] get IRQ_IMAGE but g_mode = %d, g_operation = %d", __func__,
                        g_cur_mode, g_operation);
                cmd->irq_type &= (~GF_IRQ_IMAGE_MASK);
            } else {
                if(MODE_FINGER_BASE == g_cur_mode){
                    gf_base_press_check_before_get_image();
                }
                gf_get_time_delta();
                err = g_spi_function.get_image(&g_fp_raw_data, &g_fp_raw_data_len, &g_origin_raw_data, &g_origin_raw_data_len);
                if (cmd->dump_data_flag > 0 && NULL != g_origin_raw_data && g_origin_raw_data_len < ORIGIN_DATA_LEN) {
                    cpl_memcpy(cmd->dump_data.origin_data,
                            g_origin_raw_data,
                            g_origin_raw_data_len);
                }
                if (err != GF_SUCCESS) {
                    GF_LOGE(LOG_TAG "[%s] get_image failed err = %d", __func__, err);
                    cmd->irq_type &= (~GF_IRQ_IMAGE_MASK);
                     if (!IS_MILAN_A_SERIES_BY_CHIP_TYPE(g_sensor.chip_type)) {
                        g_spi_function.data_sample_resume();
                    }
                } else {
                    g_image_count++;

                    if (cmd->dump_data_flag > 0) {
                        cpl_memcpy(cmd->dump_data.raw_data,
                                g_fp_raw_data,
                                g_fp_raw_data_len * sizeof(uint16_t));
                    }
                     /*gsc irq and image irq merged,read gsc data after get image*/
                    if ((MODE_IMAGE == g_cur_mode || MODE_FF == g_cur_mode)
                            && g_sensor.config.support_bio_assay > 0
                            && g_sensor.config.support_merged_gsc > 0) {
                        uint8_t *buf = NULL;
                        uint32_t buf_len = 0;

                        err = g_spi_function.get_gsc(&buf, &buf_len);
                        if (err != GF_SUCCESS) {
                            GF_LOGE(LOG_TAG "[%s] get_gsc failed err = %d", __func__, err);
                        }
                    }

                    // bad point test need to sample multiple frame data.
                    if (OPERATION_TEST_BAD_POINT == g_operation) {
                        g_spi_function.data_sample_resume();
                    } else {
                    }
                }
            }
        }

        if (cmd->irq_type & GF_IRQ_GSC_MASK) {
            uint8_t *buf = NULL;
            uint32_t buf_len = 0;

            GF_LOGD(LOG_TAG "[%s] :get gsc.\n", __func__);
            gf_get_time_delta();
            err = g_spi_function.get_gsc(&buf, &buf_len);
            g_dump_performance.get_gsc_data_time = gf_get_time_delta();
            if (err != GF_SUCCESS) {
                GF_LOGE(LOG_TAG "[%s] get_gsc failed err = %d", __func__, err);
                cmd->irq_type &= (~GF_IRQ_GSC_MASK);
            }
        }

        if (cmd->irq_type & GF_IRQ_HBD_MASK) {
            uint8_t *buf = NULL;
            uint32_t buf_len = 0;

            err = g_spi_function.get_hbd(&buf, &buf_len);
            if (err != GF_SUCCESS) {
                GF_LOGE(LOG_TAG "[%s] get_hbd failed err = %d", __func__, err);
                cmd->irq_type &= (~GF_IRQ_HBD_MASK);
            } else {

                switch (g_operation) {
                    case OPERATION_HEARTBEAT_KEY: {
                        err = gf_heart_beat(buf, buf_len, &cmd->heart_beat);
                        if (err != GF_SUCCESS) {
                            GF_LOGE(LOG_TAG "[%s] gf_heart_beat failed err = %d", __func__, err);
                            cmd->irq_type &= (~GF_IRQ_HBD_MASK);
                        }
                        break;
                    }

                    case OPERATION_TEST_BIO_CALIBRATION: {

                        err = g_spi_function.get_hbd_base(&cmd->test_hdb_feature.hbd_base);
                        if (GF_SUCCESS != err) {
                            GF_LOGE(LOG_TAG "[%s] get hdb base fail:%d", __func__, err);
                            break;
                        }

                        err = gf_calutate_hbd_avg(&cmd->test_hdb_feature.hbd_avg, buf, buf_len);
                        if (GF_SUCCESS != err) {
                            break;
                        }

                        gf_ta_pre_set_mode(MODE_IDLE, OPERATION_NONE, &cmd->cmd_header.reset_flag);

                        cpl_memcpy(&cmd->test_hdb_feature.hdb_data, buf, buf_len);
                        cmd->test_hdb_feature.hbd_data_len = buf_len;

                        break;
                    }

                    case OPERATION_TEST_HBD_CALIBRATION: {

                        err = g_spi_function.get_hbd_base(&cmd->test_hdb_feature.hbd_base);
                        if (GF_SUCCESS != err) {
                            GF_LOGE(LOG_TAG "[%s] get hdb base fail:%d", __func__, err);
                            break;
                        }

                        err = gf_calutate_hbd_avg(&cmd->test_hdb_feature.hbd_avg, buf, buf_len);
                        if (GF_SUCCESS != err || buf_len < 24) {
                            /* electricity_value at 23th & 24th byte*/
                            GF_LOGE(LOG_TAG "[%s] get hdb avg fail:%d", __func__, err);
                            break;
                        }

                        cmd->test_hdb_feature.electricity_value = *(uint16_t*) ((void*) (buf
                                + 11 * 2));

                        g_spi_function.download_cfg(CONFIG_NORMAL);
                        g_is_normal_cfg = 1;
                        gf_ta_pre_set_mode(MODE_IDLE, OPERATION_NONE, &cmd->cmd_header.reset_flag);

                        cpl_memcpy(&cmd->test_hdb_feature.hdb_data, buf, buf_len);
                        cmd->test_hdb_feature.hbd_data_len = buf_len;

                        break;
                    }
                    default:
                        break;
                }

            }

            if (cmd->dump_data_flag > 0) {
                cpl_memcpy(&cmd->dump_data.raw_data, buf, buf_len);
                cpl_memcpy(&cmd->dump_data.heart_beat, &cmd->heart_beat, sizeof(gf_heart_beat_t));
            }

            if (NULL != buf) {
                gf_free(buf);
            }

        }

        if ((cmd->irq_type & GF_IRQ_FW_ERR_MASK)
                || (cmd->irq_type & GF_IRQ_CFG_ERR_MASK)
                || (cmd->irq_type & GF_IRQ_ESD_ERR_MASK)) {
            GF_LOGD(LOG_TAG "[%s] :fw,cfg,esd error, ready reset chip.", __func__);
            cmd->cmd_header.reset_flag = 1;
        }

        if ((cmd->irq_type & GF_IRQ_HOMEKEY_DOWN_MASK)
            && (MODE_IMAGE == g_cur_mode || MODE_FF == g_cur_mode
                || ((MODE_NAV == g_cur_mode) && (0 == g_sensor.config.support_fw_navigation)))) {
            g_spi_function.data_sample_resume();
        }

        g_spi_function.clear_irq(irq_type);

        if ((cmd->irq_type & GF_IRQ_IMAGE_MASK)
                && (OPERATION_TEST_BAD_POINT != g_operation)
                && (OPERATION_AUTHENTICATE != g_operation)
                && !(IS_MILAN_A_SERIES_BY_CHIP_TYPE(g_sensor.chip_type))) {
            g_spi_function.data_sample_suspend();
        }

        if (cmd->irq_type & GF_IRQ_UPDATE_BASE_MASK) {
            GF_LOGD(LOG_TAG "[%s] :sensor temperature changed, ready update base.", __func__);
            err = gf_base_on_temperature_change_irq(&cmd->cmd_header.reset_flag);
            break;
        }

        // reset irq allow chip mode != g_cur_mode
        if (cmd->irq_type & GF_IRQ_RESET_MASK) {
            cmd->cmd_header.reset_flag = 1;
            GF_LOGI(LOG_TAG "[%s] get reset irq", __func__);
            //need to reset press flag
            g_finger_pressed_flag = 0;
            g_spi_function.update_temprature_dac();
            if ( MODE_NONE != g_cur_mode) {
                g_spi_function.set_mode(g_cur_mode);
            }

            if (OPERATION_SENSOR_CHECK == g_operation) {
                err = g_spi_function.check_sensor();
                if (GF_SUCCESS == err) {
                    GF_LOGD(LOG_TAG "[%s] sensor check pass", __func__);
                    cmd->test_check_sensor_test_info.is_pass_sensor_test = 1;
                } else {
                    GF_LOGD(LOG_TAG "[%s] sensor check fail", __func__);
                    cmd->test_check_sensor_test_info.is_pass_sensor_test = 0;
                }

                gf_ta_auto_cancel(&cmd->cmd_header);
            }
        }

        if (cmd->irq_type & GF_IRQ_FDT_REVERSE_MASK) {
            GF_LOGI(LOG_TAG "[%s] receive GF_IRQ_FDT_REVERSE", __func__);

            /* Del with reverse interrupt */

            if (0 == ((GF_IRQ_HOMEKEY_DOWN_MASK | GF_IRQ_HOMEKEY_UP_MASK) & cmd->irq_type)) {

                GF_LOGI(LOG_TAG "[%s] receive FDT reverse IRQ", __func__);
                if (MODE_IMAGE == g_cur_mode || MODE_FF == g_cur_mode || MODE_NAV == g_cur_mode) {
                    err = gf_base_on_fdt_reverse_irq(&cmd->cmd_header.reset_flag);
                }
                break;
            }

            /* clear the reverse interrupt when recive the mix interrupt */
            GF_LOGI(LOG_TAG "[%s] receive mix interrupt, firstly clear the reverse interrupt. ",
                    __func__);
            cmd->irq_type &= (~GF_IRQ_FDT_REVERSE_MASK);
        }

        if (cmd->irq_type & GF_IRQ_HOMEKEY_DOWN_MASK) {
            if (OPERATION_AUTHENTICATE == g_operation) {
                if (( (MODE_IMAGE == g_cur_mode) && (1 == g_sensor.config.require_down_and_up_in_pairs_for_image_mode))
                    || ( (MODE_FF == g_cur_mode) && (1 == g_sensor.config.require_down_and_up_in_pairs_for_ff_mode ))) {
                    g_finger_pressed_flag = 1;
                    g_next_mode = g_cur_mode;
                    g_next_operation = g_operation;
                }
            }

            GF_LOGI(LOG_TAG "[%s] down irq g_finger_pressed_flag = %u", __func__, g_finger_pressed_flag);

            if (MODE_IMAGE == g_cur_mode || MODE_FF == g_cur_mode || MODE_NAV == g_cur_mode) {
                g_image_count = 0;
                g_authenticate_fail_count = 0;
                g_study_enable_flag = 1;
                if (!IS_MILAN_A_SERIES_BY_CHIP_TYPE(g_sensor.chip_type))
                    g_spi_function.data_sample_resume();

                gf_base_on_down_irq();
            }

            if (MODE_HBD == g_cur_mode) {
                gf_heart_beat_init();
            }
        } else if (cmd->irq_type & GF_IRQ_HOMEKEY_UP_MASK) {
            /* broken check */
            if (g_sensor.config.support_sensor_broken_check > 0 &&
                (OPERATION_ENROLL == g_operation ||
                OPERATION_AUTHENTICATE == g_operation)) {
                gf_broken_check_on_up_irq(g_fp_raw_data, g_fp_raw_data_len, cmd);
            }

            if (g_finger_pressed_flag > 0) {
                g_finger_pressed_flag = 0;
                gf_ta_set_next_mode(&cmd->cmd_header.reset_flag);
            }

            GF_LOGI(LOG_TAG "[%s] up irq g_finger_pressed_flag = %u", __func__, g_finger_pressed_flag);

            if (IS_MILAN_A_SERIES_BY_CHIP_TYPE(g_sensor.chip_type) && (MODE_NAV == g_cur_mode) &&
                    g_sensor.config.support_fw_navigation == 0) {
                    g_spi_function.data_sample_resume();
            }

            if ((MODE_IMAGE == g_cur_mode) || (MODE_FF == g_cur_mode) || (MODE_NAV == g_cur_mode) || (MODE_SLEEP == g_cur_mode)) {
                //It's not necessary check this in base update when key up at key mode for gf5216.
                if (MODE_KEY != g_cur_mode) {
                    if (0 == g_image_count || g_authenticate_fail_count > 0) {
                        cmd->too_fast_flag = 1;
                    } else {
                        cmd->too_fast_flag = 0;
                    }
                }

                gf_base_on_up_irq(&cmd->cmd_header.reset_flag);

                if (((MODE_IMAGE == g_cur_mode || MODE_FF == g_cur_mode) && g_sensor.config.support_bio_assay > 0) || (MODE_HBD == g_cur_mode && g_sensor.config.support_hbd > 0)) {
                    gf_bio_assay_reset();
                }
            }
        }

        if ((cmd->irq_type & GF_IRQ_IMAGE_MASK)
                || (cmd->irq_type & GF_IRQ_NAV_MASK)) {
            switch (g_operation) {
                case OPERATION_ENROLL:
                case OPERATION_TEST_UNTRUSTED_ENROLL: {
                    int32_t image_quality = 0;
                    int32_t valid_area = 0;

                    err = gf_algo_get_acquired_info(g_fp_raw_data, g_fp_raw_data_len, g_operation,
                            &image_quality, &valid_area);

                    GF_GET_TIME_DELTA_X(g_dump_performance.preprocess_time);
                    if (IS_MILAN_A_SERIES_BY_CHIP_TYPE(g_sensor.chip_type) && err != GF_SUCCESS) {
                        if (GF_ERROR_ENROLL_CONTINUOUS_INVALID_PRESS == err) {
                            GF_LOGE(LOG_TAG "[%s] Invalid press is already enough on 10 times", __func__);
                        }
                        break;
                    }

                    if(g_sensor.config.support_cover_broken_check > 0) {
                        err = gf_algo_check_cover_broken(g_fp_raw_data, g_fp_raw_data_len, &cmd->cover_broken_num, g_operation, &g_retry_flag);
                        cmd->dump_data.cover_broken_flag = 1;
                        GF_GET_TIME_DELTA_X(g_dump_performance.cover_broken_check_time);
                        if (GF_ERROR_COVER_SENSOR_IS_BROKEN == err) {
                            GF_LOGE(LOG_TAG "[%s] cover sensor is broken", __func__);
                            break;
                        }
                    }

                    if (IS_MILAN_A_SERIES_BY_CHIP_TYPE(g_sensor.chip_type)) {
                        err =  g_spi_function.judge_finger_stable(g_dump_performance.preprocess_time);
                        if (err != GF_SUCCESS) {
                            g_image_count = 0;
                            gf_algo_backup_kr();
                            GF_LOGI(LOG_TAG "[%s]Milan_a_series judge stable failed, enroll won't go to algo", __func__);
                            break;
                        }
                    }
                    if (GF_SUCCESS == err) {
                        cmd->preprocess_success_flag = 1;
                        err = gf_ta_image_irq_algo(cmd);
                    }
                    break;
                }

                case OPERATION_TEST_UNTRUSTED_AUTHENTICATE:
                case OPERATION_AUTHENTICATE: {
                    int32_t image_quality = 0;
                    int32_t valid_area = 0;

                    GF_GET_TIME_DELTA_X(g_dump_performance.get_raw_data_time);

                    err = gf_base_on_image_irq(g_fp_raw_data, g_fp_raw_data_len, &cmd->cmd_header.reset_flag);
                    if (err != GF_SUCCESS) {
                        GF_LOGI(LOG_TAG "[%s] FDT INT caused by temperature", __func__);
                        break;
                    }

                    if(g_sensor.config.support_sensor_broken_check > 0 &&
                       g_authenticate_fail_count == 0) {
                        gf_broken_check_on_image_irq(g_fp_raw_data, g_fp_raw_data_len, cmd);
                    }

                    err = gf_algo_get_acquired_info(g_fp_raw_data, g_fp_raw_data_len, g_operation,
                            &image_quality, &valid_area);

                    if (GF_SAFE_CLASS_MEDIUM <= gf_algo_get_safe_class()) {
                        if (g_authenticate_fail_count
                                < (1 == g_screen_on_flag ?
                                        g_sensor.config.screen_on_authenticate_fail_retry_count :
                                        g_sensor.config.screen_off_authenticate_fail_retry_count)) {

                            do_mode_switch_flag = 0;
                        }
                    }

                    if (GF_SUCCESS != err) {
                        break;
                    }

                    GF_GET_TIME_DELTA_X(g_dump_performance.preprocess_time);
                    if (g_sensor.config.support_cover_broken_check > 0) {
                        err = gf_algo_check_cover_broken(g_fp_raw_data, g_fp_raw_data_len, &cmd->cover_broken_num, g_operation, &g_retry_flag);
                        cmd->dump_data.cover_broken_flag = 1;
                        GF_GET_TIME_DELTA_X(g_dump_performance.cover_broken_check_time);
                    }

                    if (IS_MILAN_A_SERIES_BY_CHIP_TYPE(g_sensor.chip_type)) {
                        err =  g_spi_function.judge_finger_stable(g_dump_performance.preprocess_time);
                        if (err != GF_SUCCESS) {
                            g_study_enable_flag = 0;
                            err = GF_SUCCESS;
                            gf_algo_backup_kr();
                            GF_LOGI(LOG_TAG "[%s]Milan_a_series judge stable failed ,authenticate won't study feature", __func__);
                        }
                    }

                    if (GF_SUCCESS == err) {
                        cmd->preprocess_success_flag = 1;
                        err = gf_ta_image_irq_algo(cmd);
                    }
                    break;
                }

                case OPERATION_NAV: {
                    GF_LOGD(LOG_TAG "[%s] OPERATION_NAV", __func__);

                    err = gf_base_on_nav_irq(g_fp_raw_data, g_fp_raw_data_len, &cmd->cmd_header.reset_flag);
                    if (err != GF_SUCCESS) {
                        GF_LOGI(LOG_TAG "[%s] FDT INT caused by temperature", __func__);
                        break;
                    }

                    cpl_memcpy(cmd->navigation.raw_data, g_fp_raw_data,
                            g_fp_raw_data_len * sizeof(uint16_t));

                    cmd->navigation.raw_data_len = g_fp_raw_data_len;
                    cmd->navigation.frame_num = frame_num;
                    cmd->navigation.finish_flag = over_flag;

                    GF_LOGD(LOG_TAG "[%s] data_len : %d", __func__,
                            cmd->navigation.raw_data_len);
                    break;
                }

                case OPERATION_FINGER_BASE: {
                    GF_LOGD(LOG_TAG "[%s] g_operation:%d", __func__, g_operation);
                    err = gf_base_update_finger_base(g_fp_raw_data, g_fp_raw_data_len,
                            &cmd->cmd_header.reset_flag);

                    if (IS_MILAN_F_SERIES_BY_CHIP_TYPE(g_sensor.chip_type)
                            && gf_base_is_nav_base_vaild()) {
                        uint8_t *base = NULL;
                        uint32_t base_len = 0;

                        err = gf_algo_get_nav_base((uint16_t**) &base, &base_len);
                        if (GF_SUCCESS == err) {
                            cpl_memcpy(cmd->navigation.raw_data, base, base_len * sizeof(uint16_t));
                            cmd->navigation.raw_data_len = base_len;
                        }
                    }

                    //use caliparams B translate to rawdata.
                    {
                        uint16_t *base = NULL;
                        uint32_t base_len = 0;
                        uint32_t i = 0;

                        err = gf_algo_get_base_rawdata((uint16_t**) &base, &base_len);
                        if (GF_SUCCESS == err) {
                            if (IS_MILAN_A_SERIES_BY_CHIP_TYPE(g_sensor.chip_type)) {
                                for (i = 0; i < base_len; i++) {
                                    *(cmd->test_bad_point.base_frame_data + i) = *(base + i) - 4095;
                                }
                            } else {
                                cpl_memcpy((uint8_t *) (cmd->test_bad_point.base_frame_data),
                                        (uint8_t *) base, base_len * sizeof(uint16_t));
                            }
                            cmd->test_bad_point.base_frame_len = base_len;
                        }
                    }

                    break;
                }

                case OPERATION_NAV_BASE: {
                    if (IS_MILAN_A_SERIES_BY_CHIP_TYPE(g_sensor.chip_type) && cmd->irq_type & GF_IRQ_IMAGE_MASK) {
                        g_spi_function.data_sample_resume();
                        break;
                    }

                    err = gf_base_update_nav_base(g_fp_raw_data, g_fp_raw_data_len,
                            &cmd->cmd_header.reset_flag);

                    if (GF_OSWEGO_M == g_sensor.chip_type ||
                        (IS_MILAN_A_SERIES_BY_CHIP_TYPE(g_sensor.chip_type) &&
                            0 == g_sensor.config.support_fw_navigation)) {

                        if (gf_base_is_nav_base_vaild()) {
                            cpl_memcpy(cmd->navigation.raw_data, g_fp_raw_data,
                                    g_fp_raw_data_len * sizeof(uint16_t));
                            cmd->navigation.raw_data_len = g_fp_raw_data_len;
                        }
                    }
                    break;
                }

                case OPERATION_TEST_BAD_POINT: {
                    GF_LOGD(LOG_TAG "[%s] OPERATION_TEST_BAD_POINT", __func__);

                    cpl_memcpy((uint8_t *) (cmd->dump_data.raw_data),
                            (uint8_t *) g_fp_raw_data, g_fp_raw_data_len * sizeof(uint16_t));

                    g_bad_point_test_image_count++;
                    cmd->image_count = g_bad_point_test_image_count;

                    if (g_bad_point_test_image_count
                            >= g_sensor.config.bad_point_test_max_frame_number) {
                        gf_ta_auto_cancel(&cmd->cmd_header);
                        g_bad_point_test_image_count = 0;
                    } else {
                        g_spi_function.data_sample_resume();
                    }

                    break;
                }

                case OPERATION_TEST_SENSOR_STEP1:
                    cmd->image_count = g_image_count;

                    if (GF_OSWEGO_M == g_sensor.chip_type) {
                        // ingore the first rawdata
                        if (g_image_count <= 1) {
                            g_spi_function.data_sample_resume();
                            break;
                        }
                    }

                    gf_pixel_test_step1(g_fp_raw_data, g_fp_raw_data_len);

                    if (GF_OSWEGO_M == g_sensor.chip_type) {
                        gf_ta_pre_set_mode(MODE_KEY, OPERATION_NONE, &cmd->cmd_header.reset_flag);
                        err = g_spi_function.download_cfg(CONFIG_TEST_SENSOR_B);
                        g_is_normal_cfg = 0;
                        gf_ta_pre_set_mode(MODE_DEBUG, OPERATION_TEST_SENSOR_STEP2,
                                &cmd->cmd_header.reset_flag);
                        g_image_count = 0;
                    } else {
                        err = g_spi_function.update_dac_for_sensor_pixel_open_test(0, 1);
                        g_is_normal_cfg = 0;
                        g_operation = OPERATION_TEST_SENSOR_STEP2;
                        g_image_count =1;
                    }

                    break;

                case OPERATION_TEST_SENSOR_STEP2:
                    cmd->image_count = g_image_count;

                    if (GF_OSWEGO_M == g_sensor.chip_type) {
                        // ingore the first rawdata
                        if (g_image_count <= 1) {
                            g_spi_function.data_sample_resume();
                            break;
                        }
                    }

                    gf_pixel_test_step2(g_fp_raw_data, g_fp_raw_data_len);

                    err = gf_pixel_test(&(cmd->bad_pixel_num));

                    if (GF_OSWEGO_M == g_sensor.chip_type) {
                        gf_ta_pre_set_mode(MODE_KEY, OPERATION_NONE, &cmd->cmd_header.reset_flag);
                        g_is_normal_cfg = 0;
                    } else {
                        err = g_spi_function.update_dac_for_sensor_pixel_open_test(1, 0);
                        g_is_normal_cfg = 1;
                        cmd->image_count = g_image_count;
                    }
                    break;

                case OPERATION_TEST_PERFORMANCE:

                    cmd->test_performance.get_raw_data_time = gf_get_time_delta();
                    err = gf_algo_test_performance(g_fp_raw_data, g_fp_raw_data_len,
                            &cmd->test_performance);
//                    GF_LOGI(LOG_TAG, "[%s] test performance cmd->dump_data_flag:%d", __func__,cmd->dump_data_flag);
                    if (cmd->dump_data_flag > 0) {
                        cpl_memcpy((uint8_t *) (cmd->dump_data.raw_data),
                                   (uint8_t *) g_fp_raw_data, g_fp_raw_data_len * sizeof(uint16_t));
                        gf_algo_dump_algo_debug_info(cmd->operation, &(cmd->dump_data));
                        cmd->dump_data.image_quality = cmd->test_performance.image_quality;
                        cmd->dump_data.image_valid_area = cmd->test_performance.valid_area;
                        cmd->dump_data.overlap_rate_to_big_temp = cmd->test_performance.overlay;
                        cmd->dump_data.overlap_rate_to_last_temp = cmd->test_performance.increase_rate;
                    }

                    break;
                case OPERATION_TEST_SPI_TRANSFER:
                    GF_LOGD(LOG_TAG "[%s] :aspi_transfer here log", __func__);
                    g_spi_function.data_sample_resume();
                    break;
                case OPERATION_TEST_SPI_PERFORMANCE: {
                    gf_mode_t test_mode;
                    uint8_t fw_version[10] = { 0 };
                    uint32_t offset = 7;
#if 0
                    uint8_t chip_id[GF_CHIP_ID_LEN] = {0};
                    uint8_t vendor_id[GF_VENDOR_ID_LEN] = {0};
                    uint8_t sensor_id[GF_SENSOR_ID_LEN] = {0};
#endif

                    cmd->test_spi_performance.get_image_time = gf_get_time_delta();
                    cmd->test_spi_performance.raw_data_len = g_sensor.sensor_raw_data_len;

                    gf_get_time_delta();
                    cmd->test_spi_performance.get_dr_timestamp_time = gf_get_time_delta();

                    gf_get_time_delta();
                    err = g_spi_function.get_mode(&test_mode);
                    cmd->test_spi_performance.get_mode_time = gf_get_time_delta();

                    gf_get_time_delta();

                    err = g_spi_function.get_fw_version(fw_version, 10);
                    cmd->test_spi_performance.get_fw_version_time = gf_get_time_delta();

                    cpl_memcpy(cmd->test_spi_performance.fw_version, fw_version, 7);

                    cmd->test_spi_performance.fw_version[offset++] = '0'
                            + (fw_version[7] & 0x0f);
                    cmd->test_spi_performance.fw_version[offset++] = '.';
                    cmd->test_spi_performance.fw_version[offset++] = '0'
                            + ((fw_version[8] >> 4) & 0x0f);
                    cmd->test_spi_performance.fw_version[offset++] = '0'
                            + (fw_version[8] & 0x0f);
                    cmd->test_spi_performance.fw_version[offset++] = '.';
                    cmd->test_spi_performance.fw_version[offset++] = '0'
                            + ((fw_version[9] >> 4) & 0x0f);
                    cmd->test_spi_performance.fw_version[offset++] = '0'
                            + (fw_version[9] & 0x0f);
                    cmd->test_spi_performance.fw_version[offset++] = '\0';
#if 0
                    gf_get_time_delta();

                    err = g_spi_function.get_chip_id((uint8_t *) chip_id, GF_CHIP_ID_LEN);
                    cmd->test_spi_performance.get_chip_id_time = gf_get_time_delta();
                    //cpl_memcpy(cmd->chip_id, chip_id, GF_CHIP_ID_LEN);
//                    gf_ta_char_to_string(cmd->test_spi_performance.chip_id, chip_id, 4);

                    gf_get_time_delta();
                    err = g_spi_function.get_vendor_id(vendor_id, GF_VENDOR_ID_LEN);
                    cmd->test_spi_performance.get_vendor_id_time = gf_get_time_delta();
                    //cpl_memcpy(cmd->vendor_id, vendor_id, GF_VENDOR_ID_LEN);
//                    gf_ta_char_to_string(cmd->test_spi_performance.vendor_id, vendor_id, 2);

                    gf_get_time_delta();
                    err = g_spi_function.get_sensor_id(sensor_id, GF_SENSOR_ID_LEN);
                    cmd->test_spi_performance.get_sensor_id_time = gf_get_time_delta();
                    //cpl_memcpy(cmd->sensor_id, sensor_id, GF_SENSOR_ID_LEN);
//                    gf_ta_char_to_string(cmd->test_spi_performance.sensor_id, sensor_id, GF_SENSOR_ID_LEN);
#endif
                    gf_ta_auto_cancel(&cmd->cmd_header);
                    break;
                }

                case OPERATION_TEST_FRR_FAR_RECORD_BASE_FRAME: {
                    GF_LOGD(LOG_TAG "[%s] OPERATION_TEST_FRR_FAR_RECORD_BASE_FRAME", __func__);
                    //err = gf_frr_far_calibration(g_fp_raw_data, g_fp_raw_data_len);

                    if (GF_SUCCESS == err) {
                        cpl_memcpy(cmd->test_frr_far.raw_data, g_fp_raw_data,
                                g_fp_raw_data_len * sizeof(uint16_t));
                        gf_ta_set_normal_mode(&cmd->cmd_header.reset_flag);
                    } else {
                        g_spi_function.data_sample_resume();
                    }
                    break;
                }

                case OPERATION_TEST_FRR_FAR_RECORD_ENROLL: {
                    GF_LOGD(LOG_TAG "[%s] OPERATION_TEST_FRR_FAR_RECORD_ENROLL", __func__);
                    if (IS_MILAN_A_SERIES_BY_CHIP_TYPE(g_sensor.chip_type)) {
                        err = g_spi_function.judge_finger_stable(0);
                        if (err != GF_SUCCESS) {
                            g_image_count = 0;
                            GF_LOGI(LOG_TAG "[%s]Milan_a_series judge stable failed", __func__);
                            break;
                        }
                    }

                    cmd->test_frr_far.check_flag = 1; // close restrict when got template
                    cmd->test_frr_far.data_type = TEST_TOKEN_RAW_DATA; //enroll with rawdata
                    err = gf_frr_far_enroll(g_fp_raw_data, g_fp_raw_data_len,
                            &cmd->test_frr_far);
                    if (GF_SUCCESS == err) {
                        cmd->test_frr_far.check_flag = 0;
                        cpl_memcpy(cmd->test_frr_far.raw_data, g_fp_raw_data,
                                g_fp_raw_data_len * sizeof(uint16_t));

                        err = gf_frr_far_dump_info(&(cmd->test_frr_far));
                    }
                    break;
                }

                case OPERATION_TEST_FRR_FAR_RECORD_AUTHENTICATE: {
                    GF_LOGD(LOG_TAG "[%s] OPERATION_TEST_FRR_FAR_RECORD_AUTHENTICATE", __func__);
                    if (IS_MILAN_A_SERIES_BY_CHIP_TYPE(g_sensor.chip_type)) {
                        err =  g_spi_function.judge_finger_stable(0);
                        if (err != GF_SUCCESS) {
                            g_image_count = 0;
                            GF_LOGI(LOG_TAG "[%s]Milan_a_series judge stable failed", __func__);
                            break;
                        }
                    }

                    err = gf_frr_far_authenticate_with_greedy_strategy(g_fp_raw_data,
                            g_fp_raw_data_len, &cmd->test_frr_far);
                    if (GF_SUCCESS == err) {
                        cpl_memcpy(cmd->test_frr_far.raw_data, g_fp_raw_data,
                                g_fp_raw_data_len * sizeof(uint16_t));
                        err = gf_frr_far_dump_info(&(cmd->test_frr_far));
                    }
                    break;
                }

                case OPERATION_CHECK_FINGER_LONG_PRESS: {

                    err = gf_base_do_finger_long_press_check(g_fp_raw_data, g_fp_raw_data_len,
                            &cmd->cmd_header.reset_flag);
                    if (GF_SUCCESS == err) {
                        GF_LOGI(LOG_TAG "[%s] valid finger long press", __func__);

                        g_spi_function.detect_finger_up();
                    }
                    break;
                }

                default: {
                    GF_LOGE(LOG_TAG "[%s] unknown operation:%d", __func__, g_operation);
                    break;
                }
            }

            if ((OPERATION_ENROLL == g_operation || OPERATION_AUTHENTICATE == g_operation)
                    && (gf_algo_get_preprocess_times() >= 20)) {
                if (gf_base_is_finger_base_vaild()){
                    gf_base_save_calibration();
                }
            }

        }

    } while (0);

    if (1 == do_mode_switch_flag) {
        if (GF_SUCCESS != g_spi_function.switch_mode()) {
            cmd->cmd_header.reset_flag = 1;
            GF_LOGE(LOG_TAG "[%s] auto switch mode fail, so need to reset chip", __func__);
        }
    } else {
        GF_LOGD(LOG_TAG "[%s] don't need to call switch_mode api", __func__);
    }

    if ((1 == gf_algo_get_calibration_state()) && (cmd->dump_finger_base_flag > 0)) {
        uint16_t *base_data = NULL;
        uint32_t base_data_len = 0;

        if (GF_SUCCESS == gf_algo_get_base_rawdata(&base_data, &base_data_len)) {

            cpl_memcpy(cmd->dump_data.raw_data, base_data, base_data_len * sizeof(int16_t));
            cmd->dump_data.is_base_frame_valid = 1;

            gf_algo_dump_algo_debug_info(OPERATION_FINGER_BASE, &(cmd->dump_data));
        }
    }

    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);

    return err;
}

static gf_error_t gf_ta_image_irq_algo(gf_irq_t *cmd) {
    gf_error_t err = GF_SUCCESS;
    uint64_t timestamp = 0;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    GF_GET_TIME_DELTA_X(g_dump_performance.algo_start_time);

    switch (g_operation) {
        case OPERATION_ENROLL:
        case OPERATION_TEST_UNTRUSTED_ENROLL: {
            cmd->samples_remaining = g_sensor.config.enrolling_min_templates;

            if (OPERATION_ENROLL == g_operation) {
                err = gf_algo_enroll(&cmd->group_id,
                    &cmd->finger_id,
                    &cmd->samples_remaining,
                    &cmd->duplicate_finger_id, g_user_id, g_authenticator_id);
            } else {
                err = gf_algo_test_enroll(&cmd->group_id,
                    &cmd->finger_id,
                    &cmd->samples_remaining,
                    &cmd->duplicate_finger_id);
            }

            if (0 == cmd->samples_remaining) {
                gf_ta_auto_cancel(&cmd->cmd_header);
            }
            if (GF_ERROR_ERNOLL_INCOMPLETE_TEMPLATE == err && OPERATION_ENROLL == g_operation) {
                GF_LOGE(
                        LOG_TAG "[%s] pressing is already attained to 30 times, so stop enroll", __func__);
            }
            break;
        }

        case OPERATION_AUTHENTICATE:
        case OPERATION_TEST_UNTRUSTED_AUTHENTICATE: {
            int32_t image_quality = 0;
            int32_t valid_area = 0;
            uint64_t user_id = 0;
            uint64_t authenticator_id = 0;

            cmd->auth_token.challenge = g_operation_id;
            cmd->auth_token.authenticator_id = g_authenticator_id;
            cmd->auth_token.user_id = g_user_id;

            if (OPERATION_AUTHENTICATE == g_operation) {

                err = gf_algo_authenticate(g_study_enable_flag, &cmd->group_id,
                        &cmd->finger_id, &user_id, &authenticator_id,
                        &cmd->update_flag, &image_quality, &valid_area);
            } else {
                err = gf_algo_test_authenticate(0, &cmd->group_id,
                        &cmd->finger_id, &cmd->update_flag,
                        &image_quality, &valid_area);
            }

            if (GF_SUCCESS == err) {
                gf_ta_auto_cancel(&cmd->cmd_header);

                err = gf_get_timestamp(&timestamp);
                if (GF_SUCCESS != err) {
                    GF_LOGE(LOG_TAG "[%s] failed to gf_get_timestamp, ret:%d", __func__, err);
                    break;
                }
                cmd->auth_token.timestamp = ENDIAN_SWAP_U64(timestamp);
                gf_generate_hmac_authenticate(&cmd->auth_token);

                /* Notify the finger info after OPERATION_AUTHENTICATE Success */
                gf_sync_auth_result_notify(cmd->group_id,
                                           cmd->finger_id, GF_SUCCESS);
                g_authenticate_fail_count = 0;
                g_spi_function.switch_mode();
            } else {
                /* Notify the finger info after OPERATION_AUTHENTICATE Failed */
                gf_sync_auth_result_notify(cmd->group_id, cmd->finger_id, GF_ERROR_NOT_MATCH);

                if ((OPERATION_AUTHENTICATE == g_operation) &&
                    (GF_SAFE_CLASS_MEDIUM <= gf_algo_get_safe_class())) {
                    g_authenticate_fail_count++;

                    if (g_authenticate_fail_count
                            <= (1 == g_screen_on_flag ?
                                    g_sensor.config.screen_on_authenticate_fail_retry_count :
                                    g_sensor.config.screen_off_authenticate_fail_retry_count)) {
                        g_retry_flag = 1;
                        g_spi_function.data_sample_resume();
                        err = GF_ERROR_MATCH_FAIL_AND_RETRY;
                        GF_LOGI(
                                LOG_TAG "[%s] authenticate fail, need retry, g_authenticate_fail_times:%d",
                                __func__, g_authenticate_fail_count);
                        break;
                    } else {
                        g_spi_function.data_sample_suspend();
                        if (IS_MILAN_A_SERIES_BY_CHIP_TYPE(g_sensor.chip_type)) {
                            gf_algo_print_mean_data(g_fp_raw_data, g_sensor.row, g_sensor.col);
                        }
                    }

                }
                g_authenticate_fail_count = 0;

                g_spi_function.switch_mode();
            }
            break;
        }

        default:
            break;
    }

    if (cmd->dump_data_flag > 0) {
        uint32_t raw_data_len_in_bytes = g_fp_raw_data_len * sizeof(uint16_t);

        cpl_memcpy(cmd->dump_data.raw_data, g_fp_raw_data,
                raw_data_len_in_bytes);

        if (g_sensor.config.support_sensor_broken_check > 0) {
            gf_broken_check_dump(cmd->dump_data.raw_data_for_sensor_broken_check,
                                 IMAGE_BUFFER_LEN * 2,
                                 &cmd->dump_data.sensor_broken_check_frame_num);
            GF_LOGD(LOG_TAG "[%s] broken check dumped %u base frame",
                    __func__, cmd->dump_data.sensor_broken_check_frame_num);
        }

        gf_algo_dump_algo_debug_info(cmd->operation,
                &(cmd->dump_data));
    }

    if (GF_PERFORMANCE_DUMP_ENABLED) {
        cpl_memcpy(&cmd->test_performance, &g_dump_performance,
                sizeof(gf_test_performance_t));
    }

    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);

    return err;
}

static gf_error_t gf_ta_screen_on(gf_cmd_header_t *cmd) {
    gf_mode_t mode = MODE_NONE;
    gf_operation_type_t operation = OPERATION_NONE;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    g_screen_on_flag = 1;
    g_authenticate_fail_count = 0;

    do {
        GF_LOGD(LOG_TAG "[%s] g_operation:%d, g_authenticate_is_cancel:%d", __func__, g_operation, g_authenticate_is_cancel);
        if ((OPERATION_AUTHENTICATE == g_operation
                && (g_authenticate_is_cancel == 0 || OPERATION_AUTHENTICATE == g_next_operation))
                || (OPERATION_FINGER_BASE == g_operation
                        && OPERATION_AUTHENTICATE == g_next_operation)
                || (OPERATION_NAV_BASE == g_operation && OPERATION_AUTHENTICATE == g_next_operation)) {
            GF_LOGD(LOG_TAG "[%s] g_operation:%d, g_next_operation:%d, set mode to image and operation to authenticate.", __func__, g_operation, g_next_operation);
            mode = MODE_IMAGE;
            operation = OPERATION_AUTHENTICATE;
            break;
        }

        if (OPERATION_NAV == g_operation) {
            mode = MODE_NAV;
            operation = OPERATION_NAV;
            break;
        }

        if (OPERATION_CAMERA_KEY == g_operation) {
            mode = MODE_KEY;
            operation = OPERATION_CAMERA_KEY;
            break;
        }

        if (g_sensor.config.support_nav_mode > GF_NAV_MODE_NONE) {
            mode = MODE_NAV;
            operation = OPERATION_NAV;
        } else if (g_sensor.config.support_key_mode > 0) {
            mode = MODE_KEY;
            operation = OPERATION_HOME_KEY;
        } else {
            mode = MODE_SLEEP;
            operation = OPERATION_NONE;
        }
    } while(0);

    return gf_ta_pre_set_mode(mode, operation, &cmd->reset_flag);
}

static gf_error_t gf_ta_screen_off(gf_cmd_header_t *cmd) {
    gf_mode_t mode = MODE_FF;
    gf_operation_type_t operation = OPERATION_NONE;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    g_screen_on_flag = 0;
    g_authenticate_fail_count = 0;

    if (0 == gf_algo_has_enrolled_fingerprints()) {
        if (g_finger_pressed_flag > 0) {
            g_finger_pressed_flag = 0;
            gf_ta_set_next_mode(&cmd->reset_flag);
        }
        if ((g_sensor.config.support_ff_mode > 0)
                && (g_sensor.config.support_power_key_feature > 0)) {
            mode = MODE_FF;
            operation = OPERATION_POWER_KEY;
        } else {
            mode = MODE_SLEEP;
            operation = OPERATION_NONE;
        }
    } else if (g_sensor.config.support_ff_mode > 0) {
        if (OPERATION_AUTHENTICATE == g_operation
                || (OPERATION_FINGER_BASE == g_operation
                        && OPERATION_AUTHENTICATE == g_next_operation)
                || (OPERATION_NAV_BASE == g_operation && OPERATION_AUTHENTICATE == g_next_operation)) {
            GF_LOGD(LOG_TAG "[%s] g_operation:%d, g_next_operation:%d, set mode to ff and operation to authenticate.", __func__, g_operation, g_next_operation);
            mode = MODE_FF;
            operation = OPERATION_AUTHENTICATE;
        } else {
            mode = MODE_SLEEP;
            operation = OPERATION_NONE;
        }
    } else {
        if (OPERATION_AUTHENTICATE == g_operation || (OPERATION_FINGER_BASE == g_operation && OPERATION_AUTHENTICATE == g_next_operation)) {
            GF_LOGD(LOG_TAG "[%s] g_operation:%d, g_next_operation:%d, set mode to sleep and operation to authenticate.", __func__, g_operation, g_next_operation);
            mode = MODE_SLEEP;
            operation = OPERATION_AUTHENTICATE;
        } else {
            mode = MODE_SLEEP;
            operation = OPERATION_NONE;
        }
    }

    return gf_ta_pre_set_mode(mode, operation, &cmd->reset_flag);
}

static gf_error_t gf_ta_esd_check(gf_esd_check_t *cmd) {
    gf_error_t err = GF_SUCCESS;
    uint8_t result = 0;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        if (0 == g_screen_on_flag || MODE_SLEEP == g_cur_mode || MODE_FF == g_cur_mode
                || MODE_DEBUG == g_cur_mode) {
            GF_LOGE(LOG_TAG "[%s] can't esd check screen_on = %d, mode = %d", __func__,
                    g_screen_on_flag, g_cur_mode);
            break;
        }

        err = g_spi_function.esd_check(&result);
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

    return err;
}

static gf_error_t gf_ta_camera_capture(gf_cmd_header_t *cmd) {
    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    return gf_ta_pre_set_mode(MODE_KEY, OPERATION_CAMERA_KEY, &cmd->reset_flag);
}

static gf_error_t gf_ta_enable_fingerprint_module(gf_enable_fingerprint_module_t *cmd) {
    gf_error_t err = GF_SUCCESS;
    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    if (cmd->enable_flag == 0) {
        err = gf_ta_pre_set_mode(MODE_SLEEP, OPERATION_NONE, &cmd->cmd_header.reset_flag);
    } else {
        err = gf_ta_auto_cancel(&cmd->cmd_header);
    }

    return err;
}

static gf_error_t gf_ta_enable_ff_feature(gf_enable_ff_feature_t *cmd) {
    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    g_sensor.config.support_ff_mode = cmd->enable_flag;

    return GF_SUCCESS;
}

static gf_error_t gf_ta_user_get_authenticator_version(void *buffer) {
    gf_user_authenticator_version_t *user_buffer = (gf_user_authenticator_version_t *)buffer;
    user_buffer->version = GF_AUTHENTICATOR_VERSION_CODE;

    return GF_SUCCESS;
}

static gf_error_t gf_ta_test_bad_point(gf_cmd_header_t *cmd) {
    UNUSED_VAR(cmd);

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    return gf_ta_pre_set_mode(MODE_IMAGE, OPERATION_TEST_BAD_POINT, &cmd->reset_flag);
}

static gf_error_t gf_ta_test_sensor(gf_cmd_header_t *cmd) {
    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    return gf_ta_pre_set_mode(MODE_KEY, OPERATION_NONE, &cmd->reset_flag);
}

static gf_error_t gf_ta_test_sensor_step1(gf_cmd_header_t *cmd) {
    gf_error_t err = GF_SUCCESS;
    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        err = g_spi_function.download_cfg(CONFIG_TEST_SENSOR_A);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] write config1 failed", __func__);
            break;
        }

        g_image_count = 0;

        err = gf_ta_pre_set_mode(MODE_DEBUG, OPERATION_TEST_SENSOR_STEP1, &cmd->reset_flag);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] set debug mode failed", __func__);
            break;
        }

    } while (0);

    return err;
}

static gf_error_t gf_ta_test_sensor_finish(gf_cmd_header_t *cmd) {
    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    g_image_count = 0;

    g_spi_function.download_cfg(CONFIG_NORMAL);

    g_is_normal_cfg  = 1;

    gf_ta_auto_cancel(cmd);

    return GF_SUCCESS;
}

static gf_error_t gf_ta_test_performance(gf_cmd_header_t *cmd) {
    gf_error_t err;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);
    err = gf_ta_pre_set_mode(MODE_IMAGE, OPERATION_TEST_PERFORMANCE, &cmd->reset_flag);
    g_spi_function.data_sample_resume();
    return err;
}

static gf_error_t gf_ta_test_spi_performance(gf_cmd_header_t *cmd) {
    return gf_ta_pre_set_mode(MODE_FINGER_BASE, OPERATION_TEST_SPI_PERFORMANCE, &cmd->reset_flag);
}

static gf_error_t gf_ta_test_spi_transfer(gf_cmd_header_t *cmd) {
    g_next_mode = g_cur_mode;
    g_next_operation = g_operation;
    GF_LOGD(LOG_TAG "[%s] :aspi_transfer here log", __func__);

    return gf_ta_pre_set_mode(MODE_DEBUG, OPERATION_TEST_SPI_TRANSFER, &cmd->reset_flag);
}

static gf_error_t gf_ta_test_pre_spi(gf_cmd_header_t *cmd) {
    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    if (IS_MILAN_F_SERIES_BY_CHIP_TYPE(g_sensor.chip_type)) {
        return gf_ta_pre_set_mode(MODE_IDLE, OPERATION_NONE, &cmd->reset_flag);
    } else {
        return gf_ta_pre_set_mode(MODE_IMAGE, OPERATION_NONE, &cmd->reset_flag);
    }
}

static gf_error_t gf_ta_test_spi(gf_test_spi_t *cmd) {

    gf_error_t err = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        if (IS_MILAN_F_SERIES_BY_CHIP_TYPE(g_sensor.chip_type)) {
            err = g_spi_function.get_chip_id(cmd->chip_id, GF_CHIP_ID_LEN);
            if (err != GF_SUCCESS) {
                GF_LOGE(LOG_TAG "[%s] get chip_id fail", __func__);
            }
        } else { // read firmware version for milan ab and oswego
            uint32_t offset = 7;
            uint8_t fw_version[10] = { 0 };

            err = g_spi_function.get_fw_version(fw_version, 10);
            if (GF_SUCCESS != err) {
                GF_LOGE(LOG_TAG "[%s] get firmware version fail, err:%d", __func__, err);
                break;
            }

            cpl_memcpy(cmd->fw_version, fw_version, 7);

            cmd->fw_version[offset++] = '0' + (fw_version[7] & 0x0f);
            cmd->fw_version[offset++] = '.';
            cmd->fw_version[offset++] = '0' + ((fw_version[8] >> 4) & 0x0f);
            cmd->fw_version[offset++] = '0' + (fw_version[8] & 0x0f);
            cmd->fw_version[offset++] = '.';
            cmd->fw_version[offset++] = '0' + ((fw_version[9] >> 4) & 0x0f);
            cmd->fw_version[offset++] = '0' + (fw_version[9] & 0x0f);
            cmd->fw_version[offset++] = '\0';

            if(IS_MILAN_A_SERIES_BY_CHIP_TYPE(g_sensor.chip_type)){
                cmd->sensor_otp_type = g_sensor.fw_cfg.milan_a_series.sensor_otp_type;
            }

        }
    } while (0);

    gf_ta_auto_cancel(&cmd->cmd_header);
    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);

    return err;
}

static gf_error_t gf_ta_test_reset_pin(gf_cmd_header_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);
    err = gf_ta_pre_set_mode(MODE_IMAGE, OPERATION_TEST_RESET_PIN, &cmd->reset_flag);
    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);

    return err;
}

static gf_error_t gf_ta_reset_pin_polling_e4(gf_cmd_header_t *cmd) {
    gf_error_t err = GF_ERROR_TEST_RESET_PIN;
    uint32_t irq_type = 0;
    uint16_t count;
    UNUSED_VAR(cmd);

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    for(count = 0; count < MILAN_GET_RESET_IRQ_MAX_COUNT; count++) {
        g_spi_function.get_irq_type(&irq_type, g_cur_mode);

        GF_LOGD(LOG_TAG "[%s]  irq_type:0x%x", __func__, irq_type);

        if (irq_type & (GF_IRQ_RESET_MASK)) {
            err = GF_SUCCESS;
            break;
        }

        gf_sleep(5);
    }

    g_spi_function.clear_irq(irq_type);
    gf_ta_set_mode(g_cur_mode,g_operation);

    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);

    return err;
}

static gf_error_t gf_ta_user_test_dump_templates(void *buffer) {

    gf_error_t err = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        gf_user_dump_templates_t *user_buffer = (gf_user_dump_templates_t *)buffer;
        uint32_t group_id = user_buffer->group_id;
        uint32_t finger_id = user_buffer->finger_id;

        err = gf_algo_get_fingerTempInfo_size(finger_id, &(user_buffer->buf_len));
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] get finger(%u) size fail:%d", __func__, finger_id, err);
            break;
        }

        err = gf_algo_get_finger_pdu(group_id, finger_id, user_buffer->buf, user_buffer->buf_len);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] fail to get data,err:%d", __func__, err);
            break;
        }

    } while(0);

    return err;
}

static gf_error_t gf_ta_test_pre_get_version(gf_cmd_header_t *cmd) {
    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    return gf_ta_pre_set_mode(MODE_KEY, OPERATION_NONE, &cmd->reset_flag);
}

static gf_error_t gf_ta_test_get_version(gf_test_get_version_t *cmd) {
    gf_error_t err = GF_SUCCESS;
    uint8_t fw_version[10] = { 0 };
    uint32_t offset = 7;
    uint8_t chip_id[GF_CHIP_ID_LEN] = { 0 };
    uint8_t vendor_id[64] = { 0 };
    uint32_t irq_type = 0;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);
    do {
        g_spi_function.get_irq_type(&irq_type, g_cur_mode);
        g_spi_function.clear_irq(irq_type);

        err = gf_algo_test_get_algo_version(cmd->algo_version,
                cmd->preprocess_version);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] get algo_version and preprocess_version fail", __func__);
            break;
        }

        err = g_spi_function.get_fw_version(fw_version, 10);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] get fw_version fail", __func__);
            break;
        }

        cpl_memcpy(cmd->fw_version, fw_version, 7);

        cmd->fw_version[offset++] = '0' + (fw_version[7] & 0x0f);
        cmd->fw_version[offset++] = '.';
        cmd->fw_version[offset++] = '0' + ((fw_version[8] >> 4) & 0x0f);
        cmd->fw_version[offset++] = '0' + (fw_version[8] & 0x0f);
        cmd->fw_version[offset++] = '.';
        cmd->fw_version[offset++] = '0' + ((fw_version[9] >> 4) & 0x0f);
        cmd->fw_version[offset++] = '0' + (fw_version[9] & 0x0f);
        cmd->fw_version[offset++] = '\0';

        gf_get_tee_version(cmd->tee_version, TEE_VERSION_INFO_LEN);

        cpl_memcpy(cmd->ta_version, GF_TA_VERSION,
                cpl_strlen((const int8_t*) GF_TA_VERSION));

        if (g_spi_function.get_chip_id) {

            err = g_spi_function.get_chip_id((uint8_t *) chip_id, GF_CHIP_ID_LEN);
            if (err != GF_SUCCESS) {
                break;
            }

            GF_LOGD(LOG_TAG "[%s] vendor_id = 0x%02x, 0x%02x, 0x%02x, 0x%02x", __func__, chip_id[0],
                    chip_id[1], chip_id[2], chip_id[3]);
            if (IS_MILAN_A_SERIES_BY_CHIP_TYPE(g_sensor.chip_type)) {
                GF_LOGD(LOG_TAG "[%s] chip_id = %s", __func__, chip_id);
                cpl_memcpy(cmd->chip_id, chip_id, 2);
            } else {
                cpl_memcpy(cmd->chip_id, chip_id, GF_CHIP_ID_LEN);
            }
        }

        if (IS_MILAN_A_SERIES_BY_CHIP_TYPE(g_sensor.chip_type)) {
            // 4 bytes in vendor_id, buf[0] for vendor_id, buf[1] for year, buf[2] for month, buf[3] for date
            g_spi_function.get_vendor_id(vendor_id, GF_VENDOR_ID_LEN);
            GF_LOGD(LOG_TAG "[%s] vendor_id = 0x%02x", __func__, vendor_id[0]);
            cpl_memcpy(cmd->vendor_id, &(vendor_id[0]), 1);
            cmd->vendor_id[0] = vendor_id[0];
            GF_LOGD(LOG_TAG "[%s] production_date = %d--%d--%d.", __func__, vendor_id[1]+2016,vendor_id[2],vendor_id[3]);
            cpl_memcpy(cmd->production_date, &(vendor_id[1]), 3);

            cpl_memcpy(cmd->heart_beat_algo_version, gf_get_heart_beat_version(), GF_HEART_BEAT_ALGO_VERSION_LEN);
            cmd->sensor_otp_type = g_sensor.fw_cfg.milan_a_series.sensor_otp_type;
        } else {
            g_spi_function.get_vendor_id(vendor_id, 64);
            GF_LOGD(LOG_TAG "[%s] vendor_id = 0x%02x, 0x%02x, 0x%02x, 0x%02x", __func__, vendor_id[0],
                    vendor_id[1], vendor_id[2], vendor_id[3]);
            cpl_memcpy(cmd->vendor_id, vendor_id, 4);

            GF_LOGD(LOG_TAG "[%s] production_date = 0x%02x, 0x%02x, 0x%02x, 0x%02x", __func__,
                    vendor_id[0x38], vendor_id[0x39], vendor_id[0x3a], vendor_id[0x3b]);
            cpl_memcpy(cmd->production_date, &(vendor_id[0x38]), 4);
        }
        gf_ta_auto_cancel(&cmd->cmd_header);
    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);
    return err;
}
static gf_error_t gf_ta_test_frr_far_load_group_finger(void) {
    gf_error_t err = GF_SUCCESS;
    uint8_t *buf = NULL;
    uint32_t buf_len = 0;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {

        err = gf_so_load_persistent_object((int8_t*) GF_TEST_FRR_FAR_FINGER_LST_OBJECT_ID, &buf, &buf_len);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] load so(%s) failed", __func__, GF_TEST_FRR_FAR_FINGER_LST_OBJECT_ID);
            break;
        }

        err = gf_frr_far_decode_finger_list(buf, buf_len);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] err, errno=%d", __func__, err);
            break;
        }

        print_finger_list();

    } while (0);

    if (buf != NULL) {
        CPL_MEM_FREE(buf);
    }

    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);
    return err;
}

static gf_error_t gf_ta_test_frr_far_save_group_finger(void) {
    gf_error_t err = GF_SUCCESS;
    uint8_t *buf = NULL;
    uint32_t buf_len = 0;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);
    do {
        if (g_frr_far_finger_list.finger == NULL) {
            err = gf_so_delete_persistent_object((int8_t*)GF_TEST_FRR_FAR_FINGER_LST_OBJECT_ID);
            GF_LOGE(LOG_TAG "[%s] delete (%s)  errno=%d", __func__,
                    GF_TEST_FRR_FAR_FINGER_LST_OBJECT_ID, err);
            break;
        }

        err = gf_frr_far_get_finger_list_size(&buf_len);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] get finger list size failed", __func__);
            break;
        }

        buf = (uint8_t *) CPL_MEM_MALLOC(buf_len);
        if (NULL == buf) {
            err = GF_ERROR_OUT_OF_MEMORY;
            GF_LOGE(
                    LOG_TAG "[%s] buf out of memory. errno=%d", __func__,  err);
            break;
        }

        err = gf_frr_far_encode_finger_list(buf, buf_len);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] err, errno=%d", __func__,  err);
            break;
        }

        err = gf_so_save_persistent_object((int8_t*)GF_TEST_FRR_FAR_FINGER_LST_OBJECT_ID, buf, buf_len);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] save so(%s) failed", __func__, GF_TEST_FRR_FAR_FINGER_LST_OBJECT_ID);
        }

    } while (0);

    if (buf != NULL) {
        CPL_MEM_FREE(buf);
    }

    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);
    return err;
}

static gf_error_t gf_ta_test_frr_far_init(gf_test_frr_far_init_t *cmd) {
    gf_error_t err = GF_SUCCESS;
    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        err = gf_frr_far_init(cmd);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] exit, init fail", __func__);
            break;
        }

        if (g_frr_far_finger_list.finger == NULL) {
            gf_ta_test_frr_far_load_group_finger();
        }

    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);
    return err;
}

static gf_error_t gf_ta_test_frr_far_record_base_frame(gf_cmd_header_t *cmd) {

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    return gf_ta_pre_set_mode(MODE_FINGER_BASE, OPERATION_TEST_FRR_FAR_RECORD_BASE_FRAME,
            &cmd->reset_flag);
}

static gf_error_t gf_ta_test_frr_far_record_enroll(gf_cmd_header_t *cmd) {

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    return gf_ta_pre_set_mode(MODE_IMAGE, OPERATION_TEST_FRR_FAR_RECORD_ENROLL, &cmd->reset_flag);
}

static gf_error_t gf_ta_test_frr_far_record_authenticate(gf_cmd_header_t *cmd) {
    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    return gf_ta_pre_set_mode(MODE_IMAGE, OPERATION_TEST_FRR_FAR_RECORD_AUTHENTICATE,
            &cmd->reset_flag);
}

static gf_error_t gf_ta_test_frr_far_record_authenticate_finish(gf_cmd_header_t *cmd) {
    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    return gf_ta_auto_cancel(cmd);
}

static gf_error_t gf_ta_test_frr_far_play_base_frame(gf_test_frr_far_t *cmd) {
    GF_LOGD(LOG_TAG "[%s] enter", __func__);
    UNUSED_VAR(cmd);
    return GF_SUCCESS;
}

static gf_error_t gf_ta_test_pre_download_fw_cfg(gf_cmd_header_t *cmd) {
    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    return gf_ta_pre_set_mode(MODE_KEY, OPERATION_NONE, &cmd->reset_flag);
}

static gf_error_t gf_ta_test_download_fw_cfg(gf_test_download_fw_cfg_t *cmd) {
    gf_error_t err = GF_SUCCESS;
    GF_LOGD(LOG_TAG "[%s] enter", __func__);
    gf_ta_pre_set_mode(MODE_KEY, OPERATION_NONE, &cmd->cmd_header.reset_flag);
    err = g_spi_function.test_download_fw_cfg(cmd->fw_cfg_data, cmd->fw_cfg_data_len, cmd->fw_cfg_status);
    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);
    return err;
}

static gf_error_t gf_ta_test_set_config(gf_test_set_config_t *cmd) {
    gf_algo_basic_config_t basic_config = { 0 };
    GF_LOGD(LOG_TAG "[%s] enter", __func__);
    if ((cmd->config.reissue_key_down_when_entry_image_mode
            != g_sensor.config.reissue_key_down_when_entry_image_mode)
            || (cmd->config.reissue_key_down_when_entry_ff_mode
                    != g_sensor.config.reissue_key_down_when_entry_ff_mode)) {
        g_sensor.config.reissue_key_down_when_entry_image_mode = cmd->config.reissue_key_down_when_entry_image_mode;
        g_sensor.config.reissue_key_down_when_entry_ff_mode = cmd->config.reissue_key_down_when_entry_ff_mode;
        g_spi_function.update_cfg_for_reissue_key_down();
    }
    cpl_memcpy(&g_sensor.config, &cmd->config, sizeof(gf_config_t));

    basic_config.duplicate_finger_overlay_score = g_sensor.config.duplicate_finger_overlay_score;
    basic_config.enrolling_min_templates = g_sensor.config.enrolling_min_templates;
    basic_config.forbidden_enroll_duplicate_fingers =
            g_sensor.config.forbidden_enroll_duplicate_fingers;
    basic_config.increase_rate_between_stitch_info =
            g_sensor.config.increase_rate_between_stitch_info;
    basic_config.max_fingers_per_user = g_sensor.config.max_fingers_per_user;
    basic_config.valid_image_area_threshold = g_sensor.config.valid_image_area_threshold;
    basic_config.valid_image_quality_threshold = g_sensor.config.valid_image_quality_threshold;

    if (g_sensor.chip_type == GF_MILAN_A|| g_sensor.chip_type == GF_MILAN_C) {
        basic_config.support_bio_assay = cmd->config.support_bio_assay;
    } else {
        basic_config.support_bio_assay = 0;
        g_sensor.config.support_bio_assay = 0;
    }

    gf_algo_update_config(&basic_config);

    cpl_memcpy(&cmd->config, &g_sensor.config, sizeof(gf_config_t));

    switch (cmd->token) {
        case TEST_TOKEN_SUPPORT_KEY_MODE:
        case TEST_TOKEN_SUPPORT_NAV_MODE:
            gf_ta_auto_cancel(&cmd->cmd_header);
            break;

        default:
            break;
    }

    return GF_SUCCESS;
}

static gf_error_t gf_ta_test_frr_far_play_enroll(gf_test_frr_far_t *cmd) {

    gf_error_t err = GF_SUCCESS;
    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    cmd->check_flag = 1;
    err = gf_frr_far_enroll(cmd->raw_data, g_sensor.raw_data_len, cmd);
    gf_frr_far_dump_info(cmd);

    return err;

}

static gf_error_t gf_ta_test_frr_far_play_authenticate(gf_test_frr_far_t *cmd) {

    gf_error_t err = GF_SUCCESS;
    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    err = gf_frr_far_authenticate(cmd->raw_data, g_sensor.raw_data_len, cmd);
    gf_frr_far_dump_info(cmd);

    return err;
}

static gf_error_t gf_ta_test_frr_far_enroll_finish(gf_cmd_header_t *cmd) {
    UNUSED_VAR(cmd);

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    if (g_operation == OPERATION_TEST_FRR_FAR_RECORD_ENROLL) {
        gf_ta_set_mode(MODE_IMAGE, OPERATION_NONE);
    }

    gf_frr_far_delete_tran();
    return GF_SUCCESS;
}

static gf_error_t gf_ta_test_frr_far_cancel(gf_cancel_t *cmd) {
    UNUSED_VAR(cmd);

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    gf_frr_far_cancel();

    return GF_SUCCESS;
}

static gf_error_t gf_ta_test_frr_far_save(gf_test_frr_far_save_t *cmd) {

    gf_error_t err = GF_SUCCESS;
    uint8_t *buf = NULL;
    uint32_t buf_len = 0;
    uint32_t group_id = 0;
    uint32_t finger_id = 0;
    uint8_t name[GF_FINGER_OBJECT_ID_MAX_LEN] = { 0 };
    gf_test_frr_far_finger_t* finger = NULL;
    gf_test_frr_far_finger_t* last = NULL;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);
    do {
        if (NULL == cmd) {
            GF_LOGE(LOG_TAG "[%s] bad parameter", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        group_id = cmd->group_id;
        finger_id = cmd->finger_id;
        GF_LOGD(LOG_TAG "[%s] group_id=%u, finger_id=%u", __func__, group_id, finger_id);
        GF_LOGD(LOG_TAG "[%s] finger_name=%s", __func__, cmd->finger_name);
        cpl_sprintf((int8_t*) name, (const int8_t*) GF_TEST_FRR_FAR_FINGER_OBJECT_ID, group_id, finger_id);

        err = gf_frr_far_get_fingerTempInfo_size(&buf_len);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] get finger(%u) size failed", __func__, finger_id);
            break;
        }

        buf = (uint8_t *) CPL_MEM_MALLOC(buf_len);
        if (NULL == buf) {
            err = GF_ERROR_OUT_OF_MEMORY;
            GF_LOGE(
                    LOG_TAG "[%s] buf out of memory. errno=%d", __func__, err);
            break;
        }

        err = gf_frr_far_encode_fingertempinfo(buf, buf_len);
        if (err != GF_SUCCESS) {
            break;
        }

        err = gf_so_save_persistent_object((int8_t*) name, buf, buf_len);
        if (GF_SUCCESS != err) {
            GF_LOGE( LOG_TAG "[%s] fail to save finger(%u)'s so(%s)", __func__, finger_id, name);
            break;
        }

        finger = (gf_test_frr_far_finger_t*) CPL_MEM_MALLOC(sizeof(gf_test_frr_far_finger_t));
        finger->group_id = group_id;
        finger->finger_id = finger_id;
        finger->next = NULL;

        if (g_frr_far_finger_list.finger != NULL) {
            last = g_frr_far_finger_list.finger;
            while (last->next != NULL) {
                last = last->next;
            }

            last->next = finger;

        } else {
            g_frr_far_finger_list.finger = finger;
        }

        g_frr_far_finger_list.count++;

        //print_finger_list();

        gf_ta_test_frr_far_save_group_finger();
    } while (0);

    if (buf != NULL) {
        CPL_MEM_FREE(buf);
    }
    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);

    return err;
}

static gf_error_t gf_ta_test_frr_far_remove(gf_test_frr_far_remove_t *cmd) {
    gf_error_t err = GF_SUCCESS;
    uint32_t group_id = cmd->group_id;
    uint32_t finger_id = cmd->finger_id;
    uint8_t name[GF_FINGER_OBJECT_ID_MAX_LEN] = { 0 };
    gf_test_frr_far_finger_t* pre_finger = g_frr_far_finger_list.finger;
    gf_test_frr_far_finger_t* finger = pre_finger;
    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        GF_LOGD(LOG_TAG "[%s] group_id=%u, finger_id=%u", __func__, group_id, finger_id);

        if (group_id == 0 && finger_id == 0) { //delete all
            while (NULL != finger) {
                GF_LOGI(LOG_TAG "[%s] finger_id=%u, group_id=%u", __func__, finger->finger_id,
                        finger->group_id);
                finger_id = finger->finger_id;
                group_id = finger->group_id;

                cpl_sprintf((int8_t*) name, (const int8_t*) GF_TEST_FRR_FAR_FINGER_OBJECT_ID, group_id, finger_id);
                err = gf_so_delete_persistent_object((int8_t*) name);
                if (GF_SUCCESS != err) {
                    GF_LOGE(
                        LOG_TAG "[%s] fail to delete so(%s) for finger(%u)", __func__, name, finger_id);
                    continue;
                }

                pre_finger = finger;
                finger = finger->next;
                g_frr_far_finger_list.count--;
                CPL_MEM_FREE(pre_finger);
                pre_finger = NULL;
            }

            g_frr_far_finger_list.finger = NULL;

        } else {
            cpl_sprintf((int8_t*) name, (const int8_t*) GF_TEST_FRR_FAR_FINGER_OBJECT_ID, group_id, finger_id);

            err = gf_so_delete_persistent_object((int8_t*) name);
            if (GF_SUCCESS != err) {
                GF_LOGE(
                        LOG_TAG "[%s] fail to delete so(%s) for finger(%u)", __func__, name, finger_id);
                break;
            }

            while (NULL != finger) {
                if(finger->finger_id == finger_id
                   && finger->group_id == group_id)  {
                    if (pre_finger == finger) {
                        g_frr_far_finger_list.finger = finger->next;
                    } else {
                        pre_finger->next = finger->next;
                    }

                    g_frr_far_finger_list.count--;
                    CPL_MEM_FREE(finger);
                    finger = NULL;
                    break;
                }
                pre_finger = finger;
                finger = finger->next;
            }
        }

        //print_finger_list();

        gf_ta_test_frr_far_save_group_finger();

    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);

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
    uint64_t cur_timestamp;
    uint32_t i;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    if (0
            == cpl_strncmp((const int8_t*) cmd->cmd_buf,
                    (const int8_t*) "get_mode", cpl_strlen((const int8_t*)"get_mode"))) {
        gf_mode_t mode = MODE_NONE;
        err = g_spi_function.get_mode(&mode);
        cmd->mode = mode;
    } else if (0
            == cpl_strncmp((const int8_t*) cmd->cmd_buf,
                    (const int8_t*) "set_mode", cpl_strlen((const int8_t*)"set_mode"))) {
        gf_ta_set_mode(cmd->mode, OPERATION_NONE);
    } else if (0
            == cpl_strncmp((const int8_t*) cmd->cmd_buf,
                    (const int8_t*) "timer", cpl_strlen((const int8_t*)"timer"))) {
        GF_LOGI(LOG_TAG "[%s], recevied command time", __func__);

        gf_get_timestamp(&cur_timestamp);
        GF_LOGI(LOG_TAG "[%s], get timestamp high[0x%d], low[0x%d]", __func__,
                (uint32_t ) (cur_timestamp >> 32), (uint32_t ) cur_timestamp);

        gf_sleep(10);

        gf_get_timestamp(&cur_timestamp);
        GF_LOGI(LOG_TAG "[%s], get timestamp high[0x%d], low[0x%d]", __func__,
                (uint32_t ) (cur_timestamp >> 32), (uint32_t ) cur_timestamp);
    } else if (0
            == cpl_strncmp((const int8_t*) cmd->cmd_buf,
                    (const int8_t*) "erase_fw", cpl_strlen((const int8_t*)"erase_fw"))) {
        err = g_spi_function.erase_fw();
    } else if (0
            == cpl_strncmp((const int8_t*) cmd->cmd_buf,
                    (const int8_t*) "write_product_info", cpl_strlen((const int8_t*)"write_product_info"))) {
        err = g_spi_function.write_product_info(cmd->product_cfg_idx);
    }  else if (0
            == cpl_strncmp((const int8_t*) cmd->cmd_buf,
                    (const int8_t*) "clear_storage", cpl_strlen((const int8_t*)"clear_storage"))) {

    } else if (0
            == cpl_strncmp((const int8_t*) cmd->cmd_buf,
                    (const int8_t*) "secure_share_memory_performance",
                    cpl_strlen((const int8_t*)"secure_share_memory_performance"))) {

        uint8_t *share_buf = CPL_MEM_MALLOC(102400);
        uint32_t size_buf[] = { 0, 1, 8, 32, 1024, 10240, 51200, 102400 };

        uint32_t count = sizeof(size_buf) / sizeof(uint32_t);

        for (i = 0; i < count; i++) {
            cmd->secure_share_memory_size[i] = size_buf[i];
            gf_secure_share_memory_performance(share_buf,
                    cmd->secure_share_memory_size[i],
                    &cmd->secure_share_memory_time[i]);
        }

        cmd->secure_share_memory_count = count;

        CPL_MEM_FREE(share_buf);
    } else if (0
            == cpl_strncmp((const int8_t*) cmd->cmd_buf,
                    (const int8_t*) "esd_exception", cpl_strlen((const int8_t*)"esd_exception"))) {
        g_test_esd_exception_count = cmd->esd_exception_count;
        if (g_test_esd_exception_count > 10) {
            g_test_esd_exception_count = 10;
        }
    } else if (0
            == cpl_strncmp((const int8_t*) cmd->cmd_buf,
                    (const int8_t*) "ta_crash", cpl_strlen((const int8_t*) "ta_crash"))) {
        uint32_t *addr = NULL;
        GF_LOGD(LOG_TAG "[%s] ta_crash", __func__);
        *addr = 0x1234;
    } else if (0
            == cpl_strncmp((const int8_t*) cmd->cmd_buf,
                    (const int8_t*) "write", cpl_strlen((const int8_t*)"write"))) {
        GF_LOGD(LOG_TAG "[%s] spi write address: 0x%x, val: 0x%x", __func__,cmd->address, cmd->value);
        if (IS_MILAN_A_SERIES_BY_CHIP_TYPE(g_sensor.chip_type)) {
            err = g_spi_function.spi_write_byte(cmd->address,
                    cmd->value);
        }
    } else if (0
            == cpl_strncmp((const int8_t*) cmd->cmd_buf,
                    (const int8_t*) "read", cpl_strlen((const int8_t*)"read"))) {
        GF_LOGD(LOG_TAG "[%s] spi read address: 0x%x", __func__,cmd->address);
        if (IS_MILAN_A_SERIES_BY_CHIP_TYPE(g_sensor.chip_type)) {
            err = g_spi_function.spi_read_byte(cmd->address,
                    &(cmd->value));
        }
    } else if (0
            == cpl_strncmp((const int8_t*) cmd->cmd_buf,
                    (const int8_t*) "ignore", cpl_strlen((const int8_t*)"ignore"))) {
        GF_LOGD(LOG_TAG "[%s] ignore irq type: 0x%x", __func__,cmd->ignore_irq_type);
        if (IS_MILAN_A_SERIES_BY_CHIP_TYPE(g_sensor.chip_type)) {
            err = g_spi_function.set_ignore_irq_type(cmd->ignore_irq_type);
        }

        if (cmd->ignore_irq_type <= 0) {
            g_ignore_irq_mask = 0;
        } else {
            g_ignore_irq_mask = 1 << (cmd->ignore_irq_type);
        }
    }
    return err;
}

static gf_error_t gf_ta_set_session_id(gf_set_session_id_t *cmd) {
    gf_error_t err = GF_SUCCESS;
    uint64_t session_id = 0;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        session_id = cmd->session_id;
        err = gf_wechat_api_set_session_id(session_id);
        if (GF_SUCCESS != err) {
            GF_LOGD(LOG_TAG "[%s] set session id error", __func__);
            break;
        }
    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit", __func__);

    return GF_SUCCESS;
}

static gf_error_t gf_ta_get_session_id(gf_get_session_id_t *cmd) {
    gf_error_t err = GF_SUCCESS;
    uint64_t session_id = 0;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        err = gf_wechat_api_get_session_id(&session_id);
        if (GF_SUCCESS != err) {
            GF_LOGD(LOG_TAG "[%s] get session id error", __func__);
            break;
        }
        cmd->session_id = session_id;
    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit", __func__);

    return GF_SUCCESS;
}

static gf_error_t gf_ta_start_hbd(gf_cmd_header_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    UNUSED_VAR(cmd);

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        if (g_sensor.config.support_hbd <= 0) {
            GF_LOGE(LOG_TAG "[%s] sensor do not support hbd mode!", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
        g_finger_pressed_flag = 0;
        g_next_mode = MODE_NONE;
        g_next_operation = OPERATION_NONE;
        gf_heart_beat_init();
        err = gf_ta_set_mode(MODE_HBD, OPERATION_HEARTBEAT_KEY);
    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit", __func__);

    return err;
}

static gf_error_t gf_ta_get_irq_status_for_spi_clk(gf_irq_stautus_t *cmd) {
    gf_error_t err = GF_SUCCESS;
    uint32_t irq_type = 0;

    GF_LOGD(LOG_TAG "[%s] :enter.", __func__);

    err = g_spi_function.get_irq_status_for_spi_clk(&irq_type, g_cur_mode);
    if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] :get irq type fail:%d.", __func__, err);
    }

    cmd->irq_type = irq_type;

    GF_LOGD(LOG_TAG "[%s] :irq_type=0x%x.", __func__, irq_type);

    return err;
}

static gf_error_t gf_ta_polling_nav_data(gf_irq_t *cmd) {
    gf_error_t err = GF_SUCCESS;
    uint8_t over_flag = 0;
    uint8_t frame_num = 0;

    GF_LOGD(LOG_TAG "[%s] :enter.", __func__);
    if (OPERATION_NAV != g_operation) {
        GF_LOGE(LOG_TAG "[%s] wrong operation!  g_operation: %d", __func__, g_operation);
        return GF_ERROR_GENERIC;
    }

    do {
        err = g_spi_function.get_nav_image(&g_fp_raw_data, &g_fp_raw_data_len, &frame_num, &over_flag);

        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] get_nav_image failed err = %d", __func__, err);
            break;
        }

        if (cmd->dump_data_flag > 0) {
            cpl_memcpy(cmd->dump_data.raw_data, g_fp_raw_data,
            g_fp_raw_data_len * sizeof(uint16_t));
        }

        if (3 == over_flag) {
            GF_LOGI(LOG_TAG "[%s] nav frame up to max_frame_num, just suspend", __func__);
            g_spi_function.data_sample_suspend();
        } else if (2 == over_flag){
            GF_LOGI(LOG_TAG "[%s] detect finger up, need to resume", __func__);
            g_spi_function.data_sample_resume();
        }

        cpl_memcpy(cmd->navigation.raw_data, g_fp_raw_data, g_fp_raw_data_len * sizeof(uint16_t));
        cmd->navigation.raw_data_len = g_fp_raw_data_len;
        cmd->navigation.frame_num = frame_num;
        cmd->navigation.finish_flag = over_flag;
        g_image_count = frame_num;
    } while (0);

    return err;
}

gf_error_t gf_ta_create_entry_point(void) {
    gf_error_t err = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
#ifdef GF_MEMORY_DEBUG_ENABLE
        g_ta_memory_start = (void *)gf_malloc(GF_MEMORY_MANAGER_TOTAL_SIZE);
        if (NULL == g_ta_memory_start) {
            GF_LOGE(LOG_TAG "[%s] malloc(%d bytes) fail", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }
        MEM_INIT(g_ta_memory_start, GF_MEMORY_MANAGER_TOTAL_SIZE);
#endif
        /* init gf_sensor_t */
        err = gf_sensor_init();
        if (err != GF_SUCCESS) {
            GF_LOGD(LOG_TAG "[%s] gf_sensor_init return error", __func__);
            break;
        }

        /* init function pointer */
        err = gf_spi_function_init(&g_spi_function);
        if (err != GF_SUCCESS) {
            GF_LOGD(LOG_TAG "[%s] gf_spi_function_init return error", __func__);
            break;
        }

        err = g_spi_function.fw_cfg_sanity_check();
        if (err != GF_SUCCESS) {
            GF_LOGD(LOG_TAG "[%s] fw_cfg_sanity_check return error", __func__);
            break;
        }

    } while (0);

    return err;
}

gf_error_t gf_ta_get_hmac_key(void) {
    uint8_t key[128] = { 0 };
    uint32_t err = GF_SUCCESS;
    err = gf_get_hmac_key(key, 32);
    if (err != GF_SUCCESS) {
        GF_LOGD(LOG_TAG "[%s], err:%d", __func__, err);
    }
    return err;
}

void gf_ta_destroy_entry_point(void) {
    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    gf_spi_destroy();
    gf_algo_destroy();
    gf_frr_far_destroy();

    if (g_sensor.config.support_sensor_broken_check > 0) {
        gf_broken_check_destroy();
    }

    if (g_finger_index_hash_table != NULL) {
        CPL_MEM_FREE(g_finger_index_hash_table);
        g_finger_index_hash_table = NULL;
    }

#ifdef GF_MEMORY_DEBUG_ENABLE
    MEM_DESTROY();
    if (NULL != g_ta_memory_start) {
        gf_free(g_ta_memory_start);
        g_ta_memory_start = NULL;
    }
#endif
}

static void gf_ta_sync_finger_list(void) {
    uint32_t finger_ids[MAX_FINGERS_PER_USER] = { 0 };
    uint32_t group_ids[MAX_FINGERS_PER_USER] = { 0 };
    uint32_t finger_count = 0;

    gf_algo_enumerate(group_ids, finger_ids, &finger_count);
    gf_sync_finger_list(finger_ids, finger_count);
}

static gf_error_t gf_ta_study_feature(gf_irq_t *cmd) {
    gf_error_t err = GF_SUCCESS;
    gf_save_t* save = NULL;
    gf_irq_t* irq = NULL;
    GF_LOGD(LOG_TAG "[%s] :enter.", __func__);

    do {
        if (0 == g_study_enable_flag) {
            GF_LOGE(LOG_TAG "[%s] judge_finger_stable about this auth", __func__);
            break;
        }

        if ((g_sensor.config.support_sensor_broken_check > 0)
                && (cmd->operation != OPERATION_TEST_UNTRUSTED_AUTHENTICATE)) {
            irq = CPL_MEM_MALLOC(sizeof(gf_irq_t));
            if (NULL == irq) {
                GF_LOGE(LOG_TAG "[%s] memory allocation failed", __func__);
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
            }
            cpl_memset(irq, 0, sizeof(gf_irq_t));
            gf_broken_check_on_image_irq(g_fp_raw_data, g_fp_raw_data_len, irq);
            if (irq->broken_check.disable_study > 0) {
                GF_LOGE(LOG_TAG "[%s] sensor is broken for AUTH", __func__);
                break;
            }
        }

        err = gf_algo_fingerfeature_study(&cmd->update_flag);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] gf_algo_fingerfeature_study error", __func__);
            break;
        }

        if (GF_PERFORMANCE_DUMP_ENABLED) {
            cmd->test_performance.authenticate_update_flag = g_dump_performance.authenticate_update_flag;
        }

        if (cmd->update_flag > 0) {
            uint8_t save_flag = 0;
            save = CPL_MEM_MALLOC(sizeof(gf_save_t));
            if(NULL == save) {
                GF_LOGE(LOG_TAG "[%s] memory allocation failed", __func__);
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
            }
            cpl_memset(save, 0, sizeof(gf_save_t));

            save->finger_id = cmd->finger_id;
            save->group_id = cmd->group_id;
            err = gf_algo_get_finger_study_count(save->group_id, save->finger_id, &save_flag);
            if (GF_SUCCESS != err) {
                GF_LOGE(LOG_TAG "[%s] group_id(%d), finger(%u) not exist, err:%d", __func__, save->group_id, save->finger_id, err);
                break;
            }

            if (save_flag > 0) {
                err = gf_ta_save(save);
                if (GF_SUCCESS != err) {
                    GF_LOGE(LOG_TAG "[%s] Save new studied feature error!", __func__);
                    break;
                }
                GF_LOGD(LOG_TAG "[%s] save_flag > 0,so continue the gf_ta_save to save template", __func__);
                gf_ta_sync_finger_list();
            }
        }
    } while (0);
    if(NULL != save) {
        CPL_MEM_FREE(save);
        save = NULL;
    }

    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);;
    return err;
}

gf_error_t gf_ta_invoke_user_cmd_entry_point(uint32_t cmd_id, void *buffer) {
    gf_error_t err = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] cmd_id:%d", __func__, cmd_id);

    switch (cmd_id) {
        case GF_USER_CMD_TEST_DUMP_TEMPLATES: {
            err = gf_ta_user_test_dump_templates(buffer);
            break;
        }

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

        default: {
            GF_LOGE(LOG_TAG "[%s] invalid parameters cmd_id = %d", __func__, cmd_id);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
    }

    GF_LOGD(LOG_TAG "[%s] exit.", __func__);

    return err;
}

static gf_error_t gf_ta_test_untrusted_remove(void) {
    gf_error_t err = GF_SUCCESS;

    gf_algo_test_remove(0, 0);

    return err;
}

gf_error_t gf_ta_invoke_cmd_entry_point(uint32_t cmd_id, void *buffer, uint32_t buffer_len) {
    gf_error_t err = GF_SUCCESS;
    gf_cmd_header_t *cmd = NULL;

    if (NULL == buffer || buffer_len < sizeof(gf_cmd_header_t)) {
        return GF_ERROR_BAD_PARAMS;
    }

    cmd = (gf_cmd_header_t *)buffer;

    GF_LOGD(LOG_TAG "[%s] cmd:%d", __func__, cmd_id);

    //if enineer mode otp crc check failed, finger factory test can't available
    if (0 == g_sensor_engineer_available && IS_MILAN_F_SERIES_BY_CHIP_TYPE(g_sensor.chip_type)) {
        if (cmd_id == GF_CMD_TEST_SENSOR_STEP1 || cmd_id == GF_CMD_TEST_SENSOR_FINISH
                || cmd_id == GF_CMD_TEST_BAD_POINT || cmd_id == GF_CMD_TEST_PERFORMANCE) {
            GF_LOGD(LOG_TAG "[%s] engineer mode break", __func__);
            cmd->result = GF_ERROR_SENSOR_TEST_FAILED;
            return err;
        }
    }

    switch (cmd_id) {
        case GF_CMD_DETECT_SENSOR: {
            err = gf_ta_detect_sensor((gf_detect_sensor_t *)buffer);
            break;
        }

        case GF_CMD_INIT: {
            err = gf_ta_init((gf_init_t*)buffer);
            gf_sync_authenticator_version(GF_AUTHENTICATOR_VERSION_CODE);
            break;
        }

        case GF_CMD_EXIT:
            // TODO: gf_ta_exit(cmd)
            err = GF_SUCCESS;
            break;

        case GF_CMD_DOWNLOAD_FW:
            err = gf_ta_download_fw((gf_cmd_header_t *)buffer);
            break;

        case GF_CMD_DOWNLOAD_CFG:
            err = gf_ta_download_cfg((gf_cmd_header_t *)buffer);
            break;

        case GF_CMD_INIT_FINISHED:
            err = gf_ta_init_finished((gf_init_finished_t*)buffer);
            break;

        case GF_CMD_PRE_ENROLL:
            err = gf_ta_pre_enroll((gf_pre_enroll_t*)buffer);
            break;

        case GF_CMD_ENROLL:
            err = gf_ta_enroll((gf_enroll_t*)buffer);
            break;

        case GF_CMD_POST_ENROLL:
            err = gf_ta_post_enroll();
            break;

        case GF_CMD_CANCEL:
            err = gf_ta_cancel((gf_cancel_t *)buffer);
            break;

        case GF_CMD_AUTHENTICATE:
            err = gf_ta_authenticate((gf_authenticate_t*)buffer);
            break;

        case GF_CMD_GET_AUTH_ID:
            err = gf_ta_get_auth_id((gf_get_auth_id_t*)buffer);
            break;

        case GF_CMD_SAVE:
            err = gf_ta_save((gf_save_t*)buffer);
            gf_ta_sync_finger_list();
            break;

        case GF_CMD_REMOVE:
            err = gf_ta_remove((gf_remove_t*)buffer);
            gf_ta_sync_finger_list();
            break;

        case GF_CMD_SET_ACTIVE_GROUP:
            err = gf_ta_set_active_group((gf_set_active_group_t*)buffer);
            gf_ta_sync_finger_list();
            break;

        case GF_CMD_SET_SAFE_CLASS:
            err = gf_ta_set_safe_class((gf_set_safe_class_t*)buffer);
            break;

        case GF_CMD_ENUMERATE:
            err = gf_ta_enumerate((gf_enumerate_t*)buffer);
            break;

        case GF_CMD_NAVIGATE:
            err = gf_ta_navigate((gf_cmd_header_t *)buffer);
            break;

        case GF_CMD_GET_NAVIGATION_DATA:
            err = gf_ta_get_navigation_data((gf_irq_t*)buffer);
            break;

        case GF_CMD_NAVIGATE_COMPLETE:
            err = gf_ta_navigate_complete((gf_cmd_header_t *)buffer);
            break;

        case GF_CMD_IRQ:
            err = gf_ta_irq((gf_irq_t*)buffer);
            break;

        case GF_CMD_SCREEN_ON:
            err = gf_ta_screen_on((gf_cmd_header_t *)buffer);
            break;

        case GF_CMD_SCREEN_OFF:
            err = gf_ta_screen_off((gf_cmd_header_t *)buffer);
            break;

        case GF_CMD_ESD_CHECK:
            err = gf_ta_esd_check((gf_esd_check_t*)buffer);
            break;

        case GF_CMD_CAMERA_CAPTURE:
            err = gf_ta_camera_capture((gf_cmd_header_t *)buffer);
            break;

        case GF_CMD_STUDY_FEATURE:
            err = gf_ta_study_feature((gf_irq_t *)buffer);
            break;

        case GF_CMD_TEST_BAD_POINT:
            err = gf_ta_test_bad_point((gf_cmd_header_t *)buffer);
            break;

        case GF_CMD_ENABLE_FINGERPRINT_MODULE:
            err = gf_ta_enable_fingerprint_module((gf_enable_fingerprint_module_t*)buffer);
            break;

        case GF_CMD_ENABLE_FF_FEATURE:
            err = gf_ta_enable_ff_feature((gf_enable_ff_feature_t*)buffer);
            break;

        case GF_CMD_TEST_SENSOR:
            err = gf_ta_test_sensor((gf_cmd_header_t *)buffer);
            break;

        case GF_CMD_TEST_SENSOR_STEP1:
            err = gf_ta_test_sensor_step1((gf_cmd_header_t *)buffer);
            break;

        case GF_CMD_TEST_SENSOR_FINISH:
            err = gf_ta_test_sensor_finish((gf_cmd_header_t *)buffer);
            break;

        case GF_CMD_TEST_PERFORMANCE:
            err = gf_ta_test_performance((gf_cmd_header_t *)buffer);
            break;

        case GF_CMD_TEST_SPI_PERFORMANCE:
            err = gf_ta_test_spi_performance((gf_cmd_header_t *)buffer);
            break;

        case GF_CMD_TEST_SPI_TRANSFER:
            err = gf_ta_test_spi_transfer((gf_cmd_header_t *)buffer);
            break;

        case GF_CMD_TEST_PRE_SPI:
            err = gf_ta_test_pre_spi((gf_cmd_header_t *)buffer);
            break;

        case GF_CMD_TEST_SPI:
            err = gf_ta_test_spi((gf_test_spi_t*)buffer);
            break;

        case GF_CMD_TEST_PRE_GET_VERSION:
            err = gf_ta_test_pre_get_version((gf_cmd_header_t *)buffer);
            break;

        case GF_CMD_TEST_GET_VERSION:
            err = gf_ta_test_get_version((gf_test_get_version_t*)buffer);
            break;

        case GF_CMD_TEST_DRIVER_CMD:
            err = gf_ta_test_driver_cmd((gf_test_driver_cmd_t*)buffer);
            break;

        case GF_CMD_TEST_FRR_FAR_INIT:
            err = gf_ta_test_frr_far_init((gf_test_frr_far_init_t*)buffer);
            break;

        case GF_CMD_TEST_FRR_FAR_RECORD_BASE_FRAME:
            err = gf_ta_test_frr_far_record_base_frame((gf_cmd_header_t *)buffer);
            break;

        case GF_CMD_TEST_FRR_FAR_RECORD_ENROLL:
            err = gf_ta_test_frr_far_record_enroll((gf_cmd_header_t *)buffer);
            break;

        case GF_CMD_TEST_FRR_FAR_RECORD_AUTHENTICATE:
            err = gf_ta_test_frr_far_record_authenticate((gf_cmd_header_t *)buffer);
            break;

        case GF_CMD_TEST_FRR_FAR_RECORD_AUTHENTICATE_FINISH:
            err = gf_ta_test_frr_far_record_authenticate_finish((gf_cmd_header_t *)buffer);
            break;

        case GF_CMD_TEST_FRR_FAR_PLAY_BASE_FRAME:
            err = gf_ta_test_frr_far_play_base_frame((gf_test_frr_far_t*)buffer);
            break;

        case GF_CMD_TEST_FRR_FAR_PLAY_ENROLL:
            err = gf_ta_test_frr_far_play_enroll((gf_test_frr_far_t*)buffer);
            break;

        case GF_CMD_TEST_FRR_FAR_PLAY_AUTHENTICATE:
            err = gf_ta_test_frr_far_play_authenticate((gf_test_frr_far_t*)buffer);
            break;

        case GF_CMD_TEST_FRR_FAR_ENROLL_FINISH:
            err = gf_ta_test_frr_far_enroll_finish((gf_cmd_header_t *)buffer);
            break;

        case GF_CMD_TEST_FRR_FAR_SAVE_FINGER:
            err = gf_ta_test_frr_far_save((gf_test_frr_far_save_t*)buffer);
            break;

        case GF_CMD_TEST_FRR_FAR_DEL_FINGER:
            err = gf_ta_test_frr_far_remove((gf_test_frr_far_remove_t*)buffer);
            break;

        case GF_CMD_TEST_FRR_FAR_CANCEL:
            err = gf_ta_test_frr_far_cancel((gf_cancel_t*)buffer);
            break;

        case GF_CMD_TEST_RESET_PIN:
            err = gf_ta_test_reset_pin((gf_cmd_header_t *)buffer);
            break;

        case GF_CMD_RESET_PIN_POLLING_E4:
            err = gf_ta_reset_pin_polling_e4((gf_cmd_header_t *)buffer);
            break;

        case GF_CMD_TEST_DOWNLOAD_FW_CFG:
            err = gf_ta_test_download_fw_cfg((gf_test_download_fw_cfg_t*)buffer);
            break;

        case GF_CMD_TEST_PRE_DOWNLOAD_FW_CFG:
            err = gf_ta_test_pre_download_fw_cfg((gf_cmd_header_t *)buffer);
            break;

        case GF_CMD_TEST_SENSOR_CHECK:
            err = gf_ta_sensor_check((gf_test_check_sensor_test_info_t*)buffer);
            break;

        case GF_CMD_TEST_UNTRUSTED_ENROLL:
            err = gf_ta_test_untrusted_enroll((gf_cmd_header_t *)buffer);
            break;

        case GF_CMD_TEST_UNTRUSTED_AUTHENTICATE:
            err = gf_ta_test_untrusted_authenticate((gf_authenticate_t*)buffer);
            break;

        case GF_CMD_TEST_DELETE_UNTRUSTED_ENROLLED_FINGER:
            gf_algo_cancel_test_enroll();
            gf_ta_test_untrusted_remove();
            break;

        case GF_CMD_TEST_BIO_CHECK_FINGER_EVENT:
            err = gf_ta_bio_check_finger_event((gf_irq_t *)buffer);
            break;

        case GF_CMD_TEST_BIO_CALIBRATION:
            err = gf_ta_test_bio_calibration((gf_cmd_header_t *)buffer);
            break;

        case GF_CMD_TEST_HBD_CALIBRATION:
            err = gf_ta_test_hbd_calibration((gf_cmd_header_t *)buffer);
            break;

        case GF_CMD_TEST_CANCEL:
            err = gf_ta_test_cancel((gf_cancel_t*)buffer);
            break;

        case GF_CMD_TEST_SET_CONFIG:
            err = gf_ta_test_set_config((gf_test_set_config_t*)buffer);
            break;

        case GF_CMD_SET_SESSION_ID:
            err = gf_ta_set_session_id((gf_set_session_id_t*)buffer);
            break;

        case GF_CMD_GET_SESSION_ID:
            err = gf_ta_get_session_id((gf_get_session_id_t*)buffer);
            break;

        case GF_CMD_START_HBD:
            err = gf_ta_start_hbd((gf_cmd_header_t *)buffer);
            break;

        case GF_CMD_CHECK_FINGER_LONG_PRESS:
            err = gf_base_check_finger_long_press(&cmd->reset_flag);
            break;
        case GF_CMD_GET_IRQ_STATUS_FOR_SPI_CLK:
            err = gf_ta_get_irq_status_for_spi_clk((gf_irq_stautus_t*)buffer);
            break;

        case GF_CMD_RESET_MODE:
            err = gf_ta_set_mode(g_cur_mode,g_operation);
            break;

        case GF_CMD_POLLING_NAV_DATA:
            err = gf_ta_polling_nav_data((gf_irq_t*)buffer);
            break;
        case GF_CMD_ENABLE_REISSUE_KEY_DOWN_WHEN_ENTRY_FF_MODE:
            err = gf_ta_enable_reissue_key_down(0);
            break;
        case GF_CMD_ENABLE_REISSUE_KEY_DOWN_WHEN_ENTRY_IMAGE_MODE:
            err = gf_ta_enable_reissue_key_down(1);
            break;
        case GF_CMD_TEST_FPC_KEY_DETECT:
            err = gf_ta_test_fpc_key_detect((gf_test_fpc_key_data_t*) buffer);
            break;
        case GF_CMD_TEST_FRR_FAR_RESET_CALIBRATION:
            err = gf_ta_test_frr_far_reset_calibration();
            break;
        default: {
            GF_LOGE(LOG_TAG "[%s] invalid parameters cmd_id = %d", __func__, cmd_id);
            err = GF_ERROR_UNKNOWN_CMD;
            break;
        }
    }
#ifdef GF_MEMORY_DEBUG_ENABLE
    MEM_DUMP_MEM_POOL_LEAK();
#endif

    cmd->mode = g_cur_mode;
    cmd->operation = g_operation;

    /*compatible with different TEE*/
    cmd->result = err;

    GF_LOGD(LOG_TAG "[%s] mode: %d operation :%d", __func__, g_cur_mode, g_operation);
    GF_LOGD(LOG_TAG "[%s] cmd->result = %d.", __func__, cmd->result);
    GF_LOGD(LOG_TAG "[%s] Exit.", __func__);
    return err;
}
