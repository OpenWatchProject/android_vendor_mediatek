/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#include <string.h>

#include "gf_tee_internal_api.h"
#include "gf_common.h"
#include "gf_error.h"
#include "gf_spi.h"
#include "gf_hw_common.h"
#include "cpl_string.h"

#include "milan_hv_bus.h"
#include "milan_hv_regs.h"

#define LOG_TAG "[milan_hv_bus]"

#define GF_SPI_MTK_PACKAGE_LEN    (1 << 10)
#define GF_HV_SPI_PACKAGE_LEN      ((GF_RAWDATA_LEN + 4) * 5 + 1)
#define GF_HV_SPI_TX_HEADER_LEN  5
#define GF_HV_SPI_RX_HEADER_LEN  1
#define GF_HV_SPI_TX_PACKAGE_LEN (GF_HV_SPI_PACKAGE_LEN - GF_HV_SPI_TX_HEADER_LEN)
#define GF_HV_SPI_RX_PACKAGE_LEN (GF_HV_SPI_PACKAGE_LEN - GF_HV_SPI_RX_HEADER_LEN)

#define SPI_RETRY_COUNT  (3)

const uint32_t target_little_endian = 0;

typedef struct {
    uint8_t cmd;
    uint8_t addr_h;
    uint8_t addr_l;
    uint8_t len_h;
    uint8_t len_l;
    uint8_t buf[GF_HV_SPI_TX_PACKAGE_LEN];
}__attribute__((packed)) gf_tx_buf_t;

typedef struct {
    uint8_t cmd;
    uint8_t buf[GF_HV_SPI_RX_PACKAGE_LEN];
}__attribute__((packed)) gf_rx_buf_t;

static gf_tx_buf_t g_tx_buf = { 0 };
static gf_rx_buf_t g_rx_buf = { 0 };

/* write bytes without endian_exchange */
gf_error_t gf_milan_hv_write_bytes(uint16_t addr, uint8_t *buf, uint32_t buf_len) {
    gf_error_t err = GF_SUCCESS;
    uint32_t retry_times = 0;

    FUNC_ENTER();
    do {
        if ((buf_len > GF_HV_SPI_TX_PACKAGE_LEN) || (buf_len % 2 != 0)) {
            GF_LOGE(LOG_TAG "[%s] invalid parameters", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        g_tx_buf.cmd = SPI_WRITE;
        g_tx_buf.addr_h = (uint8_t) ((addr >> 8) & 0xFF);
        g_tx_buf.addr_l = (uint8_t) (addr & 0xFF);
        g_tx_buf.len_h = (uint8_t) ((buf_len / 2 >> 8) & 0xFF);  // word length
        g_tx_buf.len_l = (uint8_t) (buf_len / 2 & 0xFF);
        if (buf != NULL) {
            if (target_little_endian) {
                uint32_t src = 0;
                uint32_t dst = buf_len - 1;
                while (src < buf_len) {
                    g_tx_buf.buf[dst] = buf[src];
                    src++;
                    dst--;
                }
            } else {
                cpl_memcpy(g_tx_buf.buf, buf, buf_len);
            }
        }

        for (; retry_times < SPI_RETRY_COUNT; retry_times++) {
            err = gf_spi_write(&g_tx_buf, &g_rx_buf, GF_HV_SPI_TX_HEADER_LEN + buf_len);
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

    FUNC_EXIT(err);
    return err;
}

/* write word with endian_exchange */
gf_error_t gf_milan_hv_write_word_with_endian_exchange(uint16_t addr, uint16_t value) {
    gf_error_t err = GF_SUCCESS;
    uint16_t data = 0;

    data = ((value >> 8) & 0x00ff) | ((value << 8) & 0xff00);
    err = gf_milan_hv_write_bytes(addr, (uint8_t *) &data, sizeof(uint16_t));

    return err;
}

/**
 * read spi data according different fifo rate.
 * read register data default using 1M rate
 * @params speed means fifo rate, unit: M
 */
/* read bytes without endian_exchange */
gf_error_t gf_milan_hv_read_bytes(uint16_t addr, uint8_t *buf, uint32_t buf_len, uint32_t speed) {
    gf_error_t err = GF_SUCCESS;
    uint32_t retry_times = 0;

    FUNC_ENTER();
    do {
        if (NULL == buf || buf_len > GF_HV_SPI_RX_PACKAGE_LEN) {
            GF_LOGE(LOG_TAG "[%s] invalid parameters, buf=%p, buf_len=%u", __func__, buf, buf_len);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        for (; retry_times < SPI_RETRY_COUNT; retry_times++) {
            g_tx_buf.cmd = SPI_WRITE;
            g_tx_buf.addr_h = (uint8_t) ((addr >> 8) & 0xFF);
            g_tx_buf.addr_l = (uint8_t) (addr & 0xFF);
            err = gf_spi_write(&g_tx_buf, &g_rx_buf, 3);
            if (err != GF_SUCCESS) {
                break;
            }

            cpl_memset(&g_tx_buf, 0xFF, sizeof(gf_tx_buf_t));
            g_tx_buf.cmd = SPI_READ;

            err = gf_spi_read(&g_tx_buf, &g_rx_buf, GF_HV_SPI_RX_HEADER_LEN + buf_len, speed);
            if (GF_SUCCESS == err) {
                if (target_little_endian) {
                    uint32_t src = buf_len;
                    uint32_t dst = 0;
                    while (dst < buf_len) {
                        buf[dst] = g_rx_buf.buf[src];
                        src--;
                        dst++;
                    }
                } else {
                    cpl_memcpy(buf, g_rx_buf.buf, buf_len);
                }
                break;
            }
        }

        if (retry_times >= SPI_RETRY_COUNT) {
            GF_LOGE(LOG_TAG "[%s] GF_ERROR_SPI_TRANSFER_ERROR", __func__);
            err = GF_ERROR_SPI_TRANSFER_ERROR;
            break;
        }
    } while (0);

    FUNC_EXIT(err);
    return err;
}

/* read word with endian_exchange */
gf_error_t gf_milan_hv_read_word_with_endian_exchange(uint16_t addr, uint16_t *value) {
    gf_error_t err = GF_SUCCESS;
    uint16_t data = 0;

    err = gf_milan_hv_read_bytes(addr, (uint8_t *) &data, sizeof(uint16_t), GF_SPI_SPEED_LOW);
    *value = ((data >> 8) & 0x00ff) | ((data << 8) & 0xff00);

    return err;
}

/* read bytes with endian_exchange */
gf_error_t gf_milan_hv_read_bytes_with_endian_exchange(uint16_t addr, uint8_t *buf,
        uint32_t buf_len) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    err = gf_milan_hv_read_bytes(addr, buf, buf_len, GF_SPI_SPEED_LOW);
    if (GF_SUCCESS != err) {
        GF_LOGE(LOG_TAG "[%s] HV read bytes error, err=%s, errno=%d",
                __func__, gf_strerror(err), err);
    } else {
        gf_hw_endian_exchange(buf, buf_len);
    }

    FUNC_EXIT(err);
    return err;
}

/* write cmd and data */
gf_error_t gf_milan_hv_write_cmd_data(uint8_t cmd, uint8_t data) {
    gf_error_t err = GF_SUCCESS;
    uint32_t retry_times = 0;
    uint8_t data_tx[2] = { 0 };
    uint8_t data_rx[2] = { 0 };

    FUNC_ENTER();

    do {
        data_tx[0] = cmd;
        data_tx[1] = data;

        for (; retry_times < SPI_RETRY_COUNT; retry_times++) {
            err = gf_spi_write(data_tx, data_rx, 2);
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

    FUNC_EXIT(err);
    return err;
}

/* write cmd */
gf_error_t gf_milan_hv_write_cmd(milan_hv_cmd_t cmd) {
    gf_error_t err = GF_SUCCESS;
    uint32_t retry_times = 0;
    FUNC_ENTER();
    GF_LOGD(LOG_TAG "[%s] cmd=0x%02X", __func__, cmd);

    do {
        g_tx_buf.cmd = cmd;

        for (; retry_times < SPI_RETRY_COUNT; retry_times++) {
            err = gf_spi_write(&g_tx_buf, &g_rx_buf, 1);
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
    FUNC_EXIT(err);
    return err;
}

