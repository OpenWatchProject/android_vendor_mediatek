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
#include <atomic.h>
#include <cpu/inc/cpuMath.h>
#include <gpio.h>
#include <heap.h>
#include <slab.h>
#include <timer.h>
#include <variant/inc/variant.h>
#include <limits.h>
#include <performance.h>
#include <sensors.h>
#include <plat/inc/rtc.h>
#include <plat/inc/spichre.h>
#include <spichre-plat.h>
#include <hostIntf.h>
#include <nanohub_math.h>
#include <nanohubPacket.h>
#include <plat/inc/rtc.h>
#include <spi.h>
#include <contexthub_core.h>
#include <accGyro.h>
#include <contexthub_core.h>
#include <cust_accGyro.h>
#include <mt_gpt.h>
#include <API_sensor_calibration.h>
#include <algos/time_sync.h>
#include "hwsen.h"
#include "eint.h"

#define ACC_NAME     "icm40605_acc"
#define GYRO_NAME     "icm40605_gyro"

#define ICM40605_FIFO_SIZE                       1024
#define G                                        9.80665
#define PI                                       3.141592
#define KSCALE_ACC_8G_RANGE                      0.002394202f     // ACC_range * 9.81f / 65536.0f;
#define KSCALE_GYRO_2000_RANGE                   0.001065264f  // GYR_range * M_PI / (180.0f * 65536.0f);

/* ICM40605 register  */
#define ICM40605_REG_DEV_CFG                 0x11
#define ICM40605_REG_SPI_SPEED               0x13
#define ICM40605_REG_INT_CFG                 0x14
#define ICM40605_REG_FIFO_CFG                0x16
#define ICM40605_REG_INT_STATUS              0x2D
#define ICM40605_REG_FIFO_BYTE_COUNT_L       0x2E
#define ICM40605_REG_FIFO_DATA               0x30
#define ICM40605_REG_SIGNAL_PATH_RESET_REG   0x4B
#define ICM40605_REG_INTF_CFG0               0x4C
#define ICM40605_REG_PWR_MGMT0               0x4E
#define ICM40605_REG_GYRO_CFG                0x4F
#define ICM40605_REG_ACC_CFG                 0x50
#define ICM40605_REG_BW_CFG                  0x52
#define ICM40605_REG_ACC_WOM_X_THR           0x54
#define ICM40605_REG_ACC_WOM_Y_THR           0x55
#define ICM40605_REG_ACC_WOM_Z_THR           0x56
#define ICM40605_REG_SMD_CFG                 0x57
#define ICM40605_REG_FIFO_CONFIG_1           0x5f
#define ICM40605_REG_INT_STATUS2             0x59
#define ICM40605_REG_FIFO_WM_TH_L            0x60
#define ICM40605_REG_FIFO_WM_TH_H            0x61
#define ICM40605_REG_INT_CFG1                0x64
#define ICM40605_REG_INT_SOURCE0             0x65
#define ICM40605_REG_INT_SOURCE1             0x66
#define ICM40605_REG_DEVID                   0x75

/* register config */
#define DEF_WOM_THRESHOLD                  100
#define GYRO_MODE_SHIFT                    2
#define ACC_BW_SHIFT                       4
#define ACC_GYRO_FS_SHIFT                  5
#define FIFO_CONFIG_SHIFT                  6
#define ICM40605L_WHOAMI                   0x32
#define ICM40605_WHOAMI                    0x33
#define ICM40606_WHOAMI                    0x37
#define BIT_SOFT_RESET                     0x01
#define BIT_INT_POL_HIGH                   0x03
#define BIT_GYRO_ODR_1K                    0x06
#define BIT_GYRO_ODR_200                   0x07
#define BIT_GYRO_ODR_25                    0x0A
#define BIT_GYRO_FS_2000                   (0x00 << ACC_GYRO_FS_SHIFT)
#define BIT_GYRO_FS_250                    (0x03 << ACC_GYRO_FS_SHIFT)
#define BIT_GYRO_BW_DIV_4                  0x01
#define BIT_GYRO_BW_DIV_2                  0x00
#define BIT_ACC_ODR_1K                     0x06
#define BIT_ACC_ODR_200                    0x07
#define BIT_ACC_ODR_100                    0x08
#define BIT_ACC_ODR_50                     0x09
#define BIT_ACC_ODR_25                     0x0A
#define BIT_ACC_FS_2G                      (0x03 << ACC_GYRO_FS_SHIFT)
#define BIT_ACC_FS_4G                      (0x02 << ACC_GYRO_FS_SHIFT)
#define BIT_ACC_FS_8G                      (0x01 << ACC_GYRO_FS_SHIFT)
#define BIT_ACC_FS_16G                     (0x00 << ACC_GYRO_FS_SHIFT)
#define BIT_ACC_BW_DIV_4                   (0x01 << ACC_BW_SHIFT)
#define BIT_ACC_BW_DIV_2                   (0x00 << ACC_BW_SHIFT)
#define BIT_ACC_LN_MODE                    0x03
#define BIT_GYRO_LN_MODE                   (0x03 << GYRO_MODE_SHIFT)
#define BIT_WOM_AND_MODE                   0x08
#define BIT_WOM_COMPARE_PRE                0x04
#define BIT_SMD_SHORT_3S                   0x03
#define BIT_SMD_SHORT_1S                   0x02
#define BIT_SMD_RESERVED                   0x01
#define BIT_SMD_DISABLED                   0x00
#define BIT_WOM_EN                         0x07
#define BIT_SMD_EN                         0x08
#define BIT_FIFO_HIRES_EN_MASK             0x10
#define BIT_FIFO_TMST_FSYNC_EN_MASK        0x08
#define BIT_FIFO_TEMP_EN_MASK              0x04
#define BIT_FIFO_GYRO_EN_MASK              0x02
#define BIT_FIFO_ACCEL_EN_MASK             0x01
#define BIT_FIFO_FLUSH                     0x02
#define BIT_FIFO_FULL_EN_MASK              0x02
#define BIT_WM_INT_EN_MASK                 0x04
#define BIT_INT1_PUSH_PULL_MASK            0x02
#define BIT_FIFO_STREAM_MODE               (0x01 << FIFO_CONFIG_SHIFT)
#define BIT_FIFO_BYPASS_MODE               (0x00 << FIFO_CONFIG_SHIFT)
#define BIT_FIFO_RECORD_MODE               (0x1 << FIFO_CONFIG_SHIFT)
#define BIT_SPI_IF_ONLY                     0x03
#define BIT_INT_ASY_RST_DIS_MASK            0x10

#define ICM40605_FIFO_SIZE                       1024
#define G                                        9.80665
#define PI                                       3.141592
#define KSCALE_ACC_8G_RANGE                      0.002394202f  //sensitivity m/s^2   // ACC_range * 9.81f / 65536.0f;
#define KSCALE_GYRO_2000_RANGE                   0.001065264f  // sesitivity rps // GYR_range * M_PI / (180.0f * 65536.0f);

#define ICM40605_ODR_2HZ_REG_VALUE                    (0xe)
#define ICM40605_ODR_3HZ_REG_VALUE                    (0xd)
#define ICM40605_ODR_6HZ_REG_VALUE                    (0xc)
#define ICM40605_ODR_13HZ_REG_VALUE                   (0xb)
#define ICM40605_ODR_25HZ_REG_VALUE                   (0xa)
#define ICM40605_ODR_50HZ_REG_VALUE                   (0x9)
#define ICM40605_ODR_100HZ_REG_VALUE                  (0x8)
#define ICM40605_ODR_200HZ_REG_VALUE                  (0x7)
#define ICM40605_ODR_500HZ_REG_VALUE                  (0x6)
#define ICM40605_ODR_1000HZ_REG_VALUE                 (0x6)
#define ICM40605_ODR_2000HZ_REG_VALUE                 (0x5)
#define ICM40605_ODR_4000HZ_REG_VALUE                 (0x4)
#define ICM40605_ODR_8000HZ_REG_VALUE                 (0x3)

#define max(x, y)   (x > y ? x : y)

#define AXIS_X              0
#define AXIS_Y              1
#define AXIS_Z              2
#define AXES_NUM            3

/* Support odr range 25HZ - 200HZ */
static uint32_t ICM40605HWRates[] = {
    SENSOR_HZ(25.0f),
    SENSOR_HZ(50.0f),
    SENSOR_HZ(100.0f),
    SENSOR_HZ(200.0f),
    0,
};

static uint8_t ICM40605_ODR_MAPPING[] = {
    ICM40605_ODR_25HZ_REG_VALUE,
    ICM40605_ODR_50HZ_REG_VALUE,
    ICM40605_ODR_100HZ_REG_VALUE,
    ICM40605_ODR_200HZ_REG_VALUE,
};

#define SPI_PACKET_SIZE 30
#define ICM40605_MAX_FIFO_SIZE (1040)   // Max FIFO count size for FIFO overflow case
#define SPI_BUF_SIZE    (ICM40605_MAX_FIFO_SIZE + 4)
#define EVT_SENSOR_ANY_MOTION       sensorGetMyEventType(SENS_TYPE_ANY_MOTION)

#define SPI_WRITE_0(addr, data) spiQueueWrite(addr, data, 2)
#define SPI_WRITE_1(addr, data, delay) spiQueueWrite(addr, data, delay)
#define GET_SPI_WRITE_MACRO(_1, _2, _3, NAME, ...) NAME
#define SPI_WRITE(...) GET_SPI_WRITE_MACRO(__VA_ARGS__, SPI_WRITE_1, SPI_WRITE_0)(__VA_ARGS__)

#define SPI_READ_0(addr, size, buf) spiQueueRead(addr, size, buf, 0)
#define SPI_READ_1(addr, size, buf, delay) spiQueueRead(addr, size, buf, delay)
#define GET_SPI_READ_MACRO(_1, _2, _3, _4, NAME, ...) NAME
#define SPI_READ(...) GET_SPI_READ_MACRO(__VA_ARGS__, SPI_READ_1, SPI_READ_0)(__VA_ARGS__)


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
    STATE_ACC_CALI = CHIP_ACC_CALI,
    STATE_ACC_CALI_DONE = CHIP_ACC_CALI_DONE,
    STATE_ACC_CFG = CHIP_ACC_CFG,
    STATE_ACC_CFG_DONE = CHIP_ACC_CFG_DONE,
    STATE_GYRO_ENABLE = CHIP_GYRO_ENABLE,
    STATE_GYRO_ENABLE_DONE = CHIP_GYRO_ENABLE_DONE,
    STATE_GYRO_DISABLE = CHIP_GYRO_DISABLE,
    STATE_GYRO_DISABLE_DONE = CHIP_GYRO_DISABLE_DONE,
    STATE_GYRO_RATECHG = CHIP_GYRO_RATECHG,
    STATE_GYRO_RATECHG_DONE = CHIP_GYRO_RATECHG_DONE,
    STATE_GYRO_CALI = CHIP_GYRO_CALI,
    STATE_GYRO_CALI_DONE = CHIP_GYRO_CALI_DONE,
    STATE_GYRO_CFG = CHIP_GYRO_CFG,
    STATE_GYRO_CFG_DONE = CHIP_GYRO_CFG_DONE,
    STATE_ANYMO_ENABLE = CHIP_ANYMO_ENABLE,
    STATE_ANYMO_ENABLE_DONE = CHIP_ANYMO_ENABLE_DONE,
    STATE_ANYMO_DISABLE = CHIP_ANYMO_DISABLE,
    STATE_ANYMO_DISABLE_DONE = CHIP_ANYMO_DISABLE_DONE,

    STATE_HW_INT_STATUS_CHECK = CHIP_HW_INT_STATUS_CHECK,
    STATE_HW_INT_HANDLING = CHIP_HW_INT_HANDLING,
    STATE_HW_INT_HANDLING_DONE = CHIP_HW_INT_HANDLING_DONE,
    STATE_INIT_DONE = CHIP_INIT_DONE,
    STATE_IDLE = CHIP_IDLE,
    STATE_SW_RESET = CHIP_RESET,
    STATE_SW_RESET_W,
    STATE_RESET_CHECK,
    STATE_INT_STATUS,
    STATE_INIT_REG,
    STATE_SENSOR_REGISTRATION,
    STATE_EINT_REGISTRATION,
};

enum SensorIndex {
    ACC = 0,
    GYR,
    ANYMO,
    NUM_OF_SENSOR,
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

struct ICM40605Sensor {
    float staticCali[AXES_NUM];
    uint64_t latency;
    int32_t accuracy;
    uint32_t rate;
    uint32_t hwRate;
    uint32_t preRealRate;
    uint32_t handle;
    float sensitivity;
    bool powered;
    bool configed;
    bool startCali;
    bool needDiscardSample;
    uint32_t samplesToDiscard;
};

static struct ICM40605Task {
    struct ICM40605Sensor sensors[NUM_OF_SENSOR];

    uint64_t hwSampleTime;
    uint64_t swSampleTime;
    uint64_t unmask_eint_time;

    uint64_t lastSampleTime;
    uint8_t *regBuffer;
    uint8_t *statusBuffer;
    uint8_t *wakeupBuffer;

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
    int32_t debug_trace;
    uint32_t fifoDataToRead;

    int latch_time_id;
    uint16_t watermark;
    /* For save reg status */
    uint8_t int_src0;
    uint8_t int_src1;
    uint8_t smd_cfg;
    uint8_t pwr_sta;
    uint8_t acc_cfg0;
    uint8_t gyro_cfg0;
    uint8_t fifo_cfg1;
} mTask;
static struct ICM40605Task *ICM40605DebugPoint;

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

static int ICM40605CalcuOdr(uint32_t *rate, uint32_t *report_rate)
{
    int i;

    for (i = 0; i < (ARRAY_SIZE(ICM40605HWRates) - 1); i++) {
        if (*rate <= ICM40605HWRates[i]) {
            *report_rate = ICM40605HWRates[i];
            break;
        }
    }

    if (*rate > ICM40605HWRates[(ARRAY_SIZE(ICM40605HWRates) - 2)]) {
        i = (ARRAY_SIZE(ICM40605HWRates) - 2);
        *report_rate = ICM40605HWRates[i];
    }

    return i;
}

void ICM40605TimerCbkF(void)
{
    mTask.hwSampleTime = rtcGetTime();
}

static int ICM40605ResetWrite(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    uint8_t val = 0;

    osLog(LOG_INFO, "ICM40605ResetWrite 0x%x\n", mTask.regBuffer[1]);
    val = mTask.regBuffer[1] | BIT_SOFT_RESET;
    /* Wait 100ms to check reset status */
    SPI_WRITE(ICM40605_REG_DEV_CFG, val, 100000);
    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
}
static int ICM40605IntStatus(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    SPI_READ(ICM40605_REG_INT_STATUS, 1, &mTask.regBuffer);
    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
}

static int ICM40605ResetCheck(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    if (0x10 & mTask.regBuffer[1])
        osLog(LOG_INFO, "ICM40605ResetCheck Done\n");
    else
        osLog(LOG_ERROR, "ICM40605ResetCheck Fail 0x%x\n", mTask.regBuffer[1]);
    sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int ICM40605ResetRead(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    SPI_READ(ICM40605_REG_DEV_CFG, 1, &mTask.regBuffer);
    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
}

static int ICM40605InitConfig(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    osLog(LOG_INFO, "ICM40605InitConfig\n");

    /* set INT push-pull */
    SPI_WRITE(ICM40605_REG_INT_CFG, 0xb);
    /* gyro odr & fs */
    mTask.gyro_cfg0 = BIT_GYRO_ODR_25 | BIT_GYRO_FS_2000;
    SPI_WRITE(ICM40605_REG_GYRO_CFG, mTask.gyro_cfg0);
    /* acc odr & fs */
    mTask.acc_cfg0 =  (BIT_ACC_ODR_25 | BIT_ACC_FS_8G);
    SPI_WRITE(ICM40605_REG_ACC_CFG, mTask.acc_cfg0);
    /* acc & gyro BW */
    SPI_WRITE(ICM40605_REG_BW_CFG, BIT_ACC_BW_DIV_2 | BIT_GYRO_BW_DIV_2);
    /* en byte mode & little endian mode & disable i2c interface */
    SPI_WRITE(ICM40605_REG_INTF_CFG0, BIT_SPI_IF_ONLY);
    /* en fifo cfg(always 16bytes mode) */
    SPI_WRITE(ICM40605_REG_FIFO_CONFIG_1, 0x2F);
    /* set wm_th=16 as default for byte mode */
    SPI_WRITE(ICM40605_REG_FIFO_WM_TH_L, 0x10);/* byte mode */
    SPI_WRITE(ICM40605_REG_FIFO_WM_TH_H, 0x00);
    /* bypass fifo */
    SPI_WRITE(ICM40605_REG_FIFO_CFG, BIT_FIFO_BYPASS_MODE);
    /* enable fifo full & WM INT */
    mTask.int_src0 |= (BIT_FIFO_FULL_EN_MASK | BIT_WM_INT_EN_MASK);
    SPI_WRITE(ICM40605_REG_INT_SOURCE0, mTask.int_src0);

    /* async reset */
    SPI_WRITE(ICM40605_REG_INT_CFG1, BIT_INT_ASY_RST_DIS_MASK);

    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
}

static void ICM40605ConfigFifo(bool odr_change)
{
    uint8_t  buffer[2];
    uint16_t watermarkReg;

    watermarkReg = mTask.watermark;

#if 1//byte mode
    if (watermarkReg < 16)
        watermarkReg = 16;
#else
    if (watermarkReg == 0)
        watermarkReg = 1;
#endif

    buffer[0] = watermarkReg & 0x00ff;
    buffer[1] = (watermarkReg & 0xff00) >> 8;

    if (odr_change) {
        /* set bypass mode to reset fifo */
        SPI_WRITE(ICM40605_REG_FIFO_CFG, BIT_FIFO_BYPASS_MODE);
        /* set threshold */
        SPI_WRITE(ICM40605_REG_FIFO_WM_TH_L, buffer[0]);
        SPI_WRITE(ICM40605_REG_FIFO_WM_TH_H, buffer[1]);
        if (mTask.sensors[ACC].powered || mTask.sensors[GYR].powered) {
            /* set fifo stream mode */
            SPI_WRITE(ICM40605_REG_FIFO_CFG, BIT_FIFO_STREAM_MODE);
        }
        osLog(LOG_INFO, "ICM40605ConfigFifo: Reset, TH_L:0x%x, TH_H:0x%x\n",
            buffer[0], buffer[1]);
    } else {
        SPI_WRITE(ICM40605_REG_FIFO_WM_TH_L, buffer[0]);
        SPI_WRITE(ICM40605_REG_FIFO_WM_TH_H, buffer[1]);
        if (mTask.sensors[ACC].powered || mTask.sensors[GYR].powered) {
            SPI_WRITE(ICM40605_REG_FIFO_CFG, BIT_FIFO_STREAM_MODE);
        } else {
            SPI_WRITE(ICM40605_REG_FIFO_CFG, BIT_FIFO_BYPASS_MODE);
        }
        osLog(LOG_INFO, "ICM40605ConfigFifo, TH_L:0x%x, TH_H:0x%x\n",
            buffer[0], buffer[1]);
    }
}

static uint16_t ICM40605CalcuWm(void)
{
    uint8_t handle;
    uint64_t min_latency = SENSOR_LATENCY_NODATA;
    uint8_t min_watermark = 1;
    uint8_t max_watermark = 30;
    uint16_t watermark = 0;
    uint32_t temp_cnt, total_cnt = 0;
    uint32_t temp_delay = 0;

    for (handle = ACC; handle <= GYR; handle++) {
        if (mTask.sensors[handle].hwRate && mTask.sensors[handle].latency != SENSOR_LATENCY_NODATA) {
            min_latency =
                mTask.sensors[handle].latency < min_latency ? mTask.sensors[handle].latency : min_latency;
        }
    }
    /*if acc and gyr off or acc and gyr latency = SENSOR_LATENCY_NODATA, watermark = 0 or 1 or 2*/
    if (min_latency == SENSOR_LATENCY_NODATA) {
        for (handle = ACC; handle <= GYR; handle++) {
            if (mTask.sensors[handle].hwRate) {
                watermark++;
            }
        }
    } else {
        for (handle = ACC; handle <= GYR; handle++) {
            if (mTask.sensors[handle].hwRate) {
                temp_delay = (1000000000ULL / mTask.sensors[handle].hwRate) << 10;
                temp_cnt = min_latency / temp_delay;
                min_watermark = mTask.sensors[handle].hwRate / SENSOR_HZ(400.0f);
                total_cnt = temp_cnt > min_watermark ? temp_cnt : min_watermark;
                osLog(LOG_INFO, "ICM40605CalcuWm, delay=%d, latency:%lld, min_wm=%d, total_cnt=%d\n",
                      temp_delay, min_latency, min_watermark, total_cnt);
            }
        }

        watermark = total_cnt;
        watermark = watermark < min_watermark ? min_watermark : watermark;
        watermark = watermark > max_watermark ? max_watermark : watermark;
    }

    return watermark * 16;/* byte mode */
}

static int ICM40605gEnable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                           void *inBuf, uint8_t inSize, uint8_t elemInSize,
                           void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;

    osLog(LOG_INFO, "ICM40605gEnable power mode=%d, time:%lld\n", mTask.sensors[ACC].powered, rtcGetTime());

    mTask.sensors[ACC].powered = true;
    mTask.pwr_sta &= 0xFC;
    mTask.pwr_sta |= BIT_ACC_LN_MODE;
    // set 10ms for accel start-up time, note:at least 200us here to sync fly changes
    SPI_WRITE(ICM40605_REG_PWR_MGMT0, mTask.pwr_sta, 10000);
    ret = spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);

    return ret;
}

static int ICM40605gDisable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                            void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    int odr = 0;
    uint8_t regValue = 0x00;
    uint32_t sampleRate = 0;
    struct accGyroCntlPacket cntlPacket;

    osLog(LOG_INFO, "ICM40605AccPowerOff\n");

    ret = rxControlInfo(&cntlPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "ICM40605AccPowerOff, rx inSize and elemSize error\n");
        return -1;
    }

    mTask.sensors[ACC].rate = 0;
    mTask.sensors[ACC].preRealRate = 0;
    mTask.sensors[ACC].hwRate = 0;
    mTask.sensors[ACC].latency = SENSOR_LATENCY_NODATA;
    mTask.sensors[ACC].needDiscardSample = false;
    mTask.sensors[ACC].samplesToDiscard = 0;

    if ((mTask.sensors[GYR].powered == false) && (mTask.sensors[ANYMO].powered == false)) {
        mt_eint_mask(mTask.hw->eint_num);
        mTask.pwr_sta &= 0xF0;
        SPI_WRITE(ICM40605_REG_PWR_MGMT0, mTask.pwr_sta, 200);
    } else if (mTask.sensors[GYR].powered == true) {  //  update gyro old ord
        if (mTask.sensors[GYR].hwRate != mTask.sensors[GYR].preRealRate) {
            mTask.sensors[GYR].hwRate = mTask.sensors[GYR].preRealRate;
            odr = ICM40605CalcuOdr(&mTask.sensors[GYR].hwRate, &sampleRate);

            regValue = ICM40605_ODR_MAPPING[odr];
            mTask.gyro_cfg0 = (mTask.gyro_cfg0 & 0xF0) | regValue;
            SPI_WRITE(ICM40605_REG_GYRO_CFG, mTask.gyro_cfg0);
            mTask.acc_cfg0 = (mTask.acc_cfg0 & 0xF0) | regValue;
            SPI_WRITE(ICM40605_REG_ACC_CFG, mTask.acc_cfg0);
            osLog(LOG_INFO, "gyro change rate to preRealRate:%d\n", mTask.sensors[GYR].hwRate);
        }
    }

    registerAccGyroFifoInfo((mTask.sensors[ACC].hwRate == 0) ? 0 : 1024000000000 / mTask.sensors[ACC].hwRate,
        (mTask.sensors[GYR].hwRate == 0) ? 0 : 1024000000000 / mTask.sensors[GYR].hwRate);

    mTask.watermark = ICM40605CalcuWm();
    mTask.sensors[ACC].powered = false;
    mTask.sensors[ACC].configed = false;

    ICM40605ConfigFifo(true);
#if 0
    if (mTask.sensors[GYR].powered == true) {
        mTask.fifo_cfg1 |= 0x2F;
        SPI_WRITE(ICM40605_REG_FIFO_CONFIG_1, mTask.fifo_cfg1);
    }
#endif

    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
}

static int ICM40605gRate(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    int odr = 0;
    uint8_t regValue = 0x00;
    uint32_t sampleRate = 0;
    uint32_t maxRate = 0;
    bool accelOdrChanged = false;
    struct accGyroCntlPacket cntlPacket;

    ret = rxControlInfo(&cntlPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "ICM40605gRate, rx inSize and elemSize error\n");
        return -1;
    }

    mTask.sensors[ACC].rate = cntlPacket.rate;
    mTask.sensors[ACC].latency = cntlPacket.latency;

    if (0 == mTask.sensors[ACC].preRealRate) {
      mTask.sensors[ACC].needDiscardSample = true;
      mTask.sensors[ACC].samplesToDiscard = 3;
    }

    odr = ICM40605CalcuOdr(&mTask.sensors[ACC].rate, &sampleRate);
    mTask.sensors[ACC].preRealRate = sampleRate;

    /*if gyr configed ,compare maxRate with acc and gyr rate*/
    if (mTask.sensors[GYR].configed) {
        maxRate = max(sampleRate, mTask.sensors[GYR].preRealRate);   // choose with preRealRate
        if ((maxRate != mTask.sensors[ACC].hwRate) || (maxRate != mTask.sensors[GYR].hwRate)) {
            mTask.sensors[ACC].hwRate = maxRate;
            mTask.sensors[GYR].hwRate = maxRate;
            odr = ICM40605CalcuOdr(&maxRate, &sampleRate);
            regValue = ICM40605_ODR_MAPPING[odr];
            mTask.gyro_cfg0 = (mTask.gyro_cfg0 & 0xF0) | regValue;
            SPI_WRITE(ICM40605_REG_GYRO_CFG, mTask.gyro_cfg0);
            mTask.acc_cfg0 = (mTask.acc_cfg0 & 0xF0) | regValue;
            SPI_WRITE(ICM40605_REG_ACC_CFG, mTask.acc_cfg0);
            accelOdrChanged = true;
         } else {
            accelOdrChanged = false;
         }
    } else {
        if ((sampleRate != mTask.sensors[ACC].hwRate)) {
            mTask.sensors[ACC].hwRate = sampleRate;
            regValue = ICM40605_ODR_MAPPING[odr];
            mTask.gyro_cfg0 = (mTask.gyro_cfg0 & 0xF0) | regValue;
            SPI_WRITE(ICM40605_REG_GYRO_CFG, mTask.gyro_cfg0);
            mTask.acc_cfg0 = (mTask.acc_cfg0 & 0xF0) | regValue;
            SPI_WRITE(ICM40605_REG_ACC_CFG, mTask.acc_cfg0);
            accelOdrChanged = true;
        } else {
            accelOdrChanged = false;
        }
    }

    registerAccGyroFifoInfo((mTask.sensors[ACC].hwRate == 0) ? 0 : 1024000000000 / mTask.sensors[ACC].hwRate,
                            (mTask.sensors[GYR].hwRate == 0) ? 0 : 1024000000000 / mTask.sensors[GYR].hwRate);
    mTask.sensors[ACC].configed = true;

    mTask.watermark = ICM40605CalcuWm();

    ICM40605ConfigFifo(accelOdrChanged);

    mt_eint_unmask(mTask.hw->eint_num);
    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
}

static int ICM40605gyEnable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                            void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;

    osLog(LOG_INFO, "ICM40605gyEnable power mode=%d\n", mTask.sensors[GYR].powered);

    mTask.sensors[GYR].powered = true;
    mTask.pwr_sta &= 0xF0;
    mTask.pwr_sta |= (BIT_GYRO_LN_MODE | BIT_ACC_LN_MODE);
    // set 50ms for gyro start-up time, note:at least 200us here to sync fly changes
    SPI_WRITE(ICM40605_REG_PWR_MGMT0, mTask.pwr_sta, 50000);
    ret = spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);

    return ret;
}

static int ICM40605gyDisable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    int odr = 0;
    uint8_t regValue = 0x00;
    uint32_t sampleRate = 0;
    struct accGyroCntlPacket cntlPacket;
    osLog(LOG_INFO, "ICM40605gyDisable\n");

    ret = rxControlInfo(&cntlPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "ICM40605AccPowerOff, rx inSize and elemSize error\n");
        return -1;
    }

    mTask.sensors[GYR].rate = 0;
    mTask.sensors[GYR].preRealRate = 0;
    mTask.sensors[GYR].hwRate = 0;
    mTask.sensors[GYR].latency = SENSOR_LATENCY_NODATA;
    mTask.sensors[GYR].needDiscardSample = false;
    mTask.sensors[GYR].samplesToDiscard = 0;

    /* enter off mode */
    if ((mTask.sensors[ACC].powered == false) && (mTask.sensors[ANYMO].powered == false)) {
        mt_eint_mask(mTask.hw->eint_num);
        mTask.pwr_sta &= 0xF0;
        SPI_WRITE(ICM40605_REG_PWR_MGMT0, mTask.pwr_sta, 200);
    } else if (mTask.sensors[ACC].powered == true) {  //  update ACC old ord
        if (mTask.sensors[ACC].hwRate != mTask.sensors[ACC].preRealRate) {
            mTask.sensors[ACC].hwRate = mTask.sensors[ACC].preRealRate;
            odr = ICM40605CalcuOdr(&mTask.sensors[ACC].hwRate, &sampleRate);
            regValue = ICM40605_ODR_MAPPING[odr];
            mTask.gyro_cfg0 = (mTask.gyro_cfg0 & 0xF0) | regValue;
            SPI_WRITE(ICM40605_REG_GYRO_CFG, mTask.gyro_cfg0);
            mTask.acc_cfg0 = (mTask.acc_cfg0 & 0xF0) | regValue;
            SPI_WRITE(ICM40605_REG_ACC_CFG, mTask.acc_cfg0);
            osLog(LOG_INFO, "acc change rate to preRealRate:%d\n", mTask.sensors[ACC].hwRate);
        }
        mTask.pwr_sta &= 0xF3;  // Gyro OFF
        SPI_WRITE(ICM40605_REG_PWR_MGMT0, mTask.pwr_sta, 200);
    } else if (mTask.sensors[ANYMO].powered == true) {
        mTask.pwr_sta &= 0xF3;  // Gyro OFF
        SPI_WRITE(ICM40605_REG_PWR_MGMT0, mTask.pwr_sta, 200);
    }

    registerAccGyroFifoInfo((mTask.sensors[ACC].hwRate == 0) ? 0 : 1024000000000 / mTask.sensors[ACC].hwRate,
        (mTask.sensors[GYR].hwRate == 0) ? 0 : 1024000000000 / mTask.sensors[GYR].hwRate);

    mTask.watermark = ICM40605CalcuWm();
    mTask.sensors[GYR].powered = false;
    mTask.sensors[GYR].configed = false;

    ICM40605ConfigFifo(true);
#if 0
    if (mTask.sensors[ACC].powered == true) {
        /* enable fifo INT */
        mTask.fifo_cfg1 |= 0x2F;
        SPI_WRITE(ICM40605_REG_FIFO_CONFIG_1, mTask.fifo_cfg1);
    }
#endif

    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
}

static int ICM40605gyRate(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                          void *inBuf, uint8_t inSize, uint8_t elemInSize,
                          void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    int odr = 0;
    uint8_t regValue = 0x00;
    uint32_t sampleRate = 0;
    uint32_t maxRate = 0;
    bool gyroOdrChanged = false;
    struct accGyroCntlPacket cntlPacket;

    ret = rxControlInfo(&cntlPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "ICM40605gRate, rx inSize and elemSize error\n");
        return -1;
    }

    mTask.sensors[GYR].rate = cntlPacket.rate;
    mTask.sensors[GYR].latency = cntlPacket.latency;

    if (0 == mTask.sensors[GYR].preRealRate) {
      mTask.sensors[GYR].needDiscardSample = true;
      mTask.sensors[GYR].samplesToDiscard = 3;
    }

    /*  get hw sample rate */
    odr = ICM40605CalcuOdr(&mTask.sensors[GYR].rate, &sampleRate);

    mTask.sensors[GYR].preRealRate = sampleRate;

    /*if gyr configed ,compare maxRate with acc and gyr rate*/
    if (mTask.sensors[ACC].configed) {
        maxRate = max(sampleRate, mTask.sensors[ACC].preRealRate);
        if ((maxRate != mTask.sensors[ACC].hwRate) || (maxRate != mTask.sensors[GYR].hwRate)) {
            mTask.sensors[ACC].hwRate = maxRate;
            mTask.sensors[GYR].hwRate = maxRate;
            /* update new odr */
            odr = ICM40605CalcuOdr(&maxRate, &sampleRate);

            regValue = ICM40605_ODR_MAPPING[odr];
            mTask.gyro_cfg0 = (mTask.gyro_cfg0 & 0xF0) | regValue;
            SPI_WRITE(ICM40605_REG_GYRO_CFG, mTask.gyro_cfg0);
            mTask.acc_cfg0 = (mTask.acc_cfg0 & 0xF0) | regValue;
            SPI_WRITE(ICM40605_REG_ACC_CFG, mTask.acc_cfg0);
            osLog(LOG_INFO, "ICM40605gyRate: ACC config\n");
            gyroOdrChanged = true;
        } else {
            gyroOdrChanged = false;
        }
    } else {
        if ((sampleRate != mTask.sensors[GYR].hwRate)) {
            mTask.sensors[GYR].hwRate = sampleRate;
            regValue = ICM40605_ODR_MAPPING[odr];
            /* update new odr */
            mTask.gyro_cfg0 = (mTask.gyro_cfg0 & 0xF0) | regValue;
            SPI_WRITE(ICM40605_REG_GYRO_CFG, mTask.gyro_cfg0);
            mTask.acc_cfg0 = (mTask.acc_cfg0 & 0xF0) | regValue;
            SPI_WRITE(ICM40605_REG_ACC_CFG, mTask.acc_cfg0);
            gyroOdrChanged = true;
        } else {
            gyroOdrChanged = false;
        }
    }

    registerAccGyroFifoInfo((mTask.sensors[ACC].hwRate == 0) ? 0 : 1024000000000 / mTask.sensors[ACC].hwRate,
                            (mTask.sensors[GYR].hwRate == 0) ? 0 : 1024000000000 / mTask.sensors[GYR].hwRate);
    mTask.sensors[GYR].configed = true;

    /* watermark update */
    mTask.watermark = ICM40605CalcuWm();
    ICM40605ConfigFifo(gyroOdrChanged);

    mt_eint_unmask(mTask.hw->eint_num);
    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
}

static int ICM40605AccCali(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                           void *inBuf, uint8_t inSize, uint8_t elemInSize,
                           void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    float bias[AXES_NUM] = {0};
    mTask.sensors[ACC].startCali = true;

    osLog(LOG_INFO, "ICM40605AccCali\n");
    Acc_init_calibration(bias);

    sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int ICM40605GyroCali(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                            void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    float slope[AXES_NUM] = {0};
    float intercept[AXES_NUM] = {0};
    mTask.sensors[GYR].startCali = true;

    osLog(LOG_INFO, "ICM40605GyroCali\n");
    Gyro_init_calibration(slope, intercept);

    sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int ICM40605AccCfgCali(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    struct accGyroCaliCfgPacket caliCfgPacket;

    ret = rxCaliCfgInfo(&caliCfgPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);

    if (ret < 0) {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "accHwCaliCheck, rx inSize and elemSize error\n");
        return -1;
    }
    osLog(LOG_INFO, "ICM40605AccCfgCali: cfgData[0]:%d, cfgData[1]:%d, cfgData[2]:%d\n",
          caliCfgPacket.caliCfgData[0], caliCfgPacket.caliCfgData[1], caliCfgPacket.caliCfgData[2]);

    mTask.sensors[ACC].staticCali[0] = (float)caliCfgPacket.caliCfgData[0] / 1000;
    mTask.sensors[ACC].staticCali[1] = (float)caliCfgPacket.caliCfgData[1] / 1000;
    mTask.sensors[ACC].staticCali[2] = (float)caliCfgPacket.caliCfgData[2] / 1000;

    sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int ICM40605GyroCfgCali(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                               void *inBuf, uint8_t inSize, uint8_t elemInSize,
                               void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    struct accGyroCaliCfgPacket caliCfgPacket;

    ret = rxCaliCfgInfo(&caliCfgPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);

    if (ret < 0) {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "gyroHwCaliCheck, rx inSize and elemSize error\n");
        return -1;
    }

    osLog(LOG_INFO, "ICM40605GyroCfgCali: cfgData[0]:%d, cfgData[1]:%d, cfgData[2]:%d\n",
          caliCfgPacket.caliCfgData[0], caliCfgPacket.caliCfgData[1], caliCfgPacket.caliCfgData[2]);

    mTask.sensors[GYR].staticCali[0] = (float)caliCfgPacket.caliCfgData[0] / 1000;
    mTask.sensors[GYR].staticCali[1] = (float)caliCfgPacket.caliCfgData[1] / 1000;
    mTask.sensors[GYR].staticCali[2] = (float)caliCfgPacket.caliCfgData[2] / 1000;

    sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static void accGetCalibration(int32_t *cali, int32_t size)
{
    cali[AXIS_X]  = mTask.accHwCali[AXIS_X];
    cali[AXIS_Y]  = mTask.accHwCali[AXIS_Y];
    cali[AXIS_Z]  = mTask.accHwCali[AXIS_Z];
    //  osLog(LOG_INFO, "accGetCalibration cali x:%d, y:%d, z:%d\n", cali[AXIS_X], cali[AXIS_Y], cali[AXIS_Z]);
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

    osLog(LOG_INFO, "accGetData x:%d, y:%d, z:%d\n", tripleSample->ix, tripleSample->iy, tripleSample->iz);
}
static void gyroGetCalibration(int32_t *cali, int32_t size)
{
    cali[AXIS_X] = mTask.gyroHwCali[AXIS_X];
    cali[AXIS_Y] = mTask.gyroHwCali[AXIS_Y];
    cali[AXIS_Z] = mTask.gyroHwCali[AXIS_Z];
    //  osLog(LOG_INFO, "gyroGetCalibration cali x:%d, y:%d, z:%d\n", cali[AXIS_X], cali[AXIS_Y], cali[AXIS_Z]);
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

    osLog(LOG_INFO, "gyroGetData x:%d, y:%d, z:%d\n", tripleSample->ix, tripleSample->iy, tripleSample->iz);
}

static void spiIsrCallBack(void *cookie, int err)
{
    if (err != 0) {
        osLog(LOG_ERROR, "lsm6dsm: spiIsrCallBack err\n");
        sensorFsmEnqueueFakeSpiEvt(mTask.spiCallBack, cookie, ERROR_EVT);
    } else {
        mTask.swSampleTime = rtcGetTime();
        sensorFsmEnqueueFakeSpiEvt(mTask.spiCallBack, cookie, SUCCESS_EVT);
    }
}

static int ICM40605Sample(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                          void *inBuf, uint8_t inSize, uint8_t elemInSize,
                          void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;

    ret = rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "ICM40605Sample, rx dataInfo error\n");
        return -1;
    }

    SPI_READ(ICM40605_REG_FIFO_BYTE_COUNT_L, 2, &mTask.statusBuffer);
    mTask.spiCallBack = spiCallBack;

    return spiBatchTxRx(&mTask.mode, spiIsrCallBack, next_state, __FUNCTION__);
}

static int ICM40605ReadFifo(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                            void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.fifoDataToRead = (mTask.statusBuffer[2] << 8) | mTask.statusBuffer[1];
    /* covert record num to bytes */
    //mTask.fifoDataToRead *= 16; /* N/A for byte mode */
    //osLog(LOG_ERROR, "ICM40605ReadFifo :mTask.fifoDataToRead = %d\n", mTask.fifoDataToRead);
    if (mTask.fifoDataToRead <= 0 || mTask.fifoDataToRead > ICM40605_MAX_FIFO_SIZE) {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
        return 0;
    }
    //mTask.fifoDataToRead += 1; // read one more byte to eliminate double interrupt

    SPI_READ(ICM40605_REG_FIFO_DATA, mTask.fifoDataToRead, &mTask.regBuffer);
    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
}

static void parseRawData(struct accGyroData *data, uint8_t *buf, uint8_t sensorType)
{
    int16_t raw_data[AXES_NUM] = {0};
    int16_t remap_data[AXES_NUM] = {0};
    int32_t SwCali[AXES_NUM] = {0};

    int32_t caliResult[AXES_NUM] = {0};
    float temp_data[AXES_NUM] = {0};
    float calibrated_data_output[AXES_NUM] = {0};
    int32_t delta_time = 0;
    int16_t status = 0;

    if (sensorType == SENS_TYPE_ACCEL)
        accGetCalibration(SwCali, 0);
    else if (sensorType == SENS_TYPE_GYRO)
        gyroGetCalibration(SwCali, 0);
    /* Use little endian mode */
    raw_data[AXIS_X] = (buf[0] | buf[1] << 8);
    raw_data[AXIS_Y] = (buf[2] | buf[3] << 8);
    raw_data[AXIS_Z] = (buf[4] | buf[5] << 8);

    //if (sensorType == SENS_TYPE_GYRO) {
    //    osLog(LOG_ERROR, "GYRO: %d %d %d\n", raw_data[AXIS_X], raw_data[AXIS_Y], raw_data[AXIS_Z]);
    //}

    raw_data[AXIS_X] = raw_data[AXIS_X] + SwCali[AXIS_X];
    raw_data[AXIS_Y] = raw_data[AXIS_Y] + SwCali[AXIS_Y];
    raw_data[AXIS_Z] = raw_data[AXIS_Z] + SwCali[AXIS_Z];

    remap_data[mTask.cvt.map[AXIS_X]] = mTask.cvt.sign[AXIS_X] * raw_data[AXIS_X];
    remap_data[mTask.cvt.map[AXIS_Y]] = mTask.cvt.sign[AXIS_Y] * raw_data[AXIS_Y];
    remap_data[mTask.cvt.map[AXIS_Z]] = mTask.cvt.sign[AXIS_Z] * raw_data[AXIS_Z];

    if (sensorType == SENS_TYPE_ACCEL) {
        temp_data[AXIS_X] = (float)remap_data[AXIS_X] * KSCALE_ACC_8G_RANGE;
        temp_data[AXIS_Y] = (float)remap_data[AXIS_Y] * KSCALE_ACC_8G_RANGE;
        temp_data[AXIS_Z] = (float)remap_data[AXIS_Z] * KSCALE_ACC_8G_RANGE;

        if (UNLIKELY(mTask.sensors[ACC].startCali)) {
            status = Acc_run_factory_calibration_timeout(delta_time,
                     temp_data, calibrated_data_output, (int *)&mTask.sensors[ACC].accuracy, rtcGetTime());
            if (status != 0) {
                mTask.sensors[ACC].startCali = false;
                if (status > 0) {
                    osLog(LOG_INFO, "ACC cali detect shake\n");
                    caliResult[AXIS_X] = (int32_t)(mTask.sensors[ACC].staticCali[AXIS_X] * 1000);
                    caliResult[AXIS_Y] = (int32_t)(mTask.sensors[ACC].staticCali[AXIS_Y] * 1000);
                    caliResult[AXIS_Z] = (int32_t)(mTask.sensors[ACC].staticCali[AXIS_Z] * 1000);
                    accGyroSendCalibrationResult(SENS_TYPE_ACCEL, (int32_t *)&caliResult[0], (uint8_t)status);
                } else
                    osLog(LOG_INFO, "ACC cali time out\n");
            } else if (mTask.sensors[ACC].accuracy == 3) {
                mTask.sensors[ACC].startCali = false;
                mTask.sensors[ACC].staticCali[AXIS_X] = calibrated_data_output[AXIS_X] - temp_data[AXIS_X];
                mTask.sensors[ACC].staticCali[AXIS_Y] = calibrated_data_output[AXIS_Y] - temp_data[AXIS_Y];
                mTask.sensors[ACC].staticCali[AXIS_Z] = calibrated_data_output[AXIS_Z] - temp_data[AXIS_Z];
                caliResult[AXIS_X] = (int32_t)(mTask.sensors[ACC].staticCali[AXIS_X] * 1000);
                caliResult[AXIS_Y] = (int32_t)(mTask.sensors[ACC].staticCali[AXIS_Y] * 1000);
                caliResult[AXIS_Z] = (int32_t)(mTask.sensors[ACC].staticCali[AXIS_Z] * 1000);
                accGyroSendCalibrationResult(SENS_TYPE_ACCEL, (int32_t *)&caliResult[0], (uint8_t)status);
                /*osLog(LOG_INFO, "ACCEL cali done:caliResult[0]:%d, caliResult[1]:%d, caliResult[2]:%d,
                               offset[0]:%f, offset[1]:%f, offset[2]:%f\n",
                               caliResult[AXIS_X], caliResult[AXIS_Y], caliResult[AXIS_Z],
                               (double)mTask.sensors[ACC].staticCali[AXIS_X],
                               (double)mTask.sensors[ACC].staticCali[AXIS_Y],
                               (double)mTask.sensors[ACC].staticCali[AXIS_Z]);*/
            }
        }

        data->sensType = sensorType;

        data->x = temp_data[AXIS_X] + mTask.sensors[ACC].staticCali[AXIS_X];
        data->y = temp_data[AXIS_Y] + mTask.sensors[ACC].staticCali[AXIS_Y];
        data->z = temp_data[AXIS_Z] + mTask.sensors[ACC].staticCali[AXIS_Z];

        //Need "m/s^2 * 1000" for factory calibration program
        //Need LSB in cali program = mTask.accFactoryData.ix * mInfo.sensitivity / mInfo.gain;
        mTask.accFactoryData.ix = (int32_t)(data->x * ACCELEROMETER_INCREASE_NUM_AP);
        mTask.accFactoryData.iy = (int32_t)(data->y * ACCELEROMETER_INCREASE_NUM_AP);
        mTask.accFactoryData.iz = (int32_t)(data->z * ACCELEROMETER_INCREASE_NUM_AP);

    } else if (sensorType == SENS_TYPE_GYRO) {
        temp_data[AXIS_X] = (float)remap_data[AXIS_X] * KSCALE_GYRO_2000_RANGE;
        temp_data[AXIS_Y] = (float)remap_data[AXIS_Y] * KSCALE_GYRO_2000_RANGE;
        temp_data[AXIS_Z] = (float)remap_data[AXIS_Z] * KSCALE_GYRO_2000_RANGE;

        if (UNLIKELY(mTask.sensors[GYR].startCali)) {
            status = Gyro_run_factory_calibration_timeout(delta_time,
                     temp_data, calibrated_data_output, (int *)&mTask.sensors[GYR].accuracy, 0, rtcGetTime());
            if (status != 0) {
                mTask.sensors[GYR].startCali = false;
                if (status > 0) {
                    osLog(LOG_INFO, "GYRO cali detect shake\n");
                    caliResult[AXIS_X] = (int32_t)(mTask.sensors[GYR].staticCali[AXIS_X] * 1000);
                    caliResult[AXIS_Y] = (int32_t)(mTask.sensors[GYR].staticCali[AXIS_Y] * 1000);
                    caliResult[AXIS_Z] = (int32_t)(mTask.sensors[GYR].staticCali[AXIS_Z] * 1000);
                    accGyroSendCalibrationResult(SENS_TYPE_GYRO, (int32_t *)&caliResult[0], (uint8_t)status);
                } else
                    osLog(LOG_INFO, "GYRO cali time out\n");
            } else if (mTask.sensors[GYR].accuracy == 3) {
                mTask.sensors[GYR].startCali = false;
                mTask.sensors[GYR].staticCali[AXIS_X] = calibrated_data_output[AXIS_X] - temp_data[AXIS_X];
                mTask.sensors[GYR].staticCali[AXIS_Y] = calibrated_data_output[AXIS_Y] - temp_data[AXIS_Y];
                mTask.sensors[GYR].staticCali[AXIS_Z] = calibrated_data_output[AXIS_Z] - temp_data[AXIS_Z];
                caliResult[AXIS_X] = (int32_t)(mTask.sensors[GYR].staticCali[AXIS_X] * 1000);
                caliResult[AXIS_Y] = (int32_t)(mTask.sensors[GYR].staticCali[AXIS_Y] * 1000);
                caliResult[AXIS_Z] = (int32_t)(mTask.sensors[GYR].staticCali[AXIS_Z] * 1000);
                accGyroSendCalibrationResult(SENS_TYPE_GYRO, (int32_t *)&caliResult[0], (uint8_t)status);
                /* osLog(LOG_INFO, "GYRO cali done: caliResult[0]:%d, caliResult[1]:%d, caliResult[2]:%d,
                        offset[0]:%f, offset[1]:%f, offset[2]:%f\n",
                                       caliResult[AXIS_X], caliResult[AXIS_Y], caliResult[AXIS_Z],
                                  (double)mTask.sensors[GYR].staticCali[AXIS_X],
                                  (double)mTask.sensors[GYR].staticCali[AXIS_Y],
                                  (double)mTask.sensors[GYR].staticCali[AXIS_Z]); */
            }
        }

        data->sensType = sensorType;

        data->x = temp_data[AXIS_X] + mTask.sensors[GYR].staticCali[AXIS_X];
        data->y = temp_data[AXIS_Y] + mTask.sensors[GYR].staticCali[AXIS_Y];
        data->z = temp_data[AXIS_Z] + mTask.sensors[GYR].staticCali[AXIS_Z];

        //Need "dps*GYROSCOPE_INCREASE_NUM_AP" for factory calibration program
        //Need LSB in cali program = mTask.accFactoryData.ix * mInfo.sensitivity / mInfo.gain;
        mTask.gyroFactoryData.ix =
            (int32_t)((float)data->x * GYROSCOPE_INCREASE_NUM_AP/*131*1000*/ / DEGREE_TO_RADIRAN_SCALAR/*pi/180*/);
        mTask.gyroFactoryData.iy =
            (int32_t)((float)data->y * GYROSCOPE_INCREASE_NUM_AP / DEGREE_TO_RADIRAN_SCALAR);
        mTask.gyroFactoryData.iz =
            (int32_t)((float)data->z * GYROSCOPE_INCREASE_NUM_AP / DEGREE_TO_RADIRAN_SCALAR);
    }

    // if (sensorType == SENS_TYPE_GYRO) {
    //     osLog(LOG_ERROR, "GYRO_report: %f %f %f\n",
    //           (double)data->x, (double)data->y, (double)data->z);
    // }

    if (mTask.debug_trace) {
        switch (sensorType) {
            case SENS_TYPE_ACCEL:
                osLog(LOG_ERROR, "ACCEL:raw_data_x=%f, raw_data_y=%f, raw_data_z=%f\n",
                      (double)data->x, (double)data->y, (double)data->z);
                break;
            case SENS_TYPE_GYRO:
                osLog(LOG_ERROR, "GYRO:raw_data_x=%f, raw_data_y=%f, raw_data_z=%f\n",
                      (double)data->x, (double)data->y, (double)data->z);
                break;
            default:
                break;
        }
    }
}

static int ICM40605Convert(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                           void *inBuf, uint8_t inSize, uint8_t elemInSize,
                           void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    struct accGyroData *data = mTask.accGyroPacket.outBuf;
    uint32_t i = 0;
    uint8_t accEventSize = 0;
    uint8_t gyroEventSize = 0;
    uint8_t accEventSize_Discard = 0;
    uint8_t gyroEventSize_Discard = 0;
    uint64_t swSampleTime = 0, realSampleTime = 0;
    uint8_t *fifo_data = &mTask.regBuffer[1];

    for (i=0; i < mTask.fifoDataToRead; i+=16) {
        if ((accEventSize + gyroEventSize) < MAX_RECV_PACKET) {
            if (mTask.sensors[ACC].configed && mTask.sensors[ACC].powered) {
                if (mTask.sensors[ACC].samplesToDiscard) {
                    mTask.sensors[ACC].samplesToDiscard--;
                    accEventSize_Discard++;
                } else {
                    parseRawData(&data[accEventSize+gyroEventSize], &fifo_data[i+1], SENS_TYPE_ACCEL);
                    accEventSize++;
                }
            }
            if (mTask.sensors[GYR].configed && mTask.sensors[GYR].powered) {
                if (mTask.sensors[GYR].samplesToDiscard) {
                    mTask.sensors[GYR].samplesToDiscard--;
                    gyroEventSize_Discard++;
                } else {
                    parseRawData(&data[accEventSize+gyroEventSize], &fifo_data[i+7], SENS_TYPE_GYRO);
                    gyroEventSize++;
                }
            }
        } else
            osLog(LOG_ERROR, "outBuf full, accEventSize = %d, gyroEventSize = %d\n", accEventSize, gyroEventSize);
    }

    /*if factory true , can't send to runtime cali in parseRawData in accGyro*/
    if (mTask.sensors[ACC].startCali || mTask.sensors[GYR].startCali) {
        accEventSize = 0;
        gyroEventSize = 0;
    }
    //osLog(LOG_INFO, "ICM40605Convert, fifoDataToRead:%d, accEventSize:%d, gyroEventSize:%d\n",
        //mTask.fifoDataToRead, accEventSize, gyroEventSize);

    swSampleTime = mTask.swSampleTime;

#if 0 /* N/A for interrupt pulse mode */
    // check if hw latch time can trust
    if (mTask.hwSampleTime > mTask.unmask_eint_time)
        temp_diff = mTask.hwSampleTime - mTask.unmask_eint_time;
    else
        temp_diff = mTask.unmask_eint_time - mTask.hwSampleTime;
    if (temp_diff < 200000)   // 200us
        mTask.hwSampleTime = mTask.lastSampleTime + minDelay * long_EventSize;
#endif
    /* End Pre Process lastSampleTime and hwSampleTime */

    realSampleTime = calcFakeInterruptTime(swSampleTime, mTask.hwSampleTime, mTask.lastSampleTime,
                                           mTask.sensors[ACC].hwRate, mTask.sensors[ACC].configed,
                                           accEventSize + accEventSize_Discard,
                                           mTask.sensors[GYR].hwRate, mTask.sensors[GYR].configed,
                                           gyroEventSize + gyroEventSize_Discard);
    //osLog(LOG_INFO, "ICM40605Convert, swSampleTime=%llu, hwSampleTime=%llu, realSampleTime=%llu, lastSampleTime=%llu, now=%llu, unmask_eint_time=%llu\n",
        //swSampleTime, mTask.hwSampleTime, realSampleTime, mTask.lastSampleTime, rtcGetTime(), mTask.unmask_eint_time);

    mTask.hwSampleTime = realSampleTime;
    mTask.lastSampleTime = realSampleTime;

    txTransferDataInfo(&mTask.dataInfo, accEventSize, gyroEventSize, realSampleTime, data, 0);

    sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
    mt_eint_unmask(mTask.hw->eint_num);
    mTask.unmask_eint_time = rtcGetTime();

    return 0;
}

static void ICM40605Isr1(int arg)
{
    if (mTask.latch_time_id < 0) {
        mTask.hwSampleTime = rtcGetTime();
        //osLog(LOG_ERROR, "ICM40605Isr1, mTask.latch_time_id < 0, mTask.hwSampleTime=%llu\n", mTask.hwSampleTime);
    } else {
        mTask.hwSampleTime = get_latch_time_timestamp(mTask.latch_time_id);
        //osLog(LOG_ERROR, "ICM40605Isr1, mTask.hwSampleTime=%llu, now_rtc=%llu\n", mTask.hwSampleTime, rtcGetTime());
    }
    accGyroHwIntCheckStatus();
}

static int ICM40605IntStatusCheck(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                  void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                  void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    SPI_READ(ICM40605_REG_INT_STATUS2, 1, &mTask.wakeupBuffer);
    SPI_READ(ICM40605_REG_INT_STATUS, 1, &mTask.statusBuffer);
    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
}

static int ICM40605IntHandling(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                               void *inBuf, uint8_t inSize, uint8_t elemInSize,
                               void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    union EmbeddedDataPoint trigger_axies;
    uint8_t fifo_status = mTask.statusBuffer[1];
    uint8_t anymo_status = mTask.wakeupBuffer[1];
    // osLog(LOG_INFO, "ICM40605IntHandling,  WM INT:0x%x, AnyMotion:0x%x\n", fifo_status, anymo_status);

    if (mTask.sensors[ANYMO].powered) {
        if (anymo_status & 0x07) {
            trigger_axies.idata = (anymo_status & 0x07);
            osLog(LOG_INFO, "Detected any motion\n");
            osEnqueueEvt(EVT_SENSOR_ANY_MOTION, trigger_axies.vptr, NULL);
        }
    }
    if (fifo_status & BIT_WM_INT_EN_MASK) {
       if (!mTask.sensors[ACC].powered && !mTask.sensors[GYR].powered) {
           osLog(LOG_INFO, "Unexpected FIFO WM INTR fired\n");
           // reset fifo
           SPI_WRITE(ICM40605_REG_FIFO_CFG, BIT_FIFO_BYPASS_MODE);
           mt_eint_unmask(mTask.hw->eint_num);
           return spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
       } else {
           accGyroInterruptOccur();
       }
    } else if (fifo_status & BIT_FIFO_FULL_EN_MASK) {
        // reset fifo
        SPI_WRITE(ICM40605_REG_FIFO_CFG, BIT_FIFO_BYPASS_MODE);
        //  set fifo stream mode
        SPI_WRITE(ICM40605_REG_FIFO_CFG, BIT_FIFO_STREAM_MODE);
        mt_eint_unmask(mTask.hw->eint_num);
        return spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
    } else
        mt_eint_unmask(mTask.hw->eint_num);

    sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);

    return 0;
}

static int ICM40605EintRegistration(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                    void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mt_eint_dis_hw_debounce(mTask.hw->eint_num);
    mt_eint_registration(mTask.hw->eint_num, EDGE_SENSITIVE, HIGH_LEVEL_TRIGGER, ICM40605Isr1, EINT_INT_UNMASK,
        EINT_INT_AUTO_UNMASK_OFF);
    sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static void ICM40605SetDebugTrace(int32_t trace)
{
    mTask.debug_trace = trace;
    osLog(LOG_ERROR, "%s ==> trace:%d\n", __func__, mTask.debug_trace);
}

static int anyMotionPowerOn(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                            void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    osLog(LOG_INFO, "Enter anyMotionPowerOn\n");
    uint8_t threshold = 60;

    if (!mTask.sensors[ACC].powered) {
#if 0
        /* set 50HZ, Low noise mode */
        mTask.acc_cfg0 &= 0xF0;
        mTask.acc_cfg0 |= BIT_ACC_ODR_50;
        SPI_WRITE(ICM40605_REG_ACC_CFG, mTask.acc_cfg0);
#endif
        mTask.pwr_sta &= 0xFC;
        mTask.pwr_sta |= BIT_ACC_LN_MODE;
        SPI_WRITE(ICM40605_REG_PWR_MGMT0, mTask.pwr_sta, 50000);
    }
    /* enable anymotion interrupt, set X,Y,Z threshold */
    threshold >>= 2;  // 1 LSB=4mg, convert to LSB
    SPI_WRITE(ICM40605_REG_ACC_WOM_X_THR, threshold);
    SPI_WRITE(ICM40605_REG_ACC_WOM_Y_THR, threshold);
    SPI_WRITE(ICM40605_REG_ACC_WOM_Z_THR, threshold);

    mTask.smd_cfg |= 0x05;
    SPI_WRITE(ICM40605_REG_SMD_CFG, mTask.smd_cfg);  // compare to previous sample
    mTask.int_src1 |= BIT_WOM_EN;
    SPI_WRITE(ICM40605_REG_INT_SOURCE1, mTask.int_src1);

    mTask.sensors[ANYMO].powered = true;

    mt_eint_unmask(mTask.hw->eint_num);
    return  spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
}

static int anyMotionPowerOff(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    osLog(LOG_INFO, "anyMotionPowerOff\n");

    mTask.sensors[ANYMO].configed = false;
    mTask.sensors[ANYMO].powered = false;
    /* for accel disable */
    mTask.int_src1 &= ~BIT_WOM_EN;
    SPI_WRITE(ICM40605_REG_INT_SOURCE1, mTask.int_src1);
    mTask.smd_cfg &= ~0x03;
    SPI_WRITE(ICM40605_REG_SMD_CFG, mTask.smd_cfg);

    if (!mTask.sensors[ACC].powered && !mTask.sensors[GYR].powered) {
        mTask.pwr_sta &= ~BIT_ACC_LN_MODE;
        SPI_WRITE(ICM40605_REG_PWR_MGMT0, mTask.pwr_sta, 50000);
        osLog(LOG_INFO, "anyMotionPowerOff, acc off, set power down mode\n");
    }

    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
}

static void accGetSensorInfo(struct sensorInfo_t *data)
{
    strncpy(data->name, ACC_NAME, sizeof(data->name));
}

static void gyroGetSensorInfo(struct sensorInfo_t *data)
{
    strncpy(data->name, GYRO_NAME, sizeof(data->name));
}

static void sensorCoreRegistration(void)
{
    struct sensorCoreInfo mInfo;
    osLog(LOG_INFO, "ICM40605RegisterCore\n");

    /* Register sensor Core */
    memset(&mInfo, 0x00, sizeof(struct sensorCoreInfo));
    mInfo.sensType = SENS_TYPE_ACCEL;
    mInfo.gain = GRAVITY_EARTH_1000;
    mInfo.sensitivity = mTask.sensors[ACC].sensitivity;
    mInfo.cvt = mTask.cvt;
    mInfo.getCalibration = accGetCalibration;
    mInfo.setCalibration = accSetCalibration;
    mInfo.getData = accGetData;
    mInfo.setDebugTrace = ICM40605SetDebugTrace;
    mInfo.getSensorInfo = accGetSensorInfo;
    sensorCoreRegister(&mInfo);

    memset(&mInfo, 0x00, sizeof(struct sensorCoreInfo));
    mInfo.sensType = SENS_TYPE_GYRO;
    mInfo.gain = GYROSCOPE_INCREASE_NUM_AP;
    mInfo.sensitivity = mTask.sensors[GYR].sensitivity;
    mInfo.cvt = mTask.cvt;
    mInfo.getCalibration = gyroGetCalibration;
    mInfo.setCalibration = gyroSetCalibration;
    mInfo.getData = gyroGetData;
    mInfo.getSensorInfo = gyroGetSensorInfo;
    sensorCoreRegister(&mInfo);
}

static int lcm40605SensorRegistration(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                      void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                      void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    sensorCoreRegistration();
    sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static struct sensorFsm ICM40605Fsm[] = {
    /* INT handle */
    sensorFsmCmd(STATE_HW_INT_STATUS_CHECK, STATE_HW_INT_HANDLING, ICM40605IntStatusCheck),
    sensorFsmCmd(STATE_HW_INT_HANDLING, STATE_HW_INT_HANDLING_DONE, ICM40605IntHandling),
    /* sample */
    sensorFsmCmd(STATE_SAMPLE, STATE_FIFO, ICM40605Sample),
    sensorFsmCmd(STATE_FIFO, STATE_CONVERT, ICM40605ReadFifo),
    sensorFsmCmd(STATE_CONVERT, STATE_SAMPLE_DONE, ICM40605Convert),

    sensorFsmCmd(STATE_ACC_ENABLE, STATE_ACC_ENABLE_DONE, ICM40605gEnable),
    sensorFsmCmd(STATE_ACC_DISABLE, STATE_ACC_DISABLE_DONE, ICM40605gDisable),
    sensorFsmCmd(STATE_ACC_RATECHG, STATE_ACC_RATECHG_DONE, ICM40605gRate),
    sensorFsmCmd(STATE_ACC_CALI, STATE_ACC_CALI_DONE, ICM40605AccCali),
    sensorFsmCmd(STATE_ACC_CFG, STATE_ACC_CFG_DONE, ICM40605AccCfgCali),

    sensorFsmCmd(STATE_GYRO_ENABLE, STATE_GYRO_ENABLE_DONE, ICM40605gyEnable),
    sensorFsmCmd(STATE_GYRO_DISABLE, STATE_GYRO_DISABLE_DONE, ICM40605gyDisable),
    sensorFsmCmd(STATE_GYRO_RATECHG, STATE_GYRO_RATECHG_DONE, ICM40605gyRate),
    sensorFsmCmd(STATE_GYRO_CALI, STATE_GYRO_CALI_DONE, ICM40605GyroCali),
    sensorFsmCmd(STATE_GYRO_CFG, STATE_GYRO_CFG_DONE, ICM40605GyroCfgCali),

    /* init state */
    sensorFsmCmd(STATE_SW_RESET, STATE_SW_RESET_W, ICM40605ResetRead),
    sensorFsmCmd(STATE_SW_RESET_W, STATE_INT_STATUS, ICM40605ResetWrite),
    sensorFsmCmd(STATE_INT_STATUS, STATE_RESET_CHECK, ICM40605IntStatus),
    sensorFsmCmd(STATE_RESET_CHECK, STATE_INIT_REG, ICM40605ResetCheck),
    sensorFsmCmd(STATE_INIT_REG, STATE_SENSOR_REGISTRATION, ICM40605InitConfig),
    sensorFsmCmd(STATE_SENSOR_REGISTRATION, STATE_EINT_REGISTRATION, lcm40605SensorRegistration),
    sensorFsmCmd(STATE_EINT_REGISTRATION, STATE_INIT_DONE, ICM40605EintRegistration),
    /* For Anymotion */
    sensorFsmCmd(STATE_ANYMO_ENABLE, STATE_ANYMO_ENABLE_DONE, anyMotionPowerOn),
    sensorFsmCmd(STATE_ANYMO_DISABLE, STATE_ANYMO_DISABLE_DONE, anyMotionPowerOff),
};

static void initSensorStruct(struct ICM40605Sensor *sensor, enum SensorIndex idx)
{
    sensor->powered = false;
    sensor->configed = false;
    sensor->startCali = false;
    sensor->staticCali[AXIS_X] = 0;
    sensor->staticCali[AXIS_Y] = 0;
    sensor->staticCali[AXIS_Z] = 0;
    sensor->accuracy = 0;
    sensor->rate = 0;
    sensor->latency = SENSOR_LATENCY_NODATA;
    sensor->hwRate = 0;
    sensor->preRealRate = 0;
}

int ICM40605Init(void)
{
    int ret = 0;
    enum SensorIndex i;
    uint8_t txData[2] = {0}, rxData[2] = {0};

    ICM40605DebugPoint = &mTask;
    insertMagicNum(&mTask.accGyroPacket);
    mTask.hw = get_cust_accGyro("ICM40605");
    if (NULL == mTask.hw) {
        osLog(LOG_ERROR, "get_cust_acc_hw fail\n");
        return 0;
    }
    osLog(LOG_INFO, "acc spi_num: %d\n", mTask.hw->i2c_num);

    if (0 != (ret = sensorDriverGetConvert(mTask.hw->direction, &mTask.cvt))) {
        osLog(LOG_ERROR, "invalid direction: %d\n", mTask.hw->direction);
    }
    osLog(LOG_INFO, "acc map[0]:%d, map[1]:%d, map[2]:%d, sign[0]:%d, sign[1]:%d, sign[2]:%d\n\r",
          mTask.cvt.map[AXIS_X], mTask.cvt.map[AXIS_Y], mTask.cvt.map[AXIS_Z],
          mTask.cvt.sign[AXIS_X], mTask.cvt.sign[AXIS_Y], mTask.cvt.sign[AXIS_Z]);

    //Only applicable for factory calibration, refer to sensor spec
    mTask.sensors[ACC].sensitivity = (float)65536 / (8 * 2);//LSB/g sensitivity
    mTask.sensors[GYR].sensitivity = (float)65536 / (2000 * 2);//LSB/dps sensitivity

    mTask.latch_time_id = alloc_latch_time();
    enable_latch_time(mTask.latch_time_id, mTask.hw->eint_num);

    for (i = ACC; i < NUM_OF_SENSOR; i++) {
        initSensorStruct(&mTask.sensors[i], i);
    }

    mTask.int_src0 = 0;
    mTask.int_src1 = 0;
    mTask.smd_cfg = 0;
    mTask.pwr_sta = 0;
    mTask.acc_cfg0 = 0;
    mTask.gyro_cfg0 = 0;
    mTask.fifo_cfg1 = 0;
    mTask.mode.speed = 5000000;
    mTask.mode.bitsPerWord = 8;
    mTask.mode.cpol = SPI_CPOL_IDLE_LO;
    mTask.mode.cpha = SPI_CPHA_LEADING_EDGE;
    mTask.mode.nssChange = true;
    mTask.mode.format = SPI_FORMAT_MSB_FIRST;
    mTask.mWbufCnt = 0;
    mTask.mRegCnt = 0;

    spiMasterRequest(mTask.hw->i2c_num, &mTask.spiDev);

    txData[0] = ICM40605_REG_DEVID | 0x80;
    ret = spiMasterRxTxSync(mTask.spiDev, rxData, txData, 2);
    osLog(LOG_ERROR, "ICM40605_WHOAMI: %02x, ret: %d\n", rxData[1], ret);
    if (ret < 0 || ((rxData[1] != ICM40605_WHOAMI) && (rxData[1] != ICM40606_WHOAMI))) {
        osLog(LOG_ERROR, "failed id match: %02x, ret: %d\n", rxData[1], ret);
        ret = -1;
        sendSensorErrToAp(ERR_SENSOR_ACC_GYR, ERR_CASE_ACC_GYR_INIT, ACC_NAME);
        sendSensorErrToAp(ERR_SENSOR_ACC_GYR, ERR_CASE_ACC_GYR_INIT, GYRO_NAME);
        spiMasterRelease(mTask.spiDev);
        disable_latch_time(mTask.latch_time_id);
        free_latch_time(mTask.latch_time_id);
        goto err_out;
    }
    else if (rxData[1] == ICM40605_WHOAMI)
        osLog(LOG_INFO, "ICM40605: auto detect success: %02x\n", rxData[1]);
    else if (rxData[1] == ICM40605L_WHOAMI)
        osLog(LOG_INFO, "ICM40605-L: auto detect success: %02x\n", rxData[1]);
    else if (rxData[1] == ICM40606_WHOAMI)
        osLog(LOG_INFO, "ICM40606: auto detect success: %02x\n", rxData[1]);
    accSensorRegister();
    gyroSensorRegister();
    anyMotionSensorRegister();
    registerAccGyroInterruptMode(ACC_GYRO_FIFO_INTERRUPTIBLE);
    registerAccGyroDriverFsm(ICM40605Fsm, ARRAY_SIZE(ICM40605Fsm));

err_out:
    return ret;
}
#ifndef CFG_OVERLAY_INIT_SUPPORT
MODULE_DECLARE(icm40605, SENS_TYPE_ACCEL, ICM40605Init);
#else
#include "mtk_overlay_init.h"
OVERLAY_DECLARE(icm40605, OVERLAY_ID_ACCGYRO, ICM40605Init);
#endif
