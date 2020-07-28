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
#include <plat/inc/spichre.h>
#include <spichre-plat.h>

#include <accGyro.h>
#include <contexthub_core.h>
#include <cust_accGyro.h>
#include <mt_gpt.h>

#define AXIS_X              0
#define AXIS_Y              1
#define AXIS_Z              2
#define AXES_NUM            3

/* WHOAMI */
#define ICM20600_WHOAMI                 0x11

#define ICM20600_PWR_MGMT_1             0x6b
#define ICM20600_PWR_MGMT_2             0x6c
#define ICM20600_REG_FIFO_R_W           0x74
#define ICM20600_REG_FIFO_COUNTH        0x72
#define ICM20600_REG_FIFO_COUNTL        0x73
#define ICM20600_REG_USER_CTRL          0x6a
#define ICM20600_REG_FIFO_EN            0x23

#define ICM20600_REG_SAMRT_DIV          0x19
/* gyro */
#define ICM20600_REG_GYRO_CONFIG_1      0x1a
#define ICM20600_REG_GYRO_CONFIG_2      0x1b

#define SHIFT_GYRO_FS_SEL               3
#define GYRO_FS_SEL_250                 (0x00 << SHIFT_GYRO_FS_SEL)
#define GYRO_FS_SEL_500                 (0x01 << SHIFT_GYRO_FS_SEL)
#define GYRO_FS_SEL_1000                (0x02 << SHIFT_GYRO_FS_SEL)
#define GYRO_FS_SEL_2000                (0x03 << SHIFT_GYRO_FS_SEL)

#define ICM20600_FS_250_LSB             131.0f
#define ICM20600_FS_500_LSB             65.5f
#define ICM20600_FS_1000_LSB            32.8f
#define ICM20600_FS_2000_LSB            16.4f

#define GYRO_AVGCFG_1X                  0
#define GYRO_AVGCFG_2X                  1
#define GYRO_AVGCFG_4X                  2
#define GYRO_AVGCFG_8X                  3
#define GYRO_AVGCFG_16X                 4
#define GYRO_AVGCFG_32X                 5
#define GYRO_AVGCFG_64X                 6
#define GYRO_AVGCFG_128X                7
#define GYRO_DLPFCFG_1                  (0x01)
#define GYRO_DLPFCFG_4                  (0x04)
#define GYRO_DLPFCFG                    (0x07)
#define GYRO_FCHOICE                    (0x00)

/* acc */
#define ICM20600_REG_ACC_CONFIG_1       0x1c
#define ICM20600_REG_ACC_CONFIG_2       0x1d

#define ICM20600_RANGE_2G               (0x00 << 3)
#define ICM20600_RANGE_4G               (0x01 << 3)
#define ICM20600_RANGE_8G               (0x02 << 3)
#define ICM20600_RANGE_16G              (0x03 << 3)

#define ACCEL_AVGCFG_1_4X               (0x00 << 4)
#define ACCEL_AVGCFG_8X                 (0x01 << 4)
#define ACCEL_AVGCFG_16X                (0x02 << 4)
#define ACCEL_AVGCFG_32X                (0x03 << 4)
#define ACCEL_FCHOICE                   1 << 3
#define ACCEL_DLPFCFG_2                 2
#define ACCEL_DLPFCFG_4                 4
#define ACCEL_DLPFCFG                   7

#define ICM20600_DEV_RESET              0x80

#define ICM20600_REG_DEVID              0x75

#define remapFifoDelay(delay_ns, pll)     ((delay_ns) * (1270ULL)/(1270ULL + pll))

enum ICM20600State {
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
    STATE_DEPOWER_W,
    STATE_ENPOWER_W,
    STATE_ACC_CONFIG,
    STATE_GYRO_CONFIG,
    STATE_CALC_RESO,
    STATE_TIMEBASE,
    STATE_CORE,
    STATE_FIFO_INIT,
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

static struct acc_data_resolution icm20600g_data_resolution[] = {
    {{ 0,  6}, 16384},  /*+/-2g  in 16-bit resolution:  0.06 mg/LSB*/
    {{ 0, 12}, 8192},   /*+/-4g  in 16-bit resolution:  0.12 mg/LSB*/
    {{ 0, 24}, 4096},   /*+/-8g  in 16-bit resolution:  0.24 mg/LSB*/
    {{ 0, 5}, 2048},    /*+/-16g in 16-bit resolution:  0.49 mg/LSB*/
};
static struct gyro_data_resolution icm20600gy_data_resolution[] = {
    {ICM20600_FS_250_LSB},
    {ICM20600_FS_500_LSB},
    {ICM20600_FS_1000_LSB},
    {ICM20600_FS_2000_LSB},
};

#define BOTH_IN_FIFO_LEN    14// 14 // ACC:6 Temperature:2 GYRO:6
#define SINGLE_IN_FIFO_LEN  8// 14 // ACC:6 Temperature:2 GYRO:6

#define SPI_PACKET_SIZE 30
#define SPI_BUF_SIZE    (1024 + 4)

#define SPI_WRITE_0(addr, data) spiQueueWrite(addr, data, 2)
#define SPI_WRITE_1(addr, data, delay) spiQueueWrite(addr, data, delay)
#define GET_SPI_WRITE_MACRO(_1, _2, _3, NAME, ...) NAME
#define SPI_WRITE(...) GET_SPI_WRITE_MACRO(__VA_ARGS__, SPI_WRITE_1, SPI_WRITE_0)(__VA_ARGS__)

#define SPI_READ_0(addr, size, buf) spiQueueRead(addr, size, buf, 0)
#define SPI_READ_1(addr, size, buf, delay) spiQueueRead(addr, size, buf, delay)
#define GET_SPI_READ_MACRO(_1, _2, _3, _4, NAME, ...) NAME
#define SPI_READ(...) GET_SPI_READ_MACRO(__VA_ARGS__, SPI_READ_1, SPI_READ_0)(__VA_ARGS__)

static struct ICM20600Task {
    bool accPowerOn;
    bool gyroPowerOn;
    bool accConfiged;
    bool gyroConfiged;
    uint8_t *regBuffer;
    int8_t timeBase;
    uint16_t fifoLen;
    uint32_t minAccGyroDelay;
    uint32_t maxAccGyroRate;
    uint32_t accRate;
    uint32_t gyroRate;
    uint16_t accRateDiv;
    uint16_t gyroRateDiv;
    uint64_t hwSampleTime;
    uint64_t swSampleTime;
    uint64_t lastSampleTime;
    uint64_t sampleTime;
    SpiCbkF spiCallBack;
    struct transferDataInfo dataInfo;
    struct accGyroDataPacket accGyroPacket;
    /* data for factory */
    struct TripleAxisDataPoint accFactoryData;
    struct TripleAxisDataPoint gyroFactoryData;

    int32_t accHwCali[AXES_NUM];
    int32_t gyroHwCali[AXES_NUM];
    struct acc_data_resolution *accReso;
    struct gyro_data_resolution *gyroReso;
    struct accGyro_hw *hw;
    struct sensorDriverConvert cvt;

    spi_cs_t cs;
    struct SpiMode mode;
    struct SpiPacket packets[SPI_PACKET_SIZE];
    struct SpiDevice *spiDev;
    uint8_t txrxBuffer[SPI_BUF_SIZE];
    uint16_t mWbufCnt;
    uint8_t mRegCnt;
    uint8_t mRetryLeft;
} mTask;
static struct ICM20600Task *icm20600DebugPoint;

static void spiQueueWrite(uint8_t addr, uint8_t data, uint32_t delay)
{
    mTask.packets[mTask.mRegCnt].size = 2;
    mTask.packets[mTask.mRegCnt].txBuf = &mTask.txrxBuffer[mTask.mWbufCnt];
    mTask.packets[mTask.mRegCnt].rxBuf = &mTask.txrxBuffer[mTask.mWbufCnt];
    mTask.packets[mTask.mRegCnt].delay = delay * 1000;
    mTask.txrxBuffer[mTask.mWbufCnt++] = addr;
    mTask.txrxBuffer[mTask.mWbufCnt++] = data;
    mTask.mWbufCnt = (mTask.mWbufCnt + 3) & 0xFFFC;
    mTask.mRegCnt++;
}

static void spiQueueRead(uint8_t addr, size_t size, uint8_t **buf, uint32_t delay)
{
    *buf = &mTask.txrxBuffer[mTask.mWbufCnt];
    mTask.packets[mTask.mRegCnt].size = size + 1;  // first byte will not contain valid data
    mTask.packets[mTask.mRegCnt].txBuf = &mTask.txrxBuffer[mTask.mWbufCnt];
    mTask.packets[mTask.mRegCnt].rxBuf = *buf;
    mTask.packets[mTask.mRegCnt].delay = delay * 1000;
    mTask.txrxBuffer[mTask.mWbufCnt++] = 0x80 | addr;
    mTask.mWbufCnt = (mTask.mWbufCnt + size + 3) & 0xFFFC;
    mTask.mRegCnt++;
}

static int spiBatchTxRx(struct SpiMode *mode,
        SpiCbkF callback, void *cookie, const char *src)
{
    int err = 0;

    if (mTask.mWbufCnt > SPI_BUF_SIZE) {
        osLog(LOG_INFO, "NO enough SPI buffer space, dropping transaction.\n");
        return -1;
    }
    if (mTask.mRegCnt > SPI_PACKET_SIZE) {
        osLog(LOG_INFO, "spiBatchTxRx too many packets!\n");
        return -1;
    }

    err = spiMasterRxTx(mTask.spiDev, mTask.cs, mTask.packets, mTask.mRegCnt, mode, callback, cookie);
    mTask.mRegCnt = 0;
    mTask.mWbufCnt = 0;
    return err;
}

void icm20600TimerCbkF(void)
{
    mTask.hwSampleTime = rtcGetTime();
}

static int icm20600ResetRead(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20600ResetRead\n");

    SPI_READ(ICM20600_PWR_MGMT_1, 1, &mTask.regBuffer);
    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
}

static int icm20600ResetWrite(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    uint8_t val = 0;

    //osLog(LOG_INFO, "icm20600ResetWrite 0x%x\n", mTask.regBuffer[1]);
    val = mTask.regBuffer[1] | ICM20600_DEV_RESET;
    SPI_WRITE(ICM20600_PWR_MGMT_1, val, 100000);
    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
}
static int icm20600PowerEnable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    uint8_t val;

    //osLog(LOG_INFO, "icm20600PowerEnable\n");
    val = 0b00001001;
    SPI_WRITE(ICM20600_PWR_MGMT_1, val, 5000);
    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
}
static int icm20600PowerDisable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    uint8_t val;

    //osLog(LOG_INFO, "icm20600PowerDisable\n");
    val = 0b01001001;
    SPI_WRITE(ICM20600_PWR_MGMT_1, val, 5000);
    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
}

static int icm20600AccConfig(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    uint8_t val;

    //osLog(LOG_INFO, "icm20600AccConfig\n");
    val = ICM20600_RANGE_4G;
    SPI_WRITE(ICM20600_REG_ACC_CONFIG_1, val);
    val = ACCEL_DLPFCFG_4;
    SPI_WRITE(ICM20600_REG_ACC_CONFIG_2, val);
    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
}

static int icm20600GyroConfig(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    uint8_t val;

    //osLog(LOG_INFO, "icm20600GyroConfig\n");
    val = GYRO_DLPFCFG_1;
    SPI_WRITE(ICM20600_REG_GYRO_CONFIG_1, val);
    val = GYRO_FS_SEL_2000;
    SPI_WRITE(ICM20600_REG_GYRO_CONFIG_2, val);
    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
}

static int icm20600CalcReso(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    uint8_t reso = 0;
    //osLog(LOG_INFO, "icm20600CalcReso:acc(0x%x), gyro(0x%x)\n", ICM20600_RANGE_4G, GYRO_FS_SEL_2000);
    reso = ICM20600_RANGE_4G >> 3;
    if (reso < sizeof(icm20600g_data_resolution) / sizeof(icm20600g_data_resolution[0])) {
        mTask.accReso = &icm20600g_data_resolution[reso];
        osLog(LOG_INFO, "acc reso: %d, sensitivity: %d\n", reso, mTask.accReso->sensitivity);
    }
    reso = GYRO_FS_SEL_2000 >> 3;
    mTask.gyroReso = &icm20600gy_data_resolution[reso];
    osLog(LOG_INFO, "gyro reso: %d, sensitivity: %f\n", reso, (double)mTask.gyroReso->sensitivity);
    sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int icm20600ReadTimeBase(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.timeBase = 11;  //icm20600 have no pll register, so Directly specify
    sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int icm20600gEnable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    uint8_t val = 0;

    osLog(LOG_INFO, "icm20600gEnable gyro power mode=%d\n", mTask.gyroPowerOn);
    mTask.accPowerOn = true;
    if (mTask.gyroPowerOn == false) {
        val = 0b00001001;
        SPI_WRITE(ICM20600_PWR_MGMT_1, val, 5000);
        val = 0b00000111;
        SPI_WRITE(ICM20600_PWR_MGMT_2, val, 35000);
    } else {
        val = 0b00000000;
        SPI_WRITE(ICM20600_PWR_MGMT_2, val, 35000);
    }
    ret = spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
    return ret;
}
static int icm20600gDisable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    uint8_t val = 0;
    struct accGyroCntlPacket cntlPacket;

    ret = rxControlInfo(&cntlPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
        osLog(LOG_INFO, "icm20600gDisable, rx water_mark err\n");
        return -1;
    }
    osLog(LOG_INFO, "icm20600gDisable: water_mark:%d\n", cntlPacket.waterMark);

    mTask.accPowerOn = false;
    mTask.accRate = 0;
    mTask.accConfiged = false;

    if (mTask.gyroConfiged == true) {
        mTask.maxAccGyroRate = mTask.gyroRate;
        mTask.minAccGyroDelay = (mTask.maxAccGyroRate == 0) ? 0 : 1000000000000ULL / mTask.maxAccGyroRate;
        registerAccGyroFifoInfo(remapFifoDelay(mTask.minAccGyroDelay, mTask.timeBase),
            remapFifoDelay(mTask.minAccGyroDelay, mTask.timeBase));
        /* set sample rate */
        val = mTask.gyroRateDiv;
        SPI_WRITE(ICM20600_REG_SAMRT_DIV, val);
        osLog(LOG_INFO, "icm20600gDisable RateDiv: %d\n", val);
    }
    /* set standby and disable fifo */
    if (mTask.gyroPowerOn == false) {
        val = 0b00111111;
        SPI_WRITE(ICM20600_PWR_MGMT_2, val, 35000);
        val = 0b00000000;
        SPI_WRITE(ICM20600_REG_FIFO_EN, val);
    } else {
        val = 0b00111000;
        SPI_WRITE(ICM20600_PWR_MGMT_2, val, 35000);
        val = 0b00010000;
        SPI_WRITE(ICM20600_REG_FIFO_EN, val);
    }
    /* reset fifo */
    val = 0b01000100;
    SPI_WRITE(ICM20600_REG_USER_CTRL, val);
    /* disable power */
    if (mTask.gyroPowerOn == false) {
        val = 0b01001001;
        SPI_WRITE(ICM20600_PWR_MGMT_1, val, 5000);
    }
    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
}
static int icm20600gRate(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    uint8_t val = 0;
    uint32_t sample_rate, water_mark;
    struct accGyroCntlPacket cntlPacket;

    ret = rxControlInfo(&cntlPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "icm20600gRate, rx inSize and elemSize error\n");
        return -1;
    }
    sample_rate = cntlPacket.rate;
    water_mark = cntlPacket.waterMark;
    osLog(LOG_INFO, "icm20600gRate, rate:%d, water_mark:%d\n",sample_rate, water_mark);

    if (sample_rate > 0 && sample_rate < 6000) {
        mTask.accRate = SENSOR_HZ(5);
        mTask.accRateDiv = 199;
    } else if (sample_rate > 9000 && sample_rate < 11000) {
        mTask.accRate = SENSOR_HZ(10);
        mTask.accRateDiv = 99;
    } else if (sample_rate > 16000 && sample_rate < 17000) {
        mTask.accRate = SENSOR_HZ(16.666);
        mTask.accRateDiv = 59;
    } else if (sample_rate > 40000 && sample_rate < 60000) {
        mTask.accRate = SENSOR_HZ(50);
        mTask.accRateDiv = 19;
    } else if (sample_rate > 90000 && sample_rate < 110000) {
        mTask.accRate = SENSOR_HZ(100);
        mTask.accRateDiv = 9;
    } else if (sample_rate > 190000 && sample_rate < 210000) {
        mTask.accRate = SENSOR_HZ(200);
        mTask.accRateDiv = 4;
    } else {
        mTask.accRate = sample_rate;
        if (sample_rate <= 4000)
            mTask.accRateDiv = 255;
        else if (sample_rate >= 1000000)
            mTask.accRateDiv = 0;
        else
            mTask.accRateDiv = 1024000 / sample_rate - 1;
    }

    if (mTask.gyroConfiged == true)
        mTask.maxAccGyroRate = (mTask.accRate > mTask.gyroRate ? mTask.accRate : mTask.gyroRate);
    else
        mTask.maxAccGyroRate = mTask.accRate;

    mTask.minAccGyroDelay = 1024000000000ULL / mTask.maxAccGyroRate;
    osLog(LOG_INFO, "icm20600gRate: %d, minAccGyroDelay:%d\n", mTask.accRate, mTask.minAccGyroDelay);

    mTask.accConfiged = true;
    registerAccGyroFifoInfo(remapFifoDelay(mTask.minAccGyroDelay, mTask.timeBase),
        remapFifoDelay(mTask.minAccGyroDelay, mTask.timeBase));
    /* set sample rate */
    if (mTask.accRate < mTask.gyroRate && mTask.gyroConfiged == true)
        val = mTask.gyroRateDiv;
    else if (mTask.accConfiged == true)
        val = mTask.accRateDiv;
    osLog(LOG_INFO, "icm20600gRate RateDiv: %d\n", val);
    SPI_WRITE(ICM20600_REG_SAMRT_DIV, val);
    /* reset fifo */
    val = 0b01000100;
    SPI_WRITE(ICM20600_REG_USER_CTRL, val);

    if (mTask.gyroConfiged == false)
        val = 0b00001000;
    else
        val = 0b00011000;
    SPI_WRITE(ICM20600_REG_FIFO_EN, val);
    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
}

static int icm20600FifoInit(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    uint8_t val;

    //osLog(LOG_INFO, "icm20600FifoInit\n");
    val = 0b00000100;
    SPI_WRITE(ICM20600_REG_USER_CTRL, val);
    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
}

static int icm20600gyEnable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    uint8_t val = 0;

    osLog(LOG_INFO, "icm20600gEnable acc power mode=%d\n", mTask.accPowerOn);
    mTask.gyroPowerOn = true;
    if (mTask.accPowerOn == false) {
        val = 0b00001001;
        SPI_WRITE(ICM20600_PWR_MGMT_1, val, 5000);
        val = 0b00111000;
        SPI_WRITE(ICM20600_PWR_MGMT_2, val, 35000);
    } else {
        val = 0b00000000;
        SPI_WRITE(ICM20600_PWR_MGMT_2, val, 35000);
    }
    ret = spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
    return ret;
}
static int icm20600gyDisable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    uint8_t val = 0;
    struct accGyroCntlPacket cntlPacket;

    ret = rxControlInfo(&cntlPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
        osLog(LOG_INFO, "icm20600gyDisable, rx water_mark err\n");
        return -1;
    }
    osLog(LOG_INFO, "icm20600gyDisable: water_mark:%d\n", cntlPacket.waterMark);

    mTask.gyroPowerOn = false;
    mTask.gyroRate = 0;
    mTask.gyroConfiged = false;

    if (mTask.accConfiged == true) {
        mTask.maxAccGyroRate = mTask.accRate;
        mTask.minAccGyroDelay = (mTask.maxAccGyroRate == 0) ? 0 : 1000000000000ULL / mTask.maxAccGyroRate;
        registerAccGyroFifoInfo(remapFifoDelay(mTask.minAccGyroDelay, mTask.timeBase),
            remapFifoDelay(mTask.minAccGyroDelay, mTask.timeBase));
        /* set sample rate */
        val = mTask.accRateDiv;
        SPI_WRITE(ICM20600_REG_SAMRT_DIV, val);
        osLog(LOG_INFO, "icm20600gyDisable RateDiv: %d\n", val);
    }
    /* set standby and disable fifo */
    if (mTask.accPowerOn == false) {
        val = 0b00111111;
        SPI_WRITE(ICM20600_PWR_MGMT_2, val, 35000);
        val = 0b00000000;
        SPI_WRITE(ICM20600_REG_FIFO_EN, val);
    } else {
        val = 0b00000111;
        SPI_WRITE(ICM20600_PWR_MGMT_2, val, 35000);
        val = 0b00001000;
        SPI_WRITE(ICM20600_REG_FIFO_EN, val);
    }
    /* reset fifo */
    val = 0b01000100;
    SPI_WRITE(ICM20600_REG_USER_CTRL, val);
    /* disable power */
    if (mTask.accPowerOn == false) {
        val = 0b01001001;
        SPI_WRITE(ICM20600_PWR_MGMT_1, val, 5000);
    }
    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
}
static int icm20600gyRate(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    uint32_t sample_rate, water_mark;
    uint8_t val = 0;
    struct accGyroCntlPacket cntlPacket;

    ret = rxControlInfo(&cntlPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "icm20600gyRate, rx inSize and elemSize error\n");
        return -1;
    }
    sample_rate = cntlPacket.rate;
    water_mark = cntlPacket.waterMark;
    osLog(LOG_INFO, "icm20600gyRate, rate:%d, water_mark:%d\n", sample_rate, water_mark);

    if (sample_rate > 0 && sample_rate < 6000) {
        mTask.gyroRate = SENSOR_HZ(5);
        mTask.gyroRateDiv = 199;
    } else if (sample_rate > 9000 && sample_rate < 11000) {
        mTask.gyroRate = SENSOR_HZ(10);
        mTask.gyroRateDiv = 99;
    } else if (sample_rate > 16000 && sample_rate < 17000) {
        mTask.gyroRate = SENSOR_HZ(16.666);
        mTask.gyroRateDiv = 59;
    } else if (sample_rate > 40000 && sample_rate < 60000) {
        mTask.gyroRate = SENSOR_HZ(50);
        mTask.gyroRateDiv = 19;
    } else if (sample_rate > 90000 && sample_rate < 110000) {
        mTask.gyroRate = SENSOR_HZ(100);
        mTask.gyroRateDiv = 9;
    } else if (sample_rate > 190000 && sample_rate < 210000) {
        mTask.gyroRate = SENSOR_HZ(200);
        mTask.gyroRateDiv = 4;
    } else {
        mTask.gyroRate = sample_rate;
        if (sample_rate <= 4000)
            mTask.gyroRateDiv = 255;
        else if (sample_rate >= 1000000)
            mTask.gyroRateDiv = 0;
        else
            mTask.gyroRateDiv = 1024000 / sample_rate - 1;
    }
    if (mTask.accConfiged == true)
        mTask.maxAccGyroRate = (mTask.accRate > mTask.gyroRate ? mTask.accRate : mTask.gyroRate);
    else
        mTask.maxAccGyroRate = mTask.gyroRate;

    mTask.minAccGyroDelay = 1024000000000ULL / mTask.maxAccGyroRate;
    osLog(LOG_INFO, "icm20600gyRate: %d, minAccGyroDelay: %d\n", mTask.gyroRate, mTask.minAccGyroDelay);

    mTask.gyroConfiged = true;
    registerAccGyroFifoInfo(remapFifoDelay(mTask.minAccGyroDelay, mTask.timeBase),
        remapFifoDelay(mTask.minAccGyroDelay, mTask.timeBase));
    /* set sample rate */
    if (mTask.accRate > mTask.gyroRate && mTask.accConfiged == true)
        val = mTask.accRateDiv;
    else if (mTask.gyroConfiged == true)
        val = mTask.gyroRateDiv;
    SPI_WRITE(ICM20600_REG_SAMRT_DIV, val);
    osLog(LOG_INFO, "icm20600gyRate RateDiv: %d\n", val);
    /* reset fifo */
    val = 0b01000100;
    SPI_WRITE(ICM20600_REG_USER_CTRL, val);

    if (mTask.accConfiged == false)
        val = 0b00010000;
    else
        val = 0b00011000;
    SPI_WRITE(ICM20600_REG_FIFO_EN, val);
    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
}

static void accGetCalibration(int32_t *cali, int32_t size)
{
    cali[AXIS_X]  = mTask.accHwCali[AXIS_X];
    cali[AXIS_Y]  = mTask.accHwCali[AXIS_Y];
    cali[AXIS_Z]  = mTask.accHwCali[AXIS_Z];
    osLog(LOG_INFO, "accGetCalibration cali x:%d, y:%d, z:%d\n", cali[AXIS_X], cali[AXIS_Y], cali[AXIS_Z]);
}
static void accSetCalibration(int32_t *cali, int32_t size)
{
    mTask.accHwCali[AXIS_X] = cali[AXIS_X];
    mTask.accHwCali[AXIS_Y] = cali[AXIS_Y];
    mTask.accHwCali[AXIS_Z] = cali[AXIS_Z];
    osLog(LOG_INFO, "accSetCalibration cali x:%d, y:%d, z:%d\n", mTask.accHwCali[AXIS_X],
        mTask.accHwCali[AXIS_Y], mTask.accHwCali[AXIS_Z]);
}
static void accGetData(void *sample)
{
    struct TripleAxisDataPoint *tripleSample = (struct TripleAxisDataPoint *)sample;
    tripleSample->ix = mTask.accFactoryData.ix;
    tripleSample->iy = mTask.accFactoryData.iy;
    tripleSample->iz = mTask.accFactoryData.iz;

    osLog(LOG_INFO,"accGetData x:%d, y:%d, z:%d\n", tripleSample->ix, tripleSample->iy, tripleSample->iz);
}
static void gyroGetCalibration(int32_t *cali, int32_t size)
{
    cali[AXIS_X] = mTask.gyroHwCali[AXIS_X];
    cali[AXIS_Y] = mTask.gyroHwCali[AXIS_Y];
    cali[AXIS_Z] = mTask.gyroHwCali[AXIS_Z];
    osLog(LOG_INFO, "gyroGetCalibration cali x:%d, y:%d, z:%d\n", cali[AXIS_X], cali[AXIS_Y], cali[AXIS_Z]);
}
static void gyroSetCalibration(int32_t *cali, int32_t size)
{
    mTask.gyroHwCali[AXIS_X] = cali[AXIS_X];
    mTask.gyroHwCali[AXIS_Y] = cali[AXIS_Y];
    mTask.gyroHwCali[AXIS_Z] = cali[AXIS_Z];
    osLog(LOG_INFO, "gyroSetCalibration cali x:%d, y:%d, z:%d\n", mTask.gyroHwCali[AXIS_X],
        mTask.gyroHwCali[AXIS_Y], mTask.gyroHwCali[AXIS_Z]);
}
static void gyroGetData(void *sample)
{
    struct TripleAxisDataPoint *tripleSample = (struct TripleAxisDataPoint *)sample;
    tripleSample->ix = mTask.gyroFactoryData.ix;
    tripleSample->iy = mTask.gyroFactoryData.iy;
    tripleSample->iz = mTask.gyroFactoryData.iz;

    osLog(LOG_INFO,"gyroGetData x:%d, y:%d, z:%d\n", tripleSample->ix, tripleSample->iy, tripleSample->iz);
}

static int icm20600RegisterCore(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
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

    sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
    return 0;
}
static int icm20600Sample(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;

    //osLog(LOG_ERROR, "icm20600Sample\n");
    ret = rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "icm20600Sample, rx dataInfo error\n");
        return -1;
    }

    SPI_READ(ICM20600_REG_FIFO_COUNTH, 2, &mTask.regBuffer);
    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
}

static void spiIsrCallBack(void *cookie, int err)
{
    if (err != 0) {
        osLog(LOG_ERROR, "icm20600: spiIsrCallBack err\n");
        sensorFsmEnqueueFakeSpiEvt(mTask.spiCallBack, cookie, ERROR_EVT);
    } else {
        mTask.swSampleTime = rtcGetTime();
        sensorFsmEnqueueFakeSpiEvt(mTask.spiCallBack, cookie, SUCCESS_EVT);
    }
}
static int icm20600ReadFifo(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.sampleTime = rtcGetTime();

    mTask.fifoLen = (mTask.regBuffer[1] << 8) | mTask.regBuffer[2];

    if (mTask.gyroConfiged && mTask.accConfiged) {
        mTask.fifoLen = (mTask.fifoLen / BOTH_IN_FIFO_LEN) * BOTH_IN_FIFO_LEN;
    } else if (mTask.gyroConfiged || mTask.accConfiged) {
        mTask.fifoLen = (mTask.fifoLen / SINGLE_IN_FIFO_LEN) * SINGLE_IN_FIFO_LEN;
    }

    if (mTask.fifoLen <= 0) {
        osLog(LOG_INFO, "icm20600ReadFifo fifo len is '%d'\n", mTask.fifoLen);
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
        return 0;
    }
    mTask.spiCallBack = spiCallBack;
    SPI_READ(ICM20600_REG_FIFO_R_W, mTask.fifoLen, &mTask.regBuffer);
    return spiBatchTxRx(&mTask.mode, spiIsrCallBack, next_state, __FUNCTION__);
}
static int icm20600gConvert(uint8_t *databuf, struct accGyroData *data)
{
    int32_t raw_data[AXES_NUM];
    int32_t remap_data[AXES_NUM];

    raw_data[AXIS_X] = (int16_t)((databuf[AXIS_X * 2] << 8) | databuf[AXIS_X * 2 + 1]);
    raw_data[AXIS_Y] = (int16_t)((databuf[AXIS_Y * 2] << 8) | databuf[AXIS_Y * 2 + 1]);
    raw_data[AXIS_Z] = (int16_t)((databuf[AXIS_Z * 2] << 8) | databuf[AXIS_Z * 2 + 1]);
    osLog(LOG_INFO, "%s acc rawdata x:%d, y:%d, z:%d\n", __func__, raw_data[AXIS_X],
        raw_data[AXIS_Y], raw_data[AXIS_Z]);

    raw_data[AXIS_X] = raw_data[AXIS_X] + mTask.accHwCali[AXIS_X];
    raw_data[AXIS_Y] = raw_data[AXIS_Y] + mTask.accHwCali[AXIS_Y];
    raw_data[AXIS_Z] = raw_data[AXIS_Z] + mTask.accHwCali[AXIS_Z];

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

    mTask.accFactoryData.ix = data->x;
    mTask.accFactoryData.iy = data->y;
    mTask.accFactoryData.iz = data->z;

    return 0;
}
static int icm20600gyConvert(uint8_t *databuf, struct accGyroData *data)
{
    int32_t raw_data[AXES_NUM];
    int32_t remap_data[AXES_NUM];

    raw_data[AXIS_X] = (int16_t)((databuf[AXIS_X * 2] << 8) | databuf[AXIS_X * 2 + 1]);
    raw_data[AXIS_Y] = (int16_t)((databuf[AXIS_Y * 2] << 8) | databuf[AXIS_Y * 2 + 1]);
    raw_data[AXIS_Z] = (int16_t)((databuf[AXIS_Z * 2] << 8) | databuf[AXIS_Z * 2 + 1]);
    osLog(LOG_INFO, "gyro rawdata x:%d, y:%d, z:%d\n", raw_data[AXIS_X],
        raw_data[AXIS_Y], raw_data[AXIS_Z]);
    raw_data[AXIS_X] = raw_data[AXIS_X] + mTask.gyroHwCali[AXIS_X];
    raw_data[AXIS_Y] = raw_data[AXIS_Y] + mTask.gyroHwCali[AXIS_Y];
    raw_data[AXIS_Z] = raw_data[AXIS_Z] + mTask.gyroHwCali[AXIS_Z];

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

    return 0;
}

static int icm20600Convert(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    struct accGyroData *data = mTask.accGyroPacket.outBuf;
    uint8_t accEventSize = 0;
    uint8_t gyroEventSize = 0;
    uint64_t swSampleTime = 0, realSampleTime = 0;
    uint8_t N = 0, i = 0;
    uint8_t *databuf;

    if (mTask.fifoLen <= 0) {
        txTransferDataInfo(&mTask.dataInfo, 0, 0, mTask.sampleTime, data);
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
        osLog(LOG_INFO, "icm20600Convert mTask.fifoLen:%u, invalid, trans 0,reset fifo\n", mTask.fifoLen);
        return 0;
    }

    if (mTask.gyroConfiged && mTask.accConfiged) {
        N = mTask.fifoLen / BOTH_IN_FIFO_LEN;
        N = N * 2;
    } else if (mTask.gyroConfiged || mTask.accConfiged) {
        N = mTask.fifoLen / SINGLE_IN_FIFO_LEN;
    }
    N = (N > MAX_RECV_PACKET) ? MAX_RECV_PACKET : N;
    if (N == MAX_RECV_PACKET)
        osLog(LOG_INFO, "SW buffer is full.\n");
    for (i = 0; i < N; ++i) {
        if (mTask.accConfiged == true && mTask.gyroConfiged == true) {
            if (i % 2 == 0) {
                databuf = &mTask.regBuffer[i * 7 + 1];
                accEventSize++;
                icm20600gConvert(databuf, &data[i]);
            } else {
                databuf = &mTask.regBuffer[i * 7 + 1 + 1];
                gyroEventSize++;
                icm20600gyConvert(databuf, &data[i]);
            }
        } else if (mTask.accConfiged == false && mTask.gyroConfiged == true) {
            databuf = &mTask.regBuffer[i * 8 + 2 + 1];
            gyroEventSize++;
            icm20600gyConvert(databuf, &data[i]);
        } else if (mTask.accConfiged == true && mTask.gyroConfiged == false) {
            databuf = &mTask.regBuffer[i * 8 + 1];
            accEventSize++;
            icm20600gConvert(databuf, &data[i]);
        } else
            osLog(LOG_INFO, "icm20600Convert accConfiged: %d, gyroConfiged:%d\n",
            mTask.accConfiged, mTask.gyroConfiged);
    }
    if ((accEventSize + gyroEventSize) != N)
        osLog(LOG_INFO, "icm20600Convert wrong num accEventSize:%d, gyroEventSize:%d,total:%d\n",
            accEventSize, gyroEventSize, N);
    swSampleTime = mTask.swSampleTime;
    realSampleTime = calcFakeInterruptTime(swSampleTime, mTask.hwSampleTime, mTask.lastSampleTime,
        mTask.accRate, mTask.accConfiged, accEventSize,
        mTask.gyroRate, mTask.gyroConfiged, gyroEventSize);
    osLog(LOG_INFO, "icm20600Convert, swSampleTime=%lld, hwSampleTime=%lld, realSampleTime=%lld, lastSampleTime=%lld, now=%lld\n",
        swSampleTime, mTask.hwSampleTime, realSampleTime, mTask.lastSampleTime, rtcGetTime());
    osLog(LOG_INFO, "icm20600Convert, accEventSize=%d, gyroEventSize=%d\n",
        accEventSize, gyroEventSize);
    filterDataFifoUninterruptible(realSampleTime, mTask.lastSampleTime,
        mTask.accRate, mTask.accConfiged, &accEventSize,
        mTask.gyroRate, mTask.gyroConfiged, &gyroEventSize);

    osLog(LOG_INFO, "icm20600Convert, resize accEventSize=%d, gyroEventSize=%d\n",
        accEventSize, gyroEventSize);
    mTask.hwSampleTime = realSampleTime;
    mTask.lastSampleTime = realSampleTime;
    txTransferDataInfo(&mTask.dataInfo, accEventSize, gyroEventSize, realSampleTime, data);
    sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static struct sensorFsm icm20600Fsm[] = {
    /* sample */
    sensorFsmCmd(STATE_SAMPLE, STATE_FIFO, icm20600Sample),
    sensorFsmCmd(STATE_FIFO, STATE_CONVERT, icm20600ReadFifo),
    sensorFsmCmd(STATE_CONVERT, STATE_SAMPLE_DONE, icm20600Convert),

    /* acc enable state */
    sensorFsmCmd(STATE_ACC_ENABLE, STATE_ACC_ENABLE_DONE, icm20600gEnable),

    /* acc disable state */
    sensorFsmCmd(STATE_ACC_DISABLE, STATE_ACC_DISABLE_DONE, icm20600gDisable),

    /* acc rate state */
    sensorFsmCmd(STATE_ACC_RATECHG, STATE_ACC_RATECHG_DONE, icm20600gRate),

    /* gyro enable state */
    sensorFsmCmd(STATE_GYRO_ENABLE, STATE_GYRO_ENABLE_DONE, icm20600gyEnable),

    /* gyro disable state */
    sensorFsmCmd(STATE_GYRO_DISABLE, STATE_GYRO_DISABLE_DONE, icm20600gyDisable),

    /* gyro rate state */
    sensorFsmCmd(STATE_GYRO_RATECHG, STATE_GYRO_RATECHG_DONE, icm20600gyRate),

    /* init state */
    sensorFsmCmd(STATE_RESET_R, STATE_RESET_W, icm20600ResetRead),
    sensorFsmCmd(STATE_RESET_W, STATE_ENPOWER_W, icm20600ResetWrite),
    sensorFsmCmd(STATE_ENPOWER_W, STATE_ACC_CONFIG, icm20600PowerEnable),
    sensorFsmCmd(STATE_ACC_CONFIG, STATE_GYRO_CONFIG, icm20600AccConfig),
    sensorFsmCmd(STATE_GYRO_CONFIG, STATE_FIFO_INIT, icm20600GyroConfig),
    sensorFsmCmd(STATE_FIFO_INIT, STATE_CALC_RESO, icm20600FifoInit),
    sensorFsmCmd(STATE_CALC_RESO, STATE_TIMEBASE, icm20600CalcReso),
    sensorFsmCmd(STATE_TIMEBASE, STATE_DEPOWER_W, icm20600ReadTimeBase),
    sensorFsmCmd(STATE_DEPOWER_W, STATE_CORE, icm20600PowerDisable),
    sensorFsmCmd(STATE_CORE, STATE_INIT_DONE, icm20600RegisterCore),
};
static void spiAutoDetect(void *cookie, int err)
{
    if (err == 0) {
        if(mTask.regBuffer[1] == ICM20600_WHOAMI) {
            osLog(LOG_INFO, "icm20600: auto detect success:0x%x\n", mTask.regBuffer[1]);
            registerAccGyroInterruptMode(ACC_GYRO_FIFO_UNINTERRUPTIBLE);
            registerAccGyroDriverFsm(icm20600Fsm, ARRAY_SIZE(icm20600Fsm));
            registerAccGyroTimerCbk(icm20600TimerCbkF);
        } else {
            osLog(LOG_INFO, "icm20600: auto detect fail\n");
            spiMasterRelease(mTask.spiDev);
        }
    } else
        osLog(LOG_ERROR, "icm20600: auto detect error (%d)\n", err);
}

int icm20600Init(void)
{
    int ret = 0;

    icm20600DebugPoint = &mTask;
    insertMagicNum(&mTask.accGyroPacket);
    mTask.hw = get_cust_accGyro("icm20600");
    if (NULL == mTask.hw) {
        osLog(LOG_ERROR, "get_cust_acc_hw fail\n");
        return 0;
    }

    mTask.accPowerOn = false;
    mTask.gyroPowerOn = false;

    osLog(LOG_ERROR, "acc spi_num: %d\n", mTask.hw->i2c_num);

    if (0 != (ret = sensorDriverGetConvert(mTask.hw->direction, &mTask.cvt))) {
        osLog(LOG_ERROR, "invalid direction: %d\n", mTask.hw->direction);
    }
    osLog(LOG_ERROR, "acc map[0]:%d, map[1]:%d, map[2]:%d, sign[0]:%d, sign[1]:%d, sign[2]:%d\n\r",
        mTask.cvt.map[AXIS_X], mTask.cvt.map[AXIS_Y], mTask.cvt.map[AXIS_Z],
        mTask.cvt.sign[AXIS_X], mTask.cvt.sign[AXIS_Y], mTask.cvt.sign[AXIS_Z]);

    mTask.mode.speed = 8000000;    //8Mhz
    mTask.mode.bitsPerWord = 8;
    mTask.mode.cpol = SPI_CPOL_IDLE_HI;
    mTask.mode.cpha = SPI_CPHA_TRAILING_EDGE;
    mTask.mode.nssChange = true;
    mTask.mode.format = SPI_FORMAT_MSB_FIRST;
    mTask.mWbufCnt = 0;
    mTask.mRegCnt = 0;

    spiMasterRequest(mTask.hw->i2c_num, &mTask.spiDev);

    SPI_READ(ICM20600_REG_DEVID, 1, &mTask.regBuffer);
    return spiBatchTxRx(&mTask.mode, spiAutoDetect, NULL, __FUNCTION__);
}
MODULE_DECLARE(icm20600, SENS_TYPE_ACCEL, icm20600Init);
