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
#include <dvfs.h>

struct SCPDmaStreamState SCPDMACallBack[SPI_HOST_NUM] = {};
struct  spiIrqTimer irqTimer[SPI_HOST_NUM] = {};
extern uint32_t get_mask_irq(IRQn_Type irq);

uint8_t spi_semphr[SPI_HOST_NUM];

wakelock_t spi_wakelock[SPI_HOST_NUM];

/* May call Spi_get_lock from non-interrupt and interrupt context */
int Spi_get_lock(uint32_t id)
{
    if(id < SPI_HOST_NUM)
    {
        if (is_in_isr())
        {
            if (atomicXchgByte(&spi_semphr[id], true) == true)
            {
                SPI_ERR("SPI %lu has been occupied!!!\n\r", id);
                configASSERT(0);
                return -EBUSY;
            }
            wake_lock_FromISR(&spi_wakelock[id]);
        }
        else
        {
            if (atomicXchgByte(&spi_semphr[id], true) == true)
            {
                SPI_ERR("SPI %lu has been occupied!!!\n\r", id);
                configASSERT(0);
                return -EBUSY;
            }
            wake_lock(&spi_wakelock[id]);
        }
        SPI_MSG("Spi_get_lock\n");
    }
    else
    {
        SPI_ERR("Spi_get_lock: invalid chip_config: id=%d\n\r", (int)id);
        return -1;
    }
    return 0;
}

/* May call Spi_release_lock from non-interrupt and interrupt context */
void Spi_release_lock(uint32_t id)
{
    if(id < SPI_HOST_NUM)
    {
        if (is_in_isr())
        {
            wake_unlock_FromISR(&spi_wakelock[id]);
            atomicWriteByte(&spi_semphr[id], false);
        }
        else
        {
            wake_unlock(&spi_wakelock[id]);
            atomicWriteByte(&spi_semphr[id], false);
        }
        SPI_MSG("Spi_release_lock\n");
    }
    else
    {
        SPI_ERR("Spi_release_lock: invalid chip_config: id=%d\n\r", (int)id);
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
        case 2:
            SPI_SET_BITS(SPI_CG_CTRL, SPI2_CLK);
            break;
        default:
            SPI_ERR("Spi_enable_clk: invalid chip_config: id=%d\n\r", (int)id);
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
        case 2:
            SPI_CLR_BITS(SPI_CG_CTRL, SPI2_CLK);
            break;
        default:
            SPI_ERR("Spi_disable_clk: invalid chip_config: id=%d\n\r", (int)id);
            break;
    }
    SPI_MSG("Spi_disable_clk\n");
}
/*Used for SCP SPI SLT Call*/
void spi_gpio_set(uint32_t id)
{
    switch (id) {
        case 0:
            //GPIO86:CS, GPIO87:MO, GPIO85:MI, this will be configed by dws
            SPI_REG_SET32(SCP_SPI0_GPIO_BASE ,
             (SPI_REG_GET32(SCP_SPI0_GPIO_BASE) & 0xFFFFF) | 0x22200000);
            //GPIO88:CK
            SPI_REG_SET32(SCP_SPI0_CLK_BASE ,
             (SPI_REG_GET32(SCP_SPI0_CLK_BASE) & 0xFFFFFFF0) | 0x2);
            break;
        case 1:
            //GPIO162:CS //GPIO163:MO, GPIO161:MI, GPIO164:CK, this will be configed by dws
            SPI_REG_SET32(SCP_SPI1_GPIO_BASE ,
             (SPI_REG_GET32(SCP_SPI1_GPIO_BASE) & 0xFFF0000F) | 0x00022220);
            break;
        case 2:
            //GPIO0:CS, GPIO2:CK, //GPIO1:MO
            SPI_REG_SET32(SCP_SPI2_GPIO_BASE ,
             (SPI_REG_GET32(SCP_SPI2_GPIO_BASE) & 0xFFFFF000) | 0x222);
            //GPIO94:MI
            SPI_REG_SET32(SCP_SPI2_MI_GPIO_BASE ,
             (SPI_REG_GET32(SCP_SPI2_MI_GPIO_BASE) & 0x0FFFFFFF) | 0x20000000);
            break;
        default:
            SPI_ERR("spi_gpio_set: invalid gpio config: id=%d\n\r", (int)id);
            break;
    }
}

/* check GPIO settings for SPI0~2 */
static void spi_gpio_check(uint32_t id)
{
    uint32_t gpio_setting = 0, gpio_setting2 = 0;

    if (id == 0) {
        gpio_setting = SPI_REG_GET32(SCP_SPI0_GPIO_BASE);
        if ((gpio_setting & 0x77700000) != 0x22200000)
                goto WRONG;

        gpio_setting2 = SPI_REG_GET32(SCP_SPI0_CLK_BASE);
        if ((gpio_setting2 & 0x7) != 0x2)
                goto WRONG;
    } else if (id == 1) {
        gpio_setting = SPI_REG_GET32(SCP_SPI1_GPIO_BASE);
        if ((gpio_setting & 0x77770) != 0x22220)
                goto WRONG;
    } else if (id == 2) {
        gpio_setting = SPI_REG_GET32(SCP_SPI2_GPIO_BASE);
        if ((gpio_setting & 0x777) != 0x222)
                goto WRONG;

        gpio_setting2 = SPI_REG_GET32(SCP_SPI2_MI_GPIO_BASE);
        if ((gpio_setting2 & 0x7000000) != 0x2000000)
                goto WRONG;
    } else {
        SPI_ERR("%s: invalid gpio config: id=%lu\n\r", __func__, id);
        return;
    }
    return;

WRONG:
    SPI_ERR("%s: wrong gpio setting %08lX %08lX: id=%lu\n\r",
            __func__, gpio_setting,
            gpio_setting2, id);
    return;
}

static struct mt_chip_conf chip_config_default0;
static struct mt_chip_conf chip_config_default1;
static struct mt_chip_conf chip_config_default2;
struct mt_chip_conf* chip_config0 = &chip_config_default0;
struct mt_chip_conf* chip_config1 = &chip_config_default1;
struct mt_chip_conf* chip_config2 = &chip_config_default2;


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
        case 2:
            chip_config = chip_config2;
            break;
        default:
            SPI_ERR("GetChipConfig: invalid chip_config: id=%d\n\r", (int)id);
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
        case 2:
            chip_config2 = chip_config;
            break;
        default:
            SPI_ERR("SetChipConfig: invalid chip_config: id=%d\n\r", (int)id);
            break;
    }
}

static struct spi_transfer* xfer0 = NULL;
static struct spi_transfer* xfer1 = NULL;
static struct spi_transfer* xfer2 = NULL;

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
        case 2:
            xfer = xfer2;
            break;
        default:
            SPI_ERR("invalid xfer: spi->id=%d\n\r", (int)id);
            break;
    }
    return xfer;
}
void SetSpiTransfer(struct spi_transfer* ptr)
{
    if (NULL == ptr) {
        SPI_ERR("spi_transfer is NULL !!!\n");
    } else {
        switch (ptr->id) {
            case 0:
                xfer0 = ptr;
                break;
            case 1:
                xfer1 = ptr;
                break;
            case 2:
                xfer2 = ptr;
                break;
            default:
                SPI_ERR("invalid ptr: ptr->id=%d\n\r", (int)ptr->id);
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
static int32_t irq_flag2 = IRQ_IDLE;

int32_t GetIrqFlag(uint32_t id)
{
    switch (id) {
        case 0:
            irq_flag = irq_flag0;
            break;
        case 1:
            irq_flag = irq_flag1;
            break;
        case 2:
            irq_flag = irq_flag2;
            break;
        default:
            SPI_ERR("GetIrqFlag: invalid irq_flag : id=%d\n\r", (int)id);
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
        case 2:
            irq_flag2 = flag;
            break;
        default:
            SPI_ERR("SetIrqFlag: invalid irq_flag : id=%d\n\r", (int)id);
            break;
    }
}
static int32_t running = 0;
static int32_t running0 = 0;
static int32_t running1 = 0;
static int32_t running2 = 0;

void SetPauseStatus(uint32_t id, int32_t status)
{
    switch (id) {
        case 0:
            running0 = status;
            break;
        case 1:
            running1 = status;
            break;
        case 2:
            running2 = status;
            break;
        default:
            SPI_ERR("SetPauseStatus: invalid running : id=%d\n\r", (int)id);
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
        case 2:
            running = running2;
            break;
        default:
            SPI_ERR("GetPauseStatus: invalid running : id=%d\n\r", (int)id);
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
    } else {
        ChipConfig = &chip_config_default2;
    }

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
        case 2:
            spiData->base = SPI_BASE2;
            break;
        default:
            SPI_ERR("invalid para: spi->id=%d\n\r", (int)spiData->id);
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
        //      SPI_MSG("Polling IRQ: irqStatus[0x%08x],xfer->id =%d\n", irqStatus, xfer->id);
    } while (irqStatus == 0);

    SPI_MSG("Polling : Status1 irqStatus[0x%08x],xfer->id =%d\n", (unsigned int)irqStatus, (int)xfer->id);

    switch (xfer->id) {
        case 0:
        case 1:
        case 2:
            SCPSpiPollingHandle(xfer->id, base);
            break;
        default:
            SPI_ERR("invalid para: xfer->id=%d\n\r", (int)xfer->id);
            break;
    }

}

void spiTimerCallback(uint32_t timerId, void *cookie) {
    struct spi_transfer *xfer = (struct spi_transfer *)cookie;
    uint32_t maskIrq = 1;
    IRQn_Type irqNum = 1;

    /* we must clear timerHandle when spi irq timeout happens*/
    irqTimer[xfer->id].timerHandle = 0;

    switch(xfer->id)
    {
        case 0:
        irqNum = SPI0_IRQn; break;
        case 1:
        irqNum = SPI1_IRQn; break;
        case 2:
        irqNum = SPI2_IRQn; break;
        default:
        SPI_ERR("spi%lu: illegal ID  in spiTimerCallback\n", xfer->id);
    return;
    }

    maskIrq = get_mask_irq(irqNum);
    SPI_ERR("spi%ld: irq:%d spiTimerCallback,maskIrqstate:%lu pending:%s,active:%s\n",
            xfer->id, irqNum, maskIrq, NVIC_GetPendingIRQ(irqNum)?"enable":"no",
            NVIC_GetActive(irqNum)?"enable":"no");

    SCPSpiTimeoutHandler(xfer->id, xfer->base);
    SPI_CHRE_READ(xfer->base, SPI_REG_STATUS1);
    SCPDMACallBack[xfer->id].callback(SCPDMACallBack[xfer->id].cookie, false);

    SPI_CHRE_READ(xfer->base, SPI_REG_STATUS1);
    if(NVIC_GetPendingIRQ(irqNum))
        NVIC_ClearPendingIRQ(irqNum);
    SPI_ERR("spiTimerCallback spi:%lu clear irq and pending irq, reset spi,disable clk!!\n",
        xfer->id);
}

//This function is used for interrupt mode
void spi0_tz_irq_handler(void)
{
#ifdef SPI_PRINT_DEBUG
    uint32_t irqStatus;
    irqStatus = SPI_CHRE_READ(SPI_BASE0, SPI_REG_STATUS0) & 0x00000003; //Read clear IRQ status
    SPI_MSG_ISR("IRQ SPI0: irqStatus[0x%08x]\n", (unsigned int)irqStatus);
#else
    SPI_CHRE_READ(SPI_BASE0, SPI_REG_STATUS0); //Read clear IRQ status
#endif

    if(irqTimer[0].timerHandle != 0)
    {
        if(timTimerCancel(irqTimer[0].timerHandle))
            irqTimer[0].timerHandle = 0;
        else
            SPI_ERR_ISR("timTimerCancel failed in SPI0 IRQ \n");
    }
    else
    {
        SPI_ERR_ISR("Assert:irqTimer[0].timerHandle=0 in SPI0 IRQ \n");
        configASSERT(0);
    }

    SCPSpiPollingHandle(0, SPI_BASE0);

    SCPDMACallBack[0].callback(SCPDMACallBack[0].cookie, false); //call back to SCPSpiDone
}
//This function is used for interrupt mode
void spi1_tz_irq_handler(void)
{

#ifdef SPI_PRINT_DEBUG
    uint32_t irqStatus;
    irqStatus = SPI_CHRE_READ(SPI_BASE1, SPI_REG_STATUS0) & 0x00000003; //Read clear IRQ status
    SPI_MSG_ISR("IRQ SPI1: irqStatus[0x%08x]\n", (unsigned int)irqStatus);
#else
    SPI_CHRE_READ(SPI_BASE1, SPI_REG_STATUS0); //Read clear IRQ status
#endif

    if(irqTimer[1].timerHandle != 0)
    {
        if(timTimerCancel(irqTimer[1].timerHandle))
            irqTimer[1].timerHandle = 0;
        else
            SPI_ERR_ISR("timTimerCancel failed in SPI1 IRQ \n");
    }
    else
    {
        SPI_ERR_ISR("Assert:irqTimer[1].timerHandle=0 in SPI1 IRQ \n");
        configASSERT(0);
    }

    SCPSpiPollingHandle(1, SPI_BASE1);

    SCPDMACallBack[1].callback(SCPDMACallBack[1].cookie, false); //call back to SCPSpiDone


}
//This function is used for interrupt mode
void spi2_tz_irq_handler(void)
{

#ifdef SPI_PRINT_DEBUG
    uint32_t irqStatus;
    irqStatus = SPI_CHRE_READ(SPI_BASE2, SPI_REG_STATUS0) & 0x00000003; //Read clear IRQ status
    SPI_MSG_ISR("IRQ SPI2: irqStatus[0x%08x]\n", (unsigned int)irqStatus);
#else
    SPI_CHRE_READ(SPI_BASE2, SPI_REG_STATUS0); //Read clear IRQ status
#endif

    if(irqTimer[2].timerHandle != 0)
    {
        if(timTimerCancel(irqTimer[2].timerHandle))
            irqTimer[2].timerHandle = 0;
        else
            SPI_ERR_ISR("timTimerCancel failed in SPI2 IRQ \n");
    }
    else
    {
        SPI_ERR_ISR("Assert:irqTimer[2].timerHandle=0 in SPI2 IRQ \n");
        configASSERT(0);
    }

    SCPSpiPollingHandle(2, SPI_BASE2);

    SCPDMACallBack[2].callback(SCPDMACallBack[2].cookie, false); //call back to SCPSpiDone

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
        case 2:
            irqTimer[2].timerHandle = 0;
            irqTimer[2].timeOut = 2000000000;
            request_irq(SPI2_IRQn, spi2_tz_irq_handler, "SPI2");
            break;
        default:
            SPI_ERR("mt_init_spi_irq: invalid IRQ setting : id=%d\n\r", (int)id);
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
        case 2:
            base = SPI_BASE2;
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

void spi_init(void)
{

    uint32_t temp=0, i=0;
    uint32_t clk_source = SET_ULPOSC_DIV4;
    uint32_t clk_state = STATUS_ULPOSC_DIV4;

    enable_clk_bus(SPI_MEM_ID);
    wake_lock_init(&spi_wakelock[0], "spi0");
    wake_lock_init(&spi_wakelock[1], "spi1");
    wake_lock_init(&spi_wakelock[2], "spi2");
//  spi_gpio_set(0);
//  spi_gpio_set(1);
//  spi_gpio_set(2);
    spi_gpio_check(0);
    spi_gpio_check(1);
    spi_gpio_check(2);
    disable_clk_bus(SPI_MEM_ID);

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
            SPI_ERR("SPI switch clk to %s timeout FAIL!\n\r",
                (clk_source==SET_ULPOSC_DIV4)? "ulposc":"clk_sys");
            break;
        }
    }
    while(temp != clk_state);

    SPI_ERR("spi init Done, clk source:%s, source_rate:%lu",
        (clk_source==SET_ULPOSC_DIV4)? "ulposc":"clk_sys", spi_clk_rate);

    for (i = 0; i < SPI_HOST_NUM; i++)
        mt_mask_and_clear_int(i);
}


