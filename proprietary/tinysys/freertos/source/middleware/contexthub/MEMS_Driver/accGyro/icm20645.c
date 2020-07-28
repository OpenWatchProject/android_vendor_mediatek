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
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <stdbool.h>
#include <stdint.h>
#include <seos.h>
#include <util.h>
#include <sensors.h>
#include <plat/inc/rtc.h>

#include <accGyro.h>
#include <contexthub_core.h>
#include <cust_accGyro.h>
#include <mt_gpt.h>
#define I2C_SPEED                       400000

#define REG_BANK_SEL                    0x7F
#define BANK_SEL_0                      0x00
#define BANK_SEL_1                      0x10
#define BANK_SEL_2                      0x20
#define BANK_SEL_3                      0x30
#define ICM20645_REG_LP_CONFIG          0x05
#define ICM20645_PWR_MGMT_1             0x06
#define ICM20645_PWR_MGMT_2             0x07
#define ICM20645_REG_FIFO_R_W           0x72
#define ICM20645_REG_FIFO_COUNTH        0x70
#define ICM20645_REG_FIFO_COUNTL        0x71
#define ICM20645_REG_FIFO_EN_2          0x67
#define ICM20645_REG_FIFO_RST           0x68
#define ICM20645_REG_USER_CTRL          0x03
#define ICM20645_REG_INT_ENABLE         0x11

#define BIT_GYRO_LP_EN                  0x10
#define BIT_ACC_LP_EN                   0x20
#define BIT_ACC_I2C_MST                 0x40

#define BIT_LP_EN                       0x20
#define BIT_CLK_PLL                     0x01
#define BIT_TEMP_DIS                    (1<<3)

#define BIT_PWR_ACCEL_STBY              0x38
#define BIT_PWR_GYRO_STBY               0x07

#define ICM20645_TIMEBASE_PLL           0x28

/* gyro */
#define ICM20645_REG_SAMRT_DIV          0x00
#define ICM20645_REG_GYRO_CONFIG        0x01
#define ICM20645_REG_GYRO_CONFIG_2      0x02

#define SHIFT_GYRO_FS_SEL                  1
#define GYRO_FS_SEL_250                     (0x00 << SHIFT_GYRO_FS_SEL)
#define GYRO_FS_SEL_500                     (0x01 << SHIFT_GYRO_FS_SEL)
#define GYRO_FS_SEL_1000                    (0x02 << SHIFT_GYRO_FS_SEL)
#define GYRO_FS_SEL_2000                    (0x03 << SHIFT_GYRO_FS_SEL)

#define ICM20645_FS_250_LSB             131.0f
#define ICM20645_FS_500_LSB             65.5f
#define ICM20645_FS_1000_LSB            32.8f
#define ICM20645_FS_2000_LSB            16.4f

#define GYRO_AVGCFG_1X                     0
#define GYRO_AVGCFG_2X                     1
#define GYRO_AVGCFG_4X                     2
#define GYRO_AVGCFG_8X                     3
#define GYRO_AVGCFG_16X                    4
#define GYRO_AVGCFG_32X                    5
#define GYRO_AVGCFG_64X                    6
#define GYRO_AVGCFG_128X                   7
#define GYRO_DLPFCFG    (7<<3)
#define GYRO_FCHOICE    0x01
/* gyro */

/* acc */
#define ICM20645_REG_SAMRT_DIV1         0x10
#define ICM20645_REG_SAMRT_DIV2         0x11
#define ICM20645_REG_ACC_CONFIG         0x14
#define ICM20645_REG_ACC_CONFIG_2       0x15

#define ACCEL_FCHOICE                      1
#define ACCEL_DLPFCFG                   (7<<3)
#define ICM20645_RANGE_2G               (0x00 << 1)
#define ICM20645_RANGE_4G               (0x01 << 1)
#define ICM20645_RANGE_8G               (0x02 << 1)
#define ICM20645_RANGE_16G              (0x03 << 1)

#define ACCEL_AVGCFG_1_4X                  0
#define ACCEL_AVGCFG_8X                    1
#define ACCEL_AVGCFG_16X                   2
#define ACCEL_AVGCFG_32X                   3
/* acc */

#define ACCEL_AVG_SAMPLE                (0x3)


#define ICM20645_REG_DATAX0             0x2d
#define ICM20645_REG_DATAY0             0x2f
#define ICM20645_REG_DATAZ0             0x31


#define ICM20645_SLEEP                  0x40    //enable low power sleep mode
#define ICM20645_DEV_RESET              0x80


/* ICM20645 Register Map  (Please refer to ICM20645 Specifications) */
#define ICM20645_REG_DEVID              0x00
#define remapFifoDelay(delay_ns, pll)     ((delay_ns) * (1270ULL)/(1270ULL + pll))

enum ICM20645State {
    STATE_SAMPLE = CHIP_SAMPLING,
    STATE_FIFO = CHIP_FIFO,
    STATE_CONVERT = CHIP_CONVERT,
    STATE_SAMPLE_DONE = CHIP_SAMPLING_DONE,
    STATE_ACC_ENABLE = CHIP_ACC_ENABLE,
    STATE_ACC_ENABLE_DONE = CHIP_ACC_ENABLE_DONE,
    STATE_ACC_DISABLE = CHIP_ACC_DISABLE,
    STATE_ACC_DISABLE_DONE = CHIP_ACC_DISABLE_DONE,
    STATE_ACC_RATECHG = CHIP_ACC_RATECHG,
    STATE_ACC_RATECHG_DONE = CHIP_ACC_RATECHG_DONE,
    STATE_GYRO_ENABLE = CHIP_GYRO_ENABLE,
    STATE_GYRO_ENABLE_DONE = CHIP_GYRO_ENABLE_DONE,
    STATE_GYRO_DISABLE = CHIP_GYRO_DISABLE,
    STATE_GYRO_DISABLE_DONE = CHIP_GYRO_DISABLE_DONE,
    STATE_GYRO_RATECHG = CHIP_GYRO_RATECHG,
    STATE_GYRO_RATECHG_DONE = CHIP_GYRO_RATECHG_DONE,
    STATE_INIT_DONE = CHIP_INIT_DONE,
    STATE_IDLE = CHIP_IDLE,
    STATE_RESET_R = CHIP_RESET,
    STATE_RESET_W,
    STATE_DEVID,
    STATE_SELCLK_R,
    STATE_SELCLK_W,
    STATE_POWER_R,
    STATE_DEPOWER_W,
    STATE_ENPOWER_W,
    STATE_DUTY_CYCLE,
    STATE_TURN_R,
    STATE_ACC_TURNON_W,
    STATE_ACC_TURNOFF_W,
    STATE_GYRO_TURNON_W,
    STATE_GYRO_TURNOFF_W,
    STATE_ACC_CONFIG1,
    STATE_ACC_CONFIG2,
    STATE_GYRO_CONFIG1,
    STATE_GYRO_CONFIG2,
    STATE_DATA_RESO,
    STATE_CALC_RESO,
    STATE_LP_R,
    STATE_ENLP_W,
    STATE_DELP_W,
    STATE_FIFO_RST,
    STATE_FIFOREG2_R,
    STATE_ACC_ENFIFO_W,
    STATE_ACC_DEFIFO_W,
    STATE_GYRO_ENFIFO_W,
    STATE_GYRO_DEFIFO_W,
    STATE_ENFIFO_W,
    STATE_DEFIFO_W,
    STATE_TIMEBASE,
    STATE_CORE,
    STATE_ACC_RATE,
    STATE_GYRO_RATE,
};

struct scale_factor {
    unsigned char  whole;
    unsigned char  fraction;
};

struct acc_data_resolution {
    struct scale_factor scalefactor;
    int                 sensitivity;
};
struct gyro_data_resolution {
    float   sensitivity;
};

static struct acc_data_resolution icm20645g_data_resolution[] = {
    {{ 0,  6}, 16384},  /*+/-2g  in 16-bit resolution:  0.06 mg/LSB*/
    {{ 0, 12}, 8192},   /*+/-4g  in 16-bit resolution:  0.12 mg/LSB*/
    {{ 0, 24}, 4096},   /*+/-8g  in 16-bit resolution:  0.24 mg/LSB*/
    {{ 0, 5}, 2048},    /*+/-16g in 16-bit resolution:  0.49 mg/LSB*/
};
static struct gyro_data_resolution icm20645gy_data_resolution[] = {
    {ICM20645_FS_250_LSB},
    {ICM20645_FS_500_LSB},
    {ICM20645_FS_1000_LSB},
    {ICM20645_FS_2000_LSB},
};
#define MAX_I2C_PER_PACKET  8
#define ICM20645_DATA_LEN   6
#define MAX_RXBUF 512
#define MAX_TXBUF (MAX_RXBUF / MAX_I2C_PER_PACKET)
static struct ICM20645Task {
    /* txBuf for i2c operation, fill register and fill value */
    bool accPowerOn;
    bool gyroPowerOn;
    bool accFifoConfiged;
    bool gyroFifoConfiged;
    uint8_t txBuf[MAX_TXBUF];
    uint8_t prvTxBuf[8];
    /* rxBuf for i2c operation, receive rawdata */
    uint8_t rxBuf[MAX_RXBUF];
    uint8_t deviceId;
    int8_t timeBase;
    uint8_t fifoLenHigh;
    uint8_t fifoLenLow;
    uint8_t waterMark;
    uint16_t fifoLen;
    uint32_t minAccGyroDelay;
    uint32_t maxAccGyroRate;
    uint32_t accRate;
    uint32_t gyroRate;
    uint16_t accRateDiv;
    uint16_t gyroRateDiv;
    uint64_t sampleTime;
    struct transferDataInfo dataInfo;
    struct accGyroDataPacket accGyroPacket;
    /* data for factory */
    struct TripleAxisDataPoint accFactoryData;
    struct TripleAxisDataPoint gyroFactoryData;

    int32_t accSwCali[AXES_NUM];
    int32_t gyroSwCali[AXES_NUM];
    struct acc_data_resolution *accReso;
    struct gyro_data_resolution *gyroReso;
    struct accGyro_hw *hw;
    struct sensorDriverConvert cvt;
    uint8_t i2c_addr;
} mTask;
static struct ICM20645Task *icm20645DebugPoint;
static void icm20645SelectBank(I2cCallbackF i2cCallBack, void *next_state, uint8_t bank)
{
    int ret = 0;
    //osLog(LOG_INFO, "icm20645SelectBank: 0x%x\n", bank);
    if (bank == BANK_SEL_0) {
        mTask.prvTxBuf[0] = REG_BANK_SEL;
        mTask.prvTxBuf[1] = bank;
        ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.prvTxBuf, 2, i2cCallBack,
                           next_state);
        if (ret != 0)
            osLog(LOG_INFO, "REG_BANK_SEL err\n");
    } else if (bank == BANK_SEL_1) {
        mTask.prvTxBuf[2] = REG_BANK_SEL;
        mTask.prvTxBuf[3] = bank;
        ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.prvTxBuf[2], 2, i2cCallBack,
                           next_state);
        if (ret != 0)
            osLog(LOG_INFO, "REG_BANK_SEL err\n");
    } else if (bank == BANK_SEL_2) {
        mTask.prvTxBuf[4] = REG_BANK_SEL;
        mTask.prvTxBuf[5] = bank;
        ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.prvTxBuf[4], 2, i2cCallBack,
                           next_state);
        if (ret != 0)
            osLog(LOG_INFO, "REG_BANK_SEL err\n");
    }
}

static int icm20645ResetRead(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20645ResetRead\n");
    icm20645SelectBank(i2cCallBack, (void *)STATE_IDLE, BANK_SEL_0);

    mTask.txBuf[0] = ICM20645_PWR_MGMT_1;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,
                         next_state);
}

static int icm20645ResetWrite(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "read 0x%x before icm20645ResetWrite\n", mTask.rxBuf[0]);

    mTask.txBuf[0] = ICM20645_PWR_MGMT_1;
    mTask.txBuf[1] = mTask.rxBuf[0] | ICM20645_DEV_RESET;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}

static int icm20645DeviceId(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    /* WE MUST SLEEP 10MS TO WAIT DEVICE RESET DONE */
    mdelay(10);
    //osLog(LOG_INFO, "icm20645DeviceId\n");

    mTask.txBuf[0] = ICM20645_REG_DEVID;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         &mTask.deviceId, 1, i2cCallBack,
                         next_state);
}

static int icm20645SelClkRead(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20645SelClkRead\n");

    if (mTask.deviceId != 0xA7) {
        osLog(LOG_INFO, "icm20645g_CheckDeviceID ERR 0x%x\n\r", mTask.deviceId);
        return 0;
    }
    mTask.txBuf[0] = ICM20645_PWR_MGMT_1;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,
                         next_state);
}

static int icm20645SelClkWrite(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20645SelClkWrite\n");

    mTask.txBuf[0] = ICM20645_PWR_MGMT_1;
    mTask.txBuf[1] = mTask.rxBuf[0] | (BIT_CLK_PLL | BIT_TEMP_DIS);
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}
static int icm20645PowerRead(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20645PowerRead\n");
    mTask.txBuf[0] = ICM20645_PWR_MGMT_1;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,
                         next_state);
}
static int icm20645PowerEnableWrite(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20645PowerEnableWrite: 0x%x\n", mTask.rxBuf[0]);
    mTask.txBuf[0] = ICM20645_PWR_MGMT_1;
    mTask.txBuf[1] = mTask.rxBuf[0] & ~ICM20645_SLEEP;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}
static int icm20645PowerDisableWrite(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20645PowerDisableWrite: 0x%x\n", mTask.rxBuf[0]);
    if (mTask.accPowerOn || mTask.gyroPowerOn) {
        osLog(LOG_INFO, "icm20645PowerDisableWrite should not disable, acc:%d, gyro:%d\n",
            mTask.accPowerOn, mTask.gyroPowerOn);
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    } else {
        mTask.txBuf[0] = ICM20645_PWR_MGMT_1;
        mTask.txBuf[1] = mTask.rxBuf[0] | ICM20645_SLEEP;
        return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                           next_state);
    }
    return 0;
}
static int icm20645DutyCycle(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20645DutyCycle\n");
    mTask.txBuf[0] = ICM20645_REG_LP_CONFIG;
    mTask.txBuf[1] = BIT_GYRO_LP_EN | BIT_ACC_LP_EN;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}
static int icm20645TurnRead(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20645TurnRead\n");
    mTask.txBuf[0] = ICM20645_PWR_MGMT_2;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,
                         next_state);
}
static int icm20645gTurnOnWrite(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20645gTurnOnWrite: 0x%x\n", mTask.rxBuf[0]);
    mTask.txBuf[0] = ICM20645_PWR_MGMT_2;
    mTask.txBuf[1] = mTask.rxBuf[0] & ~BIT_PWR_ACCEL_STBY;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}
static int icm20645gTurnOffWrite(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20645gTurnOffWrite: 0x%x\n", mTask.rxBuf[0]);
    mTask.txBuf[0] = ICM20645_PWR_MGMT_2;
    mTask.txBuf[1] = mTask.rxBuf[0] | BIT_PWR_ACCEL_STBY;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}

static int icm20645gyTurnOnWrite(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20645gyTurnOnWrite: 0x%x\n", mTask.rxBuf[0]);
    mTask.txBuf[0] = ICM20645_PWR_MGMT_2;
    mTask.txBuf[1] = mTask.rxBuf[0] & ~BIT_PWR_GYRO_STBY;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}

static int icm20645gyTurnOffWrite(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20645gyTurnOffWrite: 0x%x\n", mTask.rxBuf[0]);
    mTask.txBuf[0] = ICM20645_PWR_MGMT_2;
    mTask.txBuf[1] = mTask.rxBuf[0] | BIT_PWR_GYRO_STBY;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}
static int icm20645AccConfig1(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20645AccConfig1\n");
    icm20645SelectBank(i2cCallBack, (void *)STATE_IDLE, BANK_SEL_2);
    mTask.txBuf[0] = ICM20645_REG_ACC_CONFIG;
    mTask.txBuf[1] = (ICM20645_RANGE_4G | ACCEL_DLPFCFG | ACCEL_FCHOICE);
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}
static int icm20645AccConfig2(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;

    //osLog(LOG_INFO, "icm20645AccConfig2\n");
    mTask.txBuf[0] = ICM20645_REG_ACC_CONFIG_2;
    mTask.txBuf[1] = ACCEL_AVGCFG_1_4X;
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
    return ret;
}

static int icm20645GyroConfig1(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;

    //osLog(LOG_INFO, "icm20645GyroConfig1\n");
    mTask.txBuf[0] = ICM20645_REG_GYRO_CONFIG;
    mTask.txBuf[1] = (GYRO_FS_SEL_2000 | GYRO_FCHOICE | GYRO_DLPFCFG);
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
    return ret;
}

static int icm20645GyroConfig2(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;

    //osLog(LOG_INFO, "icm20645GyroConfig2\n");
    mTask.txBuf[0] = ICM20645_REG_GYRO_CONFIG_2;
    mTask.txBuf[1] = GYRO_AVGCFG_2X;
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
    return ret;
}

static int icm20645DataReso(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;

    //osLog(LOG_INFO, "icm20645DataReso\n");
    mTask.txBuf[0] = ICM20645_REG_ACC_CONFIG;
    ret = i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         &mTask.rxBuf[0], 1, i2cCallBack,
                         (void *)STATE_IDLE);
    mTask.txBuf[1] = ICM20645_REG_GYRO_CONFIG;
    ret = i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[1], 1,
                         &mTask.rxBuf[1], 1, i2cCallBack,
                         next_state);
    return ret;
}

static int icm20645CalcReso(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    uint8_t reso = 0;
    icm20645SelectBank(i2cCallBack, (void *)STATE_IDLE, BANK_SEL_0);
    //osLog(LOG_INFO, "icm20645CalcReso:acc(0x%x), gyro(0x%x)\n", mTask.rxBuf[0], mTask.rxBuf[1]);
    reso = 0x00;
    reso = (mTask.rxBuf[0] & ICM20645_RANGE_16G) >> 1;
    if (reso < sizeof(icm20645g_data_resolution) / sizeof(icm20645g_data_resolution[0])) {
        mTask.accReso = &icm20645g_data_resolution[reso];
        osLog(LOG_INFO, "acc reso: %d, sensitivity: %d\n", reso, mTask.accReso->sensitivity);
    }
    reso = (mTask.rxBuf[1] & GYRO_FS_SEL_2000) >> 1;
    if (reso < sizeof(icm20645gy_data_resolution) / sizeof(icm20645gy_data_resolution[0])) {
        mTask.gyroReso = &icm20645gy_data_resolution[reso];
        osLog(LOG_INFO, "gyro reso: %d, sensitivity: %f\n", reso, (double)mTask.gyroReso->sensitivity);
    }
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int icm20645ReadTimeBase(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    icm20645SelectBank(i2cCallBack, (void *)STATE_IDLE, BANK_SEL_1);
    mTask.txBuf[0] = ICM20645_TIMEBASE_PLL;
    i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         &mTask.timeBase, 1, i2cCallBack,
                         (void *)STATE_IDLE);
    icm20645SelectBank(i2cCallBack, next_state, BANK_SEL_0);
    return 0;
}
static int icm20645LpRead(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20645LpRead\n");
    return icm20645PowerRead(i2cCallBack, spiCallBack, next_state,
        inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
}

static int icm20645EnableLpWrite(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20645EnableLpWrite:0x%x\n", mTask.rxBuf[0]);
    if ((mTask.gyroPowerOn == true && mTask.accPowerOn == true) ||
            (mTask.gyroPowerOn == true && mTask.accPowerOn == false)) {
        mTask.txBuf[0] = ICM20645_PWR_MGMT_1;
        mTask.txBuf[1] = mTask.rxBuf[0] & ~BIT_LP_EN;
    } else if (mTask.gyroPowerOn == false && mTask.accPowerOn == true) {
        mTask.txBuf[0] = ICM20645_PWR_MGMT_1;
        mTask.txBuf[1] = mTask.rxBuf[0] | BIT_LP_EN;
    } else if (mTask.gyroPowerOn == false && mTask.accPowerOn == false) {
        mTask.txBuf[0] = ICM20645_PWR_MGMT_1;
        mTask.txBuf[1] = mTask.rxBuf[0] | BIT_LP_EN;
    }
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}

static int icm20645DisableLpWrite(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20645DisableLpWrite:0x%x\n", mTask.rxBuf[0]);
    mTask.txBuf[0] = ICM20645_PWR_MGMT_1;
    mTask.txBuf[1] = mTask.rxBuf[0] & ~BIT_LP_EN;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}

static int icm20645gEnable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20645gEnable\n");

    mTask.accPowerOn = true;
    return icm20645PowerRead(i2cCallBack, spiCallBack, next_state,
        inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
}
static int icm20645gDisable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    struct accGyroCntlPacket cntlPacket;

    ret = rxControlInfo(&cntlPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_INFO, "icm20645gDisable, rx water_mark err\n");
        return -1;
    }
    osLog(LOG_INFO, "icm20645gDisable: water_mark:%d\n", cntlPacket.waterMark);
    mTask.accPowerOn = false;
    mTask.accRate = 0;
    if (mTask.gyroPowerOn == true)
        mTask.maxAccGyroRate = (mTask.accRate > mTask.gyroRate ? mTask.accRate : mTask.gyroRate);
    else
        mTask.maxAccGyroRate = mTask.accRate;
    mTask.minAccGyroDelay = (mTask.maxAccGyroRate == 0) ? 0 : 1000000000000ULL / mTask.maxAccGyroRate;
    return icm20645TurnRead(i2cCallBack, spiCallBack, next_state,
        inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
}
static int icm20645gRate(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    uint32_t sample_rate, water_mark;
    struct accGyroCntlPacket cntlPacket;

    ret = rxControlInfo(&cntlPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "icm20645gRate, rx inSize and elemSize error\n");
        return -1;
    }
    sample_rate = cntlPacket.rate;
    water_mark = cntlPacket.waterMark;

    if (sample_rate > 0 && sample_rate < 6000) {
        mTask.accRate = 5000;
        mTask.accRateDiv = 224;
    } else if (sample_rate > 9000 && sample_rate < 11000) {
        mTask.accRate = 10000;
        mTask.accRateDiv = 111;
    } else if (sample_rate > 16000 && sample_rate < 17000) {
        mTask.accRate = 15000;
        mTask.accRateDiv = 74;
    } else if (sample_rate > 40000 && sample_rate < 60000) {
        mTask.accRate = 51100;
        mTask.accRateDiv = 21;
    } else if (sample_rate > 90000 && sample_rate < 110000) {
        mTask.accRate = 102300;
        mTask.accRateDiv = 10;
    } else if (sample_rate > 190000 && sample_rate < 210000) {
        mTask.accRate = 225000;
        mTask.accRateDiv = 4;
    } else {
        mTask.accRate = sample_rate;
        mTask.accRateDiv = 1125000 / sample_rate - 1;
    }
    if (mTask.gyroPowerOn == true) {
        mTask.maxAccGyroRate = (mTask.accRate > mTask.gyroRate ? mTask.accRate : mTask.gyroRate);
    } else {
        mTask.maxAccGyroRate = mTask.accRate;
    }
    mTask.waterMark = water_mark;
    mTask.minAccGyroDelay = 1000000000000ULL / mTask.maxAccGyroRate;
    osLog(LOG_INFO, "icm20645gRate: %d, minAccGyroDelay:%d, water_mark:%d\n", mTask.accRate, mTask.minAccGyroDelay, water_mark);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return ret;
}
static int icm20645gSetRate(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;

    //osLog(LOG_INFO, "icm20645gSetRate\n");
    icm20645SelectBank(i2cCallBack, (void *)STATE_IDLE, BANK_SEL_2);
	/* hardware issue, fifo odr relay on gyro rate, if acc rate > gyro rate, we should 
	 * set gyro rate register, because set acc rate register fifo odr is also gyro rate,
	 * then acc odr will < the odr user needed.
	 */
    if (mTask.accRate > mTask.gyroRate && mTask.gyroPowerOn == true && mTask.accPowerOn == true) {
        /* set gyro register */
        mTask.txBuf[0] = ICM20645_REG_SAMRT_DIV;
        mTask.txBuf[1] = mTask.accRateDiv;
        ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                           (void *)STATE_IDLE);
        if (ret != 0)
            osLog(LOG_INFO, "ICM20645_REG_SAMRT_DIV err\n");

    } else if (mTask.accPowerOn == true) {
        /* set acc register */
        mTask.txBuf[0] = ICM20645_REG_SAMRT_DIV2;
        mTask.txBuf[1] = (uint8_t)(mTask.accRateDiv % 256);
        ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                           (void *)STATE_IDLE);
        if (ret != 0)
            osLog(LOG_INFO, "ICM20645_REG_SAMRT_DIV2 err\n");

        mTask.txBuf[2] = ICM20645_REG_SAMRT_DIV1;
        mTask.txBuf[3] = (uint8_t)(mTask.accRateDiv / 256);
        ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[2], 2, i2cCallBack,
                           (void *)STATE_IDLE);
        if (ret != 0)
            osLog(LOG_INFO, "ICM20645_REG_SAMRT_DIV1 err\n");
    }
    icm20645SelectBank(i2cCallBack, next_state, BANK_SEL_0);
    return ret;
}

static int icm20645FifoReset(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;

    //osLog(LOG_INFO, "icm20645FifoReset\n");

    mTask.txBuf[0] = ICM20645_REG_FIFO_RST;
    mTask.txBuf[1] = 0X1F;
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       (void *)STATE_IDLE);
    if (ret != 0)
        osLog(LOG_INFO, "ICM20645_REG_SAMRT_DIV2 err\n");

    mTask.txBuf[2] = ICM20645_REG_FIFO_RST;
    mTask.txBuf[3] = 0X1F - 1;
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[2], 2, i2cCallBack,
                       next_state);
    if (ret != 0)
        osLog(LOG_INFO, "ICM20645_REG_SAMRT_DIV1 err\n");
    return ret;
}
static int icm20645FifoReg2Read(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20645FifoReg2Read\n");

    mTask.txBuf[0] = ICM20645_REG_FIFO_EN_2;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,
                         next_state);
}

static int icm20645gFifoEnable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20645gFifoEnable: 0x%x\n", mTask.rxBuf[0]);
    mTask.accFifoConfiged = true;
    registerAccGyroFifoInfo(remapFifoDelay(mTask.minAccGyroDelay, mTask.timeBase),
        remapFifoDelay(mTask.minAccGyroDelay, mTask.timeBase));
    mTask.txBuf[0] = ICM20645_REG_FIFO_EN_2;
    mTask.txBuf[1] = mTask.rxBuf[0] | (0b00010000);
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}

static int icm20645gFifoDisable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20645gFifoEnable: 0x%x\n", mTask.rxBuf[0]);
    mTask.accFifoConfiged = false;
    registerAccGyroFifoInfo(remapFifoDelay(mTask.minAccGyroDelay, mTask.timeBase),
        remapFifoDelay(mTask.minAccGyroDelay, mTask.timeBase));
    mTask.txBuf[0] = ICM20645_REG_FIFO_EN_2;
    mTask.txBuf[1] = mTask.rxBuf[0] & ~(0b00010000);

    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}

static int icm20645gyFifoEnable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20645gyFifoEnable: 0x%x\n", mTask.rxBuf[0]);
    mTask.gyroFifoConfiged = true;
    registerAccGyroFifoInfo(remapFifoDelay(mTask.minAccGyroDelay, mTask.timeBase),
        remapFifoDelay(mTask.minAccGyroDelay, mTask.timeBase));
    mTask.txBuf[0] = ICM20645_REG_FIFO_EN_2;
    mTask.txBuf[1] = mTask.rxBuf[0] | (0b00001110);
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}

static int icm20645gyFifoDisable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20645gyFifoDisable: 0x%x\n", mTask.rxBuf[0]);
    mTask.gyroFifoConfiged = false;
    registerAccGyroFifoInfo(remapFifoDelay(mTask.minAccGyroDelay, mTask.timeBase),
        remapFifoDelay(mTask.minAccGyroDelay, mTask.timeBase));
    mTask.txBuf[0] = ICM20645_REG_FIFO_EN_2;
    mTask.txBuf[1] = mTask.rxBuf[0] & ~(0b00001110);
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}

static int icm20645FifoEnable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20645FifoEnable\n");
    mTask.txBuf[0] = ICM20645_REG_USER_CTRL;
    mTask.txBuf[1] = 1 << 6;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}

static int icm20645FifoDisable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20645FifoDisable\n");
    mTask.txBuf[0] = ICM20645_REG_USER_CTRL;
    mTask.txBuf[1] = 0;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}

static int icm20645gyEnable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20645gyEnable\n");
    mTask.gyroPowerOn = true;
    return icm20645PowerRead(i2cCallBack, spiCallBack, next_state,
        inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
}
static int icm20645gyDisable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    struct accGyroCntlPacket cntlPacket;

    ret = rxControlInfo(&cntlPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_INFO, "icm20645gyDisable, rx water_mark err\n");
        return -1;
    }
    osLog(LOG_INFO, "icm20645gyDisable: water_mark:%d\n", cntlPacket.waterMark);
    mTask.gyroPowerOn = false;
    mTask.gyroRate = 0;
    if (mTask.accPowerOn == true)
        mTask.maxAccGyroRate = (mTask.accRate > mTask.gyroRate ? mTask.accRate : mTask.gyroRate);
    else
        mTask.maxAccGyroRate = mTask.gyroRate;
    mTask.minAccGyroDelay = (mTask.maxAccGyroRate == 0) ? 0 : 1000000000000ULL / mTask.maxAccGyroRate;
    return icm20645TurnRead(i2cCallBack, spiCallBack, next_state,
        inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
}
static int icm20645gyRate(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    uint32_t sample_rate, water_mark;
    struct accGyroCntlPacket cntlPacket;

    ret = rxControlInfo(&cntlPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "icm20645gyRate, rx inSize and elemSize error\n");
        return -1;
    }
    sample_rate = cntlPacket.rate;
    water_mark = cntlPacket.waterMark;

    if (sample_rate > 0 && sample_rate < 6000) {
        mTask.gyroRate = 5000;
        mTask.gyroRateDiv = 224;
    } else if (sample_rate > 9000 && sample_rate < 11000) {
        mTask.gyroRate = 10000;
        mTask.gyroRateDiv = 111;
    } else if (sample_rate > 16000 && sample_rate < 17000) {
        mTask.gyroRate = 15000;
        mTask.gyroRateDiv = 74;
    } else if (sample_rate > 40000 && sample_rate < 60000) {
        mTask.gyroRate = 51100;
        mTask.gyroRateDiv = 21;
    } else if (sample_rate > 90000 && sample_rate < 110000) {
        mTask.gyroRate = 102300;
        mTask.gyroRateDiv = 10;
    } else if (sample_rate > 190000 && sample_rate < 210000) {
        mTask.gyroRate = 225000;
        mTask.gyroRateDiv = 4;
    } else {
        mTask.gyroRate = sample_rate;
        if (sample_rate <= 4000)
            mTask.gyroRateDiv = 255;
        else if (sample_rate >= 1125000)
            mTask.gyroRateDiv = 0;
        else
            mTask.gyroRateDiv = 1125000 / sample_rate - 1;
    }
    if (mTask.accPowerOn == true)
        mTask.maxAccGyroRate = (mTask.accRate > mTask.gyroRate ? mTask.accRate : mTask.gyroRate);
    else
        mTask.maxAccGyroRate = mTask.gyroRate;

    mTask.waterMark = water_mark;
    mTask.minAccGyroDelay = 1000000000000ULL / mTask.maxAccGyroRate;
    osLog(LOG_INFO, "icm20645gyRate: %d, minAccGyroDelay: %d, water_mark:%d\n", mTask.gyroRate, mTask.minAccGyroDelay, water_mark);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return ret;
}
static int icm20645gySetRate(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
	/* hardware issue, fifo odr relay on gyro rate, if acc rate > gyro rate, we should 
	 * set acc rate to gyro rate register, because set acc rate register fifo odr is also gyro rate,
	 * then acc odr will < the odr user needed.
	 */
    icm20645SelectBank(i2cCallBack, (void *)STATE_IDLE, BANK_SEL_2);
    if (mTask.accRate > mTask.gyroRate && mTask.accPowerOn == true && mTask.gyroPowerOn == true) {
        /* set acc rate to gyro register */
        mTask.txBuf[0] = ICM20645_REG_SAMRT_DIV;
        mTask.txBuf[1] = mTask.accRateDiv;
        ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                           (void *)STATE_IDLE);
        if (ret != 0)
            osLog(LOG_INFO, "ICM20645_REG_SAMRT_DIV err\n");
    } else if (mTask.gyroPowerOn == true) {
        /* set gyro rate to gyro register */
        mTask.txBuf[0] = ICM20645_REG_SAMRT_DIV;
        mTask.txBuf[1] = mTask.gyroRateDiv;
        ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                           (void *)STATE_IDLE);
        if (ret != 0)
            osLog(LOG_INFO, "ICM20645_REG_SAMRT_DIV err\n");
    }

    icm20645SelectBank(i2cCallBack, next_state, BANK_SEL_0);
    return ret;
}

static void accGetCalibration(int32_t *cali, int32_t size)
{
    cali[AXIS_X] = mTask.accSwCali[AXIS_X];
    cali[AXIS_Y] = mTask.accSwCali[AXIS_Y];
    cali[AXIS_Z] = mTask.accSwCali[AXIS_Z];
    osLog(LOG_INFO, "accGetCalibration cali x:%d, y:%d, z:%d\n", cali[AXIS_X], cali[AXIS_Y], cali[AXIS_Z]);
}
static void accSetCalibration(int32_t *cali, int32_t size)
{
    mTask.accSwCali[AXIS_X] = cali[AXIS_X];
    mTask.accSwCali[AXIS_Y] = cali[AXIS_Y];
    mTask.accSwCali[AXIS_Z] = cali[AXIS_Z];
    osLog(LOG_INFO, "accSetCalibration cali x:%d, y:%d, z:%d\n", mTask.accSwCali[AXIS_X],
        mTask.accSwCali[AXIS_Y], mTask.accSwCali[AXIS_Z]);
}
static void accGetData(void *sample)
{
    struct TripleAxisDataPoint *tripleSample = (struct TripleAxisDataPoint *)sample;
    tripleSample->ix = mTask.accFactoryData.ix;
    tripleSample->iy = mTask.accFactoryData.iy;
    tripleSample->iz = mTask.accFactoryData.iz;
}
static void gyroGetCalibration(int32_t *cali, int32_t size)
{
    cali[AXIS_X] = mTask.gyroSwCali[AXIS_X];
    cali[AXIS_Y] = mTask.gyroSwCali[AXIS_Y];
    cali[AXIS_Z] = mTask.gyroSwCali[AXIS_Z];
    osLog(LOG_INFO, "gyroGetCalibration cali x:%d, y:%d, z:%d\n", cali[AXIS_X], cali[AXIS_Y], cali[AXIS_Z]);
}
static void gyroSetCalibration(int32_t *cali, int32_t size)
{
    mTask.gyroSwCali[AXIS_X] = cali[AXIS_X];
    mTask.gyroSwCali[AXIS_Y] = cali[AXIS_Y];
    mTask.gyroSwCali[AXIS_Z] = cali[AXIS_Z];
    osLog(LOG_INFO, "gyroSetCalibration cali x:%d, y:%d, z:%d\n", mTask.gyroSwCali[AXIS_X],
        mTask.gyroSwCali[AXIS_Y], mTask.gyroSwCali[AXIS_Z]);
}
static void gyroGetData(void *sample)
{
    struct TripleAxisDataPoint *tripleSample = (struct TripleAxisDataPoint *)sample;
    tripleSample->ix = mTask.gyroFactoryData.ix;
    tripleSample->iy = mTask.gyroFactoryData.iy;
    tripleSample->iz = mTask.gyroFactoryData.iz;
}

static int icm20645RegisterCore(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    struct sensorCoreInfo mInfo;
    /* Register sensor Core */
    mInfo.sensType = SENS_TYPE_ACCEL;
    mInfo.gain = GRAVITY_EARTH_1000;
    mInfo.sensitivity = mTask.accReso->sensitivity;
    mInfo.cvt = mTask.cvt;
    mInfo.getCalibration = accGetCalibration;
    mInfo.setCalibration = accSetCalibration;
    mInfo.getData = accGetData;
    sensorCoreRegister(&mInfo);

    mInfo.sensType = SENS_TYPE_GYRO;
    mInfo.gain = GYROSCOPE_INCREASE_NUM_AP;
    mInfo.sensitivity = mTask.gyroReso->sensitivity;
    mInfo.cvt = mTask.cvt;
    mInfo.getCalibration = gyroGetCalibration;
    mInfo.setCalibration = gyroSetCalibration;
    mInfo.getData = gyroGetData;
    sensorCoreRegister(&mInfo);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int icm20645Sample(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;

    ret = rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "icm20645Sample, rx dataInfo error\n");
        return -1;
    }
    mTask.txBuf[0] = ICM20645_REG_FIFO_COUNTH;
    ret = i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[0], 1,
                         &mTask.fifoLenHigh, 1, i2cCallBack,
                         (void *)STATE_IDLE);
    if (ret != 0)
        osLog(LOG_INFO, "ICM20645_REG_FIFO_COUNTH err\n");
    mTask.txBuf[1] = ICM20645_REG_FIFO_COUNTL;
    ret = i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[1], 1,
                         &mTask.fifoLenLow, 1, i2cCallBack,
                         next_state);
    if (ret != 0)
        osLog(LOG_INFO, "ICM20645_REG_FIFO_COUNTL err\n");
    return 0;
}

static int icm20645ReadFifo(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    uint8_t N, n, i;
    static uint64_t lastSampleTime = 0;
    static uint16_t lastFifoLen = 0;

    mTask.sampleTime = rtcGetTime();
    mTask.fifoLen = (mTask.fifoLenHigh << 8) | mTask.fifoLenLow;
    /* IF  (mTask.fifoLen % 6) != 0, WE SHOULD NOT READ THE REMAIN DATA IN FIFO, WE ONLY READ (mTask.fifoLen / 6) * 6 BYTES*/
    mTask.fifoLen = (mTask.fifoLen / 6) * 6;
    //osLog(LOG_INFO, "icm20645ReadFifo count: %d\n", mTask.fifoLen);
    /* hw bug workaround, when we switch fifo mode, the fifo count is smaller than we expected, sometimes is 6 */
    if (mTask.waterMark != 0 && mTask.waterMark != 1 && mTask.fifoLen <= 6)
        mTask.fifoLen = 0;
    if (mTask.fifoLen <= 0) {
        lastSampleTime = mTask.sampleTime;
        lastFifoLen = mTask.fifoLen;
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
        return 0;
    }
    N = mTask.fifoLen / MAX_I2C_PER_PACKET;
    n = mTask.fifoLen % MAX_I2C_PER_PACKET;
    if (n == 0) {
        for (i = 0; i < N - 1; ++i) {
            mTask.txBuf[i] = ICM20645_REG_FIFO_R_W;
            ret = i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[i], 1,
                                 &mTask.rxBuf[i * MAX_I2C_PER_PACKET], MAX_I2C_PER_PACKET, i2cCallBack,
                                 (void *)STATE_IDLE);
            if (ret != 0)
                osLog(LOG_INFO, "icm20645ReadFifo err lastFifoLen:%d, fifolen:%d, lastSampleTime:%lld, sampleTime:%lld\n",
                    lastFifoLen, mTask.fifoLen, lastSampleTime, mTask.sampleTime);
        }
		/* WE SHOULD WAIT ALL READ RAW DATA I2C TRNASFER DONE THEN SWITCH NEXT_STATE AFTER THE LAST TRNASFER DONE,
		 * WE SHOULD NOT GIVE ALL I2C TRANSFER NEXT_STATE STATE_IDLE, THEN CALL sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
		 * THIS WILL LEAD ALL I2C HAVE NOT TRANSFER DONE THEN CONVERT DATA, THE DATA IS RANDOM
		 */
        mTask.txBuf[i] = ICM20645_REG_FIFO_R_W;
        ret = i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[i], 1,
                             &mTask.rxBuf[i * MAX_I2C_PER_PACKET], MAX_I2C_PER_PACKET, i2cCallBack,
                             next_state);
        if (ret != 0)
            osLog(LOG_INFO, "icm20645ReadFifo err lastFifoLen:%d, fifolen:%d, lastSampleTime:%lld, sampleTime:%lld\n",
                lastFifoLen, mTask.fifoLen, lastSampleTime, mTask.sampleTime);
    } else if (n != 0) {
        for (i = 0; i < N; ++i) {
            mTask.txBuf[i] = ICM20645_REG_FIFO_R_W;
            ret = i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[i], 1,
                                 &mTask.rxBuf[i * MAX_I2C_PER_PACKET], MAX_I2C_PER_PACKET, i2cCallBack,
                                 (void *)STATE_IDLE);
            if (ret != 0)
                osLog(LOG_INFO, "icm20645ReadFifo err lastFifoLen:%d, fifolen:%d, lastSampleTime:%lld, sampleTime:%lld\n",
                    lastFifoLen, mTask.fifoLen, lastSampleTime, mTask.sampleTime);
        }
		/* WE SHOULD WAIT ALL READ RAW DATA I2C TRNASFER DONE THEN SWITCH NEXT_STATE AFTER THE LAST TRNASFER DONE,
		 * WE SHOULD NOT GIVE ALL I2C TRANSFER NEXT_STATE STATE_IDLE, THEN CALL sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
		 * THIS WILL LEAD ALL I2C HAVE NOT TRANSFER DONE THEN CONVERT DATA, THE DATA IS RANDOM
		 */
        mTask.txBuf[N] = ICM20645_REG_FIFO_R_W;
        ret = i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[N], 1,
                             &mTask.rxBuf[N * MAX_I2C_PER_PACKET], n, i2cCallBack,
                             next_state);
        if (ret != 0)
            osLog(LOG_INFO, "icm20645ReadFifo err lastFifoLen:%d, fifolen:%d, lastSampleTime:%lld, sampleTime:%lld\n",
                lastFifoLen, mTask.fifoLen, lastSampleTime, mTask.sampleTime);
    }
    lastSampleTime = mTask.sampleTime;
    lastFifoLen = mTask.fifoLen;
    return ret;
}
static int icm20645gConvert(uint8_t *databuf, struct accGyroData *data)
{
    int32_t raw_data[AXES_NUM];
    int32_t remap_data[AXES_NUM];


    raw_data[AXIS_X] = (int16_t)((databuf[AXIS_X * 2] << 8) | databuf[AXIS_X * 2 + 1]);
    raw_data[AXIS_Y] = (int16_t)((databuf[AXIS_Y * 2] << 8) | databuf[AXIS_Y * 2 + 1]);
    raw_data[AXIS_Z] = (int16_t)((databuf[AXIS_Z * 2] << 8) | databuf[AXIS_Z * 2 + 1]);
	/*osLog(LOG_INFO, "acc rawdata x:%d, y:%d, z:%d\n", raw_data[AXIS_X],
		raw_data[AXIS_Y], raw_data[AXIS_Z]);*/
    raw_data[AXIS_X] = raw_data[AXIS_X] + mTask.accSwCali[AXIS_X];
    raw_data[AXIS_Y] = raw_data[AXIS_Y] + mTask.accSwCali[AXIS_Y];
    raw_data[AXIS_Z] = raw_data[AXIS_Z] + mTask.accSwCali[AXIS_Z];

    /*remap coordinate*/
    remap_data[mTask.cvt.map[AXIS_X]] = mTask.cvt.sign[AXIS_X] * raw_data[AXIS_X];
    remap_data[mTask.cvt.map[AXIS_Y]] = mTask.cvt.sign[AXIS_Y] * raw_data[AXIS_Y];
    remap_data[mTask.cvt.map[AXIS_Z]] = mTask.cvt.sign[AXIS_Z] * raw_data[AXIS_Z];

    /* Out put the degree/second(mo/s) */
    data->sensType = SENS_TYPE_ACCEL;
    data->x = (float)remap_data[AXIS_X] * GRAVITY_EARTH_SCALAR / mTask.accReso->sensitivity;
    data->y = (float)remap_data[AXIS_Y] * GRAVITY_EARTH_SCALAR / mTask.accReso->sensitivity;
    data->z = (float)remap_data[AXIS_Z] * GRAVITY_EARTH_SCALAR / mTask.accReso->sensitivity;
    mTask.accFactoryData.ix = (int32_t)(data->x * ACCELEROMETER_INCREASE_NUM_AP);
    mTask.accFactoryData.iy = (int32_t)(data->y * ACCELEROMETER_INCREASE_NUM_AP);
    mTask.accFactoryData.iz = (int32_t)(data->z * ACCELEROMETER_INCREASE_NUM_AP);
    //osLog(LOG_INFO, "acc rawdata x:%f, y:%f, z:%f\n", (double)data->x, (double)data->y, (double)data->z);
    return 0;
}
static int icm20645gyConvert(uint8_t *databuf, struct accGyroData *data)
{
    int32_t raw_data[AXES_NUM];
    int32_t remap_data[AXES_NUM];


    raw_data[AXIS_X] = (int16_t)((databuf[AXIS_X * 2] << 8) | databuf[AXIS_X * 2 + 1]);
    raw_data[AXIS_Y] = (int16_t)((databuf[AXIS_Y * 2] << 8) | databuf[AXIS_Y * 2 + 1]);
    raw_data[AXIS_Z] = (int16_t)((databuf[AXIS_Z * 2] << 8) | databuf[AXIS_Z * 2 + 1]);
	/*osLog(LOG_INFO, "gyro rawdata x:%d, y:%d, z:%d\n", raw_data[AXIS_X],
		raw_data[AXIS_Y], raw_data[AXIS_Z]);*/
    raw_data[AXIS_X] = raw_data[AXIS_X] + mTask.gyroSwCali[AXIS_X];
    raw_data[AXIS_Y] = raw_data[AXIS_Y] + mTask.gyroSwCali[AXIS_Y];
    raw_data[AXIS_Z] = raw_data[AXIS_Z] + mTask.gyroSwCali[AXIS_Z];

    /*remap coordinate*/
    remap_data[mTask.cvt.map[AXIS_X]] = mTask.cvt.sign[AXIS_X] * raw_data[AXIS_X];
    remap_data[mTask.cvt.map[AXIS_Y]] = mTask.cvt.sign[AXIS_Y] * raw_data[AXIS_Y];
    remap_data[mTask.cvt.map[AXIS_Z]] = mTask.cvt.sign[AXIS_Z] * raw_data[AXIS_Z];

    /* Out put the degree/second(mo/s) */
    data->sensType = SENS_TYPE_GYRO;
    data->x = (float)remap_data[AXIS_X] * DEGREE_TO_RADIRAN_SCALAR / mTask.gyroReso->sensitivity;
    data->y = (float)remap_data[AXIS_Y] * DEGREE_TO_RADIRAN_SCALAR / mTask.gyroReso->sensitivity;
    data->z = (float)remap_data[AXIS_Z] * DEGREE_TO_RADIRAN_SCALAR / mTask.gyroReso->sensitivity;
    mTask.gyroFactoryData.ix =
        (int32_t)((float)remap_data[AXIS_X] * GYROSCOPE_INCREASE_NUM_AP / mTask.gyroReso->sensitivity);
    mTask.gyroFactoryData.iy =
        (int32_t)((float)remap_data[AXIS_Y] * GYROSCOPE_INCREASE_NUM_AP / mTask.gyroReso->sensitivity);
    mTask.gyroFactoryData.iz =
        (int32_t)((float)remap_data[AXIS_Z] * GYROSCOPE_INCREASE_NUM_AP / mTask.gyroReso->sensitivity);
    //osLog(LOG_INFO, "gyro rawdata x:%f, y:%f, z:%f\n", (double)data->x, (double)data->y, (double)data->z);
    return 0;
}

static int icm20645Convert(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    struct accGyroData *data = mTask.accGyroPacket.outBuf;
    uint8_t accEventSize = 0;
    uint8_t gyroEventSize = 0;
    uint8_t N, i;
    uint8_t *databuf;

    //osLog(LOG_INFO, "acc on: %d, gyro on:%d\n", mTask.accPowerOn, mTask.gyroPowerOn);
    if (mTask.fifoLen <= 0) {
        txTransferDataInfo(&mTask.dataInfo, 0, 0, mTask.sampleTime, data);
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
        osLog(LOG_INFO, "icm20645Convert mTask.fifoLen:%u\n", mTask.fifoLen);
        return 0;
    }
    N = mTask.fifoLen / ICM20645_DATA_LEN;
    N = (N > MAX_RECV_PACKET) ? MAX_RECV_PACKET : N;
    for (i = 0; i < N; ++i) {
        databuf = &mTask.rxBuf[i * ICM20645_DATA_LEN];
        if (mTask.accFifoConfiged == true && mTask.gyroFifoConfiged == true) {
            if (i % 2 == 0) {
                accEventSize++;
                icm20645gConvert(databuf, &data[i]);
            } else {
                gyroEventSize++;
                icm20645gyConvert(databuf, &data[i]);
            }
        } else if (mTask.accFifoConfiged == false && mTask.gyroFifoConfiged == true) {
            gyroEventSize++;
            icm20645gyConvert(databuf, &data[i]);
        } else if (mTask.accFifoConfiged == true && mTask.gyroFifoConfiged == false) {
            accEventSize++;
            icm20645gConvert(databuf, &data[i]);
        } else
            osLog(LOG_INFO, "icm20645Convert accFifoConfiged: %d, gyroFifoConfiged:%d\n",
            mTask.accFifoConfiged, mTask.gyroFifoConfiged);
    }
    if ((accEventSize + gyroEventSize) != N)
        osLog(LOG_INFO, "icm20645Convert wrong num accEventSize:%d, gyroEventSize:%d\n", accEventSize, gyroEventSize);
    txTransferDataInfo(&mTask.dataInfo, accEventSize, gyroEventSize, mTask.sampleTime, data);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static struct sensorFsm icm20645Fsm[] = {
    sensorFsmCmd(STATE_SAMPLE, STATE_FIFO, icm20645Sample),
    sensorFsmCmd(STATE_FIFO, STATE_CONVERT, icm20645ReadFifo),
    sensorFsmCmd(STATE_CONVERT, STATE_SAMPLE_DONE, icm20645Convert),
    /* acc enable state */
    sensorFsmCmd(STATE_ACC_ENABLE, STATE_ENPOWER_W, icm20645gEnable),
    sensorFsmCmd(STATE_ENPOWER_W, STATE_TURN_R, icm20645PowerEnableWrite),
    sensorFsmCmd(STATE_TURN_R, STATE_ACC_TURNON_W, icm20645TurnRead),
    sensorFsmCmd(STATE_ACC_TURNON_W, STATE_LP_R, icm20645gTurnOnWrite),
    sensorFsmCmd(STATE_LP_R, STATE_DELP_W, icm20645LpRead),
    sensorFsmCmd(STATE_DELP_W, STATE_ACC_ENABLE_DONE, icm20645DisableLpWrite),
    /* acc disable state */
    sensorFsmCmd(STATE_ACC_DISABLE, STATE_ACC_TURNOFF_W, icm20645gDisable),
    sensorFsmCmd(STATE_ACC_TURNOFF_W, STATE_LP_R, icm20645gTurnOffWrite),
    sensorFsmCmd(STATE_LP_R, STATE_DELP_W, icm20645LpRead),
    sensorFsmCmd(STATE_DELP_W, STATE_DEFIFO_W, icm20645DisableLpWrite),
    sensorFsmCmd(STATE_DEFIFO_W, STATE_GYRO_RATE, icm20645FifoDisable),
    sensorFsmCmd(STATE_GYRO_RATE, STATE_FIFOREG2_R, icm20645gySetRate),
    sensorFsmCmd(STATE_FIFOREG2_R, STATE_ACC_DEFIFO_W, icm20645FifoReg2Read),
    sensorFsmCmd(STATE_ACC_DEFIFO_W, STATE_FIFO_RST, icm20645gFifoDisable),
    sensorFsmCmd(STATE_FIFO_RST, STATE_ENFIFO_W, icm20645FifoReset),
    sensorFsmCmd(STATE_ENFIFO_W, STATE_POWER_R, icm20645FifoEnable),
    sensorFsmCmd(STATE_POWER_R, STATE_DEPOWER_W, icm20645PowerRead),
    sensorFsmCmd(STATE_DEPOWER_W, STATE_LP_R, icm20645PowerDisableWrite),
    sensorFsmCmd(STATE_LP_R, STATE_ENLP_W, icm20645LpRead),
    sensorFsmCmd(STATE_ENLP_W, STATE_ACC_DISABLE_DONE, icm20645EnableLpWrite),
    /* acc rate state */
    sensorFsmCmd(STATE_ACC_RATECHG, STATE_LP_R, icm20645gRate),
    sensorFsmCmd(STATE_LP_R, STATE_DELP_W, icm20645LpRead),
    sensorFsmCmd(STATE_DELP_W, STATE_DEFIFO_W, icm20645DisableLpWrite),
    sensorFsmCmd(STATE_DEFIFO_W, STATE_ACC_RATE, icm20645FifoDisable),
    sensorFsmCmd(STATE_ACC_RATE, STATE_FIFO_RST, icm20645gSetRate),
    sensorFsmCmd(STATE_FIFO_RST, STATE_FIFOREG2_R, icm20645FifoReset),
    sensorFsmCmd(STATE_FIFOREG2_R, STATE_ACC_ENFIFO_W, icm20645FifoReg2Read),
    sensorFsmCmd(STATE_ACC_ENFIFO_W, STATE_ENFIFO_W, icm20645gFifoEnable),
    sensorFsmCmd(STATE_ENFIFO_W, STATE_LP_R, icm20645FifoEnable),
    sensorFsmCmd(STATE_LP_R, STATE_ENLP_W, icm20645LpRead),
    sensorFsmCmd(STATE_ENLP_W, STATE_ACC_RATECHG_DONE, icm20645EnableLpWrite),
    /* gyro enable state */
    sensorFsmCmd(STATE_GYRO_ENABLE, STATE_ENPOWER_W, icm20645gyEnable),
    sensorFsmCmd(STATE_ENPOWER_W, STATE_TURN_R, icm20645PowerEnableWrite),
    sensorFsmCmd(STATE_TURN_R, STATE_GYRO_TURNON_W, icm20645TurnRead),
    sensorFsmCmd(STATE_GYRO_TURNON_W, STATE_LP_R, icm20645gyTurnOnWrite),
    sensorFsmCmd(STATE_LP_R, STATE_DELP_W, icm20645LpRead),
    sensorFsmCmd(STATE_DELP_W, STATE_GYRO_ENABLE_DONE, icm20645DisableLpWrite),
    /* gyro disable state */
    sensorFsmCmd(STATE_GYRO_DISABLE, STATE_GYRO_TURNOFF_W, icm20645gyDisable),
    sensorFsmCmd(STATE_GYRO_TURNOFF_W, STATE_LP_R, icm20645gyTurnOffWrite),
    sensorFsmCmd(STATE_LP_R, STATE_DELP_W, icm20645LpRead),
    sensorFsmCmd(STATE_DELP_W, STATE_DEFIFO_W, icm20645DisableLpWrite),
    sensorFsmCmd(STATE_DEFIFO_W, STATE_ACC_RATE, icm20645FifoDisable),
    sensorFsmCmd(STATE_ACC_RATE, STATE_FIFOREG2_R, icm20645gSetRate),
    sensorFsmCmd(STATE_FIFOREG2_R, STATE_GYRO_DEFIFO_W, icm20645FifoReg2Read),
    sensorFsmCmd(STATE_GYRO_DEFIFO_W, STATE_FIFO_RST, icm20645gyFifoDisable),
    sensorFsmCmd(STATE_FIFO_RST, STATE_ENFIFO_W, icm20645FifoReset),
    sensorFsmCmd(STATE_ENFIFO_W, STATE_POWER_R, icm20645FifoEnable),
    sensorFsmCmd(STATE_POWER_R, STATE_DEPOWER_W, icm20645PowerRead),
    sensorFsmCmd(STATE_DEPOWER_W, STATE_LP_R, icm20645PowerDisableWrite),
    sensorFsmCmd(STATE_LP_R, STATE_ENLP_W, icm20645LpRead),
    sensorFsmCmd(STATE_ENLP_W, STATE_GYRO_DISABLE_DONE, icm20645EnableLpWrite),
    /* gyro rate state */
    sensorFsmCmd(STATE_GYRO_RATECHG, STATE_LP_R, icm20645gyRate),
    sensorFsmCmd(STATE_LP_R, STATE_DELP_W, icm20645LpRead),
    sensorFsmCmd(STATE_DELP_W, STATE_DEFIFO_W, icm20645DisableLpWrite),
    sensorFsmCmd(STATE_DEFIFO_W, STATE_GYRO_RATE, icm20645FifoDisable),
    sensorFsmCmd(STATE_GYRO_RATE, STATE_FIFO_RST, icm20645gySetRate),
    sensorFsmCmd(STATE_FIFO_RST, STATE_FIFOREG2_R, icm20645FifoReset),
    sensorFsmCmd(STATE_FIFOREG2_R, STATE_GYRO_ENFIFO_W, icm20645FifoReg2Read),
    sensorFsmCmd(STATE_GYRO_ENFIFO_W, STATE_ENFIFO_W, icm20645gyFifoEnable),
    sensorFsmCmd(STATE_ENFIFO_W, STATE_LP_R, icm20645FifoEnable),
    sensorFsmCmd(STATE_LP_R, STATE_ENLP_W, icm20645LpRead),
    sensorFsmCmd(STATE_ENLP_W, STATE_GYRO_RATECHG_DONE, icm20645EnableLpWrite),
    /* init state */
    sensorFsmCmd(STATE_RESET_R, STATE_RESET_W, icm20645ResetRead),
    sensorFsmCmd(STATE_RESET_W, STATE_DEVID, icm20645ResetWrite),
    sensorFsmCmd(STATE_DEVID, STATE_SELCLK_R, icm20645DeviceId),
    sensorFsmCmd(STATE_SELCLK_R, STATE_SELCLK_W, icm20645SelClkRead),
    sensorFsmCmd(STATE_SELCLK_W, STATE_POWER_R, icm20645SelClkWrite),
    sensorFsmCmd(STATE_POWER_R, STATE_DEPOWER_W, icm20645PowerRead),
    sensorFsmCmd(STATE_DEPOWER_W, STATE_ENPOWER_W, icm20645PowerDisableWrite),
    sensorFsmCmd(STATE_ENPOWER_W, STATE_DUTY_CYCLE, icm20645PowerEnableWrite),
    sensorFsmCmd(STATE_DUTY_CYCLE, STATE_TURN_R, icm20645DutyCycle),
    sensorFsmCmd(STATE_TURN_R, STATE_ACC_TURNOFF_W, icm20645TurnRead),
    sensorFsmCmd(STATE_ACC_TURNOFF_W, STATE_TURN_R, icm20645gTurnOffWrite),
    sensorFsmCmd(STATE_TURN_R, STATE_GYRO_TURNOFF_W, icm20645TurnRead),
    sensorFsmCmd(STATE_GYRO_TURNOFF_W, STATE_ACC_CONFIG1, icm20645gyTurnOffWrite),
    sensorFsmCmd(STATE_ACC_CONFIG1, STATE_ACC_CONFIG2, icm20645AccConfig1),
    sensorFsmCmd(STATE_ACC_CONFIG2, STATE_GYRO_CONFIG1, icm20645AccConfig2),
    sensorFsmCmd(STATE_GYRO_CONFIG1, STATE_GYRO_CONFIG2, icm20645GyroConfig1),
    sensorFsmCmd(STATE_GYRO_CONFIG2, STATE_DATA_RESO, icm20645GyroConfig2),
    sensorFsmCmd(STATE_DATA_RESO, STATE_CALC_RESO, icm20645DataReso),
    sensorFsmCmd(STATE_CALC_RESO, STATE_TIMEBASE, icm20645CalcReso),
    sensorFsmCmd(STATE_TIMEBASE, STATE_POWER_R, icm20645ReadTimeBase),
    sensorFsmCmd(STATE_POWER_R, STATE_DEPOWER_W, icm20645PowerRead),
    sensorFsmCmd(STATE_DEPOWER_W, STATE_CORE, icm20645PowerDisableWrite),
    sensorFsmCmd(STATE_CORE, STATE_INIT_DONE, icm20645RegisterCore),
};
static void i2cAutoDetect(void *cookie, size_t tx, size_t rx, int err)
{
    if (err == 0) {
        osLog(LOG_INFO, "icm20645: auto detect success:0x%x\n", mTask.deviceId);
        registerAccGyroInterruptMode(ACC_GYRO_FIFO_UNINTERRUPTIBLE);
        registerAccGyroDriverFsm(icm20645Fsm, ARRAY_SIZE(icm20645Fsm));
    } else
        osLog(LOG_ERROR, "icm20645: auto detect error (%d)\n", err);
}

int icm20645Init(void)
{
    int ret = 0;

    icm20645DebugPoint = &mTask;
    insertMagicNum(&mTask.accGyroPacket);
    mTask.hw = get_cust_accGyro("icm20645");
    if (NULL == mTask.hw) {
        osLog(LOG_ERROR, "get_cust_acc_hw fail\n");
        return 0;
    }
    mTask.i2c_addr = mTask.hw->i2c_addr[0];
    osLog(LOG_ERROR, "acc i2c_num: %d, i2c_addr: 0x%x\n", mTask.hw->i2c_num, mTask.i2c_addr);

    if (0 != (ret = sensorDriverGetConvert(mTask.hw->direction, &mTask.cvt))) {
        osLog(LOG_ERROR, "invalid direction: %d\n", mTask.hw->direction);
    }
    osLog(LOG_ERROR, "acc map[0]:%d, map[1]:%d, map[2]:%d, sign[0]:%d, sign[1]:%d, sign[2]:%d\n\r",
        mTask.cvt.map[AXIS_X], mTask.cvt.map[AXIS_Y], mTask.cvt.map[AXIS_Z],
        mTask.cvt.sign[AXIS_X], mTask.cvt.sign[AXIS_Y], mTask.cvt.sign[AXIS_Z]);
    i2cMasterRequest(mTask.hw->i2c_num, I2C_SPEED);

    mTask.txBuf[0] = ICM20645_REG_DEVID;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         &mTask.deviceId, 1, i2cAutoDetect, NULL);
}
MODULE_DECLARE(icm20645, SENS_TYPE_ACCEL, icm20645Init);
