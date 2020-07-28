#ifndef __GF_SPI_H__
#define __GF_SPI_H__

#include "gf_error.h"

typedef enum {
    GF_SPI_SPEED_LOW = 0,
    GF_SPI_SPEED_HIGH
} gf_spi_speed_t;

gf_error_t gf_spi_write(void *tx_buf, void *rx_buf, uint32_t len);
gf_error_t gf_spi_read(void *tx_buf, void *rx_buf, uint32_t len, gf_spi_speed_t speed) ;

#endif // __GF_SPI_H__
