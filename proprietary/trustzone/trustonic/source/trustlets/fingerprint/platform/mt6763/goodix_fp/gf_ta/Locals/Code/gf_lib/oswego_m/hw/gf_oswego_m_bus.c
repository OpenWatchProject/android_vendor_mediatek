/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#include "gf_tee_internal_api.h"
#include "gf_common.h"
#include "gf_error.h"
#include "gf_spi.h"
#include "gf_oswego_m_bus.h"
#include "cpl_memory.h"
#include "cpl_string.h"

#define LOG_TAG "[gf_oswego_m_bus]"

#define GF_SPI_WRITE_CMD 0xF0
#define GF_SPI_READ_CMD  0xF1

#define GF_SPI_MTK_PACKAGE_LEN    (1 << 10)
#define GF_SPI_PACKAGE_LEN    15360
#define GF_SPI_TX_HEADER_LEN  3
#define GF_SPI_RX_HEADER_LEN  1
#define GF_SPI_TX_PACKAGE_LEN (GF_SPI_PACKAGE_LEN - GF_SPI_TX_HEADER_LEN)
#define GF_SPI_RX_PACKAGE_LEN (GF_SPI_PACKAGE_LEN - GF_SPI_RX_HEADER_LEN)

#define SPI_RETRY_COUNT  (5)

#define FF_MODE_WAKEUP_TIME_INTERVAL (70000)  // us

typedef struct {
    uint8_t cmd;
    uint8_t addr_h;
    uint8_t addr_l;
    uint8_t buf[GF_SPI_TX_PACKAGE_LEN];
}__attribute__((packed)) gf_tx_buf_t;

typedef struct {
    uint8_t cmd;
    uint8_t buf[GF_SPI_RX_PACKAGE_LEN];
}__attribute__((packed)) gf_rx_buf_t;

static gf_tx_buf_t g_tx_buf = { 0 };
static gf_rx_buf_t g_rx_buf = { 0 };

extern uint64_t g_exit_ff_mode_time;
extern uint32_t g_check_ff_mode_wakeup_flag;

void gf_oswego_m_bus_init_global_variable(void) {
    cpl_memset(&g_tx_buf, 0, sizeof(g_tx_buf));
    cpl_memset(&g_rx_buf, 0, sizeof(g_rx_buf));
}

static gf_error_t gf_oswego_m_secspi_write(uint16_t addr, uint8_t *buf, uint32_t buf_len) {
    gf_error_t err = GF_SUCCESS;
    uint32_t retry_times = 0;


    do {
        if (buf_len > GF_SPI_TX_PACKAGE_LEN) {
            GF_LOGE(LOG_TAG "[%s] invalid parameters", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (g_check_ff_mode_wakeup_flag > 0) {
            uint64_t current_time = 0;
            uint64_t delta_time = 0;
            gf_get_timestamp(&current_time);
            delta_time = current_time - g_exit_ff_mode_time;

            if (delta_time < FF_MODE_WAKEUP_TIME_INTERVAL) {
                gf_sleep((FF_MODE_WAKEUP_TIME_INTERVAL - delta_time) / 1000);
                g_check_ff_mode_wakeup_flag = 0;
            }
        }

        g_tx_buf.cmd = GF_SPI_WRITE_CMD;
        g_tx_buf.addr_h = (uint8_t) ((addr >> 8) & 0xFF);
        g_tx_buf.addr_l = (uint8_t) (addr & 0xFF);
        if (buf != NULL) {
            cpl_memcpy(g_tx_buf.buf, buf, buf_len);
        }

        for (retry_times = 0; retry_times < SPI_RETRY_COUNT; retry_times++) {
            err = gf_spi_write(&g_tx_buf, &g_rx_buf, GF_SPI_TX_HEADER_LEN + buf_len);
            if (GF_SUCCESS == err) {
                break;
            }
        }

        if (retry_times >= SPI_RETRY_COUNT) {
            GF_LOGE(LOG_TAG "[%s] GF_ERROR_SPI_TRANSFER_ERROR", __func__);
            err = GF_ERROR_SPI_TRANSFER_ERROR;
            break;
        }
    } while (0);

    return err;
}

static gf_error_t gf_oswego_m_secspi_read(uint16_t addr, uint8_t *buf, uint32_t buf_len,
        uint32_t speed) {
    gf_error_t err = GF_SUCCESS;
    uint32_t retry_times = 0;

    do {
        if (NULL == buf || buf_len > GF_SPI_RX_PACKAGE_LEN) {
            GF_LOGE(LOG_TAG "[%s] invalid parameters", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (g_check_ff_mode_wakeup_flag > 0) {
            uint64_t current_time = 0;
            uint64_t delta_time = 0;
            gf_get_timestamp(&current_time);
            delta_time = current_time - g_exit_ff_mode_time;
            if (delta_time < FF_MODE_WAKEUP_TIME_INTERVAL) {
                gf_sleep((FF_MODE_WAKEUP_TIME_INTERVAL - delta_time) / 1000);
                g_check_ff_mode_wakeup_flag = 0;
            }
        }

        for (retry_times = 0; retry_times < SPI_RETRY_COUNT; retry_times++) {
            err = gf_oswego_m_secspi_write(addr, NULL, 0);
            if (err != GF_SUCCESS) {
                break;
            }
            // clear the spi tx buff avoid send unknow data to sensor.
            cpl_memset(&g_tx_buf, 0x00, buf_len + GF_SPI_RX_HEADER_LEN);
            g_tx_buf.cmd = GF_SPI_READ_CMD;

            if ((buf_len + GF_SPI_RX_HEADER_LEN) > GF_SPI_MTK_PACKAGE_LEN) {
                /*only for support one frame image mode read*/
                uint32_t mtk_buf_len = ((buf_len + GF_SPI_RX_HEADER_LEN + 1023) / 1024) * 1024;
                err = gf_spi_read(&g_tx_buf, &g_rx_buf, mtk_buf_len, speed);
            } else {
                err = gf_spi_read(&g_tx_buf, &g_rx_buf, GF_SPI_RX_HEADER_LEN + buf_len, speed);
            }
            if (GF_SUCCESS == err) {
                cpl_memcpy(buf, g_rx_buf.buf, buf_len);
                break;
            }
        }

        if (retry_times >= SPI_RETRY_COUNT) {
            GF_LOGE(LOG_TAG "[%s] GF_ERROR_SPI_TRANSFER_ERROR", __func__);
            err = GF_ERROR_SPI_TRANSFER_ERROR;
            break;
        }
    } while (0);

    return err;
}

gf_error_t gf_oswego_m_secspi_write_byte(uint16_t addr, uint8_t value) {
    return gf_oswego_m_secspi_write(addr, &value, sizeof(uint8_t));
}

gf_error_t gf_oswego_m_secspi_write_bytes(uint16_t addr, uint8_t *buf, uint32_t buf_len) {
    return gf_oswego_m_secspi_write(addr, buf, buf_len);
}

gf_error_t gf_oswego_m_secspi_read_byte(uint16_t addr, uint8_t *value) {
    return gf_oswego_m_secspi_read(addr, value, sizeof(uint8_t), GF_SPI_SPEED_LOW);
}

gf_error_t gf_oswego_m_secspi_read_bytes(uint16_t addr, uint8_t *buf, uint32_t buf_len) {
    return gf_oswego_m_secspi_read(addr, buf, buf_len, GF_SPI_SPEED_LOW);
}

gf_error_t gf_oswego_m_secspi_read_bytes_high_speed(uint16_t addr, uint8_t *buf, uint32_t buf_len) {
    return gf_oswego_m_secspi_read(addr, buf, buf_len, GF_SPI_SPEED_MEDIUM);
}

gf_error_t gf_oswego_m_secspi_read_flash_id(uint8_t *buf, uint32_t buf_len) {
    gf_error_t err = GF_SUCCESS;
    uint8_t reg_value = 0;
    uint32_t i = 0;

    FUNC_ENTER();

    do {
        if (NULL == buf) {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        gf_oswego_m_secspi_write_byte(0x8041, 0x09);
        gf_oswego_m_secspi_write_byte(0x8042, 0xF7);

        for (i = 0; i < GF_SPI_COMPLETE_CHECK_RETRY_COUNT; i++) {
            gf_oswego_m_secspi_read_byte(0x8042, &reg_value);
            if (0xAA == reg_value) {
                break;
            }
            gf_sleep(10);
        }
        if (GF_SPI_COMPLETE_CHECK_RETRY_COUNT == i) {
            GF_LOGE(LOG_TAG "[%s] read 0x8042 timeout", __func__);
            err = GF_ERROR_TIMEOUT;
            break;
        } else {
            GF_LOGD(LOG_TAG "[%s] read 0x8042 retry count=%u", __func__, i);
        }

        gf_oswego_m_secspi_write_byte(0x5094, 0x1);

        for (i = 0; i < GF_SPI_COMPLETE_CHECK_RETRY_COUNT; i++) {
            gf_oswego_m_secspi_read_byte(0x5094, &reg_value);
            if (0x00 == reg_value) {
                break;
            }
            gf_sleep(10);
        }
        if (GF_SPI_COMPLETE_CHECK_RETRY_COUNT == i) {
            GF_LOGE(LOG_TAG "[%s] read 0x5094 timeout", __func__);
            err = GF_ERROR_TIMEOUT;
            break;
        } else {
            GF_LOGD(LOG_TAG "[%s] read 0x5094 retry count=%u", __func__, i);
        }

        gf_oswego_m_secspi_read_bytes(0x8148, buf, buf_len);
    } while (0);
    // exit from read flash mode.
    gf_oswego_m_secspi_write_byte(0x8041, 0x00);
    gf_sleep(300);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_oswego_m_secspi_read_flash(uint32_t flash_index, uint32_t offset, uint8_t *buf,
        uint32_t buf_len) {
    gf_error_t err = GF_SUCCESS;
    uint8_t reg_value = 0;
    uint32_t i = 0;
    FUNC_ENTER();
    GF_LOGD(LOG_TAG "[%s] flash_index=0x%08X, offset=%u", __func__, flash_index, offset);

    do {
        if (NULL == buf) {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        gf_oswego_m_secspi_write_byte(0x8041, 0x08);
        gf_oswego_m_secspi_write_byte(0x8042, 0xF8);

        for (i = 0; i < GF_SPI_COMPLETE_CHECK_RETRY_COUNT; i++) {
            gf_oswego_m_secspi_read_byte(0x8042, &reg_value);
            if (0xAA == reg_value) {
                break;
            }
            gf_sleep(10);
        }
        if (GF_SPI_COMPLETE_CHECK_RETRY_COUNT == i) {
            GF_LOGE(LOG_TAG "[%s] read 0x8042 timeout", __func__);
            err = GF_ERROR_TIMEOUT;
            break;
        } else {
            GF_LOGD(LOG_TAG "[%s] read 0x8042 retry count=%u", __func__, i);
        }

        gf_oswego_m_secspi_write_byte(0x5094, flash_index);

        for (i = 0; i < GF_SPI_COMPLETE_CHECK_RETRY_COUNT; i++) {
            gf_oswego_m_secspi_read_byte(0x5094, &reg_value);
            if (0x00 == reg_value) {
                break;
            }
            gf_sleep(10);
        }
        if (GF_SPI_COMPLETE_CHECK_RETRY_COUNT == i) {
            GF_LOGE(LOG_TAG "[%s] read 0x5094 timeout", __func__);
            err = GF_ERROR_TIMEOUT;
            break;
        } else {
            GF_LOGD(LOG_TAG "[%s] read 0x5094 retry count=%u", __func__, i);
        }

        gf_oswego_m_secspi_read_bytes(0x8148 + offset, buf, buf_len);
    } while (0);
    // exit from read flash mode.
    gf_oswego_m_secspi_write_byte(0x8041, 0x00);
    gf_sleep(300);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_oswego_m_secspi_write_flash(uint32_t flash_index, uint32_t offset, uint8_t *buf,
        uint32_t buf_len) {
    gf_error_t err = GF_SUCCESS;
    uint8_t *flash_buf = NULL;
    uint32_t len = offset + buf_len;
    uint8_t reg_value = 0;
    uint32_t i = 0;
    FUNC_ENTER();
    GF_LOGD(LOG_TAG "flash_index=0x%08X, offset=%u", flash_index, offset);

    do {
        if (NULL == buf) {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (len < 64) {
            len = 64;
        }

        flash_buf = (uint8_t *) CPL_MEM_MALLOC(len);
        if (NULL == flash_buf) {
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        if (0 == offset) {
            cpl_memcpy(flash_buf, buf, buf_len);
        } else {
            gf_oswego_m_secspi_read_flash(flash_index, 0, flash_buf, len);
            cpl_memcpy(&flash_buf[offset], buf, buf_len);
            gf_sleep(200);
        }

        gf_oswego_m_secspi_write_byte(0x8041, 0x0C);
        gf_oswego_m_secspi_write_byte(0x8042, 0xF4);

        for (i = 0; i < GF_SPI_COMPLETE_CHECK_RETRY_COUNT; i++) {
            gf_oswego_m_secspi_read_byte(0x8042, &reg_value);
            if (0xAA == reg_value) {
                break;
            }
            gf_sleep(10);
        }
        if (GF_SPI_COMPLETE_CHECK_RETRY_COUNT == i) {
            GF_LOGE(LOG_TAG "[%s] read 0x8042 timeout", __func__);
            err = GF_ERROR_TIMEOUT;
            break;
        } else {
            GF_LOGD(LOG_TAG "[%s] read 0x8042 retry count=%u", __func__, i);
        }

        gf_oswego_m_secspi_write_bytes(0x8148, flash_buf, len);

        gf_oswego_m_secspi_write_byte(0x5094, flash_index);

        for (i = 0; i < GF_SPI_COMPLETE_CHECK_RETRY_COUNT; i++) {
            gf_oswego_m_secspi_read_byte(0x5094, &reg_value);
            if (0x00 == reg_value) {
                break;
            }
            gf_sleep(10);
        }
        if (GF_SPI_COMPLETE_CHECK_RETRY_COUNT == i) {
            GF_LOGE(LOG_TAG "[%s] read 0x5094 timeout", __func__);
            err = GF_ERROR_TIMEOUT;
            break;
        } else {
            GF_LOGD(LOG_TAG "[%s] read 0x5094 retry count=%u", __func__, i);
        }

        gf_oswego_m_secspi_read_bytes(0x8148, flash_buf, len);
        if (cpl_memcmp(buf, &flash_buf[offset], buf_len) != 0) {
            GF_LOGE(LOG_TAG "[%s] read back failed", __func__);
            err = GF_ERROR_INVALID_DATA;
        }
    } while (0);

    gf_oswego_m_secspi_write_byte(0x8041, 0x00);

    if (flash_buf != NULL) {
        CPL_MEM_FREE(flash_buf);
    }

    FUNC_EXIT(err);
    return err;
}

