/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#include <string.h>

#include "cpl_string.h"
#include "cpl_memory.h"

#include "gf_utils.h"
#include "gf_tee_internal_api.h"
#include "gf_common.h"
#include "gf_error.h"
#include "gf_sensor.h"
#include "gf_spi.h"
#include "gf_hw_common.h"
#include "gf_fdt.h"
#include "gf_secure_object.h"
#include "gf_algo.h"
#include "gf_base.h"
#include "gf_config.h"
#include "gf_nav.h"

#include "gf_lfsr_prbs.h"

#include "gf_milan.h"

#include "milan_bus.h"
#include "milan_crc.h"
#include "milan_utils.h"
#include "milan_regs.h"
#include "milan_common.h"
#include "milan_otp_check.h"

#include "milan_e_config.h"
#include "milan_f_config.h"
#include "milan_g_config.h"
#include "milan_l_config.h"
#include "milan_fn_config.h"
#include "milan_k_config.h"
#include "milan_j_config.h"
#include "milan_h_config.h"

#include "gf_milan_platform_chip_params.h"

#define LOG_TAG "[gf_milan]"
#define OTP_CHECK_FLOW_VERSION "v1.1"
#define MILAN_CHIP_FLOW_VERSION "v1.0.8"

#define MILAN_OTP_BANK_NUM   8
#define MILAN_OTP_BANK_LEN   4
#define MILAN_OTP_LEN_BYTES  32

#define MILAN_F_SENSOR_PIXEL_TEST_DAC_UPDATE_VALUE      (20)
#define MILAN_E_SENSOR_PIXEL_TEST_DAC_UPDATE_VALUE      (20)
#define MILAN_G_SENSOR_PIXEL_TEST_DAC_UPDATE_VALUE      (20)
#define MILAN_L_SENSOR_PIXEL_TEST_DAC_UPDATE_VALUE      (20)
#define MILAN_FN_SENSOR_PIXEL_TEST_DAC_UPDATE_VALUE     (20)
#define MILAN_K_SENSOR_PIXEL_TEST_DAC_UPDATE_VALUE      (15)
#define MILAN_H_SENSOR_PIXEL_TEST_DAC_UPDATE_VALUE      (15)
#define MILAN_J_SENSOR_PIXEL_TEST_DAC_UPDATE_VALUE      (20)

#define MILAN_READ_OTP_RETRY_TIMES   (5)
#define MILAN_FDT_BASE_DATA_LEN  12

#define MILAN_READ_NAV_DATA_MAX_RETRY_TIMES  1000

static gf_error_t milan_read_fdt_base(uint16_t *fdt_base);
static gf_error_t milan_update_fdt_down_base(void);
static gf_error_t milan_update_fdt_up_base(int32_t second_update_flag);
static void milan_update_config_arrays(uint16_t register_addr, uint16_t value);

static void milan_update_scan_mode_into_config_array(milan_chip_handle_t* handle);

static gf_error_t milan_check_finger_up(void);
static gf_error_t milan_get_fifo_spi_rates(uint32_t image_tcode, uint32_t nav_tcode);
static gf_error_t milan_init_fdt_params_from_chip_config(gf_fdt_params_t* init_param);

static milan_chip_handle_t g_milan_chip_handle = { 0 };

static uint16_t g_fdt_base_data[MILAN_FDT_BASE_DATA_LEN];
static uint8_t* g_milan_image_rawdata = NULL;
static uint8_t* g_milan_nav_rawdata = NULL;
static uint32_t g_milan_valid_nav_rawdata_len = 0;

static uint32_t g_image_read_len = 0;

static uint32_t g_read_frame_num_for_nav_in_continue = 5;

/**
 * [gf_hw_init_global_variable]:Initialize the global variable in this file
 */
void gf_hw_init_global_variable(void) {
    VOID_FUNC_ENTER();

    cpl_memset(&g_milan_chip_handle, 0, sizeof(milan_chip_handle_t));
    g_milan_valid_nav_rawdata_len = 0;
    g_image_read_len = 0;

    gf_milan_bus_init_global_variable();

    VOID_FUNC_EXIT();
}

static uint16_t milan_get_irq_bit_mask(uint32_t irq_type) {
    uint16_t bit_mask = 0;

    if (irq_type & MILAN_TMR_MNT_IRQ) {
        bit_mask |= TMR_IRQ_MNT_MASK;
    }

    if (irq_type & MILAN_FDT_IRQ) {
        bit_mask |= FDT_IRQ_MASK;
    }

    if (irq_type & MILAN_ONE_FRAME_DONE_IRQ) {
        bit_mask |= ONE_FRAME_DONE_MASK;
    }

    if (irq_type & MILAN_DATA_INT_IRQ) {
        bit_mask |= DATA_INT_MASK;
    }

    if (irq_type & MILAN_ESD_INT_IRQ) {
        bit_mask |= ESD_INT_MASK;
    }

    if (irq_type & MILAN_FIFO_EMPTY_OR_FULL_IN_ADC_TEST_MODE_IRQ) {
        bit_mask |= FIFO_EMPTY_OR_FULL_IN_ADC_TEST_MODE_MASK;
    }

    if (irq_type & MILAN_FIFO_DATA_HALF_FULL_IN_ADC_TEST_MODE_IRQ) {
        bit_mask |= FIFO_DATA_HALF_FULL_IN_ADC_TEST_MODE_MASK;
    }

    if (irq_type & MILAN_REVERSE_IRQ) {
        bit_mask |= FDT_IRQ1_MASK;
    }

    if (irq_type & MILAN_FDT_MANUAL_IRQ) {
        bit_mask |= FDT_MANUAL_IRQ_MASK;
    }

    if (irq_type & MILAN_FDT_UP_IRQ) {
        if (MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_F_CHIP_ID)) {
            bit_mask |= FDT_IRQ_MASK;
        } else {
            bit_mask |= FDT_IRQ_UP_MASK;
        }
    }

    if (irq_type & MILAN_RESET_IRQ) {
        if (MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_F_CHIP_ID)) {
            bit_mask |= MILAN_F_RESET_IRQ_MASK;
        } else if (MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_K_CHIP_ID) ||
                   MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_J_CHIP_ID) ||
                   MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_H_CHIP_ID)) {
            bit_mask |= MILAN_KJH_RESET_IRQ_MASK;
        } else {
            bit_mask |= MILAN_EGL_RESET_IRQ_MASK;
        }
    }

    if (irq_type & MILAN_TEMPERATURET_IRQ) {
        if (MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_K_CHIP_ID) ||
            MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_J_CHIP_ID) ||
            MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_H_CHIP_ID)) {
            bit_mask |= MILAN_KJH_TEMPERATURE_IRQ_MASK;
        }
    }

    return bit_mask;
}

static void milan_fdt_up_detect_mode_init(gf_mode_t mode) {
    VOID_FUNC_ENTER();
    GF_LOGD(LOG_TAG "[%s] mode=%s, mode_code=%d", __func__, gf_strmode(mode), mode);

    do {
        g_milan_chip_handle.fdt_up_detect_new_mode_flag = FDT_UP_DETECT_MODE_OLD_VERSION;

        if (MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_K_CHIP_ID)) {
            uint16_t reg_data = 0;
            if (MODE_NAV == mode) {
                MILAN_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->nav_fdt_up,
                        MILAN_REG_PIXEL_CTRL4, reg_data);
            } else {
                MILAN_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->fdt_up,
                        MILAN_REG_PIXEL_CTRL4, reg_data);
            }

            if (((reg_data >> 9) & 0x1) == 0) {
                g_milan_chip_handle.fdt_up_detect_new_mode_flag = FDT_UP_DETECT_MODE_NEW_VERSION;
            }

        } else {
            GF_LOGD(LOG_TAG "[%s] don't support fdt up detect mode new version", __func__);
        }
    } while (0);

    GF_LOGD(LOG_TAG "[%s] fdt up detect mode=%d", __func__,
            g_milan_chip_handle.fdt_up_detect_new_mode_flag);
    VOID_FUNC_EXIT();
}

static void milan_init_chip_handle(void) {
    cpl_memset(&g_milan_chip_handle, 0, sizeof(milan_chip_handle_t));

    g_milan_chip_handle.vendor_id = VENDOR_ID_A;
    g_milan_chip_handle.mode = MODE_IDLE;
    g_milan_chip_handle.cur_chip_state = MILAN_STATE_IDLE;
    g_milan_chip_handle.next_chip_state = MILAN_STATE_IDLE;
    g_milan_chip_handle.chip_state = MODE_IDLE_STATE_MASK;
    g_milan_chip_handle.img_fifo_spi_rate = 9;
    g_milan_chip_handle.nav_fifo_spi_rate = 9;
    g_milan_chip_handle.is_report_finger_up_for_ff_or_image_mode = 1;
    g_milan_chip_handle.is_received_fdt_up_for_ff_or_image_mode = 0;
}

static milan_scan_mode_t milan_e_get_scan_mode(void) {
    milan_scan_mode_t scan_mode = SCAN_MODE0;

    VOID_FUNC_ENTER();

    switch (g_milan_chip_handle.chip_id & 0xFF) {
        case SENSOR_ECO_TA1: {
            scan_mode = SCAN_MODE0;
            break;
        }

        case SENSOR_ECO_TA3: {
            scan_mode = SCAN_MODE0;
            break;
        }

        default: {
            scan_mode = SCAN_MODE0;
            GF_LOGE(LOG_TAG "[%s] invalid ECO version, adapt the default scan mode 0, "
                    "chip_id=0x%08X", __func__, g_milan_chip_handle.chip_id);
            break;
        }
    }

    GF_LOGD(LOG_TAG "[%s] chip_id=0x%08X, scan_mode=%d",
            __func__, g_milan_chip_handle.chip_id, scan_mode);
    VOID_FUNC_EXIT();
    return scan_mode;
}

static milan_scan_mode_t milan_g_get_scan_mode(void) {
    milan_scan_mode_t scan_mode = SCAN_MODE0;

    VOID_FUNC_ENTER();

    switch (g_milan_chip_handle.chip_id & 0xFF) {
        case SENSOR_ECO_TA1: {
            scan_mode = SCAN_MODE0;
            break;
        }

        case SENSOR_ECO_TA3: {
            scan_mode = SCAN_MODE0;
            break;
        }

        default: {
            scan_mode = SCAN_MODE0;
            GF_LOGE(LOG_TAG "[%s] invalid ECO version, adapt the default scan mode 0, "
                    "chip_id=0x%08X", __func__, g_milan_chip_handle.chip_id);
            break;
        }
    }

    GF_LOGD(LOG_TAG "[%s] chip_id=%08X, scan_mode=%d",
            __func__, g_milan_chip_handle.chip_id, scan_mode);
    VOID_FUNC_EXIT();
    return scan_mode;
}

static uint8_t milan_get_key_state(const uint16_t* cur_frame, const uint16_t* base,
        uint32_t pixel_num, int32_t touch_threshold) {
    uint8_t result = 0;
    uint32_t i = 0;
    uint32_t count = 0;

    VOID_FUNC_ENTER();

    do {
        if ((NULL == base) || (NULL == cur_frame)) {
            GF_LOGD(LOG_TAG "[%s] no data", __func__);
            result = 1;
            break;
        }

        for (i = 0; i < pixel_num; i ++) {
            if ((int32_t)base[i] - (int32_t)cur_frame[i] >= touch_threshold) {
                count++;

                if (count >= g_sensor.nav_finger_up_pixel_num) {
                    result = 1;
                    break;
                }
            }
        }
    } while (0);

    VOID_FUNC_EXIT();

    return result;
}

static void milan_update_scan_mode_into_config_array(milan_chip_handle_t* handle) {
    uint16_t reg_data = 0;
    VOID_FUNC_ENTER();
    // update scan mode register to MILAN_REG_PIXEL_CTRL10, for IMAGE configuration
    MILAN_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(handle->config->image, MILAN_REG_PIXEL_CTRL10,
            reg_data);
    reg_data = (reg_data & 0xFFF9) | (handle->scan_mode << 1);
    MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(handle->config->image, MILAN_REG_PIXEL_CTRL10, reg_data);
    VOID_FUNC_EXIT();
}

static gf_error_t milan_init_default_dac(milan_chip_handle_t *handle) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint16_t reg_data = 0;
        uint16_t tx_code = 0;
        uint16_t sensor_broken_dac_delta = 0;
        uint16_t pixel_test_dac_delta = 0;
        milan_reg_access_t reg = { 0 };
        uint32_t dac_groups_num = MILAN_DAC_GROUPS_MAX_NUM;

        // read tx clock number
        MILAN_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(handle->config->image, MILAN_REG_PIXEL_CTRL6,
                reg_data);
        tx_code = reg_data & 0x1FFF;
        GF_LOGD(LOG_TAG "[%s] tx_code=%u", __func__, tx_code);
        if (0 == tx_code) {
            GF_LOGE(LOG_TAG "[%s] tx_code shouldn't be 0", __func__);
            err = GF_ERROR_GENERIC;
            break;
        }
        sensor_broken_dac_delta = (256 << 5) / tx_code;
        GF_LOGD(LOG_TAG "[%s] sensor_broken_dac_delta=%u", __func__, sensor_broken_dac_delta);

        // read dac_group[0]: reg[0220]
        reg_data = 0;
        MILAN_MK_REG_READ(reg, MILAN_REG_RG_PIXEL_CTRL0, &reg_data);
        err = gf_milan_reg_access(&reg);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] read dac group[0]", __func__);
            break;
        }
        g_milan_chip_handle.default_dac[0] = ((reg_data >> 4) & 0x01FF);

        if (MILAN_CHIP_ID_IS_EQUAL(handle->chip_id, MILAN_K_CHIP_ID) ||
                MILAN_CHIP_ID_IS_EQUAL(handle->chip_id, MILAN_H_CHIP_ID) ||
                MILAN_CHIP_ID_IS_EQUAL(handle->chip_id, MILAN_J_CHIP_ID)) {
            pixel_test_dac_delta = (256 * handle->pixel_test_dac_update_value) / tx_code;
            GF_LOGD(LOG_TAG "[%s] pixel_test_dac_delta=%u", __func__, pixel_test_dac_delta);

            // read dac_group[1]: reg[0236]
            reg_data = 0;
            MILAN_MK_REG_READ(reg, MILAN_REG_RG_PIXEL_DAC_P2, &reg_data);
            err = gf_milan_reg_access(&reg);
            if (err != GF_SUCCESS) {
                GF_LOGE(LOG_TAG "[%s] read dac group[1]", __func__);
                break;
            }
            g_milan_chip_handle.default_dac[1] = (reg_data & 0x01FF);

            // read dac_group[2]: reg[0238]
            reg_data = 0;
            MILAN_MK_REG_READ(reg, MILAN_REG_RG_PIXEL_DAC_P3, &reg_data);
            err = gf_milan_reg_access(&reg);
            if (err != GF_SUCCESS) {
                GF_LOGE(LOG_TAG "[%s] read dac group[2]", __func__);
                break;
            }
            g_milan_chip_handle.default_dac[2] = (reg_data & 0x01FF);

            // read dac_group[3]: reg[023a]
            reg_data = 0;
            MILAN_MK_REG_READ(reg, MILAN_REG_RG_PIXEL_DAC_P4, &reg_data);
            err = gf_milan_reg_access(&reg);
            if (err != GF_SUCCESS) {
                GF_LOGE(LOG_TAG "[%s] read dac group[3]", __func__);
                break;
            }
            g_milan_chip_handle.default_dac[3] = (reg_data & 0x01FF);
        } else {
            pixel_test_dac_delta = (256 * handle->pixel_test_dac_update_value) / tx_code;
            GF_LOGD(LOG_TAG "[%s] pixel_test_dac_delta=%u", __func__, pixel_test_dac_delta);
            dac_groups_num = 1;
        }

        {
            uint32_t i = 0;

            for (; i < dac_groups_num; i++) {
                GF_LOGD(LOG_TAG "[%s] default_dac[%u]=0x%04X", __func__, i,
                        g_milan_chip_handle.default_dac[i]);

                g_milan_chip_handle.sensor_broken.positive_dac[i] =
                        ((g_milan_chip_handle.default_dac[i] + sensor_broken_dac_delta) & 0x1FF);
                GF_LOGD(LOG_TAG "[%s] sensor_broke.positive_dac[%u]=0x%04X", __func__, i,
                        g_milan_chip_handle.sensor_broken.positive_dac[i]);

                g_milan_chip_handle.sensor_broken.negative_dac[i] =
                        g_milan_chip_handle.default_dac[i] - sensor_broken_dac_delta;
                GF_LOGD(LOG_TAG "[%s] sensor_broke.negative_dac[%u]=0x%04X", __func__, i,
                        g_milan_chip_handle.sensor_broken.negative_dac[i]);

                g_milan_chip_handle.pixel_test_chip_info.positive_dac[i] =
                        ((g_milan_chip_handle.default_dac[i] + pixel_test_dac_delta) & 0x1FF);
                GF_LOGD(LOG_TAG "[%s] pixel_test_chip_info.positive_dac[%u]=0x%04X", __func__, i,
                        g_milan_chip_handle.pixel_test_chip_info.positive_dac[i]);
            }
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static void milan_update_config_array_by_word(milan_config_t *config,
        uint32_t register_addr, uint32_t value) {
    MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(config->ff, register_addr, value);
    MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(config->fdt_manual_down, register_addr, value);
    MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(config->fdt_down, register_addr, value);
    MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(config->fdt_up, register_addr, value);
    MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(config->image, register_addr, value);
    MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(config->nav_fdt_down, register_addr, value);
    MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(config->nav_fdt_up, register_addr, value);
    MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(config->nav, register_addr, value);
    MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(config->nav_base, register_addr, value);
}

static gf_error_t milan_common_update_dac_register_from_opt(uint8_t *otp_buf) {
    gf_error_t err = GF_SUCCESS;
    uint16_t reg_data = 0;

    FUNC_ENTER();
    do {
        // [0x130]<15>  = otp[0x12]<7>
        MILAN_READ_REGISTER(MILAN_REG_IO_CTRL0, &reg_data);
        GF_LOGI(LOG_TAG "[%s] REG[0130]:%04X", __func__, reg_data);
        reg_data = (reg_data & 0x7FFF) | (((uint16_t) otp_buf[0x12] << 8) & 0x8000);
        GF_LOGI(LOG_TAG "[%s] new REG[0130]:%04X", __func__, reg_data);
        MILAN_WRITE_REGISTER(MILAN_REG_IO_CTRL0, &reg_data);

        // [0x0140]<0> = otp[0x13]<6>
        reg_data = 0;
        MILAN_READ_REGISTER(MILAN_REG_SPI_CTRL0, &reg_data);
        GF_LOGI(LOG_TAG "[%s] REG[0140]:%04X", __func__, reg_data);
        reg_data = (reg_data & 0xFFFE) | (otp_buf[0x13] >> 6);
        GF_LOGI(LOG_TAG "[%s] new REG[0140]:%04X", __func__, reg_data);
        MILAN_WRITE_REGISTER(MILAN_REG_SPI_CTRL0, &reg_data);

        // [0x0200]<5:0> = otp[12]<6:0>
        // [0x0200]<13:8> = otp[13]<5:0>
        reg_data = 0;
        MILAN_READ_REGISTER(MILAN_REG_OSC_CTRL, &reg_data);
        GF_LOGI(LOG_TAG "[%s] REG[0200]:%04X", __func__, reg_data);
        reg_data = (reg_data & 0xC0C0) | ((uint16_t)(otp_buf[0x12] & 0x3F)
                | (uint16_t)(otp_buf[0x13] & 0x3F) << 8);
        GF_LOGI(LOG_TAG "[%s] new REG[0200]:%04X", __func__, reg_data);
        MILAN_WRITE_REGISTER(MILAN_REG_OSC_CTRL, &reg_data);

        // [0x0220]<12:4> = otp[13]<7><<8 + otp[1f]<7:0>
        reg_data = 0;
        MILAN_READ_REGISTER(MILAN_REG_RG_PIXEL_CTRL0, &reg_data);
        GF_LOGI(LOG_TAG "[%s] REG[0220]:%04X", __func__, reg_data);
        reg_data = (reg_data & 0xE00F)
                | (((uint16_t) (otp_buf[0x13] & 0x80) << 5) | (uint16_t) otp_buf[0x1F] << 4);
        GF_LOGI(LOG_TAG "[%s] new REG[0220]:%04X", __func__, reg_data);
        MILAN_WRITE_REGISTER(MILAN_REG_RG_PIXEL_CTRL0, &reg_data);
    } while (0);

    FUNC_EXIT(err);
    return err;
}

static gf_error_t milan_common_update_config_array_from_opt(uint8_t *otp_buf) {
    gf_error_t err = GF_SUCCESS;
    UNUSED_VAR(otp_buf);
    return err;
}

static gf_error_t milan_f_update_dac_register_from_opt(uint8_t *otp_buf) {
    gf_error_t err = GF_SUCCESS;
    UNUSED_VAR(otp_buf);
    return err;
}

static gf_error_t milan_f_update_config_array_from_opt(uint8_t *otp_buf) {
    gf_error_t err = GF_SUCCESS;
    uint16_t reg_data = 0;

    FUNC_ENTER();

    do {
        // [0x130]<15>  = otp[0x12]<7>
        MILAN_READ_REGISTER(MILAN_REG_IO_CTRL0, &reg_data);
        GF_LOGI(LOG_TAG "[%s] REG[0130]:%04X", __func__, reg_data);
        reg_data = (reg_data & 0x7FFF) | (((uint16_t) otp_buf[0x12] << 8) & 0x8000);
        GF_LOGI(LOG_TAG "[%s] new REG[0220]:%04X", __func__, reg_data);
        milan_update_config_array_by_word(g_milan_chip_handle.config, MILAN_REG_IO_CTRL0, reg_data);

        // [0x0140]<0> = otp[0x13]<6>
        reg_data = 0;
        MILAN_READ_REGISTER(MILAN_REG_SPI_CTRL0, &reg_data);
        GF_LOGI(LOG_TAG "[%s] REG[0140]:%04X", __func__, reg_data);
        reg_data = (reg_data & 0xFFFE) | (otp_buf[0x13] >> 6);
        GF_LOGI(LOG_TAG "[%s] new REG[0140]:%04X", __func__, reg_data);
        milan_update_config_array_by_word(g_milan_chip_handle.config, MILAN_REG_SPI_CTRL0,
                reg_data);

        // [0x0200]<5:0> = otp[12]<6:0>?
        reg_data = 0;
        MILAN_READ_REGISTER(MILAN_REG_OSC_CTRL, &reg_data);
        GF_LOGI(LOG_TAG "[%s] REG[0200]:%04X", __func__, reg_data);
        reg_data = (reg_data & 0xC0C0)
                | ((uint16_t) (otp_buf[0x12] & 0x3F) | (uint16_t) (otp_buf[0x13] & 0x3F) << 8);
        GF_LOGI(LOG_TAG "[%s] REG[0200]:%04X", __func__, reg_data);
        milan_update_config_array_by_word(g_milan_chip_handle.config, MILAN_REG_OSC_CTRL, reg_data);

        // [0x0220]<12:4> = otp[13]<7><<8 + otp[1f]<7:0>
        reg_data = 0;
        MILAN_READ_REGISTER(MILAN_REG_RG_PIXEL_CTRL0, &reg_data);
        GF_LOGI(LOG_TAG "[%s] REG[0220]:%04X", __func__, reg_data);
        reg_data = (reg_data & 0xE00F)
                | (((uint16_t) (otp_buf[0x13] & 0x80) << 5) | (uint16_t) otp_buf[0x1F] << 4);
        GF_LOGI(LOG_TAG "[%s] REG[0220]:%04X", __func__, reg_data);
        milan_update_config_array_by_word(g_milan_chip_handle.config, MILAN_REG_RG_PIXEL_CTRL0,
                reg_data);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t milan_init_configuration(milan_chip_handle_t *handle,
        const milan_vendor_params_t** vendor_param) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint16_t reg_data = 0;

        if (NULL == handle || NULL == vendor_param) {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        switch (handle->chip_id >> 8) {
            case MILAN_E_CHIP_ID: {
                handle->config = &g_milan_e_config;
                *vendor_param = g_milan_e_vendor_params;
                handle->milan_raw_data_split = milan_e_raw_data_split;
                handle->milan_update_dac_register_from_opt =
                        milan_common_update_dac_register_from_opt;
                handle->milan_update_config_array_from_opt =
                        milan_common_update_config_array_from_opt;
                handle->scan_mode = milan_e_get_scan_mode();
                handle->pixel_test_dac_update_value = MILAN_E_SENSOR_PIXEL_TEST_DAC_UPDATE_VALUE;
                handle->milan_calculate_otp_crc = milan_efglfn_calculate_otp_crc;
                handle->milan_check_ft_test_info = milan_efglfn_check_ft_test_info;
                break;
            }

            case MILAN_F_CHIP_ID: {
                handle->config = &g_milan_f_config;
                *vendor_param = g_milan_f_vendor_params;
                handle->milan_raw_data_split = milan_f_raw_data_split;
                handle->milan_update_dac_register_from_opt = milan_f_update_dac_register_from_opt;
                handle->milan_update_config_array_from_opt = milan_f_update_config_array_from_opt;
                handle->pixel_test_dac_update_value = MILAN_F_SENSOR_PIXEL_TEST_DAC_UPDATE_VALUE;
                handle->milan_calculate_otp_crc = milan_efglfn_calculate_otp_crc;
                handle->milan_check_ft_test_info = milan_efglfn_check_ft_test_info;
                break;
            }

            case MILAN_G_CHIP_ID: {
                handle->config = &g_milan_g_config;
                *vendor_param = g_milan_g_vendor_params;
                handle->milan_raw_data_split = milan_g_raw_data_split;
                handle->milan_update_dac_register_from_opt =
                        milan_common_update_dac_register_from_opt;
                handle->milan_update_config_array_from_opt =
                        milan_common_update_config_array_from_opt;
                handle->scan_mode = milan_g_get_scan_mode();
                handle->pixel_test_dac_update_value = MILAN_G_SENSOR_PIXEL_TEST_DAC_UPDATE_VALUE;
                handle->milan_calculate_otp_crc = milan_efglfn_calculate_otp_crc;
                handle->milan_check_ft_test_info = milan_efglfn_check_ft_test_info;
                break;
            }

            case MILAN_L_CHIP_ID: {
                handle->config = &g_milan_l_config;
                *vendor_param = g_milan_l_vendor_params;
                handle->milan_raw_data_split = milan_l_raw_data_split;
                handle->milan_update_dac_register_from_opt =
                        milan_common_update_dac_register_from_opt;
                handle->milan_update_config_array_from_opt =
                        milan_common_update_config_array_from_opt;
                handle->pixel_test_dac_update_value = MILAN_L_SENSOR_PIXEL_TEST_DAC_UPDATE_VALUE;
                handle->milan_calculate_otp_crc = milan_efglfn_calculate_otp_crc;
                handle->milan_check_ft_test_info = milan_efglfn_check_ft_test_info;
                break;
            }

            case MILAN_FN_CHIP_ID: {
                handle->config = &g_milan_fn_config;
                *vendor_param = g_milan_fn_vendor_params;
                handle->milan_raw_data_split = milan_fn_raw_data_split;
                handle->milan_update_dac_register_from_opt =
                        milan_common_update_dac_register_from_opt;
                handle->milan_update_config_array_from_opt =
                        milan_common_update_config_array_from_opt;
                handle->pixel_test_dac_update_value = MILAN_FN_SENSOR_PIXEL_TEST_DAC_UPDATE_VALUE;
                handle->milan_calculate_otp_crc = milan_efglfn_calculate_otp_crc;
                handle->milan_check_ft_test_info = milan_efglfn_check_ft_test_info;
                break;
            }

            case MILAN_K_CHIP_ID: {
                handle->config = &g_milan_k_config;
                *vendor_param = g_milan_k_vendor_params;
                handle->vendor_id = VENDOR_ID_A;
                handle->milan_raw_data_split = milan_k_raw_data_split;
                handle->milan_update_dac_register_from_opt =
                        milan_common_update_dac_register_from_opt;
                handle->milan_update_config_array_from_opt =
                        milan_common_update_config_array_from_opt;
                MILAN_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(handle->config->image,
                        MILAN_REG_PIXEL_CTRL10, reg_data);
                handle->scan_mode = (reg_data >> 1) & RG_SCAN_MODE_MASK;

                handle->pixel_test_dac_update_value = MILAN_K_SENSOR_PIXEL_TEST_DAC_UPDATE_VALUE;
                handle->milan_calculate_otp_crc = milan_kjh_calculate_otp_crc;
                handle->milan_check_ft_test_info = milan_kjh_check_ft_test_info;
                break;
            }

            case MILAN_J_CHIP_ID: {
                handle->config = &g_milan_j_config;
                *vendor_param = g_milan_j_vendor_params;
                handle->vendor_id = VENDOR_ID_A;
                handle->milan_raw_data_split = milan_j_raw_data_split;
                handle->milan_update_dac_register_from_opt =
                        milan_common_update_dac_register_from_opt;
                handle->milan_update_config_array_from_opt =
                        milan_common_update_config_array_from_opt;
                MILAN_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(handle->config->image,
                        MILAN_REG_PIXEL_CTRL10, reg_data);
                handle->scan_mode = (reg_data >> 1) & RG_SCAN_MODE_MASK;

                handle->pixel_test_dac_update_value = MILAN_J_SENSOR_PIXEL_TEST_DAC_UPDATE_VALUE;
                handle->milan_calculate_otp_crc = milan_kjh_calculate_otp_crc;
                handle->milan_check_ft_test_info = milan_kjh_check_ft_test_info;
                break;
            }

            case MILAN_H_CHIP_ID: {
                handle->config = &g_milan_h_config;
                *vendor_param = g_milan_h_vendor_params;
                handle->vendor_id = VENDOR_ID_A;
                handle->milan_raw_data_split = milan_h_raw_data_split;
                handle->milan_update_dac_register_from_opt =
                        milan_common_update_dac_register_from_opt;
                handle->milan_update_config_array_from_opt =
                        milan_common_update_config_array_from_opt;
                MILAN_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(handle->config->image,
                        MILAN_REG_PIXEL_CTRL10, reg_data);
                handle->scan_mode = (reg_data >> 1) & RG_SCAN_MODE_MASK;

                handle->pixel_test_dac_update_value = MILAN_H_SENSOR_PIXEL_TEST_DAC_UPDATE_VALUE;
                handle->milan_calculate_otp_crc = milan_kjh_calculate_otp_crc;
                handle->milan_check_ft_test_info = milan_kjh_check_ft_test_info;
                break;
            }
            default: {
                err = GF_ERROR_SENSOR_NOT_AVAILABLE;
                GF_LOGE(LOG_TAG "[%s] unsupport sensor, chip id=0x%08X", __func__, handle->chip_id);
                break;
            }
        }

        if (GF_SUCCESS != err) {
            break;
        }

        milan_update_scan_mode_into_config_array(handle);

        MILAN_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(handle->config->image, MILAN_REG_ENCRYPT_CTRL3,
                reg_data);

        if (1 == (reg_data & RG_EXTRA_DATA_EN_MASK)) {
            handle->image_add_extra_data_rows = (reg_data & 0xff00) >> 8;
            GF_LOGD(LOG_TAG "[%s] extra_data_rows=%u", __func__, handle->image_add_extra_data_rows);
        } else {
            handle->image_add_extra_data_rows = g_sensor.row;
        }

        MILAN_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(handle->config->nav, MILAN_REG_ENCRYPT_CTRL3,
                reg_data);
        if (1 == (reg_data & RG_EXTRA_DATA_EN_MASK)) {
            handle->nav_add_extra_data_rows = (reg_data & 0xff00) >> 8;
        } else {
            if (MILAN_CHIP_ID_IS_EQUAL(handle->chip_id, MILAN_L_CHIP_ID) ||
                MILAN_CHIP_ID_IS_EQUAL(handle->chip_id, MILAN_FN_CHIP_ID)) {
                handle->nav_add_extra_data_rows = g_sensor.col;
            } else {
                handle->nav_add_extra_data_rows = g_sensor.row;
            }
        }

        MILAN_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(handle->config->image, MILAN_REG_PIXEL_CTRL10,
                reg_data);
        handle->image_double_rate_flag = reg_data & RG_DOUBLE_RATE_MASK;

        MILAN_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(handle->config->nav, MILAN_REG_PIXEL_CTRL10,
                reg_data);
        handle->nav_double_rate_flag = reg_data & RG_DOUBLE_RATE_MASK;

        gf_milan_platform_init_chip_params(handle, &g_read_frame_num_for_nav_in_continue);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

// for OTP CRC
// X^8 + X^2 + X^1 + 1
// for OTP CRC
// X^8 + X^2 + X^1 + 1
uint8_t OTP_CRC8_TAB[256] = { 0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15, 0x38, 0x3F, 0x36,
        0x31, 0x24, 0x23, 0x2A, 0x2D, 0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65, 0x48, 0x4F,
        0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D, 0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5, 0xD8,
        0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD, 0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85,
        0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD, 0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5,
        0xD2, 0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA, 0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC,
        0xA5, 0xA2, 0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A, 0x27, 0x20, 0x29, 0x2E, 0x3B,
        0x3C, 0x35, 0x32, 0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A, 0x57, 0x50, 0x59, 0x5E,
        0x4B, 0x4C, 0x45, 0x42, 0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A, 0x89, 0x8E, 0x87,
        0x80, 0x95, 0x92, 0x9B, 0x9C, 0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4, 0xF9, 0xFE,
        0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC, 0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4, 0x69,
        0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C, 0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
        0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C, 0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33,
        0x34, 0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B, 0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D,
        0x64, 0x63, 0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B, 0x06, 0x01, 0x08, 0x0F, 0x1A,
        0x1D, 0x14, 0x13, 0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB, 0x96, 0x91, 0x98, 0x9F,
        0x8A, 0x8D, 0x84, 0x83, 0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB, 0xE6, 0xE1, 0xE8,
        0xEF, 0xFA, 0xFD, 0xF4, 0xF3 };

uint8_t milan_otp_crc8(uint8_t *ucPtr, uint8_t ucLen) {
    uint8_t ucCRC8 = 0;
    while (ucLen--) {
        uint8_t ucIndex = ucCRC8 ^ (*ucPtr++);
        ucCRC8 = OTP_CRC8_TAB[ucIndex];
    }

    return (~ucCRC8);
}

static gf_error_t milan_set_mode(milan_config_unit_t *config, milan_cmd_t cmd,
        uint32_t is_need_triger) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        // when user cancel authentication and enroll, update fdt up base.
        // if get fdt down, we update. otherwise, not update
        if ( (g_milan_chip_handle.cur_chip_state == MILAN_STATE_IMAGE
             && (g_milan_chip_handle.mode == MODE_IMAGE || g_milan_chip_handle.mode == MODE_FF))
             || (g_milan_chip_handle.cur_chip_state == MILAN_STATE_NAV
            && g_milan_chip_handle.mode == MODE_NAV) ) {
            milan_update_fdt_up_base(1);
        }
        g_milan_chip_handle.auto_switch_mode_flag = 0;  // clear auto switch mode flag

        /**
         * Assure all write configuration operation will be done in idle mode.
         * the invoker of this api has assured the current mode is not FDT or FF mode.
         */
        if (MILAN_STATE_IDLE != g_milan_chip_handle.cur_chip_state) {
            err = gf_milan_secspi_write_cmd(CMD_IDLE);
            if (GF_SUCCESS != err) {
                GF_LOGE(LOG_TAG "[%s] write cmd failed, cmd=0x%02X", __func__, cmd);
                break;
            }
            /* MILAN-F need 1ms to switch mode when IDLE  */
            gf_sleep(1);
        }

        if ((NULL != config) && (0 < config->data_len)) {
            err = gf_milan_secspi_write_bytes(config->start_address, config->data,
                    config->data_len);
            if (GF_SUCCESS != err) {
                GF_LOGE(LOG_TAG "[%s] write configuration failed", __func__);
                break;
            }
        } else {
            /**
             * Just reduce the register operation to improve the performance,for FDT
             * DOWN/FDT UP/IMAGE/NAV, register 0124 will be set to be 0xFF FF. so don't write
             * 0x0124 register at here. Check if there is any IRQ haven't been cleared.
             * First clear exist IRQ.
             */
            uint16_t irq_type = 0xFFFF;

            MILAN_WRITE_REGISTER(MILAN_REG_IRQ_CTRL2, &irq_type);
        }

        /**
         * Milan-F's configuration will contain this register, so don't need to write here.
         */
        if (0 == g_milan_chip_handle.otp_crc_check_result) {
            if ((MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_E_CHIP_ID)
                 || MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_G_CHIP_ID)
                 || MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_FN_CHIP_ID)
                 || MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_L_CHIP_ID))) {
                uint16_t reg_data = 0x0008;

                GF_LOGD(LOG_TAG "[%s] OTP CRC check failed, write register()", __func__);
                MILAN_WRITE_REGISTER(MILAN_REG_RG_PIXEL_CTRL0, &reg_data);
            } else if (!(MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_F_CHIP_ID))) {
                uint16_t reg_data1 = 0x0008;
                uint16_t reg_data2 = 0x0000;

                GF_LOGD(LOG_TAG "[%s] OTP CRC check failed, write register()", __func__);
                MILAN_WRITE_REGISTER(MILAN_REG_RG_PIXEL_CTRL0, &reg_data1);
                MILAN_WRITE_REGISTER(MILAN_REG_RG_PIXEL_DAC_P2, &reg_data2);
                MILAN_WRITE_REGISTER(MILAN_REG_RG_PIXEL_DAC_P3, &reg_data2);
                MILAN_WRITE_REGISTER(MILAN_REG_RG_PIXEL_DAC_P4, &reg_data2);
            }
        }

        if (CMD_IDLE != cmd) {
            err = gf_milan_secspi_write_cmd(cmd);
            if (GF_SUCCESS != err) {
                GF_LOGE(LOG_TAG "[%s] write cmd failed, cmd=0x%02X", __func__, cmd);
                break;
            }

            gf_sleep(1);
        }

        if (is_need_triger) {
            err = gf_milan_secspi_write_cmd(CMD_TMR_TRG);
            if (GF_SUCCESS != err) {
                GF_LOGE(LOG_TAG "[%s] write trigger cmd failed", __func__);
                break;
            }
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t milan_set_image_mode(void) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint16_t reg_data = 0x0110;

        err = milan_set_mode(&(g_milan_chip_handle.config->image), CMD_IMG, 0);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] set image mode failed", __func__);
            break;
        }

        MILAN_WRITE_REGISTER(MILAN_REG_PIXEL_CTRL2, &reg_data);

        g_milan_chip_handle.suspend_data_sampling_flag = 0;
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t milan_set_nav_base_mode(void) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint16_t reg_data = 0x0110;

        if (MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_F_CHIP_ID)) {
            err = milan_set_mode(&(g_milan_chip_handle.config->nav_base), CMD_IMG, 0);
        } else {
            err = milan_set_mode(&(g_milan_chip_handle.config->nav_base), CMD_NAV, 0);
        }

        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] set IMAGE mode failed", __func__);
            break;
        }

        MILAN_WRITE_REGISTER(MILAN_REG_PIXEL_CTRL2, &reg_data);

        g_milan_chip_handle.suspend_data_sampling_flag = 0;
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t milan_set_nav_mode(void) {
    gf_error_t err = GF_SUCCESS;
    uint16_t reg_data = 0x0110;

    FUNC_ENTER();

    do {
        if (MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_F_CHIP_ID)) {
            err = milan_set_mode(&(g_milan_chip_handle.config->nav), CMD_IMG, 0);
            if (GF_SUCCESS != err) {
                GF_LOGE(LOG_TAG "[%s] set nav mode failed", __func__);
                break;
            }
        } else {
            err = milan_set_mode(&(g_milan_chip_handle.config->nav), CMD_NAV, 0);
            if (GF_SUCCESS != err) {
                GF_LOGE(LOG_TAG "[%s] set nav mode failed", __func__);
                break;
            }
            MILAN_WRITE_REGISTER(MILAN_REG_PIXEL_CTRL2, &reg_data);
        }

        g_milan_chip_handle.suspend_data_sampling_flag = 0;
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t milan_read_tcode_and_fdt_threshold(uint8_t *otp_buf) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == otp_buf) {
            GF_LOGE(LOG_TAG "[%s] bad parameters", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        GF_LOGD(LOG_TAG "[%s] otp[0x16]=0x%02X, otp[0x17]=0x%02X", __func__, otp_buf[0x16],
                otp_buf[0x17]);

        if ((0 != otp_buf[0x16]) && (0 == (otp_buf[0x16] - (uint8_t) (~otp_buf[0x17])))) {
            uint32_t diff_use = ((otp_buf[0x16] & 0x0F) + 2) * 100;
            uint32_t tcode = (((otp_buf[0x16] >> 4) & 0x0F) + 1) << 4;
            uint32_t fdt_threshold = (uint8_t) (((diff_use * 256 / tcode) / 3) >> 4);

            // update IMAGE mode's configuration register:0x005C
            GF_LOGI(LOG_TAG "[%s] update IMAGE configuration's 0x005C register to 0x%08X",
                    __func__, tcode);
            MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->image,
                    MILAN_REG_PIXEL_CTRL6, tcode);

            MILAN_UPDATE_CONFIG_ARRAY_BY_BYTE(g_milan_chip_handle.config->ff, MILAN_REG_FDT_DELTA,
                    fdt_threshold);
            MILAN_UPDATE_CONFIG_ARRAY_BY_BYTE(g_milan_chip_handle.config->fdt_manual_down,
                    MILAN_REG_FDT_DELTA, fdt_threshold);
            MILAN_UPDATE_CONFIG_ARRAY_BY_BYTE(g_milan_chip_handle.config->fdt_down,
                    MILAN_REG_FDT_DELTA, fdt_threshold);
            MILAN_UPDATE_CONFIG_ARRAY_BY_BYTE(g_milan_chip_handle.config->fdt_up,
                    MILAN_REG_FDT_DELTA, fdt_threshold - 2);
            MILAN_UPDATE_CONFIG_ARRAY_BY_BYTE(g_milan_chip_handle.config->nav_fdt_down,
                    MILAN_REG_FDT_DELTA, fdt_threshold);
            MILAN_UPDATE_CONFIG_ARRAY_BY_BYTE(g_milan_chip_handle.config->nav_fdt_up,
                    MILAN_REG_FDT_DELTA, fdt_threshold - 2);
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static uint32_t milan_generate_encrypt_key(void) {
    uint32_t id = 0;
    uint8_t buf[4] = { 0 };

    while (1) {
        gf_generate_random(buf, 4);

        id = 0;
        id = (id | buf[3]) << 8;
        id = (id | buf[2]) << 8;
        id = (id | buf[1]) << 8;
        id = (id | buf[0]);

        if (0 == id) {
            GF_LOGE(LOG_TAG "[%s] generate key is zero", __func__);
        } else {
            break;
        }
    }

    return id;
}

static void milan_set_initial_key(uint32_t key) {
    VOID_FUNC_ENTER();

    do {
        uint16_t key_value = key & 0xFFFF;
        MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->image,
                    MILAN_REG_ENCRYPT_CTRL1, key_value);
        MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->nav,
                    MILAN_REG_ENCRYPT_CTRL1, key_value);
        MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->nav_base,
                    MILAN_REG_ENCRYPT_CTRL1, key_value);

        /* initial key high word */
        key_value = (key >> 16) & 0xFFFF;
        MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->image,
                    MILAN_REG_ENCRYPT_CTRL2, key_value);
        MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->nav,
                    MILAN_REG_ENCRYPT_CTRL2, key_value);
        MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->nav_base,
                    MILAN_REG_ENCRYPT_CTRL2, key_value);
    } while (0);

    VOID_FUNC_EXIT();
}

gf_error_t gf_hw_enable_encrypt(uint32_t flag) {
    gf_error_t err = GF_SUCCESS;
    uint16_t reg_data = 0;
    uint16_t set_flag = (flag > 0) ? 1 : 0;

    FUNC_ENTER();

    do {
        g_milan_chip_handle.encrypt_flag = set_flag;
        if (0 < g_milan_chip_handle.encrypt_flag) {
            g_milan_chip_handle.encrypt_key = milan_generate_encrypt_key();
        } else {
            g_milan_chip_handle.encrypt_key = 0;
        }
        /* set encrypt key */
        milan_set_initial_key(g_milan_chip_handle.encrypt_key);

        /* enable encrypt function */
        MILAN_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->image,
                MILAN_REG_ENCRYPT_ENABLE, reg_data);
        reg_data &= (~0x1);
        reg_data |= set_flag;
        MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->image,
                    MILAN_REG_ENCRYPT_ENABLE, reg_data);

        MILAN_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->nav,
                MILAN_REG_ENCRYPT_ENABLE, reg_data);
        reg_data &= (~0x1);
        reg_data |= set_flag;
        MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->nav,
                    MILAN_REG_ENCRYPT_ENABLE, reg_data);

        MILAN_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->nav_base,
                MILAN_REG_ENCRYPT_ENABLE, reg_data);
        reg_data &= (~0x1);
        reg_data |= set_flag;
        MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->nav_base,
                    MILAN_REG_ENCRYPT_ENABLE, reg_data);
    } while (0);

    FUNC_EXIT(err);

    return err;
}


gf_error_t gf_hw_get_chip_id(uint8_t *value, uint32_t buf_len) {
    gf_error_t err = GF_SUCCESS;

    UNUSED_VAR(buf_len);

    FUNC_ENTER();

    do {
        if (NULL == value || buf_len < sizeof(uint32_t)) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad parameter", __func__);
            break;
        }

        if (MILAN_STATE_IDLE != g_milan_chip_handle.cur_chip_state) {
            err = milan_set_mode(NULL, CMD_IDLE, 0);
            if (GF_SUCCESS != err) {
                GF_LOGE(LOG_TAG "[%s] switch into idle mode fail, err = %d", __func__, err);
                break;
            }

            MILAN_READ_REGISTER(MILAN_REG_CHIP_ID, value);

            err = gf_hw_set_mode(g_milan_chip_handle.mode);
            if (GF_SUCCESS != err) {
                g_milan_chip_handle.cur_chip_state = MILAN_STATE_IDLE;
                g_milan_chip_handle.chip_state = MODE_IDLE_STATE_MASK;
                g_milan_chip_handle.mode = MODE_IDLE;
                GF_LOGE(LOG_TAG "[%s] set mode failed, mode=%s",
                        __func__, gf_strmode(g_milan_chip_handle.mode));
                break;
            }

        } else {
            MILAN_READ_REGISTER(MILAN_REG_CHIP_ID, value);
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_hw_get_vendor_id(uint8_t *buf, uint32_t buf_len) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == buf || buf_len < 2) {
            GF_LOGE(LOG_TAG "[%s] invalid parameters", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        cpl_memcpy(buf, &(g_milan_chip_handle.vendor_id), sizeof(uint16_t));

        GF_LOGD(LOG_TAG "[%s] vendor_id=0x%04X", __func__, g_milan_chip_handle.vendor_id);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t milan_read_otp_info(uint8_t *buf) {
    gf_error_t err = GF_SUCCESS;

    uint16_t reg_data = 0;
    uint8_t bank_num = 0;
    uint8_t bank_len = 0;
    uint8_t *dst = buf;

    FUNC_ENTER();

    do {
        if (MILAN_STATE_IDLE != g_milan_chip_handle.cur_chip_state) {
            err = milan_set_mode(NULL, CMD_IDLE, 0);
            if (GF_SUCCESS != err) {
                GF_LOGE(LOG_TAG "[%s] switch into idle mode fail, err = %d", __func__, err);
                break;
            }

            g_milan_chip_handle.cur_chip_state = MILAN_STATE_IDLE;
            g_milan_chip_handle.chip_state = MODE_IDLE_STATE_MASK;
        }

        err = gf_milan_secspi_write_word(MILAN_REG_OTP_CTRL1, 0x0020);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] write 0x%04X register error", __func__, MILAN_REG_OTP_CTRL1);
            break;
        }

        for (bank_num = 0; bank_num < MILAN_OTP_BANK_NUM; bank_num++) {
            gf_milan_secspi_write_word(MILAN_REG_OTP_CTRL2, bank_num);

            gf_milan_secspi_write_word(MILAN_REG_OTP_CTRL1, 0x0021);
            gf_milan_secspi_write_word(MILAN_REG_OTP_CTRL1, 0x0020);

            for (bank_len = 0; bank_len < MILAN_OTP_BANK_LEN; bank_len++) {
                gf_milan_secspi_write_word(MILAN_REG_OTP_CTRL2, (bank_len << 8));
                gf_milan_secspi_write_word(MILAN_REG_OTP_CTRL1, 0x0022);
                gf_milan_secspi_write_word(MILAN_REG_OTP_CTRL1, 0x0020);

                gf_milan_secspi_read_word(MILAN_REG_OTP_CTRL3, &reg_data);
                *dst++ = (uint8_t) (reg_data & 0xFF);

                GF_LOGD(LOG_TAG "[%s] otp_info[%d], read 0x%04X register 0x%04X", __func__,
                        (bank_num * 4 + bank_len), MILAN_REG_OTP_CTRL3, reg_data);
            }
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_hw_get_sensor_id(uint8_t *buf, uint32_t buf_len) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == buf || buf_len > OTP_LEN_IN_BYTES) {
            GF_LOGE(LOG_TAG "[%s] bad parameters", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        cpl_memcpy(buf, g_milan_chip_handle.otp, buf_len);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_hw_get_otp_data(uint8_t *buf, uint32_t buf_len) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == buf || buf_len < OTP_LEN_IN_BYTES) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad parameter, buf_len=%u", __func__, buf_len);
            break;
        }

        cpl_memcpy(buf, g_milan_chip_handle.otp, OTP_LEN_IN_BYTES);
    } while (0);

    FUNC_EXIT(err);

    return err;
}


static gf_error_t milan_check_otp_crc(int8_t retry_number, uint8_t *otp_buf) {
    gf_error_t err = GF_SUCCESS;
    uint8_t otp_data[OTP_LEN_IN_BYTES] = { 0 };
    FUNC_ENTER();
    GF_LOGD(LOG_TAG "[%s] retry_number=%d", __func__, retry_number);

    do {
        cpl_memset(otp_data, 0, OTP_LEN_IN_BYTES);

        err = milan_read_otp_info(otp_data);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] read OTP data error", __func__);

            if (retry_number > 1) {
                gf_sleep(5);
                continue;
            }
            break;
        }

        err = g_milan_chip_handle.milan_calculate_otp_crc(otp_data);
        if (GF_SUCCESS == err) {
            break;
        }

        if (retry_number > 1) {
            gf_sleep(5);
            GF_LOGD(LOG_TAG "[%s] OTP CRC check failed, need to retry", __func__);
        }
    } while (--retry_number > 0);

    if (0 == retry_number) {
        err = GF_ERROR_SENSOR_TEST_FAILED;
        GF_LOGE(LOG_TAG "[%s] OTP crc check failed", __func__);
    } else {
        if (NULL != otp_buf) {
            cpl_memcpy(otp_buf, otp_data, OTP_LEN_IN_BYTES);
        }
    }

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_milan_load_otp_info(uint8_t *otp_buf, uint32_t *otp_buf_len) {
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

    if (buf != NULL) {
        CPL_MEM_FREE(buf);
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t gf_milan_save_otp_info(uint8_t *buf, uint32_t buf_len) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    err = gf_so_save_persistent_object((int8_t*) GF_OTP_INFO_SECURE_ID, buf, buf_len);
    if (err != GF_SUCCESS) {
        GF_LOGI(LOG_TAG "[%s] save so=%s failed", __func__, GF_OTP_INFO_SECURE_ID);
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t milan_check_sensor_otp_info(uint8_t *otp_info_from_sdcard,
        uint32_t *otp_info_from_sdcard_len) {
    gf_error_t err = GF_SUCCESS;
    uint8_t otp_buf_from_so[GF_SENSOR_OTP_INFO_LEN] = { 0 };
    uint8_t otp_info_buf[GF_SENSOR_OTP_INFO_LEN] = { 0 };
    uint32_t otp_len_from_so = 0;

    FUNC_ENTER();
    do {
        GF_LOGD(LOG_TAG "[%s] OTP CHECK FLOW VERSION=%s", __func__, OTP_CHECK_FLOW_VERSION);

        err = milan_read_otp_info(g_milan_chip_handle.otp);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] read otp info from chip fail, err = %d", __func__, err);
            break;
        }

        if (GF_SUCCESS == gf_milan_load_otp_info(otp_buf_from_so, &otp_len_from_so)) {
            GF_LOGD(LOG_TAG "[%s] load otp info success", __func__);

            if (0 == cpl_memcmp(g_milan_chip_handle.otp, otp_buf_from_so, GF_SENSOR_ID_LEN)) {
                cpl_memcpy(g_milan_chip_handle.otp, otp_buf_from_so, otp_len_from_so);

                g_milan_chip_handle.milan_update_dac_register_from_opt(g_milan_chip_handle.otp);
                g_milan_chip_handle.milan_update_config_array_from_opt(g_milan_chip_handle.otp);
                break;
            }

            gf_so_delete_persistent_object((int8_t*) GF_OTP_INFO_SECURE_ID);
        }

        GF_LOGI(LOG_TAG "[%s] *otp_info_from_sdcard_len:%d", __func__, *otp_info_from_sdcard_len);

        if (*otp_info_from_sdcard_len > 0) {
            err = gf_aes_decrypt(otp_info_from_sdcard, otp_info_buf, GF_SENSOR_OTP_INFO_LEN,
                g_milan_chip_handle.otp, GF_SENSOR_ID_LEN);
            if (GF_SUCCESS != err) {
                GF_LOGE(LOG_TAG "[%s] decrypt otp info from hal fail, err = %d", __func__, err);
            }

            if (0 == cpl_memcmp(g_milan_chip_handle.otp, otp_info_buf, GF_SENSOR_ID_LEN)) {
                cpl_memcpy(g_milan_chip_handle.otp, otp_info_buf, GF_SENSOR_OTP_INFO_LEN);

                gf_milan_save_otp_info(otp_info_buf, GF_SENSOR_OTP_INFO_LEN);

                GF_LOGD(LOG_TAG "[%s] opt info from file system matches", __func__);
                g_milan_chip_handle.milan_update_dac_register_from_opt(g_milan_chip_handle.otp);
                g_milan_chip_handle.milan_update_config_array_from_opt(g_milan_chip_handle.otp);
                break;
            }
        }

        GF_LOGD(LOG_TAG "[%s] load_otp_info fail or sensor id check failed", __func__);

        err = g_milan_chip_handle.milan_calculate_otp_crc(g_milan_chip_handle.otp);
        if (GF_SUCCESS == err) {
            gf_milan_save_otp_info(g_milan_chip_handle.otp, GF_SENSOR_OTP_INFO_LEN);
            break;
        }

        err = milan_check_otp_crc(9, g_milan_chip_handle.otp);
        if (GF_SUCCESS == err) {
            gf_milan_save_otp_info(g_milan_chip_handle.otp, GF_SENSOR_OTP_INFO_LEN);
            break;
        }

        GF_LOGE(LOG_TAG "[%s] OTP check failed", __func__);
    } while (0);

    if (GF_SUCCESS == err) {
        g_milan_chip_handle.otp_crc_check_result = 1;
        GF_LOGI(LOG_TAG "[%s] OTP check success at boot up phase", __func__);

        cpl_memcpy(otp_info_buf, g_milan_chip_handle.otp, GF_SENSOR_OTP_INFO_LEN);
        *otp_info_from_sdcard_len = OTP_LEN_IN_BYTES;

        err = gf_aes_encrypt(otp_info_buf, otp_info_from_sdcard, GF_SENSOR_OTP_INFO_LEN,
                g_milan_chip_handle.otp, GF_SENSOR_ID_LEN);
        if (GF_SUCCESS != err) {
            err = GF_SUCCESS;
            *otp_info_from_sdcard_len = 0;
            GF_LOGE(LOG_TAG "[%s] encrypt otp info from chip fail, err=%d", __func__, err);
        }
    } else {
        g_milan_chip_handle.otp[0x1a] = 0xF1;
        g_milan_chip_handle.otp[0x1b] = 0xFB;
        g_milan_chip_handle.otp[0x1c] = 0x09;

        g_milan_chip_handle.otp_crc_check_result = 0;

        GF_LOGI(LOG_TAG "[%s] OTP CRC check failed, and update 0x0220 register to 0x08", __func__);
        gf_milan_secspi_write_word(MILAN_REG_RG_PIXEL_CTRL0, 0x08);

        if (MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_K_CHIP_ID)
                || MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_J_CHIP_ID)
                || MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_H_CHIP_ID)) {
                    GF_LOGI(LOG_TAG "[%s] OTP CRC check failed, and update register to 0x00",
                            __func__);
                    gf_milan_secspi_write_word(MILAN_REG_RG_PIXEL_DAC_P2, 0x00);
                    gf_milan_secspi_write_word(MILAN_REG_RG_PIXEL_DAC_P3, 0x00);
                    gf_milan_secspi_write_word(MILAN_REG_RG_PIXEL_DAC_P4, 0x00);
                }

        if (MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_F_CHIP_ID)) {
            milan_update_config_arrays(MILAN_REG_RG_PIXEL_CTRL0, 0x08);
        }
    }

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_hw_chip_config_init(void) {
    gf_error_t err = GF_SUCCESS;
    uint32_t image_tcode = 0;
    uint32_t nav_tcode = 0;
    gf_fdt_params_t init_params = {0};

    FUNC_ENTER();

    do {
        if (0 == g_milan_chip_handle.otp_crc_check_result) {
            err = GF_ERROR_SENSOR_TEST_FAILED;
            GF_LOGE(LOG_TAG "[%s] OTP crc check failed in boot phase", __func__);
            break;
        }

        err = milan_read_tcode_and_fdt_threshold(g_milan_chip_handle.otp);

        err = milan_init_default_dac(&g_milan_chip_handle);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] init default dac fail, err:%d", __func__, err);
            break;
        }

        MILAN_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->nav,
                MILAN_REG_PIXEL_CTRL6, nav_tcode);
        MILAN_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->image,
                    MILAN_REG_PIXEL_CTRL6, image_tcode);

        milan_get_fifo_spi_rates(image_tcode, nav_tcode);

        /* init FDT */
        err = milan_init_fdt_params_from_chip_config(&init_params);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] read fdt init params failed", __func__);
            break;
        }
        init_params.image_row = g_sensor.row;
        init_params.image_col = g_sensor.col;

        err = gf_fdt_init(g_config.chip_series, g_config.chip_type, &init_params);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] fdt init failed", __func__);
            break;
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

void gf_hw_destroy(void) {
}

static gf_error_t milan_enable_spi_bypass_deglitch(void) {
    uint16_t reg_data = 0;
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    do {
        // enhancement spi bypass feature
        MILAN_READ_REGISTER(MILAN_REG_IO_CTRL0, &reg_data);

        reg_data |= 0x8000;
        MILAN_WRITE_REGISTER(MILAN_REG_IO_CTRL0, &reg_data);

        // clear
        reg_data = 0;
        MILAN_READ_REGISTER(MILAN_REG_SPI_CTRL0, &reg_data);
        reg_data |= 0x0001;
        MILAN_WRITE_REGISTER(MILAN_REG_SPI_CTRL0, &reg_data);
    } while (0);
    FUNC_EXIT(err);
    return err;
}

static gf_error_t milan_get_fifo_spi_rates(uint32_t image_tcode, uint32_t nav_tcode) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do {
        uint32_t image_add_extra_data_rows = g_sensor.row;
        uint32_t nav_add_extra_data_rows = g_sensor.nav_row;
        uint32_t image_double_rate_flag = 0;
        uint32_t nav_double_rate_flag = 0;
        uint32_t reg_data = 0;

        MILAN_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->image,
                MILAN_REG_ENCRYPT_CTRL3, reg_data);
        if (1 == (reg_data & RG_EXTRA_DATA_EN_MASK)) {
            image_add_extra_data_rows = (reg_data & 0xff00) >> 8;
        } else {
            image_add_extra_data_rows = g_sensor.row;
        }

        MILAN_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->nav,
                MILAN_REG_ENCRYPT_CTRL3, reg_data);
        if (1 == (reg_data & RG_EXTRA_DATA_EN_MASK)) {
            nav_add_extra_data_rows = (reg_data & 0xff00) >> 8;
        } else {
            nav_add_extra_data_rows = g_sensor.nav_row;
        }

        MILAN_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->image,
                MILAN_REG_PIXEL_CTRL10, reg_data);
        image_double_rate_flag = reg_data & RG_DOUBLE_RATE_MASK;
        MILAN_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->nav,
                MILAN_REG_PIXEL_CTRL10, reg_data);
        nav_double_rate_flag = reg_data & RG_DOUBLE_RATE_MASK;

        GF_LOGD(LOG_TAG "[%s] get image fifo rate=%u %u %u", __func__, image_add_extra_data_rows,
                g_sensor.row, image_tcode);
        GF_LOGD(LOG_TAG "[%s] get nav fifo rate=%u %u %u", __func__, nav_add_extra_data_rows,
                g_sensor.nav_col, nav_tcode);
        if (image_double_rate_flag) {
            g_milan_chip_handle.img_fifo_spi_rate = (uint32_t) (((image_add_extra_data_rows * 2
                    * 12) / ((8 + image_tcode + 2 * g_sensor.row) * 2 / 5)));
        } else {
            g_milan_chip_handle.img_fifo_spi_rate = (uint32_t) (((image_add_extra_data_rows * 12)
                    / ((8 + image_tcode + g_sensor.row) * 2 / 5)));
        }

        if (nav_double_rate_flag) {
            g_milan_chip_handle.nav_fifo_spi_rate = (uint32_t) (((nav_add_extra_data_rows * 2
                                * 12) / ((8 + nav_tcode + 2 * g_sensor.nav_row) * 2 / 5)));
        } else {
            g_milan_chip_handle.nav_fifo_spi_rate = (uint32_t) (((nav_add_extra_data_rows * 12)
                                / ((8 + nav_tcode + g_sensor.nav_row) * 2 / 5)));
        }

        if (g_milan_chip_handle.img_fifo_spi_rate > MILAN_MAX_SPI_RATE) {
            g_milan_chip_handle.img_fifo_spi_rate = MILAN_MAX_SPI_RATE;
        }

        if (g_milan_chip_handle.nav_fifo_spi_rate > MILAN_MAX_SPI_RATE) {
            g_milan_chip_handle.nav_fifo_spi_rate = MILAN_MAX_SPI_RATE;
        }

        GF_LOGI(LOG_TAG "[%s] get image fifo rate=%u , nav fifo rate=%u", __func__,
                g_milan_chip_handle.img_fifo_spi_rate, g_milan_chip_handle.nav_fifo_spi_rate);
    } while (0);
    FUNC_EXIT(err);
    return err;
}

// in order to reduce spi transfer, so directly return the otp check result at the boot phase.
gf_error_t gf_milan_test_sensor_validity(void) {
    gf_error_t err = GF_SUCCESS;
    uint8_t otp_buf[OTP_LEN_IN_BYTES] = { 0 };

    FUNC_ENTER();

    do {
        // OTP CRC check
        err = milan_check_otp_crc(1, otp_buf);

        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] OTP CRC check failed", __func__);
            break;
        }

        err = g_milan_chip_handle.milan_check_ft_test_info(otp_buf);

        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] OTP FT test check failed", __func__);
            break;
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_hw_detect_sensor(void) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        milan_init_chip_handle();

        MILAN_READ_REGISTER(MILAN_REG_CHIP_ID, &(g_milan_chip_handle.chip_id));
        GF_LOGI(LOG_TAG "[%s] chip id=0x%08X", __func__, g_milan_chip_handle.chip_id);

        if (!CHIP_ID_IS_MILAN_F_SERIES(g_milan_chip_handle.chip_id)) {
            err = GF_ERROR_SENSOR_NOT_AVAILABLE;
            break;
        }

        switch (g_milan_chip_handle.chip_id >> 8) {
            case MILAN_E_CHIP_ID:
                g_config.chip_type = GF_CHIP_3266;
                break;
            case MILAN_F_CHIP_ID:
                g_config.chip_type = GF_CHIP_3208;
                break;
            case MILAN_G_CHIP_ID:
                g_config.chip_type = GF_CHIP_3206;
                break;
            case MILAN_L_CHIP_ID:
                g_config.chip_type = GF_CHIP_3288;
                break;
            case MILAN_FN_CHIP_ID:
                g_config.chip_type = GF_CHIP_3268;
                break;
            case MILAN_K_CHIP_ID:
                g_config.chip_type = GF_CHIP_3228;
                break;
            case MILAN_J_CHIP_ID:
                g_config.chip_type = GF_CHIP_3226;
                break;
            case MILAN_H_CHIP_ID:
                g_config.chip_type = GF_CHIP_3258;
                break;
            default:
                break;
        }
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_milan_chip_init(uint8_t *otp_info, uint32_t *otp_info_len) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    GF_LOGD(LOG_TAG "[%s] MILAN CHIP FLOW VERSION=%s", __func__, MILAN_CHIP_FLOW_VERSION);

    do {
        uint32_t i = 0;
        uint16_t reg_data = 0;
        const milan_vendor_params_t* milan_vendor_param = NULL;

        if (MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_F_CHIP_ID)) {
            MILAN_READ_REGISTER(MILAN_REG_DUMMY, &reg_data);

            GF_LOGD(LOG_TAG "[%s] Milan_F chip version = 0x%x", __func__, reg_data);
        }

        err = milan_init_configuration(&g_milan_chip_handle, &milan_vendor_param);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] init configuration failed", __func__);
            break;
        }

        reg_data = 0xFFFF;  // clear all irq at the boot phase.
        MILAN_WRITE_REGISTER(MILAN_REG_IRQ_CTRL2, &reg_data);

        milan_crc32_table_init();

        if (MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_F_CHIP_ID)) {
            reg_data = 0;
            MILAN_READ_REGISTER(MILAN_REG_OSC_CTRL, &reg_data);
            milan_update_config_arrays(MILAN_REG_OSC_CTRL, reg_data);

            reg_data = 0;
            MILAN_READ_REGISTER(MILAN_REG_RG_PIXEL_CTRL0, &reg_data);
            milan_update_config_arrays(MILAN_REG_RG_PIXEL_CTRL0, reg_data);

            reg_data = 0;
            MILAN_WRITE_REGISTER(MILAN_REG_ESD_DETECT, &reg_data);

        } else {
            reg_data = 0x0403;
            MILAN_WRITE_REGISTER(MILAN_REG_CLOCK_DIV_CTRL, &reg_data);
            milan_enable_spi_bypass_deglitch();

            // read nav col start, col step
            MILAN_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->nav,
                    MILAN_REG_PIXEL_CTRL8, reg_data);
            g_sensor.nav_gap = (reg_data & 0xFF00) >> 8;

            // read nav col num
            MILAN_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->nav,
                    MILAN_REG_PIXEL_CTRL9, reg_data);
            g_sensor.nav_col = reg_data & 0x00FF;

            GF_LOGD(LOG_TAG "[%s] override nav_gap=%u, nav_col=%u", __func__,
                    g_sensor.nav_gap, g_sensor.nav_col);
        }

        err = milan_check_sensor_otp_info(otp_info, otp_info_len);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] OTP check failed", __func__);
            break;
        }

        if (MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_K_CHIP_ID)) {
            g_milan_chip_handle.vendor_id = g_milan_chip_handle.otp[0x18];
        } else {
            // set gpio-ctrl [reg:0x0008] as down status
            reg_data = 0x1010;
            MILAN_WRITE_REGISTER(MILAN_REG_GPIO_CTRL, &reg_data);

            // read vendor id
            MILAN_READ_REGISTER(MILAN_REG_VENDOR_ID, &(g_milan_chip_handle.vendor_id));
            GF_LOGI(LOG_TAG "[%s] vendor id=0x%04X", __func__, g_milan_chip_handle.vendor_id);

            // set gpio-ctrl [reg:0x0008] as floating status
            reg_data = 0x1111;
            MILAN_WRITE_REGISTER(MILAN_REG_GPIO_CTRL, &reg_data);
        }

        for (i = 0; i < MILAN_VENDOR_MAX_COUNT; i++) {
            if (g_milan_chip_handle.vendor_id == milan_vendor_param[i].vendor_id) {
                break;
            }
        }

        if (i == MILAN_VENDOR_MAX_COUNT) {
            GF_LOGI(LOG_TAG "[%s] unknown vendor id=0x%04X, adopt the first vendor's configuration",
                    __func__, g_milan_chip_handle.vendor_id);
            i = 0;
        }

        cpl_memcpy(&(g_milan_chip_handle.vendor_param), &(milan_vendor_param[i]),
                sizeof(milan_vendor_params_t));
    } while (0);

    FUNC_EXIT(err);

    return err;
}

/*Suspend to sample finger-print data*/
gf_error_t gf_hw_data_sample_suspend(void) {
    g_milan_chip_handle.suspend_data_sampling_flag = 1;

    return GF_SUCCESS;
}

/*Resume to sample finger-print data*/
gf_error_t gf_hw_data_sample_resume(void) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint16_t reg_data = 0;

        if ((MILAN_STATE_IMAGE != g_milan_chip_handle.cur_chip_state)
                && (MILAN_STATE_NAV_BASE != g_milan_chip_handle.cur_chip_state)) {
            GF_LOGD(LOG_TAG "[%s] g_milan_chip_handle.mode=%s, g_milan_chip_handle.mode=%d, "
                    "g_milan_chip_handle.cur_chip_state=%d, can't resume data sampling",
                    __func__, gf_strmode(g_milan_chip_handle.mode), g_milan_chip_handle.mode,
                    g_milan_chip_handle.cur_chip_state);
            break;
        }

        if (0 == g_milan_chip_handle.suspend_data_sampling_flag) {
            GF_LOGI(LOG_TAG "[%s] didn't suspend data sampling, so don't need to re-trigger",
                    __func__);
            break;
        }

        g_milan_chip_handle.suspend_data_sampling_flag = 0;

        MILAN_READ_REGISTER(MILAN_REG_IRQ_CTRL3, &reg_data);

        if (0 != reg_data) {
            GF_LOGI(LOG_TAG "[%s] interrupt haven't been cleared, reg_data=0x%04X", __func__,
                    reg_data);
            break;
        }

        reg_data = 0x0110;
        MILAN_WRITE_REGISTER(MILAN_REG_PIXEL_CTRL2, &reg_data);

        g_milan_chip_handle.cur_chip_state = MILAN_STATE_IMAGE;
        g_milan_chip_handle.suspend_data_sampling_flag = 1;
        g_milan_chip_handle.auto_switch_mode_flag = 0;

        GF_LOGI(LOG_TAG "[%s] re-trigger scanning data in IMAGE mode", __func__);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_milan_update_fdt_base(uint16_t *buf, uint32_t buf_len) {
    gf_error_t err = GF_SUCCESS;

    UNUSED_VAR(buf_len);

    FUNC_ENTER();

    do {
        uint8_t i = 0;
        uint8_t fdt_area_num = MILAN_REG_SIZE(MILAN_REG_FDT_THRES) / 2;
        uint16_t fdt_thres_offset = 0;

        if (NULL == buf) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad input params", __func__);
            break;
        }

        fdt_thres_offset = MILAN_REG_FDT_THRES - g_milan_chip_handle.config->ff.start_address;
        for (i = 0; i < fdt_area_num; i++) {
            uint8_t fdt_threshold = buf[i] & 0xFF;

            GF_LOGD(LOG_TAG "[%s] fdt_threshold[%d]=0x%02X", __func__, i, fdt_threshold);

            g_milan_chip_handle.config->ff.data[fdt_thres_offset + i * 2] = fdt_threshold;
            g_milan_chip_handle.config->fdt_manual_down.data[fdt_thres_offset + i * 2] =
                    fdt_threshold;
            g_milan_chip_handle.config->fdt_down.data[fdt_thres_offset + i * 2] = fdt_threshold;
            g_milan_chip_handle.config->fdt_up.data[fdt_thres_offset + i * 2] = fdt_threshold;
            g_milan_chip_handle.config->nav_fdt_down.data[fdt_thres_offset + i * 2] = fdt_threshold;
            g_milan_chip_handle.config->nav_fdt_up.data[fdt_thres_offset + i * 2] = fdt_threshold;
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_hw_set_mode(gf_mode_t mode) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        milan_state_t milan_next_status = MILAN_STATE_IDLE;

        if (mode >= MODE_MAX) {
            GF_LOGE(LOG_TAG "[%s] bad parameter, mode=%s, mode_code=%d",
                    __func__, gf_strmode(mode), mode);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        GF_LOGI(LOG_TAG "[%s] current_mode=%s, current_mode_code=%d, cur_chip_state=%d, "
                "new_mode=%s, new_mode_code=%d", __func__, gf_strmode(g_milan_chip_handle.mode),
                g_milan_chip_handle.mode, g_milan_chip_handle.cur_chip_state, gf_strmode(mode),
                mode);

        switch (mode) {
            case MODE_DEBUG:
            case MODE_FINGER_BASE: {
                milan_next_status = MILAN_STATE_IMAGE;
                break;
            }

            case MODE_NAV_BASE: {
                milan_next_status = MILAN_STATE_NAV_BASE;
                break;
            }

            case MODE_SLEEP: {
                milan_next_status = MILAN_STATE_SLEEP;
                break;
            }

            case MODE_NAV:
            case MODE_KEY:
            case MODE_FF:
            case MODE_IMAGE: {
                gf_error_t detect_finger_up = GF_SUCCESS;

                milan_fdt_up_detect_mode_init(mode);

                detect_finger_up = milan_check_finger_up();

                if (GF_SUCCESS == detect_finger_up) {  // finger is up, so update FDT DOWN BASE
                    milan_update_fdt_down_base();
                } else if (GF_ERROR_FINGER_NOT_LEAVE == detect_finger_up) {
                    // finger is down, so update FDT UP BASE
                    milan_update_fdt_up_base(0);
                    GF_LOGI(LOG_TAG "[%s] finger down, update finger up base", __func__);
                }

                g_milan_chip_handle.is_report_finger_up_for_ff_or_image_mode = 1;
                if (((MODE_FF == mode) && (1 == g_config.reissue_key_down_when_entry_ff_mode))
                        || ((MODE_IMAGE == mode) &&
                                (1 == g_config.reissue_key_down_when_entry_image_mode))
                        || (MODE_KEY == mode &&
                                (1 == g_config.reissue_key_down_when_entry_key_mode))
                        || (MODE_NAV == mode &&
                                (1 == g_config.reissue_key_down_when_entry_nav_mode))) {
                    GF_LOGI(LOG_TAG "[%s] reissue key down for mode=%s, mode_code=%d",
                            __func__, gf_strmode(mode), mode);
                    milan_next_status = MILAN_STATE_FDT_DOWN;
                } else {
                    /**
                     * When don't reissue key down event, should check finger up first, and then
                     * check finger down. If current chip mode is MILAN_STATE_FDT_DOWN, that means
                     * finger is not press now, in order to improve the performance, so don't need
                     * switch mode to MILAN_STATE_FDT_UP, and directly use MILAN_STATE_FDT_DOWN
                     * to detect finger down.
                     */
                    if (MILAN_STATE_FDT_DOWN == g_milan_chip_handle.cur_chip_state
                            || GF_SUCCESS == detect_finger_up) {
                        milan_next_status = MILAN_STATE_FDT_DOWN;
                        GF_LOGI(LOG_TAG "[%s] finger is not down, so detect finger down", __func__);
                    } else {
                        milan_next_status = MILAN_STATE_FDT_UP;
                        g_milan_chip_handle.is_report_finger_up_for_ff_or_image_mode = 0;
                        GF_LOGI(LOG_TAG "[%s] don't reissue key down for mode=%s, mode_code=%d",
                                __func__, gf_strmode(mode), mode);
                    }
                }
                break;
            }

            case MODE_IDLE: {
                milan_next_status = MILAN_STATE_IDLE;
                break;
            }

            default: {
                GF_LOGE(LOG_TAG "[%s] mode not support 0x%04X", __func__, mode);
                err = GF_ERROR_BAD_PARAMS;
                break;
            }
        }

        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] unknown new mode=%s, mode_code=%d",
                    __func__, gf_strmode(mode), mode);
            break;
        }

        if (milan_next_status == g_milan_chip_handle.cur_chip_state) {
            GF_LOGI(LOG_TAG "[%s] chip mode is same=%d",
                    __func__, g_milan_chip_handle.cur_chip_state);
            break;
        }

        switch (milan_next_status) {
            case MILAN_STATE_FDT_DOWN: {
                if (MODE_NAV == mode) {
                    err = milan_set_mode(&(g_milan_chip_handle.config->nav_fdt_down), CMD_FDT, 0);
                } else {
                    err = milan_set_mode(&(g_milan_chip_handle.config->fdt_down), CMD_FDT, 0);
                }

                if (GF_SUCCESS != err) {
                    GF_LOGE(LOG_TAG "[%s] set FDT DOWN failed, mode=%s, mode_code=%d",
                            __func__, gf_strmode(mode), mode);
                    break;
                }

                g_milan_chip_handle.cur_chip_state = MILAN_STATE_FDT_DOWN;
                break;
            }

            case MILAN_STATE_FDT_UP: {
                if (MODE_NAV == mode) {
                    err = milan_set_mode(&(g_milan_chip_handle.config->nav_fdt_up), CMD_FDT, 0);
                } else {
                    err = milan_set_mode(&(g_milan_chip_handle.config->fdt_up), CMD_FDT, 0);
                }

                if (GF_SUCCESS != err) {
                    GF_LOGE(LOG_TAG "[%s] set FDT UP failed, mode=%s, mode_code=%d",
                            __func__, gf_strmode(mode), mode);
                    break;
                }

                g_milan_chip_handle.cur_chip_state = MILAN_STATE_FDT_UP;
                break;
            }

            case MILAN_STATE_IMAGE: {
                err = milan_set_image_mode();
                if (GF_SUCCESS != err) {
                    GF_LOGE(LOG_TAG "[%s] set IMAGE mode failed", __func__);
                    break;
                }

                g_milan_chip_handle.cur_chip_state = MILAN_STATE_IMAGE;
                break;
            }

            case MILAN_STATE_NAV_BASE: {
                err = milan_set_nav_base_mode();
                if (GF_SUCCESS != err) {
                    GF_LOGE(LOG_TAG "[%s] set NAV mode failed", __func__);
                    break;
                }

                g_milan_chip_handle.cur_chip_state = MILAN_STATE_NAV_BASE;
                break;
            }

            case MILAN_STATE_FF: {
                err = milan_set_mode(&(g_milan_chip_handle.config->ff), CMD_FF, 0);
                if (GF_SUCCESS != err) {
                    GF_LOGE(LOG_TAG "[%s] set FF mode failed", __func__);
                    break;
                }

                g_milan_chip_handle.cur_chip_state = MILAN_STATE_FF;
                break;
            }

            case MILAN_STATE_IDLE: {
                err = milan_set_mode(NULL, CMD_IDLE, 0);
                if (GF_SUCCESS != err) {
                    GF_LOGE(LOG_TAG "[%s] set IDLE mode failed", __func__);
                    break;
                }

                g_milan_chip_handle.cur_chip_state = MILAN_STATE_IDLE;
                break;
            }

            case MILAN_STATE_SLEEP: {
                err = milan_set_mode(NULL, CMD_SLEEP, 0);
                if (GF_SUCCESS != err) {
                    GF_LOGE(LOG_TAG "[%s] set SLEEP mode failed", __func__);
                    break;
                }

                g_milan_chip_handle.cur_chip_state = MILAN_STATE_SLEEP;
                break;
            }

            default: {
                GF_LOGE(LOG_TAG "[%s] invalid next chip mode", __func__);
                err = GF_ERROR_BAD_PARAMS;
                break;
            }
        }
    } while (0);

    if (GF_SUCCESS == err) {
        g_milan_chip_handle.mode = mode;
    }

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_hw_get_mode(gf_mode_t *value) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    *value = g_milan_chip_handle.mode;

    GF_LOGD(LOG_TAG "[%s] g_milan_chip_handle.mode=%s, g_milan_chip_handle.mode=%d",
            __func__, gf_strmode(*value), *value);
    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_milan_pre_get_image(void) {
    return GF_SUCCESS;
}

gf_error_t gf_milan_get_image(uint8_t *origin_data, uint32_t *origin_data_len) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    GF_LOGD(LOG_TAG "[%s] image_add_extra_data_rows=%u, g_sensor.col=%u", __func__,

    g_milan_chip_handle.image_add_extra_data_rows, g_sensor.col);

    do {
        uint32_t data_len = (g_milan_chip_handle.image_add_extra_data_rows * g_sensor.col * 3) / 2;
        g_image_read_len = data_len + 4;
        g_milan_image_rawdata = origin_data;

        err = gf_milan_secspi_read_bytes_by_speed(MILAN_REG_DATA_READ, g_milan_image_rawdata,
                g_image_read_len, g_milan_chip_handle.img_fifo_spi_rate);

        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] read image failed, err=%s, errno=%d",
                    __func__, gf_strerror(err), err);
            break;
        }

        err = milan_crc32_check(g_milan_image_rawdata, data_len);
        if (GF_SUCCESS == err) {
            if (0 < g_milan_chip_handle.encrypt_flag) {
                gf_prbs_decrypt(g_milan_chip_handle.encrypt_key,
                    (uint16_t*)(void*)g_milan_image_rawdata,
                    (uint16_t*)(void*)g_milan_image_rawdata, (data_len >> 1));
            }
            *origin_data_len = g_image_read_len;
            break;
        }

        *origin_data_len = 0;
    } while (0);

    if (GF_SUCCESS != err && (MODE_IMAGE == g_milan_chip_handle.mode
            || MODE_FF == g_milan_chip_handle.mode)) {
        g_milan_chip_handle.auto_switch_mode_flag = 1;
        g_milan_chip_handle.next_chip_state = MILAN_STATE_FDT_UP;
        GF_LOGI(LOG_TAG "[%s] read image data fail, wait to switch to next mode(fdt up), "
                "err=%s, errno=%d", __func__, gf_strerror(err), err);
    }

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_milan_post_get_image(uint16_t *raw_data, uint32_t *raw_data_len) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint32_t data_len = (g_milan_chip_handle.image_add_extra_data_rows * g_sensor.col * 3) / 2;

        /* decipher and do data re-order */
        err = g_milan_chip_handle.milan_raw_data_split(raw_data, g_milan_image_rawdata,
                data_len, g_sensor.col, g_milan_chip_handle.image_double_rate_flag, 1,
                g_milan_chip_handle.scan_mode);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] split raw_data failed", __func__);
            break;
        }

        g_milan_chip_handle.suspend_data_sampling_flag = 1;

        *raw_data_len = g_sensor.row * g_sensor.col;
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_milan_pre_get_nav_image(void) {
    return GF_SUCCESS;
}

gf_error_t gf_milan_get_nav_image(uint8_t *origin_data, uint32_t *origin_data_len) {
    gf_error_t err = GF_SUCCESS;
    // for MTK platform: Milan-F use continue mode, and read 5 frame one time
    uint32_t read_frame_num =
            (0 == MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_F_CHIP_ID)
                    && 0 == MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_H_CHIP_ID)) ?
                    1 : g_read_frame_num_for_nav_in_continue;
    uint32_t data_len = (g_milan_chip_handle.nav_add_extra_data_rows * g_sensor.nav_col * 3) / 2;

    FUNC_ENTER();

    GF_LOGD(LOG_TAG "[%s] data_len=%d, read_frame_num = %d", __func__, data_len, read_frame_num);
    GF_LOGD(LOG_TAG "[%s] g_milan_chip_handle.nav_add_extra_data_rows=%u",
            __func__, g_milan_chip_handle.nav_add_extra_data_rows);

    g_milan_valid_nav_rawdata_len = 0;

    do {
        uint32_t read_counter = 0;
        uint16_t reg_data = 0;
        uint32_t read_bytes = (data_len + 4) * read_frame_num;

        if (MILAN_STATE_NAV != g_milan_chip_handle.cur_chip_state) {
            GF_LOGE(LOG_TAG "[%s] current mode is not nav, g_milan_chip_handle.cur_chip_state=%d",
                    __func__, g_milan_chip_handle.cur_chip_state);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        g_milan_nav_rawdata = origin_data;

        if (0 == MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_F_CHIP_ID)
                && 0 == MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_H_CHIP_ID)) {
            if (0 != ((read_bytes + 1) % 1024)) {
                read_bytes = ((read_bytes / 1024) + 1) * 1024 - 1;
            }
        }

        GF_LOGD(LOG_TAG "[%s] read_bytes = %d", __func__, read_bytes);

        for (; read_counter < MILAN_READ_NAV_DATA_MAX_RETRY_TIMES; read_counter++) {
            reg_data = 0;
            MILAN_READ_REGISTER(MILAN_REG_FIFO_USED, &reg_data);

            GF_LOGD(LOG_TAG "[%s] reg_data = 0x%4x", __func__, reg_data);

            if (reg_data > 0) {
                break;
            }
        }

        if (read_counter >= MILAN_READ_NAV_DATA_MAX_RETRY_TIMES) {
            *origin_data_len = 0;
            GF_LOGE(LOG_TAG "[%s] detect data fail", __func__);
            err = GF_ERROR_SPI_GENERAL_ERROR;
            break;
        }

        err = gf_milan_secspi_read_bytes_by_speed(MILAN_REG_DATA_READ, g_milan_nav_rawdata,
                read_bytes, g_milan_chip_handle.nav_fifo_spi_rate);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] read nav failed, err=%s, errno=%d",
                    __func__, gf_strerror(err), err);
            break;
        }

        {
            uint32_t i = 0;

            for (; i < read_frame_num; i++) {
                uint8_t* pdata = NULL;
                pdata = g_milan_nav_rawdata + (data_len + 4) * i;
                err = milan_crc32_check(pdata, data_len);
                if (GF_SUCCESS != err) {
                    GF_LOGE(LOG_TAG "[%s] crc32 frame failed, err=%s, errno=%d",
                            __func__, gf_strerror(err), err);
                    break;
                }

                if (0 < g_milan_chip_handle.encrypt_flag) {
                    gf_prbs_decrypt(g_milan_chip_handle.encrypt_key, (uint16_t*)(void*)pdata,
                        (uint16_t*)(void*)pdata, (data_len >> 1));
                }
                g_milan_valid_nav_rawdata_len += (data_len + 4);
            }
        }

        if (GF_SUCCESS == err) {
            *origin_data_len = g_milan_valid_nav_rawdata_len;
            if (0 == MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_F_CHIP_ID)
                    && 0 == MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_H_CHIP_ID)) {
                reg_data = 0x0110;
                MILAN_WRITE_REGISTER(MILAN_REG_PIXEL_CTRL2, &reg_data);
            }
            break;
        }
        g_milan_valid_nav_rawdata_len = 0;
    } while (0);

    if (GF_SUCCESS != err) {
        g_milan_chip_handle.auto_switch_mode_flag = 1;
        g_milan_chip_handle.next_chip_state = MILAN_STATE_FDT_UP;
        GF_LOGI(LOG_TAG "[%s] read nav raw data fail, wait to switch to next mode(fdt up), "
                "err=%s, errno=%d", __func__, gf_strerror(err), err);
    }

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_milan_post_get_nav_image(uint16_t *raw_data, uint32_t *raw_data_len,
        uint8_t *frame_num, uint8_t *over_flag) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint16_t* base = NULL;
        uint32_t base_len = 0;
        uint32_t one_frame_nav_data_len = (g_milan_chip_handle.nav_add_extra_data_rows
                * g_sensor.nav_col * 3) / 2;
        uint32_t one_frame_nav_rawdata_len = g_sensor.nav_col * g_sensor.nav_row;

        *frame_num = 0;
        *over_flag = 0;

        GF_LOGD(LOG_TAG "[%s] g_milan_valid_nav_rawdata_len = %d",
                __func__, g_milan_valid_nav_rawdata_len);
        gf_nav_get_base(&base, &base_len);

        for (; g_milan_valid_nav_rawdata_len >= one_frame_nav_data_len + 4;
                raw_data += NAV_BUFFER_LEN) {
            err = g_milan_chip_handle.milan_raw_data_split(raw_data, g_milan_nav_rawdata,
                    one_frame_nav_data_len, g_sensor.nav_col,
                    g_milan_chip_handle.nav_double_rate_flag, g_sensor.nav_gap,
                    g_milan_chip_handle.scan_mode);
            if (GF_SUCCESS != err) {
                GF_LOGE(LOG_TAG "[%s] split raw data frame failed", __func__);
                break;
            }

            g_milan_nav_rawdata += (one_frame_nav_data_len + 4);
            g_milan_valid_nav_rawdata_len -= (one_frame_nav_data_len + 4);

            *frame_num += 1;
            *raw_data_len = one_frame_nav_rawdata_len;

            if (0 == milan_get_key_state(raw_data, base, one_frame_nav_rawdata_len,
                    g_sensor.nav_finger_up_pixel_diff)) {
                GF_LOGD(LOG_TAG "[%s] finger is untouched, stop NAV data capturing", __func__);
                *over_flag = 1;

                if (g_config.nav_double_click_interval_in_ms != 0) {
                    /* finger leaving is detected when get NAV image,
                     * so detect finger down directly */
                    GF_LOGD(LOG_TAG "[%s] finger leaving, set to fdt down", __func__);
                    err = milan_set_mode(&(g_milan_chip_handle.config->nav_fdt_down), CMD_FDT, 0);
                    if (GF_SUCCESS != err) {
                        GF_LOGE(LOG_TAG "[%s] set fdt down failed", __func__);
                        break;
                    }

                    g_milan_chip_handle.cur_chip_state = MILAN_STATE_FDT_DOWN;
                }
                break;
            }
        }
    } while (0);

    GF_LOGD(LOG_TAG "[%s] *frame_num = %d, *raw_data_len = %d",
            __func__, *frame_num, *raw_data_len);
    g_milan_valid_nav_rawdata_len = 0;

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_milan_pre_get_nav_base(void) {
    gf_error_t err = GF_SUCCESS;
    if (MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_F_CHIP_ID)) {
       err = gf_milan_pre_get_image();
    }

    return err;
}

gf_error_t gf_milan_get_nav_base(uint8_t *origin_data, uint32_t *origin_data_len) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint32_t data_len = 0;
        if (MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_F_CHIP_ID)) {
            err = gf_milan_get_image(origin_data, origin_data_len);
            break;
        }

        data_len = g_milan_chip_handle.nav_add_extra_data_rows * g_sensor.nav_col * 3 / 2;
        g_milan_nav_rawdata = origin_data;
        GF_LOGD(LOG_TAG "[%s] data_len=%u, nav_spi_rate=%u",
                __func__, data_len, g_milan_chip_handle.nav_fifo_spi_rate);

        err = gf_milan_secspi_read_bytes_by_speed(MILAN_REG_DATA_READ,
                g_milan_nav_rawdata, data_len + 4, g_milan_chip_handle.nav_fifo_spi_rate);

        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] read nav base failed, err=%s, errno=%d",
                    __func__, gf_strerror(err), err);
            break;
        }

        *origin_data_len = data_len + 4;
        err = milan_crc32_check(g_milan_nav_rawdata, data_len);
        if (GF_SUCCESS == err) {
            if (0 < g_milan_chip_handle.encrypt_flag) {
                gf_prbs_decrypt(g_milan_chip_handle.encrypt_key,
                    (uint16_t*)(void*)g_milan_image_rawdata,
                    (uint16_t*)(void*)g_milan_image_rawdata, (data_len >> 1));
            }
            *origin_data_len = data_len + 4;
            break;
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_milan_post_get_nav_base(uint16_t *raw_data, uint32_t *raw_data_len) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint32_t data_len = 0;
        if (MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_F_CHIP_ID)) {
            err = gf_milan_post_get_image(raw_data, raw_data_len);
            break;
        }

        /* decipher and do data re-order */
        data_len = g_milan_chip_handle.nav_add_extra_data_rows * g_sensor.nav_col * 3 / 2;
        err = g_milan_chip_handle.milan_raw_data_split(raw_data, g_milan_nav_rawdata,
                data_len, g_sensor.nav_col, g_milan_chip_handle.nav_double_rate_flag,
                g_sensor.nav_gap, g_milan_chip_handle.scan_mode);
        if (err != GF_SUCCESS) {
            break;
        }

        g_milan_chip_handle.suspend_data_sampling_flag = 1;

        *raw_data_len = g_sensor.nav_row * g_sensor.nav_col;
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_hw_clear_irq(uint32_t irq_type) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint16_t reg_data = 0;

        GF_LOGD(LOG_TAG "[%s] g_milan_chip_handle.mode=%s, g_milan_chip_handle.mode=%d",
                __func__, gf_strmode(g_milan_chip_handle.mode), g_milan_chip_handle.mode);
        GF_LOGD(LOG_TAG "[%s] irq_type=%s, irq_type_code=0x%08X",
                __func__, gf_strirq(irq_type), irq_type);
        GF_LOGD(LOG_TAG "[%s] g_milan_chip_handle.cur_chip_state=%d",
                __func__, g_milan_chip_handle.cur_chip_state);

        if ((MODE_FF == g_milan_chip_handle.mode || MODE_IMAGE == g_milan_chip_handle.mode)
                && (1 == g_milan_chip_handle.is_received_fdt_up_for_ff_or_image_mode)
                && (MILAN_STATE_FDT_UP == g_milan_chip_handle.cur_chip_state)) {
            irq_type |= GF_IRQ_FINGER_UP_MASK;
            g_milan_chip_handle.is_received_fdt_up_for_ff_or_image_mode = 0;
        }

        if (irq_type & GF_IRQ_RESET_MASK) {
            reg_data |= milan_get_irq_bit_mask(MILAN_RESET_IRQ);
        }

        if (irq_type & GF_IRQ_TMR_IRQ_MNT_MASK) {
            reg_data |= TMR_IRQ_MNT_MASK;
        }

        if (irq_type & GF_IRQ_FINGER_DOWN_MASK) {
            reg_data |= FDT_IRQ_MASK;
        }

        if (irq_type & GF_IRQ_FINGER_UP_MASK) {
            reg_data |= milan_get_irq_bit_mask(MILAN_FDT_UP_IRQ);
        }

        if (irq_type & GF_IRQ_ONE_FRAME_DONE_MASK) {
            reg_data |= ONE_FRAME_DONE_MASK;
        }

        if (irq_type & GF_IRQ_IMAGE_MASK) {
            reg_data |= DATA_INT_MASK;
        }

        if (irq_type & GF_IRQ_NAV_MASK) {
            reg_data |= DATA_INT_MASK;
        }

        if (irq_type & GF_IRQ_ESD_IRQ_MASK) {
            reg_data |= ESD_INT_MASK;
        }

        if (irq_type & GF_IRQ_ADC_FIFO_FULL_MASK) {
            reg_data |= FIFO_EMPTY_OR_FULL_IN_ADC_TEST_MODE_MASK;
        }

        if (irq_type & GF_IRQ_ADC_FIFO_HALF_MASK) {
            reg_data |= FIFO_DATA_HALF_FULL_IN_ADC_TEST_MODE_MASK;
        }

        if (irq_type & GF_IRQ_FDT_REVERSE_MASK) {
            reg_data |= FDT_IRQ1_MASK;
        }

        if (0 == reg_data) {  // does not need to clear irq
            GF_LOGD(LOG_TAG "[%s] without valid interrupt, irq_type=%s, irq_type_code=0x%08X",
                    __func__, gf_strirq(irq_type), irq_type);
            break;
        }

        MILAN_WRITE_REGISTER(MILAN_REG_IRQ_CTRL2, &reg_data);

        if (reg_data & milan_get_irq_bit_mask(MILAN_RESET_IRQ)) {
            g_milan_chip_handle.cur_chip_state = MILAN_STATE_IDLE;
            GF_LOGI(LOG_TAG "[%s] clear reset IRQ", __func__);
            break;
        }

        /* check update up or down base */
        if (reg_data &
            milan_get_irq_bit_mask(MILAN_FDT_IRQ | MILAN_FDT_UP_IRQ | MILAN_REVERSE_IRQ)) {
            if (MILAN_STATE_FDT_DOWN == g_milan_chip_handle.cur_chip_state) {
                if ((reg_data & milan_get_irq_bit_mask(MILAN_REVERSE_IRQ)) &&
                    (0 == (reg_data & milan_get_irq_bit_mask(MILAN_FDT_IRQ | MILAN_FDT_UP_IRQ)))) {
                    GF_LOGD(LOG_TAG "[%s] FDT_DOWN reverse interrupt, update down base", __func__);

                    milan_update_fdt_down_base();

                    if ((1 == g_milan_chip_handle.data_base_status)
                            || (MODE_IMAGE != g_milan_chip_handle.mode
                                    && MODE_FF != g_milan_chip_handle.mode
                                    && MODE_NAV != g_milan_chip_handle.mode)) {
                        GF_LOGD(LOG_TAG "[%s] re-trigger fdt_down", __func__);

                        if (MODE_NAV == g_milan_chip_handle.mode) {
                            err = milan_set_mode(&(g_milan_chip_handle.config->nav_fdt_down),
                                    CMD_FDT, 0);
                        } else {
                            err = milan_set_mode(&(g_milan_chip_handle.config->fdt_down),
                                    CMD_FDT, 0);
                        }

                        if (GF_SUCCESS != err) {
                            g_milan_chip_handle.cur_chip_state = MILAN_STATE_IDLE;
                            GF_LOGE(LOG_TAG "[%s] set FDT DOWN mode failed", __func__);
                            break;
                        }

                        g_milan_chip_handle.cur_chip_state = MILAN_STATE_FDT_DOWN;
                    } else {
                        GF_LOGD(LOG_TAG "[%s] switch to idle mode", __func__);
                        g_milan_chip_handle.auto_switch_mode_flag = 1;
                        g_milan_chip_handle.cur_chip_state = MILAN_STATE_IDLE;
                    }

                    break;
                } else {
                    // in key mode fdt down,
                    if (g_milan_chip_handle.mode == MODE_KEY) {
                        GF_LOGD(LOG_TAG "[%s] FDT_UP update up base", __func__);
                        milan_update_fdt_up_base(0);
                    }
                }
            } else if (MILAN_STATE_FDT_UP == g_milan_chip_handle.cur_chip_state) {
                GF_LOGD(LOG_TAG "[%s] FDT_UP update down base", __func__);
                milan_update_fdt_down_base();
            }
        }

        if ((GF_IRQ_IMAGE_MASK & irq_type) || (GF_IRQ_NAV_MASK & irq_type)) {
            /**
             * Because now, Navigation feature use IMAGE mode's continue mode. so
             * do nothing here.
             */
            if (MODE_NAV == g_milan_chip_handle.mode) {
                g_milan_chip_handle.cur_chip_state = MILAN_STATE_NAV;
                break;
            }

            if (0 == g_milan_chip_handle.auto_switch_mode_flag) {
                /**
                 * When g_milan_chip_handle.suspend_data_sampling_flag flag is 0,
                 * should trigger rescanning data flow, no matter what mode.
                 */
                if (0 == g_milan_chip_handle.suspend_data_sampling_flag) {
                    uint16_t reg_data_triger = 0x0110;

                    MILAN_WRITE_REGISTER(MILAN_REG_PIXEL_CTRL2, &reg_data_triger);

                    g_milan_chip_handle.cur_chip_state = MILAN_STATE_IMAGE;

                    GF_LOGI(LOG_TAG "[%s] re-trigger scanning data in IMAGE mode", __func__);
                    break;
                }

                if ((MODE_IMAGE == g_milan_chip_handle.mode) ||
                        (MODE_FF == g_milan_chip_handle.mode)) {
                    g_milan_chip_handle.auto_switch_mode_flag = 1;
                    g_milan_chip_handle.next_chip_state = MILAN_STATE_FDT_UP;
                    break;
                }
            }
        }

        if ((MILAN_STATE_FDT_DOWN == g_milan_chip_handle.cur_chip_state
                || MILAN_STATE_FF == g_milan_chip_handle.cur_chip_state)
                && (GF_IRQ_FINGER_DOWN_MASK & irq_type)) {
            g_milan_chip_handle.cur_chip_state = MILAN_STATE_IDLE;
            g_milan_chip_handle.auto_switch_mode_flag = 1;

            if (MODE_IMAGE == g_milan_chip_handle.mode || MODE_FF == g_milan_chip_handle.mode) {
                g_milan_chip_handle.next_chip_state = MILAN_STATE_IMAGE;
                break;
            }

            if (MODE_NAV == g_milan_chip_handle.mode) {
                g_milan_chip_handle.next_chip_state = MILAN_STATE_NAV;
                break;
            }

            if (MODE_KEY == g_milan_chip_handle.mode) {
                g_milan_chip_handle.next_chip_state = MILAN_STATE_FDT_UP;
                break;
            }

            // unknown mode, so reset auto switch mode flag.
            g_milan_chip_handle.auto_switch_mode_flag = 0;
            break;
        }

        if ((MILAN_STATE_FDT_UP == g_milan_chip_handle.cur_chip_state
                || MILAN_STATE_FF == g_milan_chip_handle.cur_chip_state)
                && (GF_IRQ_FINGER_UP_MASK & irq_type)) {
            g_milan_chip_handle.cur_chip_state = MILAN_STATE_IDLE;

            if ((MODE_KEY == g_milan_chip_handle.mode) || (MODE_IMAGE == g_milan_chip_handle.mode)
                    || (MODE_NAV == g_milan_chip_handle.mode)
                    || (MODE_FF == g_milan_chip_handle.mode)) {
                g_milan_chip_handle.auto_switch_mode_flag = 1;
                g_milan_chip_handle.next_chip_state = MILAN_STATE_FDT_DOWN;
                break;
            }
        }
    } while (0);

    GF_LOGD(LOG_TAG "[%s] g_milan_chip_handle.auto_switch_mode_flag=%d", __func__,
            g_milan_chip_handle.auto_switch_mode_flag);

    GF_LOGD(LOG_TAG "[%s] g_milan_chip_handle.cur_chip_state=%d", __func__,
            g_milan_chip_handle.cur_chip_state);

    GF_LOGD(LOG_TAG "[%s] g_milan_chip_handle.next_chip_state=%d", __func__,
            g_milan_chip_handle.next_chip_state);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_hw_get_irq_type(uint32_t *irq_type) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint16_t reg_data = 0;

        *irq_type = 0;

        GF_LOGD(LOG_TAG "[%s] g_milan_chip_handle.mode=%s, g_milan_chip_handle.mode=%d, "
                "g_milan_chip_handle.cur_chip_state=%d", __func__,
                gf_strmode(g_milan_chip_handle.mode), g_milan_chip_handle.mode,
                g_milan_chip_handle.cur_chip_state);

        MILAN_READ_REGISTER(MILAN_REG_IRQ_CTRL3, &reg_data);

        GF_LOGD(LOG_TAG "[%s] IRQ type=0x%04X", __func__, reg_data);

        switch (g_milan_chip_handle.cur_chip_state) {
            case MILAN_STATE_SLEEP: {
                if (reg_data & (~(milan_get_irq_bit_mask(MILAN_RESET_IRQ)))) {
                    GF_LOGI(LOG_TAG "[%s] invalid irq=0x%04X", __func__, reg_data);
                    break;
                }

                break;
            }

            case MILAN_STATE_FF:
            case MILAN_STATE_FDT_DOWN:
            case MILAN_STATE_FDT_UP: {
                if (reg_data & (~(milan_get_irq_bit_mask(MILAN_FDT_IRQ | MILAN_FDT_UP_IRQ |
                        MILAN_RESET_IRQ | MILAN_REVERSE_IRQ)))) {
                    GF_LOGI(LOG_TAG "[%s] invalid irq=0x%04X", __func__, reg_data);
                    break;
                }

                if (reg_data & milan_get_irq_bit_mask(MILAN_FDT_IRQ | MILAN_FDT_UP_IRQ)) {
                    if (MILAN_STATE_FDT_DOWN == g_milan_chip_handle.cur_chip_state
                            || (MILAN_STATE_FF == g_milan_chip_handle.cur_chip_state)) {
                        *irq_type |= GF_IRQ_FINGER_DOWN_MASK;
                        g_milan_chip_handle.sensor_broken.update_direction = 1;
                    } else if (MILAN_STATE_FDT_UP == g_milan_chip_handle.cur_chip_state) {
                        if ((0 == g_milan_chip_handle.is_report_finger_up_for_ff_or_image_mode)
                                && (MODE_FF == g_milan_chip_handle.mode
                                        || MODE_IMAGE == g_milan_chip_handle.mode)) {
                            g_milan_chip_handle.is_report_finger_up_for_ff_or_image_mode = 1;
                            g_milan_chip_handle.is_received_fdt_up_for_ff_or_image_mode = 1;
                        } else {
                            *irq_type |= GF_IRQ_FINGER_UP_MASK;
                        }
                    } else {
                        GF_LOGE(LOG_TAG "[%s] g_milan_chip_handle.cur_chip_state=0x%02X", __func__,
                                g_milan_chip_handle.cur_chip_state);
                    }
                }

                if (reg_data & FDT_IRQ1_MASK) {
                    *irq_type |= GF_IRQ_FDT_REVERSE_MASK;
                }

                break;
            }

            case MILAN_STATE_IMAGE: {
                if (reg_data & (~milan_get_irq_bit_mask(MILAN_DATA_INT_IRQ | MILAN_RESET_IRQ))) {
                    GF_LOGI(LOG_TAG "[%s] invalid irq=0x%04X", __func__, reg_data);
                    break;
                }

                if (reg_data & DATA_INT_MASK) {
                    *irq_type |= GF_IRQ_IMAGE_MASK;
                }

                break;
            }

            case MILAN_STATE_NAV:
            case MILAN_STATE_NAV_BASE: {
                if (reg_data & (~milan_get_irq_bit_mask(MILAN_DATA_INT_IRQ | MILAN_RESET_IRQ))) {
                    GF_LOGI(LOG_TAG "[%s] invalid irq=0x%04X", __func__, reg_data);
                    break;
                }

                if (reg_data & DATA_INT_MASK) {
                    *irq_type |= GF_IRQ_NAV_MASK;
                }

                break;
            }

            case MILAN_STATE_IDLE: {
                if (reg_data & (~milan_get_irq_bit_mask(MILAN_TMR_MNT_IRQ | MILAN_RESET_IRQ))) {
                    GF_LOGI(LOG_TAG "[%s] invalid irq=0x%04X", __func__, reg_data);

                    if (reg_data & DATA_INT_MASK) {
                        *irq_type |= GF_IRQ_IMAGE_MASK;
                        GF_LOGI(LOG_TAG "[%s] For interrupt pin test, chip mode is idle, and host "
                                "set image irq. So idle mode should return image irq", __func__);
                        break;
                    }

                    break;
                }

                if (reg_data & TMR_IRQ_MNT_MASK) {
                    *irq_type |= GF_IRQ_TMR_IRQ_MNT_MASK;
                }
                break;
            }

            default: {
                err = GF_ERROR_BAD_PARAMS;
                GF_LOGE(LOG_TAG "[%s] unknown chip mode=%d", __func__,
                        g_milan_chip_handle.cur_chip_state);
                break;
            }
        }

        if ((GF_SUCCESS == err) && (reg_data & milan_get_irq_bit_mask(MILAN_RESET_IRQ))) {
            GF_LOGI(LOG_TAG "[%s] receive reset irq", __func__);

            if (!MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_K_CHIP_ID)) {
                // set gpio-ctrl [reg:0x0008] as floating status
                reg_data = 0x1111;
                MILAN_WRITE_REGISTER(MILAN_REG_GPIO_CTRL, &reg_data);
            }

            reg_data = 0;
            MILAN_READ_REGISTER(MILAN_REG_CHIP_STATE, &reg_data);

            GF_LOGD(LOG_TAG "[%s] chip state=0x%04X", __func__, reg_data);
            if (reg_data & MODE_IDLE_STATE_MASK) {
                *irq_type |= GF_IRQ_RESET_MASK;

                if (MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_F_CHIP_ID)) {
                    /* write zero into 0x0204, only for first batch GF3208 */
                    reg_data = 0;
                    MILAN_WRITE_REGISTER(MILAN_REG_ESD_DETECT, &reg_data);
                } else {
                    reg_data = 0x0403;
                    MILAN_WRITE_REGISTER(MILAN_REG_CLOCK_DIV_CTRL, &reg_data);
                    milan_enable_spi_bypass_deglitch();
                }
            }
            g_milan_chip_handle.milan_update_dac_register_from_opt(g_milan_chip_handle.otp);
            g_milan_chip_handle.milan_update_config_array_from_opt(g_milan_chip_handle.otp);
        }
    } while (0);

    GF_LOGD(LOG_TAG "[%s] irq_type=%s, irq_type_code=0x%08X",
            __func__, gf_strirq(*irq_type), *irq_type);
    FUNC_EXIT(err);

    return err;
}

static void milan_update_config_arrays(uint16_t register_addr, uint16_t value) {
    VOID_FUNC_ENTER();

    GF_LOGD(LOG_TAG "[%s] register=0x%04X, value=0x%04X", __func__, register_addr, value);

    // update FF mode's configuration.
    MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->ff, register_addr, value)

    // update IMAGE mode's configuration.
    MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->image, register_addr, value)

    // update NAV base mode's configuration.
    MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->nav_base, register_addr, value)

    // update FDT manual mode's configuration
    MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->fdt_manual_down, register_addr,
            value)

    // update FDT down mode's configuration.
    MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->fdt_down, register_addr, value)

    // update NAV FDT down mode's configuration.
    MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->nav_fdt_down,
            register_addr, value)

    // update FDT up mode's configuration.
    MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->fdt_up, register_addr, value)

    // update NAV FDT up mode's configuration.
    MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->nav_fdt_up, register_addr, value)

    // update navigation configuration.
    MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->nav, register_addr, value)

    VOID_FUNC_EXIT();
}

gf_error_t gf_milan_nav_complete(void) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();
    do {
        if (MODE_NAV != g_milan_chip_handle.mode) {
            err = GF_ERROR_GENERIC;
            GF_LOGE(LOG_TAG "[%s] g_milan_chip_handle.mode=%s, g_milan_chip_handle.mode=%d is not "
                    "nav mode", __func__, gf_strmode(g_milan_chip_handle.mode),
                    g_milan_chip_handle.mode);
            break;
        }

        if (MILAN_STATE_NAV != g_milan_chip_handle.cur_chip_state) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] error g_milan_chip_handle.cur_chip_state=%d", __func__,
                    g_milan_chip_handle.cur_chip_state);
            break;
        }

        GF_LOGD(LOG_TAG "[%s] set to fdt up", __func__);
        err = milan_set_mode(&(g_milan_chip_handle.config->nav_fdt_up), CMD_FDT, 0);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] set fdt up failed", __func__);
            break;
        }

        g_milan_chip_handle.cur_chip_state = MILAN_STATE_FDT_UP;
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_hw_get_orientation(uint16_t *orientation, uint16_t *facing) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    do {
        if (NULL == orientation || NULL == facing) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad parameter", __func__);
            break;
        }

        *orientation = g_milan_chip_handle.vendor_param.orientation;
        *facing = g_milan_chip_handle.vendor_param.facing;
    } while (0);
    FUNC_EXIT(err);
    return err;
}

static gf_error_t milan_update_dac_value(uint16_t dac_groups[]) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint16_t reg_data = 0;
        milan_reg_access_t reg = { 0 };

        GF_LOGD(LOG_TAG "[%s] new dac value, dac_groups[0]=0x%04X, dac_groups[1]=0x%04X, "
                "dac_groups[2]=0x%04X, dac_groups[3]=0x%04X", __func__, dac_groups[0],
                dac_groups[1], dac_groups[2], dac_groups[3]);

        MILAN_MK_REG_READ(reg, MILAN_REG_RG_PIXEL_CTRL0, &reg_data);
        err = gf_milan_reg_access(&reg);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] READ reg=0x0220 failed", __func__);
            break;
        }

        reg_data = ((reg_data & 0xe00f) | (dac_groups[0] << 4));
        // set dac_group[0]: reg[0220]
        MILAN_MK_REG_WRITE(reg, MILAN_REG_RG_PIXEL_CTRL0, &reg_data);
        err = gf_milan_reg_access(&reg);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] write reg=0x0220 failed", __func__);
            break;
        }

        if (MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_K_CHIP_ID) ||
                MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_J_CHIP_ID) ||
                MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_H_CHIP_ID)) {
            // set dac_group[1]: reg[0236]
            MILAN_MK_REG_WRITE(reg, MILAN_REG_RG_PIXEL_DAC_P2, &dac_groups[1]);
            err = gf_milan_reg_access(&reg);
            if (err != GF_SUCCESS) {
                GF_LOGE(LOG_TAG "[%s] write reg=0x0236 failed", __func__);
                break;
            }

            // set dac_group[2]: reg[0238]
            MILAN_MK_REG_WRITE(reg, MILAN_REG_RG_PIXEL_DAC_P3, &dac_groups[2]);
            err = gf_milan_reg_access(&reg);
            if (err != GF_SUCCESS) {
                GF_LOGE(LOG_TAG "[%s] write reg=0x0238 failed", __func__);
                break;
            }

            // set dac_group[3]: reg[023a]
            MILAN_MK_REG_WRITE(reg, MILAN_REG_RG_PIXEL_DAC_P4, &dac_groups[3]);
            err = gf_milan_reg_access(&reg);
            if (err != GF_SUCCESS) {
                GF_LOGE(LOG_TAG "[%s] write reg=0x023A failed", __func__);
                break;
            }
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t gf_milan_set_sensor_dac_value(uint16_t dac_groups[], uint32_t is_trigger_image) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint16_t reg_data = 0;
        milan_reg_access_t reg = { 0 };

        err = milan_update_dac_value(dac_groups);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] update dac value failed", __func__);
            break;
        }

        if (is_trigger_image) {
            // trigger image capture
            reg_data = 0x0110;
            MILAN_MK_REG_WRITE(reg, MILAN_REG_PIXEL_CTRL2, &reg_data);
            err = gf_milan_reg_access(&reg);
            if (GF_SUCCESS != err) {
                GF_LOGE(LOG_TAG "[%s] write trigger cmd failed", __func__);
                break;
            }
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_milan_update_dac_for_sensor_broken_check(uint32_t is_set_default_dac,
        uint32_t is_trigger_image) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (0 == g_milan_chip_handle.sensor_broken.set_flag) {
            GF_LOGE(LOG_TAG "[%s] sensor broken check is disabled", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        {
            // clear irq
            uint16_t reg_data = 0;
            milan_reg_access_t reg = { 0 };

            reg_data = 0xFFFF;
            MILAN_MK_REG_WRITE(reg, MILAN_REG_IRQ_CTRL2, &reg_data);
            err = gf_milan_reg_access(&reg);
            if (err != GF_SUCCESS) {
                GF_LOGE(LOG_TAG "[%s] write clear irq error", __func__);
                break;
            }
        }

        // update dac
        if (1 == is_set_default_dac) {
            // default DAC
            gf_milan_set_sensor_dac_value(g_milan_chip_handle.default_dac, is_trigger_image);
        } else {
            if (g_milan_chip_handle.sensor_broken.update_direction) {
                // first refs raw data
                gf_milan_set_sensor_dac_value(g_milan_chip_handle.sensor_broken.negative_dac,
                        is_trigger_image);  // negative dac value
            } else {
                // second refs raw data
                gf_milan_set_sensor_dac_value(g_milan_chip_handle.sensor_broken.positive_dac,
                        is_trigger_image);  // positive dac value
            }

            g_milan_chip_handle.sensor_broken.update_direction =
                    !g_milan_chip_handle.sensor_broken.update_direction;
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_milan_start_sensor_broken_check(void) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (1 == g_milan_chip_handle.sensor_broken.set_flag) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] need to stop first", __func__);
            break;
        }

        g_milan_chip_handle.sensor_broken.set_flag = 1;
        g_milan_chip_handle.sensor_broken.update_direction = 1;
    } while (0);
    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_milan_stop_sensor_broken_check(void) {
    gf_error_t err;
    FUNC_ENTER();

    if (1 == g_milan_chip_handle.sensor_broken.set_flag) {
        cpl_memset(&(g_milan_chip_handle.sensor_broken), 0,
                sizeof(g_milan_chip_handle.sensor_broken));
    }
    err = GF_SUCCESS;
    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_milan_update_dac_for_sensor_pixel_open_test(uint32_t is_set_default_dac,
        uint32_t is_trigger_image) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (1 == is_set_default_dac) {
            err = gf_milan_set_sensor_dac_value(g_milan_chip_handle.default_dac, is_trigger_image);
        } else {
            err = gf_milan_set_sensor_dac_value(
                    g_milan_chip_handle.pixel_test_chip_info.positive_dac, is_trigger_image);
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_milan_detect_finger_up(void) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        g_milan_chip_handle.mode = MODE_KEY;

        if (MILAN_STATE_FDT_UP == g_milan_chip_handle.cur_chip_state) {
            GF_LOGI(LOG_TAG "[%s] current chip mode is FDT UP", __func__);
            break;
        }

        err = milan_set_mode(&(g_milan_chip_handle.config->fdt_up), CMD_FDT, 0);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] switch to fdt up mode failed", __func__);
            break;
        }

        g_milan_chip_handle.cur_chip_state = MILAN_STATE_FDT_UP;
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t milan_update_fdt_down_base(void) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint32_t i = 0;
        uint16_t fdt_down_base[MILAN_FDT_BASE_DATA_LEN] = { 0 };
        uint16_t fdt_thres_offset = MILAN_REG_FDT_THRES
                - g_milan_chip_handle.config->fdt_down.start_address;

        /* read FDT 24 bytes area average data  */
        err = gf_milan_secspi_read_bytes(MILAN_REG_FDT_AVG, (uint8_t*) fdt_down_base,
                sizeof(fdt_down_base));  // MILAN_REG_FDT_AVG:0x00A2
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] read fdt base data failed", __func__);
            break;
        }

        gf_hw_endian_exchange((uint8_t *) fdt_down_base, (int32_t) sizeof(fdt_down_base));

        for (i = 0; i < MILAN_FDT_BASE_DATA_LEN; i++) {
            uint8_t value = (fdt_down_base[i] & 0xFFF) >> 1;
            uint32_t offset = fdt_thres_offset + i * 2;

            g_milan_chip_handle.config->ff.data[offset] = value;
            g_milan_chip_handle.config->fdt_down.data[offset] = value;
            g_milan_chip_handle.config->fdt_manual_down.data[offset] = value;
            g_milan_chip_handle.config->nav_fdt_down.data[offset] = value;

            GF_LOGD(LOG_TAG "[%s] fdt_down_base[%d]=0x%02X", __func__, i, value);
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t milan_update_fdt_up_base(int32_t second_update_flag) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    GF_LOGD(LOG_TAG "[%s] second update=%d", __func__, second_update_flag);

    do {
        uint16_t reg_data = 0;
        uint8_t fdt_delta_down = 0, fdt_delta_up = 0;
        uint32_t i = 0;
        uint16_t fdt_up_base[MILAN_FDT_BASE_DATA_LEN] = { 0 };
        uint16_t fdt_up_base_second[MILAN_FDT_BASE_DATA_LEN] = { 0 };
        uint16_t fdt_touch_flag1 = 0;
        uint16_t fdt_thres_offset = MILAN_REG_FDT_THRES
                - g_milan_chip_handle.config->fdt_up.start_address;
        uint32_t cur_chip_state = g_milan_chip_handle.cur_chip_state;

        if (FDT_UP_DETECT_MODE_NEW_VERSION == g_milan_chip_handle.fdt_up_detect_new_mode_flag) {
            /* donot add delta down for Milan_KJH */
            fdt_delta_down = 0;
            fdt_delta_up = 0;
        } else {
            MILAN_GET_REGISTER_FROM_CONFIG_ARRAY_BY_BYTE(g_milan_chip_handle.config->fdt_down,
                    MILAN_REG_FDT_DELTA, fdt_delta_down);
            GF_LOGI(LOG_TAG "[%s] FDT down delta=0x%02X", __func__, fdt_delta_down);

            MILAN_GET_REGISTER_FROM_CONFIG_ARRAY_BY_BYTE(g_milan_chip_handle.config->fdt_up,
                    MILAN_REG_FDT_DELTA, fdt_delta_up);
            GF_LOGI(LOG_TAG "[%s] FDT UP delta=0x%02X", __func__, fdt_delta_up);
        }

        /* read FDT 24 bytes area average data  */
        err = gf_milan_secspi_read_bytes(MILAN_REG_FDT_AVG, (uint8_t*) fdt_up_base,
                sizeof(fdt_up_base));  // MILAN_REG_FDT_AVG:0x00A2
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] read FDT UP base failed", __func__);
            break;
        }
        gf_hw_endian_exchange((uint8_t *) fdt_up_base, (int32_t) sizeof(fdt_up_base));

        for (i = 0; i < MILAN_FDT_BASE_DATA_LEN; ++i) {
            fdt_up_base[i] = ((fdt_up_base[i] & 0xFFF) >> 1) + fdt_delta_down;
            GF_LOGD(LOG_TAG "[%s] fdt_up_base[%d]=0x%04X", __func__, i,
                    fdt_up_base[i]);
        }

        /**
         * Read register(00C8) to check which area is touch 0
         */
        MILAN_READ_REGISTER(MILAN_REG_TOUCH_FLAG0, &reg_data);
        fdt_touch_flag1 = reg_data;
        GF_LOGD(LOG_TAG "[%s] fdt_touch_flag1 register(00C8)=0x%04X", __func__, reg_data);

        if (second_update_flag == 0) {
            /* set the base of non-touched block to fdt_threshold of up */
            for (i = 0; i < MILAN_FDT_BASE_DATA_LEN; ++i) {
                if (0 == (reg_data & (1 << i))) {
                    fdt_up_base[i] = fdt_delta_up;
                }
            }
        } else {
            uint16_t fdt_touch_flag2 = 0;
            /* check fdt up base 2 times, after got image
             * accroding wet finger update fdt up base process
             */
            err = milan_read_fdt_base(fdt_up_base_second);
            // revert cur_chip_state
            g_milan_chip_handle.cur_chip_state = cur_chip_state;
            if (err != GF_SUCCESS) {
                GF_LOGE(LOG_TAG "[%s] get fdt base failed", __func__);
                break;
            }
            // read fdt_touch_flag2
            MILAN_READ_REGISTER(MILAN_REG_TOUCH_FLAG0, &reg_data);

            fdt_touch_flag2 = reg_data;
            GF_LOGD(LOG_TAG "[%s] fdt_touch_flag2 register(0x00C8)=0x%02X", __func__, reg_data);
            reg_data = fdt_touch_flag1 | fdt_touch_flag2;

            for (i = 0; i < MILAN_FDT_BASE_DATA_LEN; i++) {
                if (0 == (reg_data & (1 << i))) {
                    fdt_up_base[i] = fdt_delta_up;
                } else {
                    fdt_up_base_second[i] += fdt_delta_down;
                    if (fdt_up_base_second[i] < fdt_up_base[i]) {
                        fdt_up_base[i] = fdt_up_base_second[i];
                    }
                }
            }
        }

        for (i = 0; i < MILAN_FDT_BASE_DATA_LEN; i++) {
            uint16_t offset = fdt_thres_offset + i * 2;

            g_milan_chip_handle.config->fdt_up.data[offset] = fdt_up_base[i];
            g_milan_chip_handle.config->nav_fdt_up.data[offset] = fdt_up_base[i];

            GF_LOGD(LOG_TAG "[%s] fdt_up_base[%d]=0x%02X", __func__, i,
                    fdt_up_base[i]);
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t milan_read_fdt_base(uint16_t *fdt_base) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint16_t reg_data = 0;
        milan_config_unit_t *config = NULL;

        if (NULL == fdt_base) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] g_milan_chip_handle.cur_chip_state=%d, bad parameter", __func__,
                    g_milan_chip_handle.cur_chip_state);
            break;
        }

        // err = milan_set_mode(&(g_milan_chip_handle.config->fdt_manual_down), CMD_IDLE, 0);

        config = &(g_milan_chip_handle.config->fdt_manual_down);
        err = gf_milan_secspi_write_cmd(CMD_IDLE);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] write cmd failed, cmd=0x%02X", __func__, CMD_IDLE);
            break;
        }
        g_milan_chip_handle.cur_chip_state = MILAN_STATE_IDLE;

        /* MILAN need 1ms to switch mode when IDLE  */
        gf_sleep(1);
        err = gf_milan_secspi_write_bytes(config->start_address, config->data,
                                          config->data_len);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] write configuration failed", __func__);
            break;
        }

        /* set fdt command */
        err = gf_milan_secspi_write_cmd(CMD_FDT);  // 0xC1
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] write fdt cmd failed", __func__);
            break;
        }
        g_milan_chip_handle.cur_chip_state = MILAN_STATE_FDT_DOWN;

        gf_sleep(1);

        /* trigger fdt manual */
        err = gf_milan_secspi_write_cmd(CMD_TMR_TRG);  // 0xA1
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] trigger fdt manual failed", __func__);
            break;
        }

        /* delay 6ms for 8 * 8 area, 3ms for 1 * 8 ms*/
        MILAN_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->fdt_manual_down,
                MILAN_REG_PIXEL_CTRL4, reg_data);
        if (FDT_DETECT_AREA_IS_1X8(reg_data)) {
            gf_sleep(3);
            GF_LOGD(LOG_TAG "[%s] FDT manual down detect area=1X8", __func__);
        } else {
            gf_sleep(6);
            GF_LOGD(LOG_TAG "[%s] FDT manual down detect area=8X8", __func__);
        }

        /* switch to idle mode */
        err = gf_milan_secspi_write_cmd(CMD_IDLE);  // 0xC0
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] switch to idle mode failed", __func__);
            break;
        }
        g_milan_chip_handle.cur_chip_state = MILAN_STATE_IDLE;

        gf_sleep(1);

        /* read FDT 24 bytes area average data  */
        err = gf_milan_secspi_read_bytes(MILAN_REG_FDT_AVG, (uint8_t*) fdt_base,
        MILAN_FDT_BASE_DATA_LEN * sizeof(uint16_t));  // MILAN_REG_FDT_AVG:0x00A2
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] read fdt base data failed", __func__);
            break;
        }

        /* read block size */
        MILAN_READ_REGISTER(MILAN_REG_PIXEL_CTRL4, &reg_data);
        // MILAN_REG_PIXEL_CTRL4:0x0058

        /* MILAN_REG_PIXEL_CTRL4 bit8 */
        reg_data &= 0x100;

        {
            uint32_t i = 0;
            uint32_t offset_bits = (reg_data != 0) ? 4 : 1;

            gf_hw_endian_exchange((uint8_t *) fdt_base, MILAN_FDT_BASE_DATA_LEN * sizeof(uint16_t));

            for (i = 0; i < MILAN_FDT_BASE_DATA_LEN; ++i) {
                fdt_base[i] = (fdt_base[i] & 0xFFF) >> offset_bits;
                GF_LOGD(LOG_TAG "[%s] fdt_base[%d]=0x%02X", __func__, i, fdt_base[i]);
            }
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_milan_get_fdt_base(uint16_t **buf, uint32_t *length) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        cpl_memset(g_fdt_base_data, 0,
                   sizeof(g_fdt_base_data));

        err = milan_read_fdt_base((uint16_t *) g_fdt_base_data);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] get FDT base failed", __func__);
            break;
        }

        *buf = g_fdt_base_data;
        *length = MILAN_FDT_BASE_DATA_LEN;
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t milan_check_finger_up(void) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint32_t i = 0;
        uint16_t fdt_average_data[MILAN_FDT_BASE_DATA_LEN] = { 0 };
        uint16_t fdt_thres_offset = MILAN_REG_FDT_THRES
                - g_milan_chip_handle.config->fdt_down.start_address;
        uint16_t fdt_up_delta = 0;

        err = milan_read_fdt_base((uint16_t *) fdt_average_data);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] get FDT average data failed", __func__);
            break;
        }

        MILAN_GET_REGISTER_FROM_CONFIG_ARRAY_BY_BYTE(g_milan_chip_handle.config->fdt_up,
                MILAN_REG_FDT_DELTA, fdt_up_delta);

        GF_LOGD(LOG_TAG "[%s] delta_up=0x%02X", __func__, fdt_up_delta);
        for (i = 0; i < MILAN_FDT_BASE_DATA_LEN; i++) {
            if (g_milan_chip_handle.config->fdt_down.data[i * 2 + fdt_thres_offset] - fdt_up_delta
                    >= fdt_average_data[i]) {
                GF_LOGD(LOG_TAG "[%s] fdt_average_data[%d]=0x%04X", __func__, i,
                        fdt_average_data[i]);
                GF_LOGD(LOG_TAG "[%s] g_milan_chip_handle.config->fdt_down.data[%d]=0x%04X",
                        __func__, i * 2 + fdt_thres_offset,
                        g_milan_chip_handle.config->fdt_down.data[i * 2 + fdt_thres_offset]);
                GF_LOGI(LOG_TAG "[%s] area[%d] is finger down", __func__, i);
                break;
            }
        }

        if (i < MILAN_FDT_BASE_DATA_LEN) {
            err = GF_ERROR_FINGER_NOT_LEAVE;
            GF_LOGE(LOG_TAG "[%s] finger is not leave, i=%u", __func__, i);
            break;
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_milan_get_fdt_threshold(uint16_t* threshold) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        milan_reg_access_t reg = { 0 };
        uint16_t reg_data = 0, fdt_threshold = 0;

        if (NULL == threshold) {
            GF_LOGE(LOG_TAG "[%s] invalid parameters", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        /* average threshold delta */
        MILAN_MK_REG_READ(reg, MILAN_REG_FDT_DELTA, &reg_data);
        err = gf_milan_reg_access(&reg);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] read fdt delta", __func__);
            break;
        }

        /* MILAN_REG_FDT_DELTA[15:8] */
        fdt_threshold = (reg_data >> 8);

        *threshold = fdt_threshold;
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_milan_switch_mode(void) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        GF_LOGD(LOG_TAG "[%s] g_milan_chip_handle.auto_switch_mode_flag=%d", __func__,
                g_milan_chip_handle.auto_switch_mode_flag);

        GF_LOGD(LOG_TAG "[%s] g_milan_chip_handle.suspend_data_sampling_flag=%d", __func__,
                g_milan_chip_handle.suspend_data_sampling_flag);

        GF_LOGD(LOG_TAG "[%s] g_milan_chip_handle.cur_chip_state=%d, next_chip_state=%d", __func__,
                g_milan_chip_handle.cur_chip_state, g_milan_chip_handle.next_chip_state);

        if (0 == g_milan_chip_handle.auto_switch_mode_flag) {
            GF_LOGD(LOG_TAG "[%s] don't do mode auto switch", __func__);
            break;
        }
        switch (g_milan_chip_handle.next_chip_state) {
            case MILAN_STATE_FDT_UP: {
                if (MODE_NAV == g_milan_chip_handle.mode) {
                    err = milan_set_mode(&(g_milan_chip_handle.config->nav_fdt_up), CMD_FDT, 0);
                } else {
                    err = milan_set_mode(&(g_milan_chip_handle.config->fdt_up), CMD_FDT, 0);
                }
                if (GF_SUCCESS != err) {
                    GF_LOGE(LOG_TAG "[%s] set FDT UP mode failed", __func__);
                    break;
                }

                g_milan_chip_handle.cur_chip_state = MILAN_STATE_FDT_UP;

                break;
            }

            case MILAN_STATE_FDT_DOWN: {
                if (MODE_NAV == g_milan_chip_handle.mode) {
                    err = milan_set_mode(&(g_milan_chip_handle.config->nav_fdt_down), CMD_FDT, 0);
                } else {
                    err = milan_set_mode(&(g_milan_chip_handle.config->fdt_down), CMD_FDT, 0);
                }

                if (GF_SUCCESS != err) {
                    GF_LOGE(LOG_TAG "[%s] set FDT DOWN mode failed", __func__);
                    break;
                }

                g_milan_chip_handle.cur_chip_state = MILAN_STATE_FDT_DOWN;
                break;
            }

            case MILAN_STATE_IMAGE: {
                err = milan_set_image_mode();
                if (GF_SUCCESS != err) {
                    GF_LOGE(LOG_TAG "[%s] set image mode failed", __func__);
                    break;
                }

                GF_LOGI(LOG_TAG "[%s] success to set IMAGE mode", __func__);

                g_milan_chip_handle.cur_chip_state = MILAN_STATE_IMAGE;
                break;
            }

            case MILAN_STATE_NAV: {
                err = milan_set_nav_mode();
                if (GF_SUCCESS != err) {
                    GF_LOGE(LOG_TAG "[%s] set nav mode failed", __func__);
                    break;
                }

                GF_LOGI(LOG_TAG "[%s] success to set NAV mode", __func__);

                g_milan_chip_handle.cur_chip_state = MILAN_STATE_NAV;
                break;
            }

            default: {
                err = GF_ERROR_GENERIC;
                GF_LOGE(LOG_TAG "[%s] invalid next chip mode=%d", __func__,
                        g_milan_chip_handle.next_chip_state);
                break;
            }
        }
    } while (0);

    g_milan_chip_handle.auto_switch_mode_flag = 0;

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_milan_set_data_base_status(uint8_t is_data_base_ok) {
    g_milan_chip_handle.data_base_status = is_data_base_ok;

    return GF_SUCCESS;
}

static gf_error_t milan_init_fdt_params_from_chip_config(gf_fdt_params_t* init_param) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint16_t value = 0;
        if ((GF_MILAN_F_SERIES != g_config.chip_series) || (NULL == init_param)) {
            err = GF_ERROR_GENERIC;
            GF_LOGE(LOG_TAG "[%s] it must be milan_f_series chip", __func__);
            break;
        }

        /* 0xca[7:0] */
        value = 0;
        MILAN_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->fdt_down,
            MILAN_REG_FDT_AREA_NUM, value);
        init_param->fdt_area_num = (value & 0xff);
        GF_LOGD(LOG_TAG "[%s] fdt_area_num=0x%02X", __func__, init_param->fdt_area_num);

         /* 0x5c[12:0] */
        value = 0;
        MILAN_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->fdt_down,
            MILAN_REG_PIXEL_CTRL6, value);
        init_param->fdt_tx_num = (value & 0xfff);
        GF_LOGD(LOG_TAG "[%s] fdt_tx_num=0x%02X", __func__, init_param->fdt_tx_num);

        /* 0x82[15:8] */
        value = 0;
        MILAN_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->fdt_down,
            MILAN_REG_FDT_DELTA, value);
        init_param->fdt_threshold = (value >> 8);
        GF_LOGD(LOG_TAG "[%s] fdt_threshold=0x%02X", __func__, init_param->fdt_threshold);

         /* 0x5c[12:0] */
        value = 0;
        MILAN_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->image,
            MILAN_REG_PIXEL_CTRL6, value);
        init_param->image_tx_num = (value & 0xfff);
        GF_LOGD(LOG_TAG "[%s] image_tx_num=0x%02X", __func__, init_param->image_tx_num);

        /* 0x5c[12:0] */
        value = 0;
        MILAN_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->nav,
            MILAN_REG_PIXEL_CTRL6, value);
        init_param->nav_tx_num = (value & 0xfff);
        GF_LOGD(LOG_TAG "[%s] nav_tx_num:0x%2x", __func__, init_param->nav_tx_num);

        /* main config paramters about MilanF is set manually */
        if (!MILAN_CHIP_ID_IS_EQUAL(g_milan_chip_handle.chip_id, MILAN_F_CHIP_ID)) {
            value = 0;
            MILAN_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->nav,
                MILAN_REG_PIXEL_CTRL8, value);
            /* 0x62[7:0] */
            init_param->nav_row_start = (value & 0xff);
            GF_LOGD(LOG_TAG "[%s] nav_row_start=0x%02X", __func__, init_param->nav_row_start);

             /* 0x62[15:8] */
            init_param->nav_row_step = (value >> 8);
             GF_LOGD(LOG_TAG "[%s] nav_row_step=0x%02X", __func__, init_param->nav_row_step);

            /* 0x64[7:0] */
            value = 0;
            MILAN_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(g_milan_chip_handle.config->nav,
                MILAN_REG_PIXEL_CTRL9, value);
            init_param->nav_row_num = (value & 0xff);
            GF_LOGD(LOG_TAG "[%s] nav_row_num=0x%02X", __func__, init_param->nav_row_num);
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_hw_read_register(uint32_t addr, uint8_t* content, uint32_t read_len) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (content == NULL) {
            GF_LOGE(LOG_TAG "[%s] content container is null", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
        if (read_len > GF_MAX_SPI_RW_LEN) {
            GF_LOGE(LOG_TAG "[%s] illegal buffer size", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        err = gf_milan_secspi_read_bytes((uint16_t)addr, content, read_len);
        // gf_hw_endian_exchange(temp_buffer, read_len);

        GF_LOGD(
            LOG_TAG "[%s] READ 0x%04x(%d bytes)", __func__, addr, read_len);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_hw_write_register(uint32_t addr, uint8_t* content, uint32_t write_len) {
    gf_error_t err = GF_SUCCESS;
    uint32_t i = 0;

    FUNC_ENTER();

    do {
        if (content == NULL) {
            GF_LOGE(LOG_TAG "[%s] content container is null", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
        if (write_len > GF_MAX_SPI_RW_LEN) {
            GF_LOGE(LOG_TAG "[%s] illegal buffer size", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        for (i = 0; i< write_len; i++) {
            GF_LOGD(LOG_TAG "[%s] WRITE 0x%02X ", __func__, content[i]);
        }
        // gf_hw_endian_exchange(temp_buffer, write_len);
        err = gf_milan_secspi_write_bytes((uint16_t)addr, content, write_len);
        GF_LOGD(
            LOG_TAG "[%s] WRITE 0x%04x(%d bytes)", __func__, addr, write_len);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_milan_get_spi_speed(uint32_t irq_type, uint32_t *speed) {
    gf_error_t ret = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL== speed) {
            ret = GF_ERROR_GENERIC;
            break;
        }

        *speed = GF_SPI_SPEED_LOW;
        if (GF_IRQ_NAV_MASK == irq_type) {
             *speed = g_milan_chip_handle.nav_fifo_spi_rate;
        }

        if (GF_IRQ_IMAGE_MASK == irq_type) {
            *speed = g_milan_chip_handle.img_fifo_spi_rate;
        }
    }while(0);

    FUNC_EXIT(ret);
    return ret;
}

gf_error_t gf_milan_host_irq_set(void) {
    gf_error_t err = GF_SUCCESS;
    uint16_t reg_data = 0xFFFF;

    FUNC_ENTER();

    do {
        MILAN_WRITE_REGISTER(MILAN_REG_IRQ_CTRL2, &reg_data);

        reg_data = 0x0008;
        MILAN_WRITE_REGISTER(MILAN_REG_IRQ_CTRL0, &reg_data);

        MILAN_WRITE_REGISTER(MILAN_REG_IRQ_CTRL4, &reg_data);
    } while (0);

    FUNC_EXIT(err);

    return err;
}
