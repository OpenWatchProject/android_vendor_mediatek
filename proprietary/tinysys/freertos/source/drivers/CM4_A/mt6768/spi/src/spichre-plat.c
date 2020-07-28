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
#include <util.h>

#include <timer.h>
#include <plat/inc/rtc.h>

#include <platform.h>
#include <plat/inc/spichre.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <spichre-plat.h>
#include <wakelock.h>
#include <semphr.h>
#include <atomic.h>
#ifdef CFG_VCORE_DVFS_SUPPORT
#include <dvfs.h>
#endif

struct SCPDmaStreamState SCPDMACallBack[SPI_HOST_NUM] = {};
struct  spiIrqTimer irqTimer[SPI_HOST_NUM] = {};
extern uint32_t get_mask_irq(IRQn_Type irq);

uint8_t spi_semphr[SPI_HOST_NUM];

wakelock_t spi_wakelock[SPI_HOST_NUM];

/* May call Spi_get_lock from non-interrupt and interrupt context */
int Spi_get_lock(uint32_t id)
{
    if (id < SPI_HOST_NUM) {
        if (is_in_isr()) {
            if (atomicXchgByte(&spi_semphr[id], true) == true) {
                SPI_MSG("SPI %d has been occupied!\n", id);
                configASSERT(0);
                return -EBUSY;
            }
            wake_lock_FromISR(&spi_wakelock[id]);
        } else {
            if (atomicXchgByte(&spi_semphr[id], true) == true) {
                SPI_MSG("SPI %d has been occupied!\n", id);
                configASSERT(0);
                return -EBUSY;
            }
            wake_lock(&spi_wakelock[id]);
        }
        SPI_MSG("Spi_get_lock\n");
    } else {
        SPI_MSG("get lock invalid id=%d\n", (int)id);
        return -1;
    }
    return 0;
}

/* May call Spi_release_lock from non-interrupt and interrupt context */
void Spi_release_lock(uint32_t id)
{
    if (id < SPI_HOST_NUM) {
        if (is_in_isr()) {
            wake_unlock_FromISR(&spi_wakelock[id]);
            atomicWriteByte(&spi_semphr[id], false);
        } else {
            wake_unlock(&spi_wakelock[id]);
            atomicWriteByte(&spi_semphr[id], false);
        }
        SPI_MSG("Spi_release_lock\n");
    } else {
        SPI_MSG("release lock invalid id=%d\n", (int)id);
    }
}

void Spi_enable_clk(uint32_t id)
{
    switch (id) {
        case 0:
            SPI_SET_BITS(SPI_CG_CTRL, SPI0_CLK);
            break;
        case 1:
            SPI_SET_BITS(SPI_CG_CTRL, SPI1_CLK);
            break;
        default:
            SPI_MSG("enable clk invalid id=%d\n", (int)id);
            break;
    }
    SPI_MSG("Spi_enable_clk\n");
}

void Spi_disable_clk(uint32_t id)
{
    switch (id) {
        case 0:
            SPI_CLR_BITS(SPI_CG_CTRL, SPI0_CLK);
            break;
        case 1:
            SPI_CLR_BITS(SPI_CG_CTRL, SPI1_CLK);
            break;
        default:
            SPI_MSG("disable clk invalid id=%d\n", (int)id);
            break;
    }
    SPI_MSG("Spi_disable_clk\n");
}




static struct mt_chip_conf chip_config_default0;
static struct mt_chip_conf chip_config_default1;
struct mt_chip_conf* chip_config0 = &chip_config_default0;
struct mt_chip_conf* chip_config1 = &chip_config_default1;

struct mt_chip_conf* GetChipConfig(uint32_t id)
{
    struct mt_chip_conf* chip_config = NULL;

    switch (id) {
        case 0:
            chip_config = chip_config0;
            break;
        case 1:
            chip_config = chip_config1;
            break;
        default:
            SPI_MSG("GetChipConfig invalid id=%d\n", (int)id);
            break;
    }

    return chip_config;

}

void SetChipConfig(uint32_t id, struct mt_chip_conf* chip_config)
{
    switch (id) {
        case 0:
            chip_config0 = chip_config;
            break;
        case 1:
            chip_config1 = chip_config;
            break;
        default:
            SPI_MSG("SetChipConfig invalid id=%d\n", (int)id);
            break;
    }
}

static struct spi_transfer* xfer0 = NULL;
static struct spi_transfer* xfer1 = NULL;

struct spi_transfer* GetSpiTransfer(uint32_t id)
{
    struct spi_transfer* xfer = NULL;

    switch (id) {
        case 0:
            xfer = xfer0;
            break;
        case 1:
            xfer = xfer1;
            break;
        default:
            SPI_MSG("Get Transfer invalid id:%d\n", (int)id);
            break;
    }

    return xfer;
}
void SetSpiTransfer(struct spi_transfer* ptr)
{
    if (NULL == ptr) {
        SPI_MSG("SetSpiTransfer NULL para\n");
    } else {
        switch (ptr->id) {
            case 0:
                xfer0 = ptr;
                break;
            case 1:
                xfer1 = ptr;
                break;
            default:
                SPI_MSG("Set Transfer invalid id:%d\n", (int)ptr->id);
                break;
        }
    }
}

/*
* 1: wait interrupt to handle
* 2: finish interrupt handle turn to idle state
*/
static int32_t irq_flag = IRQ_IDLE;
static int32_t irq_flag0 = IRQ_IDLE;
static int32_t irq_flag1 = IRQ_IDLE;

int32_t GetIrqFlag(uint32_t id)
{
    switch (id) {
        case 0:
            irq_flag = irq_flag0;
            break;
        case 1:
            irq_flag = irq_flag1;
            break;
        default:
            SPI_MSG("GetIrqFlag invalid id=%d\n", (int)id);
            break;
    }

    return irq_flag;
}

void SetIrqFlag(uint32_t id, enum spi_irq_flag flag)
{
    switch (id) {
        case 0:
            irq_flag0 = flag;
            break;
        case 1:
            irq_flag1 = flag;
            break;
        default:
            SPI_MSG("SetIrqFlag invalid id=%d\n", (int)id);
            break;
    }
}

static int32_t running = 0;
static int32_t running0 = 0;
static int32_t running1 = 0;

void SetPauseStatus(uint32_t id, int32_t status)
{
    switch (id) {
        case 0:
            running0 = status;
            break;
        case 1:
            running1 = status;
            break;
        default:
            SPI_MSG("SetPauseStatus invalid id:%d\n", (int)id);
            break;
    }
}
int32_t GetPauseStatus(uint32_t id)
{
    switch (id) {
        case 0:
            running = running0;
            break;
        case 1:
            running = running1;
            break;
        default:
            SPI_MSG("GetPauseStatus invalid id:%d\n", id);
            break;
    }
    return running;
}

struct mt_chip_conf* SpiGetDefaultChipConfig(uint32_t id)
{
    struct mt_chip_conf *ChipConfig = NULL;

    if (id == 0) {
        ChipConfig = &chip_config_default0;
    } else if (id == 1) {
        ChipConfig = &chip_config_default1;
    } else
        SPI_MSG("Get chip config invalid id:%d\n", id);

    return ChipConfig;
}

int config_spi_base(struct spi_transfer* spiData)
{
    int ret = 0;

    switch (spiData->id) {
        case 0:
            spiData->base = SPI_BASE0;
            break;
        case 1:
            spiData->base = SPI_BASE1;
            break;
        default:
            SPI_ERR("config base invalid id:%d\n", (int)spiData->id);
            ret = -1;
            break;
    }
    return ret;
}

//This function is used for polling mode
void SpiPollingHandler(struct spi_transfer *xfer)
{
    volatile uint32_t irqStatus = 0;
    uint32_t base = xfer->base;

    do {
        irqStatus = SPI_CHRE_READ(base, SPI_REG_STATUS1) & 0x00000001;
        //SPI_MSG("Polling IRQ: irqStatus[0x%08x],xfer->id =%d\n", irqStatus, xfer->id);
    } while (irqStatus == 0);

    //SPI_MSG("Polling : Status1 irqStatus[0x%08x],xfer->id =%d\n", (unsigned int)irqStatus, (int)xfer->id);

    switch (xfer->id) {
        case 0:
        case 1:
            SCPSpiPollingHandle(xfer->id, base);
            break;
        default:
            SPI_MSG("SpiPollingHandler invalid ID:%d\n", (int)xfer->id);
            break;
    }

}

void spiTimerCallback(uint32_t timerId, void *cookie)
{
    struct spi_transfer *xfer = (struct spi_transfer *)cookie;
    uint32_t maskIrq = 1;
    IRQn_Type irqNum = 1;

    /* we must clear timerHandle when spi irq timeout happens*/
    irqTimer[xfer->id].timerHandle = 0;

    switch (xfer->id) {
        case 0:
            irqNum = SPI0_IRQn;
            break;
        case 1:
            irqNum = SPI1_IRQn;
            break;
        default:
            SPI_MSG("spiTimerCallback invalid ID:%d\n", xfer->id);
            return;
    }

    maskIrq = get_mask_irq(irqNum);
    SPI_ERR("spi%ld: irq:%d maskIrqstate:%lu pending:%s,active:%s\n",
            xfer->id, irqNum, maskIrq, NVIC_GetPendingIRQ(irqNum) ? "Y" : "N",
            NVIC_GetActive(irqNum) ? "Y" : "N");

    SCPSpiTimeoutHandler(xfer->id, xfer->base);
    SPI_CHRE_READ(xfer->base, SPI_REG_STATUS1);
    SCPDMACallBack[xfer->id].callback(SCPDMACallBack[xfer->id].cookie, -EINVAL);

    SPI_CHRE_READ(xfer->base, SPI_REG_STATUS1);
    if (NVIC_GetPendingIRQ(irqNum))
        NVIC_ClearPendingIRQ(irqNum);
    SPI_MSG("spiTimerCallback spi:%d clear irq and pending irq, reset spi,disable clk!!\n",
            xfer->id);
}

//This function is used for interrupt mode
void spi0_tz_irq_handler(void)
{
    uint32_t irqStatus;
    irqStatus = SPI_CHRE_READ(SPI_BASE0, SPI_REG_STATUS0) & 0x00000003; //Read clear IRQ status

    //GIC dispatch spi irq unexpected, while spi HW never issued interrupt.
    if (!irqStatus) {
        SPI_ERR_ISR("No irq captured in spi0 ISR\n");
        return;
    }
#ifdef SPI_PRINT_DEBUG
    SPI_MSG_ISR("IRQ SPI0: irqStatus[0x%08x]\n", (unsigned int)irqStatus);
#endif

    if (irqTimer[0].timerHandle != 0) {
        if (timTimerCancel(irqTimer[0].timerHandle))
            irqTimer[0].timerHandle = 0;
        else
            SPI_ERR_ISR("TimerCancel failed\n");
    } else {
        SPI_ERR_ISR("Assert:timerHandle=0\n");
        configASSERT(0);
    }

    SCPSpiPollingHandle(0, SPI_BASE0);

    SCPDMACallBack[0].callback(SCPDMACallBack[0].cookie, false); //call back to SCPSpiDone
}

//This function is used for interrupt mode
void spi1_tz_irq_handler(void)
{
    uint32_t irqStatus;
    irqStatus = SPI_CHRE_READ(SPI_BASE1, SPI_REG_STATUS0) & 0x00000003; //Read clear IRQ status

    //GIC dispatch spi irq unexpected, while spi HW never issued interrupt.
    if (!irqStatus) {
        SPI_ERR_ISR("No irq captured in spi1 ISR\n");
        return;
    }
#ifdef SPI_PRINT_DEBUG
    SPI_MSG_ISR("IRQ SPI1: irqStatus[0x%08x]\n", (unsigned int)irqStatus);
#endif

    if (irqTimer[1].timerHandle != 0) {
        if (timTimerCancel(irqTimer[1].timerHandle))
            irqTimer[1].timerHandle = 0;
        else
            SPI_ERR_ISR("TimerCancel failed\n");
    } else {
        SPI_ERR_ISR("Assert:timerHandle=0\n");
        configASSERT(0);
    }

    SCPSpiPollingHandle(1, SPI_BASE1);

    SCPDMACallBack[1].callback(SCPDMACallBack[1].cookie, false); //call back to SCPSpiDone
}

void mt_init_spi_irq(uint32_t id)
{
    switch (id) {
        case 0:
            irqTimer[0].timerHandle = 0;
            irqTimer[0].timeOut = 2000000000;
            request_irq(SPI0_IRQn, spi0_tz_irq_handler, "SPI0");
            break;
        case 1:
            irqTimer[1].timerHandle = 0;
            irqTimer[1].timeOut = 2000000000;
            request_irq(SPI1_IRQn, spi1_tz_irq_handler, "SPI1");
            break;
        default:
            SPI_MSG("init irq invalid id:%d\n", (int)id);
            break;
    }
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
        reg_val &= ~(SPI_CMD_FINISH_IE_MASK | SPI_CMD_PAUSE_IE_OFFSET);
        SPI_CHRE_WRITE(base, SPI_REG_CMD, reg_val);

        /* clear all interrupt */
        SPI_CHRE_READ(base, SPI_REG_STATUS0);
    }
}

void spi_init(void)
{
    uint32_t temp = 0, i = 0;
    uint32_t clk_source = SET_ULPOSC_DIV4;
    uint32_t clk_state = STATUS_ULPOSC_DIV4;

    wake_lock_init(&spi_wakelock[0], "spi0");
    wake_lock_init(&spi_wakelock[1], "spi1");

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
    do { /* Polling clock setting ready */
        temp = (SPI_REG_GET32(SPI_BCLK_CK_SEL) & ~SPI_BCLK_CK_SW_STATUS_MASK);
        i++;
        if (i > 10000) {
            SPI_ERR("switch clk to %s Fail!\n",
                    (clk_source == SET_ULPOSC_DIV4) ? "ulposc" : "clk_sys");
            break;
        }
    } while (temp != clk_state);

    SPI_MSG("init Done, source:%s,rate:%d\n",
            (clk_source == SET_ULPOSC_DIV4) ? "ulposc" : "clk_sys", spi_clk_rate);

    if (NVIC_GetPendingIRQ(SPI0_IRQn))
        NVIC_ClearPendingIRQ(SPI0_IRQn);
    if (NVIC_GetPendingIRQ(SPI1_IRQn))
        NVIC_ClearPendingIRQ(SPI1_IRQn);

    //without enable spi clk, spi interrupt bit canot be writen
    for (i = 0; i < SPI_HOST_NUM; i++) {
        Spi_enable_clk(i);
        mt_mask_and_clear_int(i);
        Spi_disable_clk(i);
    }
}


