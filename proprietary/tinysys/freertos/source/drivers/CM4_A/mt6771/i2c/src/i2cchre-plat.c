#include <stdlib.h>
#include <string.h>
#include <driver_api.h>
#include <interrupt.h>
#ifdef CFG_VCORE_DVFS_SUPPORT
#include <dvfs.h>
#endif
/* CHRE */
#include "i2c.h"
#include <i2cchre-plat.h>
#include <atomicBitset.h>
#include <atomic.h>
#include <dma.h>
#include <util.h>
#include <timer.h>
#include <semphr.h>
#include <scp_sem.h>
#include <mt_gpt.h>
#include <plat/inc/rtc.h>
#include <wakelock.h>

#define I2C_MAX_QUEUE_DEPTH     12
#define I2C_MAX_TXBUF_SIZE      8

enum MtI2cSpiMasterState {
    MT_I2C_MASTER_IDLE,
    MT_I2C_MASTER_START,
    MT_I2C_MASTER_START_SYNC,
};

struct I2cMtState {
    struct {
        union {
            uint8_t *buf;
            const uint8_t *cbuf;
            uint8_t byte;
        };
        size_t size;
        size_t offset;
        bool preamble;

        I2cCallbackF callback;
        void *cookie;
    } rx, tx;

    enum {
        MT_I2C_DISABLED,
        MT_I2C_MASTER,
    } mode;
    /* support i2c transfer timeout control for different i2c controller */
    uint32_t timerHandle;
    uint32_t timeOut;
    // MtI2cSpiMasterState
    uint8_t masterState;
    int err;
    uint32_t retry;
};

struct MtI2cDev {
    // const struct MtI2cCfg *cfg;
    // const struct MtI2cBoardCfg *board;
    // mt_i2c *i2c;
    uint32_t id;
    uint32_t base;
    uint32_t dma_base;

    struct I2cMtState state;

    enum mt_trans_op op;

    uint32_t next;
    uint32_t last;
    uint32_t clk_src_in_khz;
    // struct Gpio *scl;
    // struct Gpio *sda;
    uint8_t addr;
    uint8_t ver;
    wakelock_t wakelock;
    bool dma_mode;
    bool set_ltiming;
    uint32_t speedInHz;
};


// static struct MtI2cDev mMtI2cDevs[ARRAY_SIZE(mMtI2cCfgs)];
static struct MtI2cDev mMtI2cDevs[I2C_NR] = {
    {
        .id = 0,
        .base = I2C0_BASE,
        .dma_base = DMA_BASE+0x100,
        .ver = 2,
        .set_ltiming = 1,
        .clk_src_in_khz = I3C_SRC_CLK_RATE / DEFAULT_DIV,
        .speedInHz = MAX_ST_MODE_SPEED * 1000,
    },
    {
        .id = 1,
        .base = I2C1_BASE,
        .dma_base = DMA_BASE+0x200,
        .ver = 0,
        .set_ltiming = 0,
        .clk_src_in_khz = I2C_SRC_CLK_RATE / DEFAULT_DIV,
        .speedInHz = MAX_ST_MODE_SPEED * 1000,
    },
};


struct MtI2cXfer {
    uint32_t        id;
    uint8_t         txBuf[I2C_MAX_TXBUF_SIZE];
    size_t          txSize;
    uint8_t         *rxBuf;
    size_t          rxSize;
    I2cCallbackF    callback;
    void           *cookie;
    uint8_t         busId; /* for us these are both fine in a uint 8 */
    uint8_t         addr;
    enum mt_trans_op op;
    uint32_t        retry;
};

ATOMIC_BITSET_DECL(mXfersValid, I2C_MAX_QUEUE_DEPTH, static);

static void mtI2cMasterTxRxDone(struct MtI2cDev *pdev, int err);
static int mtI2cSpeedSet(struct MtI2cDev *i2c);

static struct MtI2cXfer mXfers[I2C_MAX_QUEUE_DEPTH] = { };

#ifdef CFG_TESTSUITE_SUPPORT
uint8_t tx_data[128], rx_data[128];
uint32_t pattern;
#endif

void i2c_dump_info(struct MtI2cDev *i2c);
//void i2c_dump_dma_info(struct MtI2cDev *i2c);

static struct MtI2cXfer *mtI2cGetXfer(void) {
    int32_t idx = atomicBitsetFindClearAndSet(mXfersValid);

    if (idx < 0)
        return NULL;
    else
        return mXfers + idx;
}

static void mtI2cPutXfer(struct MtI2cXfer *xfer) {
    if (xfer)
        atomicBitsetClearBit(mXfersValid, xfer - mXfers);
}

/* need to improve */
static int i2c_clock_enable(struct MtI2cDev *i2c) {
    if (i2c->ver == 2)
        drv_set_reg32(SCP_CLK_CG_CTRL, (I2C_MCLK_CG_BIT |
                      I2C_BCLK_CG_BIT | I3C0_CG_BIT));
    else
        drv_set_reg32(SCP_CLK_CG_CTRL, (I2C_MCLK_CG_BIT | I2C_BCLK_CG_BIT));

    /* no need enable DMA clock, platform will take care */
    /*drv_set_reg32(SCP_CLK_CG_CTRL, (DMA_CH0_CG_BIT|DMA_CH1_CG_BIT));*/
    return 0;
}

/* need to improve */
static int i2c_clock_disable(struct MtI2cDev *i2c) {
    if (i2c->ver == 2)
        drv_clr_reg32(SCP_CLK_CG_CTRL, (I2C_MCLK_CG_BIT |
                      I2C_BCLK_CG_BIT | I3C0_CG_BIT));
    else
        drv_clr_reg32(SCP_CLK_CG_CTRL, (I2C_MCLK_CG_BIT | I2C_BCLK_CG_BIT));
    /* no need disable DMA clock, platform will take care */
    /*drv_clr_reg32(SCP_CLK_CG_CTRL, (DMA_CH0_CG_BIT|DMA_CH1_CG_BIT));*/
    return 0;
}

#define mt_reg_sync_writel(v, a)  *(volatile unsigned int *)(a) = (v)

static void _i2c_writew(uint16_t value, struct MtI2cDev *i2c, uint16_t offset) {
    mt_reg_sync_writel(value, (i2c->base) + (offset));
}

static uint16_t _i2c_readw(struct MtI2cDev *i2c, uint8_t offset) {
    return DRV_Reg32((i2c->base) + (offset));
}

#define i2c_writew(val, i2c, ofs) \
        do { \
            if (((i2c)->ver != 0x2) || (V2_##ofs != 0xfff)) \
                _i2c_writew(val, i2c, ((i2c)->ver == 0x2) ? (V2_##ofs) : ofs); \
        } while (0)

#define i2c_readw(i2c, ofs) \
        ({ \
            uint16_t value = 0; \
                if (((i2c)->ver != 0x2) || (V2_##ofs != 0xfff)) \
                    value = _i2c_readw(i2c, ((i2c)->ver == 0x2) ? (V2_##ofs) : ofs); \
                value; \
        })

static void i2c_writew_dma(uint32_t value, struct MtI2cDev *i2c, uint8_t offset) {
    mt_reg_sync_writel(value, (i2c->dma_base) + (offset));
}
#if 0
static inline uint32_t i2c_readw_dma(struct MtI2cDev *i2c, uint8_t offset) {
    return DRV_Reg32((i2c->dma_base) + (offset));
}
#endif
/* The sharing I2C bus of AP and SCP is no longer be used, so     */
/* the HW semaphore should not be used anymore. This removal also */
/* can prevent unexpected result of SCP's reset stress test       */
#if 0
#define I2C_SEM_TIMEOUT 100
static int i2c_get_scp_semaphore(struct MtI2cDev *i2c)
{
    int ret = 0;
    int count = I2C_SEM_TIMEOUT;

    /* Semaphore handling */
    do {
        switch (i2c->id) {
            case I2C0:
                ret = get_scp_semaphore(SEMAPHORE_I2C0);
                break;
            case I2C1:
                ret = get_scp_semaphore(SEMAPHORE_I2C1);
                break;
            default:
                I2CLOG("Invalid I2C number!\n");
                return ret;
        }
        if (ret == 1) {
            /* get semaphore successfully */
            break;
        }
    } while (count--);

    if (count <= 0) {
        I2CERR("i2c%ld get sem tmo\n", i2c->id);
        return -ETIMEDOUT_I2C;
    }

    return 0;
}

static int i2c_release_scp_semaphore(struct MtI2cDev *i2c)
{
    int ret = 0;
    int count = I2C_SEM_TIMEOUT;

    /* Semaphore handling */
    do {
        switch (i2c->id) {
            case I2C0:
                ret = release_scp_semaphore(SEMAPHORE_I2C0);
                break;
            case I2C1:
                ret = release_scp_semaphore(SEMAPHORE_I2C1);
                break;
            default:
                I2CLOG("Invalid I2C number!\n");
                return ret;
        }
        if (ret == 1) {
            /* release semaphore successfully */
            break;
        }
    } while (count--);

    if (count <= 0) {
        I2CERR("i2c%ld rel sem tmo\n", i2c->id);
        return -ETIMEDOUT_I2C;
    }

    return 0;
}
#endif

/* May call i2c_get_lock from non-interrupt and interrupt context */
void i2c_get_lock(struct MtI2cDev *i2c) {
    if (is_in_isr()) {
        wake_lock_FromISR(&i2c->wakelock);
    } else {
        wake_lock(&i2c->wakelock);
    }
    /* i2c_get_scp_semaphore(i2c); */
    I2CDBG("i2c_get_lock\n");
}

/* May call i2c_release_lock from non-interrupt and interrupt context */
void i2c_release_lock(struct MtI2cDev *i2c) {
    /* i2c_release_scp_semaphore(i2c); */
    if (is_in_isr()) {
        wake_unlock_FromISR(&i2c->wakelock);
    } else {
        wake_unlock(&i2c->wakelock);
    }
    I2CDBG("i2c_release_lock\n");
}

static void mt_i2c_init_hw(struct MtI2cDev *i2c) {
    i2c_writew(0x1ff, i2c, OFFSET_INTR_STAT);
    if (i2c->ver == 2)
        i2c_writew(I2C_SOFT_RST | I2C_ERR_RST, i2c, OFFSET_SOFTRESET);
    else
        i2c_writew(I2C_SOFT_RST, i2c, OFFSET_SOFTRESET);
    i2c_writew(I2C_IO_CONFIG_OPEN_DRAIN, i2c, OFFSET_IO_CONFIG);
}

static void I2cTimerCallback(uint32_t timerId, void *cookie) {
    struct MtI2cDev *i2c = (struct MtI2cDev *)cookie;

    /* I2CERR("i2c%ld: I2cTimerCallback\n", i2c->id);*/
    /* we must clear timerHandle when i2c occur transfer timeout */
    i2c->state.timerHandle = 0;
    mtI2cMasterTxRxDone(i2c, -ETIMEDOUT_I2C);
}

static void mt_i2c_req_gdma(struct MtI2cDev *i2c) {
#if defined(CFG_I2C_CH0_DMA_SUPPORT) || defined(CFG_I2C_CH1_DMA_SUPPORT)
    DMA_CHAN dma_ch;
    int ret;
#endif

    if (i2c->dma_mode) {
        switch(i2c->id) {
#ifdef CFG_I2C_CH0_DMA_SUPPORT
        case 0:
            dma_ch = I2C0_DMA_CH;
            break;
#endif
#ifdef CFG_I2C_CH1_DMA_SUPPORT
        case 1:
            dma_ch = I2C1_DMA_CH;
            break;
#endif
            default:
                /*I2CERR("i2c%ld: not support DMA\n", i2c->id);*/
                configASSERT(0);
                return;
        }
#if defined(CFG_I2C_CH0_DMA_SUPPORT) || defined(CFG_I2C_CH1_DMA_SUPPORT)
        ret = mt_req_gdma(dma_ch);
        if (ret < 0) {
            /*I2CERR("i2c%ld: DMA channel req err (%d)\n", i2c->id, ret);*/
            configASSERT(0);
        }
#endif
    }
}

static void mt_i2c_free_gdma(struct MtI2cDev *i2c) {
#if defined(CFG_I2C_CH0_DMA_SUPPORT) || defined(CFG_I2C_CH1_DMA_SUPPORT)
    DMA_CHAN dma_ch;
    int ret;
#endif

    if (i2c->dma_mode) {
        switch(i2c->id) {
#ifdef CFG_I2C_CH0_DMA_SUPPORT
            case 0:
                dma_ch = I2C0_DMA_CH;
                break;
#endif
#ifdef CFG_I2C_CH1_DMA_SUPPORT
            case 1:
                dma_ch = I2C1_DMA_CH;
                break;
#endif
            default:
                /*I2CERR("i2c%ld: not support DMA\n", i2c->id);*/
                configASSERT(0);
                return;
        }
#if defined(CFG_I2C_CH0_DMA_SUPPORT) || defined(CFG_I2C_CH1_DMA_SUPPORT)
        ret = mt_free_gdma(dma_ch);
        if (ret < 0) {
            /*I2CERR("i2c%ld: DMA channel free err (%d)\n", i2c->id, ret);*/
            configASSERT(0);
        }
#endif
    }
}
static void mtI2cStartEnable(struct MtI2cDev *i2c) {
    uint16_t addr_reg, intr_mask;
    uint16_t control_reg;
    uint32_t control_dma;
    int data_size;
    const uint8_t *ptr;

    if ((i2c->state.tx.size > 8) || (i2c->state.rx.size > 8))
        i2c->dma_mode = true;
    else
        i2c->dma_mode = false;
    mt_i2c_req_gdma(i2c);
    i2c_get_lock(i2c);

    /* Update IIC timing here*/
    mtI2cSpeedSet(i2c);

    control_reg = I2C_CONTROL_ACKERR_DET_EN | I2C_CONTROL_CLK_EXT_EN;
    if (i2c->dma_mode == true) /* DMA */
        control_reg |= I2C_CONTROL_DMA_EN | I2C_CONTROL_DMAACK_EN | I2C_CONTROL_ASYNC_MODE;;

    if (i2c->op == I2C_MASTER_WRRD)
        control_reg |= I2C_CONTROL_DIR_CHANGE | I2C_CONTROL_RS; /* default use repeated start */
    i2c_writew(control_reg, i2c, OFFSET_CONTROL);

    i2c_writew(I2C_DELAY_LEN, i2c, OFFSET_DELAY_LEN);

    addr_reg = i2c->addr << 1;
    if (i2c->op == I2C_MASTER_RD)
        addr_reg |= 0x1;
    i2c_writew(addr_reg, i2c, OFFSET_SLAVE_ADDR);

    /* Clear interrupt status */
    i2c_writew(0x1ff, i2c, OFFSET_INTR_STAT);
    i2c_writew(I2C_FIFO_ADDR_CLR, i2c, OFFSET_FIFO_ADDR_CLR);
    /* Enable interrupt */
    intr_mask = I2C_TRANSAC_COMP;
    if (i2c->ver == 2)
        intr_mask |= I2C_MASTER_ERR | I2C_IBI_INTR;
    i2c_writew(intr_mask, i2c, OFFSET_INTR_MASK);

    /* Set transfer and transaction len */
    if (i2c->op == I2C_MASTER_WRRD) {
        i2c_writew(i2c->state.tx.size, i2c, OFFSET_TRANSFER_LEN);
        i2c_writew(i2c->state.rx.size, i2c, OFFSET_TRANSFER_LEN_AUX);
        i2c_writew(0x02, i2c, OFFSET_TRANSAC_LEN);
    } else if (i2c->op == I2C_MASTER_WR) {
        i2c_writew(i2c->state.tx.size, i2c, OFFSET_TRANSFER_LEN);
        i2c_writew(0x01, i2c, OFFSET_TRANSAC_LEN);
    } else {
        i2c_writew(i2c->state.rx.size, i2c, OFFSET_TRANSFER_LEN);
        i2c_writew(0x01, i2c, OFFSET_TRANSAC_LEN);
    }

    I2CDBG("mtI2cStartEnable I2C next : %d, last : %d\n",  i2c->next, i2c->last);
    I2CDBG("i2c%ld: addr %x, trans size = %d\n", i2c->id, i2c->addr, i2c->state.tx.size);

    if (i2c->dma_mode == true) {
        if (i2c->op == I2C_MASTER_RD) {
            i2c_writew_dma(0x0, i2c, OFFSET_DMA_START);
            i2c_writew_dma((uint32_t)(i2c->base + OFFSET_DATA_PORT) , i2c, OFFSET_SRC_ADDR);
            i2c_writew_dma((uint32_t)i2c->state.rx.buf, i2c, OFFSET_DST_ADDR);
            i2c_writew_dma(i2c->state.rx.size, i2c, OFFSET_COUNT);
            control_dma = DMA_WPSD | DMA_DRQ | DMA_DINC;
            i2c_writew_dma(control_dma, i2c, OFFSET_CON);
        } else if (i2c->op == I2C_MASTER_WR) {
            i2c_writew_dma(0x0, i2c, OFFSET_DMA_START);
            i2c_writew_dma((uint32_t)(i2c->base + OFFSET_DATA_PORT) , i2c, OFFSET_DST_ADDR);
            i2c_writew_dma((uint32_t)i2c->state.tx.buf, i2c, OFFSET_SRC_ADDR);
            i2c_writew_dma(i2c->state.tx.size, i2c, OFFSET_COUNT);
            control_dma = DMA_WPSD | DMA_DRQ | DMA_SINC;
            i2c_writew_dma(control_dma, i2c, OFFSET_CON);
        } else if (i2c->op == I2C_MASTER_WRRD) {
            /* we don't support I2C_MASTER_WRRD ON MT6758 */
        }

        I2C_MB();
        i2c_writew_dma(DMA_START_EN, i2c, OFFSET_DMA_START);
    }


    if (i2c->dma_mode == false) {
        if (i2c->op != I2C_MASTER_RD) {
            data_size = i2c->state.tx.size;
            ptr = i2c->state.tx.cbuf;
            while (data_size--) {
                i2c_writew(*ptr, i2c, OFFSET_DATA_PORT);
                I2CDBG("i2c%ld: addr %x write byte = 0x%x\n", i2c->id, i2c->addr, *ptr);
                ptr++;
            }
        }
    }
    if (atomicReadByte(&i2c->state.masterState)!= MT_I2C_MASTER_START_SYNC) {
        if (i2c->state.timerHandle == 0) {
            i2c->state.timerHandle = timTimerSet(i2c->state.timeOut, 0, 50, I2cTimerCallback, i2c, true);
            if (i2c->state.timerHandle == 0)
                I2CERR("i2c%ld:i2c->state.timerHandle=%ld\n",
                             i2c->id, i2c->state.timerHandle);
        }
        else
            configASSERT(0);
    }

    /* All register must be prepared before setting the start bit [SMP] */
    I2C_MB();
    i2c_writew(I2C_TRANSAC_START, i2c, OFFSET_START);
    /***** i2c_dump_info(i2c); *****/

    return;
}

static void mtI2cMasterTxRxDone(struct MtI2cDev *pdev, int err) {
    struct I2cMtState *state = &pdev->state;
    size_t txOffst = state->tx.offset;
    size_t rxOffst = state->rx.offset;
    uint32_t id;
    int i;
    struct MtI2cXfer *xfer;

    /*****
    if (pdev->board->sleepDev >= 0)
        platReleaseDevInSleepMode(pdev->board->sleepDev);
    *****/

    i2c_release_lock(pdev);
    mt_i2c_free_gdma(pdev);

    state->tx.offset = 0;
    state->rx.offset = 0;

    if (err && (state->retry-- > 0)) {
        mt_i2c_init_hw(pdev);
        mtI2cStartEnable(pdev);
        state->err = err;
        return;
    }

    if (state->tx.callback != NULL)
        state->tx.callback(state->tx.cookie, txOffst, rxOffst, err);
    state->err = err;
    extern TaskHandle_t CHRE_TaskHandle;
    if (xTaskResumeFromISR(CHRE_TaskHandle) == pdTRUE) {
        portYIELD_WITHIN_API();
    }
    do {
        id = atomicAdd32bits(&pdev->next, 1);
    } while (!id);

    for (i = 0; i < I2C_MAX_QUEUE_DEPTH; i++) {
        xfer = &mXfers[i];

        if (xfer->busId == (pdev - mMtI2cDevs) &&
                atomicCmpXchg32bits(&xfer->id, id, 0)) {
            pdev->addr = xfer->addr;
            state->tx.cbuf = xfer->txBuf;
            state->tx.offset = 0;
            state->tx.size = xfer->txSize;
            state->tx.callback = xfer->callback;
            state->tx.cookie = xfer->cookie;
            state->rx.buf = xfer->rxBuf;
            state->rx.offset = 0;
            state->rx.size = xfer->rxSize;
            state->rx.callback = NULL;
            state->rx.cookie = NULL;
            pdev->op = xfer->op;
            state->retry = xfer->retry;
            atomicWriteByte(&state->masterState, MT_I2C_MASTER_START);
            /*****
            if (pdev->board->sleepDev >= 0)
                platRequestDevInSleepMode(pdev->board->sleepDev, 12);
            *****/
            mtI2cPutXfer(xfer);
            mtI2cStartEnable(pdev);
            return;
        }
    }

    i2c_clock_disable(pdev);
    atomicWriteByte(&state->masterState, MT_I2C_MASTER_IDLE);
}

void i2c_dump_info(struct MtI2cDev *i2c) {
    I2CERR("\r\ni2c%ld ba %p\n", i2c->id, (void*)i2c->base);
    I2CERR(I2CTAG "SAR=%x,IMK=%x,IST=%x,CTL=%x,TRSL=%x\r\n"
           I2CTAG "TSACL=%x,HTM=%x,LTM=%x,S=%x,FS=%x\r\n"
           I2CTAG "IO=%x,HS=%x,DST=%x,EXC=%x,TRSLA=%x\r\n"
           I2CTAG "CLD=%x\r\n",
           (i2c_readw(i2c, OFFSET_SLAVE_ADDR)),
           (i2c_readw(i2c, OFFSET_INTR_MASK)),
           (i2c_readw(i2c, OFFSET_INTR_STAT)),
           (i2c_readw(i2c, OFFSET_CONTROL)),
           (i2c_readw(i2c, OFFSET_TRANSFER_LEN)),
           (i2c_readw(i2c, OFFSET_TRANSAC_LEN)),
           (i2c_readw(i2c, OFFSET_TIMING)),
           (i2c_readw(i2c, OFFSET_LTIMING)),
           (i2c_readw(i2c, OFFSET_START)),
           (i2c_readw(i2c, OFFSET_FIFO_STAT)),
           (i2c_readw(i2c, OFFSET_IO_CONFIG)),
           (i2c_readw(i2c, OFFSET_HS)),
           (i2c_readw(i2c, OFFSET_DEBUGSTAT)),
           (i2c_readw(i2c, OFFSET_EXT_CONF)),
           (i2c_readw(i2c, OFFSET_TRANSFER_LEN_AUX)),
           (i2c_readw(i2c, OFFSET_CLOCK_DIV)));
}
#if 0
void i2c_dump_dma_info(struct MtI2cDev *i2c) {

    I2CERR("\r\ni2c%ld base dma address %p\n", i2c->id, (void*)i2c->dma_base);
    I2CERR("I2C%ld dma register:\r\n"
           I2CTAG "SRC_ADDR=%lx,DST_ADDR=%lx,WPPT=%lx,WPPO=%lx,COUNT=%lx,CON=%lx\r\n"
           I2CTAG "DMA_START=%lx,INTSTA=%lx,ACKINT=%lx,RLCT=%lx,LIMITER=%lx\r\n",
           i2c->id,
           (i2c_readw_dma(i2c, OFFSET_SRC_ADDR)),
           (i2c_readw_dma(i2c, OFFSET_DST_ADDR)),
           (i2c_readw_dma(i2c, OFFSET_WPPT)),
           (i2c_readw_dma(i2c, OFFSET_WPPO)),
           (i2c_readw_dma(i2c, OFFSET_COUNT)),
           (i2c_readw_dma(i2c, OFFSET_CON)),
           (i2c_readw_dma(i2c, OFFSET_DMA_START)),
           (i2c_readw_dma(i2c, OFFSET_INTSTA)),
           (i2c_readw_dma(i2c, OFFSET_ACKINT)),
           (i2c_readw_dma(i2c, OFFSET_RLCT)),
           (i2c_readw_dma(i2c, OFFSET_LIMITER)));
}
#endif

static void mtI2cIsrEvent(struct MtI2cDev *i2c) {
    uint16_t intr_stat;
    uint16_t data_size;
    uint8_t *ptr;
    int err = 0;

    /* Clear interrupt mask */
    i2c_writew(0, i2c, OFFSET_INTR_MASK);
    intr_stat = i2c_readw(i2c, OFFSET_INTR_STAT);
    i2c_writew(0x1ff, i2c, OFFSET_INTR_STAT);

    if (!intr_stat) {/* the intr_stat is 0, it means the status is always cleared */
         I2CERR("the old intr trigger,do nothing and return\n");
         return;
    }
    if (atomicReadByte(&i2c->state.masterState) != MT_I2C_MASTER_START_SYNC) {
        if (i2c->state.timerHandle != 0) {
            if (timTimerCancel(i2c->state.timerHandle))
                i2c->state.timerHandle = 0;
            else
                I2CERR("i2c%ld:intr_stat=0x%x,timTimerCancel fail\n",
                    i2c->id, intr_stat);
        } else {
            I2CERR("i2c%ld:intr_stat=0x%x,retry=%lu\n",
                i2c->id, intr_stat, i2c->state.retry);
            if (i2c->ver == 2)
                i2c_writew(I2C_SOFT_RST | I2C_ERR_RST, i2c, OFFSET_SOFTRESET);
            configASSERT(0);
        }
    }
    /* check the intr status */
    if (intr_stat & (I2C_HS_NACKERR | I2C_ACKERR)) {
        if (!i2c->state.retry)
            i2c_dump_info(i2c);
        mt_i2c_init_hw(i2c);
        err = -121; /* need to check */
        goto out;
    }
    /* check IBI intr*/
    if (intr_stat & I2C_IBI_INTR) {
        i2c_writew(I2C_TRANSAC_COMP | I2C_MASTER_ERR | I2C_IBI_INTR, i2c, OFFSET_INTR_MASK);
        i2c_writew(i2c_readw(i2c, OFFSET_CONTROL) | I2C_CONTROL_RS, i2c, OFFSET_CONTROL);
        i2c_writew(I2C_ERR_RST, i2c, OFFSET_SOFTRESET);
        err = -121;
        goto out;
    }

    if (i2c->op != I2C_MASTER_WR && i2c->dma_mode == false) {
        data_size = i2c->state.rx.size;
        ptr = i2c->state.rx.buf;
        I2CDBG("i2c%ld: read size = 0x%x\n", i2c->id, data_size);
        while (data_size--) {
            *ptr = i2c_readw(i2c, OFFSET_DATA_PORT);
            I2CDBG("i2c%ld: addr 0x%x read byte = 0x%x\n", i2c->id, i2c->state.tx.cbuf[0], *ptr);
            ptr++;
        }
        err = 0;
    }
    I2CDBG("i2c%ld: transfer done.\n", i2c->id);

out:
    mtI2cMasterTxRxDone(i2c, err);
    return;
}

static void i2c0_irq_handler(void) {
    mtI2cIsrEvent(&mMtI2cDevs[0]);
}

static void i2c1_irq_handler(void) {
    mtI2cIsrEvent(&mMtI2cDevs[1]);
}

/* calculate i2c port speed */
static int32_t mtk_i2c_calculate_speed(struct MtI2cDev *i2c,
    uint32_t clk_src_in_khz,
    uint32_t speed_hkz,
    uint32_t *timing_step_cnt,
    uint32_t *timing_sample_cnt) {

    uint32_t khz;
    uint32_t step_cnt;
    uint32_t sample_cnt;
    uint32_t sclk;
    uint32_t hclk;
    uint32_t max_step_cnt;
    uint32_t sample_div = MAX_SAMPLE_CNT_DIV;
    uint32_t step_div;
    uint32_t min_div;
    uint32_t best_mul;
    uint32_t cnt_mul;

    if (speed_hkz > MAX_HS_MODE_SPEED) {
        return -EINVAL_I2C;
    } else if (speed_hkz > MAX_FS_MODE_SPEED) {
        max_step_cnt = MAX_HS_STEP_CNT_DIV;
    } else {
        max_step_cnt = MAX_STEP_CNT_DIV;
    }
    step_div = max_step_cnt;

    /* Find the best combination */
    khz = speed_hkz;
    hclk = clk_src_in_khz;
    min_div = ((hclk >> 1) + khz - 1) / khz;
    best_mul = MAX_SAMPLE_CNT_DIV * max_step_cnt;
    for (sample_cnt = 1; sample_cnt <= MAX_SAMPLE_CNT_DIV; sample_cnt++) {
        step_cnt = (min_div + sample_cnt - 1) / sample_cnt;
        cnt_mul = step_cnt * sample_cnt;
        if (step_cnt > max_step_cnt)
            continue;
        if (cnt_mul < best_mul) {
            best_mul = cnt_mul;
            sample_div = sample_cnt;
            step_div = step_cnt;
            if (best_mul == min_div)
                break;
        }
    }
    sample_cnt = sample_div;
    step_cnt = step_div;
    sclk = hclk / (2 * sample_cnt * step_cnt);
    if (sclk > khz) {
        I2CERR("i2c%lu %s mode: unsupported speed (%ldkhz)\n",
               i2c->id, (speed_hkz > MAX_FS_MODE_SPEED) ?  "HS" : "ST/FT", (long int)khz);
        return -EINVAL_I2C;
    }

    step_cnt--;
    sample_cnt--;

    *timing_step_cnt = step_cnt;
    *timing_sample_cnt = sample_cnt;

    return 0;
}

/*Set i2c port speed*/
static int32_t i2c_set_speed(struct MtI2cDev *i2c) {
    int ret = 0;
    uint32_t step_cnt = 0;
    uint32_t sample_cnt = 0;
    uint32_t l_step_cnt = 0;
    uint32_t l_sample_cnt = 0;
    uint32_t speed_khz = i2c->speedInHz / 1000;
    uint32_t clk_src_in_khz = i2c->clk_src_in_khz;
    uint32_t duty = 50;
    uint16_t high_speed_reg;
    uint16_t timing_reg = 0;
    uint16_t ltiming_reg = 0;

    if (clk_src_in_khz == 0)
        return -EINVAL_I2C;

    if (speed_khz > MAX_FS_MODE_SPEED) {
        /* Set the high speed mode register */
        ret = mtk_i2c_calculate_speed(i2c, clk_src_in_khz,
                    MAX_FS_MODE_SPEED, &l_step_cnt, &l_sample_cnt);
        if (ret < 0)
            return ret;

        ret = mtk_i2c_calculate_speed(i2c, clk_src_in_khz,
                    speed_khz, &step_cnt, &sample_cnt);
        if (ret < 0)
            return ret;

        high_speed_reg = 0x3 |
                (sample_cnt & 0x7) << 12 |
                (step_cnt & 0x7) << 8;

        timing_reg = (l_sample_cnt & 0x7) << 8 |
                (l_step_cnt & 0x3f) << 0;

        if (i2c->set_ltiming)
                ltiming_reg = (l_sample_cnt << 6) | (l_step_cnt << 0) |
                        (sample_cnt & 0x7) << 12 |
                        (step_cnt & 0x7) << 9;
    } else {
        if (speed_khz > MAX_ST_MODE_SPEED && speed_khz <= MAX_FS_MODE_SPEED)
                duty = DUTY_CYCLE;
        ret = mtk_i2c_calculate_speed(i2c, clk_src_in_khz,
              (speed_khz * 50 / duty), &step_cnt, &sample_cnt);
        if (ret < 0)
            return ret;

        ret = mtk_i2c_calculate_speed(i2c, clk_src_in_khz,
              (speed_khz * 50 / (100 - duty)), &l_step_cnt, &l_sample_cnt);
        if (ret < 0)
            return ret;

        timing_reg = (sample_cnt & 0x7) << 8 |
                     (step_cnt & 0x3f) << 0;

        if (i2c->set_ltiming)
            ltiming_reg = (l_sample_cnt & 0x7) << 6 |
                          (l_step_cnt & 0x3f) << 0;
        /* Disable the high speed transaction */
        high_speed_reg = 0x0;
    }

    i2c_writew(timing_reg, i2c, OFFSET_TIMING);
    i2c_writew(high_speed_reg, i2c, OFFSET_HS);
    if (i2c->set_ltiming)
        i2c_writew(ltiming_reg, i2c, OFFSET_LTIMING);

    return 0;
}

static int mtI2cSpeedSet(struct MtI2cDev *i2c) {
#ifdef CFG_VCORE_DVFS_SUPPORT
#ifdef GET_CUR_FREQ_FROM_DVFS
    uint32_t freq;

    freq = get_cur_opp();
    /*Don't support user's setting speedInHZ now*/
    /*Set 400KHz*/
    if (freq == FREQ_416MHZ)
        i2c_writew(0x0c, i2c, OFFSET_TIMING);
    else if (freq == FREQ_330MHZ)
        i2c_writew(0x0a, i2c, OFFSET_TIMING);
    else if (freq == FREQ_286MHZ)
        i2c_writew(0x08, i2c, OFFSET_TIMING);
    else
        i2c_writew(0x06, i2c, OFFSET_TIMING);

    I2CDBG("%s: freq = %d\n", __func__, freq);
#else
    /* ulposc use 250MHz */
    //i2c_writew(0x07, i2c, OFFSET_TIMING);
    /* workaround use system timer 26M, before ulposc ready */
    i2c_writew(0x06, i2c, OFFSET_TIMING);
#endif
#endif

    if ((i2c->speedInHz == 0) || i2c_set_speed(i2c) < 0)
        i2c_writew(0x102, i2c, OFFSET_HS);

    i2c_writew(I2C_FS_START_CON, i2c, OFFSET_EXT_CONF);

    /* push pull */
    i2c_writew(I2C_IO_CONFIG_OPEN_DRAIN, i2c, OFFSET_IO_CONFIG);

    if (i2c->ver == 2)
        i2c_writew((DEFAULT_DIV - 1) << 8 | (DEFAULT_DIV - 1),
                   i2c, OFFSET_CLOCK_DIV);
    else
        i2c_writew(DEFAULT_DIV - 1, i2c, OFFSET_CLOCK_DIV);

    return 0;
}

int i2cMasterRequest(uint32_t busId, uint32_t speedInHz) {
    /* mt6771 i2c on-chip <=> off-chip mapping */
    /* scp_i2c0<=>ap_i2c1 and scp_i2c1<=>ap_i2c6 */
    uint32_t busId_t = 0;
    if (busId == 1)
        busId_t = 0;
    else if (busId == 6)
        busId_t = 1;
    else {
        /* I2CERR("%s BusId not support %d\n", __func__, __LINE__); */
        return -EINVAL;
    }

    if (busId_t >= ARRAY_SIZE(mMtI2cDevs))
        return -EINVAL;

    struct MtI2cDev *pdev = &mMtI2cDevs[busId_t];
    struct I2cMtState *state = &pdev->state;

    I2CDBG("%s: %d\n", __func__, __LINE__);

    if (state->mode == MT_I2C_DISABLED) {
        state->mode = MT_I2C_MASTER;
        state->timerHandle = 0;
        state->timeOut = 2000000000;

        pdev->next = 2;
        pdev->last = 1;
        atomicBitsetInit(mXfersValid, I2C_MAX_QUEUE_DEPTH);
        atomicWriteByte(&state->masterState, MT_I2C_MASTER_IDLE);
        pdev->speedInHz = speedInHz;

        /* Need enable clock */
        i2c_clock_enable(pdev);
        mtI2cSpeedSet(pdev);

        request_irq(I2C0_IRQn, i2c0_irq_handler, "I2C0");
        request_irq(I2C1_IRQn, i2c1_irq_handler, "I2C1");
#ifdef CFG_VCORE_DVFS_SUPPORT
        scp_wakeup_src_setup(I2C0_WAKE_CLK_CTRL, 1);
        scp_wakeup_src_setup(I2C1_WAKE_CLK_CTRL, 1);
#endif
        i2c_clock_disable(pdev);
    } else {
        return -EBUSY;
    }
    /* LOG here */
    return 0;
}

int i2cMasterRelease(uint32_t busId) {
    /* mt6771 i2c on-chip <=> off-chip mapping */
    /* scp_i2c0<=>ap_i2c1 and scp_i2c1<=>ap_i2c6 */
    uint32_t busId_t = 0;
    if (busId == 1)
        busId_t = 0;
    else if (busId == 6)
        busId_t = 1;
    else {
        /* I2CERR("%s BusId not support %d\n", __func__, __LINE__); */
        return -EINVAL;
    }

    struct MtI2cDev *pdev = &mMtI2cDevs[busId_t];
    struct I2cMtState *state = &pdev->state;

    I2CDBG("%s: %d\n", __func__, __LINE__);

    if (state->mode == MT_I2C_MASTER) {
        if (atomicReadByte(&state->masterState) == MT_I2C_MASTER_IDLE) {
            state->mode = MT_I2C_DISABLED;
            // mtI2cIrqEnable(pdev, I2C_CR2_ITERREN | I2C_CR2_ITEVTEN);
            // mtI2cDisable(pdev);
            // pwrUnitClock(PERIPH_BUS_APB1, cfg->clock, false);

            // gpioRelease(pdev->scl);
            // gpioRelease(pdev->sda);

            return 0;
        } else {
            return -EBUSY;
        }
    } else {
        return -EINVAL;
    }
}

int i2cMasterTxRxSync(uint32_t busId, uint32_t addr, const void *txBuf, size_t txSize,
                  void *rxBuf, size_t rxSize, I2cCallbackF callback, void *cookie) {
    uint32_t id;
    unsigned long long timeout;

    /* mt6771 i2c on-chip <=> off-chip mapping */
    /* scp_i2c0<=>ap_i2c1 and scp_i2c1<=>ap_i2c6 */
    uint32_t busId_t = 0;
    if (busId == 1)
        busId_t = 0;
    else if (busId == 6)
        busId_t = 1;
    else {
        /* I2CERR("%s BusId not support %d\n", __func__, __LINE__); */
        return -EINVAL;
    }

    if (busId_t >= ARRAY_SIZE(mMtI2cDevs))
        return -EINVAL;
    else if (addr & 0x80)
        return -ENXIO;

    struct MtI2cDev *pdev = &mMtI2cDevs[busId_t];
    struct I2cMtState *state = &pdev->state;

    struct MtI2cXfer *xfer = mtI2cGetXfer();

    timeout = get_boot_time_ns() + 5000000;
    while (atomicReadByte(&state->masterState)!= MT_I2C_MASTER_IDLE &&
                get_boot_time_ns() < timeout);
    if (atomicReadByte(&state->masterState)!= MT_I2C_MASTER_IDLE) {
        mtI2cPutXfer(xfer);
                return -ETIMEDOUT_I2C;
    }

    if (xfer) {
        xfer->busId = busId_t;
        xfer->addr = addr;
        if (txSize > I2C_MAX_TXBUF_SIZE) {
            mtI2cPutXfer(xfer);
            return -ENXIO;
        }
        memcpy(xfer->txBuf, txBuf, txSize);
        // xfer->txBuf = txBuf;
        xfer->txSize = txSize;
        xfer->rxBuf = rxBuf;
        xfer->rxSize = rxSize;
        xfer->callback = callback;
        xfer->cookie = cookie;
        /* need to check */
        if (xfer->txSize == 0 && xfer->rxSize == 0) {
            mtI2cPutXfer(xfer);
            return -EINVAL;
        } else if (xfer->rxSize == 0)
            xfer->op = I2C_MASTER_WR;
        else if (xfer->txSize == 0)
            xfer->op = I2C_MASTER_RD;
        else
            xfer->op = I2C_MASTER_WRRD;

        /* Because DMA mode don't support write-read style. Returen error to user */
        if ((xfer->rxSize > 0) && (xfer->txSize > 0) && (xfer->rxSize > 8 || xfer->txSize > 8)) {
            /*I2CLOG("DMA don't support write-read style\n");*/
            mtI2cPutXfer(xfer);
            return -ENOWR;
        }

        do {
            id = atomicAdd32bits(&pdev->last, 1);
        } while (!id);

        // after this point the transfer can be picked up by the transfer
        // complete interrupt
        atomicWrite32bits(&xfer->id, id);

        // only initiate transfer here if we are in IDLE. Otherwise the transfer
        // completion interrupt will start the next transfer (not necessarily
        // this one)
        //while (state->masterState == MT_I2C_MASTER_START && count--);
        if (atomicCmpXchgByte((uint8_t *)&state->masterState,
                              MT_I2C_MASTER_IDLE, MT_I2C_MASTER_START_SYNC)) {
            // it is possible for this transfer to already be complete by the
            // time we get here. if so, transfer->id will have been set to 0.
            if (xfer->op == I2C_MASTER_WR)
                I2CDBG("i2c%ld: Tx buf addr 0x%x write size 0x%x\n", pdev->id,
                  xfer->txBuf, xfer->txSize);
            else if (xfer->op == I2C_MASTER_WRRD)
                I2CDBG("i2c%ld: TxRX buf addr 0x%x\n", pdev->id, xfer->txBuf);
            if (atomicCmpXchg32bits(&xfer->id, id, 0)) {
                pdev->addr = xfer->addr;
                state->tx.cbuf = xfer->txBuf;
                state->tx.offset = 0;
                state->tx.size = xfer->txSize;
                state->tx.callback = xfer->callback;
                state->tx.cookie = xfer->cookie;
                state->retry = 2;
                state->rx.buf = xfer->rxBuf;
                state->rx.offset = 0;
                state->rx.size = xfer->rxSize;
                state->rx.callback = NULL;
                state->rx.cookie = NULL;
                state->err = 0;
                pdev->op = xfer->op;

                mtI2cPutXfer(xfer);

                i2c_clock_enable(pdev);
                mtI2cStartEnable(pdev);
            } else {
                atomicWriteByte(&state->masterState, MT_I2C_MASTER_IDLE);
                I2CERR("i2c%ld: %s error, %d, %lu, %lu\n", pdev->id, __func__,
                    state->masterState, xfer->id, id);
            }
        }
        timeout = get_boot_time_ns() + 5000000;
        while (atomicReadByte(&state->masterState)!= MT_I2C_MASTER_IDLE &&
            get_boot_time_ns() < timeout);
        if (atomicReadByte(&state->masterState)!= MT_I2C_MASTER_IDLE) {
            I2CLOG("i2c%ld: i2cMasterTxRxsync xfer timeout %d\n",
                   pdev->id, state->masterState);
            mtI2cMasterTxRxDone(pdev, -ETIMEDOUT_I2C);
        }
        //atomicWriteByte(&state->masterState, MT_I2C_MASTER_IDLE);
        return state->err;
    } else {
        /* I2CERR("i2c%ld: i2cMasterTxRx xfer == NULL\n", pdev->id); */
        return -EBUSY;
    }
}

int i2cMasterTxRxRetry(uint32_t busId, uint32_t addr, const void *txBuf, size_t txSize,
                       void *rxBuf, size_t rxSize, I2cCallbackF callback, void *cookie,
                       uint32_t retry) {
    uint32_t id;
    /* mt6771 i2c on-chip <=> off-chip mapping */
    /* scp_i2c0<=>ap_i2c1 and scp_i2c1<=>ap_i2c6 */
    uint32_t busId_t = 0;
    if (busId == 1)
        busId_t = 0;
    else if (busId == 6)
        busId_t = 1;
    else {
        I2CLOG("%s BusId not support %d\n", __func__, __LINE__);
        return -EINVAL;
    }

    if (busId_t >= ARRAY_SIZE(mMtI2cDevs))
        return -EINVAL;
    else if (addr & 0x80)
        return -ENXIO;

    struct MtI2cDev *pdev = &mMtI2cDevs[busId_t];
    struct I2cMtState *state = &pdev->state;

    struct MtI2cXfer *xfer = mtI2cGetXfer();

    if (xfer) {
        xfer->busId = busId_t;
        xfer->addr = addr;
        if (txSize > I2C_MAX_TXBUF_SIZE) {
            mtI2cPutXfer(xfer);
            return -ENXIO;
        }
        memcpy(xfer->txBuf, txBuf, txSize);
        // xfer->txBuf = txBuf;
        xfer->txSize = txSize;
        xfer->rxBuf = rxBuf;
        xfer->rxSize = rxSize;
        xfer->callback = callback;
        xfer->cookie = cookie;
        xfer->retry = retry;
        /* need to check */
        if (xfer->txSize == 0 && xfer->rxSize == 0) {
            mtI2cPutXfer(xfer);
            return -EINVAL;
        } else if (xfer->rxSize == 0)
            xfer->op = I2C_MASTER_WR;
        else if (xfer->txSize == 0)
            xfer->op = I2C_MASTER_RD;
        else
            xfer->op = I2C_MASTER_WRRD;

        /* Because DMA mode don't support write-read style. Returen error to user */
        if ((xfer->rxSize > 0) && (xfer->txSize > 0) && (xfer->rxSize > 8 || xfer->txSize > 8)) {
            /* I2CLOG("DMA don't support write-read style\n"); */
            mtI2cPutXfer(xfer);
            return -ENOWR;
        }

        do {
            id = atomicAdd32bits(&pdev->last, 1);
        } while (!id);

        // after this point the transfer can be picked up by the transfer
        // complete interrupt
        atomicWrite32bits(&xfer->id, id);

        // only initiate transfer here if we are in IDLE. Otherwise the transfer
        // completion interrupt will start the next transfer (not necessarily
        // this one)
        if (atomicCmpXchgByte((uint8_t *)&state->masterState,
                              MT_I2C_MASTER_IDLE, MT_I2C_MASTER_START)) {
            // it is possible for this transfer to already be complete by the
            // time we get here. if so, transfer->id will have been set to 0.
            if (xfer->op == I2C_MASTER_WR)
                I2CDBG("i2c%ld: Tx buf addr 0x%x write size 0x%x\n", pdev->id,
                  xfer->txBuf, xfer->txSize);
            else if (xfer->op == I2C_MASTER_WRRD)
                I2CDBG("i2c%ld: TxRX buf addr 0x%x\n", pdev->id, xfer->txBuf);
            if (atomicCmpXchg32bits(&xfer->id, id, 0)) {
                pdev->addr = xfer->addr;
                state->tx.cbuf = xfer->txBuf;
                state->tx.offset = 0;
                state->tx.size = xfer->txSize;
                state->tx.callback = xfer->callback;
                state->tx.cookie = xfer->cookie;
                state->rx.buf = xfer->rxBuf;
                state->rx.offset = 0;
                state->rx.size = xfer->rxSize;
                state->rx.callback = NULL;
                state->rx.cookie = NULL;
                state->retry = xfer->retry;
                pdev->op = xfer->op;

                mtI2cPutXfer(xfer);

                i2c_clock_enable(pdev);
                mtI2cStartEnable(pdev);
            } else {
                atomicWriteByte(&state->masterState, MT_I2C_MASTER_IDLE);
                I2CERR("i2c%ld: %s error, %d, %lu, %lu\n", pdev->id, __func__,
                    state->masterState, xfer->id, id);
            }
        }
        return 0;
    } else {
        /* I2CERR("i2c%ld: i2cMasterTxRx xfer == NULL\n", pdev->id); */
        return -EBUSY;
    }
}

int i2cMasterTxRx(uint32_t busId, uint32_t addr, const void *txBuf, size_t txSize,
                  void *rxBuf, size_t rxSize, I2cCallbackF callback, void *cookie) {
    return i2cMasterTxRxRetry(busId, addr, txBuf, txSize,
                      rxBuf, rxSize, callback, cookie, 3);
}

void i2cCallback(void *cookie, size_t tx, size_t rx, int err) {
    I2CLOG("i2c transfer -------> err : %d\r\n", err);
}

static void mt_i2c_mask_and_clear_int(struct MtI2cDev *i2c)
{
    if (i2c) {
        /* mask all interrupt */
        i2c_writew(0, i2c, OFFSET_INTR_MASK);
        /* clear all interrupt */
        i2c_writew(0x1ff, i2c, OFFSET_INTR_STAT);
    }
}

int32_t i2c_init(void) {
    int i = 0;
#ifdef CFG_VCORE_DVFS_SUPPORT
    int32_t tmo_cnt = 1000;
    uint32_t reg;
#endif

    /* init wake lock */
    wake_lock_init(&mMtI2cDevs[0].wakelock, "i2c0");
    wake_lock_init(&mMtI2cDevs[1].wakelock, "i2c1");

#ifdef CFG_VCORE_DVFS_SUPPORT
    /* set clock src ulposc_div8 250Mhz/8 = 31.25Mhz for I2C1 (MT6771)*/
    DRV_SetReg32(SCP_BCLK_CK_SEL, SCP_BCLK_CK_SEL_ULPOSC_DIV8);

    /* wait for status update */
    do {
        tmo_cnt--;
        reg = DRV_Reg32(SCP_BCLK_CK_SEL);
    } while(((reg & SCP_BCLK_CK_STA_MASK) != SCP_BCLK_CK_STA_ULPOSC_DIV8) && tmo_cnt);

    if (tmo_cnt == 0)
        I2CERR("clk update err 0x%lx\n", reg);

    /* set I3C0 clock src ulposc_div4 250Mhz/4 = 62.5Mhz for I3C0 (MT6771)*/
     DRV_SetReg32(SCP_I3C_BCLK_SW, SCP_I3C_BCLK_SW_ULPOSC_DIV4);

    /* wait for I3C status update*/
    tmo_cnt = 1000;
    do {
        tmo_cnt--;
        reg = DRV_Reg32(SCP_I3C_BCLK_SW);
    } while (((reg & SCP_BCLK_CK_STA_MASK) != SCP_I3C_BCLK_CK_STA_ULPOSC_DIV4) && tmo_cnt);

    if (tmo_cnt == 0)
        I2CERR("I3C clk update err 0x%lx\n", reg);
#endif

    for (i = 0; i < I2C_NR; i++) {
        i2c_clock_enable(&mMtI2cDevs[i]);
        mt_i2c_mask_and_clear_int(&mMtI2cDevs[i]);
        mt_i2c_init_hw(&mMtI2cDevs[i]);
        i2c_set_speed(&mMtI2cDevs[i]);
        if (i == 0) {
            /* clear pending irq to avoid old CIRQ trigger */
            request_irq(I2C0_IRQn, i2c0_irq_handler, "I2C0");
#ifdef CFG_VCORE_DVFS_SUPPORT
            scp_wakeup_src_setup(I2C0_WAKE_CLK_CTRL, 1);
#endif
        } else if (i == 1) {
            /* clear pending irq to avoid old CIRQ trigger */
            request_irq(I2C1_IRQn, i2c1_irq_handler, "I2C1");
#ifdef CFG_VCORE_DVFS_SUPPORT
            scp_wakeup_src_setup(I2C1_WAKE_CLK_CTRL, 1);
#endif
        }
        i2c_clock_disable(&mMtI2cDevs[i]);
    }

    return 0;
}

#ifdef CFG_TESTSUITE_SUPPORT
int islower(int c)
{
    return ((c >= 'a') && (c <= 'z'));
}

int isupper(int c)
{
    return ((c >= 'A') && (c <= 'Z'));
}

int isalpha(int c)
{
    return isupper(c) || islower(c);
}

void hexdump(const void *ptr, size_t len)
{
    uint32_t address = (uint32_t)ptr;
    size_t count;

    I2CPF("\r\ndump address 0x%08x size %d\r\n", address, len);

    for (count = 0 ; count < len; count += 16) {
        I2CPF("0x%08x: ", address);
        I2CPF(" %08x %08x %08x %08x |",
            *(const uint32_t *)address,
            *(const uint32_t *)(address + 4),
            *(const uint32_t *)(address + 8),
            *(const uint32_t *)(address + 12));

        I2CPF("|\r\n");
        address += 16;
    }
}

void hexdump8(const void *ptr, size_t len)
{
    uint32_t address = (uint32_t)ptr;
    size_t count;
    int i;

    I2CPF("\r\ndump address 0x%08x size %d\r\n ", address, len);

    for (count = 0 ; count < len; count += 16) {
        I2CPF("0x%08x: ", address);
        for (i = 0; i < 16; i+=4) {
            I2CPF("%02x %02x %02x %02x ",
                *(const uint8_t *)(address + i),
                *(const uint8_t *)(address + i + 1),
                *(const uint8_t *)(address + i + 2),
                *(const uint8_t *)(address + i + 3));
        }
        I2CPF("\r\n");
        address += 16;
    }
}

int32_t mt_i2c_test(void) {
    int32_t ret = 0;
    int32_t i = 0;

    char tx[2] = {0xD0};
    char rx[2];

    for (i = 1; i < 2; i++) {
        i2cMasterTx(1, 0x77, tx, 1, i2cCallback, NULL);
        i2cMasterRx(1, 0x77, rx, 1, i2cCallback, NULL);
    }
    return ret;
}

#define EEPROM_ADD 0x51
#define EEPROM_SPEED 100000

int32_t mt_i2c_test_1(void) {
    int32_t ret = 0;
    int32_t i;

    I2CLOG("%s\r\n", __func__);

    for (i = 0; i < 16 ;i++) {
        tx_data[i] = i + 0x30;
    }

    tx_data[0] = 0x00; /*word address*/
    tx_data[1] = 0x0D; /*write data*/

    ret = i2cMasterRequest(0, EEPROM_SPEED);
    if (ret < 0) {
        I2CERR("%s i2cMasterRequest for ch0 failed\r\n", __func__);
        return ret;
    }
    /*write*/
    ret = i2cMasterTx(0, EEPROM_ADD, tx_data, 8, i2cCallback, NULL);
    if (ret < 0)
        I2CLOG("%s i2cMasterTx ch0 failed\r\n", __func__);
    i2cMasterRelease(0);

    tx_data[0] = 0x01; /*word address*/
    tx_data[1] = 0xDA; /*write data*/

    ret = i2cMasterRequest(1, EEPROM_SPEED);
    if (ret < 0) {
        I2CLOG("%s i2cMasterRequest for ch1 failed\r\n", __func__);
        return ret;
    }
    /*write*/
    ret = i2cMasterTx(1, EEPROM_ADD, tx_data, 2, i2cCallback, NULL);
    if (ret < 0)
        I2CLOG("%s i2cMasterTx for ch1 failed\r\n", __func__);
    i2cMasterRelease(1);

    I2CLOG("\r\n\r\n");

    return ret;
}

int32_t mt_i2c_test_2(void) {
    int32_t ret = 0;

    I2CLOG("%s\r\n", __func__);

    tx_data[0] = 0x00; /*word address*/
    tx_data[1] = 0x0D; /*write data*/

    ret = i2cMasterRequest(0, EEPROM_SPEED);
    if (ret < 0) {
        I2CLOG("%s i2cMasterRequest for ch0 failed\r\n", __func__);
        return ret;
    }
    /*read*/
    ret = i2cMasterTxRx(0, EEPROM_ADD, tx_data, 1, rx_data, 8, i2cCallback, NULL);
    if (ret < 0) {
        I2CLOG("%s i2cMasterTxRx for ch0 failed\r\n", __func__);
        return ret;
    }
    i2cMasterRelease(0);

    I2CLOG("\r\n\r\n");

    return ret;
}

int32_t mt_i2c_test_3(void) {
    int32_t ret = 0;

    I2CLOG("%s\r\n", __func__);

    tx_data[0] = 0x00; /*word address*/
    tx_data[1] = 0xDA; /*write data*/

    ret = i2cMasterRequest(1, EEPROM_SPEED);
    if (ret < 0) {
        I2CLOG("%s i2cMasterRequest for ch1 failed\r\n", __func__);
        return ret;
    }
    /*read*/
    ret = i2cMasterTxRx(1, EEPROM_ADD, tx_data, 1, rx_data, 8, i2cCallback, NULL);
    if (ret < 0) {
        I2CLOG("%s i2cMasterTxRx for ch1 failed\r\n", __func__);
        return ret;
    }
    i2cMasterRelease(1);

    I2CLOG("\r\n\r\n");

    return ret;
}

void dma_i2c_tx(struct MtI2cDev *i2c, uint32_t src_addr,
                  uint32_t dst_addr, uint32_t count) {
    if (count == 0) {
        I2CERR("%s size = 0\r\n", __func__);
        return;
    }

    i2c_writew_dma(0x0, i2c, OFFSET_DMA_START);
    i2c_writew_dma(src_addr, i2c, OFFSET_SRC_ADDR);
    i2c_writew_dma(dst_addr, i2c, OFFSET_DST_ADDR);
    i2c_writew_dma(count, i2c, OFFSET_COUNT);
    i2c_writew_dma((0x10014|(1<<15)), i2c, OFFSET_CON);

    I2C_MB();
    i2c_writew_dma(0x8000, i2c, OFFSET_DMA_START);
}

void dma_i2c_rx(struct MtI2cDev *i2c, uint32_t src_addr,
                  uint32_t dst_addr, uint32_t count) {
    if (count == 0) {
        I2CERR("%s size = 0\r\n", __func__);
        return;
    }

    i2c_writew_dma(0x0, i2c, OFFSET_DMA_START);
    i2c_writew_dma(src_addr, i2c, OFFSET_SRC_ADDR);
    i2c_writew_dma(dst_addr, i2c, OFFSET_DST_ADDR);
    i2c_writew_dma(count, i2c, OFFSET_COUNT);
    i2c_writew_dma((0x10018|(1<<15)), i2c, OFFSET_CON);

    I2C_MB();
    i2c_writew_dma(0x8000, i2c, OFFSET_DMA_START);
}

#define I2C_OFFSET_CONTROL_VAL (0x324)

int32_t mt_i2c_dma_tx(struct MtI2cDev *i2c) {
    int32_t ret = 0;
    int32_t rw = 0; // read:1, write:0
    uint32_t cnt = 0, i, i2c_addr, dst, src;

    pattern++;

    for (i = 0; i < 16 ;i++) {
        tx_data[i] = i + 0x10 + pattern;
    }

    tx_data[0] = 0x0; /*write address 0*/

    hexdump(tx_data, 64);

    cnt = 16;
    rw = 0; // write
    i2c_addr = (EEPROM_ADD << 1)|(rw & 0x1);
    src = (uint32_t)&tx_data[0];
    dst = (uint32_t)(i2c->base + OFFSET_DATA_PORT);

    i2c_writew(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP,
               i2c, OFFSET_INTR_MASK);
    i2c_writew(I2C_OFFSET_CONTROL_VAL, i2c, OFFSET_CONTROL);
    i2c_writew(cnt, i2c, OFFSET_TRANSFER_LEN);
    i2c_writew(0x1, i2c, OFFSET_DCM_EN); // HW_CG_EN

    i2c_writew(0x1, i2c, OFFSET_TRANSAC_LEN);

    i2c_writew(i2c_addr, i2c, OFFSET_SLAVE_ADDR);

    dma_i2c_tx(i2c, src, dst, cnt);

    I2C_MB();
    i2c_writew(0x01, i2c, OFFSET_START);

    while(i2c_readw(i2c, OFFSET_START));

    i2c_writew(0x1ff, i2c, OFFSET_INTR_STAT);

    hexdump(tx_data, 64);

    return ret;
}

int32_t mt_i2c_dma_rx(struct MtI2cDev *i2c) {
    int32_t ret = 0;
    int32_t rw = 0; // read:1, write:0
    uint32_t cnt = 0, i, i2c_addr, src, dst;

    for (i = 0; i < 64 ;i++) {
        tx_data[i] = 0x00;
        rx_data[i] = 0x00;
    }
    hexdump(tx_data, 64);
    hexdump(rx_data, 64);

    tx_data[0] = 0x0; /*write byte 0*/

    cnt = 1;
    rw = 0; // write
    i2c_addr = (EEPROM_ADD << 1)|(rw & 0x1);
    src = (uint32_t)&tx_data[0];
    dst = (uint32_t)(i2c->base + OFFSET_DATA_PORT);

    i2c_writew(I2C_OFFSET_CONTROL_VAL, i2c, OFFSET_CONTROL);
    i2c_writew(0x0001, i2c, OFFSET_TRANSFER_LEN);

    i2c_writew(0x1, i2c, OFFSET_DCM_EN); // HW_CG_EN

    i2c_writew(0x01, i2c, OFFSET_TRANSAC_LEN);

    i2c_writew(i2c_addr, i2c, OFFSET_SLAVE_ADDR);

    dma_i2c_tx(i2c, src, dst, cnt);

    i2c_writew(0x01, i2c, OFFSET_TRANSAC_LEN);

    I2C_MB();
    i2c_writew(0x01, i2c, OFFSET_START);

    while(i2c_readw(i2c, OFFSET_START));

    i2c_writew(0x1ff, i2c, OFFSET_INTR_STAT);
    i2c_writew(I2C_OFFSET_CONTROL_VAL, i2c, OFFSET_CONTROL);

    /************************************************/

    i2c_writew(0x0, i2c, OFFSET_DCM_EN); // HW_CG_EN
    i2c_writew(0x1, i2c, OFFSET_SOFTRESET);

    I2C_MB();
    i2c_writew_dma(0x0, i2c, OFFSET_DMA_START);

    i2c_writew(I2C_OFFSET_CONTROL_VAL, i2c, OFFSET_CONTROL);

    cnt = 64;
    rw = 1; // read
    i2c_addr = (EEPROM_ADD << 1)|(rw & 0x1);
    src = (uint32_t)(i2c->base + OFFSET_DATA_PORT);
    dst = (uint32_t)&rx_data[0];

    i2c_writew(i2c_addr, i2c, OFFSET_SLAVE_ADDR);
    i2c_writew(cnt, i2c, OFFSET_TRANSFER_LEN);
    i2c_writew(0x1, i2c, OFFSET_DCM_EN); // HW_CG_EN

    i2c_writew(0x01, i2c, OFFSET_TRANSAC_LEN);

    I2C_MB();
    i2c_writew(0x01, i2c, OFFSET_START);
    dma_i2c_rx(i2c, src, dst, cnt);

    /************************************************/
    while(i2c_readw(i2c, OFFSET_START));

    i2c_writew(0x1ff, i2c, OFFSET_INTR_STAT);

    hexdump(tx_data, 64);
    hexdump(rx_data, 64);

    return ret;
}

int32_t mt_i2c_dma_i2c0_tx(void) {
    int32_t ret;
    struct MtI2cDev *i2c = &mMtI2cDevs[0];

    i2c_clock_enable(i2c);
    ret = mt_i2c_dma_tx(i2c);
    i2c_clock_disable(i2c);

    return ret;
}

int32_t mt_i2c_dma_i2c0_rx(void) {
    int32_t ret;
    struct MtI2cDev *i2c = &mMtI2cDevs[0];

    i2c_clock_enable(i2c);
    ret = mt_i2c_dma_rx(i2c);
    i2c_clock_disable(i2c);

    return ret;
}

int32_t mt_i2c_dma_i2c1_tx(void) {
    int32_t ret;
    struct MtI2cDev *i2c = &mMtI2cDevs[1];

    i2c_clock_enable(i2c);
    ret = mt_i2c_dma_tx(i2c);
    i2c_clock_disable(i2c);

    return ret;
}

int32_t mt_i2c_dma_i2c1_rx(void) {
    int32_t ret;
    struct MtI2cDev *i2c = &mMtI2cDevs[1];

    i2c_clock_enable(i2c);
    ret = mt_i2c_dma_rx(i2c);
    i2c_clock_disable(i2c);

    return ret;
}
#endif

#if defined(CFG_SLT_SUPPORT) || defined(CFG_TESTSUITE_SUPPORT)
#define I2C0_SLT_TEST_SLAVE_ADD 0x50
#define I2C0_SLT_TEST_DATA_ADD 0x03
#define I2C0_SLT_TEST_PATTERN 0x00

int32_t mt_i2c_slt_i2c0(void) {
    int32_t ret = 0;
    struct MtI2cDev *i2c = &mMtI2cDevs[0];
    volatile uint32_t timeout;
    uint16_t intr_stat, data, control_reg;

    i2c_clock_enable(i2c);

    i2c->addr = I2C0_SLT_TEST_SLAVE_ADD;
    control_reg = I2C_CONTROL_ACKERR_DET_EN | I2C_CONTROL_CLK_EXT_EN
        | I2C_CONTROL_DIR_CHANGE | I2C_CONTROL_RS;

    i2c_writew(0x1ff, i2c, OFFSET_INTR_STAT);
    i2c_writew((i2c->addr << 1), i2c, OFFSET_SLAVE_ADDR);
    i2c_writew(control_reg, i2c, OFFSET_CONTROL);
    i2c_writew(0x1800, i2c, OFFSET_EXT_CONF);
    i2c_writew(0x02, i2c, OFFSET_TRANSAC_LEN);
    i2c_writew(0x01, i2c, OFFSET_TRANSFER_LEN);
    i2c_writew(0x01, i2c, OFFSET_TRANSFER_LEN_AUX);
    i2c_writew(0x08, i2c, OFFSET_TIMING);
    i2c_writew(0x00, i2c, OFFSET_IO_CONFIG);
    i2c_writew(0x01, i2c, OFFSET_FIFO_ADDR_CLR);
    i2c_writew(I2C0_SLT_TEST_DATA_ADD, i2c, OFFSET_DATA_PORT);
    I2C_MB();
    i2c_writew(0x01, i2c, OFFSET_START);

    timeout = 0xffffff;
    while (i2c_readw(i2c, OFFSET_START)) {
        timeout--;
        if (timeout == 0) {
            i2c_dump_info(i2c);
            ret = -1;
            goto out;
        }
    };

    intr_stat = i2c_readw(i2c, OFFSET_INTR_STAT);
    i2c_writew(0x1ff, i2c, OFFSET_INTR_STAT);

    if (intr_stat & (I2C_HS_NACKERR | I2C_ACKERR)) {
        I2CERR("i2c%ld: addr = 0x%x, ACK error = 0x%x\n", i2c->id, i2c->addr, intr_stat);
        i2c_dump_info(i2c);
        ret = -1;
        goto out;
    }

    data = i2c_readw(i2c, OFFSET_DATA_PORT);
    I2CERR("i2c%ld: addr = 0x%x, data = 0x%x\n", i2c->id, i2c->addr, data);

    if (data != I2C0_SLT_TEST_PATTERN) {
        I2CERR("i2c%ld: data compare error\n", i2c->id);
        ret = -1;
    }

out:
    i2c_clock_disable(i2c);

    return ret;
}

#define I2C1_SLT_TEST_SLAVE_ADD 0x4b
#define I2C1_SLT_TEST_DATA_ADD 0x20
#define I2C1_SLT_TEST_PATTERN 0xa5
int32_t mt_i2c_slt_i2c1(void) {
    int32_t ret = 0;
    struct MtI2cDev *i2c = &mMtI2cDevs[1];
    volatile uint32_t timeout;
    uint16_t intr_stat, data, control_reg;

    /* Read after Write from RT9465 (0x4b) */

    i2c_clock_enable(i2c);

    i2c->addr = I2C1_SLT_TEST_SLAVE_ADD;
    control_reg = I2C_CONTROL_ACKERR_DET_EN | I2C_CONTROL_CLK_EXT_EN
        | I2C_CONTROL_DIR_CHANGE | I2C_CONTROL_RS;

    i2c_writew(0x1ff, i2c, OFFSET_INTR_STAT);
    i2c_writew((i2c->addr << 1), i2c, OFFSET_SLAVE_ADDR);
    i2c_writew(control_reg, i2c, OFFSET_CONTROL);
    i2c_writew(0x1800, i2c, OFFSET_EXT_CONF);
    i2c_writew(0x02, i2c, OFFSET_TRANSAC_LEN);
    i2c_writew(0x01, i2c, OFFSET_TRANSFER_LEN);
    i2c_writew(0x01, i2c, OFFSET_TRANSFER_LEN_AUX);
    i2c_writew(0x08, i2c, OFFSET_TIMING);
    i2c_writew(0x00, i2c, OFFSET_IO_CONFIG);
    i2c_writew(0x01, i2c, OFFSET_FIFO_ADDR_CLR);
    i2c_writew(I2C1_SLT_TEST_DATA_ADD, i2c, OFFSET_DATA_PORT);
    I2C_MB();
    i2c_writew(0x01, i2c, OFFSET_START);

    timeout = 0xffffff;
    while (i2c_readw(i2c, OFFSET_START)) {
        timeout--;
        if (timeout == 0) {
            i2c_dump_info(i2c);
            ret = -1;
            goto out;
        }
    };

    intr_stat = i2c_readw(i2c, OFFSET_INTR_STAT);
    i2c_writew(0x1ff, i2c, OFFSET_INTR_STAT);

    if (intr_stat & (I2C_HS_NACKERR | I2C_ACKERR)) {
        I2CERR("i2c%ld: addr = 0x%x, ACK error = 0x%x\n", i2c->id, i2c->addr, intr_stat);
        i2c_dump_info(i2c);
        ret = -1;
        goto out;
    }

    data = i2c_readw(i2c, OFFSET_DATA_PORT);
    I2CERR("i2c%ld: addr = 0x%x, data = 0x%x\n", i2c->id, i2c->addr, data);

#ifdef SLT_CHECK_DATA
    if (data != I2C1_SLT_TEST_PATTERN) {
        I2CERR("i2c%ld:  data compare error\n", i2c->id);
        ret = -1;
    }
#endif

out:
    i2c_clock_disable(i2c);

    return ret;
}

int32_t mt_i2c_slt(void) {
    int32_t ret0, ret1, ret;

    ret0 = mt_i2c_slt_i2c0();
    if (ret0)
        I2CERR("mt_i2c_slt_i2c0 fail!\r\n");

    ret1 = mt_i2c_slt_i2c1();
    if (ret1)
        I2CERR("mt_i2c_slt_i2c1 fail!\r\n");

    if (ret0 || ret1)
        ret = -1;
    else
        ret = 0;

    return ret;
}
#endif
