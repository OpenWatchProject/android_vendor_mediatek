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

#ifndef __PLAT_SPI_CHRE_PLAT_H
#define __PLAT_SPI_CHRE_PLAT_H

#include <gpio.h>
#include <platform.h>

#include <stdint.h>
#include <stdbool.h>
#include <mt_reg_base.h>


#define SPI_HOST_NUM    (2)

#define SPI_BASE0       (SPI0_BASE)  //scp spi base
#define SPI_BASE1       (SPI1_BASE)

#define CLK_SCP_CG_CTRL_BASE_  (CLK_CTRL_BASE)
#define SPI_CG_CTRL     (CLK_SCP_CG_CTRL_BASE_ + 0x30)
#define SPI0_CLK  (1 << 13)
#define SPI1_CLK  (1 << 14)

#define ULPOSC_DIV4_RATE    ((250*1000000)>>2) /* ULPOSC, Clk rate different by platform*/
#define SYS_CLK_DIV4_RATE    ((26*1000000)>>2) /* When ULPOSC not ready, clk_sys = 26M*/
#define SPI_BCLK_CK_SEL     (CLK_SCP_CG_CTRL_BASE_ + 0x4C)
#define SPI_BCLK_CK_SEL_MASK       (0xFFFFFFFC)
#define SPI_BCLK_CK_SW_STATUS_MASK (0xFFFFF0FF)
#define SET_CLK_SYS_DIV4           (0x00)
#define SET_ULPOSC_DIV4            (0x02)
#define STATUS_CLK_SYS_DIV4        (0x100)
#define STATUS_ULPOSC_DIV4         (0x400)

extern struct SCPDmaStreamState SCPDMACallBack[SPI_HOST_NUM];

struct spiIrqTimer {
    uint32_t timerHandle;
    uint32_t timeOut;
};

extern struct spiIrqTimer irqTimer[SPI_HOST_NUM];

uint32_t spi_clk_rate;
extern void SCPSpiTimeoutHandler(uint32_t spi_id, uint32_t spi_base);
void spiTimerCallback(uint32_t timerId, void *cookie);

void Spi_enable_clk(uint32_t id);
void Spi_disable_clk(uint32_t id);
int Spi_get_lock(uint32_t id);
void Spi_release_lock(uint32_t id);

void spi_init(void);

struct mt_chip_conf* GetChipConfig(uint32_t id);
void SetChipConfig(uint32_t id, struct mt_chip_conf* chip_config);
struct spi_transfer* GetSpiTransfer(uint32_t id);
void SetSpiTransfer(struct spi_transfer* ptr);
int32_t GetIrqFlag(uint32_t id);
void SetIrqFlag(uint32_t id, enum spi_irq_flag flag);
void SetPauseStatus(uint32_t id, int32_t status);
int32_t GetPauseStatus(uint32_t id);
struct mt_chip_conf* SpiGetDefaultChipConfig(uint32_t id);
int config_spi_base(struct spi_transfer* spiData);
void SpiPollingHandler(struct spi_transfer *xfer);

void spi0_tz_irq_handler(void);
void spi1_tz_irq_handler(void);
void mt_init_spi_irq(uint32_t id);





#endif /* __PLAT_SPI_H */
