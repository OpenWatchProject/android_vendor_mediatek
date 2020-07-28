#include "tlStd.h"
#include "TlApi/TlApi.h"
#include "spi.h"
#include "drtlspi_api.h"
#include "gf_tee_internal_api.h"
#include "gf_error.h"
#include "gf_spi.h"


#define LOG_TAG "[gf_spi] "

#define SPI_1MH_HIGH_TIME 54
#define SPI_1MH_LOW_TIME  54

#define SPI_6MH_HIGH_TIME 9
#define SPI_6MH_LOW_TIME  9

static const struct mt_chip_conf g_mt_chip_conf = {
        .setuptime    = 50,
        .holdtime     = 50,
        .high_time    = SPI_1MH_HIGH_TIME,
        .low_time     = SPI_1MH_LOW_TIME,
        .cs_idletime  = 10,
        .ulthgh_thrsh = 0,
        .cpol         = SPI_CPOL_0,
        .cpha         = SPI_CPHA_0,
        .rx_mlsb      = SPI_MSB,
        .tx_mlsb      = SPI_MSB,
        .tx_endian    = SPI_LENDIAN,
        .rx_endian    = SPI_LENDIAN,
        .com_mod      = DMA_TRANSFER,
        .pause        = 0,
        .finish_intr  = 1,
        .deassert     = 0,
        .ulthigh      = 0,
        .tckdly       = 0
};

gf_error_t gf_spi_write(void *tx_buf, void *rx_buf, uint32_t buf_len) {
    gf_error_t err = GF_SUCCESS;
    tlApiResult_t ret = TLAPI_OK;

    do {
        struct mt_chip_conf config;
        memcpy(&config, &g_mt_chip_conf, sizeof(struct mt_chip_conf));
        if (buf_len < 32) {
            config.com_mod = FIFO_TRANSFER;
        }

        ret = drSpiSend1(tx_buf, rx_buf, buf_len, &config, 1);
        if (ret != TLAPI_OK) {
            GF_LOGE(LOG_TAG "gf_spi_write drSpiSend failed ret = 0x%x", ret);
            err = GF_ERROR_SPI_TRANSFER_ERROR;
            break;
        }
    } while (0);

    return err;
}

gf_error_t gf_spi_read(void *tx_buf, void *rx_buf, uint32_t buf_len, gf_spi_speed_t speed) {
    gf_error_t err = GF_SUCCESS;
    struct mt_chip_conf config;
    tlApiResult_t ret;

    do {
        if (NULL == rx_buf) {
            GF_LOGE(LOG_TAG "gf_spi_read invalid parameters");
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        memcpy(&config, &g_mt_chip_conf, sizeof(struct mt_chip_conf));
        if (buf_len < 32) {
            config.com_mod = FIFO_TRANSFER;
        } else if (GF_SPI_SPEED_HIGH == speed) {
            config.high_time = SPI_6MH_HIGH_TIME;
            config.low_time = SPI_6MH_LOW_TIME;
        }

        ret = drSpiSend1(tx_buf, rx_buf, buf_len, &config, 1);
        if (ret != TLAPI_OK) {
            GF_LOGE(LOG_TAG "gf_spi_read drSpiSend failed ret = 0x%x", ret);
            err = GF_ERROR_SPI_TRANSFER_ERROR;
            break;
        }
    } while (0);

    return err;
}
