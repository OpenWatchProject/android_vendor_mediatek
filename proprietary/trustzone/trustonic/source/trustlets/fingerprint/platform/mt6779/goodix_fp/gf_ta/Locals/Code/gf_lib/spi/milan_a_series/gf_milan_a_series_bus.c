/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/


#include "gf_tee_internal_api.h"
#include "gf_spi_common.h"
#include "gf_common.h"
#include "gf_error.h"
#include "gf_spi.h"
#include "gf_milan_a_series_bus.h"
#include "cpl_memory.h"
#include "cpl_string.h"

#define LOG_TAG "[gf_milan_ab_bus] "

#define GF_SPI_WRITE_CMD 0xF0
#define GF_SPI_READ_CMD  0xF1
#define GF_SPI_DUMMY     0x00

#define GF_SPI_PACKAGE_LEN    30000/*14336*/
#define GF_SPI_TX_HEADER_LEN  6
#define GF_SPI_RX_HEADER_LEN  2
#define GF_SPI_TX_PACKAGE_LEN (GF_SPI_PACKAGE_LEN - GF_SPI_TX_HEADER_LEN)
#define GF_SPI_RX_PACKAGE_LEN (GF_SPI_PACKAGE_LEN - GF_SPI_RX_HEADER_LEN)

#define SPI_RETRY_COUNT  (5)

typedef struct {
    uint8_t cmd;
    uint8_t addr_h;
    uint8_t addr_l;
    uint8_t len_h;
    uint8_t len_l;
    uint8_t buf[GF_SPI_TX_PACKAGE_LEN];
    uint8_t dummy;
}__attribute__ ((packed)) gf_tx_buf_t;

typedef struct {
    uint8_t cmd;
    uint8_t buf[GF_SPI_RX_PACKAGE_LEN];
}__attribute__ ((packed)) gf_rx_buf_t;

static gf_tx_buf_t g_tx_buf = { 0 };
static gf_rx_buf_t g_rx_buf = { 0 };

static gf_error_t gf_milan_a_series_secspi_write(uint16_t addr, uint8_t *buf, uint32_t buf_len) {
    gf_error_t err = GF_SUCCESS;
    uint32_t retry_times = 0;

    do {
        if (buf_len > GF_SPI_TX_PACKAGE_LEN) {
            GF_LOGE(LOG_TAG "[%s] invalid parameters", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        g_tx_buf.cmd = GF_SPI_WRITE_CMD;
        g_tx_buf.addr_h = (uint8_t) ((addr >> 8) & 0xFF);
        g_tx_buf.addr_l = (uint8_t) (addr & 0xFF);
        g_tx_buf.len_h = (uint8_t) (((buf_len / 2) >> 8) & 0xFF);
        g_tx_buf.len_l = (uint8_t) ((buf_len / 2) & 0xFF);
        if (buf != NULL) {
            cpl_memcpy(g_tx_buf.buf, buf, buf_len);
            endian_exchange(g_tx_buf.buf, buf_len); //change the read data to little endian.
        }
        g_tx_buf.dummy = GF_SPI_DUMMY;

        for (; retry_times < SPI_RETRY_COUNT; retry_times++) {
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

static gf_error_t gf_milan_a_series_secspi_read(uint16_t addr, uint8_t *buf, uint32_t buf_len,
        uint32_t speed) {
    gf_error_t err = GF_SUCCESS;
//    uint32_t packet_cnt = 0;
//    uint32_t remain_len = 0;
    uint32_t retry_times = 0;

    do {
        if (NULL == buf || (GF_SPI_RX_HEADER_LEN + buf_len) > GF_SPI_RX_PACKAGE_LEN) {
            GF_LOGE(LOG_TAG "[%s] invalid parameters", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        for (; retry_times < SPI_RETRY_COUNT; retry_times++) {
            g_tx_buf.cmd = GF_SPI_WRITE_CMD;
            g_tx_buf.addr_h = (uint8_t) ((addr >> 8) & 0xFF);
            g_tx_buf.addr_l = (uint8_t) (addr & 0xFF);
            g_tx_buf.len_h = GF_SPI_DUMMY;

            err = gf_spi_write(&g_tx_buf, &g_rx_buf, 4);
            if (GF_SUCCESS != err) {
                GF_LOGE(LOG_TAG "[%s] : spi address send error.\n", __func__);
                break;
            }

            cpl_memset(&g_tx_buf, 0x00, GF_SPI_RX_HEADER_LEN + buf_len);
            cpl_memset(&g_rx_buf, 0x00, GF_SPI_RX_HEADER_LEN + buf_len);
            g_tx_buf.cmd = GF_SPI_READ_CMD;
            err = gf_spi_read(&g_tx_buf, &g_rx_buf, GF_SPI_RX_HEADER_LEN + buf_len, speed);
            if (GF_SUCCESS == err) {
                cpl_memcpy(buf, g_rx_buf.buf, buf_len);
                //change the read data to little endian.
                endian_exchange(buf, buf_len);
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

gf_error_t gf_milan_a_series_secspi_read_word(uint16_t addr, uint16_t *value) {
    return gf_milan_a_series_secspi_read_bytes(addr, (uint8_t *) value, 2);
}

gf_error_t gf_milan_a_series_secspi_write_word(uint16_t addr, uint16_t value) {
    return gf_milan_a_series_secspi_write_bytes(addr, (uint8_t *) &value, 2);
}

gf_error_t gf_milan_a_series_secspi_write_byte(uint16_t addr, uint8_t value) {
    gf_error_t ret = GF_SUCCESS;
    uint16_t val = 0;

    do {
        if (0 != addr % 2) {
            addr--;
            ret = gf_milan_a_series_secspi_read_word(addr, &val);
            if (GF_SUCCESS != ret) {
                break;
            }
            val &=  0x00FF;
            val |= value << 8;
            ret = gf_milan_a_series_secspi_write_word(addr, val);
            if (GF_SUCCESS != ret) {
                break;
            }
        } else {
            ret = gf_milan_a_series_secspi_read_word(addr, &val);
            if (GF_SUCCESS != ret) {
                break;
            }
            val &=  0xFF00;
            val |= value;
            ret = gf_milan_a_series_secspi_write_word(addr, val);
            if (GF_SUCCESS != ret) {
                break;
            }
        }
    } while (0);

    return ret;
}

gf_error_t gf_milan_a_series_secspi_write_bytes(uint16_t addr, uint8_t *buf, uint32_t buf_len) {
    return gf_milan_a_series_secspi_write(addr, buf, buf_len);
}

gf_error_t gf_milan_a_series_secspi_read_byte(uint16_t addr, uint8_t *value) {
    gf_error_t ret = GF_SUCCESS;
    uint16_t val = 0;

    do {
        if (0 != addr % 2) {
            addr--;
            ret = gf_milan_a_series_secspi_read_word(addr, &val);
            if (GF_SUCCESS != ret) {
                break;
            }
            *value = (uint8_t) (val >> 8);
        } else {
            ret = gf_milan_a_series_secspi_read_word(addr, &val);
            if (GF_SUCCESS != ret) {
                break;
            }
            *value = (uint8_t) val;
        }
    } while (0);

    return ret;
}

gf_error_t gf_milan_a_series_secspi_read_bytes(uint16_t addr, uint8_t *buf, uint32_t buf_len) {

    return gf_milan_a_series_secspi_read(addr, buf, buf_len, GF_SPI_SPEED_LOW);

}

gf_error_t gf_milan_a_series_secspi_read_bytes_high_speed(uint16_t addr, uint8_t *buf, uint32_t buf_len) {
    return gf_milan_a_series_secspi_read(addr, buf, buf_len, GF_SPI_SPEED_HIGH);
}

gf_error_t gf_milan_a_series_secspi_read_bytes_with_speed(uint16_t addr, uint8_t *buf, uint32_t buf_len, uint32_t speed) {
    if (speed < GF_SPI_SPEED_LOW || speed > GF_SPI_SPEED_HIGH) {
        GF_LOGE(LOG_TAG "[%s] speed error, speed =%u", __func__, speed);
        return GF_ERROR_BAD_PARAMS;
    }
    return gf_milan_a_series_secspi_read(addr, buf, buf_len, speed);
}

gf_error_t gf_milan_a_series_secspi_read_flash(uint32_t flash_index, uint32_t offset, uint8_t *buf,
        uint32_t buf_len) {
    gf_error_t err = GF_SUCCESS;

    UNUSED_VAR(flash_index);
    UNUSED_VAR(offset);
    UNUSED_VAR(buf);
    UNUSED_VAR(buf_len);

    return err;
}

gf_error_t gf_milan_a_series_secspi_write_flash(uint32_t flash_index, uint32_t offset, uint8_t *buf,
        uint32_t buf_len) {
    gf_error_t err = GF_SUCCESS;

    UNUSED_VAR(flash_index);
    UNUSED_VAR(offset);
    UNUSED_VAR(buf);
    UNUSED_VAR(buf_len);

    return err;
}

