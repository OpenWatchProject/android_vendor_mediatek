/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#include "gf_tee_internal_api.h"
#include "gf_common.h"
#include "gf_config.h"
#include "gf_error.h"
#include "gf_fw_cfg.h"
#include "gf_sensor.h"
#include "GF11_Rawdata_InvCipher.h"

#include "gf_spi.h"
#include "gf_hw_common.h"
#include "gf_oswego_m_bus.h"
#include "gf_oswego_m.h"
#include "gf_oswego_m_download.h"
#include "cpl_string.h"
#include "cpl_memory.h"
#include "gf_status.h"

#define LOG_TAG "[gf_oswego_m]"

#define GF_OSWEGO_M_FW_VERSION_REG          (0x8000)
#define GF_OSWEGO_M_CHIP_WORK_MODE_REG      (0x8018)
#define GF_OSWEGO_M_ESD_REG                 (0x8040)
#define GF_OSWEGO_M_MODE_REG                (0x8043)
#define GF_OSWEGO_M_SAMPLING_PAUSE_REG      (0x8044)
#define GF_OSWEGO_M_BUFFER_STATUS_H_REG     (0x8140)
#define GF_OSWEGO_M_BUFFER_STATUS_L_REG     (0x8141)
#define GF_OSWEGO_M_NAV_FRAME_NUM_REG       (0x8146)
#define GF_OSWEGO_M_NAV_BUF0_REG            (0x8148)
#define GF_OSWEGO_M_NAV_BUF0_CHECKSUM_REG   (0x9a9a)
#define GF_OSWEGO_M_NAV_BUF1_REG            (0x9b48)
#define GF_OSWEGO_M_NAV_BUF1_CHECKSUM_REG   (0xb49a)

#define GF_IRQ_STATUS_H_ENABLE          (1 << 7)
#define GF_IRQ_STATUS_H_IMAGE           (1 << 6)
#define GF_IRQ_STATUS_H_HOME_KEY_ENABLE (1 << 5)
#define GF_IRQ_STATUS_H_HOME_KEY_STATUS (1 << 4)
#define GF_IRQ_STATUS_H_MENU_KEY_ENABLE (1 << 3)
#define GF_IRQ_STATUS_H_MENU_KEY_STATUS (1 << 2)
#define GF_IRQ_STATUS_H_BACK_KEY_ENABLE (1 << 1)
#define GF_IRQ_STATUS_H_BACK_KEY_STATUS (1 << 0)
#define GF_IRQ_STATUS_L_RESET           (1 << 7)
#define GF_IRQ_STATUS_L_NAV_IMAGE       (1 << 6)

static const uint8_t g_mode_array[] = {
        0x00, /* MODE_IMAGE */
        0x01, /* MODE_KEY */
        0x02, /* MODE_SLEEP */
        0x03, /* MODE_FF */
        0x10, /* MODE_NAV */
        0x11, /* MODE_NAV_BASE */
        0x56, /* MODE_DEBUG */
        0x58, /* MODE_FINGER_BASE */
        0X60, /* MODE_IDLE */
        };

#define GFX18M_RAWDATA_LEN 13824
#define GFX16M_RAWDATA_LEN 13216

typedef struct {
    uint8_t status_h;
    uint8_t status_l;
    uint8_t ring_h;
    uint8_t ring_l;
    uint8_t noise_h;
    uint8_t noise_l;
    uint8_t ring_long_press_stdp_h;
    uint8_t ring_long_press_stdp_l;
    uint8_t data[GFX16M_RAWDATA_LEN];
    uint8_t checksum_h;
    uint8_t checksum_l;
    uint8_t reversed[12];
}__attribute__((packed)) gfx16m_rawdata_t;

typedef struct {
    uint8_t status_h;
    uint8_t status_l;
    uint8_t ring_h;
    uint8_t ring_l;
    uint8_t noise_h;
    uint8_t noise_l;
    uint8_t ring_long_press_stdp_h;
    uint8_t ring_long_press_stdp_l;
    uint8_t data[GFX18M_RAWDATA_LEN];
    uint8_t checksum_h;
    uint8_t checksum_l;
    uint8_t reversed[12];
}__attribute__((packed)) gfx18m_rawdata_t;

#define GF_OSWEGO_M_PRODUCT_INFO_LEN 64
#define GF_OSWEGO_M_VENDOR_ID_LEN    4
#define GF_OSWEGO_M_CHIP_TYPE_LEN    2
#define GF_OSWEGO_M_DATE_LEN         3

typedef struct {
    uint8_t vendor_id[GF_OSWEGO_M_VENDOR_ID_LEN];
    uint8_t product_cfg_idx;
    uint8_t chip_type[GF_OSWEGO_M_CHIP_TYPE_LEN];
    uint8_t smt_test_error_code;
    uint8_t module_test1_error_code;
    uint8_t module_test2_error_code;
    uint8_t key_touch_test_error_code;
    uint8_t date[GF_OSWEGO_M_DATE_LEN];
    uint8_t silkscreen_flag;
} gf_product_info_t;

typedef struct {
    uint32_t line_len;
    uint32_t line_aligned_len;
    uint32_t sensor_raw_data_len;
    uint32_t nav_frame_len;
} gf_oswego_buf_len_t;

static uint8_t *g_rawdata = NULL;
static gf_product_info_t g_product_info = { { 0 }, 0, { 0 }, 0, 0, 0, 0, { 0 }, 0 };
static gf_oswego_m_cfg_data_t *g_cfg_data = NULL;
static gf_oswego_buf_len_t g_buf_len_info = { 0 };

static uint32_t g_last_irq_status = 0;
static gf_mode_t g_cur_chip_mode = MODE_IDLE;

static uint8_t g_pre_nav_frame_status[6] = { 0 };
static uint8_t *g_pre_nav_frame = NULL;
static uint8_t g_pre_nav_frame_num = 0;
static uint8_t g_pre_nav_over_flag = 0;

static uint8_t* g_nav_base_nav_frame = NULL;
static uint16_t *g_gf_raw_data_tmp = NULL;  // tmp buffer

static uint8_t checksum_nav_base[2] = { 0 };
static uint16_t g_fw_version = 0;

uint64_t g_exit_ff_mode_time = 0;
uint32_t g_check_ff_mode_wakeup_flag = 0;

/**
 * [gf_hw_init_global_variable]:Initialize the global variable in this file
 */
void gf_hw_init_global_variable(void) {
    VOID_FUNC_ENTER();

    cpl_memset(&g_product_info, 0, sizeof(gf_product_info_t));
    cpl_memset(&g_buf_len_info, 0, sizeof(gf_oswego_buf_len_t));

    g_last_irq_status = 0;
    g_cur_chip_mode = MODE_IDLE;

    cpl_memset(g_pre_nav_frame_status, 0, sizeof(g_pre_nav_frame_status));
    g_fw_version = 0;
    g_exit_ff_mode_time = 0;
    g_check_ff_mode_wakeup_flag = 0;

    gf_oswego_m_bus_init_global_variable();

    VOID_FUNC_EXIT();
}

static gf_error_t oswego_m_raw_data_split(uint16_t *out, uint8_t *raw_data) {
    gf_error_t ret = GF_SUCCESS;
    uint8_t *src = raw_data;
    uint16_t *dst = g_gf_raw_data_tmp;
    uint8_t tmp[3] = { 0 };
    uint32_t i = 0;
    uint32_t j = 0;
    FUNC_ENTER();
    if (g_sensor.raw_data_is_encrypt > 0) {
        /* length should be multiple of 12  */
        GF11_Rawdata_AES_InvCipher(raw_data, (g_buf_len_info.sensor_raw_data_len + 11) / 12 * 12,
                0);
    }

    for (j = 0; j < g_buf_len_info.sensor_raw_data_len; j += g_buf_len_info.line_aligned_len) {
        for (i = 0; i < g_buf_len_info.line_len; i += 3) {
            tmp[0] = *src++;
            tmp[1] = *src++;
            tmp[2] = *src++;

            *dst++ = (tmp[0] << 4) + ((tmp[1] & 0xF0) >> 4);
            *dst++ = ((tmp[1] & 0x0F) << 8) + tmp[2];
        }
        src += (g_buf_len_info.line_aligned_len - g_buf_len_info.line_len);
    }

    for (i = 0; i < g_sensor.row; i++) {
        for (j = 0; j < g_sensor.col; j++) {
            *out++ = g_gf_raw_data_tmp[j * g_sensor.row + i];
        }
    }
    FUNC_EXIT(ret);
    return ret;
}

static gf_error_t oswego_m_nav_raw_data_split(uint16_t *out, uint8_t *raw_data, uint8_t frame_num) {
    gf_error_t ret = GF_SUCCESS;

    uint8_t *src = raw_data;
    uint16_t *dst = g_gf_raw_data_tmp;
    uint8_t tmp[3] = { 0 };
    uint32_t i = 0;
    uint32_t j = 0;
    uint32_t n = 0;
    uint32_t len = 0;

    FUNC_ENTER();
    len = g_buf_len_info.nav_frame_len * frame_num;

    for (j = 0; j < len; j += g_buf_len_info.line_aligned_len) {
        for (i = 0; i < g_buf_len_info.line_len; i += 3) {
            tmp[0] = *src++;
            tmp[1] = *src++;
            tmp[2] = *src++;

            *dst++ = (tmp[0] << 4) + ((tmp[1] & 0xF0) >> 4);
            *dst++ = ((tmp[1] & 0x0F) << 8) + tmp[2];
        }
        src += (g_buf_len_info.line_aligned_len - g_buf_len_info.line_len);
    }

    for (n = 0; n < frame_num; n++) {
        uint16_t offset = g_sensor.row * g_sensor.nav_col * n;

        for (i = 0; i < g_sensor.row; i++) {
            for (j = 0; j < g_sensor.nav_col; j++) {
                *out++ = g_gf_raw_data_tmp[j * g_sensor.row + i + offset];
            }
        }
    }

    FUNC_EXIT(ret);
    return ret;
}

static void oswego_m_get_cfg_data(void) {
    uint32_t i = 0;
    VOID_FUNC_ENTER();
    if (g_product_info.vendor_id[0] != 0) {
        for (i = 0; i < OSWEGO_M_VENDOR_CFG_COUNT_MAX; i++) {
            if (g_product_info.vendor_id[0]
                    == g_oswego_m_fw_cfg.cfg_data_by_vendor_id[i]->vendor_id) {
                GF_LOGI(LOG_TAG "[%s] find vendor config, i=%u", __func__, i);
                g_cfg_data = &(g_oswego_m_fw_cfg.cfg_data_by_vendor_id[i]->cfg);
                VOID_FUNC_EXIT();
                return;
            }
        }

        if (OSWEGO_M_VENDOR_CFG_COUNT_MAX == i) {
            GF_LOGE(LOG_TAG "[%s] unknown vendor_id=0x%02X", __func__, g_product_info.vendor_id[0]);
        }
    }

    if (g_product_info.product_cfg_idx != 0) {
        for (i = 0; i < OSWEGO_M_PRODUCT_CFG_COUNT_MAX; i++) {
            if (g_product_info.product_cfg_idx
                    == g_oswego_m_fw_cfg.cfg_data_by_product_idx[i]->product_cfg_idx) {
                GF_LOGI(LOG_TAG "[%s] find product_cfg_idx config, i=%u", __func__, i);
                g_cfg_data = &(g_oswego_m_fw_cfg.cfg_data_by_product_idx[i]->cfg);
                VOID_FUNC_EXIT();
                return;
            }
        }

        if (OSWEGO_M_PRODUCT_CFG_COUNT_MAX == i) {
            GF_LOGE(LOG_TAG "[%s] unknown product_cfg_idx=0x%02X",
                    __func__, g_product_info.product_cfg_idx);
        }
    }

    // default config
    if (GF_CHIP_316M == g_config.chip_type
            || GF_CHIP_318M == g_config.chip_type
            || GF_CHIP_3118M == g_config.chip_type) {
        GF_LOGI(LOG_TAG "[%s] use default product_cfg_data", __func__);
        g_cfg_data = &(g_oswego_m_fw_cfg.cfg_data_by_product_idx[1]->cfg);
    } else {
        GF_LOGI(LOG_TAG "[%s] use default cfg_data", __func__);
        g_cfg_data = &(g_oswego_m_fw_cfg.cfg_data_by_vendor_id[0]->cfg);
    }
    VOID_FUNC_EXIT();
}

/* check chip version, MP/ECO... */
gf_error_t gf_hw_get_chip_id(uint8_t *buf, uint32_t buf_len) {
    gf_error_t ret = GF_SUCCESS;
    FUNC_ENTER();
    UNUSED_VAR(buf_len);
    /* no chip version for M chip */
    *buf = 0x00;
    FUNC_EXIT(ret);
    return ret;
}

gf_error_t gf_oswego_m_get_fw_version(uint8_t *buf, uint32_t buf_len) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    do {
        if (NULL == buf || buf_len < 10) {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        err = gf_oswego_m_secspi_read_bytes(GF_OSWEGO_M_FW_VERSION_REG, buf, 10);
        GF_LOGD(LOG_TAG "[%s] fw version=0x%02X, 0x%02X, 0x%02X", __func__, buf[7], buf[8], buf[9]);
    } while (0);
    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hw_get_vendor_id(uint8_t *buf, uint32_t buf_len) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    do {
        if (NULL == buf || buf_len < GF_OSWEGO_M_VENDOR_ID_LEN) {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        // uint8_t my_vendor[4] = { 0x05};
        // gf_oswego_m_secspi_write_flash(0x0C, 0x00, my_vendor, 4);
        // gf_sleep(200);
        // err = gf_oswego_m_secspi_read_flash(0x0C, 0x00, buf, buf_len);
        cpl_memcpy(buf, g_product_info.vendor_id, GF_OSWEGO_M_VENDOR_ID_LEN);
    } while (0);
    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_oswego_m_get_production_date(uint8_t *buf, uint32_t buf_len) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    do {
        if (NULL == buf || buf_len < GF_OSWEGO_M_DATE_LEN) {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        // uint8_t my_vendor[4] = { 0x15, 0x12, 0x24, 0xB5};
        // gf_oswego_m_secspi_write_flash(0x0C, 0x38, my_vendor, 4);
        // gf_sleep(200);
        // err = gf_oswego_m_secspi_read_flash(0x0C, 0x38, buf, buf_len);
        cpl_memcpy(buf, g_product_info.date, GF_OSWEGO_M_DATE_LEN);
    } while (0);
    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hw_get_sensor_id(uint8_t *buf, uint32_t buf_len) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    do {
        if (NULL == buf || buf_len < 16) {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        err = gf_oswego_m_secspi_read_flash_id(buf, buf_len);
    } while (0);
    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hw_get_otp_data(uint8_t *buf, uint32_t buf_len) {
    UNUSED_VAR(buf);
    UNUSED_VAR(buf_len);
    return GF_SUCCESS;
}

static gf_error_t gf_oswego_m_get_product_info(void) {
    gf_error_t err = GF_SUCCESS;
    uint8_t product_info[GF_OSWEGO_M_PRODUCT_INFO_LEN] = { 0 };
    FUNC_ENTER();
    do {
        err = gf_oswego_m_secspi_read_flash(0x0C, 0x00, product_info, GF_OSWEGO_M_PRODUCT_INFO_LEN);
        if (err != GF_SUCCESS) {
            break;
        }

        cpl_memcpy(g_product_info.vendor_id, product_info, GF_OSWEGO_M_VENDOR_ID_LEN);
        g_product_info.product_cfg_idx = product_info[0x19 + 1];
        cpl_memcpy(g_product_info.chip_type, &product_info[0x1C + 1], GF_OSWEGO_M_CHIP_TYPE_LEN);
        g_product_info.smt_test_error_code = product_info[0x20 + 1];
        g_product_info.module_test1_error_code = product_info[0x20 + 5];
        g_product_info.module_test2_error_code = product_info[0x20 + 9];
        g_product_info.key_touch_test_error_code = product_info[0x20 + 13];
        cpl_memcpy(g_product_info.date, &product_info[0x38], GF_OSWEGO_M_DATE_LEN);
        g_product_info.silkscreen_flag = product_info[0x3C];

        GF_LOGI(LOG_TAG "[%s] vendor_id=0x%02X, 0x%02X, 0x%02X, 0x%02X", __func__,
                g_product_info.vendor_id[0], g_product_info.vendor_id[1],
                g_product_info.vendor_id[2], g_product_info.vendor_id[3]);

        GF_LOGI(LOG_TAG "[%s] product_cfg_idx=0x%02X", __func__, g_product_info.product_cfg_idx);

        GF_LOGI(LOG_TAG "[%s] chip_type=GF%x%02XM", __func__,
                g_product_info.chip_type[0], g_product_info.chip_type[1]);

        GF_LOGI(LOG_TAG "[%s] date=20%02X-%02X-%02X", __func__,
                g_product_info.date[0], g_product_info.date[1], g_product_info.date[2]);
    } while (0);
    FUNC_EXIT(err);
    return err;
}

// only for test
gf_error_t gf_oswego_m_write_product_info(uint8_t product_cfg_idx) {
    gf_error_t err = GF_SUCCESS;
    uint8_t product_info[OSWEGO_M_PRODUCT_CFG_COUNT_MAX][GF_OSWEGO_M_PRODUCT_INFO_LEN] = { {
    0x05, 0x00, 0x00, 0x00, 0x05, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0xF0, 0x10, 0x55, 0x03, 0x18, 0x8F,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0x05, 0x18, 0xED, 0x55, 0xF0, 0x10, 0x00
    }, {
    0x0A, 0x00, 0x00, 0x00, 0x0A, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0xF1, 0x0F, 0x55, 0x03, 0x18, 0x8F,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0x05, 0x19, 0xEC, 0x55, 0xF0, 0x10, 0x00
    }, {
    0x15, 0x00, 0x00, 0x00, 0x0F, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0xF2, 0x0E, 0x55, 0x03, 0x18, 0x8F,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0x05, 0x20, 0xC5, 0x55, 0xF0, 0x10, 0x00
    },
    };
    FUNC_ENTER();
    GF_LOGD(LOG_TAG "[%s] product_cfg_idx=0x%02X", __func__, product_cfg_idx);

    do {
        if (product_cfg_idx >= OSWEGO_M_PRODUCT_CFG_COUNT_MAX) {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        err = gf_oswego_m_secspi_write_flash(0x0C, 0x00, product_info[product_cfg_idx],
        GF_OSWEGO_M_PRODUCT_INFO_LEN);
    } while (0);
    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hw_detect_sensor(void) {
    gf_error_t err = GF_SUCCESS;
    uint8_t chip_9p_version[4] = { 0 };

    FUNC_ENTER();

    gf_oswego_m_secspi_read_bytes(0x4220, chip_9p_version, 4);
    GF_LOGI(LOG_TAG "[%s] 9p version=0x%02X%02X%02X%02X.", __func__, chip_9p_version[3],
            chip_9p_version[2], chip_9p_version[1], chip_9p_version[0]);

    if (0x00 == chip_9p_version[3] && 0x90 == chip_9p_version[2]
        && 0x08 == chip_9p_version[1]) {
        err = GF_SUCCESS;
    } else {
        err = GF_ERROR_SENSOR_NOT_AVAILABLE;
    }

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_oswego_m_chip_init(uint8_t *download_fw_flag, uint8_t *download_cfg_flag) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint16_t chip_fw_version;
        uint32_t irq_type = 0;
        uint8_t version[20] = { 0 };

        if (NULL == g_gf_raw_data_tmp) {
            g_gf_raw_data_tmp = (uint16_t *) CPL_MEM_MALLOC(
                    g_sensor.row * g_sensor.col * sizeof(uint16_t));
            if (NULL == g_gf_raw_data_tmp) {
                GF_LOGE(LOG_TAG "[%s] g_gf_raw_data_tmp out of memory", __func__);
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
            }
        }

        if (*download_fw_flag > 0) {
            GF_LOGI(LOG_TAG "[%s] need force download fw", __func__);
            break;
        }

        err = gf_oswego_m_get_fw_version(version, 20);
        // ingore this error code, check fw version

        chip_fw_version = (uint16_t) (((version[7] << 12) & 0xF000) | ((version[8] << 8) & 0x0F00)
                | (version[9] & 0x00FF));
        g_fw_version = (uint16_t) (((g_oswego_m_fw_cfg.fw_data[12] << 8) & 0xFF00)
                | (g_oswego_m_fw_cfg.fw_data[13] & 0x00FF));
        GF_LOGI(LOG_TAG "[%s] chip_fw_version[0x%04X]", __func__, chip_fw_version);

        if (0 == chip_fw_version || 0xFFFF == chip_fw_version) {
            GF_LOGE(LOG_TAG "[%s] spi communication error", __func__);
            err = GF_ERROR_SPI_COMMUNICATION;
            break;
        }

        if (g_fw_version != chip_fw_version) {
            GF_LOGI(LOG_TAG "[%s] need do fw upgrade, g_fw_version[0x%04X], "
                    "chip_fw_version[0x%04X]", __func__, g_fw_version, chip_fw_version);
            *download_fw_flag = 1;
            *download_cfg_flag = 1;
        }

        gf_hw_set_mode(MODE_KEY);
        gf_hw_get_irq_type(&irq_type);
        gf_hw_clear_irq(irq_type);

        err = gf_oswego_m_get_product_info();
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_oswego_m_test_erase_fw(void) {
    gf_error_t err = GF_SUCCESS;
    uint8_t reg_value = 0;
    int32_t i = 0;
    int32_t len = 4096;
    uint8_t write_buf[256] = { 0 };
    uint32_t j = 0;
    FUNC_ENTER();
    do {
        cpl_memset(write_buf, 0x55, 256);

        gf_oswego_m_secspi_write_byte(0x8041, 0x0c);
        gf_oswego_m_secspi_write_byte(0x8042, 0xf4);

        for (; i < GF_SPI_COMPLETE_CHECK_RETRY_COUNT; i++) {
            gf_oswego_m_secspi_read_byte(0x8042, &reg_value);
            if (0xAA == reg_value) {
                break;
            }
            gf_sleep(10);
        }
        if (GF_SPI_COMPLETE_CHECK_RETRY_COUNT == i) {
            GF_LOGE(LOG_TAG "[%s] timeout", __func__);
            err = GF_ERROR_TIMEOUT;
            break;
        }

        for (j = 0; j < 4; j++) {
            for (i = 0; i < len; i += 256) {
                gf_oswego_m_secspi_write_bytes(0x8148 + i, write_buf, 256);
            }

            gf_oswego_m_secspi_write_byte(0x5094, j);

            for (; i < GF_SPI_COMPLETE_CHECK_RETRY_COUNT; i++) {
                gf_oswego_m_secspi_read_byte(0x5094, &reg_value);
                if (0x00 == reg_value) {
                    break;
                }
                gf_sleep(10);
            }

            if (GF_SPI_COMPLETE_CHECK_RETRY_COUNT == i) {
                GF_LOGE(LOG_TAG "[%s] timeout", __func__);
                err = GF_ERROR_TIMEOUT;
                break;
            }
        }
    } while (0);

    gf_oswego_m_secspi_write_byte(0x8041, 0x00);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_oswego_m_download_fw(uint8_t *reset_flag) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err =  gf_oswego_m_download_fw_from_file(g_oswego_m_fw_cfg.fw_data);
    *reset_flag = 1;
    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_oswego_m_download_cfg(gf_config_type_t config_type) {
    gf_error_t err = GF_SUCCESS;
    uint8_t *cfg = NULL;
    FUNC_ENTER();
    do {
        gf_algo_cfg_t algo_cfg = { 0 };
        gf_decode_ret_t ret = 0;
        cfg = (uint8_t *) CPL_MEM_MALLOC(g_oswego_m_fw_cfg.cfg_data_len);
        if (NULL == cfg) {
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        if (CONFIG_TEST_PIXEL_OPEN_A == config_type) {
            cpl_memcpy(cfg, g_oswego_m_fw_cfg.cfg_data_pixel_test,
                    g_oswego_m_fw_cfg.cfg_data_len);
            // close TX
            cfg[152 - 7] &= 0xfe;
            // a cfg
            cfg[151] = 0x01;
            cfg[GF_OSWEGO_M_CFG_LENGTH - 1] = 0x00;
        } else if (CONFIG_TEST_PIXEL_OPEN_B == config_type) {
            cpl_memcpy(cfg, g_oswego_m_fw_cfg.cfg_data_pixel_test,
                    g_oswego_m_fw_cfg.cfg_data_len);
            // close TX
            cfg[152 - 7] &= 0xfe;
            // b cfg
            cfg[151] = 0x02;
            cfg[GF_OSWEGO_M_CFG_LENGTH - 1] = 0x00;
        } else {
            uint8_t value = 0;
            uint8_t value1 = 0;

            oswego_m_get_cfg_data();

            if (1 == g_config.reissue_key_down_when_entry_ff_mode) {
                value |= 1 << 6;
            }

            if (1 == g_config.reissue_key_down_when_entry_image_mode) {
                value |= 1 << 5;
            }

            if (1 == g_config.reissue_key_down_when_entry_nav_mode) {
                value1 |= 1 << 1;
            }

            cpl_memcpy(cfg, g_cfg_data->data, g_oswego_m_fw_cfg.cfg_data_len);

            cfg[36] &= 0xFD;
            cfg[36] |= value1;

            cfg[43] &= 0x9F;
            cfg[43] |= value;
            // forced fw does not clear irq
            cfg[66] |= 0x02;
        }

        err = gf_oswego_m_download_cfg_from_file(cfg, config_type);

        // update encrypt status
        cpl_memset(&algo_cfg, 0, sizeof(algo_cfg));
        ret = gf_decodeAlgoCfgFromFwCfg(cfg, g_oswego_m_fw_cfg.cfg_data_len, &algo_cfg);
        if (GF_DECODE_SUCESS == ret) {
            g_sensor.raw_data_is_encrypt = algo_cfg.isEnableEncrypt;
        }
    } while (0);

    if (cfg != NULL) {
        CPL_MEM_FREE(cfg);
    }
    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_oswego_m_test_download_fw(uint8_t *fw_data, uint32_t fw_data_len) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();
    do {
        if (fw_data == NULL || fw_data_len != GF_OSWEGO_M_FW_LENGTH) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE("[%s] bad parameter", __func__);
            break;
        }

        err = gf_oswego_m_download_fw_from_file(fw_data);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_oswego_m_test_download_cfg(uint8_t *cfg_data, uint32_t cfg_data_len) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();
    do {
        if (cfg_data == NULL || cfg_data_len != GF_OSWEGO_M_CFG_LENGTH) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE("[%s] bad parameter", __func__);
            break;
        }

        err = gf_oswego_m_download_cfg_from_file(cfg_data, CONFIG_TEST_FROM_FILE);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_hw_chip_config_init(void) {
    gf_error_t err = GF_SUCCESS;
    gf_algo_cfg_t algo_cfg = { 0 };
    gf_decode_ret_t ret = GF_DECODE_SUCESS;
    uint8_t aes_seed[32] = { 0 };
    uint8_t aes_config[168] = { 0 };
    uint8_t aes_config_len = 0;

    FUNC_ENTER();

    /* chip AES decrypt init */
    GF11_AES_Config_Init(aes_seed, 32, aes_config, &aes_config_len);

    if (0x00 == g_product_info.vendor_id[0]) {  // vendor_id maybe init by gf_oswego_m_chip_init()
        err = gf_oswego_m_get_product_info();
    }

    oswego_m_get_cfg_data();

    // init algo preprocess params
    ret = gf_decodeAlgoCfgFromFwCfg(g_cfg_data->data, g_oswego_m_fw_cfg.cfg_data_len,
            &algo_cfg);
    if (ret != GF_DECODE_SUCESS) {
        GF_LOGE("[%s] gf_decodeAlgoCfgFromFwCfg failed", __func__);
        err = GF_ERROR_SPI_FW_CFG_DATA_ERROR;
        FUNC_EXIT(err);
        return err;
    }

    g_sensor.raw_data_is_encrypt = algo_cfg.isEnableEncrypt;

    g_sensor.flag_chip_info = algo_cfg.isFloating | (algo_cfg.isPixelCancel << 1)
            | (algo_cfg.isCoating << 2);

    if (g_config.chip_type == GF_CHIP_316M || g_config.chip_type == GF_CHIP_516M ||
        g_config.chip_type == GF_CHIP_816M) {
        g_sensor.flag_chip_info |= (0x3F << 3);
    } else if (g_config.chip_type == GF_CHIP_318M || g_config.chip_type == GF_CHIP_3118M ||
        g_config.chip_type == GF_CHIP_518M || g_config.chip_type == GF_CHIP_5118M) {
        g_sensor.flag_chip_info |= (0x3E << 3);
    }

    g_sensor.flag_chip_info = g_sensor.flag_chip_info | (g_sensor.row << 14) | (g_sensor.col << 23);

    if (96 == g_sensor.col) {
        g_sensor.thr_select_bmp = 300;
    } else if (algo_cfg.isCoating) {
        g_sensor.thr_select_bmp = 400;
    } else {
        g_sensor.thr_select_bmp = 600;
    }

    g_buf_len_info.line_len = g_sensor.row * 3 / 2;
    g_buf_len_info.line_aligned_len = (g_buf_len_info.line_len + 15) / 16 * 16;
    g_buf_len_info.sensor_raw_data_len = g_buf_len_info.line_aligned_len * g_sensor.col;
    g_buf_len_info.nav_frame_len = g_buf_len_info.line_aligned_len * g_sensor.nav_col;

    GF_LOGI("[%s] raw_data_is_encrypt=%u, flag_chip_info=%d, thr_select_bmp=%d", __func__,
            g_sensor.raw_data_is_encrypt, g_sensor.flag_chip_info, g_sensor.thr_select_bmp);

    FUNC_EXIT(err);

    return err;
}

/*Suspend to sample finger-print data*/
gf_error_t gf_hw_data_sample_suspend(void) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_oswego_m_secspi_write_byte(GF_OSWEGO_M_SAMPLING_PAUSE_REG, 0x01);
    FUNC_EXIT(err);
    return err;
}

/*Resume to sample finger-print data*/
gf_error_t gf_hw_data_sample_resume(void) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    err = gf_oswego_m_secspi_write_byte(GF_OSWEGO_M_SAMPLING_PAUSE_REG, 0x00);
    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hw_set_mode(gf_mode_t mode) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (mode >= sizeof(g_mode_array) / sizeof(g_mode_array[0])) {
            GF_LOGE(LOG_TAG "[%s] mode=%s, mode_code=%d", __func__, gf_strmode(mode), mode);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (MODE_SLEEP == g_cur_chip_mode && mode != MODE_SLEEP) {
            GF_LOGE(LOG_TAG "[%s] mode=%s, mode_code=%d", __func__, gf_strmode(mode), mode);
            GF_LOGE(LOG_TAG "[%s] g_cur_chip_mode=%s, g_cur_chip_mode=%d",
                    __func__, gf_strmode(g_cur_chip_mode), g_cur_chip_mode);
            g_cur_chip_mode = MODE_IDLE;
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        gf_oswego_m_secspi_write_byte(GF_OSWEGO_M_MODE_REG, g_mode_array[mode]);

        /* if old mode is FF,need sleep 70ms */
        if (MODE_FF == g_cur_chip_mode) {
            g_check_ff_mode_wakeup_flag = 1;
            gf_get_timestamp(&g_exit_ff_mode_time);
        }

        GF_LOGI(LOG_TAG "[%s] set mode=0x%02X", __func__, g_mode_array[mode]);
        g_cur_chip_mode = mode;
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_hw_get_mode(gf_mode_t *value) {
    gf_error_t ret = GF_SUCCESS;
    uint8_t mode = 0;
    uint8_t i = 0;
    uint32_t size = sizeof(g_mode_array) / sizeof(g_mode_array[0]);

    FUNC_ENTER();
    gf_oswego_m_secspi_read_byte(GF_OSWEGO_M_MODE_REG, &mode);
    GF_LOGI(LOG_TAG "[%s] get mode=0x%02X", __func__, mode);

    for (; i < size; i++) {
        if (mode == g_mode_array[i]) {
            *value = i;
            break;
        }
    }
    FUNC_EXIT(ret);
    return ret;
}

gf_error_t gf_oswego_m_pre_get_image(void) {
    return GF_SUCCESS;
}

gf_error_t gf_oswego_m_get_image(uint8_t *origin_data, uint32_t *origin_data_len) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint16_t checksum;
        uint16_t sensor_checksum;
        uint8_t *crc_data = NULL;
        uint32_t crc_data_len = 0;
        uint32_t read_len = 0;
        if (GF_CHIP_3118M == g_config.chip_type || GF_CHIP_318M == g_config.chip_type
            || GF_CHIP_5118M == g_config.chip_type || GF_CHIP_518M == g_config.chip_type) {
            gfx18m_rawdata_t *raw_data = (gfx18m_rawdata_t*)origin_data;
            read_len = sizeof(gfx18m_rawdata_t) - 12 * sizeof(uint8_t);
            crc_data_len = read_len - sizeof(raw_data->status_h) - sizeof(raw_data->status_l)
                        - sizeof(raw_data->checksum_h) - sizeof(raw_data->checksum_l);
            crc_data = &(raw_data->ring_h);

            gf_oswego_m_secspi_read_bytes_high_speed(GF_OSWEGO_M_BUFFER_STATUS_H_REG, origin_data,
                    read_len);
            sensor_checksum = (uint16_t) raw_data->checksum_h << 8 | raw_data->checksum_l;
        } else {
            gfx16m_rawdata_t *raw_data = (gfx16m_rawdata_t*)origin_data;
            read_len = sizeof(gfx16m_rawdata_t) - 12 * sizeof(uint8_t);
            crc_data_len = read_len - sizeof(raw_data->status_h) - sizeof(raw_data->status_l)
                        - sizeof(raw_data->checksum_h) - sizeof(raw_data->checksum_l);
            crc_data = &(raw_data->ring_h);

            gf_oswego_m_secspi_read_bytes_high_speed(GF_OSWEGO_M_BUFFER_STATUS_H_REG, origin_data,
                    read_len);
            sensor_checksum = (uint16_t) raw_data->checksum_h << 8 | raw_data->checksum_l;
        }

        *origin_data_len = read_len;
        g_rawdata = origin_data;

        checksum = (uint16_t) gf_oswego_m_get_checksum(crc_data, crc_data_len);
        if (checksum != sensor_checksum) {
            GF_LOGE(LOG_TAG "[%s] checksum failed calculate=0x%04X, sensor send=0x%04X", __func__,
                    checksum, sensor_checksum);
            err = GF_ERROR_SPI_RAW_DATA_CRC_FAILED;
            break;
        }
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_oswego_m_post_get_image(uint16_t *raw_data, uint32_t *raw_data_len) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (GF_CHIP_3118M == g_config.chip_type || GF_CHIP_318M == g_config.chip_type
            || GF_CHIP_5118M == g_config.chip_type || GF_CHIP_518M == g_config.chip_type) {
            /* decipher and do data re-order */
            oswego_m_raw_data_split(raw_data, ((gfx18m_rawdata_t *)g_rawdata)->data);
        } else {
            oswego_m_raw_data_split(raw_data, ((gfx16m_rawdata_t *)g_rawdata)->data);
        }

        *raw_data_len = g_sensor.row * g_sensor.col;
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_oswego_m_pre_get_nav_image(void) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        gf_oswego_m_secspi_read_bytes(GF_OSWEGO_M_BUFFER_STATUS_L_REG, g_pre_nav_frame_status, 6);
        g_pre_nav_frame_num = g_pre_nav_frame_status[5] & 0x0f;
        g_pre_nav_over_flag = ((g_pre_nav_frame_status[5] & 0x80) ? 1 : 0);

        GF_LOGI(LOG_TAG "[%s] framenum=%u, overflag=%u",
                __func__, g_pre_nav_frame_num, g_pre_nav_over_flag);
        if (g_pre_nav_over_flag > 0) {
            gf_oswego_m_secspi_write_byte(GF_OSWEGO_M_NAV_FRAME_NUM_REG,
                    g_pre_nav_frame_status[5] & 0x7f);
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_oswego_m_get_nav_image(uint8_t *origin_data, uint32_t *origin_data_len) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (0 == g_pre_nav_frame_num) {
            *origin_data_len = 0;
            break;
        }

        g_pre_nav_frame = origin_data;
        if (g_pre_nav_frame_status[0] & (0x1 << 5)) {
            GF_LOGI(LOG_TAG "[%s] read nav data from buf1", __func__);

            /*  first nav image byte maybe error in high speed ,so read from 0x9b47 or 0x8147  */
            err = gf_oswego_m_secspi_read_bytes_high_speed(GF_OSWEGO_M_NAV_BUF1_REG - 1,
                    g_pre_nav_frame, g_buf_len_info.nav_frame_len * (g_pre_nav_frame_num) + 1);
        } else {
            GF_LOGI(LOG_TAG "[%s] read nav data from buf0", __func__);

            err = gf_oswego_m_secspi_read_bytes_high_speed(GF_OSWEGO_M_NAV_BUF0_REG - 1,
                    g_pre_nav_frame, g_buf_len_info.nav_frame_len * (g_pre_nav_frame_num) + 1);
        }
        if (err != GF_SUCCESS) {
            break;
        }

        *origin_data_len = g_buf_len_info.nav_frame_len * (g_pre_nav_frame_num) + 1;
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_oswego_m_post_get_nav_image(uint16_t *raw_data, uint32_t *raw_data_len,
        uint8_t *frame_num, uint8_t *over_flag) {
    gf_error_t err = GF_SUCCESS;


    FUNC_ENTER();

    do {
        uint8_t checksum[2] = { 0 };
        uint16_t checknum_c = 0;
        uint16_t checknum_r = 0;
        if (g_pre_nav_frame_num == 0) {
            *over_flag = g_pre_nav_over_flag;
            *frame_num = g_pre_nav_frame_num;
            *raw_data_len = 0;
            break;
        }

        if (g_pre_nav_frame_status[0] & (0x1 << 5)) {
            GF_LOGI(LOG_TAG "[%s] read nav data from buf1", __func__);

            gf_oswego_m_secspi_read_bytes(GF_OSWEGO_M_NAV_BUF1_CHECKSUM_REG, checksum, 2);
        } else {
            GF_LOGI(LOG_TAG "[%s] read nav data from buf0", __func__);

            gf_oswego_m_secspi_read_bytes(GF_OSWEGO_M_NAV_BUF0_CHECKSUM_REG, checksum, 2);
        }

        checknum_c = gf_oswego_m_get_checksum(g_pre_nav_frame + 1,
                g_buf_len_info.nav_frame_len * (g_pre_nav_frame_num));
        checknum_r = (((uint16_t) (checksum[0] << 8)) | ((uint16_t) checksum[1]));

        if (checknum_c != checknum_r) {
            GF_LOGE(LOG_TAG "[%s] checknum failed. checknum_c=0x%04X, checknumc_r=0x%04X",
                    __func__, checknum_c, checknum_r);
            err = GF_ERROR_SPI_RAW_DATA_CRC_FAILED;
            break;
        }

        oswego_m_nav_raw_data_split(raw_data, g_pre_nav_frame + 1, g_pre_nav_frame_num);

        *raw_data_len = g_sensor.row * g_sensor.nav_col * (g_pre_nav_frame_num);
        *over_flag = g_pre_nav_over_flag;
        *frame_num = g_pre_nav_frame_num;
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_oswego_m_pre_get_nav_base(void) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    err = gf_oswego_m_secspi_read_bytes(GF_OSWEGO_M_NAV_BUF0_CHECKSUM_REG, checksum_nav_base, 2);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_oswego_m_get_nav_base(uint8_t *origin_data, uint32_t *origin_data_len) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do {
        uint16_t checknum_c = 0;
        uint16_t checknum_r = 0;
        g_nav_base_nav_frame = origin_data;

        err = gf_oswego_m_secspi_read_bytes_high_speed(GF_OSWEGO_M_NAV_BUF0_REG - 1,
                g_nav_base_nav_frame, g_buf_len_info.nav_frame_len + 1);
        if (err != GF_SUCCESS) {
            break;
        }

        *origin_data_len = g_buf_len_info.nav_frame_len + 1;

        checknum_c = gf_oswego_m_get_checksum(g_nav_base_nav_frame + 1,
                g_buf_len_info.nav_frame_len);
        checknum_r = (((uint16_t) (checksum_nav_base[0] << 8)) | ((uint16_t) checksum_nav_base[1]));
        if (checknum_c != checknum_r) {
            GF_LOGE(LOG_TAG "[%s] checksum failed, calculate=0x%04X, sensor send=0x%04X",
                    __func__, checknum_c, checknum_r);
            err = GF_ERROR_SPI_RAW_DATA_CRC_FAILED;
            break;
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_oswego_m_post_get_nav_base(uint16_t *raw_data, uint32_t *raw_data_len) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        err = oswego_m_nav_raw_data_split(raw_data, g_nav_base_nav_frame + 1, 1);

        *raw_data_len = g_sensor.row * g_sensor.nav_col;
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_hw_clear_irq(uint32_t irq_type) {
    gf_error_t err = GF_SUCCESS;

    UNUSED_VAR(irq_type);
    FUNC_ENTER();
    GF_LOGD(LOG_TAG "[%s] irq_type=%s, irq_type_code=0x%08X, g_last_irq_status=0x%04X",
            __func__, gf_strirq(irq_type), irq_type, g_last_irq_status);

    if ((g_last_irq_status & 0x8000) > 0) {
        gf_oswego_m_secspi_write_byte(GF_OSWEGO_M_BUFFER_STATUS_H_REG,
                (g_last_irq_status >> 8) & 0x7F);
    }

    if ((g_last_irq_status & 0x80) > 0) {
        gf_oswego_m_secspi_write_byte(GF_OSWEGO_M_BUFFER_STATUS_L_REG, g_last_irq_status & 0x7F);
    }
    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hw_get_irq_type(uint32_t *irq_type) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint8_t status[2] = { 0 };

        err = gf_oswego_m_secspi_read_bytes(GF_OSWEGO_M_BUFFER_STATUS_H_REG, status, 2);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] read address=0x%04X failed", __func__,
                    GF_OSWEGO_M_BUFFER_STATUS_H_REG);
            break;
        }

        if (0xFF == status[0]) {
            GF_LOGE(LOG_TAG "[%s] 0xFF invalid irq", __func__);
            err = GF_ERROR_INVALID_DATA;
            break;
        }

        // if IRQ_RESET=1, need ensure without other irq
        if ((MODE_FF == g_cur_chip_mode) && (status[1] > 0x80)) {
            GF_LOGE(LOG_TAG "[%s] invalid irq in the ff mode, status[0]=0x%02X, status[1]=0x%02X",
                    __func__, status[0], status[1]);
            err = GF_ERROR_INVALID_DATA;
            break;
        }

        g_last_irq_status = (uint32_t) (status[0] << 8 | status[1]);

        if (status[1] & GF_IRQ_STATUS_L_RESET) {
            *irq_type |= GF_IRQ_RESET_MASK;
            g_cur_chip_mode = MODE_IDLE;
        }

        if (status[0] & GF_IRQ_STATUS_H_ENABLE) {
            /* image interrupt */
            if (status[0] & GF_IRQ_STATUS_H_IMAGE) {
                /* image interrupt. */
                *irq_type |= GF_IRQ_IMAGE_MASK;
            }

            /*  home key interrupt */
            if (status[0] & GF_IRQ_STATUS_H_HOME_KEY_ENABLE) {
                /* key interrupt. */
                if (status[0] & GF_IRQ_STATUS_H_HOME_KEY_STATUS) {
                    /*  home key down */
                    *irq_type |= GF_IRQ_FINGER_DOWN_MASK;
                } else {
                    /*  home key up */
                    *irq_type |= GF_IRQ_FINGER_UP_MASK;
                }
            }

            if (status[0] & GF_IRQ_STATUS_H_MENU_KEY_ENABLE) {
                /* menu key interrupt. */
                if (status[0] & GF_IRQ_STATUS_H_MENU_KEY_STATUS) {
                    /*  menu key down */
                    *irq_type |= GF_IRQ_MENUKEY_DOWN_MASK;
                } else {
                    /*  menu key up */
                    *irq_type |= GF_IRQ_MENUKEY_UP_MASK;
                }
            }

            if (status[0] & GF_IRQ_STATUS_H_BACK_KEY_ENABLE) {
                /* back key interrupt. */
                if (status[0] & GF_IRQ_STATUS_H_BACK_KEY_STATUS) {
                    /*  back key down */
                    *irq_type |= GF_IRQ_BACKKEY_DOWN_MASK;
                } else {
                    /*  back key up */
                    *irq_type |= GF_IRQ_BACKKEY_UP_MASK;
                }
            }

            if (status[1] & GF_IRQ_STATUS_L_NAV_IMAGE) {
                /* navigation interrupt. */
                *irq_type |= GF_IRQ_NAV_MASK;
            }
        }

        if (g_fw_version < 0x1500) {  // check 0x8018 required fw version 1.05.00 or above
            break;
        }

        if (*irq_type & (GF_IRQ_IMAGE_MASK | GF_IRQ_NAV_MASK)) {
            uint8_t mode = 0;
            uint8_t chip_work_mode = 0;
            uint8_t i = 0;
            uint32_t size = sizeof(g_mode_array) / sizeof(g_mode_array[0]);

            err = gf_oswego_m_secspi_read_byte(GF_OSWEGO_M_CHIP_WORK_MODE_REG, &mode);
            if (GF_SUCCESS != err) {
                GF_LOGE(LOG_TAG "[%s] read address=0x%04X failed", __func__,
                        GF_OSWEGO_M_CHIP_WORK_MODE_REG);
                break;
            }

            for (; i < size; i++) {
                if (mode == g_mode_array[i]) {
                    chip_work_mode = i;
                    break;
                }
            }

            if (chip_work_mode != g_cur_chip_mode) {
                GF_LOGI(LOG_TAG "[%s] chip_work_mode=%s, chip_work_mode=%u, g_cur_chip_mode=%s, "
                        "g_cur_chip_mode=%u", __func__, gf_strmode(chip_work_mode), chip_work_mode,
                        gf_strmode(g_cur_chip_mode), g_cur_chip_mode);

                if (*irq_type & GF_IRQ_NAV_MASK) {
                    *irq_type &= ~GF_IRQ_NAV_MASK;
                } else {
                    *irq_type &= ~GF_IRQ_IMAGE_MASK;
                }
            }
        }
    } while (0);

    GF_LOGD(LOG_TAG "[%s] irq_type=%s, irq_type_code=0x%08X",
            __func__, gf_strirq(*irq_type), *irq_type);
    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_oswego_m_esd_check(uint8_t *result) {
    gf_error_t err = GF_SUCCESS;

    uint8_t reg = 0;
    FUNC_ENTER();
    do {
        err = gf_oswego_m_secspi_read_byte(GF_OSWEGO_M_ESD_REG, &reg);
        if (0xc6 == reg) {
            gf_oswego_m_secspi_write_byte(GF_OSWEGO_M_ESD_REG, 0xAA);
            *result = 0;
            break;
        }

        gf_sleep(10);
        err = gf_oswego_m_secspi_read_byte(GF_OSWEGO_M_ESD_REG, &reg);
        if (0xc6 == reg) {
            gf_oswego_m_secspi_write_byte(GF_OSWEGO_M_ESD_REG, 0xAA);
            *result = 0;
        } else {
            GF_LOGE(LOG_TAG "[%s] ESD check failed, register=0x%02X", __func__, reg);
            *result = 1;
        }
    } while (0);
    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hw_get_orientation(uint16_t *orientation, uint16_t *facing) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    *orientation = g_cfg_data->orientation;
    *facing = g_cfg_data->facing;

    GF_LOGD(LOG_TAG "[%s] exit orientation=%u, facing=%u", __func__, *orientation, *facing);
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

        err = gf_oswego_m_secspi_read_bytes((uint16_t)addr, content, read_len);

        GF_LOGD(
            LOG_TAG "[%s] READ 0x%04X(%d bytes)", __func__, addr, read_len);
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
            GF_LOGD(LOG_TAG "[%s] WRITE 0x%02X", __func__, content[i]);
        }

        err = gf_oswego_m_secspi_write_bytes((uint16_t)addr, content, write_len);

        GF_LOGD(
            LOG_TAG "[%s] WRITE 0x%04X(%d bytes)", __func__, addr, write_len);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_oswego_m_get_spi_speed(uint32_t irq_type, uint32_t *speed) {
    gf_error_t ret = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL== speed) {
            ret = GF_ERROR_BAD_PARAMS;
            break;
        }

        *speed = GF_SPI_SPEED_LOW;
        if (GF_IRQ_NAV_MASK == irq_type) {
            *speed = GF_SPI_SPEED_MEDIUM;
        }

        if (GF_IRQ_IMAGE_MASK == irq_type) {
            *speed = GF_SPI_SPEED_MEDIUM;
        }
    }while(0);

    GF_LOGD(LOG_TAG "[%s] speed=%u", __func__, *speed);
    FUNC_EXIT(ret);
    return ret;
}

void gf_hw_destroy(void) {
    VOID_FUNC_ENTER();

    if (g_gf_raw_data_tmp != NULL) {
        CPL_MEM_FREE(g_gf_raw_data_tmp);
        g_gf_raw_data_tmp = NULL;
    }

    VOID_FUNC_EXIT();
}

gf_error_t gf_oswego_m_fw_cfg_init(void) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        gf_fw_cfg_init();

        if (GF_OSWEGO_M_FW_LENGTH != g_oswego_m_fw_cfg.fw_data_len) {
            GF_LOGE(LOG_TAG "[%s] error fw data length", __func__);
            err = GF_ERROR_SPI_FW_CFG_DATA_ERROR;
            break;
        }

        if (GF_OSWEGO_M_CFG_LENGTH != g_oswego_m_fw_cfg.cfg_data_len) {
            GF_LOGE(LOG_TAG "[%s] error cfg data length", __func__);
            err = GF_ERROR_SPI_FW_CFG_DATA_ERROR;
            break;
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}
