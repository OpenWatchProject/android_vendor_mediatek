/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __GF_SPI_H__
#define __GF_SPI_H__

#include "gf_error.h"

// compatible with milan AB, uint:Mbps
#define GF_SPI_SPEED_LOW 1
#define GF_SPI_SPEED_MEDIUM 6
#define GF_SPI_SPEED_HIGH 9




#define GF_SPI_CPHA    0x01            /* clock phase */
#define GF_SPI_CPOL    0x02            /* clock polarity */
#define GF_SPI_MODE_0  (0|0)           /* (original MicroWire) */
#define GF_SPI_MODE_1  (0|GF_SPI_CPHA)
#define GF_SPI_MODE_2  (GF_SPI_CPOL|0)
#define GF_SPI_MODE_3  (GF_SPI_CPOL|GF_SPI_CPHA)

enum gf_spi_sample_sel {
    GF_POSEDGE,
    GF_NEGEDGE
};

enum gf_spi_cs_pol {
    GF_ACTIVE_LOW,
    GF_ACTIVE_HIGH
};

enum gf_spi_cpol {
    GF_SPI_CPOL_0,
    GF_SPI_CPOL_1
};

enum gf_spi_cpha {
    GF_SPI_CPHA_0,
    GF_SPI_CPHA_1
};

enum gf_spi_mlsb {
    GF_SPI_LSB,
    GF_SPI_MSB
};

enum gf_spi_endian {
    GF_SPI_LENDIAN,
    GF_SPI_BENDIAN
};

enum gf_spi_transfer_mode {
    GF_FIFO_TRANSFER,
    GF_DMA_TRANSFER,
};

enum gf_spi_pause_mode {
    GF_PAUSE_MODE_DISABLE,
    GF_PAUSE_MODE_ENABLE
};
enum gf_spi_finish_intr {
    GF_FINISH_INTR_DIS,
    GF_FINISH_INTR_EN,
};

enum gf_spi_deassert_mode {
    GF_DEASSERT_DISABLE,
    GF_DEASSERT_ENABLE
};

enum gf_spi_ulthigh {
    GF_ULTRA_HIGH_DISABLE,
    GF_ULTRA_HIGH_ENABLE
};

enum gf_spi_tckdly {
    GF_TICK_DLY0,
    GF_TICK_DLY1,
    GF_TICK_DLY2,
    GF_TICK_DLY3
};

enum gf_spi_irq_flag {
    GF_IRQ_IDLE,
    GF_IRQ_BUSY
};

struct gf_mt_chip_conf {
    unsigned int  setuptime;
    unsigned int holdtime;
    unsigned int high_time;
    unsigned int low_time;
    unsigned int cs_idletime;
    unsigned int ulthgh_thrsh;
    enum gf_spi_sample_sel sample_sel;
    enum gf_spi_cs_pol cs_pol;
    enum gf_spi_cpol cpol;
    enum gf_spi_cpha cpha;
    enum gf_spi_mlsb tx_mlsb;
    enum gf_spi_mlsb rx_mlsb;
    enum gf_spi_endian tx_endian;
    enum gf_spi_endian rx_endian;
    enum gf_spi_transfer_mode com_mod;
    enum gf_spi_pause_mode pause;
    enum gf_spi_finish_intr finish_intr;
    enum gf_spi_deassert_mode deassert;
    enum gf_spi_ulthigh ulthigh;
    enum gf_spi_tckdly tckdly;
};


gf_error_t gf_spi_write(void *tx_buf, void *rx_buf, uint32_t len);
gf_error_t gf_spi_read(void *tx_buf, void *rx_buf, uint32_t len, uint32_t speed);
gf_error_t gf_spi_init(void);

#endif  // __GF_SPI_H__

