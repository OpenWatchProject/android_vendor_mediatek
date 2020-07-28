/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#include "string.h"
#include "gf_error.h"
#include "gf_common.h"
#include "gf_user_type_define.h"
#include "gf_algo.h"
#include "gf_secure_object.h"
#include "gf_tee_internal_api.h"
#include "gf_spi.h"
#include "gf_hw_common.h"
#include "gf_ta.h"

#include "gf_sensor.h"
#include "gf_utils.h"
#include "cpl_memory.h"
#include "cpl_stdio.h"
#include "cpl_math.h"
#include "cpl_string.h"
#include "gf_algo_crc32.h"
#include "gf_base.h"
#include "gf_broken.h"
#include "gf_fido.h"
#include "gf_irq.h"
#include "gf_state_common.h"
#include "gf_type_define.h"
#include "gf_nav.h"
#include "gf_config.h"
#include "gf_state.h"
#include "gf_bad_point_test.h"
#include "gf_ta_chip_series.h"
#include "gf_ta_test.h"
#include "gf_ta_test_chip_series.h"

#define LOG_TAG "[gf_ta]"
#define GET_LAST_ID_TIMEOUT_US (5000000)

int32_t g_screen_on_flag = 1;
gf_test_performance_t g_dump_performance = { 0 };
uint8_t g_is_normal_cfg = 1;
gf_authenticate_fido_t g_authenticator_fido = { .cmd_header = { 0 }, 0 };

void gf_ta_cancel_operation(void);
static gf_error_t gf_ta_switch_fingerprint_user(uint32_t group_id);

gf_error_t gf_ta_pre_enroll(gf_pre_enroll_t *cmd);
gf_error_t gf_ta_enroll(gf_enroll_t *cmd);
gf_error_t gf_ta_post_enroll(void);
gf_error_t gf_ta_cancel(gf_cancel_t *cmd);
gf_error_t gf_ta_authenticate(gf_authenticate_t *cmd);
gf_error_t gf_ta_get_auth_id(gf_get_auth_id_t *cmd);
gf_error_t gf_ta_remove(gf_remove_t *cmd);
gf_error_t gf_ta_set_active_group(gf_set_active_group_t *cmd);
gf_error_t gf_ta_enumerate(gf_enumerate_t *cmd);
gf_error_t gf_ta_authenticate_study(gf_authenticate_study_t *cmd);

static uint32_t *g_finger_index_hash_table = NULL;
static uint8_t g_set_active_group_flag = 0;
uint64_t g_operation_id = 0;
uint64_t g_authenticator_id = 0;
uint64_t g_user_id = 0;
static uint64_t g_challenge = 0;

// g_image_data & g_nav_data & g_hbd_data use the same memory buffer, like union
gf_image_data_t *g_image_data = NULL;
gf_nav_data_t *g_nav_data = NULL;
gf_hbd_data_t *g_hbd_data = NULL;

gf_base_data_t *g_base_data = NULL;

gf_image_origin_data_t *g_image_origin_data = NULL;
gf_nav_origin_data_t *g_nav_origin_data = NULL;

uint8_t g_last_authenticate_result = 0;

static void gf_ta_init_global_variable(void) {
    VOID_FUNC_ENTER();

    g_screen_on_flag = 1;
    cpl_memset(&g_dump_performance, 0, sizeof(gf_test_performance_t));
    g_is_normal_cfg = 1;

    g_set_active_group_flag = 0;
    g_operation_id = 0;
    g_authenticator_id = 0;
    g_user_id = 0;
    g_challenge = 0;

    gf_nav_init_global_variable();

    gf_algo_init_global_variable();

    gf_state_init_global_variable();

    gf_hw_init_global_variable();

    gf_base_init_global_variable();

    gf_irq_init_global_variable();

    gf_ta_chip_series_init_global_variable();

    gf_ta_test_init_global_variable();

    gf_ta_test_chip_series_init_global_variable();

    // So far, broken module has no global variable need initialize
    // gf_broken_init_global_variable();

    VOID_FUNC_EXIT();
}

static gf_error_t gf_ta_generate_64bit_rand(uint64_t *rand) {
    gf_error_t err = GF_SUCCESS;
    uint8_t buf_h[4] = { 0 };
    uint8_t buf_l[4] = { 0 };
    uint64_t id = 0;

    do {
        err = gf_generate_random(buf_h, 4);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] failed to general the random num", __func__);
            break;
        }
        err = gf_generate_random(buf_l, 4);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] failed to general the random num", __func__);
            break;
        }

        id = (id | buf_h[3]) << 8;
        id = (id | buf_h[2]) << 8;
        id = (id | buf_h[1]) << 8;
        id = (id | buf_h[0]) << 8;
        id = (id | buf_l[3]) << 8;
        id = (id | buf_l[2]) << 8;
        id = (id | buf_l[1]) << 8;
        id = (id | buf_l[0]);
    } while (0);

    if (0 == id) {
        gf_get_timestamp(&id);
        if (0 == id) {
            id = 1;
        }
    }

    *rand = id;

    return GF_SUCCESS;
}

static gf_error_t gf_ta_save_auth_token(uint32_t group_id) {
    gf_error_t err = GF_SUCCESS;
    uint8_t *buf = NULL;
    uint32_t buf_len = 0;
    uint8_t name[GF_FINGER_OBJECT_ID_MAX_LEN] = { 0 };

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

        cpl_sprintf((int8_t*) name, (const int8_t*) GF_AUTH_TOKEN_OBJECT_ID_BY_GROUP, group_id);

        err = gf_so_save_persistent_object((int8_t*) name, buf, buf_len);
        GF_LOGD(LOG_TAG "[%s] Save secure id in auth_token_%d.so", __func__, group_id);
    } while (0);

    CPL_MEM_FREE(buf);

    return err;
}

static gf_error_t gf_ta_load_auth_token(uint32_t group_id) {
    gf_error_t err = GF_SUCCESS;
    uint8_t *buf = NULL;
    uint32_t buf_len = 0;
    uint8_t name[GF_FINGER_OBJECT_ID_MAX_LEN] = { 0 };

    do {
        cpl_sprintf((int8_t*) name, (const int8_t*) GF_AUTH_TOKEN_OBJECT_ID_BY_GROUP, group_id);

        GF_LOGD(LOG_TAG "[%s] Loading secure id from auth_token_%d.so now", __func__, group_id);
        err = gf_so_load_persistent_object((int8_t*) name, &buf, &buf_len);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] Load secure id from auth_token_%d.so failed", __func__, group_id);
            GF_LOGD(LOG_TAG "[%s] Loading secure id from default auth_token.so now", __func__);

            err = gf_so_load_persistent_object((int8_t*) GF_AUTH_TOKEN_OBJECT_ID, &buf, &buf_len);
            if (GF_SUCCESS != err) {
                GF_LOGE(LOG_TAG "[%s] Load secure id failed", __func__);
                break;
            }

            gf_so_save_persistent_object((int8_t*) name, buf, buf_len);
            gf_so_delete_persistent_object((int8_t*) GF_AUTH_TOKEN_OBJECT_ID);
        }

        err = gf_so_load_auth_token(&g_user_id, &g_authenticator_id, buf, buf_len);
    } while (0);

    CPL_MEM_FREE(buf);

    return err;
}

static gf_error_t gf_ta_init_algo_module(void) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        gf_algo_config_t algo_config = { 0 };

        algo_config.flag_chip_info = g_sensor.flag_chip_info;
        algo_config.thr_select_bmp = g_sensor.thr_select_bmp;

        err = gf_hw_get_chip_id(algo_config.chip_id, GF_CHIP_ID_LEN);
        GF_ERROR_BREAK(err);

        err = gf_hw_get_vendor_id(algo_config.vendor_id, GF_VENDOR_ID_LEN);
        GF_ERROR_BREAK(err);

        err = gf_hw_get_sensor_id(algo_config.sensor_id, GF_SENSOR_ID_LEN);
        GF_ERROR_BREAK(err);

        err = gf_hw_get_otp_data(algo_config.otp_info, GF_SENSOR_OTP_INFO_LEN);
        GF_ERROR_BREAK(err);

        algo_config.basic_config.chip_series = g_config.chip_series;

        algo_config.basic_config.max_fingers = g_config.max_fingers;
        algo_config.basic_config.max_fingers_per_user = g_config.max_fingers_per_user;

        algo_config.basic_config.support_bio_assay = g_config.support_bio_assay;

        algo_config.basic_config.enrolling_min_templates =
                g_config.enrolling_min_templates;
        algo_config.basic_config.valid_image_quality_threshold =
                g_config.valid_image_quality_threshold;
        algo_config.basic_config.valid_image_area_threshold =
                g_config.valid_image_area_threshold;
        algo_config.basic_config.duplicate_finger_overlay_score =
                g_config.duplicate_finger_overlay_score;
        algo_config.basic_config.increase_rate_between_stitch_info =
                g_config.increase_rate_between_stitch_info;

        algo_config.basic_config.forbidden_enroll_duplicate_fingers =
                g_config.forbidden_enroll_duplicate_fingers;

        algo_config.basic_config.authenticate_order =
                g_config.authenticate_order;

        /*check template_update_save_threshold can not be less than GF_MIN_TEMPLATE_UPDATE_COUNT_SAVE_THRESHOLD
         * and can not be greater than GF_MAX_TEMPLATE_UPDATE_COUNT_SAVE_THRESHOLD
         */
        if (GF_MIN_TEMPLATE_UPDATE_COUNT_SAVE_THRESHOLD > g_config.template_update_save_threshold) {
            g_config.template_update_save_threshold = GF_MIN_TEMPLATE_UPDATE_COUNT_SAVE_THRESHOLD;
        } else if (GF_MAX_TEMPLATE_UPDATE_COUNT_SAVE_THRESHOLD
                < g_config.template_update_save_threshold) {
            g_config.template_update_save_threshold = GF_MAX_TEMPLATE_UPDATE_COUNT_SAVE_THRESHOLD;
        }
        algo_config.basic_config.template_update_save_threshold =
                g_config.template_update_save_threshold;

        err = gf_algo_init(&algo_config);

        /* reset group id and reload fingerprint list,because group id cleared in gf_algo_init */
        g_set_active_group_flag = 0;
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_detect_sensor(gf_detect_sensor_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        err = gf_spi_init();
        if (GF_SUCCESS != err) {
            break;
        }

        /*init global variable*/
        gf_ta_init_global_variable();

        err = gf_hw_detect_sensor();
        if (GF_SUCCESS != err) {
            break;
        }

        // gf_config_init_by_chip_type reassigns g_config,
        // because milan_f_series detect_sensor could change chip_type
        gf_config_init_by_chip_type(g_config.chip_type);
        gf_sensor_info_init_by_chip_type(g_config.chip_type);

        cpl_memcpy(&cmd->config, &g_config, sizeof(gf_config_t));
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_exit(gf_cmd_header_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    err = gf_state_execute_operation(OPERATION_NONE, &cmd->reset_flag);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_init_finger_index_hash_table(void) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        // init fingerprint index hash table.
        uint32_t index_hash_table_len = g_config.max_fingers_per_user * sizeof(uint32_t);

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

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_init_finished(gf_init_finished_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        err = gf_ta_init_finger_index_hash_table();
        if (GF_SUCCESS != err) {
            break;
        }

        // init chip
        gf_hw_chip_config_init();

        // init algorithm module
        err = gf_ta_init_algo_module();
        if (GF_SUCCESS != err) {
            break;
        }

        gf_hw_get_orientation(&cmd->orientation, &cmd->facing);

        if (g_config.support_sensor_broken_check) {
            err = gf_broken_check_init();
            if (err != GF_SUCCESS) {
                break;
            }
        }

        err = gf_base_init_finished(&cmd->cmd_header.reset_flag);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_ta_pre_enroll(gf_pre_enroll_t *cmd) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    gf_ta_generate_64bit_rand(&g_challenge);
    cmd->challenge = g_challenge;
    gf_base_on_pre_enroll(&(cmd->cmd_header.reset_flag));

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_ta_enroll(gf_enroll_t *cmd) {
    gf_error_t err = GF_SUCCESS;
    uint32_t group_id = cmd->group_id;
    gf_hw_auth_token_t *token = &cmd->hat;
    gf_hw_auth_token_t hat = { 0 };
    uint32_t hmac_len = sizeof(hat.hmac);

    FUNC_ENTER();

    do {
        if (NULL != token) {
            if (token->challenge == g_challenge) {
                g_user_id = token->user_id;
            }

            if (g_config.forbidden_untrusted_enroll > 0) {
                if (token->version != cmd->system_auth_token_version) {
                    GF_LOGE(LOG_TAG "[%s] invalid hat version code detected", __func__);
                    err = GF_ERROR_INVALID_HAT_VERSION;
                    break;
                }

                if ((token->challenge == 0 || (token->challenge != g_challenge
                        && (token->authenticator_type & GF_HW_AUTH_FINGERPRINT)))) {
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
        } else if (g_config.forbidden_untrusted_enroll > 0) {
            GF_LOGE(LOG_TAG "[%s] invalid or null token", __func__);
            err = GF_ERROR_UNTRUSTED_ENROLL;
            break;
        }

        if (0 == g_set_active_group_flag) {
            gf_ta_switch_fingerprint_user(group_id);
        }

        err = gf_algo_pre_enroll(group_id);
        if (GF_SUCCESS != err) {
            break;
        }
        gf_broken_init_global_variable();

        err = gf_state_execute_operation(OPERATION_ENROLL, &cmd->cmd_header.reset_flag);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_ta_post_enroll(void) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    g_challenge = 0;

    FUNC_EXIT(err);
    return err;
}

static gf_error_t gf_ta_stop_navigation(void) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    gf_nav_set_mode(g_config.support_nav_mode);
    gf_nav_stop();

    FUNC_EXIT(err);
    return err;
}

void gf_ta_cancel_operation(void) {
    gf_operation_type_t cur_operation = OPERATION_NONE;

    VOID_FUNC_ENTER();

    cur_operation = gf_state_get_current_operation();

    switch (cur_operation) {
        case OPERATION_ENROLL:
            gf_algo_cancel_enroll();
            break;

        case OPERATION_AUTHENTICATE_IMAGE:
        case OPERATION_AUTHENTICATE_FF:
        case OPERATION_AUTHENTICATE_FIDO:
            gf_algo_cancel_authenticate();
            break;

        case OPERATION_TEST_PERFORMANCE:
            gf_algo_cancel_test_performance();
            break;

        case OPERATION_TEST_FRR_FAR_RECORD_BASE_FRAME:
        case OPERATION_TEST_FRR_FAR_RECORD_ENROLL:
        case OPERATION_TEST_FRR_FAR_RECORD_AUTHENTICATE:
            gf_frr_far_cancel();
            break;

        case OPERATION_TEST_REAL_TIME_DATA:
        case OPERATION_TEST_BMP_DATA:
            gf_state_execute_operation(OPERATION_TEST_KEY_MODE, NULL);
            break;

        case OPERATION_TEST_UNTRUSTED_ENROLL:
            gf_algo_cancel_test_enroll();
            break;

        case OPERATION_TEST_UNTRUSTED_AUTHENTICATE:
            gf_algo_cancel_test_authenticate();
            break;

        case OPERATION_FINGER_BASE:
        case OPERATION_NAV_BASE:
        case OPERATION_NAV:
            gf_ta_stop_navigation();
            break;

        case OPERATION_TEST_BAD_POINT:
            gf_bad_point_test_destroy();
            break;

        default:
            gf_ta_chip_series_cancel_operation(cur_operation);
            break;
    }

    VOID_FUNC_EXIT();
}

gf_error_t gf_ta_cancel(gf_cancel_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    cmd->operation = gf_state_get_current_operation();

    /*
     * there is a trap:
     * when the function is called, #gf_ta_cancel_operation release some resource;
     * but #gf_ta_pre_set_mode maybe can not change the mode immediately,such as cancel but sensor is pressed, it will be set mode next;
     * in this condition, attention the released resource is NULL;
     */
    err = gf_state_cancel_operation(&cmd->cmd_header.reset_flag);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_authenticate_fido(gf_authenticate_fido_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    GF_LOGD(LOG_TAG "[%s] group_id=%u", __func__, cmd->group_id);
    g_last_authenticate_result = 0;
    g_authenticator_fido.group_id = cmd->group_id;
    g_authenticator_fido.aaid_len = cmd->aaid_len;
    cpl_memcpy(g_authenticator_fido.aaid, cmd->aaid, g_authenticator_fido.aaid_len);
    g_authenticator_fido.final_challenge_len = cmd->final_challenge_len;
    cpl_memcpy(g_authenticator_fido.final_challenge, cmd->final_challenge,
            g_authenticator_fido.final_challenge_len);

    if (0 == g_set_active_group_flag) {
        gf_ta_switch_fingerprint_user(g_authenticator_fido.group_id);
    }

    err = gf_state_execute_operation(OPERATION_AUTHENTICATE_FIDO, &cmd->cmd_header.reset_flag);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_ta_authenticate(gf_authenticate_t *cmd) {
    gf_error_t err = GF_SUCCESS;
    gf_operation_type_t operation = OPERATION_AUTHENTICATE_IMAGE;

    FUNC_ENTER();

    GF_LOGD(LOG_TAG "[%s] group_id=%u", __func__, cmd->group_id);

    g_last_authenticate_result = 0;
    g_operation_id = cmd->operation_id;

    if (0 == g_set_active_group_flag) {
        gf_ta_switch_fingerprint_user(cmd->group_id);
    }

    if (-1 != cmd->screen_on_flag) {
        g_screen_on_flag = cmd->screen_on_flag;
    }

    if (0 == g_screen_on_flag) {
        if (g_config.support_ff_mode > 0) {
            operation = OPERATION_AUTHENTICATE_FF;
        } else {
            operation = OPERATION_AUTHENTICATE_SLEEP;
        }
    }
    err = gf_state_execute_operation(operation, &cmd->cmd_header.reset_flag);
    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_ta_get_auth_id(gf_get_auth_id_t *cmd) {
    cmd->auth_id = g_authenticator_id;
    return GF_SUCCESS;
}

gf_error_t gf_ta_get_last_identify_id(uint8_t *pIdBuf, uint32_t *pBufLen) {
    gf_error_t ret = GF_SUCCESS;
    uint32_t id = 0;

    FUNC_ENTER();

    do {
        if (NULL == pIdBuf || NULL == pBufLen) {
            ret = GF_ERROR_BAD_PARAMS;
            break;
        }

        ret = gf_algo_get_last_identify_id(&id, GET_LAST_ID_TIMEOUT_US);
        if (GF_SUCCESS != ret) {
            break;
        }

        *pBufLen = sizeof(uint32_t);
        cpl_memcpy(pIdBuf, &id, *pBufLen);
    } while (0);

    FUNC_EXIT(ret);

    return ret;
}

gf_error_t gf_ta_get_id_list(uint8_t *pIdBuf, uint32_t *pBufLen) {
    gf_error_t ret = GF_SUCCESS;
    gf_enumerate_t enumerate = { .cmd_header = { 0 }, 0 };
    uint32_t len = 4;
    uint32_t i = 0;

    FUNC_ENTER();

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

    FUNC_EXIT(ret);

    return ret;
}

gf_error_t gf_ta_get_authenticator_version(uint32_t *pVersion) {
    gf_error_t ret = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == pVersion) {
            GF_LOGE(LOG_TAG "[%s] bad parameter", __func__);
            ret = GF_ERROR_BAD_PARAMS;
            break;
        }

        *pVersion = GF_AUTHENTICATOR_VERSION_CODE;
    } while (0);

    FUNC_EXIT(ret);

    return ret;
}

static gf_error_t gf_ta_find_slot_for_finger(gf_finger_status_t status, uint32_t finger_id,
        uint32_t *slot) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint32_t i = 0;

        if (NULL == slot) {
            GF_LOGE(LOG_TAG "[%s] bad parameter", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        for (; i < g_config.max_fingers_per_user; i++) {
            if (((FINGER_STATUS_ENROLLING == status) && (g_finger_index_hash_table[i] == 0))
                    || (g_finger_index_hash_table[i] == finger_id)) {
                *slot = i;
                GF_LOGI(LOG_TAG "[%s] find slot(%d) for finger(%u)", __func__, i, finger_id);
                break;
            }
        }

        if (i == g_config.max_fingers_per_user) {
            err = GF_ERROR_REACH_FINGERS_UPLIMIT;
            GF_LOGE(LOG_TAG "[%s] can not find the slot for finger(%u)", __func__, finger_id);
            break;
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_save(gf_save_t *cmd) {
    uint8_t *buf = NULL;
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint32_t buf_len = 0;
        uint32_t group_id = 0;
        uint32_t finger_id = 0;
        uint32_t finger_index = 0;
        uint8_t name[GF_FINGER_OBJECT_ID_MAX_LEN] = { 0 };
        gf_finger_status_t status = FINGER_STATUS_IDLE;

        if (NULL == cmd) {
            GF_LOGE(LOG_TAG "[%s] bad parameter", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        group_id = cmd->group_id;
        finger_id = cmd->finger_id;
        GF_LOGD(LOG_TAG "[%s] group_id=%u, finger_id=%u", __func__, group_id, finger_id);

        err = gf_algo_get_finger_status(group_id, finger_id, &status);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] group_id, finger not exist. group_id=%u, finger_id=%u",
                    __func__, group_id, finger_id);
            break;
        }

        if (FINGER_STATUS_ENROLLING == status) {
            gf_ta_generate_64bit_rand(&g_authenticator_id);
            gf_ta_save_auth_token(cmd->group_id);
        }

        err = gf_ta_find_slot_for_finger(status, finger_id, &finger_index);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] can not find the slot to save finger(%u)",
                    __func__, finger_id);
            break;
        }

        cpl_sprintf((int8_t*) name, (const int8_t*) GF_FINGER_OBJECT_ID, group_id, finger_index);

        err = gf_algo_get_fingerTempInfo_size(finger_id, &buf_len);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] get finger(%u) size failed", __func__, finger_id);
            break;
        }

        buf = (uint8_t *) CPL_MEM_MALLOC(buf_len);
        if (NULL == buf) {
            err = GF_ERROR_OUT_OF_MEMORY;
            GF_LOGE(LOG_TAG "[%s] buf out of memory. err=%s, errno=%d",
                    __func__, gf_strerror(err), err);
            break;
        }
        err = gf_algo_get_finger_pdu(group_id, finger_id, buf, buf_len);
        if (err != GF_SUCCESS) {
            break;
        }

        err = gf_so_save_persistent_object((int8_t*) name, buf, buf_len);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] fail to save finger(%u)'s so(%s)", __func__, finger_id, name);
            break;
        }

        // NOTE: here it must be success. so don't care error code
        gf_algo_set_finger_status(group_id, finger_id, FINGER_STATUS_READY);
        g_finger_index_hash_table[finger_index] = finger_id;

        if (FINGER_STATUS_ENROLLING == status) {
            gf_error_t tmp_err = GF_SUCCESS;

            cpl_sprintf((int8_t*) name, (const int8_t*) GF_FINGER_OBJECT_ID_BAK, group_id,
                    finger_index);

            tmp_err = gf_so_save_persistent_object((int8_t*) name, buf, buf_len);
            if (GF_SUCCESS != tmp_err) {
                GF_LOGE(LOG_TAG "[%s] fail to save finger(%u)'s so(%s)",
                        __func__, finger_id, name);
            } else {
                GF_LOGI(LOG_TAG "[%s] success to save finger(%u)'s so(%s)",
                        __func__, finger_id, name);
            }
        }
    } while (0);

    CPL_MEM_FREE(buf);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_update_stitch(gf_update_stitch_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint32_t group_id = 0;
        uint32_t finger_id = 0;

        if (NULL == cmd) {
            GF_LOGE(LOG_TAG "[%s] bad parameter", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        group_id = cmd->group_id;
        finger_id = cmd->finger_id;
        GF_LOGD(LOG_TAG "[%s] group_id=%u, finger_id=%u", __func__, group_id, finger_id);

        err = gf_algo_update_stitch(group_id, finger_id);
        if (GF_SUCCESS != err) {
            break;
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_ta_remove(gf_remove_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint32_t slot = 0;
        uint32_t group_id = cmd->group_id;
        uint32_t finger_id = cmd->finger_id;
        uint8_t name[GF_FINGER_OBJECT_ID_MAX_LEN] = { 0 };
        uint32_t finger_ids[MAX_FINGERS_PER_USER] = { 0 };
        uint32_t group_ids[MAX_FINGERS_PER_USER] = { 0 };
        uint32_t finger_count = 0;
        uint32_t i = 0;
        uint32_t removing_templates = 0;

        GF_LOGD(LOG_TAG "[%s] group_id=%u, finger_id=%u", __func__, group_id, finger_id);

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
                GF_LOGE(LOG_TAG "[%s] can not find the slot for finger(%u)",
                        __func__, finger_ids[i]);
                // NOTE: because google's application's limit, so should return success here
                err = GF_SUCCESS;
                cmd->deleted_fids[i] = finger_ids[i];
                cmd->deleted_gids[i] = group_ids[i];
                continue;
            }

            cpl_sprintf((int8_t*) name, (const int8_t*) GF_FINGER_OBJECT_ID, group_id, slot);

            err = gf_so_delete_persistent_object((int8_t*) name);
            if (GF_SUCCESS != err) {
                GF_LOGE(LOG_TAG "[%s] fail to delete so(%s) for finger(%u)",
                        __func__, name, finger_ids[i]);
                continue;
            }

            cpl_sprintf((int8_t*) name, (const int8_t*) GF_FINGER_OBJECT_ID_BAK, group_id, slot);

            err = gf_so_delete_persistent_object((int8_t*) name);
            if (GF_SUCCESS != err) {
                GF_LOGE(LOG_TAG "[%s] fail to delete so(%s) for finger(%u)",
                        __func__, name, finger_ids[i]);
                removing_templates--;
                continue;
            }

            g_finger_index_hash_table[slot] = 0;
            err = gf_algo_remove(group_id, finger_ids[i]);

            cmd->deleted_fids[i] = finger_ids[i];
            cmd->deleted_gids[i] = group_ids[i];
        }
        cmd->removing_templates = removing_templates;
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_load_finger(uint32_t group_id, uint32_t finger_index, uint32_t *finger_id) {
    gf_error_t err = GF_SUCCESS;
    uint8_t *buf = NULL;
    uint32_t buf_len = 0;
    uint8_t name[GF_FINGER_OBJECT_ID_MAX_LEN] = { 0 };

    FUNC_ENTER();
    do {

        if (NULL == finger_id) {
            GF_LOGE(LOG_TAG "[%s] bad parameter", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        cpl_sprintf((int8_t*) name, (const int8_t*) GF_FINGER_OBJECT_ID, group_id, finger_index);

        err = gf_so_load_persistent_object((int8_t*) name, &buf, &buf_len);
        if (GF_SUCCESS == err) {
            GF_LOGD(LOG_TAG "[%s] load so(%s) success", __func__, name);

            err = gf_algo_add(buf, buf_len, group_id, finger_id);
            if (GF_SUCCESS == err) {
                gf_error_t tmp_err = GF_SUCCESS;

                GF_LOGD(LOG_TAG "[%s] add finger(%u) success", __func__, *finger_id);

                cpl_sprintf((int8_t*) name, (const int8_t*) GF_FINGER_OBJECT_ID_BAK, group_id,
                        finger_index);
                tmp_err = gf_so_save_persistent_object((int8_t*) name, buf, buf_len);
                if (GF_SUCCESS == tmp_err) {
                    GF_LOGI(LOG_TAG "[%s] sync data to backup object(%s) success", __func__, name);
                } else {
                    GF_LOGE(LOG_TAG "[%s] sync data to backup object(%s) failed.", __func__, name);
                }

                break;
            }

            if (GF_ERROR_REACH_FINGERS_UPLIMIT == err) {
                GF_LOGE(LOG_TAG "[%s] reach to the up limit when add so(%s)", __func__, name);
                break;
            }

            GF_LOGE(LOG_TAG "[%s] fail to add finger(%s)", __func__, name);
        } else {
            GF_LOGD(LOG_TAG "[%s] load so(%s) failed", __func__, name);
        }

        // load the finger from the backup object
        cpl_sprintf((int8_t*) name, (const int8_t*) GF_FINGER_OBJECT_ID_BAK, group_id,
                finger_index);
        err = gf_so_load_persistent_object((int8_t*) name, &buf, &buf_len);
        if (GF_SUCCESS == err) {
            GF_LOGD(LOG_TAG "[%s] load so(%s) success", __func__, name);

            err = gf_algo_add(buf, buf_len, group_id, finger_id);
            if (GF_SUCCESS == err) {
                GF_LOGI(LOG_TAG "[%s] load finger(%u) success", __func__, *finger_id);
            } else {
                GF_LOGE(LOG_TAG "[%s] load finger(%s) failed", __func__, name);
            }
        } else {
            GF_LOGD(LOG_TAG "[%s] load so(%s) failed", __func__, name);
        }
    } while (0);

    if (buf != NULL) {
        CPL_MEM_FREE(buf);
        buf = NULL;
    }

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_switch_fingerprint_user(uint32_t group_id) {
    gf_error_t err = GF_SUCCESS;
    uint32_t i = 0;

    FUNC_ENTER();

    do {
        g_set_active_group_flag = 1;
        GF_LOGD(LOG_TAG "[%s] group_id=%u", __func__, group_id);

        gf_algo_set_active_group(group_id);

        err = gf_ta_load_auth_token(group_id);

        cpl_memset(g_finger_index_hash_table, 0,
                g_config.max_fingers_per_user * sizeof(uint32_t));

        for (i = 0; i < g_config.max_fingers_per_user; i++) {
            uint32_t finger_id = 0;

            err = gf_ta_load_finger(group_id, i, &finger_id);
            if (GF_SUCCESS == err) {
                GF_LOGI(LOG_TAG "[%s] load group_id(%u), index(%u) success",
                        __func__, group_id, i);
                g_finger_index_hash_table[i] = finger_id;
            } else if (GF_ERROR_REACH_FINGERS_UPLIMIT != err) {
                GF_LOGI(LOG_TAG "[%s] load group_id(%u), index(%u) failed",
                        __func__, group_id, i);
                err = GF_SUCCESS;
            } else {
                GF_LOGI(LOG_TAG "[%s] reach to the up limit", __func__);
                err = GF_SUCCESS;
                break;
            }
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_ta_set_active_group(gf_set_active_group_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == cmd) {
            GF_LOGE(LOG_TAG "[%s] bad parameter", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        err = gf_ta_switch_fingerprint_user(cmd->group_id);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_set_safe_class(gf_set_safe_class_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == cmd) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad parameter", __func__);
            break;
        }

        err = gf_algo_set_safe_class(cmd->safe_class);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_ta_enumerate(gf_enumerate_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();
    do {
        if (0 == g_set_active_group_flag) {
            err = GF_ERROR_NO_GROUP_ID;
            break;
        }
        err = gf_algo_enumerate(cmd->group_ids, cmd->finger_ids, &(cmd->size));
    } while (0);

    FUNC_EXIT(err);
    return err;
}

static gf_error_t gf_ta_navigate(gf_nav_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        err = gf_nav_set_mode(cmd->nav_mode);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] navigation set mode failed, err=%s, errno=%d",
                    __func__, gf_strerror(err), err);
            break;
        }

        err = gf_state_execute_operation(OPERATION_NAV, &(cmd->cmd_header.reset_flag));
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] set OPERATION_NAV failed, err=%s, errno=%d",
                    __func__, gf_strerror(err), err);
            break;
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_screen_on(gf_cmd_header_t *cmd) {
    gf_error_t err = GF_SUCCESS;
    gf_operation_type_t operation = OPERATION_NONE;
    gf_operation_type_t cur_operation = OPERATION_NONE;
    gf_operation_type_t top_operation = OPERATION_NONE;

    FUNC_ENTER();

    cur_operation = gf_state_get_current_operation();
    top_operation = gf_state_get_next_operation();
    g_screen_on_flag = 1;

    GF_LOGD(LOG_TAG "[%s] cur_operation=%s, cur_operation_code=%d",
            __func__, gf_stroperation(cur_operation), cur_operation);
    GF_LOGD(LOG_TAG "[%s] top_operation=%s, top_operation_code=%d",
            __func__, gf_stroperation(top_operation), top_operation);
    do {
        if (((gf_state_is_authenticate(cur_operation) > 0) && (0 == g_last_authenticate_result))
                || (gf_state_is_authenticate(top_operation))) {
            operation = OPERATION_AUTHENTICATE_IMAGE;
            break;
        }
        if (OPERATION_NAV == cur_operation) {
            operation = OPERATION_NAV;
            break;
        }

        if (OPERATION_CAMERA_KEY == cur_operation) {
            operation = OPERATION_CAMERA_KEY;
            break;
        }

        operation = gf_state_get_default_operation();
    } while (0);

    // ensure screen on can't be in OPERATION_SCREEN_OFF_SLEEP
    if (OPERATION_SCREEN_OFF_SLEEP == cur_operation) {
        gf_state_cancel_operation(&cmd->reset_flag);
    }

    err = gf_state_execute_operation(operation, &cmd->reset_flag);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_screen_off(gf_cmd_header_t *cmd) {
    gf_error_t err = GF_SUCCESS;
    gf_operation_type_t operation = OPERATION_SCREEN_OFF_SLEEP;
    gf_operation_type_t cur_operation = OPERATION_INVAILD;
    gf_operation_type_t top_operation = OPERATION_INVAILD;

    FUNC_ENTER();

    g_screen_on_flag = 0;
    cur_operation = gf_state_get_current_operation();
    top_operation = gf_state_get_next_operation();

    if (0 == gf_algo_has_enrolled_fingerprints()) {
        gf_state_require_down_and_up_in_pairs_end(&cmd->reset_flag);
        if ((g_config.support_ff_mode > 0)
                && (g_config.support_power_key_feature > 0)) {
            operation = OPERATION_POWER_KEY;
        } else {
            operation = OPERATION_NONE;
        }
    } else if (g_config.support_ff_mode > 0) {
        if (((gf_state_is_authenticate(cur_operation) > 0) && (0 == g_last_authenticate_result))
                || (gf_state_is_authenticate(top_operation) > 0)) {
            operation = OPERATION_AUTHENTICATE_FF;
            g_last_authenticate_result = 0;
        }
    } else {
        if ((gf_state_is_authenticate(cur_operation) > 0 && (0 == g_last_authenticate_result))
                || (gf_state_is_authenticate(top_operation) > 0)) {
            operation = OPERATION_AUTHENTICATE_SLEEP;
        }
    }

    err = gf_state_execute_operation(operation, &cmd->reset_flag);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_camera_capture(gf_cmd_header_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    err = gf_state_execute_operation(OPERATION_CAMERA_KEY, &cmd->reset_flag);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_enable_fingerprint_module(gf_enable_fingerprint_module_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    if (cmd->enable_flag == 0) {
        err = gf_state_execute_operation(OPERATION_NONE, &cmd->cmd_header.reset_flag);
    } else {
        err = gf_state_cancel_test_operation(&cmd->cmd_header.reset_flag);
    }

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_enable_ff_feature(gf_enable_ff_feature_t *cmd) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    g_config.support_ff_mode = cmd->enable_flag;

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_ta_create_entry_point(void) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint32_t size = 0;
        uint32_t origin_size = 0;

        /* init gf_config_t */
        gf_config_init();

        /* init gf_sensor_t */
        err = gf_sensor_init();
        if (err != GF_SUCCESS) {
            break;
        }

        size = sizeof(gf_image_data_t);
        if (sizeof(gf_nav_data_t) > size) {
            size = sizeof(gf_nav_data_t);
        }
        if (sizeof(gf_hbd_data_t) > size) {
            size = sizeof(gf_hbd_data_t);
        }

        if (NULL == g_image_data) {
            g_image_data = CPL_MEM_MALLOC(size);
            if (NULL == g_image_data) {
                err = GF_ERROR_OUT_OF_MEMORY;
                GF_LOGE(LOG_TAG "[%s] out of memory err=%s, errno=%d",
                        __func__, gf_strerror(err), err);
                break;
            }
        }

        cpl_memset(g_image_data, 0, size);

        g_nav_data = (gf_nav_data_t *)g_image_data;
        g_hbd_data = (gf_hbd_data_t *)g_image_data;

        origin_size = sizeof(gf_image_origin_data_t);
        if (sizeof(gf_nav_origin_data_t) > origin_size) {
            origin_size = sizeof(gf_nav_origin_data_t);
        }

        if (NULL == g_image_origin_data) {
            g_image_origin_data = CPL_MEM_MALLOC(origin_size);
            if (NULL == g_image_origin_data) {
                err = GF_ERROR_OUT_OF_MEMORY;
                GF_LOGE(LOG_TAG "[%s] out of memory err=%s, errno=%d",
                        __func__, gf_strerror(err), err);
                break;
            }
        }

        cpl_memset(g_image_origin_data, 0, origin_size);
        g_nav_origin_data = (gf_nav_origin_data_t*) g_image_origin_data;

        if (NULL == g_base_data) {
            g_base_data = CPL_MEM_MALLOC(sizeof(gf_base_data_t));
            if (NULL == g_base_data) {
                err = GF_ERROR_OUT_OF_MEMORY;
                GF_LOGE(LOG_TAG "[%s] out of memory err=%s, errno=%d",
                        __func__, gf_strerror(err), err);
                break;
            }

            cpl_memset(g_base_data, 0, sizeof(gf_base_data_t));
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_ta_set_hmac_key(gf_hmac_key_t* cmd) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_set_hmac_key(cmd->hmac_key, sizeof(cmd->hmac_key));
    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_ta_get_hmac_key(void) {
    uint8_t key[128] = { 0 };
    uint32_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_get_hmac_key(key, 32);

    FUNC_EXIT(err);
    return err;
}

void gf_ta_destroy_entry_point(void) {
    VOID_FUNC_ENTER();

    gf_hw_destroy();

    CPL_MEM_FREE(g_image_data);
    g_image_data = NULL;
    g_nav_data = NULL;
    g_hbd_data = NULL;

    CPL_MEM_FREE(g_image_origin_data);
    g_image_origin_data = NULL;
    g_nav_origin_data = NULL;

    CPL_MEM_FREE(g_base_data);
    g_base_data = NULL;

    gf_algo_destroy();

    if (g_config.support_sensor_broken_check > 0) {
        gf_broken_check_destroy();
    }

    if (g_finger_index_hash_table != NULL) {
        CPL_MEM_FREE(g_finger_index_hash_table);
        g_finger_index_hash_table = NULL;
    }

    VOID_FUNC_EXIT();
}

static void gf_ta_sync_finger_list(void) {
    uint32_t finger_ids[MAX_FINGERS_PER_USER] = { 0 };
    uint32_t group_ids[MAX_FINGERS_PER_USER] = { 0 };
    uint32_t finger_count = 0;

    VOID_FUNC_ENTER();

    gf_algo_enumerate(group_ids, finger_ids, &finger_count);
    gf_sync_finger_list(finger_ids, finger_count);

    VOID_FUNC_EXIT();
}

gf_error_t gf_ta_authenticate_study(gf_authenticate_study_t *cmd) {
    gf_error_t err = GF_SUCCESS;
    gf_save_t save_cmd;
    gf_update_stitch_t update_stitch_cmd;
    FUNC_ENTER();

    do {
        if (g_config.support_sensor_broken_check) {
            err = gf_broken_check(&cmd->broken_check, &cmd->cmd_header.reset_flag, cmd->operation);
            GF_ERROR_BREAK(err);
        }
        if (0 == cmd->broken_check.disable_study) {
            err = gf_algo_finger_feature_study(cmd->finger_id, &cmd->save_flag,
                    &cmd->update_stitch_flag);
            if (GF_SUCCESS != err) {
                GF_LOGE(LOG_TAG "[%s] finger feature study failed err=%d", __func__, err);
                break;
            }

            if (cmd->save_flag > 0) {
                cpl_memset(&save_cmd, 0, sizeof(gf_save_t));
                save_cmd.finger_id = cmd->finger_id;
                save_cmd.group_id = cmd->group_id;
                err = gf_ta_save(&save_cmd);
                if (GF_SUCCESS != err) {
                    GF_LOGE(LOG_TAG "[%s] save finger failed err=%d", __func__, err);
                    break;
                }
            }

            if (cmd->update_stitch_flag > 0) {
                cpl_memset(&update_stitch_cmd, 0, sizeof(gf_update_stitch_t));
                update_stitch_cmd.finger_id = cmd->finger_id;
                update_stitch_cmd.group_id = cmd->group_id;
                err = gf_ta_update_stitch(&update_stitch_cmd);
                if (GF_SUCCESS != err) {
                    GF_LOGE(LOG_TAG "[%s] update stitch failed err=%d", __func__, err);
                }
            }
        }
    } while (0);
    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_ta_invoke_cmd_entry_point(uint32_t cmd_id, void *buffer, uint32_t buffer_len) {
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
        case GF_CMD_DETECT_SENSOR: {
            err = gf_ta_detect_sensor((gf_detect_sensor_t *) buffer);
            break;
        }

        case GF_CMD_EXIT: {
            err = gf_ta_exit(cmd);
            break;
        }

        case GF_CMD_INIT_FINISHED: {
            err = gf_ta_init_finished((gf_init_finished_t*) buffer);
            break;
        }

        case GF_CMD_PRE_ENROLL: {
            err = gf_ta_pre_enroll((gf_pre_enroll_t*) buffer);
            break;
        }

        case GF_CMD_ENROLL: {
            err = gf_ta_enroll((gf_enroll_t*) buffer);
            break;
        }

        case GF_CMD_POST_ENROLL: {
            err = gf_ta_post_enroll();
            break;
        }

        case GF_CMD_CANCEL: {
            err = gf_ta_cancel((gf_cancel_t *) buffer);
            break;
        }

        case GF_CMD_AUTHENTICATE: {
            err = gf_ta_authenticate((gf_authenticate_t*) buffer);
            break;
        }

        case GF_CMD_AUTHENTICATE_FIDO: {
            err = gf_ta_authenticate_fido((gf_authenticate_fido_t*) buffer);
            break;
        }

        case GF_CMD_GET_AUTH_ID: {
            err = gf_ta_get_auth_id((gf_get_auth_id_t*) buffer);
            break;
        }

        case GF_CMD_SAVE: {
            err = gf_ta_save((gf_save_t*) buffer);
            if (GF_SUCCESS == err) {
                gf_ta_sync_finger_list();
            }
            break;
        }

        case GF_CMD_UPDATE_STITCH: {
            err = gf_ta_update_stitch((gf_update_stitch_t*) buffer);
            break;
        }

        case GF_CMD_REMOVE: {
            err = gf_ta_remove((gf_remove_t*) buffer);
            if (GF_SUCCESS == err) {
                gf_ta_sync_finger_list();
            }
            break;
        }

        case GF_CMD_SET_ACTIVE_GROUP: {
            err = gf_ta_set_active_group((gf_set_active_group_t*) buffer);
            if (GF_SUCCESS == err) {
                gf_ta_sync_finger_list();
            }
            break;
        }

        case GF_CMD_SET_SAFE_CLASS: {
            err = gf_ta_set_safe_class((gf_set_safe_class_t*) buffer);
            break;
        }

        case GF_CMD_ENUMERATE: {
            err = gf_ta_enumerate((gf_enumerate_t*) buffer);
            break;
        }

        case GF_CMD_NAVIGATE: {
            err = gf_ta_navigate((gf_nav_t *) buffer);
            break;
        }

        case GF_CMD_IRQ: {
            err = gf_irq((gf_irq_t*) buffer);
            break;
        }

        case GF_CMD_SCREEN_ON: {
            err = gf_ta_screen_on((gf_cmd_header_t *) buffer);
            break;
        }

        case GF_CMD_SCREEN_OFF: {
            err = gf_ta_screen_off((gf_cmd_header_t *) buffer);
            break;
        }

        case GF_CMD_CAMERA_CAPTURE: {
            err = gf_ta_camera_capture((gf_cmd_header_t *) buffer);
            break;
        }

        case GF_CMD_ENABLE_FINGERPRINT_MODULE: {
            err = gf_ta_enable_fingerprint_module((gf_enable_fingerprint_module_t*) buffer);
            break;
        }

        case GF_CMD_ENABLE_FF_FEATURE: {
            err = gf_ta_enable_ff_feature((gf_enable_ff_feature_t*) buffer);
            break;
        }

        case GF_CMD_AUTHENTICATE_STUDY: {
            err = gf_ta_authenticate_study((gf_authenticate_study_t*)buffer);
            break;
        }

        default: {
            err = gf_ta_chip_series_invoke_cmd_entry_point(cmd_id, buffer);
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
