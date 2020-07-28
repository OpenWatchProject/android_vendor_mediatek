/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#include "cpl_string.h"
#include "cpl_memory.h"
#include "cpl_math.h"
#include "gf_tee_internal_api.h"
#include "gf_common.h"
#include "gf_error.h"
#include "gf_sensor.h"
#include "milan_e_hv_config.h"
#include "milan_j_hv_config.h"
#include "milan_fn_hv_config.h"
#include "milan_hv_bus.h"
#include "milan_hv_common.h"
#include "milan_hv_regs.h"
#include "gf_config.h"
#include "gf_type_define.h"
#include "gf_vfw.h"
#include "gf_vfw_milan_e_hv.h"
#include "gf_vfw_milan_fn_hv.h"
#include "gf_vfw_milan_j_hv.h"

#define LOG_TAG "[gf_vfw]"

#define MILAN_HV_CHIP_FLOW_VERSION "v1.0.0"

#define MILAN_HV_PIXEL_TEST_DAC_UPDATE_VALUE        20
#define MILAN_HV_OTP_INFO_CHECK_MAX_RETRY_TIMES     9

#define MILAN_HV_READ_NAV_DATA_MAX_RETRY_TIMES   (1000)

#define MILAN_HV_TCODE_BASE_VALUE 71

#define TOUCH_DIFF_TABLE_SIZE 36

static uint16_t g_touch_diff_table_5266[] = {
    847, 801, 759, 718, 679, 643, 609, 577, 547, 520,
    490, 490, 446, 446, 412, 412, 400, 400, 400, 400,
    365, 365, 365, 365, 308, 308, 308, 308, 300, 300,
    300, 300, 300, 300, 300, 300
};

static uint16_t g_touch_diff_table_6226[] = {
     647, 601, 559, 518, 479, 443, 409, 377, 347, 320,
     290, 290, 246, 246, 212, 212, 196, 196, 196, 196,
     187, 187, 187, 187, 178, 178, 178, 178, 168, 168,
     168, 168, 159, 159, 159, 159
};

static gf_error_t vfw_read_fdt_data_without_trigger(uint16_t *fdt_data);
static gf_error_t vfw_dac_init(void);
static gf_error_t vfw_set_state(milan_hv_chip_state_t state, uint8_t need_trigger);
/********************* Global Variable Definition Start *********************/

static uint16_t g_fdt_base_buf[MILAN_HV_FDT_BASE_LEN] = { 0 };
static uint16_t g_saved_fdt_down_base[MILAN_HV_FDT_BASE_LEN] = { 0 };
static uint8_t g_is_fdt_down_base_saved = 0;

static milan_hv_vfw_handle_t g_vfw_handle = { 0 };

// static uint8_t g_filed_down_base[MILAN_HV_FDT_BASE_LEN] = { 0 };
// static uint8_t g_is_filed_down_base_cleard = 1;

/********************* Global Variable Definition End *********************/

/********************* Internal Function Definition Start *********************/
static gf_error_t vfw_function_init(void) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do {
        g_vfw_handle.function.get_otp_data_crc = NULL;
        g_vfw_handle.function.set_general_reg = NULL;
        g_vfw_handle.function.update_dac_to_regs = NULL;

        if (MILAN_HV_CHIP_ID_IS_EQUAL(g_vfw_handle.chip_id, MILAN_E_HV_CHIP_ID)) {
            g_vfw_handle.function.get_otp_data_crc = vfw_milan_e_hv_get_otp_data_crc;
            g_vfw_handle.function.set_general_reg = vfw_milan_e_hv_set_general_reg;
            g_vfw_handle.function.update_dac_to_regs = vfw_milan_e_hv_update_dac_to_regs;
            break;
        }

        if (MILAN_HV_CHIP_ID_IS_EQUAL(g_vfw_handle.chip_id, MILAN_J_HV_CHIP_ID)) {
            g_vfw_handle.function.get_otp_data_crc = vfw_milan_j_hv_get_otp_data_crc;
            g_vfw_handle.function.set_general_reg = vfw_milan_j_hv_set_general_reg;
            g_vfw_handle.function.update_dac_to_regs = vfw_milan_j_hv_update_dac_to_regs;
            break;
        }

        if (MILAN_HV_CHIP_ID_IS_EQUAL(g_vfw_handle.chip_id, MILAN_FN_HV_CHIP_ID)) {
            g_vfw_handle.function.get_otp_data_crc = vfw_milan_fn_hv_get_otp_data_crc;
            g_vfw_handle.function.set_general_reg = vfw_milan_fn_hv_set_general_reg;
            g_vfw_handle.function.update_dac_to_regs = vfw_milan_fn_hv_update_dac_to_regs;
            break;
        }

        err = GF_ERROR_BAD_PARAMS;
        GF_LOGE(LOG_TAG "[%s] unsupport chip id=%d", __func__, g_vfw_handle.chip_id);
    } while (0);

    FUNC_EXIT(err);
    return err;
}

static void vfw_init_diff_table(void) {
    VOID_FUNC_ENTER();

    if (GF_CHIP_5266 == g_config.chip_type) {
        g_vfw_handle.touch_diff_table = g_touch_diff_table_5266;
    } else {
        g_vfw_handle.touch_diff_table = g_touch_diff_table_6226;
    }

    VOID_FUNC_EXIT();
}

/* only old otp version module could enable/disable fdt tx strategy */
static void vfw_init_fdt_tx_strategy(void) {
    VOID_FUNC_ENTER();

    if (HV_OLD_OTP_VERSION == g_vfw_handle.otp_version
        && 1 == g_config.enable_fdt_tx_strategy) {
        g_vfw_handle.fdt_tx_strategy = HV_FDT_ENABLE_TX_STRATEGY;
    } else {
        g_vfw_handle.fdt_tx_strategy = HV_FDT_DISABLE_TX_STRATEGY;
    }

    VOID_FUNC_EXIT();
}
/* if fdt tx enable, nav must be enable, otherwise could config it */
static void vfw_init_nav_tx_strategy(void) {
    VOID_FUNC_ENTER();

    if (HV_FDT_ENABLE_TX_STRATEGY == g_vfw_handle.fdt_tx_strategy) {
        g_vfw_handle.nav_tx_strategy = HV_NAV_ENABLE_TX_STRATEGY;
        g_config.enable_nav_tx_strategy = 1;
    } else {
        if (1 == g_config.enable_nav_tx_strategy) {
            g_vfw_handle.nav_tx_strategy = HV_NAV_ENABLE_TX_STRATEGY;
        } else {
            g_vfw_handle.nav_tx_strategy = HV_NAV_DISABLE_TX_STRATEGY;
        }
    }
    VOID_FUNC_EXIT();
}

static void vfw_init_tx_strategy(void) {
    VOID_FUNC_ENTER();

    vfw_init_fdt_tx_strategy();
    vfw_init_nav_tx_strategy();

    VOID_FUNC_EXIT();
}

static gf_error_t vfw_read_otp_data(uint8_t *buf) {
    gf_error_t err = GF_SUCCESS;

    uint16_t reg_data = 0;
    uint8_t bank_num = 0;
    uint8_t bank_len = 0;
    uint8_t *dst = buf;

    FUNC_ENTER();

    do {
        if (NULL == buf) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad parameter", __func__);
            break;
        }

        err = gf_milan_hv_write_word_with_endian_exchange(MILAN_HV_REG_OTP_CTRL1, 0x0020);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] write 0x%04X register error", __func__, MILAN_HV_REG_OTP_CTRL1);
            break;
        }

        for (bank_num = 0; bank_num < MILAN_HV_OTP_DATA_BANK_NUM; bank_num++) {
            gf_milan_hv_write_word_with_endian_exchange(MILAN_HV_REG_OTP_CTRL2, bank_num);

            gf_milan_hv_write_word_with_endian_exchange(MILAN_HV_REG_OTP_CTRL1, 0x0021);
            gf_milan_hv_write_word_with_endian_exchange(MILAN_HV_REG_OTP_CTRL1, 0x0020);

            for (bank_len = 0; bank_len < MILAN_HV_OTP_DATA_BANK_LEN; bank_len++) {
                gf_milan_hv_write_word_with_endian_exchange(MILAN_HV_REG_OTP_CTRL2,
                        (bank_len << 8));
                gf_milan_hv_write_word_with_endian_exchange(MILAN_HV_REG_OTP_CTRL1, 0x0022);
                gf_milan_hv_write_word_with_endian_exchange(MILAN_HV_REG_OTP_CTRL1, 0x0020);

                gf_milan_hv_read_word_with_endian_exchange(MILAN_HV_REG_OTP_CTRL3, &reg_data);
                *dst++ = (uint8_t) (reg_data & 0xFF);

                GF_LOGD(LOG_TAG "[%s] otp_data[%d], read 0x%04X register value=0x%04X", __func__,
                        (bank_num * 4 + bank_len), MILAN_HV_REG_OTP_CTRL3, reg_data);
            }
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t vfw_parse_otp_version(void) {
    gf_error_t err = GF_SUCCESS;
    // uint8_t ft_day;

    FUNC_ENTER();

    do {
        uint8_t ft_year;
        uint8_t ft_month;
        g_vfw_handle.otp_version = HV_NONE_OTP_VERSION;

        ft_year  = (g_vfw_handle.otp_data[0x08] & 0x3E) >> 1;
        // ft_day   = ( g_vfw_handle.otp_data[0x09] & 0x7C) >> 2;
        ft_month = ((g_vfw_handle.otp_data[0x08] & 0xC0) >> 6)
            | (2 << (g_vfw_handle.otp_data[0x09] & 0x03));

        GF_LOGD(LOG_TAG "[%s] FT time:%02d-%02d", __func__, ft_year, ft_month);

        if ((ft_year <= 16) || (ft_year == 17 && ft_month < 3)) {
            GF_LOGD(LOG_TAG "[%s] Old otp version found", __func__);
            g_vfw_handle.otp_version = HV_OLD_OTP_VERSION;
            break;
        }

        if (0xE0 == g_vfw_handle.otp_data[0x1C] && 0x00 != g_vfw_handle.otp_data[0x1B]) {
            GF_LOGD(LOG_TAG "[%s] New otp version found", __func__);
            g_vfw_handle.otp_version = HV_NEW_OTP_VERSION;
            break;
        }

        GF_LOGE(LOG_TAG "[%s] invalid otp version", __func__);
        err = GF_ERROR_INVALID_OTP_VERSION;
    } while (0);

    /* when update otp version, update tx strategy as the same time */
    if (GF_SUCCESS == err) {
        vfw_init_tx_strategy();
    }

    FUNC_EXIT(err);

    return err;
}

static gf_error_t vfw_otp_data_crc(uint8_t* otp_buf) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do {
        uint8_t crc = 0;
        if (NULL == otp_buf || NULL == g_vfw_handle.function.get_otp_data_crc) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad parameter", __func__);
            break;
        }

        err = vfw_parse_otp_version();
        GF_ERROR_BREAK(err);

        if (GF_CHIP_5266 == g_config.chip_type
            && HV_NEW_OTP_VERSION == g_vfw_handle.otp_version) {
            g_vfw_handle.function.get_otp_data_crc = vfw_milan_j_hv_get_otp_data_crc;
        }

        crc = g_vfw_handle.function.get_otp_data_crc(otp_buf);

        if (crc != otp_buf[0x19]) {
            err = GF_ERROR_SENSOR_TEST_FAILED;
            GF_LOGE(LOG_TAG "[%s] OTP CRC check failed", __func__);
            break;
        }

        GF_LOGI(LOG_TAG "[%s] OTP CRC check success", __func__);
    } while (0);

    FUNC_EXIT(err);
    return err;
}

static uint16_t vfw_parse_fdt_tcode(void) {
    uint16_t fdt_tcode = 0;

    VOID_FUNC_ENTER();

    // new otp version
    if (HV_NEW_OTP_VERSION == g_vfw_handle.otp_version) {
        fdt_tcode = g_vfw_handle.otp_data[0x1B];
    // old otp version
    } else {
        // disable tx strategy
        if (0 == g_config.enable_fdt_tx_strategy) {
            fdt_tcode = HV_TCODE_MAX;
        // enable tx strategy
        } else {
            fdt_tcode = g_vfw_handle.otp_data[0x17] + 1;
        }
    }
    VOID_FUNC_EXIT();

    return fdt_tcode;
}

static uint32_t vfw_parse_touch_diff(void) {
    uint32_t touch_diff = 0;

    VOID_FUNC_ENTER();
    // new otp version
    if (HV_NEW_OTP_VERSION == g_vfw_handle.otp_version) {
        touch_diff = (((g_vfw_handle.otp_data[0x11] & 0x3E) >> 1) + 1) * 16;
    // old otp version
    } else {
        // disable tx strategy
        uint8_t index = (g_vfw_handle.otp_data[0x17] - MILAN_HV_TCODE_BASE_VALUE) / 5;
        if (0 == g_config.enable_fdt_tx_strategy) {
            if (index >= TOUCH_DIFF_TABLE_SIZE) {
                // typical value
                index = 21;
                GF_LOGE(LOG_TAG "[%s] hv_tcode is not in the table", __func__);
            }
            touch_diff = g_vfw_handle.touch_diff_table[index];
        // enable tx strategy
        } else {
            touch_diff = (((g_vfw_handle.otp_data[0x11] & 0x3E) >> 1) + 5) * 50;
        }
    }
    VOID_FUNC_EXIT();

    return touch_diff;
}

static gf_error_t vfw_parse_otp_info(uint8_t* otp_buf) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint16_t fdt_tcode = 0;
        uint32_t touch_diff = 0;
        uint32_t fdt_delta = 0;
        err = vfw_parse_otp_version();
        GF_ERROR_BREAK(err);

        fdt_tcode = vfw_parse_fdt_tcode();

        touch_diff = vfw_parse_touch_diff();

        fdt_delta = (touch_diff / 16) / 3;

        g_vfw_handle.otp_info.valid = 1;
        g_vfw_handle.otp_info.product_id = otp_buf[0x10];
        g_vfw_handle.otp_info.otp_tcode = otp_buf[0x17] + 1;
        g_vfw_handle.otp_info.fdt_tcode = fdt_tcode;
        g_vfw_handle.otp_info.fdt_down_delta = fdt_delta;
        g_vfw_handle.otp_info.fdt_up_delta = fdt_delta - 2;

        g_vfw_handle.otp_info.dac_h = (((otp_buf[0x11]) & 0x01) << 8) | (otp_buf[0x16]);
        g_vfw_handle.otp_info.dac_l = (((otp_buf[0x11]) & 0x40) << 2) | (otp_buf[0x1F]);

        GF_LOGI(LOG_TAG "[%s] get otp info success:", __func__);
        GF_LOGI(LOG_TAG "[%s] product_id=0x%02X", __func__, g_vfw_handle.otp_info.product_id);
        GF_LOGI(LOG_TAG "[%s] otp_tcode=0x%04X", __func__, g_vfw_handle.otp_info.otp_tcode);
        GF_LOGI(LOG_TAG "[%s] fdt_tcode=0x%04X", __func__, g_vfw_handle.otp_info.fdt_tcode);
        GF_LOGI(LOG_TAG "[%s] fdt_down_delta=0x%04X",
            __func__, g_vfw_handle.otp_info.fdt_down_delta);
        GF_LOGI(LOG_TAG "[%s] fdt_up_delta=0x%04X", __func__, g_vfw_handle.otp_info.fdt_up_delta);
        GF_LOGI(LOG_TAG "[%s] dac_h=0x%04X", __func__, g_vfw_handle.otp_info.dac_h);
        GF_LOGI(LOG_TAG "[%s] dac_l=0x%04X", __func__, g_vfw_handle.otp_info.dac_l);
    } while (0);

    FUNC_EXIT(err);
    return err;
}

static gf_error_t vfw_check_otp_data(uint8_t retry_count, uint8_t* otp_buf) {
    gf_error_t err = GF_SUCCESS;
    uint16_t i = 0;
    uint8_t temp_data[GF_SENSOR_OTP_INFO_LEN] = { 0 };

    FUNC_ENTER();

    do {
        if (NULL == otp_buf) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad parameter", __func__);
            break;
        }

        for (i = 0; i < retry_count; i++) {
            cpl_memset(temp_data, 0, sizeof(temp_data));

            err = vfw_read_otp_data(temp_data);
            if (err != GF_SUCCESS) {
                gf_sleep(5);
                continue;
            }

            err = vfw_otp_data_crc(temp_data);
            if (err != GF_SUCCESS) {
                gf_sleep(5);
                continue;
            }

            cpl_memcpy(otp_buf, temp_data, sizeof(temp_data));
            break;
        }
    } while (0);

    FUNC_EXIT(err);
    return err;
}

static void vfw_update_tcode_into_config_array(void) {
    uint16_t tcode = g_vfw_handle.otp_info.otp_tcode;
    uint16_t fdt_tcode = g_vfw_handle.otp_info.fdt_tcode;

    VOID_FUNC_ENTER();

    MILAN_HV_UPDATE_CONFIG_ARRAY_BY_WORD(g_vfw_handle.config->ff,
            MILAN_HV_REG_PIXEL_CTRL6, fdt_tcode);
    MILAN_HV_UPDATE_CONFIG_ARRAY_BY_WORD(g_vfw_handle.config->fdt_manual_down,
            MILAN_HV_REG_PIXEL_CTRL6, fdt_tcode);
    MILAN_HV_UPDATE_CONFIG_ARRAY_BY_WORD(g_vfw_handle.config->fdt_down,
            MILAN_HV_REG_PIXEL_CTRL6, fdt_tcode);
    MILAN_HV_UPDATE_CONFIG_ARRAY_BY_WORD(g_vfw_handle.config->fdt_up,
            MILAN_HV_REG_PIXEL_CTRL6, fdt_tcode);
    MILAN_HV_UPDATE_CONFIG_ARRAY_BY_WORD(g_vfw_handle.config->image,
            MILAN_HV_REG_PIXEL_CTRL6, tcode);
    MILAN_HV_UPDATE_CONFIG_ARRAY_BY_WORD(g_vfw_handle.config->nav_fdt_down,
            MILAN_HV_REG_PIXEL_CTRL6, fdt_tcode);
    MILAN_HV_UPDATE_CONFIG_ARRAY_BY_WORD(g_vfw_handle.config->nav_fdt_up,
            MILAN_HV_REG_PIXEL_CTRL6, fdt_tcode);

    if (HV_NAV_ENABLE_TX_STRATEGY == g_vfw_handle.nav_tx_strategy) {
        MILAN_HV_UPDATE_CONFIG_ARRAY_BY_WORD(g_vfw_handle.config->nav,
            MILAN_HV_REG_PIXEL_CTRL6, tcode);
        MILAN_HV_UPDATE_CONFIG_ARRAY_BY_WORD(g_vfw_handle.config->nav_base,
            MILAN_HV_REG_PIXEL_CTRL6, tcode);
    } else {
        MILAN_HV_UPDATE_CONFIG_ARRAY_BY_WORD(g_vfw_handle.config->nav,
            MILAN_HV_REG_PIXEL_CTRL6, fdt_tcode);
        MILAN_HV_UPDATE_CONFIG_ARRAY_BY_WORD(g_vfw_handle.config->nav_base,
            MILAN_HV_REG_PIXEL_CTRL6, fdt_tcode);
    }

    GF_LOGI(LOG_TAG "[%s] update to arrays tcode=0x%04X", __func__, tcode);

    VOID_FUNC_EXIT();
}

static void vfw_update_fdt_threshold_into_config_array(void) {
    uint16_t fdt_delta = g_vfw_handle.otp_info.fdt_down_delta;

    VOID_FUNC_ENTER();

    MILAN_HV_UPDATE_CONFIG_ARRAY_BY_BYTE(g_vfw_handle.config->ff, MILAN_HV_REG_FDT_DELTA,
            fdt_delta);
    MILAN_HV_UPDATE_CONFIG_ARRAY_BY_BYTE(g_vfw_handle.config->fdt_manual_down,
            MILAN_HV_REG_FDT_DELTA, fdt_delta);
    MILAN_HV_UPDATE_CONFIG_ARRAY_BY_BYTE(g_vfw_handle.config->fdt_down, MILAN_HV_REG_FDT_DELTA,
            fdt_delta);
    MILAN_HV_UPDATE_CONFIG_ARRAY_BY_BYTE(g_vfw_handle.config->fdt_up, MILAN_HV_REG_FDT_DELTA,
            fdt_delta - 2);
    MILAN_HV_UPDATE_CONFIG_ARRAY_BY_BYTE(g_vfw_handle.config->nav_fdt_down, MILAN_HV_REG_FDT_DELTA,
            fdt_delta);
    MILAN_HV_UPDATE_CONFIG_ARRAY_BY_BYTE(g_vfw_handle.config->nav_fdt_up, MILAN_HV_REG_FDT_DELTA,
            fdt_delta - 2);

    GF_LOGI(LOG_TAG "[%s] update to arrays fdt_delta=0x%04X", __func__, fdt_delta);

    VOID_FUNC_EXIT();
}

static void vfw_update_scan_mode_into_config_array(void) {
    uint16_t reg_data = 0;

    VOID_FUNC_ENTER();

    // update scan mode register to MILAN_HV_REG_PIXEL_CTRL10, for IMAGE configuration
    MILAN_HV_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(g_vfw_handle.config->image,
            MILAN_HV_REG_PIXEL_CTRL10, reg_data);

    reg_data = (reg_data & 0xFFF9) | (g_vfw_handle.scan_mode << 1);
    MILAN_HV_UPDATE_CONFIG_ARRAY_BY_WORD(g_vfw_handle.config->image, MILAN_HV_REG_PIXEL_CTRL10,
            reg_data);

    VOID_FUNC_EXIT();
}

/* init handle struct image extra data rows */
static void vfw_image_extra_data_rows_init(void) {
    uint16_t reg_data = 0;

    VOID_FUNC_ENTER();

    MILAN_HV_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(g_vfw_handle.config->image,
            MILAN_HV_REG_ENCRYPT_CTRL3, reg_data);
    if (1 == (reg_data & RG_EXTRA_DATA_EN_MASK)) {
        g_vfw_handle.image_add_extra_data_rows = (reg_data & 0xff00) >> 8;
    } else {
        g_vfw_handle.image_add_extra_data_rows = g_sensor.row;
    }
    GF_LOGI(LOG_TAG "[%s] image extra_data_rows=%u", __func__,
            g_vfw_handle.image_add_extra_data_rows);

    VOID_FUNC_EXIT();
}

/* init handle struct nav extra data rows */
static void vfw_nav_extra_data_rows_init(void) {
    uint16_t reg_data = 0;

    VOID_FUNC_ENTER();

    MILAN_HV_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(g_vfw_handle.config->nav,
            MILAN_HV_REG_ENCRYPT_CTRL3, reg_data);
    if (1 == (reg_data & RG_EXTRA_DATA_EN_MASK)) {
        g_vfw_handle.nav_add_extra_data_rows = (reg_data & 0xff00) >> 8;
    } else {
        if (MILAN_HV_CHIP_ID_IS_EQUAL(g_vfw_handle.chip_id, MILAN_FN_HV_CHIP_ID)) {
            g_vfw_handle.nav_add_extra_data_rows = g_sensor.col;
        } else {
            g_vfw_handle.nav_add_extra_data_rows = g_sensor.row;
        }
    }

    GF_LOGI(LOG_TAG "[%s] nav extra_data_rows=%u", __func__,
            g_vfw_handle.image_add_extra_data_rows);

    VOID_FUNC_EXIT();
}

/* init handle struct extra data rows */
static void vfw_extra_data_rows_init(void) {
    VOID_FUNC_ENTER();

    vfw_image_extra_data_rows_init();
    vfw_nav_extra_data_rows_init();

    VOID_FUNC_EXIT();
}

/* init handle struct image double_rate_flag */
static void vfw_image_double_rate_flag_init(void) {
    uint16_t reg_data = 0;

    VOID_FUNC_ENTER();

    MILAN_HV_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(g_vfw_handle.config->image,
            MILAN_HV_REG_PIXEL_CTRL10, reg_data);
    g_vfw_handle.image_double_rate_flag = reg_data & RG_DOUBLE_RATE_MASK;

    GF_LOGI(LOG_TAG "[%s] image_double_rate_flag=%u", __func__,
            g_vfw_handle.image_double_rate_flag);

    VOID_FUNC_EXIT();
}

/* init handle struct nav double_rate_flag */
static void vfw_nav_double_rate_flag_init(void) {
    uint16_t reg_data = 0;

    VOID_FUNC_ENTER();

    MILAN_HV_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(g_vfw_handle.config->nav,
            MILAN_HV_REG_PIXEL_CTRL10, reg_data);
    g_vfw_handle.nav_double_rate_flag = reg_data & RG_DOUBLE_RATE_MASK;

    GF_LOGI(LOG_TAG "[%s] nav_double_rate_flag=%u", __func__, g_vfw_handle.nav_double_rate_flag);

    VOID_FUNC_EXIT();
}

/* init handle struct double_rate_flag */
static void vfw_double_rate_flag_init(void) {
    VOID_FUNC_ENTER();

    vfw_image_double_rate_flag_init();
    vfw_nav_double_rate_flag_init();

    VOID_FUNC_EXIT();
}

static gf_error_t vfw_config_init(void) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint32_t i = 0;
        const milan_hv_vendor_params_t* vendor_param = NULL;
        switch (g_vfw_handle.chip_id >> 8) {
            case MILAN_E_HV_CHIP_ID: {
                g_vfw_handle.config = &g_milan_e_hv_config;
                vendor_param = g_milan_e_hv_vendor_params;
                g_vfw_handle.scan_mode = SCAN_MODE0;
                g_vfw_handle.pixel_test_dac_update_value = MILAN_HV_PIXEL_TEST_DAC_UPDATE_VALUE;
                break;
            }

            case MILAN_FN_HV_CHIP_ID: {
                g_vfw_handle.config = &g_milan_fn_hv_config;
                vendor_param = g_milan_fn_hv_vendor_params;
                g_vfw_handle.pixel_test_dac_update_value = MILAN_HV_PIXEL_TEST_DAC_UPDATE_VALUE;
                break;
            }

            case MILAN_J_HV_CHIP_ID: {
                uint16_t reg_data = 0;
                g_vfw_handle.config = &g_milan_j_hv_config;
                vendor_param = g_milan_j_hv_vendor_params;
                g_vfw_handle.vendor_id = VENDOR_ID_A;

                MILAN_HV_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(g_vfw_handle.config->image,
                        MILAN_HV_REG_PIXEL_CTRL10, reg_data);
                g_vfw_handle.scan_mode = (reg_data >> 1) & RG_SCAN_MODE_MASK;
                g_vfw_handle.pixel_test_dac_update_value = MILAN_HV_PIXEL_TEST_DAC_UPDATE_VALUE;
                break;
            }

            default: {
                err = GF_ERROR_SENSOR_NOT_AVAILABLE;
                GF_LOGE(LOG_TAG "[%s] unsupport HV sensor, chip id=0x%08X", __func__,
                        g_vfw_handle.chip_id);
                break;
            }
        }

        GF_ERROR_BREAK(err);

        for (i = 0; i < MILAN_HV_VENDOR_MAX_COUNT; i++) {
            if (g_vfw_handle.vendor_id == vendor_param[i].vendor_id) {
                break;
            }
        }

        GF_LOGI(LOG_TAG "[%s] vendor id=0x%04X ", __func__, g_vfw_handle.vendor_id);

        cpl_memcpy(&(g_vfw_handle.vendor_param), &(vendor_param[i]),
                sizeof(milan_hv_vendor_params_t));

        vfw_update_scan_mode_into_config_array();

        vfw_extra_data_rows_init();

        vfw_double_rate_flag_init();

        if (0 == g_vfw_handle.otp_info.valid) {
            err = GF_ERROR_SENSOR_TEST_FAILED;
            GF_LOGE(LOG_TAG "[%s] OTP crc check failed in boot phase", __func__);
            break;
        }

        /* Use otp information to fill configuration and regsiters */
        vfw_update_tcode_into_config_array();

        vfw_update_fdt_threshold_into_config_array();

        err = vfw_dac_init();
    } while (0);

    FUNC_EXIT(err);
    return err;
}

static gf_error_t vfw_test_dac_init(void) {
    uint16_t tx_code = 0;
    uint16_t reg_data = 0;
    uint32_t i = 0;
    gf_error_t err = GF_SUCCESS;

    VOID_FUNC_ENTER();

    // read tx clock number
    MILAN_HV_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(g_vfw_handle.config->image,
            MILAN_HV_REG_PIXEL_CTRL6, reg_data);

    tx_code = reg_data & 0x1FFF;
    GF_LOGI(LOG_TAG "[%s] tx_code=0x%04X", __func__, tx_code);

    if (0 == tx_code) {
        err = GF_ERROR_BAD_PARAMS;
        GF_LOGE(LOG_TAG "[%s] tx_code is zero", __func__);
        return err;
    }

    for (i = 0; i < MILAN_HV_DAC_GROUPS_MAX_NUM; i++) {
        GF_LOGD(LOG_TAG "[%s] default_dac[%u]=0x%04X", __func__, i, g_vfw_handle.default_dac[i]);

        g_vfw_handle.sensor_broken.positive_dac[i] =
                g_vfw_handle.default_dac[i] + BROKEN_CHECK_DAC_DELTA;

        GF_LOGD(LOG_TAG "[%s] sensor_broke.positive_dac[%u]=0x%04X", __func__, i,
                g_vfw_handle.sensor_broken.positive_dac[i]);

        g_vfw_handle.sensor_broken.negative_dac[i] =
                g_vfw_handle.default_dac[i] - BROKEN_CHECK_DAC_DELTA;

        GF_LOGD(LOG_TAG "[%s] sensor_broke.negative_dac[%u]=0x%04X", __func__, i,
                g_vfw_handle.sensor_broken.negative_dac[i]);

        g_vfw_handle.pixel_test_chip_info.positive_dac[i] =
                g_vfw_handle.default_dac[i] + MILAN_HV_PIXEL_TEST_DAC_DELTA;

        GF_LOGD(LOG_TAG "[%s] pixel_test_chip_info.positive_dac[%u]=0x%04X", __func__, i,
                g_vfw_handle.pixel_test_chip_info.positive_dac[i]);
    }

    if (MILAN_HV_CHIP_ID_IS_EQUAL(g_vfw_handle.chip_id, MILAN_FN_HV_CHIP_ID)) {
        g_vfw_handle.pixel_open_test_delta = 0.3120 * tx_code * MILAN_HV_PIXEL_TEST_DAC_DELTA / 3;
    } else {
        g_vfw_handle.pixel_open_test_delta = 0.283 * tx_code * MILAN_HV_PIXEL_TEST_DAC_DELTA / 3;
    }
    g_sensor.pixel_open_threshold = g_vfw_handle.pixel_open_test_delta;

    VOID_FUNC_EXIT();
    return err;
}

static gf_error_t vfw_dac_init(void) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint32_t i = 0;

        GF_LOGI(LOG_TAG "[%s] dac_h=0x%4X, dac_l=0x%4X", __func__, g_vfw_handle.otp_info.dac_h,
                g_vfw_handle.otp_info.dac_l);

        // use dac_l as default dac when system startup
        for (i = 0; i < MILAN_HV_DAC_GROUPS_MAX_NUM; i++) {
            g_vfw_handle.default_dac[i] = g_vfw_handle.otp_info.dac_l;
        }

        vfw_test_dac_init();

        err = g_vfw_handle.function.update_dac_to_regs(g_vfw_handle.default_dac);
    } while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 * [vfw_enable_hv: enable high-voltage]
 */
static gf_error_t vfw_enable_hv(void) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    err = gf_milan_hv_write_cmd_data(SPI_WRITE_HV, 0x08);

    FUNC_EXIT(err);
    return err;
}

static gf_error_t vfw_set_tx_strategy(milan_hv_chip_state_t state) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    switch (state) {
        // control by fdt_tx_strategy
        case STATE_FDT_DOWN:
        case STATE_FDT_UP:
        case STATE_FDT_NAV_DOWN:
        case STATE_FDT_NAV_UP: {
            if (HV_FDT_ENABLE_TX_STRATEGY == g_vfw_handle.fdt_tx_strategy) {
                err = gf_vfw_enable_tx();
            } else {
                err = gf_vfw_disable_tx();
            }
            break;
        }

        // control by nav_tx_strategy
        case STATE_NAV: {
            if (HV_NAV_ENABLE_TX_STRATEGY == g_vfw_handle.nav_tx_strategy) {
                err = gf_vfw_enable_tx();
            } else {
                err = gf_vfw_disable_tx();
            }
            break;
        }

        // image must be enable tx
        case STATE_IMAGE_BROKEN_CHECK_DEFAULT:
        case STATE_IMAGE_BROKEN_CHECK_NEGATIVE:
        case STATE_IMAGE_BROKEN_CHECK_POSITIVE:
        case STATE_IMAGE:
        case STATE_IMAGE_CONTINUE:
        case STATE_IMAGE_TEST_PIXEL_OPEN_DEFAULT:
        case STATE_IMAGE_TEST_PIXEL_OPEN_POSITIVE:
        case STATE_IMAGE_TEST_BAD_POINT: {
            err = gf_vfw_enable_tx();
            break;
        }

        // tx strategy in finger/nav base will be control in gf_base
        case STATE_IMAGE_FINGER_BASE:
        case STATE_NAV_BASE:
        case STATE_FDT_MANUAL_DOWN: {
            break;
        }

        default: {
            break;
        }
    }  // end switch case

    FUNC_EXIT(err);
    return err;
}

/**
 * [vfw_disable_hv: disable high-voltage]
 */
static gf_error_t vfw_disable_hv(void) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    err = gf_milan_hv_write_cmd_data(SPI_WRITE_HV, 0x00);

    FUNC_EXIT(err);
    return err;
}

/**
 * [vfw_set_dac_h: set the work dac value is high-voltage dac]
 */
static gf_error_t vfw_set_dac_h(void) {
    gf_error_t err = GF_SUCCESS;
    uint32_t i = 0;

    FUNC_ENTER();

    for (i = 0; i < MILAN_HV_DAC_GROUPS_MAX_NUM; i++) {
        g_vfw_handle.default_dac[i] = g_vfw_handle.cur_dac_h;
    }
    err = g_vfw_handle.function.update_dac_to_regs(g_vfw_handle.default_dac);

    FUNC_EXIT(err);
    return err;
}

/**
 * [vfw_set_dac_l: set the work dac value is low-voltage dac]
 */
static gf_error_t vfw_set_dac_l(void) {
    gf_error_t err = GF_SUCCESS;
    uint32_t i = 0;
    FUNC_ENTER();

    for (i = 0; i < MILAN_HV_DAC_GROUPS_MAX_NUM; i++) {
        g_vfw_handle.default_dac[i] = g_vfw_handle.otp_info.dac_l;
    }

    err = g_vfw_handle.function.update_dac_to_regs(g_vfw_handle.default_dac);

    FUNC_EXIT(err);
    return err;
}

/**
 * [vfw_update_fdt_up_base_into_config_array: update fdt up base in config arrays]
 */
static void vfw_update_fdt_up_base_into_config_array(uint16_t* fdt_data, uint16_t fdt_touch_flag) {
    uint32_t i = 0;
    uint16_t fdt_up_base[MILAN_HV_FDT_BASE_LEN];
    uint16_t fdt_thres_offset = MILAN_HV_REG_FDT_THRES - MILAN_HV_CONFIG_ADDRESS_START;

    VOID_FUNC_ENTER();

    GF_LOGI(LOG_TAG "[%s] fdt_down_delta=0x%04X, fdt_up_delta=0x%04X, fdt_touch_flag=0x%04X",
            __func__, g_vfw_handle.otp_info.fdt_down_delta, g_vfw_handle.otp_info.fdt_up_delta,
            fdt_touch_flag);

    for (i = 0; i < MILAN_HV_FDT_BASE_LEN; i++) {
        uint32_t offset = fdt_thres_offset + i * 2;
        uint8_t value = 0;

        if (0 == (fdt_touch_flag & (1 << i))) {
            fdt_up_base[i] = g_vfw_handle.otp_info.fdt_up_delta;
        } else {
            fdt_up_base[i] = fdt_data[i] + g_vfw_handle.otp_info.fdt_down_delta;
        }

        /* Because 0x84~0x9A register only 8bits, so just use low 8bits */
        value = fdt_up_base[i] & 0xFF;

        GF_LOGI(LOG_TAG "[%s] fdt_up_base[%d]=0x%04X", __func__, i, fdt_up_base[i]);
        GF_LOGI(LOG_TAG "[%s] value=0x%02X", __func__, value);

        g_vfw_handle.config->fdt_up[offset] = value;
        g_vfw_handle.config->nav_fdt_up[offset] = value;
    }

    VOID_FUNC_EXIT();
}

/**
 * [vfw_update_fdt_down_base_into_config_array: update fdt down base in config arrays]
 */
static void vfw_update_fdt_down_base_into_config_array(uint16_t* fdt_data) {
    uint32_t i;
    uint16_t fdt_thres_offset = MILAN_HV_REG_FDT_THRES - MILAN_HV_CONFIG_ADDRESS_START;

    VOID_FUNC_ENTER();

    for (i = 0; i < MILAN_HV_FDT_BASE_LEN; i++) {
        uint32_t offset = fdt_thres_offset + i * 2;
        uint8_t value = 0;

        /* Because 0x84~0x9A register only 8bits, so just use low 8bits */
        value = fdt_data[i] & 0xFF;

        GF_LOGI(LOG_TAG "[%s] fdt_down_base[%d]=0x%04X", __func__, i, fdt_data[i]);
        GF_LOGI(LOG_TAG "[%s] value[%d]=0x%04X", __func__, i, value);

        g_vfw_handle.config->ff[offset] = value;
        g_vfw_handle.config->fdt_down[offset] = value;
        g_vfw_handle.config->fdt_manual_down[offset] = value;
        g_vfw_handle.config->nav_fdt_down[offset] = value;
    }

    VOID_FUNC_EXIT();
}

static gf_error_t vfw_get_state_config(milan_hv_chip_state_t state, uint8_t *config) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do {
        if (NULL == config) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] config is NULL", __func__);
            break;
        }

        switch (state) {
            case STATE_FDT_DOWN: {
                cpl_memcpy(config, g_vfw_handle.config->fdt_down, MILAN_HV_CONFIG_LENGTH);
                break;
            }

            case STATE_FDT_UP: {
                cpl_memcpy(config, g_vfw_handle.config->fdt_up, MILAN_HV_CONFIG_LENGTH);
                break;
            }

            case STATE_FDT_MANUAL_DOWN: {
                cpl_memcpy(config, g_vfw_handle.config->fdt_manual_down, MILAN_HV_CONFIG_LENGTH);
                break;
            }

            case STATE_FDT_NAV_DOWN: {
                cpl_memcpy(config, g_vfw_handle.config->nav_fdt_down, MILAN_HV_CONFIG_LENGTH);
                break;
            }

            case STATE_FDT_NAV_UP: {
                cpl_memcpy(config, g_vfw_handle.config->nav_fdt_up, MILAN_HV_CONFIG_LENGTH);
                break;
            }

            case STATE_NAV: {
                cpl_memcpy(config, g_vfw_handle.config->nav, MILAN_HV_CONFIG_LENGTH);
                break;
            }

            case STATE_NAV_BASE: {
                cpl_memcpy(config, g_vfw_handle.config->nav_base, MILAN_HV_CONFIG_LENGTH);
                break;
            }

            case STATE_IMAGE_BROKEN_CHECK_DEFAULT:
            case STATE_IMAGE_BROKEN_CHECK_NEGATIVE:
            case STATE_IMAGE_BROKEN_CHECK_POSITIVE: {
                cpl_memcpy(config, g_vfw_handle.config->image, MILAN_HV_CONFIG_LENGTH);
                MILAN_HV_UPDATE_CONFIG_ARRAY_BY_WORD(config, MILAN_HV_REG_PIXEL_CTRL6,
                    BROKEN_CHECK_TCODE);
                break;
            }

            case STATE_IMAGE:
            case STATE_IMAGE_FINGER_BASE:
            case STATE_IMAGE_TEST_PIXEL_OPEN_DEFAULT:
            case STATE_IMAGE_TEST_PIXEL_OPEN_POSITIVE:
            case STATE_IMAGE_TEST_BAD_POINT_FINGER_BASE:
            case STATE_IMAGE_TEST_BAD_POINT: {
                cpl_memcpy(config, g_vfw_handle.config->image, MILAN_HV_CONFIG_LENGTH);
                break;
            }

            case STATE_IMAGE_CONTINUE: {
                cpl_memcpy(config, g_vfw_handle.config->image, MILAN_HV_CONFIG_LENGTH);
                // if current mode is image_continue_mode ,set it
                MILAN_HV_UPDATE_CONFIG_ARRAY_BY_WORD(config, MILAN_HV_REG_PIXEL_CTRL1, 0x0002);
                break;
            }

            case STATE_IDLE:
            case STATE_SLEEP: {
                break;
            }

            default: {
                err = GF_ERROR_BAD_PARAMS;
                GF_LOGE(LOG_TAG "[%s] not support the state", __func__);
                break;
            }
        }  // end switch case
    } while (0);

    FUNC_EXIT(err);
    return err;
}

static gf_error_t vfw_download_config(uint8_t *config) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do {
        if (NULL == config) {
            GF_LOGE(LOG_TAG "[%s] config is NULL", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        err = gf_milan_hv_write_bytes(MILAN_HV_CONFIG_ADDRESS_START, config,
                MILAN_HV_CONFIG_LENGTH);
        GF_ERROR_BREAK(err);

        gf_sleep(3);
    } while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 * [vfw_set_state: set chip state]
 * @param  state [which will set to chip]
 * @param  need_trigger [wheather need trigger]
 * @return       [gf_error]
 */
static gf_error_t vfw_set_state(milan_hv_chip_state_t state, uint8_t need_trigger) {
    gf_error_t err = GF_SUCCESS;
    milan_hv_chip_state_t chip_mode = state & CHIP_MODE_MASK;
    milan_hv_cmd_t cmd = CMD_IDLE;

    FUNC_ENTER();
    GF_LOGD(LOG_TAG "[%s] vfw set state=%s", __func__, gf_vfw_strstate(state));

    do {
        switch (chip_mode) {
            case CHIP_MODE_IDLE:
                cmd = CMD_IDLE;
                break;

            case CHIP_MODE_SLEEP: {
                uint16_t irq_type = 0xFFFF;
                err = vfw_set_state(STATE_IDLE, 0);
                GF_ERROR_BREAK(err);

                // Just reduce the register operation to improve the performance,for FDT DOWN/FDT
                // UP/IMAGE/NAV,
                // register 0124 will be set to be 0xFF FF. so don't write 0x0124 register at here.
                // Check if there is any IRQ haven't been cleared. First clear exist IRQ.
                gf_milan_hv_write_word_with_endian_exchange(MILAN_HV_REG_IRQ_CTRL2, irq_type);

                cmd = CMD_SLEEP;
                break;
            }

            case CHIP_MODE_FDT:
                err = vfw_set_state(STATE_IDLE, 0);
                GF_ERROR_BREAK(err);

                err = vfw_set_dac_l();
                GF_ERROR_BREAK(err);

                err = vfw_disable_hv();
                GF_ERROR_BREAK(err);

                cmd = CMD_FDT;
                break;

            case CHIP_MODE_IMAGE:
                err = vfw_set_state(STATE_IDLE, 0);
                GF_ERROR_BREAK(err);

                if (STATE_IMAGE_BROKEN_CHECK_DEFAULT == state) {
                    err = vfw_set_dac_l();
                    GF_ERROR_BREAK(err);
                } else if (STATE_IMAGE_FINGER_BASE == state || STATE_IMAGE_TEST_BAD_POINT == state
                        || STATE_IMAGE_TEST_PIXEL_OPEN_DEFAULT == state
                        || STATE_IMAGE_TEST_BAD_POINT_FINGER_BASE == state) {
                    err = vfw_set_dac_l();
                    GF_ERROR_BREAK(err);
                } else if (STATE_IMAGE_BROKEN_CHECK_POSITIVE == state) {
                    err = g_vfw_handle.function.update_dac_to_regs(
                            g_vfw_handle.sensor_broken.positive_dac);
                    GF_ERROR_BREAK(err);
                } else if (STATE_IMAGE_BROKEN_CHECK_NEGATIVE == state) {
                    err = g_vfw_handle.function.update_dac_to_regs(
                            g_vfw_handle.sensor_broken.negative_dac);
                    GF_ERROR_BREAK(err);
                } else if (STATE_IMAGE_TEST_PIXEL_OPEN_POSITIVE == state) {
                    err = g_vfw_handle.function.update_dac_to_regs(
                            g_vfw_handle.pixel_test_chip_info.positive_dac);
                    GF_ERROR_BREAK(err);
                } else {
                    err = vfw_set_dac_h();
                    GF_ERROR_BREAK(err);
                }

                if (STATE_IMAGE_TEST_BAD_POINT == state
                    || STATE_IMAGE_TEST_BAD_POINT_FINGER_BASE == state) {
                    err = vfw_disable_hv();
                    GF_ERROR_BREAK(err);
                } else {
                    err = vfw_enable_hv();
                    GF_ERROR_BREAK(err);
                }

                cmd = CMD_IMG;
                break;

            case CHIP_MODE_NAV:
                err = vfw_set_state(STATE_IDLE, 0);
                GF_ERROR_BREAK(err);

                err = vfw_set_dac_l();
                GF_ERROR_BREAK(err);

                err = vfw_disable_hv();
                GF_ERROR_BREAK(err);

                cmd = CMD_NAV;
                break;

            default:
                err = GF_ERROR_BAD_PARAMS;
                break;
        }

        GF_ERROR_BREAK(err);

        // when callers set mode to idle or sleep ,don't need download config
        if (((CHIP_MODE_IDLE != chip_mode) && (CHIP_MODE_SLEEP != chip_mode))
                && (STATE_IMAGE_TEST_PIXEL_OPEN_POSITIVE != state)) {
            uint8_t config[MILAN_HV_CONFIG_LENGTH] = { 0 };
            err = vfw_get_state_config(state, config);
            GF_ERROR_BREAK(err);

            err = vfw_download_config(config);
            GF_ERROR_BREAK(err);
        }

        err = vfw_set_tx_strategy(state);
        GF_ERROR_BREAK(err);

        // write cmd to chip
        err = gf_milan_hv_write_cmd(cmd);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] write CMD failed, cmd=0x%02X", __func__, cmd);
            break;
        }
        gf_sleep(1);

        // trigger a image when cur mode is image or nav
        if (1 == need_trigger) {
            if ((CHIP_MODE_IMAGE == chip_mode) || (CHIP_MODE_NAV == chip_mode)) {
                gf_milan_hv_write_word_with_endian_exchange(MILAN_HV_REG_PIXEL_CTRL2, 0x0110);
            }
        }

        g_vfw_handle.cur_chip_state = state;
    } while (0);

    FUNC_EXIT(err);
    return err;
}

static gf_error_t vfw_read_fdt_data_without_trigger(uint16_t *fdt_data) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do {
        uint32_t i = 0;
        uint16_t reg_data = 0;
        uint8_t offset_bits = 0;
        uint16_t fdt_up_base_with_tx[MILAN_HV_FDT_BASE_LEN];

        err = vfw_set_state(STATE_IDLE, 0);
        GF_ERROR_BREAK(err);

        err = gf_milan_hv_read_bytes_with_endian_exchange(MILAN_HV_REG_FDT_AVG,
                (uint8_t*) fdt_up_base_with_tx,
                MILAN_HV_FDT_BASE_LEN * sizeof(uint16_t));
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] read fdt base(enable_tx) failed", __func__);
            break;
        }

        MILAN_HV_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(g_vfw_handle.config->fdt_manual_down,
                MILAN_HV_REG_PIXEL_CTRL4, reg_data);
        if (FDT_DETECT_AREA_IS_1X8(reg_data)) {
            offset_bits = 1;
        } else {
            offset_bits = 4;
        }

        for (i = 0; i < MILAN_HV_FDT_BASE_LEN; i++) {
            fdt_data[i] = (fdt_up_base_with_tx[i] & 0xFFF) >> offset_bits;
            GF_LOGD(LOG_TAG "[%s] fdt_data[%d]=0x%04X", __func__, i, fdt_data[i]);
        }
    } while (0);

    FUNC_EXIT(err);
    return err;
}

static gf_error_t vfw_get_fdt_down_base_from_config(uint16_t fdt_base_area_num,
        uint16_t *fdt_base) {
    gf_error_t err = GF_SUCCESS;
    uint16_t fdt_thres_offset = MILAN_HV_REG_FDT_THRES - MILAN_HV_CONFIG_ADDRESS_START;
    uint8_t i = 0;

    FUNC_ENTER();

    do {
        if (NULL == fdt_base) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad parameters.", __func__);
            break;
        }

        for (i = 0; i < fdt_base_area_num; i++) {
            uint32_t offset = fdt_thres_offset + i * 2;
            fdt_base[i] = g_vfw_handle.config->fdt_down[offset];
            GF_LOGD(LOG_TAG "[%s] fdt_base[%d]=0x%04X", __func__, i, fdt_base[i]);
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t vfw_get_fdt_diff_bigger_count(uint16_t *fdt_raw_data,
        uint16_t *fdt_base, uint16_t fdt_base_area_num, uint16_t *num) {
    gf_error_t err = GF_SUCCESS;
    uint16_t count = 0;
    uint8_t i = 0;

    FUNC_ENTER();
    do {
        if (NULL == fdt_raw_data || NULL == fdt_base || NULL == num) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad parameters.", __func__);
            break;
        }

        /*
         * Caculate every area |FDT rawdata - FDT down base| > down_delta count.
        */
        for (i = 0; i < fdt_base_area_num; i++) {
            if (cpl_abs(fdt_raw_data[i] - fdt_base[i])
                    > g_vfw_handle.otp_info.fdt_down_delta) {
                count++;
            }
        }
        GF_LOGD(LOG_TAG "[%s] bigger count=%u", __func__, count);
        *num = count;
    } while (0);

    FUNC_EXIT(err);
    return err;
}

static gf_error_t vfw_get_fdt_diff_smaller_count(uint16_t *fdt_raw_data,
        uint16_t *fdt_base, uint16_t fdt_base_area_num, uint16_t *num) {
    gf_error_t err = GF_SUCCESS;
    uint16_t count = 0;
    uint8_t i = 0;

    FUNC_ENTER();
    do {
        if (NULL == fdt_raw_data || NULL == fdt_base || NULL == num) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad parameters.", __func__);
            break;
        }

        /*
         * Caculate every area |FDT rawdata - FDT down base| < down_delta / 3  count.
        */
        for (i = 0; i < fdt_base_area_num; i++) {
            if (cpl_abs(fdt_raw_data[i] - fdt_base[i])
                    < g_vfw_handle.otp_info.fdt_down_delta / 3) {
                count++;
            }
        }
        GF_LOGD(LOG_TAG "[%s] smaller count=%u", __func__, count);
        *num = count;
    } while (0);

    FUNC_EXIT(err);
    return err;
}


static gf_error_t vfw_save_fdt_down_base(uint16_t *fdt_base, uint16_t fdt_base_area_num) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == fdt_base) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad parameters.", __func__);
            break;
        }

        /*
         * save the raw data to g_saved_fdt_down_base.
        */
        cpl_memcpy(g_saved_fdt_down_base, fdt_base, fdt_base_area_num * sizeof(uint16_t));
        g_is_fdt_down_base_saved = 1;
        GF_LOGD(LOG_TAG "[%s] save fdt down base", __func__);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

/********************* Internal Function Definition End *********************/

/********************* External Function Definition Start *********************/

/**
 * [gf_vfw_switch_mode : switch chip state when an IRQ coming
 */
gf_error_t gf_vfw_switch_mode(void) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    GF_LOGD(LOG_TAG "[%s] cur_chip_state=%s, mode=%s", __func__,
            gf_vfw_strstate(g_vfw_handle.cur_chip_state), gf_strmode(g_vfw_handle.mode));

    switch (g_vfw_handle.mode) {
        case MODE_NAV: {
            if (STATE_FDT_NAV_DOWN == g_vfw_handle.cur_chip_state) {
                err = vfw_set_state(STATE_NAV, 1);
            } else if (STATE_NAV == g_vfw_handle.cur_chip_state) {
                err = vfw_set_state(STATE_FDT_NAV_UP, 0);
            } else {
                err = vfw_set_state(STATE_FDT_NAV_DOWN, 0);
            }
            break;
        }

        case MODE_KEY: {
            if (STATE_FDT_DOWN == g_vfw_handle.cur_chip_state) {
                err = vfw_set_state(STATE_FDT_UP, 0);
            } else {
                err = vfw_set_state(STATE_FDT_DOWN, 0);
            }
            break;
        }

        case MODE_IMAGE:
        case MODE_FF: {
            if (STATE_FDT_DOWN == g_vfw_handle.cur_chip_state) {
                err = vfw_set_state(STATE_IMAGE, 1);
            } else if (STATE_IMAGE == g_vfw_handle.cur_chip_state) {
                err = vfw_set_state(STATE_FDT_UP, 0);
            } else {
                err = vfw_set_state(STATE_FDT_DOWN, 0);
            }
            break;
        }

        case MODE_IMAGE_CONTINUE: {
            if (STATE_FDT_DOWN == g_vfw_handle.cur_chip_state) {
                err = vfw_set_state(STATE_IMAGE_CONTINUE, 1);
            } else if (STATE_IMAGE_CONTINUE == g_vfw_handle.cur_chip_state) {
                err = vfw_set_state(STATE_FDT_UP, 0);
            } else {
                err = vfw_set_state(STATE_FDT_DOWN, 0);
            }
            break;
        }

        case MODE_TEST_BAD_POINT: {
            if (STATE_FDT_DOWN == g_vfw_handle.cur_chip_state) {
                err = vfw_set_state(STATE_IMAGE_TEST_BAD_POINT, 1);
            } else if (STATE_IMAGE_TEST_BAD_POINT == g_vfw_handle.cur_chip_state) {
                err = vfw_set_state(STATE_FDT_UP, 0);
            } else {
                err = vfw_set_state(STATE_FDT_DOWN, 0);
            }
            break;
        }

        case MODE_BROKEN_CHECK_DEFAULT:
        case MODE_BROKEN_CHECK_NEGATIVE:
        case MODE_BROKEN_CHECK_POSITIVE: {
            if (STATE_FDT_UP != g_vfw_handle.cur_chip_state) {
                err = vfw_set_state(STATE_FDT_UP, 0);
            }
            break;
        }

        default: {
            GF_LOGD(LOG_TAG "[%s] mode=%d not supported", __func__, g_vfw_handle.mode);
            break;
        }
    }

    GF_LOGD(LOG_TAG "[%s] cur_chip_state=%s",
            __func__, gf_vfw_strstate(g_vfw_handle.cur_chip_state));

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_vfw_init(void) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();
    vfw_init_diff_table();
    err = vfw_function_init();
    FUNC_EXIT(err);

    return err;
}
/*
void gf_vfw_clear_filed_down_base(void) {
    VOID_FUNC_ENTER();
    g_is_filed_down_base_cleard = 1;
    VOID_FUNC_EXIT();
}
*/
gf_error_t gf_vfw_init_otp(uint8_t *hal_otp_data, uint32_t *hal_otp_data_len, uint8_t *tee_otp_data,
        uint32_t *tee_otp_data_len) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == hal_otp_data || NULL == hal_otp_data_len || NULL == tee_otp_data
                || NULL == tee_otp_data_len) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad parameter, &hal_otp_data=%p, &hal_otp_data_len=%p, "
                    "&tee_otp_data=%p, &tee_otp_data_len=%p", __func__, (void *)hal_otp_data,
                    (void *)hal_otp_data_len, (void *)tee_otp_data, (void *)tee_otp_data_len);
            break;
        }

        err = vfw_read_otp_data(g_vfw_handle.otp_data);
        GF_ERROR_BREAK(err);

        // TEE OTP data is exist
        if (*tee_otp_data_len != 0) {
            if (0 == cpl_memcmp(g_vfw_handle.otp_data, tee_otp_data, GF_SENSOR_ID_LEN)) {
                cpl_memcpy(g_vfw_handle.otp_data, tee_otp_data, GF_SENSOR_OTP_INFO_LEN);
                GF_LOGI(LOG_TAG "[%s] Use TEE storage OTP as system OTP data", __func__);
                break;
            }
        }

        // HAL OTP data is exist
        if (*hal_otp_data_len != 0) {
            if (0 == cpl_memcmp(g_vfw_handle.otp_data, hal_otp_data, GF_SENSOR_ID_LEN)) {
                cpl_memcpy(g_vfw_handle.otp_data, hal_otp_data, GF_SENSOR_OTP_INFO_LEN);
                GF_LOGI(LOG_TAG "[%s] Use HAL storage OTP as system OTP data", __func__);
                break;
            }
        }

        // Use OTP data which in the chip OTP zone
        err = vfw_otp_data_crc(g_vfw_handle.otp_data);
        if (GF_SUCCESS == err) {
            GF_LOGI(LOG_TAG "[%s] Use Chip internal OTP as system OTP data", __func__);
            break;
        }

        // Retry to access OTP
        err = vfw_check_otp_data(MILAN_HV_OTP_INFO_CHECK_MAX_RETRY_TIMES, g_vfw_handle.otp_data);
    } while (0);

    if (GF_SUCCESS == err) {
        GF_LOGD(LOG_TAG "[%s] check sensor OTP data success", __func__);
        vfw_parse_otp_info(g_vfw_handle.otp_data);

        g_vfw_handle.cur_dac_h = g_vfw_handle.otp_info.dac_h;

        cpl_memcpy(tee_otp_data, g_vfw_handle.otp_data, GF_SENSOR_OTP_INFO_LEN);
        cpl_memcpy(hal_otp_data, g_vfw_handle.otp_data, GF_SENSOR_OTP_INFO_LEN);
        *tee_otp_data_len = GF_SENSOR_OTP_INFO_LEN;
        *hal_otp_data_len = GF_SENSOR_OTP_INFO_LEN;
    } else {
        g_vfw_handle.otp_info.valid = 0;
        g_vfw_handle.otp_data[0x1A] = 0xF1;
        g_vfw_handle.otp_data[0x1B] = 0xFB;
        g_vfw_handle.otp_data[0x1C] = 0x09;

        GF_LOGE(LOG_TAG "[%s] check sensor OTP data failed, set 0x0220 register=0x08", __func__);
        gf_milan_hv_write_word_with_endian_exchange(MILAN_HV_REG_RG_PIXEL_CTRL0, 0x08);

        if (MILAN_HV_CHIP_ID_IS_EQUAL(g_vfw_handle.chip_id, MILAN_J_HV_CHIP_ID)) {
            GF_LOGI(LOG_TAG "[%s] set dac register=0x00", __func__);
            gf_milan_hv_write_word_with_endian_exchange(MILAN_HV_REG_RG_PIXEL_DAC_P2, 0x00);
            gf_milan_hv_write_word_with_endian_exchange(MILAN_HV_REG_RG_PIXEL_DAC_P3, 0x00);
            gf_milan_hv_write_word_with_endian_exchange(MILAN_HV_REG_RG_PIXEL_DAC_P4, 0x00);
        }
    }
    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_vfw_get_vendor_id(uint16_t *vendor_id) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do {
        if (NULL == vendor_id) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad parameter", __func__);
            break;
        }

        *vendor_id = g_vfw_handle.vendor_id;
    } while (0);

    FUNC_EXIT(err);
    return err;
}

void gf_vfw_chip_handle_init(void) {
    cpl_memset(&g_vfw_handle, 0, sizeof(milan_hv_vfw_handle_t));

    g_vfw_handle.vendor_id = VENDOR_ID_A;
    g_vfw_handle.mode = MODE_IDLE;
    g_vfw_handle.cur_chip_state = CHIP_MODE_IDLE;
    g_vfw_handle.img_fifo_spi_rate = 5;
    g_vfw_handle.nav_fifo_spi_rate = 5;
    g_vfw_handle.is_report_finger_up_for_ff_or_image_mode = 1;
    g_vfw_handle.is_received_fdt_up_for_ff_or_image_mode = 0;
    g_vfw_handle.init_complete_flag = 0;
    g_vfw_handle.fdt_threshold = 8;
    g_vfw_handle.fdt_tx_strategy = HV_FDT_NONE_TX_STRATEGY;
    g_vfw_handle.nav_tx_strategy = HV_NAV_NONE_TX_STRATEGY;
    g_vfw_handle.current_tx_status = HV_TX_NONE;
}

gf_error_t gf_vfw_enable_spi_bypass_deglitch(void) {
    uint16_t reg_data;
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        // enhancement spi bypass feature
        reg_data = 0;
        gf_milan_hv_read_word_with_endian_exchange(MILAN_HV_REG_IO_CTRL0, &reg_data);

        reg_data |= 0x8000;
        gf_milan_hv_write_word_with_endian_exchange(MILAN_HV_REG_IO_CTRL0, reg_data);

        // clear
        reg_data = 0;
        gf_milan_hv_read_word_with_endian_exchange(MILAN_HV_REG_SPI_CTRL0, &reg_data);
        reg_data |= 0x0001;
        gf_milan_hv_write_word_with_endian_exchange(MILAN_HV_REG_SPI_CTRL0, reg_data);
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_vfw_chip_init(void) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    GF_LOGD(LOG_TAG "[%s] MILAN HV CHIP FLOW VERSION=%s", __func__, MILAN_HV_CHIP_FLOW_VERSION);

    do {
        uint16_t reg_data = 0x1010;
        // set gpio-ctrl [reg:0x0008] as down status
        gf_milan_hv_write_word_with_endian_exchange(MILAN_HV_REG_GPIO_CTRL, reg_data);

        // read vendor id
        gf_milan_hv_read_word_with_endian_exchange(MILAN_HV_REG_VENDOR_ID,
                &(g_vfw_handle.vendor_id));
        GF_LOGI(LOG_TAG "[%s] vendor id=0x%04X", __func__, g_vfw_handle.vendor_id);

        // set gpio-ctrl [reg:0x0008] as floating status
        reg_data = 0x1111;
        gf_milan_hv_write_word_with_endian_exchange(MILAN_HV_REG_GPIO_CTRL, reg_data);

        err = vfw_config_init();
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] init configuration failed", __func__);
            break;
        }

        reg_data = 0xFFFF;  // clear all irq at the boot phase.
        gf_milan_hv_write_word_with_endian_exchange(MILAN_HV_REG_IRQ_CTRL2, reg_data);

        gf_vfw_enable_spi_bypass_deglitch();

        // read nav col start, col step
        MILAN_HV_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(g_vfw_handle.config->nav,
                MILAN_HV_REG_PIXEL_CTRL8, reg_data);
        g_sensor.nav_gap = (reg_data & 0xFF00) >> 8;

        // read nav col num
        MILAN_HV_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(g_vfw_handle.config->nav,
                MILAN_HV_REG_PIXEL_CTRL9, reg_data);
        g_sensor.nav_col = reg_data & 0x00FF;

        GF_LOGD(LOG_TAG "[%s] override nav_gap=%u, nav_col=%u",
                __func__, g_sensor.nav_gap, g_sensor.nav_col);

        if (g_vfw_handle.function.set_general_reg != NULL) {
            GF_LOGI(LOG_TAG "[%s] system startup. set general registers", __func__);
            g_vfw_handle.function.set_general_reg();
        }
        g_vfw_handle.init_complete_flag = 1;
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_vfw_get_image(uint8_t *origin_data, uint32_t *origin_data_len, uint8_t frame_num) {
    gf_error_t err = GF_SUCCESS;
    uint16_t reg_data = 0;
    FUNC_ENTER();

    do {
        uint32_t data_len = 0;
        if (NULL == origin_data || NULL == origin_data_len) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad parameter", __func__);
            break;
        }

        // check fifo data
        gf_milan_hv_read_word_with_endian_exchange(MILAN_HV_REG_FIFO_USED, &reg_data);
        GF_LOGD(LOG_TAG "[%s] reg_data=0x%4x", __func__, reg_data);
        if (reg_data == 0) {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        data_len = (g_vfw_handle.image_add_extra_data_rows * g_sensor.col * 3) / 2;
        *origin_data_len = frame_num * (data_len + CRC32_SIZE);

        err = gf_milan_hv_read_bytes(MILAN_HV_REG_DATA_READ, origin_data, *origin_data_len,
                g_vfw_handle.img_fifo_spi_rate);
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_vfw_get_nav_base_image(uint8_t *origin_data, uint32_t *origin_data_len) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint32_t data_len = 0;
        if (NULL == origin_data || NULL == origin_data_len) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad parameter", __func__);
            break;
        }

        data_len = g_vfw_handle.nav_add_extra_data_rows * g_sensor.nav_col * 3 / 2;
        *origin_data_len = data_len + 4;

        err = gf_milan_hv_read_bytes(MILAN_HV_REG_DATA_READ, origin_data, data_len + 4,
                g_vfw_handle.nav_fifo_spi_rate);
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_vfw_get_nav_image(uint8_t *origin_data, uint32_t *origin_data_len) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint16_t reg_data = 0;
        uint32_t data_len = 0;
        uint32_t read_counter = 0;
        for (; read_counter < MILAN_HV_READ_NAV_DATA_MAX_RETRY_TIMES; read_counter++) {
            reg_data = 0;
            gf_milan_hv_read_word_with_endian_exchange(MILAN_HV_REG_FIFO_USED, &reg_data);
            GF_LOGD(LOG_TAG "[%s] reg_data = 0x%4x", __func__, reg_data);
            if (reg_data > 0) {
                break;
            } else {
                gf_sleep(1);
            }
        }

        if (read_counter >= MILAN_HV_READ_NAV_DATA_MAX_RETRY_TIMES) {
            *origin_data_len = 0;
            GF_LOGE(LOG_TAG "[%s] detect data fail", __func__);
            err = GF_ERROR_GENERIC;
            break;
        }

        data_len = (g_vfw_handle.nav_add_extra_data_rows * g_sensor.nav_col * 3) / 2;
        err = gf_milan_hv_read_bytes(MILAN_HV_REG_DATA_READ, origin_data, data_len + 4,
                g_vfw_handle.nav_fifo_spi_rate);
        if (err != GF_SUCCESS) {
            GF_LOGI(LOG_TAG "[%s] read nav image failed", __func__);
            break;
        }

        /* trigger next frame nav data */
        reg_data = 0x0110;
        gf_milan_hv_write_word_with_endian_exchange(MILAN_HV_REG_PIXEL_CTRL2, reg_data);

        *origin_data_len = data_len + 4;
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_vfw_set_mode(gf_mode_t mode) {
    gf_error_t err = GF_SUCCESS;
    uint32_t is_finger_touched = 0;

    FUNC_ENTER();

    g_vfw_handle.mode = mode;
    GF_LOGI(LOG_TAG "[%s] vfw_set_mode=%s", __func__, gf_strmode(mode));

    switch (mode) {
        case MODE_SLEEP: {
            err = vfw_set_state(STATE_SLEEP, 0);
            break;
        }

        case MODE_IDLE: {
            err = vfw_set_state(STATE_IDLE, 0);
            break;
        }

        case MODE_KEY: {
            err = vfw_set_state(STATE_FDT_DOWN, 0);
            break;
        }

        case MODE_NAV: {
            err = vfw_set_state(STATE_FDT_NAV_DOWN, 0);
            break;
        }

        case MODE_TEST_BAD_POINT:
        case MODE_IMAGE_CONTINUE:
        case MODE_IMAGE:
        case MODE_FF: {
            if ((MODE_FF == mode && 1 == g_config.reissue_key_down_when_entry_ff_mode)
                    || (MODE_FF != mode && 1 == g_config.reissue_key_down_when_entry_image_mode)) {
                err = vfw_set_state(STATE_FDT_DOWN, 0);
            } else {
                err = gf_vfw_check_finger_touched(&is_finger_touched);
                GF_ERROR_BREAK(err);

                GF_LOGI(LOG_TAG "[%s] is_finger_touched=%d, cur_chip_state=%d", __func__,
                        is_finger_touched, g_vfw_handle.cur_chip_state);

                if (0 == is_finger_touched) {
                    err = vfw_set_state(STATE_FDT_DOWN, 0);
                    GF_LOGI(LOG_TAG "[%s] finger is not touched, so detect finger down", __func__);
                } else {
                    err = vfw_set_state(STATE_FDT_UP, 0);
                    GF_LOGI(LOG_TAG "[%s] finger is touched, so detect finger up", __func__);
                }
            }

            break;
        }

        case MODE_DEBUG: {
            err = vfw_set_state(STATE_IMAGE, 1);
            break;
        }

        case MODE_FINGER_BASE: {
            err = vfw_set_state(STATE_IMAGE_FINGER_BASE, 1);
            break;
        }

        case MODE_TEST_BAD_POINT_FINGER_BASE: {
            err = vfw_set_state(STATE_IMAGE_TEST_BAD_POINT_FINGER_BASE, 1);
            break;
        }

        case MODE_NAV_BASE: {
            err = vfw_set_state(STATE_NAV_BASE, 1);
            break;
        }

        case MODE_BROKEN_CHECK_DEFAULT: {
            err = vfw_set_state(STATE_IMAGE_BROKEN_CHECK_DEFAULT, 1);
            break;
        }

        case MODE_BROKEN_CHECK_NEGATIVE: {
            err = vfw_set_state(STATE_IMAGE_BROKEN_CHECK_NEGATIVE, 1);
            break;
        }

        case MODE_BROKEN_CHECK_POSITIVE: {
            err = vfw_set_state(STATE_IMAGE_BROKEN_CHECK_POSITIVE, 1);
            break;
        }

        case MODE_TEST_PIXEL_OPEN_DEFAULT: {
            err = vfw_set_state(STATE_IMAGE_TEST_PIXEL_OPEN_DEFAULT, 1);
            break;
        }

        case MODE_TEST_PIXEL_OPEN_POSITIVE: {
            err = vfw_set_state(STATE_IMAGE_TEST_PIXEL_OPEN_POSITIVE, 1);
            break;
        }

        default: {
            GF_LOGE(LOG_TAG "[%s] mode=0x%04X not supported", __func__, mode);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
    }

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_vfw_get_mode(gf_mode_t *value) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    if (NULL == value) {
        err = GF_ERROR_BAD_PARAMS;
    } else {
        *value = g_vfw_handle.mode;
    }

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_vfw_get_irq_type(uint16_t *irq_type) {
    gf_error_t err = GF_SUCCESS;
    uint16_t reg_data = 0;

    FUNC_ENTER();

    do {
        if (NULL == irq_type) {
            GF_LOGE(LOG_TAG "[%s] invalid params", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        err = gf_milan_hv_read_word_with_endian_exchange(MILAN_HV_REG_IRQ_CTRL3, &reg_data);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] get irq type failed", __func__);
            break;
        }
        *irq_type = reg_data;
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_vfw_clear_irq(uint16_t irq_type) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();
    err = gf_milan_hv_write_word_with_endian_exchange(MILAN_HV_REG_IRQ_CTRL2, irq_type);
    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_vfw_data_sample_resume(void) {
    gf_error_t err = GF_SUCCESS;
    milan_hv_chip_state_t chip_mode = g_vfw_handle.cur_chip_state & CHIP_MODE_MASK;

    FUNC_ENTER();

    do {
        if ((chip_mode != CHIP_MODE_IMAGE) && (chip_mode != CHIP_MODE_NAV)) {
            GF_LOGI(LOG_TAG "[%s] nothing to do, chip_mode=%s", __func__,
                    gf_vfw_strstate(chip_mode));
            break;
        }

        // trigger a image when cur mode is image or nav
        err = gf_milan_hv_write_word_with_endian_exchange(MILAN_HV_REG_PIXEL_CTRL2, 0x0110);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_vfw_data_sample_suspend(void) {
    gf_error_t err = GF_SUCCESS;
    milan_hv_chip_state_t chip_mode = g_vfw_handle.cur_chip_state & CHIP_MODE_MASK;

    FUNC_ENTER();

    do {
        if ((chip_mode != CHIP_MODE_IMAGE) && (chip_mode != CHIP_MODE_NAV)) {
            GF_LOGI(LOG_TAG "[%s] nothing to do, chip_mode=%s", __func__,
                    gf_vfw_strstate(chip_mode));
            break;
        }

        switch (g_vfw_handle.mode) {
            case MODE_DEBUG:
            case MODE_FINGER_BASE:
            case MODE_NAV_BASE:
            case MODE_TEST_PIXEL_OPEN_DEFAULT:
            case MODE_TEST_PIXEL_OPEN_POSITIVE:
                // clear irq
                err = gf_milan_hv_write_word_with_endian_exchange(MILAN_HV_REG_IRQ_CTRL2, 0xFFFF);
                break;

            default:
                err = gf_vfw_switch_mode();
                break;
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_vfw_update_fdt_base(uint16_t *buf) {
    gf_error_t err = GF_SUCCESS;

    uint8_t i;
    uint8_t fdt_threshold;
    uint16_t offset;
    uint16_t fdt_thres_offset = MILAN_HV_REG_FDT_THRES - MILAN_HV_CONFIG_ADDRESS_START;

    FUNC_ENTER();

    do {
        if (NULL == buf) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad input params", __func__);
            break;
        }

        for (i = 0; i < MILAN_HV_FDT_BASE_LEN; i++) {
            fdt_threshold = buf[i] & 0xFF;
            offset = fdt_thres_offset + i * 2;

            GF_LOGD(LOG_TAG "[%s] fdt_threshold[%d]=0x%02X", __func__, i, fdt_threshold);

            g_vfw_handle.config->ff[offset] = fdt_threshold;
            g_vfw_handle.config->fdt_manual_down[offset] = fdt_threshold;
            g_vfw_handle.config->fdt_down[offset] = fdt_threshold;
            g_vfw_handle.config->fdt_up[offset] = fdt_threshold;
            g_vfw_handle.config->nav_fdt_down[offset] = fdt_threshold;
            g_vfw_handle.config->nav_fdt_up[offset] = fdt_threshold;
            g_vfw_handle.fdt_base[i] = fdt_threshold;
        }
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_vfw_read_fdt_data_with_trigger(uint16_t *fdt_data) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do {
        uint16_t reg_data = 0;
        uint32_t i = 0;
        uint32_t offset_bits = 1;

        if (NULL == fdt_data) {
            GF_LOGE(LOG_TAG "[%s] invalid params", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        err = vfw_set_state(STATE_FDT_MANUAL_DOWN, 0);
        GF_ERROR_BREAK(err);

        /* trigger fdt manual */
        err = gf_milan_hv_write_cmd(CMD_TMR_TRG);  // 0xA1
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] trigger fdt manual failed", __func__);
            break;
        }

        /* delay 6ms for 8 * 8 area, 3ms for 1 * 8 ms*/
        MILAN_HV_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(g_vfw_handle.config->fdt_manual_down,
                MILAN_HV_REG_PIXEL_CTRL4, reg_data);
        if (FDT_DETECT_AREA_IS_1X8(reg_data)) {
            offset_bits = 1;
            gf_sleep(3);
            GF_LOGD(LOG_TAG "[%s] FDT manual down detect area=1X8", __func__);
        } else {
            offset_bits = 4;
            gf_sleep(6);
            GF_LOGD(LOG_TAG "[%s] FDT manual down detect area=8X8", __func__);
        }

        /* switch to idle mode */
        err = vfw_set_state(STATE_IDLE, 0);
        GF_ERROR_BREAK(err);

        /* read FDT 24 bytes area average data  */
        err = gf_milan_hv_read_bytes_with_endian_exchange(MILAN_HV_REG_FDT_AVG, (uint8_t*) fdt_data,
        MILAN_HV_FDT_BASE_LEN * sizeof(uint16_t));  // MILAN_REG_FDT_AVG:0x00A2
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] read fdt data failed", __func__);
            break;
        }

        for (i = 0; i < MILAN_HV_FDT_BASE_LEN; ++i) {
            fdt_data[i] = (fdt_data[i] & 0xFFF) >> offset_bits;
            GF_LOGD(LOG_TAG "[%s] fdt_data[%d]=0x%02X", __func__, i, fdt_data[i]);
        }
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_vfw_disable_tx(void) {
    gf_error_t err = GF_SUCCESS;
    uint16_t val = 0;
    FUNC_ENTER();

    do {
        if (g_vfw_handle.current_tx_status == HV_TX_DISABLED) {
            break;
        }

        err = gf_milan_hv_read_word_with_endian_exchange(MILAN_HV_REG_RG_PIXEL_CTRL5, &val);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] read TX failed, addr=0x%02X", __func__,
                    MILAN_HV_REG_RG_PIXEL_CTRL5);
            break;
        }

        CLEAR_BIT(val, 8);
        err = gf_milan_hv_write_word_with_endian_exchange(MILAN_HV_REG_RG_PIXEL_CTRL5, val);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] write TX failed, addr=0x%02X", __func__,
                    MILAN_HV_REG_RG_PIXEL_CTRL5);
            break;
        }
        g_vfw_handle.current_tx_status = HV_TX_DISABLED;
        GF_LOGD(LOG_TAG "[%s] disable tx", __func__);
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_vfw_enable_tx(void) {
    gf_error_t err = GF_SUCCESS;
    uint16_t val = 0;
    FUNC_ENTER();

    do {
        if (g_vfw_handle.current_tx_status == HV_TX_ENABLED) {
            break;
        }

        err = gf_milan_hv_read_word_with_endian_exchange(MILAN_HV_REG_RG_PIXEL_CTRL5, &val);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] read TX failed, addr=0x%02X", __func__,
                    MILAN_HV_REG_RG_PIXEL_CTRL5);
            break;
        }

        SET_BIT(val, 8);
        err = gf_milan_hv_write_word_with_endian_exchange(MILAN_HV_REG_RG_PIXEL_CTRL5, val);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] write TX failed, addr=0x%02X", __func__,
                    MILAN_HV_REG_RG_PIXEL_CTRL5);
            break;
        }
        g_vfw_handle.current_tx_status = HV_TX_ENABLED;
        GF_LOGD(LOG_TAG "[%s] enable tx", __func__);
    } while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 * [gf_vfw_check_temperature_change : When down irq coming, call this function to check wheather
 * temperature have changed]
 */
gf_error_t gf_vfw_check_temperature_change(uint32_t *is_temperature_change) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do {
        uint8_t i = 0;
        uint16_t fdt_touch_flag = 0;
        uint16_t fdt_up_base_without_trigger[MILAN_HV_FDT_BASE_LEN];
        uint16_t fdt_up_base_with_trigger[MILAN_HV_FDT_BASE_LEN];
        milan_hv_chip_state_t chip_state = g_vfw_handle.cur_chip_state;
        milan_hv_tx_status_t store_tx_status = g_vfw_handle.current_tx_status;

        if (NULL == is_temperature_change) {
            GF_LOGE(LOG_TAG "[%s] invalid params", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        // Notes: when down irq coming, fdt data have alreday set to register
        // So if in fdt tx enable strategy, fdt data in register is alreday tx enable data
        // eles fdt data in register is alreday tx disable data
        err = vfw_read_fdt_data_without_trigger(fdt_up_base_without_trigger);
        GF_ERROR_BREAK(err);

        cpl_memcpy(g_fdt_base_buf, fdt_up_base_without_trigger, sizeof(g_fdt_base_buf));

        err = gf_milan_hv_read_word_with_endian_exchange(MILAN_HV_REG_TOUCH_FLAG0, &fdt_touch_flag);
        GF_ERROR_BREAK(err);
        GF_LOGD(LOG_TAG "[%s] fdt_touch_flag=0x%04X", __func__, fdt_touch_flag);

        if (HV_FDT_ENABLE_TX_STRATEGY == g_vfw_handle.fdt_tx_strategy) {
            err = gf_vfw_disable_tx();
        } else {
            err = gf_vfw_enable_tx();
        }
        GF_ERROR_BREAK(err);

        err = gf_vfw_read_fdt_data_with_trigger(fdt_up_base_with_trigger);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] read fdt base(disable_tx) failed", __func__);
            break;
        }

        // recovery tx status
        if (HV_TX_DISABLED == store_tx_status) {
            err = gf_vfw_disable_tx();
        } else {
            err = gf_vfw_enable_tx();
        }
        GF_ERROR_BREAK(err);

        for (i = 0; i < MILAN_HV_FDT_BASE_LEN; ++i) {
            if (cpl_abs((fdt_up_base_without_trigger[i] - fdt_up_base_with_trigger[i]))
                    >= g_vfw_handle.fdt_threshold) {
                break;
            }
        }

        if (MILAN_HV_FDT_BASE_LEN == i) {
            *is_temperature_change = 1;
        } else {
            *is_temperature_change = 0;
            vfw_update_fdt_up_base_into_config_array(fdt_up_base_without_trigger, fdt_touch_flag);
            g_vfw_handle.cur_chip_state = chip_state;
        }
    } while (0);

    FUNC_EXIT(err);
    return err;
}

/* When set mode, we should check whether finger is touched to add strategy of reissue down */
gf_error_t gf_vfw_check_finger_touched(uint32_t *is_finger_touched) {
    gf_error_t err = GF_SUCCESS;
    milan_hv_chip_state_t chip_state = g_vfw_handle.cur_chip_state;
    FUNC_ENTER();

    do {
        uint32_t i = 0;
        uint16_t fdt_touch_flag = 0;
        uint16_t fdt_up_base_1[MILAN_HV_FDT_BASE_LEN];
        uint16_t fdt_up_base_2[MILAN_HV_FDT_BASE_LEN];
        milan_hv_tx_status_t store_tx_status = g_vfw_handle.current_tx_status;

        GF_LOGD(LOG_TAG "[%s] before check finger touched, cur_chip_state=0x%08x, state_string=%s",
                __func__, chip_state, gf_vfw_strstate(chip_state));

        if (HV_FDT_ENABLE_TX_STRATEGY == g_vfw_handle.fdt_tx_strategy) {
            err = gf_vfw_enable_tx();
        } else {
            err = gf_vfw_disable_tx();
        }
        GF_ERROR_BREAK(err);

        err = gf_vfw_read_fdt_data_with_trigger(fdt_up_base_1);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] read fdt base failed", __func__);
            break;
        }

        err = gf_milan_hv_read_word_with_endian_exchange(MILAN_HV_REG_TOUCH_FLAG0, &fdt_touch_flag);
        GF_ERROR_BREAK(err);

        if (HV_FDT_ENABLE_TX_STRATEGY == g_vfw_handle.fdt_tx_strategy) {
            err = gf_vfw_disable_tx();
        } else {
            err = gf_vfw_enable_tx();
        }
        GF_ERROR_BREAK(err);

        err = gf_vfw_read_fdt_data_with_trigger(fdt_up_base_2);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] read fdt base(disable_tx) failed", __func__);
            break;
        }

        // recovery tx status
        if (HV_TX_DISABLED == store_tx_status) {
            err = gf_vfw_disable_tx();
        } else {
            err = gf_vfw_enable_tx();
        }
        GF_ERROR_BREAK(err);

        for (i = 0; i < MILAN_HV_FDT_BASE_LEN; ++i) {
            if (cpl_abs((fdt_up_base_1[i] - fdt_up_base_2[i]))
                    >= g_vfw_handle.fdt_threshold) {
                break;
            }
        }

        if (MILAN_HV_FDT_BASE_LEN == i) {
            *is_finger_touched = 0;
        } else {
            *is_finger_touched = 1;
            vfw_update_fdt_up_base_into_config_array(fdt_up_base_1, fdt_touch_flag);
        }
    } while (0);

    // after check finger touched, chip is in idle state, so need recovery chip state
    g_vfw_handle.cur_chip_state = chip_state;
    GF_LOGD(LOG_TAG "[%s] recovery chip state=%s", __func__, gf_vfw_strstate(chip_state));
    vfw_set_state(chip_state, 0);

    FUNC_EXIT(err);
    return err;
}

/*
 * clear saved the fdt down base.
*/
void gf_vfw_clear_saved_fdt_down_base(void) {
    VOID_FUNC_ENTER();

    cpl_memset(g_saved_fdt_down_base, 0, MILAN_HV_FDT_BASE_LEN * sizeof(uint16_t));
    g_is_fdt_down_base_saved = 0;
    GF_LOGD(LOG_TAG "[%s] clear saved fdt down base", __func__);

    VOID_FUNC_EXIT();
}

gf_error_t gf_vfw_check_temperature_change_on_up_irq(uint32_t *is_temperature_change) {
    gf_error_t err = GF_SUCCESS;
    uint16_t fdt_raw_data[MILAN_HV_FDT_BASE_LEN] = { 0 };
    uint16_t area_num = 0;
    uint16_t fdt_base_area_num = 0;

    FUNC_ENTER();

    do {
        if (NULL == is_temperature_change) {
            GF_LOGE(LOG_TAG "[%s] invalid parameters", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        *is_temperature_change = 0;

        err = vfw_read_fdt_data_without_trigger(fdt_raw_data);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] read fdt raw data failed", __func__);
            break;
        }

        if (0 == g_is_fdt_down_base_saved) {
            GF_LOGD(LOG_TAG "[%s] only update fdt down base[0]", __func__);
            vfw_update_fdt_down_base_into_config_array(fdt_raw_data);
            break;
        }

        /*
         * Caculate every area |fdt rawdata - fdt saved down base| > down_delta count.
        */
        MILAN_HV_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(g_vfw_handle.config->fdt_up,
                MILAN_HV_REG_FDT_AREA_NUM, fdt_base_area_num);

        err = vfw_get_fdt_diff_bigger_count(fdt_raw_data, g_saved_fdt_down_base,
                fdt_base_area_num, &area_num);
        GF_ERROR_BREAK(err);

        if (area_num > (fdt_base_area_num >> 1)) {
            GF_LOGI(LOG_TAG "[%s] check as temperature[1], will update all base", __func__);
            *is_temperature_change = 1;
            break;
        } else {
            err = vfw_get_fdt_diff_smaller_count(fdt_raw_data, g_saved_fdt_down_base,
                    fdt_base_area_num, &area_num);
            GF_ERROR_BREAK(err);

            if (fdt_base_area_num == area_num) {
                gf_vfw_clear_saved_fdt_down_base();
            }
            vfw_update_fdt_down_base_into_config_array(fdt_raw_data);
            GF_LOGD(LOG_TAG "[%s] only update fdt down base[1]", __func__);
        }
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_vfw_check_temperature_change_on_reverse_irq(uint32_t *is_temperature_change) {
    gf_error_t err = GF_SUCCESS;
    uint16_t *current_fdt_data = g_fdt_base_buf;
    uint16_t fdt_down_base[MILAN_HV_FDT_BASE_LEN] = {0};
    uint16_t fdt_base_area_num = 0;
    uint16_t area_num = 0;

    FUNC_ENTER();

    do {
        if (NULL == is_temperature_change) {
            GF_LOGE(LOG_TAG "[%s] invalid parameters", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        *is_temperature_change = 0;

        // get fdt base area num
        MILAN_HV_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(g_vfw_handle.config->fdt_up,
                MILAN_HV_REG_FDT_AREA_NUM, fdt_base_area_num);

        // get current fdt down base
        err = vfw_get_fdt_down_base_from_config(fdt_base_area_num, fdt_down_base);
        GF_ERROR_BREAK(err);

        // compare current fdt data and current fdt down base
        err = vfw_get_fdt_diff_bigger_count(current_fdt_data, fdt_down_base,
                fdt_base_area_num, &area_num);
        GF_ERROR_BREAK(err);

        if (area_num > (fdt_base_area_num >> 1)) {
            GF_LOGI(LOG_TAG "[%s] check as temperature[0], will update all base", __func__);
            *is_temperature_change = 1;
            break;
        }

        // no saved fdt down base
        if (0 == g_is_fdt_down_base_saved) {
            err = vfw_save_fdt_down_base(current_fdt_data, fdt_base_area_num);
            GF_ERROR_BREAK(err);

            vfw_update_fdt_down_base_into_config_array(current_fdt_data);
            break;
        }

        // fdt down base have been saved
        // compare current fdt data and saved fdt down base
        err = vfw_get_fdt_diff_bigger_count(current_fdt_data, g_saved_fdt_down_base,
                fdt_base_area_num, &area_num);
        GF_ERROR_BREAK(err);

        if (area_num > (fdt_base_area_num >> 1)) {
            GF_LOGI(LOG_TAG "[%s] check as temperature[1], will update all base", __func__);
            *is_temperature_change = 1;
            break;
        } else {
            // compare current fdt data and saved fdt down base
            err = vfw_get_fdt_diff_smaller_count(current_fdt_data, g_saved_fdt_down_base,
                    fdt_base_area_num, &area_num);
            GF_ERROR_BREAK(err);

            if (fdt_base_area_num == area_num) {
                gf_vfw_clear_saved_fdt_down_base();
            }
            GF_LOGD(LOG_TAG "[%s] update fdt down base.", __func__);
            vfw_update_fdt_down_base_into_config_array(current_fdt_data);
        }
    } while (0);

    if (GF_SUCCESS == err && *is_temperature_change != 1) {
        err = vfw_set_state(STATE_FDT_DOWN, 0);
    }

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_vfw_update_fdt_down_base_with_reverse(void) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        // update down base normally
        vfw_update_fdt_down_base_into_config_array(g_fdt_base_buf);

        // swith mode from idle back to fdt down
        err = gf_milan_hv_write_bytes(MILAN_HV_CONFIG_ADDRESS_START, g_vfw_handle.config->fdt_down,
                MILAN_HV_CONFIG_LENGTH);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] write configuration failed", __func__);
            break;
        }
        gf_sleep(3);

        err = gf_milan_hv_write_cmd(CMD_FDT);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] write CMD_FDT failed, cmd=0x%02X", __func__, CMD_FDT);
            break;
        }
        gf_sleep(1);

        g_vfw_handle.cur_chip_state = STATE_FDT_DOWN;
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_vfw_get_fdt_threshold(uint16_t *threshold) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == threshold) {
            GF_LOGE(LOG_TAG "[%s] invalid parameters", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        *threshold = g_vfw_handle.fdt_threshold;
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_vfw_get_nav_base_threshold(uint16_t *threshold) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do {
        if (NULL == threshold) {
            GF_LOGE(LOG_TAG "[%s] bad parameter", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
        /**
         * use formula : nav_base_delta = (otpTouchDiff + 5)*50 * 0.08;
         */
        *threshold = (((g_vfw_handle.otp_data[0x11] & 0x3E) >> 1) + 5) << 2;
    } while (0);

    FUNC_EXIT(err);
    return err;
}


gf_error_t gf_vfw_test_sensor_validity(void) {
    gf_error_t err = GF_SUCCESS;
    uint8_t otp_data[GF_SENSOR_OTP_INFO_LEN] = { 0 };

    FUNC_ENTER();
    err = vfw_check_otp_data(1, otp_data);
    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_vfw_generate_image_irq_by_manual(void) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint16_t reg_data = 0xFFFF;
        // clear all irq
        err = gf_milan_hv_write_word_with_endian_exchange(MILAN_HV_REG_IRQ_CTRL2, reg_data);
        if (err != GF_SUCCESS) {
            break;
        }

        // enable image irq bit
        reg_data = 0x0008;
        err = gf_milan_hv_write_word_with_endian_exchange(MILAN_HV_REG_IRQ_CTRL0, reg_data);
        if (err != GF_SUCCESS) {
            break;
        }

        // generate image irq
        err = gf_milan_hv_write_word_with_endian_exchange(MILAN_HV_REG_IRQ_CTRL4, reg_data);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

const char *gf_vfw_strstate(milan_hv_chip_state_t state) {
    switch (state) {
        case STATE_IDLE:
            return "STATE_IDLE";

        case STATE_SLEEP:
            return "STATE_SLEEP";

        case CHIP_MODE_FF:
            return "CHIP_MODE_FF";

        case CHIP_MODE_FDT:
            return "CHIP_MODE_FDT";

        case STATE_IMAGE:
            return "STATE_IMAGE";

        case STATE_NAV:
            return "STATE_NAV";

        case STATE_NAV_BASE:
            return "STATE_NAV_BASE";

        case STATE_FDT_DOWN:
            return "STATE_FDT_DOWN";

        case STATE_FDT_UP:
            return "STATE_FDT_UP";

        case STATE_FDT_NAV_DOWN:
            return "STATE_FDT_NAV_DOWN";

        case STATE_FDT_NAV_UP:
            return "STATE_FDT_NAV_UP";

        case STATE_FDT_MANUAL_DOWN:
            return "STATE_FDT_MANUAL_DOWN";

        case STATE_IMAGE_FINGER_BASE:
            return "STATE_IMAGE_FINGER_BASE";

        case STATE_IMAGE_CONTINUE:
            return "STATE_IMAGE_CONTINUE";

        case STATE_IMAGE_BROKEN_CHECK_DEFAULT:
            return "STATE_IMAGE_BROKEN_CHECK_DEFAULT";

        case STATE_IMAGE_BROKEN_CHECK_NEGATIVE:
            return "STATE_IMAGE_BROKEN_CHECK_NEGATIVE";

        case STATE_IMAGE_BROKEN_CHECK_POSITIVE:
            return "STATE_IMAGE_BROKEN_CHECK_POSITIVE";

        case STATE_IMAGE_TEST_BAD_POINT:
            return "STATE_IMAGE_TEST_BAD_POINT";

        case STATE_IMAGE_TEST_PIXEL_OPEN_DEFAULT:
            return "STATE_IMAGE_TEST_PIXEL_OPEN_DEFAULT";

        case STATE_IMAGE_TEST_PIXEL_OPEN_POSITIVE:
            return "STATE_IMAGE_TEST_PIXEL_OPEN_POSITIVE";

        case STATE_IMAGE_TEST_BAD_POINT_FINGER_BASE:
            return "STATE_IMAGE_TEST_BAD_POINT_FINGER_BASE";

        default:
            GF_LOGE(LOG_TAG "[%s] undefined chip state=0x%x", __func__, state);
            return "undefined chip state";
    }
}

/* For hw to get g_vfw_handle variables */
uint32_t gf_vfw_get_chip_id(void) {
    if (0 == g_vfw_handle.init_complete_flag) {
        gf_milan_hv_read_bytes_with_endian_exchange(MILAN_HV_REG_CHIP_ID,
                (uint8_t *) &(g_vfw_handle.chip_id), 4);
    }

    return g_vfw_handle.chip_id;
}

uint8_t gf_vfw_get_product_id(void) {
    return g_vfw_handle.otp_info.product_id;
}

uint32_t gf_vfw_get_image_double_rate_flag(void) {
    return g_vfw_handle.image_double_rate_flag;
}

uint32_t gf_vfw_get_nav_double_rate_flag(void) {
    return g_vfw_handle.nav_double_rate_flag;
}

milan_hv_scan_mode_t gf_vfw_get_scan_mode(void) {
    return g_vfw_handle.scan_mode;
}

void gf_vfw_set_general_reg(void) {
    g_vfw_handle.function.set_general_reg();
}

milan_hv_chip_state_t gf_vfw_get_cur_chip_state(void) {
    return g_vfw_handle.cur_chip_state;
}

milan_hv_chip_state_t gf_vfw_get_cur_chip_mode(void) {
    return (g_vfw_handle.cur_chip_state & CHIP_MODE_MASK);
}

gf_mode_t gf_vfw_get_cur_mode(void) {
    return g_vfw_handle.mode;
}

void gf_vfw_set_dac_h(uint16_t dac_h) {
    VOID_FUNC_ENTER();
    g_vfw_handle.cur_dac_h = dac_h;
    GF_LOGI(LOG_TAG "[%s] update dac_h=0x%04x", __func__, dac_h);
    VOID_FUNC_EXIT();
}

uint16_t gf_vfw_get_orientation(void) {
    return g_vfw_handle.vendor_param.orientation;
}

uint16_t gf_vfw_get_cur_facing(void) {
    return g_vfw_handle.vendor_param.facing;
}

gf_error_t gf_vfw_get_otp_info(milan_hv_otp_info_t *info) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do {
        if (NULL == info) {
            GF_LOGE(LOG_TAG "[%s] invalid params", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        cpl_memcpy(info, &g_vfw_handle.otp_info, sizeof(milan_hv_otp_info_t));
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_vfw_get_otp_data(uint8_t *otp_data) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do {
        if (NULL == otp_data) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] invalid params", __func__);
            break;
        }

        cpl_memcpy(otp_data, g_vfw_handle.otp_data, sizeof(g_vfw_handle.otp_data));
    } while (0);

    FUNC_EXIT(err);
    return err;
}

uint8_t gf_vfw_is_enable_fdt_tx_strategy(void) {
    return ((HV_FDT_ENABLE_TX_STRATEGY == g_vfw_handle.fdt_tx_strategy) ? 1 : 0);
}

uint8_t gf_vfw_is_enable_nav_tx_strategy(void) {
    return ((HV_NAV_ENABLE_TX_STRATEGY == g_vfw_handle.nav_tx_strategy) ? 1 : 0);
}

/********************* External Function Definition End *********************/
