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
#include <algos/time_sync.h>
#include <atomic.h>
#include <cpu/inc/cpuMath.h>
#include <gpio.h>
#include <heap.h>
#include <hostIntf.h>
/* #include <isr.h> */
#include <nanohub_math.h>
#include <nanohubPacket.h>
/* #include <plat/inc/exti.h> */
/* #include <plat/inc/gpio.h> */
/* #include <plat/inc/syscfg.h> */
#include <plat/inc/rtc.h>
#include <sensors.h>
#include <seos.h>
#include <slab.h>
#include <spi.h>
#include <plat/inc/spichre.h>
#include <spichre-plat.h>
#include <timer.h>
/* #include <variant/inc/sensType.h> */
#include <variant/inc/variant.h>
#include <util.h>
#include <accGyro.h>
#include <cust_accGyro.h>
#include "hwsen.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <contexthub_core.h>
#include "eint.h"
#include <performance.h>
#ifndef CFG_MTK_CALIBRATION_V1_0
#include <API_sensor_calibration.h>
#else
#include "calibration/factory_calibration/factory_cal.h"
#endif

#define ACC_NAME     "lis2hh12"
#define GYRO_NAME     "virtual_gyro"
#define LIS2HH12_WAI_VALUE                              (0x41)
#define kScale_acc    0.00239364f     // ACC_range * 9.81f / 32768.0f;
#define kScale_gyr    (DEGREE_TO_RADIRAN_SCALAR * 70 / 1000.0f)
#define kScale_temp   0.001953125f    // temperature in deg C
/* One sample of triaxial sensor is expressed on 6 byte */
#define LIS2HH12_ONE_SAMPLE_BYTE                        6

/* LIS2HH12 registers */
#define LIS2HH12_WAI_ADDR                               (0X0F)
#define LIS2HH12_CTRL_REG1_ADDR                         (0x20)
#define LIS2HH12_CTRL_REG2_ADDR                         (0x21)
#define LIS2HH12_CTRL_REG3_ADDR                         (0x22)
#define LIS2HH12_CTRL_REG4_ADDR                         (0x23)
#define LIS2HH12_CTRL_REG5_ADDR                         (0x24)
#define LIS2HH12_CTRL_REG6_ADDR                         (0x25)
#define LIS2HH12_CTRL_REG7_ADDR                         (0x26)
#define LIS2HH12_STATUS_REG_ADDR                        (0x27)
#define LIS2HH12_OUT_X_L_ADDR                           (0x28)
#define LIS2HH12_FIFO_CTRL_REG_ADDR                     (0x2E)
#define LIS2HH12_FIFO_SRC_REG_ADDR                      (0x2F)
#define LIS2HH12_IG_CFG2_REG_ADDR                       (0x36)
#define LIS2HH12_IG_SRC2_REG_ADDR                       (0x37)
#define LIS2HH12_IG_THS2_REG_ADDR                       (0x38)
#define LIS2HH12_IG_DUR2_REG_ADDR                       (0x39)
#define LIS2HH12_XL_REFERENCE_REG_ADDR                  (0x3A)


#define LIS2HH12_CTRL_REG1_BASE                      ((0 << 7) |    /* HR */ \
                                                      (0 << 6) |    /* ODR2 */ \
                                                      (0 << 5) |    /* ODR1 */ \
                                                      (0 << 4) |    /* ODR0 */ \
                                                      (1 << 3) |    /* BDU */ \
                                                      (1 << 2) |    /* Zen */ \
                                                      (1 << 1) |    /* Yen */ \
                                                      (1 << 0))     /* Xen */
#define LIS2HH12_CTRL_REG2_BASE                      ((0 << 7) |    /* - */ \
                                                      (0 << 6) |    /* DFC1 */ \
                                                      (0 << 5) |    /* DFC0 */ \
                                                      (0 << 4) |    /* HPM1 */ \
                                                      (0 << 3) |    /* HPM0 */ \
                                                      (0 << 2) |    /* FDS */ \
                                                      (0 << 1) |    /* HPIS1 */ \
                                                      (1 << 0))     /* HPIS2*/

#define LIS2HH12_CTRL_REG3_BASE                      ((0 << 7) |    /* FIFO_EN */ \
                                                      (0 << 6) |    /* STOP_FTH */ \
                                                      (0 << 5) |    /* I1_INACT */ \
                                                      (0 << 4) |    /* I1_IG2 */ \
                                                      (0 << 3) |    /* I1_IG1 */ \
                                                      (1 << 2) |    /* I1_OVR */ \
                                                      (1 << 1) |    /* I1_FTH */ \
                                                      (0 << 0))     /* I1_DRDY*/

#define LIS2HH12_CTRL_REG4_BASE                      ((0 << 7) |    /* BW2 */ \
                                                      (0 << 6) |    /* BW1 */ \
                                                      (1 << 5) |    /* FS1 */ \
                                                      (1 << 4) |    /* FS0 */ \
                                                      (1 << 3) |    /* BW_SCALE_ODR */ \
                                                      (1 << 2) |    /* IF_ADD_INC */ \
                                                      (1 << 1) |    /* I2C_DISABLE */ \
                                                      (0 << 0))     /* SIM */

#define LIS2HH12_CTRL_REG5_BASE                      ((0 << 7) |    /* DEBUG */ \
                                                      (0 << 6) |    /* SOFT_RESET */ \
                                                      (0 << 5) |    /* DEC1 */ \
                                                      (0 << 4) |    /* DEC0 */ \
                                                      (0 << 3) |    /* ST2 */ \
                                                      (0 << 2) |    /* ST1 */ \
                                                      (0 << 1) |    /* H_LACTIVE */ \
                                                      (0 << 0))     /* PP_OD */

#define LIS2HH12_CTRL_REG7_BASE                      ((0 << 7) |    /* - */ \
                                                      (0 << 6) |    /* - */ \
                                                      (0 << 5) |    /* DCRM2 */ \
                                                      (0 << 4) |    /* DCRM1 */ \
                                                      (0 << 3) |    /* LIR2 */ \
                                                      (0 << 2) |    /* LIR1 */ \
                                                      (0 << 1) |    /* 4D_IG2 */ \
                                                      (0 << 0))     /* 4D_IG1 */


#define LIS2HH12_FIFO_CTRL_REG_BASE                  ((0 << 7) |    /* FM2 */ \
                                                      (0 << 6) |    /* FM1 */ \
                                                      (0 << 5) |    /* FM0 */ \
                                                      (0 << 4) |    /* FTH4 */ \
                                                      (0 << 3) |    /* FTH3 */ \
                                                      (0 << 2) |    /* FTH2 */ \
                                                      (0 << 1) |    /* FTH1 */ \
                                                      (0 << 0))     /* FTH0 */

#define LIS2HH12_IG_CFG2_BASE                       ((0 << 7) |    /* AOI */ \
                                                      (0 << 6) |    /* 6D */ \
                                                      (1 << 5) |    /* ZHIE */ \
                                                      (0 << 4) |    /* ZLIE */ \
                                                      (1 << 3) |    /* YHIE */ \
                                                      (0 << 2) |    /* YLIE */ \
                                                      (1 << 1) |    /* XHIE */ \
                                                      (0 << 0))     /* XLIE */



/* LIS2HH12 fifo status */
#define LIS2HH12_FIFO_STATUS2_FIFO_EMPTY               (0x20)
#define LIS2HH12_FIFO_STATUS2_FIFO_FULL_OVERRUN        (0x40)
#define LIS2HH12_FIFO_STATUS2_FIFO_ERROR               (LIS2HH12_FIFO_STATUS2_FIFO_EMPTY | \
                                                        LIS2HH12_FIFO_STATUS2_FIFO_FULL_OVERRUN)
#define LIS2HH12_FIFO_STATUS2_FIFO_WATERMARK           (0x80)

#define LIS2HH12_WAKEUP_MASK                           (0x40)
#define LIS2HH12_IG2_TO_INT1                           (0X10)

/* LIS2HH12 fifo modes */
#define LIS2HH12_FIFO_BYPASS_MODE                      (0x00)
#define LIS2HH12_FIFO_FIFO_MODE                        (0x20)
#define LIS2HH12_FIFO_STREAM_MODE                      (0x40)
#define LIS2HH12_FIFO_CTRL2_FTH_MASK                   (0x1F)
#define LIS2HH12_FIFO_EN                               (0x80)

/* LIS2HH12 SW RESET */
#define LIS2HH12_SW_RESET                              (0x40)

/*LIS2HH12 ODR RATE related*/
#define LIS2HH12_ODR_10HZ_ACCEL_STD                      1
#define LIS2HH12_ODR_50HZ_ACCEL_STD                      1
#define LIS2HH12_ODR_100HZ_ACCEL_STD                     1
#define LIS2HH12_ODR_200HZ_ACCEL_STD                     1
#define LIS2HH12_ODR_400HZ_ACCEL_STD                     1


#define LIS2HH12_ODR_10HZ_REG_VALUE                    (0x10)
#define LIS2HH12_ODR_50HZ_REG_VALUE                    (0x20)
#define LIS2HH12_ODR_100HZ_REG_VALUE                   (0x30)
#define LIS2HH12_ODR_200HZ_REG_VALUE                   (0x40)
#define LIS2HH12_ODR_400HZ_REG_VALUE                   (0x50)

#define max(x, y)   (x > y ? x : y)
#define SPI_PACKET_SIZE  30
#define SPI_BUF_SIZE    (1024 + 4)
#define SPI_WRITE_0(addr, data) spiQueueWrite(addr, data, 2)
#define SPI_WRITE_1(addr, data, delay) spiQueueWrite(addr, data, delay)
#define GET_SPI_WRITE_MACRO(_1, _2, _3, NAME, ...) NAME
#define SPI_WRITE(...) GET_SPI_WRITE_MACRO(__VA_ARGS__, SPI_WRITE_1, SPI_WRITE_0)(__VA_ARGS__)
#define SPI_READ_0(addr, size, buf) spiQueueRead(addr, size, buf, 0)
#define SPI_READ_1(addr, size, buf, delay) spiQueueRead(addr, size, buf, delay)
#define GET_SPI_READ_MACRO(_1, _2, _3, _4, NAME, ...) NAME
#define SPI_READ(...) GET_SPI_READ_MACRO(__VA_ARGS__, SPI_READ_1, SPI_READ_0)(__VA_ARGS__)
#define EVT_SENSOR_ANY_MOTION       sensorGetMyEventType(SENS_TYPE_ANY_MOTION)

enum LIS2HH12State {
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
    STATE_ACC_SELF_TEST = CHIP_ACC_SELFTEST,
    STATE_ACC_SELF_TEST_DONE = CHIP_ACC_SELFTEST_DONE,
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
    STATE_INIT_REG,
    STATE_SENSOR_REGISTRATION,
    STATE_EINT_REGISTRATION,
};

enum SensorIndex {
    ACC = 0,
    ANYMO,
    NUM_OF_SENSOR,
};

struct lis2hh12Sensor {
    bool powered;
    bool configed;
    bool startCali;
    float staticCali[AXES_NUM];
    int32_t accuracy;
    uint32_t rate;
    uint64_t latency;
    uint32_t hwRate;  // rate set in hw
    uint8_t samplesToDiscard;
    float sensitivity;
};

static struct LIS2HH12Task {
    struct lis2hh12Sensor sensors[NUM_OF_SENSOR];

    uint16_t watermark;
    uint32_t fifoDataToRead;
    uint64_t hwSampleTime;
    uint64_t swSampleTime;
    uint64_t lastSampleTime;
    uint8_t *regBuffer;
    uint8_t *statusBuffer;
    uint8_t *wakeupBuffer;
    uint8_t *tempStatusBuffer;
    uint8_t *tempBuffer;
    uint8_t *dummyBuffer;
    float temperature;
    bool tempReady;

    SpiCbkF spiCallBack;
    struct transferDataInfo dataInfo;
    struct accGyroDataPacket accGyroPacket;
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
    /* data for factory */
    struct TripleAxisDataPoint accFactoryData;
    int32_t accSwCali[AXES_NUM];
    int32_t debug_trace;

    int latch_time_id;
#ifdef CFG_MTK_CALIBRATION_V1_0
    struct AccFactoryCal accFactoryCal;
#endif
} mTask;

static void spiQueueWrite(uint8_t addr, uint8_t data, uint32_t delay) {
    mTask.packets[mTask.mRegCnt].size = 2;
    mTask.packets[mTask.mRegCnt].txBuf = &mTask.txrxBuffer[mTask.mWbufCnt];
    mTask.packets[mTask.mRegCnt].rxBuf = &mTask.txrxBuffer[mTask.mWbufCnt];
    mTask.packets[mTask.mRegCnt].delay = delay * 1000;
    mTask.txrxBuffer[mTask.mWbufCnt++] = addr;
    mTask.txrxBuffer[mTask.mWbufCnt++] = data;
    mTask.mWbufCnt = (mTask.mWbufCnt + 3) & 0xFFFC;
    mTask.mRegCnt++;
}

static void spiQueueRead(uint8_t addr, size_t size, uint8_t **buf, uint32_t delay) {
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
        SpiCbkF callback, void *cookie) {
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

static void accGetCalibration(int32_t *cali, int32_t size) {
    cali[AXIS_X] = mTask.accSwCali[AXIS_X];
    cali[AXIS_Y] = mTask.accSwCali[AXIS_Y];
    cali[AXIS_Z] = mTask.accSwCali[AXIS_Z];
    /* osLog(LOG_INFO, "accGetCalibration cali x:%d, y:%d, z:%d\n", cali[AXIS_X], cali[AXIS_Y], cali[AXIS_Z]); */
}

static void accSetCalibration(int32_t *cali, int32_t size) {
    mTask.accSwCali[AXIS_X] = cali[AXIS_X];
    mTask.accSwCali[AXIS_Y] = cali[AXIS_Y];
    mTask.accSwCali[AXIS_Z] = cali[AXIS_Z];
    /* osLog(LOG_INFO, "accSetCalibration cali x:%d, y:%d, z:%d\n", mTask.accSwCali[AXIS_X],
        mTask.accSwCali[AXIS_Y], mTask.accSwCali[AXIS_Z]); */
}

static void accGetData(void *sample) {
    struct TripleAxisDataPoint *tripleSample = (struct TripleAxisDataPoint *)sample;
    tripleSample->ix = mTask.accFactoryData.ix;
    tripleSample->iy = mTask.accFactoryData.iy;
    tripleSample->iz = mTask.accFactoryData.iz;
}

static void lis2hh12SetDebugTrace(int32_t trace) {
    mTask.debug_trace = trace;
}

static void accGetSensorInfo(struct sensorInfo_t *data)
{
    strncpy(data->name, ACC_NAME, sizeof(data->name));
}

static void gyroGetSensorInfo(struct sensorInfo_t *data)
{
    strncpy(data->name, GYRO_NAME, sizeof(data->name));
}

static void lis2hh12Isr1(int arg) {
    if (mTask.latch_time_id < 0) {
        mTask.hwSampleTime = rtcGetTime();
        //osLog(LOG_ERROR, "lis2hh12Isr1, real=%lld\n", mTask.hwSampleTime);
    } else {
        mTask.hwSampleTime = get_latch_time_timestamp(mTask.latch_time_id);
        //osLog(LOG_ERROR, "lis2hh12Isr1, fake=%lld, real=%lld\n",
            //rtcGetTime(), mTask.hwSampleTime);
    }

    accGyroHwIntCheckStatus();
}

static int anyMotionPowerOn(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                            void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    uint32_t delay = 25;


    if (!mTask.sensors[ACC].configed) {
        SPI_WRITE(LIS2HH12_CTRL_REG1_ADDR, LIS2HH12_CTRL_REG1_BASE | LIS2HH12_ODR_50HZ_REG_VALUE, 5);
        delay = (1024000000 / SENSOR_HZ(50.0f));                     // 50Hz->20ms
    }

    /* enable anymotion interrupt */
    SPI_WRITE(LIS2HH12_IG_THS2_REG_ADDR, 0x02, 5);                          // threshold: 31mg*2=62mg
    SPI_WRITE(LIS2HH12_IG_DUR2_REG_ADDR, 0x00, 5);                          // Duration=0
    SPI_READ(LIS2HH12_XL_REFERENCE_REG_ADDR, 6, &mTask.dummyBuffer, delay); //dummy read to make x y z to zero
    SPI_WRITE(LIS2HH12_IG_CFG2_REG_ADDR, LIS2HH12_IG_CFG2_BASE, 5);         // enable XH and YH and ZH interrupt generation
    SPI_WRITE(LIS2HH12_CTRL_REG3_ADDR, LIS2HH12_CTRL_REG3_BASE              //add anymo int control
        | LIS2HH12_IG2_TO_INT1 | (mTask.sensors[ACC].powered ? LIS2HH12_FIFO_EN : 0x00), 5);
    mTask.sensors[ANYMO].powered = true;

    return  spiBatchTxRx(&mTask.mode, spiCallBack, next_state);
}

static int anyMotionPowerOff(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {

    mTask.sensors[ANYMO].configed = false;
    mTask.sensors[ANYMO].powered = false;
    SPI_WRITE(LIS2HH12_CTRL_REG3_ADDR, LIS2HH12_CTRL_REG3_BASE      //add anymo int control, prevent unknown int
        | (mTask.sensors[ACC].powered ? LIS2HH12_FIFO_EN : 0x00), 5);
    SPI_WRITE(LIS2HH12_IG_CFG2_REG_ADDR, 0x00, 5);    // disable XH and YH ZH interrupt generation

    /* for accel disable */
    if (!mTask.sensors[ACC].powered) {
         SPI_WRITE(LIS2HH12_CTRL_REG1_ADDR, LIS2HH12_CTRL_REG1_BASE, 25);
         osLog(LOG_INFO, "anyMotionPowerOff, acc off, set power down mode\n");
    }

    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state);
}


static int lis2hh12AccPowerOn(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {

    mTask.sensors[ACC].powered = true;
    /*enable fifo interrupt*/
    SPI_WRITE(LIS2HH12_CTRL_REG3_ADDR, LIS2HH12_CTRL_REG3_BASE
        | LIS2HH12_FIFO_EN | (mTask.sensors[ANYMO].powered ? LIS2HH12_IG2_TO_INT1 : 0x00), 5); /*FIFO: I1_WTM, I1_OVERRUN*/

    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state);
}

static uint8_t LIS2HH12ImuRatesRegValue[] = {
    LIS2HH12_ODR_10HZ_REG_VALUE,     /* 10Hz */
    LIS2HH12_ODR_50HZ_REG_VALUE,     /* 50Hz */
    LIS2HH12_ODR_100HZ_REG_VALUE,    /* 100Hz */
    LIS2HH12_ODR_200HZ_REG_VALUE,    /* 200Hz */
    LIS2HH12_ODR_400HZ_REG_VALUE,    /* 400Hz */
};
static uint32_t LIS2HH12ImuRates[] = {
    SENSOR_HZ(10.0f),                /* 10Hz */
    SENSOR_HZ(50.0f),                /* 50Hz */
    SENSOR_HZ(100.0f),               /* 100Hz */
    SENSOR_HZ(200.0f),               /* 200Hz */
    SENSOR_HZ(400.0f),               /* 400Hz */
    0,
};
static uint8_t LIS2HH12AccelRatesSamplesToDiscard[] = {
    LIS2HH12_ODR_10HZ_ACCEL_STD,     /* 10Hz */
    LIS2HH12_ODR_50HZ_ACCEL_STD,     /* 50Hz */
    LIS2HH12_ODR_100HZ_ACCEL_STD,    /* 100Hz */
    LIS2HH12_ODR_200HZ_ACCEL_STD,    /* 200Hz */
    LIS2HH12_ODR_400HZ_ACCEL_STD,    /* 400Hz */
};

static int lis2hh12CalcuOdr(uint32_t *rate, uint32_t *report_rate) {
    int i;
    for (i = 0; i < (ARRAY_SIZE(LIS2HH12ImuRates) - 1); i++) {
        if (*rate <= LIS2HH12ImuRates[i]) {
            *report_rate = LIS2HH12ImuRates[i];
            break;
        }
    }
    if (*rate > LIS2HH12ImuRates[(ARRAY_SIZE(LIS2HH12ImuRates) - 2)]) {
        i = (ARRAY_SIZE(LIS2HH12ImuRates) - 2);
        *report_rate = LIS2HH12ImuRates[i];
    }
    if (i == (ARRAY_SIZE(LIS2HH12ImuRates) - 1 )) {
        osLog(LOG_ERROR, "ODR not valid! Selected smallest ODR available\n");
        return -1;
    }
    return i;
}
static uint16_t lis2hh12CalcuWm(void)
{
    uint64_t min_latency = SENSOR_LATENCY_NODATA;
    uint8_t min_watermark = 1;
    uint8_t max_watermark = MAX_RECV_PACKET;
    uint16_t watermark = 0;
    uint32_t temp_cnt, total_cnt = 0;
    uint32_t temp_delay = 0;

    if (mTask.sensors[ACC].hwRate && mTask.sensors[ACC].latency != SENSOR_LATENCY_NODATA) {
        min_latency =
            mTask.sensors[ACC].latency < min_latency ? mTask.sensors[ACC].latency : min_latency;
    }

    /*if acc and gyr off or acc and gyr latency = SENSOR_LATENCY_NODATA, watermark = 0 or 1 or 2*/
    if (min_latency == SENSOR_LATENCY_NODATA) {
        if (mTask.sensors[ACC].hwRate) {
            watermark++;
        }
    } else {
        if (mTask.sensors[ACC].hwRate) {
            temp_delay = (1000000000ULL / mTask.sensors[ACC].hwRate) << 10;
            temp_cnt = min_latency / temp_delay;
            min_watermark = mTask.sensors[ACC].hwRate / SENSOR_HZ(400.0f);
            total_cnt += temp_cnt > min_watermark ? temp_cnt : min_watermark;
            osLog(LOG_INFO, "lis2hh12CalcuWm, delay=%lu, latency:%lld, min_wm=%d, total_cnt=%lu\n",
                temp_delay, min_latency, min_watermark, total_cnt);
        }
        watermark = total_cnt;
        watermark = watermark < min_watermark ? min_watermark : watermark;
        watermark = watermark > max_watermark ? max_watermark : watermark;
    }
    /*limit to fifo length that sensor can support,
       *the max fifo length minus 2 to prevent constant fifo full interrupt
       */
    watermark = watermark > (LIS2HH12_FIFO_CTRL2_FTH_MASK - 1) ?
        (LIS2HH12_FIFO_CTRL2_FTH_MASK - 1) : watermark;
    osLog(LOG_INFO, "lis2hh12CalcuWm, real watermark=%d\n", watermark);
    /*write into register, if 1 is set, interrupt when the second data has finished*/
    watermark --;
    if (watermark == 0)
        watermark = 1;  //hw can't set 0
    return watermark;
}

static void lis2hh12ConfigFifo(bool odr_change) {
    uint8_t regValue;
    uint16_t watermarkReg;
    watermarkReg = mTask.watermark;
    regValue = *((uint8_t *)&watermarkReg);

    if (odr_change)
        SPI_WRITE(LIS2HH12_FIFO_CTRL_REG_ADDR, LIS2HH12_FIFO_CTRL_REG_BASE |
            LIS2HH12_FIFO_BYPASS_MODE, 50);                  //first bypass all ,to reset fifo

    SPI_WRITE(LIS2HH12_FIFO_CTRL_REG_ADDR, LIS2HH12_FIFO_CTRL_REG_BASE |
        (regValue & LIS2HH12_FIFO_CTRL2_FTH_MASK) | LIS2HH12_FIFO_STREAM_MODE, 50);

    //osLog(LOG_ERROR, "lis2hh12ConfigFifo watermark:%d\n", regValue);
}

static int lis2hh12AccRate(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    int ret = 0;
    int odr = 0;
    uint8_t regValue = 0x00;
    uint32_t sampleRate = 0;
    uint32_t delay = 25;
    bool accelOdrChanged = false;
    struct accGyroCntlPacket cntlPacket;

    ret = rxControlInfo(&cntlPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "lis2hh12AccRate, rx inSize and elemSize error\n");
        return -1;
    }

    mTask.sensors[ACC].rate = cntlPacket.rate;
    mTask.sensors[ACC].latency = cntlPacket.latency;
    mTask.watermark = cntlPacket.waterMark;


    odr = lis2hh12CalcuOdr(&mTask.sensors[ACC].rate, &sampleRate);
    if (odr < 0) {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "lis2hh12AccRate, calcu odr error\n");
        return -1;
    }

    if (mTask.sensors[ANYMO].powered) {
        SPI_WRITE(LIS2HH12_IG_CFG2_REG_ADDR, 0x00, 5);   // disable anymotion interrupt firstly
    }

    if ((sampleRate != mTask.sensors[ACC].hwRate)) {
        mTask.sensors[ACC].hwRate = sampleRate;
        regValue = LIS2HH12ImuRatesRegValue[odr];

        delay = LIS2HH12AccelRatesSamplesToDiscard[odr] * (1024000000 / sampleRate);
        mTask.sensors[ACC].samplesToDiscard = LIS2HH12AccelRatesSamplesToDiscard[odr];

        SPI_WRITE(LIS2HH12_CTRL_REG1_ADDR, LIS2HH12_CTRL_REG1_BASE | regValue, 30);
        accelOdrChanged = true;
    } else {
        accelOdrChanged = false;
    }

    registerAccGyroFifoInfo((mTask.sensors[ACC].hwRate == 0) ? 0 : 1024000000000 / mTask.sensors[ACC].hwRate, 0);
    mTask.sensors[ACC].configed = true;
    // osLog(LOG_ERROR, "lis2hh12AccRate acc hwRate:%d\n",mTask.sensors[ACC].hwRate);
    mTask.watermark = lis2hh12CalcuWm();

    lis2hh12ConfigFifo(accelOdrChanged);

    if (mTask.sensors[ANYMO].powered) {
        SPI_WRITE(LIS2HH12_IG_DUR2_REG_ADDR, 0x00, 5);                          // Duration=0
        SPI_READ(LIS2HH12_XL_REFERENCE_REG_ADDR, 6, &mTask.dummyBuffer, delay); //dummy read to make x y z to zero
        SPI_WRITE(LIS2HH12_IG_CFG2_REG_ADDR, LIS2HH12_IG_CFG2_BASE, 5);         // enable XH and YH and ZH interrupt generation
    }

    mt_eint_unmask(mTask.hw->eint_num);
    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state);
}

static int lis2hh12AccPowerOff(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    int ret = 0;
    uint32_t delay = 25;

    struct accGyroCntlPacket cntlPacket;
    ret = rxControlInfo(&cntlPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "lis2hh12AccPowerOff, rx inSize and elemSize error\n");
        return -1;
    }

    mTask.sensors[ACC].rate = 0;
    mTask.sensors[ACC].hwRate = 0;
    mTask.sensors[ACC].latency = SENSOR_LATENCY_NODATA;
    mTask.sensors[ACC].samplesToDiscard = 0;

    mt_eint_mask(mTask.hw->eint_num);

    SPI_WRITE(LIS2HH12_CTRL_REG3_ADDR, LIS2HH12_CTRL_REG3_BASE, 50);          // fifo disable
    SPI_WRITE(LIS2HH12_FIFO_CTRL_REG_ADDR, LIS2HH12_FIFO_CTRL_REG_BASE, 50);  // bypass mode, reset fifo

    if (mTask.sensors[ANYMO].powered) {
        SPI_WRITE(LIS2HH12_IG_CFG2_REG_ADDR, 0x00, 5);                           // disable anymotion interrupt firstly

        SPI_WRITE(LIS2HH12_CTRL_REG1_ADDR, LIS2HH12_CTRL_REG1_BASE | LIS2HH12_ODR_50HZ_REG_VALUE, 5);
        delay = (1024000000 / SENSOR_HZ(50.0f));                             // 50Hz->20ms

        SPI_WRITE(LIS2HH12_IG_DUR2_REG_ADDR, 0x00, 5);                        // Duration=0
        SPI_READ(LIS2HH12_XL_REFERENCE_REG_ADDR, 6, &mTask.dummyBuffer, delay); //dummy read to make x y z to zero
        SPI_WRITE(LIS2HH12_IG_CFG2_REG_ADDR, LIS2HH12_IG_CFG2_BASE, 5);       // enable XH and YH and ZH interrupt generation
        SPI_WRITE(LIS2HH12_CTRL_REG3_ADDR, LIS2HH12_CTRL_REG3_BASE | LIS2HH12_IG2_TO_INT1, 5);
        mt_eint_unmask(mTask.hw->eint_num);
    } else {
        SPI_WRITE(LIS2HH12_CTRL_REG1_ADDR, LIS2HH12_CTRL_REG1_BASE, 30);      // odr->0
    }

    registerAccGyroFifoInfo(0, 0);

    mTask.sensors[ACC].powered = false;
    mTask.sensors[ACC].configed = false;

    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state);
}
static int lis2hh12AccCali(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                            void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
#ifndef CFG_MTK_CALIBRATION_V1_0
    float bias[AXES_NUM] = {0};
#endif

    mTask.sensors[ACC].startCali = true;
#ifndef CFG_MTK_CALIBRATION_V1_0
    Acc_init_calibration(bias);
#else
    accFactoryCalibrateInit(&mTask.accFactoryCal);
#endif
    sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int lis2hh12AccCfgCali(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                            void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    struct accGyroCaliCfgPacket caliCfgPacket;

    ret = rxCaliCfgInfo(&caliCfgPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);

    if (ret < 0) {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "lis2hh12AccCfgCali, rx inSize and elemSize error\n");
        return -1;
    }

    mTask.sensors[ACC].staticCali[0] = (float)caliCfgPacket.caliCfgData[0] / 1000;
    mTask.sensors[ACC].staticCali[1] = (float)caliCfgPacket.caliCfgData[1] / 1000;
    mTask.sensors[ACC].staticCali[2] = (float)caliCfgPacket.caliCfgData[2] / 1000;

    sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int lis2hh12AccSelfTest(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                            void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    accGyroSendTestResult(SENS_TYPE_ACCEL, 1); /* 0 fail, 1 success, and you could define by yourself */
    sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int lis2hh12IntStatusCheck(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                            void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    /*osLog(LOG_INFO, "lis2hh12IntStatusCheck\n");*/
    if (mTask.sensors[ANYMO].powered)
        SPI_READ(LIS2HH12_IG_SRC2_REG_ADDR, 1, &mTask.wakeupBuffer);

    SPI_READ(LIS2HH12_FIFO_SRC_REG_ADDR, 1, &mTask.statusBuffer);

    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state);
}

static int lis2hh12IntHandling(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    int err = 0;
    union EmbeddedDataPoint trigger_axies;
    uint8_t fifo_status = mTask.statusBuffer[1];

    if (mTask.sensors[ANYMO].powered) {
        uint8_t anymo_status = mTask.wakeupBuffer[1];
        if (anymo_status & LIS2HH12_WAKEUP_MASK) {
            trigger_axies.idata = (anymo_status & 0x7f);
            osLog(LOG_INFO, "Detected any motion\n");
            // as other vendor do , we do not enable software mask here
            osEnqueueEvt(EVT_SENSOR_ANY_MOTION, trigger_axies.vptr, NULL);
        }
    }
/*
    if ((fifo_status & LIS2HH12_FIFO_STATUS2_FIFO_ERROR) == 0) {
        if (fifo_status & LIS2HH12_FIFO_STATUS2_FIFO_WATERMARK) {
            accGyroInterruptOccur();
        } else {
            mt_eint_unmask(mTask.hw->eint_num);  // for other interrupt
        // osLog(LOG_INFO, "anymo uneint\n");
        }
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
    } else {
        if ((fifo_status & LIS2HH12_FIFO_STATUS2_FIFO_FULL_OVERRUN)) {
            osLog(LOG_INFO, "FIFO full\n");

            watermarkReg = mTask.watermark;
            regValue = *((uint8_t *)&watermarkReg);

            SPI_WRITE(LIS2HH12_FIFO_CTRL_REG_ADDR, LIS2HH12_FIFO_CTRL_REG_BASE |
                (regValue & 0x1F) | LIS2HH12_FIFO_FIFO_MODE, 50);

            err = spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
        } else {
            osLog(LOG_INFO, "FIFO empty\n");
            sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
        }
        // If not enter accGyroInterruptOccur by INT_FIFO_WM, need unmask eint here
        mt_eint_unmask(mTask.hw->eint_num);
    }
*/
    if (fifo_status & LIS2HH12_FIFO_STATUS2_FIFO_WATERMARK) {
        accGyroInterruptOccur();
    } else if (fifo_status & LIS2HH12_FIFO_STATUS2_FIFO_FULL_OVERRUN) {
        osLog(LOG_INFO, "FIFO full\n");
        accGyroInterruptOccur();
    } else {
        if ((fifo_status & LIS2HH12_FIFO_STATUS2_FIFO_EMPTY))
            osLog(LOG_INFO, "FIFO empty\n");
        mt_eint_unmask(mTask.hw->eint_num); // for other interrupt
        // osLog(LOG_INFO, "anymo uneint\n");
    }
    sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);

    return err;
}

static void spiIsrCallBack(void *cookie, int err) {
    if (err != 0) {
        osLog(LOG_ERROR, "lis2hh12: spiIsrCallBack err\n");
        sensorFsmEnqueueFakeSpiEvt(mTask.spiCallBack, cookie, ERROR_EVT);
    } else {
        mTask.swSampleTime = rtcGetTime();
        sensorFsmEnqueueFakeSpiEvt(mTask.spiCallBack, cookie, SUCCESS_EVT);
    }
}

static int lis2hh12Sample(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    int ret = 0;

    ret = rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "lis2hh12Sample, rx dataInfo error\n");
        return -1;
    }

    SPI_READ(LIS2HH12_FIFO_SRC_REG_ADDR, 1, &mTask.statusBuffer);

    mTask.spiCallBack = spiCallBack;
    return spiBatchTxRx(&mTask.mode, spiIsrCallBack, next_state);
}

static int lis2hh12ReadFifo(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    uint8_t fifo_status = mTask.statusBuffer[1];

    mTask.fifoDataToRead = fifo_status & LIS2HH12_FIFO_CTRL2_FTH_MASK;

    mTask.fifoDataToRead = mTask.fifoDataToRead * LIS2HH12_ONE_SAMPLE_BYTE;
    SPI_READ(LIS2HH12_OUT_X_L_ADDR, mTask.fifoDataToRead, &mTask.regBuffer);
    // osLog(LOG_INFO, "lis2hh12ReadFifo length:%d\n", mTask.fifoDataToRead);

#if 0
    if (temp_status & 0x04) {
        mTask.tempReady = true;
        SPI_READ(LIS2HH12_OUT_TEMP_L_ADDR, 2, &mTask.tempBuffer);
    } else
        mTask.tempReady = false;
#endif

    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state);
}

static void parseRawData(struct accGyroData *data, uint8_t *buf, uint8_t sensorType) {
    int16_t raw_data[AXES_NUM] = {0};
    int16_t remap_data[AXES_NUM] = {0};
    int32_t SwCali[AXES_NUM] = {0};

    int32_t caliResult[AXES_NUM] = {0};
    float temp_data[AXES_NUM] = {0};
    float calibrated_data_output[AXES_NUM] = {0};
    int16_t status = 0;

    if (sensorType == SENS_TYPE_ACCEL)
        accGetCalibration(SwCali, 0);

    raw_data[AXIS_X] = (buf[0] | buf[1] << 8);
    raw_data[AXIS_Y] = 0 - (buf[2] | buf[3] << 8); // workaround for sensor coordinate axis incorrect
    raw_data[AXIS_Z] = (buf[4] | buf[5] << 8);


    raw_data[AXIS_X] = raw_data[AXIS_X] + SwCali[AXIS_X];
    raw_data[AXIS_Y] = raw_data[AXIS_Y] + SwCali[AXIS_Y];
    raw_data[AXIS_Z] = raw_data[AXIS_Z] + SwCali[AXIS_Z];

    remap_data[mTask.cvt.map[AXIS_X]] = mTask.cvt.sign[AXIS_X] * raw_data[AXIS_X];
    remap_data[mTask.cvt.map[AXIS_Y]] = mTask.cvt.sign[AXIS_Y] * raw_data[AXIS_Y];
    remap_data[mTask.cvt.map[AXIS_Z]] = mTask.cvt.sign[AXIS_Z] * raw_data[AXIS_Z];

    //osLog(LOG_ERROR, "ACCEL:raw_data_x=%d, raw_data_y=%d, raw_data_z=%d\n",
        //remap_data[0], remap_data[1], remap_data[2]);

    if (sensorType == SENS_TYPE_ACCEL) {
        temp_data[AXIS_X] = (float)remap_data[AXIS_X] * GRAVITY_EARTH_SCALAR / mTask.sensors[ACC].sensitivity;
        temp_data[AXIS_Y] = (float)remap_data[AXIS_Y] * GRAVITY_EARTH_SCALAR / mTask.sensors[ACC].sensitivity;
        temp_data[AXIS_Z] = (float)remap_data[AXIS_Z] * GRAVITY_EARTH_SCALAR / mTask.sensors[ACC].sensitivity;
        //Reserve Log for Debug x,y,z stdev should under 0.04 for static cali calculate can success
        //osLog(LOG_ERROR, "temp_data[AXIS_X]=%f, temp_data[AXIS_Y]=%f, temp_data[AXIS_Z]=%f\n",
        //            (double)temp_data[AXIS_X], (double)temp_data[AXIS_Y], (double)temp_data[AXIS_Z]);

        if (UNLIKELY(mTask.sensors[ACC].startCali)) {
#ifndef CFG_MTK_CALIBRATION_V1_0
            status = Acc_run_factory_calibration_timeout(0,
                temp_data, calibrated_data_output, (int *)&mTask.sensors[ACC].accuracy, rtcGetTime());
            if (status != 0) {
                mTask.sensors[ACC].startCali = false;
                if (status > 0) {
                    osLog(LOG_INFO, "ACC cali detect shake %lld\n", rtcGetTime());
                    caliResult[AXIS_X] = (int32_t)(mTask.sensors[ACC].staticCali[AXIS_X] * 1000);
                    caliResult[AXIS_Y] = (int32_t)(mTask.sensors[ACC].staticCali[AXIS_Y] * 1000);
                    caliResult[AXIS_Z] = (int32_t)(mTask.sensors[ACC].staticCali[AXIS_Z] * 1000);
                    accGyroSendCalibrationResult(SENS_TYPE_ACCEL, (int32_t *)&caliResult[0], (uint8_t)status);
                } else
                    osLog(LOG_INFO, "ACC cali time out %lld\n", rtcGetTime());
            } else if (mTask.sensors[ACC].accuracy == 3) {
                mTask.sensors[ACC].startCali = false;
                mTask.sensors[ACC].staticCali[AXIS_X] = calibrated_data_output[AXIS_X] - temp_data[AXIS_X];
                mTask.sensors[ACC].staticCali[AXIS_Y] = calibrated_data_output[AXIS_Y] - temp_data[AXIS_Y];
                mTask.sensors[ACC].staticCali[AXIS_Z] = calibrated_data_output[AXIS_Z] - temp_data[AXIS_Z];
                caliResult[AXIS_X] = (int32_t)(mTask.sensors[ACC].staticCali[AXIS_X] * 1000);
                caliResult[AXIS_Y] = (int32_t)(mTask.sensors[ACC].staticCali[AXIS_Y] * 1000);
                caliResult[AXIS_Z] = (int32_t)(mTask.sensors[ACC].staticCali[AXIS_Z] * 1000);
                accGyroSendCalibrationResult(SENS_TYPE_ACCEL, (int32_t *)&caliResult[0], (uint8_t)status);
                osLog(LOG_INFO, "ACC cali done %lld:caliResult[0]:%ld, caliResult[1]:%ld, caliResult[2]:%ld, offset[0]:%f, offset[1]:%f, offset[2]:%f\n",
                    rtcGetTime(), caliResult[AXIS_X], caliResult[AXIS_Y], caliResult[AXIS_Z],
                    (double)mTask.sensors[ACC].staticCali[AXIS_X],
                    (double)mTask.sensors[ACC].staticCali[AXIS_Y],
                    (double)mTask.sensors[ACC].staticCali[AXIS_Z]);
            }
#else
            status = accFactoryCalibrateRun(&mTask.accFactoryCal,
                temp_data[AXIS_X], temp_data[AXIS_Y], temp_data[AXIS_Z], rtcGetTime());
            if (status == UNSTABLE_DETECT) {
                mTask.sensors[ACC].startCali = false;
                caliResult[AXIS_X] = (int32_t)(mTask.sensors[ACC].staticCali[AXIS_X] * 1000);
                caliResult[AXIS_Y] = (int32_t)(mTask.sensors[ACC].staticCali[AXIS_Y] * 1000);
                caliResult[AXIS_Z] = (int32_t)(mTask.sensors[ACC].staticCali[AXIS_Z] * 1000);
                accGyroSendCalibrationResult(SENS_TYPE_ACCEL, (int32_t *)&caliResult[0], (uint8_t)status);
            } else if (status == STILL_DETECT) {
                mTask.sensors[ACC].startCali = false;
                accFactoryCalibrateRemoveBias(&mTask.accFactoryCal,
                    temp_data[AXIS_X], temp_data[AXIS_Y], temp_data[AXIS_Z],
                    &calibrated_data_output[AXIS_X], &calibrated_data_output[AXIS_Y],
                    &calibrated_data_output[AXIS_Z]);
                accFactoryCalibrateGetBias(&mTask.accFactoryCal,
                    &mTask.sensors[ACC].staticCali[AXIS_X],
                    &mTask.sensors[ACC].staticCali[AXIS_Y],
                    &mTask.sensors[ACC].staticCali[AXIS_Z],
                    (int *)&mTask.sensors[ACC].accuracy);
                caliResult[AXIS_X] = (int32_t)(mTask.sensors[ACC].staticCali[AXIS_X] * 1000);
                caliResult[AXIS_Y] = (int32_t)(mTask.sensors[ACC].staticCali[AXIS_Y] * 1000);
                caliResult[AXIS_Z] = (int32_t)(mTask.sensors[ACC].staticCali[AXIS_Z] * 1000);
                accGyroSendCalibrationResult(SENS_TYPE_ACCEL, (int32_t *)&caliResult[0], (uint8_t)status);
                osLog(LOG_INFO, "accel cali bias: [%f, %f, %f]\n",
                    (double)mTask.sensors[ACC].staticCali[AXIS_X],
                    (double)mTask.sensors[ACC].staticCali[AXIS_Y],
                    (double)mTask.sensors[ACC].staticCali[AXIS_Z]);
            }
#endif
        }


        data->sensType = sensorType;

#ifndef CFG_MTK_CALIBRATION_V1_0
        data->x = temp_data[AXIS_X] + mTask.sensors[ACC].staticCali[AXIS_X];
        data->y = temp_data[AXIS_Y] + mTask.sensors[ACC].staticCali[AXIS_Y];
        data->z = temp_data[AXIS_Z] + mTask.sensors[ACC].staticCali[AXIS_Z];
#else
        data->x = temp_data[AXIS_X] - mTask.sensors[ACC].staticCali[AXIS_X];
        data->y = temp_data[AXIS_Y] - mTask.sensors[ACC].staticCali[AXIS_Y];
        data->z = temp_data[AXIS_Z] - mTask.sensors[ACC].staticCali[AXIS_Z];
#endif

        mTask.accFactoryData.ix = (int32_t)(data->x * ACCELEROMETER_INCREASE_NUM_AP);
        mTask.accFactoryData.iy = (int32_t)(data->y * ACCELEROMETER_INCREASE_NUM_AP);
        mTask.accFactoryData.iz = (int32_t)(data->z * ACCELEROMETER_INCREASE_NUM_AP);

        //osLog(LOG_ERROR, "ACCEL:staticCali_x=%d, staticCali_y=%d, staticCali_z=%d\n",
            //mTask.sensors[ACC].staticCali[AXIS_X], mTask.sensors[ACC].staticCali[AXIS_Y],
            //mTask.sensors[ACC].staticCali[AXIS_Z]);

    }

    if (mTask.debug_trace) {
        switch (sensorType) {
            case SENS_TYPE_ACCEL:
                osLog(LOG_ERROR, "ACCEL:raw_data_x=%f, raw_data_y=%f, raw_data_z=%f\n",
                    (double)data->x, (double)data->y, (double)data->z);
                break;
            default:
                break;
        }
    }
}

static int lis2hh12Convert(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    struct accGyroData *data = mTask.accGyroPacket.outBuf;
    uint8_t accEventSize = 0;
    uint64_t swSampleTime = 0, realSampleTime = 0;
    uint8_t *fifo_data = &mTask.regBuffer[1];
    uint16_t fifo_offset = 0;

    while (fifo_offset < mTask.fifoDataToRead) {
        if (mTask.sensors[ACC].samplesToDiscard > 0)
            mTask.sensors[ACC].samplesToDiscard--;
        else {
            if (accEventSize < MAX_RECV_PACKET) {
                 parseRawData(&data[accEventSize], &fifo_data[fifo_offset], SENS_TYPE_ACCEL);
                 accEventSize++;
            } else
                osLog(LOG_ERROR, "outBuf full, accEventSize = %d\n", accEventSize);
        }
        fifo_offset += LIS2HH12_ONE_SAMPLE_BYTE;
    }
#ifndef CFG_MTK_CALIBRATION_V1_0
    /*if startcali true , can't send to runtime cali in parseRawData to accGyro*/
    if (mTask.sensors[ACC].startCali) {
        accEventSize = 0;
        registerAccGyroFifoInfo((mTask.sensors[ACC].hwRate == 0) ?
            0 : 1024000000000 / mTask.sensors[ACC].hwRate, 0);
    }
#endif

    swSampleTime = mTask.swSampleTime;

    realSampleTime = calcFakeInterruptTime(swSampleTime, mTask.hwSampleTime, mTask.lastSampleTime,
        mTask.sensors[ACC].hwRate, mTask.sensors[ACC].configed, accEventSize, 0, 0, 0);

    // osLog(LOG_INFO, "lis2hh12Convert, swSampleTime=%lld, hwSampleTime=%lld,
    //realSampleTime=%lld, lastSampleTime=%lld, now=%lld, accSize:%d\n",
    //swSampleTime, mTask.hwSampleTime, realSampleTime,
    //mTask.lastSampleTime, rtcGetTime(), accEventSize);

    mTask.hwSampleTime = realSampleTime;
    mTask.lastSampleTime = realSampleTime;
    txTransferDataInfo(&mTask.dataInfo, accEventSize, 0, realSampleTime, data, 0);

    mt_eint_unmask(mTask.hw->eint_num);
    sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int lis2hh12SwReset(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {

    SPI_WRITE(LIS2HH12_CTRL_REG5_ADDR, LIS2HH12_SW_RESET, 50000);
    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state);

}

static int lis2hh12InitReg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    /* During init, reset all configurable registers to default values */

    SPI_WRITE(LIS2HH12_CTRL_REG1_ADDR, LIS2HH12_CTRL_REG1_BASE, 5); /*ENBLE X Y Z, BDU*/
    SPI_WRITE(LIS2HH12_CTRL_REG2_ADDR, LIS2HH12_CTRL_REG2_BASE, 5); /*HP filter Normal mode*/
    SPI_WRITE(LIS2HH12_CTRL_REG3_ADDR, LIS2HH12_CTRL_REG3_BASE, 5); /*WTM, OVERRUN on INT1 pin*/
    SPI_WRITE(LIS2HH12_CTRL_REG4_ADDR, LIS2HH12_CTRL_REG4_BASE, 5); /*FS=+/-8g, BW 400HZ, MLTI Read*/
    SPI_WRITE(LIS2HH12_CTRL_REG5_ADDR, LIS2HH12_CTRL_REG5_BASE, 5); /* ACTIVE HIGH, PUSH PULL*/
    SPI_WRITE(LIS2HH12_CTRL_REG7_ADDR, LIS2HH12_CTRL_REG7_BASE, 5); /* INT NO Latched*/

    SPI_WRITE(LIS2HH12_FIFO_CTRL_REG_ADDR, LIS2HH12_FIFO_CTRL_REG_BASE); /* FIFO  bypass mode*/ //
    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state);
}

static void sensorCoreRegistration(void) {
    struct sensorCoreInfo mInfo;
    memset(&mInfo, 0x00, sizeof(struct sensorCoreInfo));
    /* Register sensor Core */
    mInfo.sensType = SENS_TYPE_ACCEL;
    mInfo.gain = GRAVITY_EARTH_1000;
    mInfo.sensitivity = mTask.sensors[ACC].sensitivity;
    mInfo.cvt = mTask.cvt;
    mInfo.getCalibration = accGetCalibration;
    mInfo.setCalibration = accSetCalibration;
    mInfo.getData = accGetData;
    mInfo.setDebugTrace = lis2hh12SetDebugTrace;
    mInfo.getSensorInfo = accGetSensorInfo;
    sensorCoreRegister(&mInfo);

    memset(&mInfo, 0x00, sizeof(struct sensorCoreInfo));
    mInfo.sensType = SENS_TYPE_GYRO;
    mInfo.getSensorInfo = gyroGetSensorInfo;
    sensorCoreRegister(&mInfo);
}

static int lis2hh12SensorRegistration(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    sensorCoreRegistration();
    sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int lis2hh12EintRegistration(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mt_eint_dis_hw_debounce(mTask.hw->eint_num);
    mt_eint_registration(mTask.hw->eint_num, LEVEL_SENSITIVE, HIGH_LEVEL_TRIGGER, lis2hh12Isr1, EINT_INT_UNMASK,
        EINT_INT_AUTO_UNMASK_OFF);
    sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static struct sensorFsm lis2hh12Fsm[] = {
    sensorFsmCmd(STATE_HW_INT_STATUS_CHECK, STATE_HW_INT_HANDLING, lis2hh12IntStatusCheck),
    sensorFsmCmd(STATE_HW_INT_HANDLING, STATE_HW_INT_HANDLING_DONE, lis2hh12IntHandling),

    sensorFsmCmd(STATE_SAMPLE, STATE_FIFO, lis2hh12Sample),
    sensorFsmCmd(STATE_FIFO, STATE_CONVERT, lis2hh12ReadFifo),
    sensorFsmCmd(STATE_CONVERT, STATE_SAMPLE_DONE, lis2hh12Convert),

    sensorFsmCmd(STATE_ACC_ENABLE, STATE_ACC_ENABLE_DONE, lis2hh12AccPowerOn),
    sensorFsmCmd(STATE_ACC_DISABLE, STATE_ACC_DISABLE_DONE, lis2hh12AccPowerOff),
    sensorFsmCmd(STATE_ACC_RATECHG, STATE_ACC_RATECHG_DONE, lis2hh12AccRate),

    sensorFsmCmd(STATE_ACC_CALI, STATE_ACC_CALI_DONE, lis2hh12AccCali),
    sensorFsmCmd(STATE_ACC_CFG, STATE_ACC_CFG_DONE, lis2hh12AccCfgCali),

    sensorFsmCmd(STATE_ACC_SELF_TEST, STATE_ACC_SELF_TEST_DONE, lis2hh12AccSelfTest),

    sensorFsmCmd(STATE_SW_RESET, STATE_INIT_REG, lis2hh12SwReset),
    sensorFsmCmd(STATE_INIT_REG, STATE_SENSOR_REGISTRATION, lis2hh12InitReg),
    sensorFsmCmd(STATE_SENSOR_REGISTRATION, STATE_EINT_REGISTRATION, lis2hh12SensorRegistration),
    sensorFsmCmd(STATE_EINT_REGISTRATION, STATE_INIT_DONE, lis2hh12EintRegistration),

    /* For Anymotion */
    sensorFsmCmd(STATE_ANYMO_ENABLE, STATE_ANYMO_ENABLE_DONE, anyMotionPowerOn),
    sensorFsmCmd(STATE_ANYMO_DISABLE, STATE_ANYMO_DISABLE_DONE, anyMotionPowerOff),
};

static void initSensorStruct(struct lis2hh12Sensor *sensor, enum SensorIndex idx) {
    sensor->powered = false;
    sensor->configed = false;
    sensor->rate = 0;
    sensor->latency = SENSOR_LATENCY_NODATA;
    sensor->hwRate = 0;  // rate set in hw

    sensor->startCali = false;
    sensor->staticCali[AXIS_X] = 0;
    sensor->staticCali[AXIS_Y] = 0;
    sensor->staticCali[AXIS_Z] = 0;
    sensor->accuracy = 0;
}

int lis2hh12Init(void) {
    int ret = 0;
    uint8_t txData[2] = {0}, rxData[2] = {0};
    enum SensorIndex i;
    insertMagicNum(&mTask.accGyroPacket);
    mTask.hw = get_cust_accGyro("lis2hh12");
    if (NULL == mTask.hw) {
        osLog(LOG_ERROR, "get_cust_acc_hw fail\n");
        ret = -1;
        goto err_out;
    }
    osLog(LOG_INFO, "acc spi_num: %d\n", mTask.hw->i2c_num);
    if (0 != (ret = sensorDriverGetConvert(mTask.hw->direction, &mTask.cvt))) {
        osLog(LOG_ERROR, "invalid direction: %d\n", mTask.hw->direction);
    }
    osLog(LOG_ERROR, "acc map[0]:%d, map[1]:%d, map[2]:%d, sign[0]:%d, sign[1]:%d, sign[2]:%d\n\r",
        mTask.cvt.map[AXIS_X], mTask.cvt.map[AXIS_Y], mTask.cvt.map[AXIS_Z],
        mTask.cvt.sign[AXIS_X], mTask.cvt.sign[AXIS_Y], mTask.cvt.sign[AXIS_Z]);
    mTask.sensors[ACC].sensitivity = (float)65536 / (8 * 2);  // +/-8g

    mTask.watermark = 0;
    mTask.temperature = 25.0f;  // init with 25 Celsius
    mTask.tempReady = false;
    mTask.fifoDataToRead = 0;

    mTask.latch_time_id = alloc_latch_time();
    enable_latch_time(mTask.latch_time_id, mTask.hw->eint_num);

    for (i = ACC; i < NUM_OF_SENSOR; i++) {
        initSensorStruct(&mTask.sensors[i], i);
    }
    mTask.mode.speed = 8000000;    //8Mhz
    mTask.mode.bitsPerWord = 8;
    mTask.mode.cpol = SPI_CPOL_IDLE_LO;
    mTask.mode.cpha = SPI_CPHA_LEADING_EDGE;
    mTask.mode.nssChange = true;
    mTask.mode.format = SPI_FORMAT_MSB_FIRST;
    mTask.mWbufCnt = 0;
    mTask.mRegCnt = 0;
    spiMasterRequest(mTask.hw->i2c_num, &mTask.spiDev);
    txData[0] = LIS2HH12_WAI_ADDR | 0x80;
    for (uint8_t i = 0; i < 3;) {
        ret = spiMasterRxTxSync(mTask.spiDev, rxData, txData, 2);
        if (ret >= 0 && (rxData[1] == LIS2HH12_WAI_VALUE))
            break;
        ++i;
        if (i >= 3) {
            ret = -1;
            sendSensorErrToAp(ERR_SENSOR_ACC, ERR_CASE_ACC_INIT, ACC_NAME);
            spiMasterRelease(mTask.spiDev);
            disable_latch_time(mTask.latch_time_id);
            free_latch_time(mTask.latch_time_id);
            goto err_out;
        }
    }
    osLog(LOG_INFO, "lis2hh12 auto detect success: %02x\n", rxData[1]);
    accSensorRegister();
    anyMotionSensorRegister();
    registerAccGyroInterruptMode(ACC_GYRO_FIFO_INTERRUPTIBLE);
    registerAccGyroDriverFsm(lis2hh12Fsm, ARRAY_SIZE(lis2hh12Fsm));

#ifdef CFG_MTK_CALIBRATION_V1_0
    const struct AccFactoryCalParameters acc_factory_cal_parameters = {
        .param = {
            .var_threshold = 8e-3f,
            .bias_mean_limit = 0.5f,
            .debounce_time_duration_nanos = SEC_TO_NANOS(0.5f),
            .window_time_duration_nanos = SEC_TO_NANOS(1),
            .standard_x = 0.0f,
            .standard_y = 0.0f,
            .standard_z = 9.807f,
            .factory_calibration_enable = true,
        },
    };
    accFactoryCalibrateParam(&mTask.accFactoryCal, &acc_factory_cal_parameters);
#endif
err_out:
    return ret;
}
#ifndef CFG_OVERLAY_INIT_SUPPORT
MODULE_DECLARE(lis2hh12, SENS_TYPE_ACCEL, lis2hh12Init);
#else
#include "mtk_overlay_init.h"
OVERLAY_DECLARE(lis2hh12, OVERLAY_ID_ACCGYRO, lis2hh12Init);
#endif

