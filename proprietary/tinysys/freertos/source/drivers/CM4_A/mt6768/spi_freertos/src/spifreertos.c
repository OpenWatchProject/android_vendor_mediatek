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

#include <errno.h>
#include <string.h>
#include <interrupt.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <spifreertos.h>
#include <wakelock.h>
#include <semphr.h>

#include <driver_api.h>
#include <FreeRTOS.h>

#ifdef CFG_VCORE_DVFS_SUPPORT
#include <dvfs.h>
#endif

#ifdef CFG_CACHE_SUPPORT
#include <cache_internal.h>
#include <cache_dram_management.h>
#define SPI_CACHE_LINE_SIZE MTK_CACHE_LINE_SIZE
#define upper_len(len) ((len % SPI_CACHE_LINE_SIZE) ? \
    ((len / SPI_CACHE_LINE_SIZE + 1) * SPI_CACHE_LINE_SIZE) : len)
#endif

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

struct systemResource {
    wakelock_t spi_wakelock;
    SemaphoreHandle_t spi_irq_semaphore;
    SemaphoreHandle_t spi_mutex;
    __attribute__((aligned(32))) /*DRAM_REGION_BSS*/ char recv_temp_data[MAX_SIZE];
    __attribute__((aligned(32))) /*DRAM_REGION_BSS*/ char send_temp_data[MAX_SIZE];
};

struct systemResource spiResource[2] = {};
struct spiStatus spiMstatus[2] = {};

static struct mt_chip_conf mt_chip_spi_conf = {
    .setuptime = 10,
    .holdtime = 10,
    .high_time = 5,
    .low_time = 5,
    .cs_idletime = 10,
    .ulthgh_thrsh = 0,
    .cpol = 0,
    .cpha = 0,
    .rx_mlsb = 1,
    .tx_mlsb = 1,
    .tx_endian = 0,
    .rx_endian = 0,
    .com_mod = DMA_TRANSFER,
    .pause = 0,
    .finish_intr = 1,
    .deassert = 0,
    .ulthigh = 0,
    .tckdly = 0
};

static struct SpiMode spi_mode = {
    .speed = 8000000,
    .bitsPerWord = 8,
    .cpol = SPI_CPOL_IDLE_LO,
    .cpha = SPI_CPHA_LEADING_EDGE,
    .nssChange = true,
    .format = SPI_FORMAT_MSB_FIRST,
};

static void SCPSpiTimeoutHandler(struct spi_transfer *xfer);
static void SCPSpiPollingHandle(struct spi_transfer *xfer_intr);
//static void dump_reg_config(uint32_t base);
static void ResetSpi(uint32_t base);


static struct spi_transfer spi_dev[] = {
    {
        .id = 0,
        .base = SPI_BASE0,
        .is_polling = 0,
    },
    {
        .id = 1,
        .base = SPI_BASE1,
        .is_polling = 0,
    },
};

/* May call Spi_get_lock from non-interrupt and interrupt context */
static int Spi_get_lock(uint32_t id)
{
    if(id < SPI_HOST_NUM)
    {
        if (is_in_isr())
        {
            wake_lock_FromISR(&spiResource[id].spi_wakelock);
        }
        else
        {
            wake_lock(&spiResource[id].spi_wakelock);
        }
        SPI_MSG("Spi_get_lock\n");
    }
    else
    {
        SPI_MSG("get lock invalid id=%d\n", (int)id);
        return -1;
    }
    return 0;
}


/* May call Spi_release_lock from non-interrupt and interrupt context */
static void Spi_release_lock(uint32_t id)
{
    if(id < SPI_HOST_NUM)
    {
        if (is_in_isr())
        {
            wake_unlock_FromISR(&spiResource[id].spi_wakelock);
        }
        else
        {
            wake_unlock(&spiResource[id].spi_wakelock);
        }
        SPI_MSG("Spi_release_lock\n");
    }
    else
    {
        SPI_MSG("release lock invalid id=%d\n", (int)id);
    }
}


static void Spi_enable_clk(uint32_t id)
{
    switch (id) {
        case 0:
            DRV_WriteReg32(SPI_SRT_CLK_CG, SPI0_CLK);
            break;
        case 1:
            DRV_WriteReg32(SPI_SRT_CLK_CG, SPI1_CLK);
            break;
        default:
            SPI_MSG("enable clk invalid id=%d\n", (int)id);
            break;
    }
    SPI_MSG("Spi_enable_clk\n");
}

static void Spi_disable_clk(uint32_t id)
{
    switch (id) {
        case 0:
            DRV_WriteReg32(SPI_CLR_CLK_CG, SPI0_CLK);
            break;
        case 1:
            DRV_WriteReg32(SPI_CLR_CLK_CG, SPI1_CLK);
            break;
        default:
            SPI_MSG("disable clk invalid id=%d\n", (int)id);
            break;
    }
    SPI_MSG("Spi_disable_clk\n");
}

//This function is used for polling mode
static void SpiPollingHandler(struct spi_transfer *xfer)
{
    volatile uint32_t irqStatus = 0;

    do {
        irqStatus = SPI_CHRE_READ(xfer->base, SPI_REG_STATUS1) & 0x00000001;
        //      SPI_MSG("Polling IRQ: irqStatus[0x%08x],xfer->id =%d\n", irqStatus, xfer->id);
    } while (irqStatus == 0);

    //SPI_MSG("Polling : Status1 irqStatus[0x%08x],xfer->id =%d\n", (unsigned int)irqStatus, (int)xfer->id);

    switch (xfer->id) {
        case 0:
        case 1:
            SCPSpiPollingHandle(xfer);
            break;
        default:
            SPI_MSG("SpiPollingHandler invalid ID:%d\n", (int)xfer->id);
            break;
    }

}


//This function is used for interrupt mode
void spi0_tz_irq_handler(void)
{
    uint32_t irqStatus;
    portBASE_TYPE taskWoken = pdFALSE;
    irqStatus = SPI_CHRE_READ(SPI_BASE0, SPI_REG_STATUS0) & 0x00000003; //Read clear IRQ status

    //GIC dispatch spi irq unexpected, while spi HW never issued interrupt.
    if (!irqStatus) {
        SPI_ERR_ISR("No irq captured in spi0 ISR\n");
        return;
    }
#ifdef SPI_PRINT_DEBUG
    SPI_MSG_ISR("IRQ SPI0: irqStatus[0x%08x]\n", (unsigned int)irqStatus);
#endif

    SCPSpiPollingHandle(&spi_dev[0]);
    xSemaphoreGiveFromISR(spiResource[0].spi_irq_semaphore, &taskWoken);
    portYIELD_FROM_ISR(taskWoken);

}

//This function is used for interrupt mode
void spi1_tz_irq_handler(void)
{
    uint32_t irqStatus;
    portBASE_TYPE taskWoken = pdFALSE;
    irqStatus = SPI_CHRE_READ(SPI_BASE1, SPI_REG_STATUS0) & 0x00000003; //Read clear IRQ status

    //GIC dispatch spi irq unexpected, while spi HW never issued interrupt.
    if (!irqStatus) {
        SPI_ERR_ISR("No irq captured in spi1 ISR\n");
        return;
    }
#ifdef SPI_PRINT_DEBUG
    SPI_MSG_ISR("IRQ SPI1: irqStatus[0x%08x]\n", (unsigned int)irqStatus);
#endif

    SCPSpiPollingHandle(&spi_dev[1]);

    xSemaphoreGiveFromISR(spiResource[1].spi_irq_semaphore, &taskWoken);
    portYIELD_FROM_ISR(taskWoken);

}

static void mt_mask_and_clear_int(uint32_t id)
{
    uint32_t base = 0, reg_val = 0;

    switch (id) {
        case 0:
            base = SPI_BASE0;
            break;
        case 1:
            base = SPI_BASE1;
            break;
        default:
            break;
    }

    if (base) {
        /* mask all interrupt */
        reg_val = SPI_CHRE_READ(base, SPI_REG_CMD);
        reg_val &= ~ (SPI_CMD_FINISH_IE_MASK | SPI_CMD_PAUSE_IE_OFFSET);
        SPI_CHRE_WRITE(base, SPI_REG_CMD, reg_val);

        /* clear all interrupt */
        SPI_CHRE_READ(base, SPI_REG_STATUS0);
    }
}

static void SpiRecvCheck(struct spi_transfer *xfer)
{
#ifdef SPI_TEST_DEBUG
    uint32_t cnt, i;

    SPI_MSG_ISR("SpiRecvCheck\n");

    cnt = (xfer->len % 4) ? (xfer->len / 4 + 1) : (xfer->len / 4);
    for (i = 0; i < cnt; i++) {
       // if (*((uint32_t *) xfer->rx_dma + i) != *((uint32_t *) xfer->tx_dma + i)) {
            SPI_ERR_ISR("Error!! tx dma %d is:%x\n", (int)i, (unsigned int)(*((uint32_t *) xfer->tx_dma + i)));
            SPI_ERR_ISR("Error!! rx dma %d is:%x\n", (int)i, (unsigned int)(*((uint32_t *) xfer->rx_dma + i)));
      // }
    }
#endif
}

static void dump_reg_config(uint32_t base)
{
    SPI_MSG("SPI_REG_CFG0=0x%x\n", (unsigned int)SPI_CHRE_READ(base, SPI_REG_CFG0));
    SPI_MSG("SPI_REG_CFG1=0x%x\n", (unsigned int)SPI_CHRE_READ(base, SPI_REG_CFG1));
    SPI_MSG("SPI_REG_CFG2=0x%x\n", (unsigned int)SPI_CHRE_READ(base, SPI_REG_CFG2));
    SPI_MSG("SPI_REG_TX_SRC=0x%x\n", (unsigned int)SPI_CHRE_READ(base, SPI_REG_TX_SRC));
    SPI_MSG("SPI_REG_RX_DST=0x%x\n", (unsigned int)SPI_CHRE_READ(base, SPI_REG_RX_DST));
    SPI_MSG("SPI_REG_CMD=0x%x\n", (unsigned int)SPI_CHRE_READ(base, SPI_REG_CMD));
    SPI_MSG("SPI_REG_PAD_SEL=0x%x\n", (unsigned int)SPI_CHRE_READ(base, SPI_REG_PAD_SEL));
}

static void SCPSpiTimeoutHandler(struct spi_transfer *xfer)
{
    xfer->spi_status->spi_mstatus = IDLE;
    ResetSpi(xfer->base);
    xfer->spi_status->irq_flag = IRQ_IDLE;
    dump_reg_config(xfer->base);
}

static void SCPSpiPollingHandle(struct spi_transfer *xfer_intr)
{
    struct mt_chip_conf *chip_config_intr = NULL;
    uint32_t reg_val, cnt;
    uint32_t i;
    uint32_t base = xfer_intr->base;

    SPI_MSG("SpiHandleIRQ%x\n", (unsigned int)xfer_intr->id);

    chip_config_intr = xfer_intr->chip_config;

    if (NULL == chip_config_intr) {
        SPI_MSG_ISR("NULL chip_config!\n");
        return;
    }

    /*pause mode*/
    if (chip_config_intr->pause) {
        if (xfer_intr->spi_status->spi_mstatus == INPROGRESS) {
            xfer_intr->spi_status->spi_mstatus = PAUSED;
            SPI_MSG_ISR("IRQ set PAUSED state\n");
        } else {
            SPI_ERR_ISR("Invalid status.\n");
        }
    } else {
        xfer_intr->spi_status->spi_mstatus = IDLE;
        SPI_MSG_ISR("IRQ set IDLE state\n");
    }

    SPI_MSG_ISR("start to read data !!\n");
    /*FIFO*/
    if ((chip_config_intr->com_mod == FIFO_TRANSFER) && xfer_intr->rx_buf) {
        SPI_MSG_ISR("xfer->len:%d\n", (int)xfer_intr->len);
        cnt = (xfer_intr->len % 4) ? (xfer_intr->len / 4 + 1) : (xfer_intr->len / 4);
        for (i = 0; i < cnt; i++) {
            reg_val = SPI_CHRE_READ(base, SPI_REG_RX_DATA); /*get the data from rx*/
            SPI_MSG_ISR("SPI_RX_DATA_REG:0x%x\n", (unsigned int)reg_val);
            *((uint32_t *)xfer_intr->rx_buf + i) = reg_val;
        }
    }

    if (chip_config_intr->com_mod == DMA_TRANSFER)
        SpiRecvCheck(xfer_intr);

    if (xfer_intr->is_last_xfer == 1 && xfer_intr->is_transfer_end == 1) {
        xfer_intr->spi_status->spi_mstatus = IDLE;
        ResetSpi(base);
        SPI_MSG_ISR("Pause set IDLE state & disable clk\n");

    }

    /*set irq flag to ask SpiNextMessage continue to run*/

    xfer_intr->spi_status->irq_flag = IRQ_IDLE;

    SPI_MSG_ISR("SCPSpiHandle-----------xfer end-------\n");
    return;
}

#if 0
static void dump_chip_config(struct mt_chip_conf *chip_config)
{
    if (chip_config != NULL) {
        SPI_MSG("setuptime=%d\n", (int)chip_config->setuptime);
        SPI_MSG("holdtime=%d\n", (int)chip_config->holdtime);
        SPI_MSG("high_time=%d\n", (int)chip_config->high_time);
        SPI_MSG("low_time=%d\n", (int)chip_config->low_time);
        SPI_MSG("cs_idletime=%d\n", (int)chip_config->cs_idletime);
        SPI_MSG("ulthgh_thrsh=%d\n", (int)chip_config->ulthgh_thrsh);
        SPI_MSG("cpol=%d\n", chip_config->cpol);
        SPI_MSG("cpha=%d\n", chip_config->cpha);
        SPI_MSG("tx_mlsb=%d\n", chip_config->tx_mlsb);
        SPI_MSG("rx_mlsb=%d\n", chip_config->rx_mlsb);
        SPI_MSG("tx_endian=%d\n", chip_config->tx_endian);
        SPI_MSG("rx_endian=%d\n", chip_config->rx_endian);
        SPI_MSG("com_mod=%d\n", chip_config->com_mod);
        SPI_MSG("pause=%d\n", chip_config->pause);
        SPI_MSG("finish_intr=%d\n", chip_config->finish_intr);
        SPI_MSG("deassert=%d\n", chip_config->deassert);
        SPI_MSG("ulthigh=%d\n", chip_config->ulthigh);
        SPI_MSG("tckdly=%d\n", chip_config->tckdly);
    } else {
        SPI_ERR("NULL chip_config!\n");
    }
}
#endif

static uint32_t IsInterruptEnable(uint32_t base)
{
    uint32_t cmd;
    cmd = SPI_CHRE_READ(base, SPI_REG_CMD);
    return (cmd >> SPI_CMD_FINISH_IE_OFFSET) & 1;
}

static void inline clear_pause_bit(uint32_t base)
{
    uint32_t reg_val;

    reg_val = SPI_CHRE_READ(base, SPI_REG_CMD);
    reg_val &= ~SPI_CMD_PAUSE_EN_MASK;
    SPI_CHRE_WRITE(base, SPI_REG_CMD, reg_val);
}

static void inline SetPauseBit(uint32_t base)
{
    uint32_t reg_val;

    reg_val = SPI_CHRE_READ(base, SPI_REG_CMD);
    reg_val |= 1 << SPI_CMD_PAUSE_EN_OFFSET;
    SPI_CHRE_WRITE(base, SPI_REG_CMD, reg_val);
}

static void inline ClearResumeBit(uint32_t base)
{
    uint32_t reg_val;

    reg_val = SPI_CHRE_READ(base, SPI_REG_CMD);
    reg_val &= ~SPI_CMD_RESUME_MASK;
    SPI_CHRE_WRITE(base, SPI_REG_CMD, reg_val);
}

/*
*  SpiSetupPacket: used to define per data length and loop count
* @ ptr : data structure from User
*/
static int inline SpiSetupPacket(struct spi_transfer *xfer)
{
    uint32_t packet_size = 0;
    uint32_t packet_loop = 0;
    uint32_t cfg1 = 0;
    uint32_t base = xfer->base;

    /*set transfer packet and loop*/
    if (xfer->len < PACKET_SIZE)
        packet_size = xfer->len;
    else
        packet_size = PACKET_SIZE;

    if (xfer->len % packet_size) {
        ///packet_loop = xfer->len/packet_size + 1;
        /*parameter not correct, there will be more data transfer,notice user to change*/
        SPI_MSG("Lens(%u) not multiple of %d\n", (unsigned int)xfer->len, PACKET_SIZE);
    }
    packet_loop = (xfer->len) / packet_size;

    cfg1 = SPI_CHRE_READ(base, SPI_REG_CFG1);
    cfg1 &= ~(SPI_CFG1_PACKET_LENGTH_MASK + SPI_CFG1_PACKET_LOOP_MASK);
    cfg1 |= (packet_size - 1) << SPI_CFG1_PACKET_LENGTH_OFFSET;
    cfg1 |= (packet_loop - 1) << SPI_CFG1_PACKET_LOOP_OFFSET;
    SPI_CHRE_WRITE(base, SPI_REG_CFG1, cfg1);

    return 0;
}

static void inline SpiStartTransfer(uint32_t base)
{
    uint32_t reg_val;
    reg_val = SPI_CHRE_READ(base, SPI_REG_CMD);
    reg_val |= 1 << SPI_CMD_ACT_OFFSET;

    /*All register must be prepared before setting the start bit [SMP]*/
    SPI_CHRE_WRITE(base, SPI_REG_CMD, reg_val);

    return;
}

static void inline SpiDisableDma(uint32_t base)
{
    uint32_t  cmd;

    cmd = SPI_CHRE_READ(base, SPI_REG_CMD);
    cmd &= ~SPI_CMD_TX_DMA_MASK;
    cmd &= ~SPI_CMD_RX_DMA_MASK;
    SPI_CHRE_WRITE(base, SPI_REG_CMD, cmd);
}

#define INVALID_DMA_ADDRESS 0xffffffff

static inline void SpiEnableDma(struct spi_transfer *xfer, uint32_t mode)
{
    uint32_t cmd;
    uint32_t base = xfer->base;

    cmd = SPI_CHRE_READ(base, SPI_REG_CMD);
    SPI_MSG("SpiEnableDma tx_dma=0x%x,rx_dma=0x%x\n", (unsigned int)xfer->tx_dma , (unsigned int)xfer->rx_dma);


#define SPI_4B_ALIGN 0x4
    /* set up the DMA bus address */
    if ((mode == DMA_TRANSFER) || (mode == OTHER1)) {
        if ((xfer->tx_buf != NULL) || ((xfer->tx_dma != INVALID_DMA_ADDRESS) && (xfer->tx_dma != 0))) {
            if (xfer->tx_dma & (SPI_4B_ALIGN - 1)) {
                SPI_ERR("Tx_DMA addr not 4B align,buf:%p,dma:%x\n",
                        xfer->tx_buf, (unsigned int)xfer->tx_dma);
            }
        SPI_CHRE_WRITE(base, SPI_REG_TX_SRC, xfer->tx_dma);
        cmd |= 1 << SPI_CMD_TX_DMA_OFFSET;
        }
    }
    if ((mode == DMA_TRANSFER) || (mode == OTHER2)) {
        if ((xfer->rx_buf != NULL) || ((xfer->rx_dma != INVALID_DMA_ADDRESS) && (xfer->rx_dma != 0))) {
            if (xfer->rx_dma & (SPI_4B_ALIGN - 1)) {
                SPI_ERR("Rx_DMA addr not 4B align,buf:%p,dma:%x\n",
                        xfer->rx_buf, (unsigned int)xfer->rx_dma);
            }
            SPI_CHRE_WRITE(base, SPI_REG_RX_DST, xfer->rx_dma);
            cmd |= 1 << SPI_CMD_RX_DMA_OFFSET;
        }
    }

    SPI_MSG("SPI_REG_TX_SRC=0x%x\n", (unsigned int)SPI_CHRE_READ(base, SPI_REG_TX_SRC));
    SPI_MSG("SPI_REG_RX_DST=0x%x\n", (unsigned int)SPI_CHRE_READ(base, SPI_REG_RX_DST));

    SPI_CHRE_WRITE(base, SPI_REG_CMD, cmd);
}

static void  inline SpiResumeTransfer(uint32_t base)
{
    uint32_t reg_val;

    reg_val = SPI_CHRE_READ(base, SPI_REG_CMD);
    reg_val &= ~SPI_CMD_RESUME_MASK;
    reg_val |= 1 << SPI_CMD_RESUME_OFFSET;
    /*All register must be prepared before setting the start bit [SMP]*/
    SPI_CHRE_WRITE(base, SPI_REG_CMD, reg_val);

    return;
}

static void ResetSpi(uint32_t base)
{
    uint32_t reg_val;

    /*set the software reset bit in SPI_REG_CMD.*/
    reg_val = SPI_CHRE_READ(base, SPI_REG_CMD);
    reg_val &= ~SPI_CMD_RST_MASK;
    reg_val |= 1 << SPI_CMD_RST_OFFSET;
    SPI_CHRE_WRITE(base, SPI_REG_CMD, reg_val);

    reg_val = SPI_CHRE_READ(base, SPI_REG_CMD);
    reg_val &= ~SPI_CMD_RST_MASK;
    SPI_CHRE_WRITE(base, SPI_REG_CMD, reg_val);

    SPI_MSG("ResetSpi!\n");

    return;
}


/*
* Write chip configuration to HW register
*/
static int SpiSetup(struct spi_transfer *xfer)
{
    uint32_t reg_val, base;
    struct mt_chip_conf *chip_config = xfer->chip_config;
    int ret = 0;

    base = xfer->base;

    if (chip_config == NULL) {
        SPI_MSG("SpiSetup chip_config is NULL !!\n");
        return -EINVAL;
    } else {
        /*set the timing*/
        reg_val = SPI_CHRE_READ(base, SPI_REG_CFG2);
        reg_val &= ~(SPI_CFG2_SCK_HIGH_MASK | SPI_CFG2_SCK_LOW_MASK);
        reg_val |= ((chip_config->high_time - 1) << SPI_CFG2_SCK_HIGH_OFFSET);
        reg_val |= ((chip_config->low_time - 1) << SPI_CFG2_SCK_LOW_OFFSET);
        SPI_CHRE_WRITE(base, SPI_REG_CFG2, reg_val);

        reg_val = SPI_CHRE_READ(base, SPI_REG_CFG0);
        reg_val &= ~(SPI_CFG0_CS_HOLD_MASK | SPI_CFG0_CS_SETUP_MASK);
        reg_val |= ((chip_config->holdtime - 1) << SPI_CFG0_CS_HOLD_OFFSET);
        reg_val |= ((chip_config->setuptime - 1) << SPI_CFG0_CS_SETUP_OFFSET);
        SPI_CHRE_WRITE(base, SPI_REG_CFG0, reg_val);

        reg_val = SPI_CHRE_READ(base, SPI_REG_CFG1);
        reg_val &= ~(SPI_CFG1_CS_IDLE_MASK);
        reg_val |= ((chip_config->cs_idletime - 1) << SPI_CFG1_CS_IDLE_OFFSET);

        reg_val &= ~(SPI_CFG1_GET_TICK_DLY_MASK);
        reg_val |= ((chip_config->tckdly) << SPI_CFG1_GET_TICK_DLY_OFFSET);
        SPI_CHRE_WRITE(base, SPI_REG_CFG1, reg_val);

        /*config CFG1 bit[28:26] is 0*/
        reg_val = SPI_CHRE_READ(base, SPI_REG_CFG1);
        reg_val &= ~(0x7 << 26);
        SPI_CHRE_WRITE(base, SPI_REG_CFG1, reg_val);
#ifndef SPI_TEST_DEBUG
        SPI_CHRE_WRITE(base, SPI_REG_PAD_SEL, 0x0);
#else
        SPI_CHRE_WRITE(base, SPI_REG_PAD_SEL, 0x7);
#endif
        /*set the mlsbx and mlsbtx*/
        reg_val = SPI_CHRE_READ(base, SPI_REG_CMD);
        reg_val &= ~(SPI_CMD_TX_ENDIAN_MASK | SPI_CMD_RX_ENDIAN_MASK);
        reg_val &= ~(SPI_CMD_TXMSBF_MASK | SPI_CMD_RXMSBF_MASK);
        reg_val &= ~(SPI_CMD_CPHA_MASK | SPI_CMD_CPOL_MASK);
        reg_val &= ~(SPI_CMD_SAMPLE_SEL_MASK | SPI_CMD_CS_POL_MASK);
        reg_val |= (chip_config->tx_mlsb << SPI_CMD_TXMSBF_OFFSET);
        reg_val |= (chip_config->rx_mlsb << SPI_CMD_RXMSBF_OFFSET);
        reg_val |= (chip_config->tx_endian << SPI_CMD_TX_ENDIAN_OFFSET);
        reg_val |= (chip_config->rx_endian << SPI_CMD_RX_ENDIAN_OFFSET);
        reg_val |= (chip_config->cpha << SPI_CMD_CPHA_OFFSET);
        reg_val |= (chip_config->cpol << SPI_CMD_CPOL_OFFSET);
        reg_val |= (chip_config->sample_sel << SPI_CMD_SAMPLE_SEL_OFFSET);
        reg_val |= (chip_config->cs_pol << SPI_CMD_CS_POL_OFFSET);
        SPI_CHRE_WRITE(base, SPI_REG_CMD, reg_val);

        if (xfer->is_polling) { //if not last xfer us polling
            reg_val = SPI_CHRE_READ(base, SPI_REG_CMD);
            reg_val &= ~ SPI_CMD_PAUSE_EN_MASK;
            reg_val &= ~ SPI_CMD_PAUSE_IE_MASK;/*disable pause IE in polling mode*/
            reg_val &= ~ SPI_CMD_FINISH_IE_MASK;/*disable finish IE in polling mode*/
            reg_val |= (chip_config->pause << SPI_CMD_PAUSE_EN_OFFSET);
            SPI_CHRE_WRITE(base, SPI_REG_CMD, reg_val);
        } else {
            /*set finish interrupt always enable*/
            reg_val = SPI_CHRE_READ(base, SPI_REG_CMD);
            reg_val &= ~ SPI_CMD_FINISH_IE_MASK;
            reg_val &= ~ SPI_CMD_PAUSE_IE_MASK;
            reg_val &= ~ SPI_CMD_PAUSE_EN_MASK;
            reg_val |= (1 << SPI_CMD_FINISH_IE_OFFSET);
            reg_val |= (chip_config->pause << SPI_CMD_PAUSE_IE_OFFSET);
            reg_val |= (chip_config->pause << SPI_CMD_PAUSE_EN_OFFSET);
            SPI_CHRE_WRITE(base, SPI_REG_CMD, reg_val);
        }

        /*set the communication of mode*/
        reg_val = SPI_CHRE_READ(base, SPI_REG_CMD);
        reg_val &= ~ SPI_CMD_TX_DMA_MASK;
        reg_val &= ~ SPI_CMD_RX_DMA_MASK;
        SPI_CHRE_WRITE(base, SPI_REG_CMD, reg_val);

        /*set deassert mode*/
        reg_val = SPI_CHRE_READ(base, SPI_REG_CMD);
        reg_val &= ~SPI_CMD_DEASSERT_MASK;
        reg_val |= (chip_config->deassert << SPI_CMD_DEASSERT_OFFSET);
        SPI_CHRE_WRITE(base, SPI_REG_CMD, reg_val);
    }
    return ret;
}

static int SpiTransfer(struct spi_transfer *xfer)
{
    int ret = 0;
    BaseType_t wait_smp;
    uint32_t   mode, cnt, i;
    struct mt_chip_conf *chip_config = xfer->chip_config;

    if ((!xfer)) {
        SPI_ERR("Transfer:NULL xfer.\n");
        goto out;
    }

    if (chip_config == NULL) {
        SPI_MSG("SpiNextXfer NULL chip_config\n");
        goto out;
    }

    /*wait intrrupt had been clear*/
    while (IRQ_BUSY == xfer->spi_status->irq_flag) {
        /*need a pause instruction to avoid unknow exception*/
        SPI_MSG("wait IRQ handle finish\n");
    }

    /*set flag to block next transfer*/
    xfer->spi_status->irq_flag = IRQ_BUSY;

    ret = SpiSetup(xfer);
    if (ret < 0)
        return ret;

    if ((!IsInterruptEnable(xfer->base))) {
        SPI_MSG("interrupt is disable\n");
    }

    mode = chip_config->com_mod;

    if (mode == FIFO_TRANSFER) {
        if (xfer->len > SPI_FIFO_SIZE) {
            ret = -ELEN;
            SPI_ERR("invalid xfer len\n");
            goto fail;
        }
    }

    /*
       * cannot 1K align & FIFO->DMA need used pause mode
       * this is to clear pause bit (CS turn to idle after data transfer done)
    */
    if (mode == DMA_TRANSFER) {
        if ((xfer->is_last_xfer == 1) && (xfer->is_transfer_end == 1))
            clear_pause_bit(xfer->base);
    } else if (mode == FIFO_TRANSFER) {
        if (xfer->is_transfer_end == 1)
            clear_pause_bit(xfer->base);
    } else {
        SPI_MSG("invalid xfer mode\n");
        ret = -EMODE;
        goto fail;
    }

    //SetPauseStatus(IDLE); //runing status, nothing about pause mode
    //disable DMA
    SpiDisableDma(xfer->base);

    /*need setting transfer data length & loop count*/
    SpiSetupPacket(xfer);

    /*Using FIFO to send data*/
    if (mode == FIFO_TRANSFER) {
        cnt = (xfer->len % 4) ? (xfer->len / 4 + 1) : (xfer->len / 4);
        for (i = 0; i < cnt; i++) {
            SPI_CHRE_WRITE(xfer->base, SPI_REG_TX_DATA, *((uint32_t *) xfer->tx_buf + i));
            SPI_MSG("tx_buf data is:%x\n", (unsigned int)(*((uint32_t *) xfer->tx_buf + i)));
            SPI_MSG("tx_buf addr is:%p\n", (uint32_t *)xfer->tx_buf + i);
        }
    }
    /*Using DMA to send data*/
    if (mode == DMA_TRANSFER) {
        SpiEnableDma(xfer, mode);
    }

    //SpiRecvCheck(xfer);
    SPI_MSG("xfer->id = %d, running=%d.\n", (int)xfer->id, (int)xfer->spi_status->spi_mstatus);

    if (xfer->spi_status->spi_mstatus == PAUSED) { //running
        SPI_MSG("pause status to resume.\n");
        xfer->spi_status->spi_mstatus = INPROGRESS;  //set running status

        SpiResumeTransfer(xfer->base);
    } else if (xfer->spi_status->spi_mstatus == IDLE) {
        SPI_MSG("The xfer start\n");
        /*if there is only one transfer, pause bit should not be set.*/
        if ((chip_config->pause)) { // need to think whether is last  msg <&& !is_last_xfer(msg,xfer)>
            SPI_MSG("set pause mode.\n");
            SetPauseBit(xfer->base);
        }
        /*All register must be prepared before setting the start bit [SMP]*/

        xfer->spi_status->spi_mstatus = INPROGRESS;  //set running status

        SpiStartTransfer(xfer->base);
        SPI_MSG("SpiStartTransfer\n");
    } else {
        SPI_MSG("Invalid pause status\n");
        ret = -ESTATUS;
        goto fail;
    }
    //dump_reg_config(xfer->base);
    //dump_chip_config(xfer->chip_config);

    if (xfer->is_polling) //If pause mode is used, first transfer use polling, second transfer use interrupt -> spix_tz_irq_handler
        SpiPollingHandler(xfer); //ex: transfer 1048 byte data, first 1024 use polling, second 24 byte transfer use interrupt
    else {
        wait_smp = xSemaphoreTake(spiResource[xfer->id].spi_irq_semaphore, 2000 / portTICK_RATE_MS);
        if (wait_smp == pdTRUE)
            SPI_MSG("Spi interrupt wait success\n");
        else {
            SPI_ERR("Spi interrupt wait timeout\n");
            SCPSpiTimeoutHandler(xfer);
            SPI_CHRE_READ(xfer->base, SPI_REG_STATUS1);
            ret = -EMODE;
        }
    }

    //Spi_disable_clk is disabled in second interrupt spix_tz_irq_handler -> SCPSpiDone -> Spi_disable_clk()
    /*exit pause mode*/
    if (xfer->spi_status->spi_mstatus == PAUSED && xfer->is_last_xfer == 1) {
        ClearResumeBit(xfer->base);
    }

    return ret;

fail:
    xfer->spi_status->spi_mstatus = IDLE;  //set running status
    ResetSpi(xfer->base);
    xfer->spi_status->irq_flag = IRQ_IDLE;
    return ret;

out:
    return -EINVAL;

}


static int spi_handle(struct spi_transfer* spiData, const struct SpiMode *mode)
{
    int ret = 0;
    uint32_t packet_loop, rest_size;
    uint32_t tmp_speed;
    uint32_t divider, sck_time;

    if (mode == NULL)
        mode = &spi_mode;
    spiData->is_last_xfer = 1;
    Spi_enable_clk(spiData->id);

    SPI_MSG("tx_dma=0x%x,rx_dma=0x%x\n", (unsigned int)spiData->tx_dma , (unsigned int)spiData->rx_dma);
    SPI_MSG("xfer.len=%d, xfer.is_dma_used=%d\n ", (int)spiData->len, (int)spiData->is_dma_used);
    SPI_MSG("IRQ status=%d\n ", (int)spiData->spi_status->irq_flag);
    while (IRQ_BUSY == spiData->spi_status->irq_flag) {
        /*need a pause instruction to avoid unknow exception*/
        SPI_MSG("IPC wait IRQ handle finish\n");
    }

    /* Initialize CPOL, CPHA, DATA MSB/LSM and SPI speed */
    spiData->chip_config->cpol = mode->cpol;
    spiData->chip_config->cpha = mode->cpha;
    if(mode->format == SPI_FORMAT_MSB_FIRST)
    {
        spiData->chip_config->tx_mlsb = 1;
        spiData->chip_config->rx_mlsb = 1;
    }
    else
    {
        spiData->chip_config->tx_mlsb = 0;
        spiData->chip_config->rx_mlsb = 0;
    }

    tmp_speed = mode->speed;
    if (tmp_speed == 0) {
        SPI_MSG("Invalid spi speed 0Hz!! Use 8M instead.\n");
        tmp_speed = 8*1000000;
    }
    if (tmp_speed > (spi_clk_rate>>1)) {
        SPI_MSG("Too high spi speed(%d)! Use max spi_clk_rate/2 instead(%d).\n",
            tmp_speed, (spi_clk_rate>>1) );
        tmp_speed = spi_clk_rate>>1;
    }

    divider = (spi_clk_rate + (tmp_speed>>1)) / tmp_speed;
    sck_time = (divider+1)>>1;

    spiData->chip_config->high_time = sck_time;
    spiData->chip_config->low_time = sck_time;

#ifndef NO_DMA_LENGTH_LIMIT
    packet_loop = spiData->len / 1024;
    rest_size = spiData->len % 1024;
    SPI_MSG("packet_loop=%d,rest_size=%d\n", (int)packet_loop, (int)rest_size);

    if (spiData->len <= 1024 || rest_size == 0) {
        ret = SpiTransfer(spiData);
    } else {
        spiData->chip_config->pause = 1;
        spiData->is_last_xfer = 0;
        spiData->len = 1024 * packet_loop;
        ret = SpiTransfer(spiData); //Use polling here
        if (ret < 0)
            goto fail;

        spiData->chip_config->pause = 0;
        spiData->is_last_xfer = 1;
        spiData->len = rest_size;
        spiData->rx_dma = spiData->rx_dma + 1024 * packet_loop;
        spiData->tx_dma = spiData->tx_dma + 1024 * packet_loop;

        ret = SpiTransfer(spiData); //Use interrupt here
        /* If Transfer success, disable spi clk in IRQ handler done ->  SCPSpiDone() */
        if (ret < 0)
            goto fail;
        //xfer.rx_buf = xfer_p->rx_buf;
    }
#else
    ret = SpiTransfer(spiData);
    if (ret < 0)
        goto fail;
#endif

fail:
    Spi_disable_clk(spiData->id);
    return ret;
}

int SCPSpiRxTxSync(uint8_t busId, void *rxBuf, const void *txBuf,
                      size_t size, const struct SpiMode *mode)
{
    int ret = 0;

    struct spi_transfer *xfer_p = &spi_dev[busId];

    if (busId >= (ARRAY_SIZE(spi_dev)) || (size > MAX_SIZE)) {
        SPI_ERR("busid error or max transmission capacity beyond 1024\n");
        configASSERT(0);
    }
    if (rxBuf == NULL && txBuf == NULL) {
        SPI_ERR("Error!!! spi slave data buffer is NULL.\n");
        configASSERT(0);
    }
    SPI_MSG("SCPSpiRxTx Start\n");
    xSemaphoreTake(spiResource[busId].spi_mutex, portMAX_DELAY);
    SPI_ERR("SCPSpiRxTx transfer size = %d\n", size);
    ret = Spi_get_lock(xfer_p->id);
    if(ret != 0)
        return ret;
    if (size > (SPI_FIFO_SIZE)) {
        xfer_p->chip_config = &mt_chip_spi_conf;
        xfer_p->chip_config->com_mod = DMA_TRANSFER;
        xfer_p->is_dma_used = 1;
#ifdef CFG_CACHE_SUPPORT
        memset(&spiResource[busId].recv_temp_data, 0, MAX_SIZE);
        memset(&spiResource[busId].send_temp_data, 0, MAX_SIZE);
        if (txBuf != NULL)
            memcpy(&spiResource[busId].send_temp_data, txBuf, size);
        xfer_p->tx_dma = (unsigned int)&spiResource[busId].send_temp_data;
        xfer_p->rx_dma = (unsigned int)&spiResource[busId].recv_temp_data;
#else
        xfer_p->rx_dma = (unsigned int)rxBuf;
        xfer_p->tx_dma = (unsigned int)txBuf;
#endif
    } else {
        memset(&spiResource[busId].recv_temp_data, 0, SPI_FIFO_SIZE);
        memset(&spiResource[busId].send_temp_data, 0, SPI_FIFO_SIZE);
        if (txBuf != NULL)
            memcpy(&spiResource[busId].send_temp_data, txBuf, size);
        xfer_p->tx_buf = &spiResource[busId].send_temp_data;
        xfer_p->rx_buf = &spiResource[busId].recv_temp_data;
        xfer_p->is_dma_used = 0;
        xfer_p->chip_config = &mt_chip_spi_conf;
        xfer_p->chip_config->com_mod = FIFO_TRANSFER;
    }
    xfer_p->len = size;
    xfer_p->is_transfer_end = 1;
    xfer_p->spi_status = &spiMstatus[xfer_p->id];

#ifdef CFG_CACHE_SUPPORT
    SPI_MSG("SCPSpi dma flush\n");
    if (xfer_p->chip_config->com_mod == DMA_TRANSFER)
        /*hal_cache_flush_all_cache_lines(CACHE_DCACHE);*/
        hal_cache_flush_multiple_cache_lines(CACHE_DCACHE,
            (uint32_t)&spiResource[busId].send_temp_data, upper_len(xfer_p->len));
#endif
    ret = spi_handle(xfer_p, mode);
#ifdef CFG_CACHE_SUPPORT
    SPI_MSG("SCPSpi dma invalidate\n");
    if (xfer_p->chip_config->com_mod == DMA_TRANSFER)
        /*hal_cache_invalidate_all_cache_lines(CACHE_DCACHE);*/
        hal_cache_invalidate_multiple_cache_lines(CACHE_DCACHE,
            (uint32_t)&spiResource[busId].recv_temp_data, upper_len(xfer_p->len));
#endif

    if (rxBuf != NULL)
        memcpy((char*)rxBuf, &spiResource[busId].recv_temp_data, size);

    /* Any error happens during spi_handle, release lock here */
    /* Normally, lock will be released in isr context SCPSpiDone() */
    Spi_release_lock(xfer_p->id);
    xSemaphoreGive(spiResource[busId].spi_mutex);

    return ret;
}

void freertos_spi_init(void)
{

    uint32_t temp = 0, i = 0;
    uint32_t clk_source = SET_ULPOSC_DIV4;
    uint32_t clk_state = STATUS_ULPOSC_DIV4;

    wake_lock_init(&spiResource[0].spi_wakelock, "spi0");
    wake_lock_init(&spiResource[1].spi_wakelock, "spi1");

    spiResource[0].spi_irq_semaphore = xSemaphoreCreateBinary();
    spiResource[1].spi_irq_semaphore = xSemaphoreCreateBinary();

    spiResource[0].spi_mutex= xSemaphoreCreateMutex();
    spiResource[1].spi_mutex = xSemaphoreCreateMutex();

    request_irq(SPI0_IRQn, spi0_tz_irq_handler, "SPI0");
    request_irq(SPI1_IRQn, spi1_tz_irq_handler, "SPI1");

#ifdef CFG_VCORE_DVFS_SUPPORT
    clk_source = SET_ULPOSC_DIV4;
    clk_state = STATUS_ULPOSC_DIV4;
    spi_clk_rate = ULPOSC_DIV4_RATE;
#else
    clk_source = SET_CLK_SYS_DIV4;
    clk_state = STATUS_CLK_SYS_DIV4;
    spi_clk_rate = SYS_CLK_DIV4_RATE;
#endif

    /* uplsoc is calibrated in lk, scp is on in kernel stage */
    SPI_REG_SET32(SPI_BCLK_CK_SEL , clk_source);
    do /* Polling clock setting ready */
    {
        temp = (SPI_REG_GET32(SPI_BCLK_CK_SEL) & ~SPI_BCLK_CK_SW_STATUS_MASK);
        i++;
        if(i > 10000)
        {
            SPI_ERR("switch clk to %s Fail!\n",
                (clk_source==SET_ULPOSC_DIV4)? "ulposc":"clk_sys");
            break;
        }
    }
    while(temp != clk_state);

    SPI_MSG("init Done, source:%s,rate:%d\n",
        (clk_source==SET_ULPOSC_DIV4)? "ulposc":"clk_sys", spi_clk_rate);

    //without enable spi clk, spi interrupt bit canot be writen
    for (i = 0; i < SPI_HOST_NUM; i++) {
        Spi_enable_clk(i);
        mt_mask_and_clear_int(i);
        Spi_disable_clk(i);
    }

}
