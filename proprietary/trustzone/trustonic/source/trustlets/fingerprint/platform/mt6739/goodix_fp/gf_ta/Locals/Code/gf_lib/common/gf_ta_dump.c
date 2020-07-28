/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#include "gf_error.h"
#include "gf_common.h"
#include "gf_algo.h"
#include "gf_tee_internal_api.h"
#include "gf_ta_dump.h"
#include "cpl_string.h"
#include "gf_nav.h"

#define LOG_TAG "[gf_ta_dump]"

extern gf_image_data_t *g_image_data;
extern gf_nav_data_t *g_nav_data;
extern gf_hbd_data_t *g_hbd_data;
extern gf_base_data_t *g_base_data;

extern gf_image_origin_data_t *g_image_origin_data;
extern gf_nav_origin_data_t *g_nav_origin_data;

static gf_error_t gf_ta_dump_template(void *buffer) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        gf_dump_template_t *template = (gf_dump_template_t *) buffer;
        uint32_t group_id = template->group_id;
        uint32_t finger_id = template->finger_id;

        err = gf_algo_get_fingerTempInfo_size(finger_id, &(template->template_len));
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] get finger(%u) size", __func__, finger_id);
            break;
        }

        err = gf_algo_get_finger_pdu(group_id, finger_id, template->template_data,
                template->template_len);
        if (err != GF_SUCCESS) {
            break;
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_dump_data(void *buffer) {
    gf_error_t err = GF_SUCCESS;
    gf_dump_data_t *dump_data = (gf_dump_data_t *) buffer;
    uint16_t* base = NULL;
    uint32_t base_len = 0;

    FUNC_ENTER();

    switch (dump_data->operation) {
        case OPERATION_ENROLL:
        case OPERATION_TEST_UNTRUSTED_ENROLL:
        case OPERATION_AUTHENTICATE_FF:
        case OPERATION_AUTHENTICATE_IMAGE:
        case OPERATION_TEST_UNTRUSTED_AUTHENTICATE:
        case OPERATION_AUTHENTICATE_FIDO:
        case OPERATION_TEST_PERFORMANCE:
            gf_algo_dump_algo_debug_info(dump_data->operation, g_image_data);
            cpl_memcpy(&dump_data->data.image, g_image_data, sizeof(gf_image_data_t));
            break;

        case OPERATION_FINGER_BASE:
            cpl_memcpy(dump_data->data.image.raw_data, g_base_data->finger_base_rawdata,
                    g_base_data->finger_base_rawdata_len * sizeof(uint16_t));
            dump_data->data.image.raw_data_len = g_base_data->finger_base_rawdata_len;
            gf_algo_dump_algo_debug_info(dump_data->operation, &dump_data->data.image);
            break;

        case OPERATION_NAV:
            cpl_memcpy(&dump_data->data.nav, g_nav_data, sizeof(gf_nav_data_t));
            break;

        case OPERATION_NAV_BASE:
            gf_nav_get_base(&base, &base_len);
            cpl_memcpy(dump_data->data.nav.raw_data[0], base, base_len * sizeof(uint16_t));
            dump_data->data.nav.raw_data_len = base_len;
            break;

        case OPERATION_HEARTBEAT_KEY:
        case OPERATION_TEST_BIO_CALIBRATION:
        case OPERATION_TEST_HBD_CALIBRATION:
            cpl_memcpy(&dump_data->data.hbd, g_hbd_data, sizeof(gf_hbd_data_t));
            break;

        default:
            cpl_memcpy(&dump_data->data.image, g_image_data, sizeof(gf_image_data_t));
            break;
    }

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_ta_dump_origin_data(void *buffer) {
    gf_error_t err = GF_SUCCESS;
    gf_dump_origin_data_t *dump_data = (gf_dump_origin_data_t *) buffer;

    FUNC_ENTER();

    switch (dump_data->operation) {
        case OPERATION_ENROLL:
            case OPERATION_TEST_UNTRUSTED_ENROLL:
            case OPERATION_AUTHENTICATE_FF:
            case OPERATION_AUTHENTICATE_IMAGE:
            case OPERATION_TEST_UNTRUSTED_AUTHENTICATE:
            case OPERATION_AUTHENTICATE_FIDO:
            case OPERATION_TEST_PERFORMANCE:
            cpl_memcpy(&dump_data->data.origin_image, g_image_origin_data,
                    sizeof(gf_image_origin_data_t));
            break;
        case OPERATION_NAV:
            cpl_memcpy(&dump_data->data.origin_nav, g_nav_origin_data,
                    sizeof(gf_nav_origin_data_t));
            break;
        default:
            cpl_memcpy(&dump_data->data.origin_image, g_image_origin_data,
                    sizeof(gf_image_origin_data_t));
            break;
    }

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_ta_invoke_dump_cmd_entry_point(uint32_t cmd_id, void *buffer, uint32_t buffer_len) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    if (NULL == buffer || buffer_len < sizeof(gf_cmd_header_t)) {
        return GF_ERROR_BAD_PARAMS;
    }

    GF_LOGD(LOG_TAG "[%s] cmd_id=%s, cmd_id=%u", __func__, gf_strcmd((gf_cmd_id_t) cmd_id), cmd_id);

    switch (cmd_id) {
        case GF_CMD_DUMP_TEMPLATE:
            err = gf_ta_dump_template(buffer);
            break;

        case GF_CMD_DUMP_DATA:
            err = gf_ta_dump_data(buffer);
            break;

        case GF_CMD_DUMP_ORIGIN_DATA:
            err = gf_ta_dump_origin_data(buffer);
            break;

        default: {
            GF_LOGE(LOG_TAG "[%s] invalid parameters cmd_id=%s, cmd_id_code=%u", __func__,
                    gf_strcmd((gf_cmd_id_t) cmd_id), cmd_id);
            err = GF_ERROR_UNKNOWN_CMD;
            break;
        }
    }

    FUNC_EXIT(err);

    return err;
}

