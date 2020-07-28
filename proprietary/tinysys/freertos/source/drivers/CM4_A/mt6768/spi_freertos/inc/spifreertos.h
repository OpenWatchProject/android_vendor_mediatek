/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __SPI_FREERTOS_H
#define __SPI_FREERTOS_H

#include <driver_api.h>
#include <stdint.h>
#include <stdbool.h>
#include <mt_reg_base.h>

#define SPI_HOST_NUM (2)

#define SPI_FIFO_SIZE 32
#define MAX_SIZE 1024

#define PACKET_SIZE 0x400
//#define SPI_TEST_DEBUG

//#define EINVAL      201 /* Invalid argument */
#define ELEN        202 /* LEN more than 32byte */
#define EMODE       203 /* SPI more ERROR */
#define ESTATUS     204 /* Pause Status ERROR */


enum spi_cpol {
    SPI_CPOL_IDLE_LO,
    SPI_CPOL_IDLE_HI,
};

enum spi_cpha {
    SPI_CPHA_LEADING_EDGE,
    SPI_CPHA_TRAILING_EDGE,
};

enum spi_mlsb {
    SPI_FORMAT_LSB_FIRST,
    SPI_FORMAT_MSB_FIRST,
};

struct SpiMode {
    enum spi_cpol cpol;

    enum spi_cpha cpha;

    uint8_t bitsPerWord;
    enum spi_mlsb format;

    uint16_t txWord;

    uint32_t speed;

    bool nssChange;
};

enum spi_sample_sel {
    POSEDGE,
    NEGEDGE
};
enum spi_cs_pol {
    ACTIVE_LOW,
    ACTIVE_HIGH
};

enum spi_endian {
    SPI_LENDIAN,
    SPI_BENDIAN
};

enum spi_transfer_mode {
    FIFO_TRANSFER,
    DMA_TRANSFER,
    OTHER1,
    OTHER2,
};

enum spi_pause_mode {
    PAUSE_MODE_DISABLE,
    PAUSE_MODE_ENABLE
};
enum spi_finish_intr {
    FINISH_INTR_DIS,
    FINISH_INTR_EN,
};

enum spi_deassert_mode {
    DEASSERT_DISABLE,
    DEASSERT_ENABLE
};

enum spi_ulthigh {
    ULTRA_HIGH_DISABLE,
    ULTRA_HIGH_ENABLE
};

enum spi_tckdly {
    TICK_DLY0,
    TICK_DLY1,
    TICK_DLY2,
    TICK_DLY3
};
enum spi_irq_flag {
    IRQ_IDLE,
    IRQ_BUSY
};

enum master_status {
    IDLE,
    INPROGRESS,
    PAUSED
};

struct spiStatus {
    enum master_status spi_mstatus;
    enum spi_irq_flag irq_flag;
};

struct mt_chip_conf {
    uint32_t setuptime;
    uint32_t holdtime;
    uint32_t high_time;
    uint32_t low_time;
    uint32_t cs_idletime;
    uint32_t ulthgh_thrsh;
    enum spi_sample_sel sample_sel;
    enum spi_cs_pol cs_pol;
    enum spi_cpol cpol;
    enum spi_cpha cpha;
    enum spi_mlsb tx_mlsb;
    enum spi_mlsb rx_mlsb;
    enum spi_endian tx_endian;
    enum spi_endian rx_endian;
    enum spi_transfer_mode com_mod;
    enum spi_pause_mode pause;
    enum spi_finish_intr finish_intr;
    enum spi_deassert_mode deassert;
    enum spi_ulthigh ulthigh;
    enum spi_tckdly tckdly;
};

struct spi_transfer {
    uint32_t    id;
    uint32_t    base;
    uint32_t    running;

    const void  *tx_buf;
    void        *rx_buf;
    uint32_t    len;
    uint32_t    is_dma_used;
    uint32_t    is_transfer_end; /*for clear pause bit*/

    uint8_t     is_last_xfer;

    uint32_t    tx_dma; //dma_addr_t
    uint32_t    rx_dma; //dma_addr_t
    uint32_t    is_polling;
    uint32_t    cache_line_size;

    struct spiStatus *spi_status;

    struct mt_chip_conf *chip_config;
};

/*
 * secure dirver map region (via drApiMapPhys)
 * 1. t-driver will own virtual address
 * 2. SPI used virtual address range is 0x80000 - 0xFEFFF
 */


#define SPI_REG_CFG0        (0x00)
#define SPI_REG_CFG1        (0x04)
#define SPI_REG_TX_SRC      (0x08)
#define SPI_REG_RX_DST      (0x0C)
#define SPI_REG_TX_DATA     (0x10)
#define SPI_REG_RX_DATA     (0x14)
#define SPI_REG_CMD         (0x18)
#define SPI_REG_STATUS0     (0x1C)
#define SPI_REG_STATUS1     (0x20)
#define SPI_REG_PAD_SEL     (0x24)
#define SPI_REG_CFG2        (0x28)

/************************************************************************/

#define SPI_CFG0_CS_HOLD_OFFSET             0
#define SPI_CFG0_CS_SETUP_OFFSET            16

#define SPI_CFG0_CS_HOLD_MASK               0xffff
#define SPI_CFG0_CS_SETUP_MASK              0xffff0000

#define SPI_CFG1_CS_IDLE_OFFSET             0
#define SPI_CFG1_PACKET_LOOP_OFFSET         8
#define SPI_CFG1_PACKET_LENGTH_OFFSET       16
#define SPI_CFG1_GET_TICK_DLY_OFFSET        29

#define SPI_CFG1_CS_IDLE_MASK               0xff
#define SPI_CFG1_PACKET_LOOP_MASK           0xff00
#define SPI_CFG1_PACKET_LENGTH_MASK         0x3ff0000
#define SPI_CFG1_GET_TICK_DLY_MASK          0xe0000000

#define SPI_CFG2_SCK_HIGH_OFFSET            0
#define SPI_CFG2_SCK_LOW_OFFSET             16

#define SPI_CFG2_SCK_HIGH_MASK              0xffff
#define SPI_CFG2_SCK_LOW_MASK               0xffff0000


#define SPI_CMD_ACT_OFFSET                  0
#define SPI_CMD_RESUME_OFFSET               1
#define SPI_CMD_RST_OFFSET                  2
#define SPI_CMD_PAUSE_EN_OFFSET             4
#define SPI_CMD_DEASSERT_OFFSET             5
#define SPI_CMD_SAMPLE_SEL_OFFSET           6
#define SPI_CMD_CS_POL_OFFSET               7
#define SPI_CMD_CPHA_OFFSET                 8
#define SPI_CMD_CPOL_OFFSET                 9
#define SPI_CMD_RX_DMA_OFFSET               10
#define SPI_CMD_TX_DMA_OFFSET               11
#define SPI_CMD_TXMSBF_OFFSET               12
#define SPI_CMD_RXMSBF_OFFSET               13
#define SPI_CMD_RX_ENDIAN_OFFSET            14
#define SPI_CMD_TX_ENDIAN_OFFSET            15
#define SPI_CMD_FINISH_IE_OFFSET            16
#define SPI_CMD_PAUSE_IE_OFFSET             17

#define SPI_CMD_ACT_MASK                    0x1
#define SPI_CMD_RESUME_MASK                 0x2
#define SPI_CMD_RST_MASK                    0x4
#define SPI_CMD_PAUSE_EN_MASK               0x10
#define SPI_CMD_DEASSERT_MASK               0x20
#define SPI_CMD_SAMPLE_SEL_MASK             0x40
#define SPI_CMD_CS_POL_MASK                 0x80
#define SPI_CMD_CPHA_MASK                   0x100
#define SPI_CMD_CPOL_MASK                   0x200
#define SPI_CMD_RX_DMA_MASK                 0x400
#define SPI_CMD_TX_DMA_MASK                 0x800
#define SPI_CMD_TXMSBF_MASK                 0x1000
#define SPI_CMD_RXMSBF_MASK                 0x2000
#define SPI_CMD_RX_ENDIAN_MASK              0x4000
#define SPI_CMD_TX_ENDIAN_MASK              0x8000
#define SPI_CMD_FINISH_IE_MASK              0x10000
#define SPI_CMD_PAUSE_IE_MASK               0x20000

#define SPI_ULTRA_HIGH_EN_OFFSET            0
#define SPI_ULTRA_HIGH_THRESH_OFFSET        16

#define SPI_ULTRA_HIGH_EN_MASK              0x1
#define SPI_ULTRA_HIGH_THRESH_MASK          0xffff0000


#define SPI_REG_GET32(addr)     (*(volatile uint32_t *)(addr))
#define SPI_REG_SET32(addr, val)    (*(volatile uint32_t *)(addr)) = (val)

#define SPI_CHRE_READ(base, offset)      SPI_REG_GET32(base + offset)
#define SPI_CHRE_WRITE(base, offset, val)        SPI_REG_SET32(base + offset, val)


#define SPI_BASE0       (SPI0_BASE)  //scp spi base
#define SPI_BASE1       (SPI1_BASE)


#define CLK_SCP_CG_CTRL_BASE_  (CLK_CTRL_BASE)
#define SPI_SRT_CLK_CG  (CLK_SCP_CG_CTRL_BASE_ + 0x30)
#define SPI_CLR_CLK_CG  (CLK_SCP_CG_CTRL_BASE_ + 0x58)
#define SPI0_CLK  (1 << 13)
#define SPI1_CLK  (1 << 14)

#define ULPOSC_DIV4_RATE    ((250*1000000)>>2) /* ULPOSC, Clk rate different by platform*/
#define SYS_CLK_DIV4_RATE    ((26*1000000)>>2) /* When ULPOSC not ready, clk_sys = 26M*/
#define SPI_BCLK_CK_SEL     (CLK_SCP_CG_CTRL_BASE_ + 0x4C)
#define SPI_BCLK_CK_SEL_MASK       (0xFFFFFFFC)
#define SPI_BCLK_CK_SW_STATUS_MASK (0xFFFFF0FF)
#define SET_CLK_SYS_DIV4           (0x00)
#define SET_ULPOSC_DIV4            (0x02)
#define STATUS_CLK_SYS_DIV4         (0x100)
#define STATUS_ULPOSC_DIV4          (0x400)


struct spiIrqTimer {
    uint32_t timerHandle;
    uint32_t timeOut;
};

uint32_t spi_clk_rate;

int SCPSpiRxTxSync(uint8_t busId, void *rxBuf, const void *txBuf,
                      size_t size, const struct SpiMode *mode);
void freertos_spi_init(void);


//#define SPI_TEST_DEBUG
//#define SPI_PRINT_DEBUG
#define SPI_PRINT_ERR_DEBUG
#ifdef SPI_PRINT_DEBUG
#ifndef CFG_LOG_FILTER
#define SPITAG                  "[SPI-SCP] "
#define SPI_MSG(fmt, arg...)    PRINTF_D(SPITAG fmt, ##arg)
#define SPI_ERR(fmt, arg...)    PRINTF_E(SPITAG fmt, ##arg)
#define SPI_MSG_ISR(fmt, arg...)    PRINTF_D(SPITAG fmt, ##arg) //May need porting if print in ISR is supported
#define SPI_ERR_ISR(fmt, arg...)    PRINTF_E(SPITAG fmt, ##arg) //May need porting if print in ISR is supported
#else
#define SPITAG                  "[SPI-SCP-FILTER] "
#define SPI_MSG(fmt, arg...)    mlprintf(LOG_SPI_ERROR, SPITAG fmt, ##arg)
#define SPI_ERR(fmt, arg...)    mlprintf(LOG_SPI_ERROR, SPITAG fmt, ##arg)
#define SPI_MSG_ISR(fmt, arg...)    mlprintf(LOG_SPI_DEBUG, SPITAG fmt, ##arg) //May need porting if print in ISR is supported
#define SPI_ERR_ISR(fmt, arg...)    mlprintf(LOG_SPI_ERROR, SPITAG fmt, ##arg) //May need porting if print in ISR is supported
#endif

#elif defined(SPI_PRINT_ERR_DEBUG)
#ifndef CFG_LOG_FILTER
#define SPITAG                  "[SPI-SCP] "
#define SPI_MSG(fmt, arg...)
#define SPI_ERR(fmt, arg...)    PRINTF_E(SPITAG fmt, ##arg)
#define SPI_MSG_ISR(fmt, arg...)
#define SPI_ERR_ISR(fmt, arg...)    PRINTF_E(SPITAG fmt, ##arg) //May need porting if print in ISR is supported
#else
#define SPITAG                  "[SPI-SCP-FILTER] "
#define SPI_MSG(fmt, arg...)
#define SPI_ERR(fmt, arg...)    mlprintf(LOG_SPI_ERROR, SPITAG fmt, ##arg)
#define SPI_MSG_ISR(fmt, arg...)
#define SPI_ERR_ISR(fmt, arg...)    mlprintf(LOG_SPI_ERROR, SPITAG fmt, ##arg) //May need porting if print in ISR is supported
#endif

#else
#define SPI_MSG(fmt, arg...)
#define SPI_ERR(fmt, arg...)
#define SPI_MSG_ISR(fmt, arg...)
#define SPI_ERR_ISR(fmt, arg...)
#endif



#endif /* __PLAT_SPI_H */
