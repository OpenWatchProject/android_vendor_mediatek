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

#define ACC_NAME     "lsm6dsm_acc"
#define GYRO_NAME     "lsm6dsm_gyro"
#define LSM6DSM_WAI_VALUE                             (0x6a)

#define kScale_acc    0.00239364f     // ACC_range * 9.81f / 32768.0f;
#define kScale_gyr    (DEGREE_TO_RADIRAN_SCALAR * 70 / 1000.0f) //0.00106472439f  // GYR_range * M_PI / (180.0f * 32768.0f);
#define kScale_temp   0.00390625f    // temperature in deg C

/* One sample of triaxial sensor is expressed on 6 byte */
#define LSM6DSM_ONE_SAMPLE_BYTE                       6

/* LSM6DSM registers */
#define LSM6DSM_FUNC_CFG_ACCESS_ADDR                  (0x01)
#define LSM6DSM_FIFO_CTRL1_ADDR                       (0x06)
#define LSM6DSM_FIFO_CTRL2_ADDR                       (0x07)
#define LSM6DSM_FIFO_CTRL3_ADDR                       (0x08)
#define LSM6DSM_FIFO_CTRL4_ADDR                       (0x09)
#define LSM6DSM_FIFO_CTRL5_ADDR                       (0x0a)
#define LSM6DSM_DRDY_PULSE_CFG_ADDR                   (0x0b)
#define LSM6DSM_INT1_CTRL_ADDR                        (0x0d)
#define LSM6DSM_INT2_CTRL_ADDR                        (0x0e)
#define LSM6DSM_WAI_ADDR                              (0x0f)
#define LSM6DSM_CTRL1_XL_ADDR                         (0x10)
#define LSM6DSM_CTRL2_G_ADDR                          (0x11)
#define LSM6DSM_CTRL3_C_ADDR                          (0x12)
#define LSM6DSM_CTRL4_C_ADDR                          (0x13)
#define LSM6DSM_CTRL5_C_ADDR                          (0x14)
#define LSM6DSM_CTRL6_C_ADDR                          (0x15)
#define LSM6DSM_CTRL10_C_ADDR                         (0x19)
#define LSM6DSM_MASTER_CONFIG_ADDR                    (0x1a)
#define LSM6DSM_WAKEUP_SRC_ADDR                       (0x1b)
#define LSM6DSM_STATUS_REG_ADDR                       (0x1e)
#define LSM6DSM_OUT_TEMP_L_ADDR                       (0x20)
#define LSM6DSM_OUT_TEMP_H_ADDR                       (0x21)
#define LSM6DSM_OUTX_L_G_ADDR                         (0x22)
#define LSM6DSM_OUTX_L_XL_ADDR                        (0x28)
#define LSM6DSM_SENSORHUB1_REG_ADDR                   (0x2e)
#define LSM6DSM_FIFO_STATUS1_ADDR                     (0x3a)
#define LSM6DSM_FIFO_STATUS2_ADDR                     (0x3b)
#define LSM6DSM_FIFO_DATA_OUT_L_ADDR                  (0x3e)
#define LSM6DSM_STEP_COUNTER_L_ADDR                   (0x4b)
#define LSM6DSM_FUNC_SRC_ADDR                         (0x53)
#define LSM6DSM_TAP_CFG_ADDR                          (0x58)
#define LSM6DSM_WAKE_UP_THS                           (0x5b)
#define LSM6DSM_WAKE_UP_DUR_ADDR                      (0x5c)
#define LSM6DSM_MD1_CFG_ADDR                          (0x5e)

#define LSM6DSM_PEDO_THS_REG                          (0x0F)
#define LSM6DSM_PEDO_DEB_REG                          (0x14)

#define LSM6DSM_SW_RESET                              (0x01)
#define LSM6DSM_RESET_PEDOMETER                       (0x02)
#define LSM6DSM_ENABLE_FUNC_CFG_ACCESS                (0x80)
#define LSM6DSM_ENABLE_DIGITAL_FUNC                   (0x04)
#define LSM6DSM_ENABLE_PEDOMETER_DIGITAL_FUNC         (0x10)
#define LSM6DSM_ENABLE_SIGN_MOTION_DIGITAL_FUNC       (0x01)
#define LSM6DSM_ENABLE_TIMER_DIGITAL_FUNC             (0x20)
#define LSM6DSM_MASTER_CONFIG_PULL_UP_EN              (0x08)
#define LSM6DSM_MASTER_CONFIG_MASTER_ON               (0x01)
#define LSM6DSM_MASTER_CONFIG_DRDY_ON_INT1            (0x80)

/* LSM6DSM fifo modes */
#define LSM6DSM_WAKEUP_MASK                           (0x0f)
#define LSM6DSM_XL_LOWPOWER_MODE                      (0x10)
#define LSM6DSM_WU_INIT1_ENABLE                       (0x20)
#define LSM6DSM_TAP_INTERRUPTS_ENABLE                 (0x80)

/* LSM6DSM fifo status */
#define LSM6DSM_FIFO_STATUS2_FIFO_EMPTY               (0x10)
#define LSM6DSM_FIFO_STATUS2_FIFO_FULL_SMART          (0x20)
#define LSM6DSM_FIFO_STATUS2_FIFO_FULL_OVERRUN        (0x40)
#define LSM6DSM_FIFO_STATUS2_FIFO_ERROR               (LSM6DSM_FIFO_STATUS2_FIFO_EMPTY | \
                                                       LSM6DSM_FIFO_STATUS2_FIFO_FULL_SMART | \
                                                       LSM6DSM_FIFO_STATUS2_FIFO_FULL_OVERRUN)
#define LSM6DSM_FIFO_STATUS2_FIFO_WATERMARK           (0x80)

/* LSM6DSM fifo modes */
#define LSM6DSM_FIFO_BYPASS_MODE                      (0x00)
#define LSM6DSM_FIFO_CONTINUOS_MODE                   (0x36)
#define LSM6DSM_FIFO_CTRL2_FTH_MASK                   (0x07)

/* LSM6DSM fifo decimators */
#define LSM6DSM_FIFO_SAMPLE_NOT_IN_FIFO               (0x00)
#define LSM6DSM_FIFO_NO_DECIMATION                    (0x01)
#define LSM6DSM_FIFO_DECIMATION_FACTOR_2              (0x02)
#define LSM6DSM_FIFO_DECIMATION_FACTOR_3              (0x03)
#define LSM6DSM_FIFO_DECIMATION_FACTOR_4              (0x04)
#define LSM6DSM_FIFO_DECIMATION_FACTOR_8              (0x05)
#define LSM6DSM_FIFO_DECIMATION_FACTOR_16             (0x06)
#define LSM6DSM_FIFO_DECIMATION_FACTOR_32             (0x07)

#define LSM6DSM_FIFOOFF_CONTINUE_REG_VALUE            (0x06)
#define LSM6DSM_FIFO12HZ_CONTINUE_REG_VALUE           (0x0e)
#define LSM6DSM_FIFO26HZ_CONTINUE_REG_VALUE           (0x16)
#define LSM6DSM_FIFO52HZ_CONTINUE_REG_VALUE           (0x1e)
#define LSM6DSM_FIFO104HZ_CONTINUE_REG_VALUE          (0x26)
#define LSM6DSM_FIFO208HZ_CONTINUE_REG_VALUE          (0x2e)
#define LSM6DSM_FIFO416HZ_CONTINUE_REG_VALUE          (0x36)

/*LSM6DSM ODR RATE related*/
#define LSM6DSM_ODR_DELAY_US_GYRO_POWER_ON              80000
#define LSM6DSM_ODR_12HZ_ACCEL_STD                      1
#define LSM6DSM_ODR_26HZ_ACCEL_STD                      1
#define LSM6DSM_ODR_52HZ_ACCEL_STD                      1
#define LSM6DSM_ODR_104HZ_ACCEL_STD                     1
#define LSM6DSM_ODR_208HZ_ACCEL_STD                     1
#define LSM6DSM_ODR_416HZ_ACCEL_STD                     1
#define LSM6DSM_ODR_12HZ_GYRO_STD                       2
#define LSM6DSM_ODR_26HZ_GYRO_STD                       3
#define LSM6DSM_ODR_52HZ_GYRO_STD                       3
#define LSM6DSM_ODR_104HZ_GYRO_STD                      3
#define LSM6DSM_ODR_208HZ_GYRO_STD                      3
#define LSM6DSM_ODR_416HZ_GYRO_STD                      3

#define LSM6DSM_ODR_12HZ_REG_VALUE                    (0x10)
#define LSM6DSM_ODR_26HZ_REG_VALUE                    (0x20)
#define LSM6DSM_ODR_52HZ_REG_VALUE                    (0x30)
#define LSM6DSM_ODR_104HZ_REG_VALUE                   (0x40)
#define LSM6DSM_ODR_208HZ_REG_VALUE                   (0x50)
#define LSM6DSM_ODR_416HZ_REG_VALUE                   (0x60)



/* LSM6DSM default base registers status */
/* LSM6DSM_FUNC_CFG_ACCESS_BASE: enable embedded functions register */
#define LSM6DSM_FUNC_CFG_ACCESS_BASE                  (0x00)

/*the follow register set is to operate the embeded function in unusual register, such as pedo threshold etc. */
#define LSM6DSM_FUNC_CFG_ACCESS_FUNC_CFG_EN           (0x80)


/* LSM6DSM_DRDY_PULSE_CFG_BASE: enable pulsed interrupt register */
#define LSM6DSM_DRDY_PULSE_CFG_BASE                   (0x00)

/* LSM6DSM_INT1_CTRL_BASE: interrupt 1 control register default settings */
#define LSM6DSM_INT1_CTRL_BASE                       ((0 << 7) |    /* INT1_STEP_DETECTOR */ \
                                                      (0 << 6) |    /* INT1_SIGN_MOT */ \
                                                      (1 << 5) |    /* INT1_FULL_FLAG */ \
                                                      (1 << 4) |    /* INT1_FIFO_OVR */ \
                                                      (1 << 3) |    /* INT1_FTH */ \
                                                      (0 << 2) |    /* INT1_BOOT */ \
                                                      (0 << 1) |    /* INT1_DRDY_G */ \
                                                      (0 << 0))     /* INT1_DRDY_XL */

/* LSM6DSM_INT2_CTRL_BASE: interrupt 2 control register default settings */
#define LSM6DSM_INT2_CTRL_BASE                       ((0 << 7) |    /* INT2_STEP_DELTA */ \
                                                      (0 << 6) |    /* INT2_STEP_OV */ \
                                                      (0 << 5) |    /* INT2_FULL_FLAG */ \
                                                      (0 << 4) |    /* INT2_FIFO_OVR */ \
                                                      (0 << 3) |    /* INT2_FTH */ \
                                                      (0 << 2) |    /* INT2_DRDY_TEMP */ \
                                                      (0 << 1) |    /* INT2_DRDY_G */ \
                                                      (0 << 0))     /* INT2_DRDY_XL */

/* LSM6DSM_CTRL1_XL_BASE: accelerometer sensor register default settings */
#define LSM6DSM_CTRL1_XL_BASE                        ((0 << 7) |    /* ODR_XL3 */ \
                                                      (0 << 6) |    /* ODR_XL2 */ \
                                                      (0 << 5) |    /* ODR_XL1 */ \
                                                      (0 << 4) |    /* ODR_XL0 */ \
                                                      (1 << 3) |    /* FS_XL1 */ \
                                                      (1 << 2) |    /* FS_XL0 */ \
                                                      (1 << 1) |    /* LPF1_BW_SEL */ \
                                                      (0 << 0))     /* (0) */

/* LSM6DSM_CTRL2_G_BASE: gyroscope sensor register default settings */
#define LSM6DSM_CTRL2_G_BASE                         ((0 << 7) |    /* ODR_G3 */ \
                                                      (0 << 6) |    /* ODR_G2 */ \
                                                      (0 << 5) |    /* ODR_G1 */ \
                                                      (0 << 4) |    /* ODR_G0 */ \
                                                      (1 << 3) |    /* FS_G1 */ \
                                                      (1 << 2) |    /* FS_G0 */ \
                                                      (0 << 1) |    /* FS_125 */ \
                                                      (0 << 0))     /* (0) */

/* LSM6DSM_CTRL3_C_BASE: control register 3 default settings */
#define LSM6DSM_CTRL3_C_BASE                         ((0 << 7) |    /* BOOT */ \
                                                      (1 << 6) |    /* BDU */ \
                                                      (0 << 5) |    /* H_LACTIVE */ \
                                                      (0 << 4) |    /* PP_OD */ \
                                                      (0 << 3) |    /* SIM */ \
                                                      (1 << 2) |    /* IF_INC */ \
                                                      (0 << 1) |    /* BLE */ \
                                                      (0 << 0))     /* SW_RESET */

/* LSM6DSM_CTRL4_C_BASE: control register 4 default settings */
#define LSM6DSM_CTRL4_C_BASE                         ((0 << 7) |    /* DEN_XL_EN */ \
                                                      (0 << 6) |    /* SLEEP */ \
                                                      (1 << 5) |    /* INT2_on_INT1 */ \
                                                      (0 << 4) |    /* DEN_DRDY_MASK */ \
                                                      (0 << 3) |    /* DRDY_MASK */ \
                                                      (1 << 2) |    /* I2C_disable */ \
                                                      (0 << 1) |    /* LPF1_SEL_G */ \
                                                      (0 << 0))     /* (0) */

/* LSM6DSM_CTRL5_C_BASE: control register 5 default settings */
#define LSM6DSM_CTRL5_C_BASE                          (0x00)

/* LSM6DSM_CTRL6_C_BASE: control register 6 default settings */
#define LSM6DSM_CTRL6_C_BASE                          (0x00)


/* LSM6DSM_CTRL10_C_BASE: control register 10 default settings */
#define LSM6DSM_CTRL10_C_BASE                        ((0 << 7) |    /* (0) */ \
                                                      (0 << 6) |    /* (0) */ \
                                                      (0 << 5) |    /* TIMER_EN */ \
                                                      (0 << 4) |    /* PEDO_EN */ \
                                                      (0 << 3) |    /* TILT_EN */ \
                                                      (0 << 2) |    /* FUNC_EN */ \
                                                      (0 << 1) |    /* PEDO_RST_STEP */ \
                                                      (0 << 0))     /* SIGN_MOTION_EN */

#define LSM6DSM_CTRL10_C_FUNC_EN                      0x04          /*Embeded Function Enable, such as pedo, timer, tilt*/
#define LSM6DSM_CTRL10_C_PEDO_EN                      0x10          /*Pedo enable*/



#define LSM6DSM_MD1_CFG_BASE                         ((0 << 7) |    /* INT1_INACT_STATE */ \
                                                      (0 << 6) |    /* INT1_SINGLE_TAP*/ \
                                                      (0 << 5) |    /* INT1_WU */ \
                                                      (0 << 4) |    /* INT1_FF */ \
                                                      (0 << 3) |    /* INT1_DOUBLE_TAP */ \
                                                      (0 << 2) |    /* INT1_6D */ \
                                                      (0 << 1) |    /* INT1_TILT*/ \
                                                      (0 << 0))     /* INT1_TIMER*/


/* LSM6DSM_TAP_CFG_BASE: interrupt 1 control  */
#define LSM6DSM_TAP_CFG_BASE                         ((0 << 7) |    /* INTERRUPTS_ENABLE */ \
                                                      (0 << 6) |    /*  */ \
                                                      (0 << 5) |    /*  */ \
                                                      (0 << 4) |    /* SLODPE_EN */ \
                                                      (0 << 3) |    /* ... */ \
                                                      (0 << 2) |    /* ... */ \
                                                      (0 << 1) |    /* TAP_ZEN */ \
                                                      (0 << 0))     /* DISABLE LIR  latch mode,TRIGGER MODE */

#define LSM6DSM_WAKE_UP_DUR_BASE                      (0x00)


#define max(x, y)   (x > y ? x : y)

#define SPI_PACKET_SIZE 30
#define FIFO_DATA_SIZE  (MAX_RECV_PACKET * LSM6DSM_ONE_SAMPLE_BYTE)//limit to accGyro common data buffer size
#define TEMPERATURE_SIZE (2)
#define SPI_BUF_SIZE    ((FIFO_DATA_SIZE + 4) + (TEMPERATURE_SIZE + 4) + 4)

#define SPI_WRITE_0(addr, data) spiQueueWrite(addr, data, 2)
#define SPI_WRITE_1(addr, data, delay) spiQueueWrite(addr, data, delay)
#define GET_SPI_WRITE_MACRO(_1, _2, _3, NAME, ...) NAME
#define SPI_WRITE(...) GET_SPI_WRITE_MACRO(__VA_ARGS__, SPI_WRITE_1, SPI_WRITE_0)(__VA_ARGS__)

#define SPI_READ_0(addr, size, buf) spiQueueRead(addr, size, buf, 0)
#define SPI_READ_1(addr, size, buf, delay) spiQueueRead(addr, size, buf, delay)
#define GET_SPI_READ_MACRO(_1, _2, _3, _4, NAME, ...) NAME
#define SPI_READ(...) GET_SPI_READ_MACRO(__VA_ARGS__, SPI_READ_1, SPI_READ_0)(__VA_ARGS__)

#define EVT_SENSOR_ANY_MOTION       sensorGetMyEventType(SENS_TYPE_ANY_MOTION)
#ifdef CFG_HW_STEP_COUNTER_SUPPORT
#define EVT_SENSOR_STEP_COUNTER     sensorGetMyEventType(SENS_TYPE_STEP_COUNT)
#endif



enum LSM6DSMState {
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
    STATE_INIT_REG,
    STATE_SENSOR_REGISTRATION,
    STATE_EINT_REGISTRATION,
};

enum SensorIndex {
    ACC = 0,
    GYR,
    ANYMO,
#ifdef CFG_HW_STEP_COUNTER_SUPPORT
    STEPCNT,
#endif
    NUM_OF_SENSOR,
};

struct lsm6dsmSensor {
    bool powered;
    bool configed;
    bool lowPower;
    bool startCali;

    float staticCali[AXES_NUM];
    int32_t accuracy;

    uint32_t rate;
    uint64_t latency;
    uint32_t hwRate;  //rate set in hw
    uint32_t preRealRate;

    uint8_t samplesDecimator;
    uint8_t samplesDecimatorCounter;
    uint8_t samplesToDiscard;
    uint8_t decimators;
    uint8_t fifoSip;

    float sensitivity;
};

static struct LSM6DSMTask {
    struct lsm6dsmSensor sensors[NUM_OF_SENSOR];

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

    float temperature;
    bool tempReady;

    bool accGyroPowerd;
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
#ifdef CFG_HW_STEP_COUNTER_SUPPORT
    uint8_t *stepBuffer;
    uint32_t total_step_cnt;
    uint32_t last_step_cnt;
#endif

    /* data for factory */
    struct TripleAxisDataPoint accFactoryData;
    struct TripleAxisDataPoint gyroFactoryData;
    int32_t accSwCali[AXES_NUM];
    int32_t gyroSwCali[AXES_NUM];
    int32_t debug_trace;

    int latch_time_id;
#ifdef CFG_MTK_CALIBRATION_V1_0
    struct AccFactoryCal accFactoryCal;
    struct GyroFactoryCal gyroFactoryCal;
#endif
} mTask;

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

static void accGetCalibration(int32_t *cali, int32_t size)
{
    cali[AXIS_X] = mTask.accSwCali[AXIS_X];
    cali[AXIS_Y] = mTask.accSwCali[AXIS_Y];
    cali[AXIS_Z] = mTask.accSwCali[AXIS_Z];
    /* osLog(LOG_INFO, "accGetCalibration cali x:%d, y:%d, z:%d\n", cali[AXIS_X], cali[AXIS_Y], cali[AXIS_Z]); */
}
static void accSetCalibration(int32_t *cali, int32_t size)
{
    mTask.accSwCali[AXIS_X] = cali[AXIS_X];
    mTask.accSwCali[AXIS_Y] = cali[AXIS_Y];
    mTask.accSwCali[AXIS_Z] = cali[AXIS_Z];
    /* osLog(LOG_INFO, "accSetCalibration cali x:%d, y:%d, z:%d\n", mTask.accSwCali[AXIS_X],
        mTask.accSwCali[AXIS_Y], mTask.accSwCali[AXIS_Z]); */
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
    /* osLog(LOG_INFO, "gyroGetCalibration cali x:%d, y:%d, z:%d\n", cali[AXIS_X], cali[AXIS_Y], cali[AXIS_Z]); */
}
static void gyroSetCalibration(int32_t *cali, int32_t size)
{
    mTask.gyroSwCali[AXIS_X] = cali[AXIS_X];
    mTask.gyroSwCali[AXIS_Y] = cali[AXIS_Y];
    mTask.gyroSwCali[AXIS_Z] = cali[AXIS_Z];
    /* osLog(LOG_INFO, "gyroSetCalibration cali x:%d, y:%d, z:%d\n", mTask.gyroSwCali[AXIS_X],
        mTask.gyroSwCali[AXIS_Y], mTask.gyroSwCali[AXIS_Z]); */
}
static void gyroGetData(void *sample)
{
    struct TripleAxisDataPoint *tripleSample = (struct TripleAxisDataPoint *)sample;
    tripleSample->ix = mTask.gyroFactoryData.ix;
    tripleSample->iy = mTask.gyroFactoryData.iy;
    tripleSample->iz = mTask.gyroFactoryData.iz;
}


static void lsm6dsmSetDebugTrace(int32_t trace)
{

    mTask.debug_trace = trace;
    osLog(LOG_ERROR, "%s ==> trace:%d\n", __func__, mTask.debug_trace);
}

static void accGetSensorInfo(struct sensorInfo_t *data)
{
    strncpy(data->name, ACC_NAME, sizeof(data->name));
}

static void gyroGetSensorInfo(struct sensorInfo_t *data)
{
    strncpy(data->name, GYRO_NAME, sizeof(data->name));
}

static void lsm6dsmIsr1(int arg)
{
    if (mTask.latch_time_id < 0) {
        mTask.hwSampleTime = rtcGetTime();
        //osLog(LOG_ERROR, "lsm6dsmIsr1, real=%lld\n", mTask.hwSampleTime);
    } else {
        mTask.hwSampleTime = get_latch_time_timestamp(mTask.latch_time_id);
        //osLog(LOG_ERROR, "lsm6dsmIsr1, fake=%lld, real=%lld\n",
        //rtcGetTime(), mTask.hwSampleTime);
    }
    accGyroHwIntCheckStatus();
}

static int anyMotionPowerOn(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                            void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    osLog(LOG_INFO, "Enter anyMotionPowerOn\n");
    uint32_t delay = 25;
    if (!mTask.sensors[ACC].powered) {
        /* acc off, and wake up is on , so set acc to lower power mode */
        SPI_WRITE(LSM6DSM_CTRL6_C_ADDR, LSM6DSM_CTRL6_C_BASE | LSM6DSM_XL_LOWPOWER_MODE, 25);
        mTask.sensors[ACC].lowPower = true;

        osLog(LOG_INFO, "anyMotionPowerOn, set acc low power mode \n");
    }
    if (!mTask.sensors[ACC].configed) {
        if (mTask.sensors[GYR].configed && (mTask.sensors[GYR].hwRate >= SENSOR_HZ(52.0f))) {
            delay = (1024000000 / SENSOR_HZ(52.0f));
            SPI_WRITE(LSM6DSM_CTRL1_XL_ADDR, LSM6DSM_CTRL1_XL_BASE | LSM6DSM_ODR_52HZ_REG_VALUE, 25); /* set to low power */
            osLog(LOG_INFO, "anyMotionPowerOn, acc hasn't set rate, set 52hz\n");
        } else {
            delay = (1024000000 / SENSOR_HZ(26.0f / 2.0f));
            SPI_WRITE(LSM6DSM_CTRL1_XL_ADDR, LSM6DSM_CTRL1_XL_BASE | LSM6DSM_ODR_12HZ_REG_VALUE, 25); /* set to low power */
            osLog(LOG_INFO, "anyMotionPowerOn, acc hasn't set rate, set 12.5hz\n");
        }
    }
    /* enable anymotion interrupt */
    SPI_WRITE(LSM6DSM_WAKE_UP_DUR_ADDR, LSM6DSM_WAKE_UP_DUR_BASE);
    SPI_WRITE(LSM6DSM_WAKE_UP_THS, 0x01);                                                         //@8g  , 125mg threshold
    SPI_WRITE(LSM6DSM_TAP_CFG_ADDR, LSM6DSM_TAP_CFG_BASE | LSM6DSM_TAP_INTERRUPTS_ENABLE, delay); //
    SPI_WRITE(LSM6DSM_MD1_CFG_ADDR, LSM6DSM_MD1_CFG_BASE | LSM6DSM_WU_INIT1_ENABLE);              //EABLE WU INT TO int1

    mTask.sensors[ANYMO].powered = true;

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

    SPI_WRITE(LSM6DSM_MD1_CFG_ADDR, LSM6DSM_MD1_CFG_BASE); //disable WU int
    SPI_WRITE(LSM6DSM_TAP_CFG_ADDR, LSM6DSM_TAP_CFG_BASE); //disable interrupt

#ifdef CFG_HW_STEP_COUNTER_SUPPORT
    if (!mTask.sensors[ACC].powered && !mTask.sensors[STEPCNT].powered) {
#else
    if (!mTask.sensors[ACC].powered) {
#endif
        SPI_WRITE(LSM6DSM_CTRL1_XL_ADDR, LSM6DSM_CTRL1_XL_BASE, 25);
        osLog(LOG_INFO, "anyMotionPowerOff, acc off, set power down mode\n");
    }

    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
}

#ifdef CFG_HW_STEP_COUNTER_SUPPORT
static int stepCountPowerOn(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                            void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    osLog(LOG_INFO, "Enter stepCountPowerOn\n");

    if (!mTask.sensors[ACC].powered && !mTask.sensors[STEPCNT].powered) {
        /* set accel to low power mode if accel is not enabled */
        SPI_WRITE(LSM6DSM_CTRL6_C_ADDR, LSM6DSM_CTRL6_C_BASE, 25);
        SPI_WRITE(LSM6DSM_CTRL1_XL_ADDR, LSM6DSM_CTRL1_XL_BASE | LSM6DSM_ODR_26HZ_REG_VALUE, 25);
        osLog(LOG_ERROR, "stepCountPowerOn, enter acc low power mode\n");
    }

    mTask.sensors[STEPCNT].powered = true;

    SPI_WRITE(LSM6DSM_CTRL10_C_ADDR, LSM6DSM_CTRL10_C_BASE | \
              LSM6DSM_CTRL10_C_FUNC_EN | LSM6DSM_CTRL10_C_PEDO_EN, 5); /*enbeded function & pedo enable*/
    spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
    return 0;
}

static int stepCountPowerOff(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    osLog(LOG_INFO, "Enter stepCountPowerOff\n");

    mTask.sensors[STEPCNT].configed = false;
    mTask.sensors[STEPCNT].powered = false;

    /* unset step_cnt_en bit */
    SPI_WRITE(LSM6DSM_CTRL10_C_ADDR, LSM6DSM_CTRL10_C_BASE, 5);

    /* clear last_step_cnt when poweroff */
    mTask.last_step_cnt = 0;

    /* for accel disable */
    if (!mTask.sensors[ACC].powered && !mTask.sensors[ANYMO].powered) {
        SPI_WRITE(LSM6DSM_CTRL1_XL_ADDR, LSM6DSM_CTRL1_XL_BASE, 5000);
        osLog(LOG_ERROR, "stepCountPowerOff, enter acc suspend mode\n");
    }

    spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
    return 0;
}

static int StepCntSendData(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                           void *inBuf, uint8_t inSize, uint8_t elemInSize,
                           void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    union EmbeddedDataPoint step_cnt;
    uint32_t cur_step_cnt;
    //uint32_t last_step_cnt = 0;
    //uint32_t now_chip_cnt;
    //now_chip_cnt = (mTask.stepBuffer[1] | (mTask.stepBuffer[2] << 8));
    //osLog(LOG_INFO, "StepCntSendData buf1= 0x%x,buf2=0x%x\n", mTask.stepBuffer[1], mTask.stepBuffer[2]);
    cur_step_cnt = (mTask.stepBuffer[1] | (mTask.stepBuffer[2] << 8));
    osLog(LOG_INFO, "StepCntSendData buf1= 0x%x,buf2=0x%x\n", mTask.stepBuffer[1], mTask.stepBuffer[2]);

    if (cur_step_cnt != mTask.last_step_cnt) {
        /* Check for possible overflow */
        if (cur_step_cnt < mTask.last_step_cnt) {
            mTask.total_step_cnt += cur_step_cnt + (0xFFFF - mTask.last_step_cnt);
        } else {
            mTask.total_step_cnt += (cur_step_cnt - mTask.last_step_cnt);
        }

        mTask.last_step_cnt = cur_step_cnt;
        step_cnt.idata = mTask.total_step_cnt;
        osLog(LOG_INFO, "step count =%d\n", step_cnt.idata);
        osEnqueueEvt(EVT_SENSOR_STEP_COUNTER, step_cnt.vptr, NULL);
    }
    sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
    return 0;
}
static int stepCntGetData(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                          void *inBuf, uint8_t inSize, uint8_t elemInSize,
                          void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "stepCntGetData\n");
    SPI_READ(LSM6DSM_STEP_COUNTER_L_ADDR, 2, &mTask.stepBuffer);
    spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
    return 0;
}

#endif


static int lsm6dsmAccPowerOn(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    osLog(LOG_INFO, "lsm6dsmAccPowerOn\n");
    bool spiTransfer = false;
    if (mTask.sensors[ACC].lowPower) {
        SPI_WRITE(LSM6DSM_CTRL6_C_ADDR, LSM6DSM_CTRL6_C_BASE, 25);
        spiTransfer = true;
        mTask.sensors[ACC].lowPower = false;
        osLog(LOG_INFO, "acc power on, set acc high performance mode\n");
    }

    mTask.sensors[ACC].powered = true;
    /*enable fifo interrupt*/
    if (!mTask.accGyroPowerd) {
        mTask.accGyroPowerd = true;
        SPI_WRITE(LSM6DSM_INT1_CTRL_ADDR, LSM6DSM_INT1_CTRL_BASE);
        spiTransfer = true;
    }
    if (spiTransfer) {
        return spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
    } else {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
        return 0;
    }
}


static int lsm6dsmGyroPowerOn(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    osLog(LOG_INFO, "lsm6dsmGyroPowerOn\n");
    mTask.sensors[GYR].powered = true;
    bool spiTransfer = false;
    /*enable fifo interrupt*/
    if (!mTask.accGyroPowerd) {
        mTask.accGyroPowerd = true;
        SPI_WRITE(LSM6DSM_INT1_CTRL_ADDR, LSM6DSM_INT1_CTRL_BASE);
        spiTransfer = true;
    }
    /*
    if (!mTask.sensors[GYR].configed) {
        SPI_WRITE(LSM6DSM_CTRL2_G_ADDR, LSM6DSM_CTRL2_G_BASE | LSM6DSM_ODR_12HZ_REG_VALUE, 300000);
        spiTransfer = true;
        osLog(LOG_INFO, "lsm6dsmGyroPowerOn, _ODR_416HZ\n");
    }
    */
    if (spiTransfer) {
        return spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
    } else {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
        return 0;
    }
}


static uint8_t LSM6DSMImuRatesRegValue[] = {
    LSM6DSM_ODR_12HZ_REG_VALUE,   /* 3.25Hz - do not exist, use 12.5Hz */
    LSM6DSM_ODR_12HZ_REG_VALUE,   /* 6.5Hz - do not exist, use 12.5Hz */
    LSM6DSM_ODR_12HZ_REG_VALUE,   /* 12.5Hz */
    LSM6DSM_ODR_26HZ_REG_VALUE,   /* 26Hz */
    LSM6DSM_ODR_52HZ_REG_VALUE,   /* 52Hz */
    LSM6DSM_ODR_104HZ_REG_VALUE,  /* 104Hz */
    LSM6DSM_ODR_208HZ_REG_VALUE,  /* 208Hz */
    LSM6DSM_ODR_416HZ_REG_VALUE,  /* 416Hz */
};

static uint32_t LSM6DSMImuRates[] = {
    SENSOR_HZ(26.0f / 8.0f),      /* 3.25Hz */
    SENSOR_HZ(26.0f / 4.0f),      /* 6.5Hz */
    SENSOR_HZ(26.0f / 2.0f),      /* 12.5Hz */
    SENSOR_HZ(26.0f),             /* 26Hz */
    SENSOR_HZ(52.0f),             /* 52Hz */
    SENSOR_HZ(104.0f),            /* 104Hz */
    SENSOR_HZ(208.0f),            /* 208Hz */
    SENSOR_HZ(416.0f),            /* 416Hz */
    0,
};

static uint8_t LSM6DSMAccelRatesSamplesToDiscard[] = {
    LSM6DSM_ODR_12HZ_ACCEL_STD,     /* 3.25Hz - do not exist, use 12.5Hz */
    LSM6DSM_ODR_12HZ_ACCEL_STD,     /* 6.5Hz - do not exist, use 12.5Hz */
    LSM6DSM_ODR_12HZ_ACCEL_STD,     /* 12.5Hz */
    LSM6DSM_ODR_26HZ_ACCEL_STD,     /* 26Hz */
    LSM6DSM_ODR_52HZ_ACCEL_STD,     /* 52Hz */
    LSM6DSM_ODR_104HZ_ACCEL_STD,    /* 104Hz */
    LSM6DSM_ODR_208HZ_ACCEL_STD,    /* 208Hz */
    LSM6DSM_ODR_416HZ_ACCEL_STD,    /* 416Hz */
};

/* When gyroscope change odr but sensor is already on, few samples should be discarded */
static uint8_t LSM6DSMGyroRatesSamplesToDiscard[] = {
    LSM6DSM_ODR_12HZ_GYRO_STD,      /* 3.25Hz - do not exist, use 12.5Hz */
    LSM6DSM_ODR_12HZ_GYRO_STD,      /* 6.5Hz - do not exist, use 12.5Hz */
    LSM6DSM_ODR_12HZ_GYRO_STD,      /* 12.5Hz */
    LSM6DSM_ODR_26HZ_GYRO_STD,      /* 26Hz */
    LSM6DSM_ODR_52HZ_GYRO_STD,      /* 52Hz */
    LSM6DSM_ODR_104HZ_GYRO_STD,     /* 104Hz */
    LSM6DSM_ODR_208HZ_GYRO_STD,     /* 208Hz */
    LSM6DSM_ODR_416HZ_GYRO_STD,     /* 416Hz */
};

/* When sensors switch status from power-down, constant boottime must be considered, some samples should be discarded */
static uint8_t LSM6DSMRatesSamplesToDiscardGyroPowerOn[] = {
    LSM6DSM_ODR_DELAY_US_GYRO_POWER_ON / 80000, /* 3.25Hz - do not exist, use 12.5Hz = 80000us */
    LSM6DSM_ODR_DELAY_US_GYRO_POWER_ON / 80000, /* 6.5Hz - do not exist, use 12.5Hz = 80000us */
    LSM6DSM_ODR_DELAY_US_GYRO_POWER_ON / 80000, /* 12.5Hz = 80000us */
    LSM6DSM_ODR_DELAY_US_GYRO_POWER_ON / 38461, /* 26Hz = 38461us */
    LSM6DSM_ODR_DELAY_US_GYRO_POWER_ON / 19230, /* 52Hz = 19230s */
    LSM6DSM_ODR_DELAY_US_GYRO_POWER_ON / 9615,  /* 104Hz = 9615us */
    LSM6DSM_ODR_DELAY_US_GYRO_POWER_ON / 4807,  /* 208Hz = 4807us */
    LSM6DSM_ODR_DELAY_US_GYRO_POWER_ON / 2403,  /* 416Hz = 2403us */
};

static int lsm6dsmCalcuOdr(uint32_t *rate, uint32_t *report_rate)
{
    int i;

    for (i = 0; i < (ARRAY_SIZE(LSM6DSMImuRates) - 1); i++) {
        if (*rate <= LSM6DSMImuRates[i]) {
            *report_rate = LSM6DSMImuRates[i];
            break;
        }
    }

    if (*rate > LSM6DSMImuRates[(ARRAY_SIZE(LSM6DSMImuRates) - 2)]) {
        i = (ARRAY_SIZE(LSM6DSMImuRates) - 2);
        *report_rate = LSM6DSMImuRates[i];
    }

    if (i == (ARRAY_SIZE(LSM6DSMImuRates) - 1)) {
        //ERROR_PRINT("ODR not valid! Selected smallest ODR available\n");
        return -1;
    }

    return i;
}

static uint16_t lsm6dsmCalcuWm_Decimation(void)
{
    uint8_t handle;
    uint64_t min_latency = SENSOR_LATENCY_NODATA;
    uint8_t min_watermark = 1;
    uint8_t max_watermark = MAX_RECV_PACKET;
    uint16_t watermark = 0;
    uint32_t temp_cnt, total_cnt = 0;
    uint32_t temp_delay = 0;

    if (0 == mTask.sensors[ACC].hwRate)
        mTask.sensors[ACC].decimators = 0;
    else
        mTask.sensors[ACC].decimators = 1;

    if (0 == mTask.sensors[GYR].hwRate)
        mTask.sensors[GYR].decimators = 0;
    else
        mTask.sensors[GYR].decimators = 1;

    if ((!mTask.sensors[GYR].hwRate) && (!mTask.sensors[ACC].hwRate)) {
        mTask.sensors[ACC].fifoSip = 0;
        mTask.sensors[GYR].fifoSip = 0;
    } else if ((!mTask.sensors[GYR].hwRate) && (mTask.sensors[ACC].hwRate)) {
        mTask.sensors[ACC].fifoSip = 1;
        mTask.sensors[GYR].fifoSip = 0;
    } else if ((mTask.sensors[GYR].hwRate) && (!mTask.sensors[ACC].hwRate)) {
        mTask.sensors[ACC].fifoSip = 0;
        mTask.sensors[GYR].fifoSip = 1;
    } else {
        mTask.sensors[ACC].fifoSip = mTask.sensors[GYR].decimators;
        mTask.sensors[GYR].fifoSip = mTask.sensors[ACC].decimators;
    }


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
                total_cnt += temp_cnt > min_watermark ? temp_cnt : min_watermark;
                osLog(LOG_INFO, "lsm6dsmCalcuWm_Decimation, delay=%u, latency:%llu, min_wm=%u, total_cnt=%u\n",
                      temp_delay, min_latency, min_watermark, total_cnt);
            }
        }

        watermark = total_cnt;
        watermark = watermark < min_watermark ? min_watermark : watermark;
        watermark = watermark > max_watermark ? max_watermark : watermark;
    }
    /*one sample byte is 6 ,then /2 write into register*/
    watermark =  watermark * 6 / 2;

    return watermark;
}

/*
 * lsm6dsm_decimatorToFifoDecimatorReg: get decimator reg value based on decimation factor
 * @dec: FIFO sample decimation factor.
 */
static uint8_t lsm6dsmToFifoDecimatorReg(uint8_t dec)
{
    uint8_t regValue;

    switch (dec) {
        case 0:
            regValue = LSM6DSM_FIFO_SAMPLE_NOT_IN_FIFO;
            break;
        case 1:
            regValue = LSM6DSM_FIFO_NO_DECIMATION;
            break;
        case 2:
            regValue = LSM6DSM_FIFO_DECIMATION_FACTOR_2;
            break;
        case 3:
            regValue = LSM6DSM_FIFO_DECIMATION_FACTOR_3;
            break;
        case 4:
            regValue = LSM6DSM_FIFO_DECIMATION_FACTOR_4;
            break;
        case 8:
            regValue = LSM6DSM_FIFO_DECIMATION_FACTOR_8;
            break;
        case 16:
            regValue = LSM6DSM_FIFO_DECIMATION_FACTOR_16;
            break;
        case 32:
            regValue = LSM6DSM_FIFO_DECIMATION_FACTOR_32;
            break;
        default:
            regValue = LSM6DSM_FIFO_SAMPLE_NOT_IN_FIFO;
            break;
    }

    return regValue;
}

static void lsm6dsmConfigFifo(bool odr_change)
{
    uint8_t regValue, buffer[3];
    uint16_t watermarkReg;

    watermarkReg = mTask.watermark;

    /* Program fifo and enable or disable it */
    if (odr_change) {
        buffer[0] = *((uint8_t *)&watermarkReg);
        buffer[1] = (*((uint8_t *)&watermarkReg + 1) & LSM6DSM_FIFO_CTRL2_FTH_MASK);
        buffer[2] = (lsm6dsmToFifoDecimatorReg(mTask.sensors[GYR].decimators) << 3) |
                    lsm6dsmToFifoDecimatorReg(mTask.sensors[ACC].decimators);

        SPI_WRITE(LSM6DSM_FIFO_CTRL1_ADDR, buffer[0], 5);
        SPI_WRITE(LSM6DSM_FIFO_CTRL2_ADDR, buffer[1], 5);
        SPI_WRITE(LSM6DSM_FIFO_CTRL3_ADDR, buffer[2], 5);

        SPI_WRITE(LSM6DSM_FIFO_CTRL5_ADDR, LSM6DSM_FIFO_BYPASS_MODE, 50);     //first bypass all ,to reset fifo

        switch (max(mTask.sensors[ACC].hwRate, mTask.sensors[GYR].hwRate)) {
            case 0:
                regValue = LSM6DSM_FIFOOFF_CONTINUE_REG_VALUE;
                break;
            case SENSOR_HZ(26.0f / 2.0f):
                regValue = LSM6DSM_FIFO12HZ_CONTINUE_REG_VALUE;
                break;
            case SENSOR_HZ(26.0f):
                regValue = LSM6DSM_FIFO26HZ_CONTINUE_REG_VALUE;
                break;
            case SENSOR_HZ(52.0f):
                regValue = LSM6DSM_FIFO52HZ_CONTINUE_REG_VALUE;
                break;
            case SENSOR_HZ(104.0f):
                regValue = LSM6DSM_FIFO104HZ_CONTINUE_REG_VALUE;
                break;
            case SENSOR_HZ(208.0f):
                regValue = LSM6DSM_FIFO208HZ_CONTINUE_REG_VALUE;
                break;
            case SENSOR_HZ(416.0f):
                regValue = LSM6DSM_FIFO416HZ_CONTINUE_REG_VALUE;
                break;
            default:
                regValue = LSM6DSM_FIFOOFF_CONTINUE_REG_VALUE;
                break;
        }

        SPI_WRITE(LSM6DSM_FIFO_CTRL5_ADDR, regValue, 2);   // set fifo continuous mode
        osLog(LOG_INFO, "lsm6dsmConfigFifo, ctrl1:0x%x, ctrl2:0x%x, ctrl3:0x%x, ctrl5:0x%x\n",
              buffer[0], buffer[1], buffer[2], regValue);

    } else {
        buffer[0] = *((uint8_t *)&watermarkReg);
        buffer[1] = (*((uint8_t *)&watermarkReg + 1) & LSM6DSM_FIFO_CTRL2_FTH_MASK);

        SPI_WRITE(LSM6DSM_FIFO_CTRL1_ADDR, buffer[0], 5);
        SPI_WRITE(LSM6DSM_FIFO_CTRL2_ADDR, buffer[1], 5);
        osLog(LOG_INFO, "lsm6dsmConfigFifo, ctrl1:0x%x, ctrl2:0x%x\n",
              buffer[0], buffer[1]);
    }

}

static int lsm6dsmAccRate(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                          void *inBuf, uint8_t inSize, uint8_t elemInSize,
                          void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    int odr = 0;
    uint8_t regValue = 0x00;
    uint32_t sampleRate = 0;
    uint32_t maxRate = 0;
    uint32_t delay = 25;
    bool accelOdrChanged = false;

    struct accGyroCntlPacket cntlPacket;

    ret = rxControlInfo(&cntlPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);

    if (ret < 0) {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "lsm6dsmAccRate, rx inSize and elemSize error\n");
        return -1;
    }

    mTask.sensors[ACC].rate = cntlPacket.rate;
    mTask.sensors[ACC].latency = cntlPacket.latency;


    //osLog(LOG_ERROR, "lsm6dsmAccRate acc rate:%d, latency:%lld\n",
    //mTask.sensors[ACC].rate, mTask.sensors[ACC].latency);

    odr = lsm6dsmCalcuOdr(&mTask.sensors[ACC].rate, &sampleRate);

    if (odr < 0) {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "lsm6dsmAccRate, calcu odr error\n");
        return -1;
    }

    if (odr < 3) {
#ifdef CFG_HW_STEP_COUNTER_SUPPORT
        if (mTask.sensors[STEPCNT].powered) {
            sampleRate = SENSOR_HZ(26.0f);
            odr = 3;
        } else {
            sampleRate = SENSOR_HZ(26.0f / 2.0f);
            odr = 2;
        }
#else
        sampleRate = SENSOR_HZ(26.0f / 2.0f);
        odr = 2;
#endif
    }
    mTask.sensors[ACC].preRealRate = sampleRate;

    if (mTask.sensors[ANYMO].powered) {
        SPI_WRITE(LSM6DSM_MD1_CFG_ADDR, LSM6DSM_MD1_CFG_BASE, 25);  //Disable WU INT TO int1firstly
    }

    /*if gyr configed ,compare maxRate with acc and gyr rate*/
    if (mTask.sensors[GYR].configed) {
        maxRate = max(sampleRate, mTask.sensors[GYR].preRealRate);  //choose with preRealRate
        if ((maxRate != mTask.sensors[ACC].hwRate) || (maxRate != mTask.sensors[GYR].hwRate)) {
            mTask.sensors[ACC].hwRate = maxRate;
            mTask.sensors[GYR].hwRate = maxRate;

            odr = lsm6dsmCalcuOdr(&maxRate, &sampleRate);
            if (odr < 0) {
                sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
                osLog(LOG_ERROR, "lsm6dsmAccRate, calcu odr error\n");
                return -1;
            }

            regValue = LSM6DSMImuRatesRegValue[odr];

            delay = LSM6DSMGyroRatesSamplesToDiscard[odr] * (1024000000 / mTask.sensors[ACC].hwRate);
            mTask.sensors[ACC].samplesToDiscard = LSM6DSMAccelRatesSamplesToDiscard[odr];
            mTask.sensors[GYR].samplesToDiscard = LSM6DSMGyroRatesSamplesToDiscard[odr];

            SPI_WRITE(LSM6DSM_CTRL1_XL_ADDR, LSM6DSM_CTRL1_XL_BASE | regValue, 30);
            SPI_WRITE(LSM6DSM_CTRL2_G_ADDR, LSM6DSM_CTRL2_G_BASE | regValue, 30);
            accelOdrChanged = true;
        } else {
            accelOdrChanged = false;
        }
    } else {
        if ((sampleRate != mTask.sensors[ACC].hwRate)) {
            mTask.sensors[ACC].hwRate = sampleRate;
            regValue = LSM6DSMImuRatesRegValue[odr];

            delay = LSM6DSMAccelRatesSamplesToDiscard[odr] * (1024000000 / mTask.sensors[ACC].hwRate);
            mTask.sensors[ACC].samplesToDiscard = LSM6DSMAccelRatesSamplesToDiscard[odr];

            SPI_WRITE(LSM6DSM_CTRL1_XL_ADDR, LSM6DSM_CTRL1_XL_BASE | regValue, 30);
            accelOdrChanged = true;
        } else {
            accelOdrChanged = false;
        }
    }

    registerAccGyroFifoInfo((mTask.sensors[ACC].hwRate == 0) ? 0 : 1024000000000 / mTask.sensors[ACC].hwRate,
                            (mTask.sensors[GYR].hwRate == 0) ? 0 : 1024000000000 / mTask.sensors[GYR].hwRate);

    mTask.sensors[ACC].configed = true;

    //osLog(LOG_ERROR, "lsm6dsmAccRate acc hwRate:%d, gyro hwRate:%d\n",
    //mTask.sensors[ACC].hwRate, mTask.sensors[GYR].hwRate);

    //watermark decimation
    mTask.watermark = lsm6dsmCalcuWm_Decimation();

    lsm6dsmConfigFifo(accelOdrChanged);
    if (mTask.sensors[ANYMO].powered) {
        SPI_WRITE(LSM6DSM_TAP_CFG_ADDR, LSM6DSM_TAP_CFG_BASE | LSM6DSM_TAP_INTERRUPTS_ENABLE, delay); //
        SPI_WRITE(LSM6DSM_MD1_CFG_ADDR, LSM6DSM_MD1_CFG_BASE | LSM6DSM_WU_INIT1_ENABLE);   //EABLE WU INT TO int1
    }

    mt_eint_unmask(mTask.hw->eint_num);
    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
}

static int lsm6dsmGyroRate(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                           void *inBuf, uint8_t inSize, uint8_t elemInSize,
                           void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    int odr = 0;
    uint8_t regValue = 0x00;
    uint32_t sampleRate = 0;
    uint32_t maxRate = 0;
    uint32_t delay = 25;
    bool gyroOdrChanged = false;
    bool gyroFirstEnable = false;

    struct accGyroCntlPacket cntlPacket;

    //osLog(LOG_INFO, "lsm6dsmGyroRate\n");

    ret = rxControlInfo(&cntlPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);

    if (ret < 0) {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "lsm6dsmGyroRate, rx inSize and elemSize error\n");
        return -1;
    }
    if (!mTask.sensors[GYR].preRealRate) {
        gyroFirstEnable = true;
    }
    mTask.sensors[GYR].rate = cntlPacket.rate;
    mTask.sensors[GYR].latency = cntlPacket.latency;

    odr = lsm6dsmCalcuOdr(&mTask.sensors[GYR].rate, &sampleRate);

    if (odr < 0) {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "lsm6dsmGyroRate, calcu odr error\n");
        return -1;
    }


    if (odr < 2) {
#ifdef CFG_HW_STEP_COUNTER_SUPPORT
        if (mTask.sensors[STEPCNT].powered) {
            sampleRate = SENSOR_HZ(26.0f);
            odr = 3;
        } else {
            sampleRate = SENSOR_HZ(26.0f / 2.0f);
            odr = 2;
        }
#else
        sampleRate = SENSOR_HZ(26.0f / 2.0f);
        odr = 2;
#endif
    }

    mTask.sensors[GYR].preRealRate = sampleRate;

    if (mTask.sensors[ANYMO].powered) {
        SPI_WRITE(LSM6DSM_MD1_CFG_ADDR, LSM6DSM_MD1_CFG_BASE, 25);  //Disable WU INT TO int1firstly
    }

    if (mTask.sensors[ACC].configed) {
        maxRate = max(sampleRate, mTask.sensors[ACC].preRealRate);  //choose with preRealRate
        if ((maxRate != mTask.sensors[ACC].hwRate) || (maxRate != mTask.sensors[GYR].hwRate)) {
            mTask.sensors[ACC].hwRate = maxRate;
            mTask.sensors[GYR].hwRate = maxRate;

            odr = lsm6dsmCalcuOdr(&maxRate, &sampleRate);
            if (odr < 0) {
                sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
                osLog(LOG_ERROR, "lsm6dsmGyroRate, calcu odr error\n");
                return -1;
            }
            regValue = LSM6DSMImuRatesRegValue[odr];
            delay = LSM6DSMGyroRatesSamplesToDiscard[odr] * (1024000000 / mTask.sensors[ACC].hwRate);

            mTask.sensors[ACC].samplesToDiscard = LSM6DSMAccelRatesSamplesToDiscard[odr];
            mTask.sensors[GYR].samplesToDiscard = LSM6DSMGyroRatesSamplesToDiscard[odr];

            SPI_WRITE(LSM6DSM_CTRL1_XL_ADDR, LSM6DSM_CTRL1_XL_BASE | regValue, 30);
            SPI_WRITE(LSM6DSM_CTRL2_G_ADDR, LSM6DSM_CTRL2_G_BASE | regValue, 30);
            gyroOdrChanged = true;
        } else {
            gyroOdrChanged = false;
        }
    } else {
        if ((sampleRate != mTask.sensors[GYR].hwRate)) {
            mTask.sensors[GYR].hwRate = sampleRate;
            regValue = LSM6DSMImuRatesRegValue[odr];

            mTask.sensors[GYR].samplesToDiscard = LSM6DSMGyroRatesSamplesToDiscard[odr];
            //delay = LSM6DSMGyroRatesSamplesToDiscard[odr] * (1024000000 / maxRate);
            SPI_WRITE(LSM6DSM_CTRL2_G_ADDR, LSM6DSM_CTRL2_G_BASE | regValue, 30);
            gyroOdrChanged = true;
        } else {
            gyroOdrChanged = false;
        }
    }

    if (gyroFirstEnable)
        mTask.sensors[GYR].samplesToDiscard += LSM6DSMRatesSamplesToDiscardGyroPowerOn[odr];

    registerAccGyroFifoInfo((mTask.sensors[ACC].hwRate == 0) ? 0 : 1024000000000 / mTask.sensors[ACC].hwRate,
                            (mTask.sensors[GYR].hwRate == 0) ? 0 : 1024000000000 / mTask.sensors[GYR].hwRate);

    mTask.sensors[GYR].configed = true;

    //watermark decimation
    mTask.watermark = lsm6dsmCalcuWm_Decimation();
    lsm6dsmConfigFifo(gyroOdrChanged);

    if (mTask.sensors[ANYMO].powered) {
        SPI_WRITE(LSM6DSM_TAP_CFG_ADDR, LSM6DSM_TAP_CFG_BASE | LSM6DSM_TAP_INTERRUPTS_ENABLE, delay); //
        SPI_WRITE(LSM6DSM_MD1_CFG_ADDR, LSM6DSM_MD1_CFG_BASE | LSM6DSM_WU_INIT1_ENABLE);   //EABLE WU INT TO int1
    }

    mt_eint_unmask(mTask.hw->eint_num);
    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
}


static int lsm6dsmAccPowerOff(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    int odr = 0;
    uint8_t regValue = 0x00;
    uint32_t sampleRate = 0;
    uint32_t delay = 25;
    struct accGyroCntlPacket cntlPacket;

    osLog(LOG_INFO, "lsm6dsmAccPowerOff\n");

    ret = rxControlInfo(&cntlPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);

    if (ret < 0) {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "lsm6dsmAccPowerOff, rx inSize and elemSize error\n");
        return -1;
    }

    mTask.sensors[ACC].rate = 0;
    mTask.sensors[ACC].preRealRate = 0;
    mTask.sensors[ACC].hwRate = 0;
    mTask.sensors[ACC].latency = SENSOR_LATENCY_NODATA;
    mTask.sensors[ACC].samplesToDiscard = 0;


    if (mTask.sensors[GYR].powered == false) {
        mt_eint_mask(mTask.hw->eint_num);
        mTask.accGyroPowerd = false;

        SPI_WRITE(LSM6DSM_INT1_CTRL_ADDR, 0x00);
    } else {
        if (mTask.sensors[GYR].hwRate != mTask.sensors[GYR].preRealRate) {
            mTask.sensors[GYR].hwRate = mTask.sensors[GYR].preRealRate;
            odr = lsm6dsmCalcuOdr(&(mTask.sensors[GYR].hwRate), &sampleRate);
            if (odr < 0) {
                sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
                osLog(LOG_ERROR, "lsm6dsmAccPowerOff, calcu odr error\n");
                return -1;
            }

            regValue = LSM6DSMImuRatesRegValue[odr];
            //delay = LSM6DSMGyroRatesSamplesToDiscard[odr] * (1024000000 / mTask.sensors[GYR].hwRate);

            mTask.sensors[GYR].samplesToDiscard = LSM6DSMGyroRatesSamplesToDiscard[odr];
            SPI_WRITE(LSM6DSM_CTRL2_G_ADDR, LSM6DSM_CTRL2_G_BASE | regValue, 30);
            osLog(LOG_INFO, "lsm6dsmAccPowerOff, gyro change rate to preRealRate:%u\n", mTask.sensors[GYR].hwRate);
        }
    }

    if (mTask.sensors[ANYMO].powered) {
        SPI_WRITE(LSM6DSM_MD1_CFG_ADDR, LSM6DSM_MD1_CFG_BASE, 25);  //Disable WU INT TO int1 firstly

        SPI_WRITE(LSM6DSM_CTRL1_XL_ADDR, LSM6DSM_CTRL1_XL_BASE, 25); //lsm6dsm switch mode need power down first
        /* acc off, and wake up is on , so set acc to lower power mode */
        SPI_WRITE(LSM6DSM_CTRL6_C_ADDR, LSM6DSM_CTRL6_C_BASE | LSM6DSM_XL_LOWPOWER_MODE, 25);
        mTask.sensors[ACC].lowPower = true;
        if (mTask.sensors[GYR].configed && (mTask.sensors[GYR].hwRate >= SENSOR_HZ(52.0f))) {
            delay = (1024000000 / SENSOR_HZ(52.0f));
            SPI_WRITE(LSM6DSM_CTRL1_XL_ADDR, LSM6DSM_CTRL1_XL_BASE | LSM6DSM_ODR_52HZ_REG_VALUE, 25); /* set to low power */
        } else {
            delay = (1024000000 / SENSOR_HZ(26.0f / 2.0f));
            SPI_WRITE(LSM6DSM_CTRL1_XL_ADDR, LSM6DSM_CTRL1_XL_BASE | LSM6DSM_ODR_12HZ_REG_VALUE, 25); /* set to low power */
        }
        osLog(LOG_INFO, "accPowerOff, enter acc low power mode because of any motion on \n");
    } else {
#ifdef CFG_HW_STEP_COUNTER_SUPPORT
        if (mTask.sensors[STEPCNT].powered)
            SPI_WRITE(LSM6DSM_CTRL1_XL_ADDR, LSM6DSM_CTRL1_XL_BASE | LSM6DSM_ODR_26HZ_REG_VALUE, 25);
        else
            SPI_WRITE(LSM6DSM_CTRL1_XL_ADDR, LSM6DSM_CTRL1_XL_BASE, 30);
#else
        SPI_WRITE(LSM6DSM_CTRL1_XL_ADDR, LSM6DSM_CTRL1_XL_BASE, 30);
#endif
    }
    registerAccGyroFifoInfo((mTask.sensors[ACC].hwRate == 0) ? 0 : 1024000000000 / mTask.sensors[ACC].hwRate,
                            (mTask.sensors[GYR].hwRate == 0) ? 0 : 1024000000000 / mTask.sensors[GYR].hwRate);

    mTask.watermark = lsm6dsmCalcuWm_Decimation();
    lsm6dsmConfigFifo(true);

    if (mTask.sensors[ANYMO].powered) {
        SPI_WRITE(LSM6DSM_TAP_CFG_ADDR, LSM6DSM_TAP_CFG_BASE | LSM6DSM_TAP_INTERRUPTS_ENABLE, delay); //
        SPI_WRITE(LSM6DSM_MD1_CFG_ADDR, LSM6DSM_MD1_CFG_BASE | LSM6DSM_WU_INIT1_ENABLE);  //EABLE WU INT TO int1
        mt_eint_unmask(mTask.hw->eint_num);
    }

    mTask.sensors[ACC].powered = false;
    mTask.sensors[ACC].configed = false;

    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
}

static int lsm6dsmGyroPowerOff(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                               void *inBuf, uint8_t inSize, uint8_t elemInSize,
                               void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    int odr = 0;
    uint8_t regValue = 0x00;
    uint32_t sampleRate = 0;
    uint32_t delay = 25;
    struct accGyroCntlPacket cntlPacket;

    osLog(LOG_INFO, "lsm6dsmGyroPowerOff\n");
    ret = rxControlInfo(&cntlPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);

    if (ret < 0) {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "lsm6dsmGyroPowerOff, rx inSize and elemSize error\n");
        return -1;
    }

    mTask.sensors[GYR].rate = 0;
    mTask.sensors[GYR].hwRate = 0;
    mTask.sensors[GYR].preRealRate = 0;
    mTask.sensors[GYR].latency = SENSOR_LATENCY_NODATA;
    mTask.sensors[GYR].samplesToDiscard = 0;

    if (mTask.sensors[ANYMO].powered) {
        SPI_WRITE(LSM6DSM_MD1_CFG_ADDR, LSM6DSM_MD1_CFG_BASE, 25);  //Disable WU INT TO int1firstly
    }

    if (mTask.sensors[ACC].powered == false) {
        mt_eint_mask(mTask.hw->eint_num);
        mTask.accGyroPowerd = false;

        SPI_WRITE(LSM6DSM_INT1_CTRL_ADDR, 0x00);
    } else {
        if (mTask.sensors[ACC].hwRate != mTask.sensors[ACC].preRealRate) {
            mTask.sensors[ACC].hwRate = mTask.sensors[ACC].preRealRate;
            odr = lsm6dsmCalcuOdr(&(mTask.sensors[ACC].hwRate), &sampleRate);
            if (odr < 0) {
                sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
                osLog(LOG_ERROR, "lsm6dsmGyroPowerOff, calcu odr error\n");
                return -1;
            }
            regValue = LSM6DSMImuRatesRegValue[odr];
            delay = LSM6DSMAccelRatesSamplesToDiscard[odr] * (1024000000 / mTask.sensors[ACC].hwRate);
            mTask.sensors[ACC].samplesToDiscard = LSM6DSMAccelRatesSamplesToDiscard[odr];

            SPI_WRITE(LSM6DSM_CTRL1_XL_ADDR, LSM6DSM_CTRL1_XL_BASE | regValue, 30);
            osLog(LOG_INFO, "lsm6dsmGyroPowerOff, acc change rate to preRealRate:%u\n", mTask.sensors[ACC].hwRate);
        }
    }

    SPI_WRITE(LSM6DSM_CTRL2_G_ADDR, LSM6DSM_CTRL2_G_BASE, 30);

    registerAccGyroFifoInfo((mTask.sensors[ACC].hwRate == 0) ? 0 : 1024000000000 / mTask.sensors[ACC].hwRate,
                            (mTask.sensors[GYR].hwRate == 0) ? 0 : 1024000000000 / mTask.sensors[GYR].hwRate);

    mTask.watermark = lsm6dsmCalcuWm_Decimation();
    lsm6dsmConfigFifo(true);

    if (mTask.sensors[ANYMO].powered) {
        SPI_WRITE(LSM6DSM_TAP_CFG_ADDR, LSM6DSM_TAP_CFG_BASE | LSM6DSM_TAP_INTERRUPTS_ENABLE, delay); //
        SPI_WRITE(LSM6DSM_MD1_CFG_ADDR, LSM6DSM_MD1_CFG_BASE | LSM6DSM_WU_INIT1_ENABLE);  //EABLE WU INT TO int1
        mt_eint_unmask(mTask.hw->eint_num);
    }

    mTask.sensors[GYR].powered = false;
    mTask.sensors[GYR].configed = false;

    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
}

static int lsm6dsmAccCali(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                          void *inBuf, uint8_t inSize, uint8_t elemInSize,
                          void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
#ifndef CFG_MTK_CALIBRATION_V1_0
    float bias[AXES_NUM] = {0};
#endif

    mTask.sensors[ACC].startCali = true;
    osLog(LOG_INFO, "lsm6dsmAccCali\n");
#ifndef CFG_MTK_CALIBRATION_V1_0
    Acc_init_calibration(bias);
#else
    accFactoryCalibrateInit(&mTask.accFactoryCal);
#endif
    sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int lsm6dsmGyroCali(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                           void *inBuf, uint8_t inSize, uint8_t elemInSize,
                           void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
#ifndef CFG_MTK_CALIBRATION_V1_0
    float slope[AXES_NUM] = {0};
    float intercept[AXES_NUM] = {0};
#endif

    mTask.sensors[GYR].startCali = true;
    osLog(LOG_INFO, "lsm6dsmGyroCali\n");
#ifndef CFG_MTK_CALIBRATION_V1_0
    Gyro_init_calibration(slope, intercept);
#else
    gyroFactoryCalibrateInit(&mTask.gyroFactoryCal);
#endif
    sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int lsm6dsmAccCfgCali(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
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
    osLog(LOG_INFO, "lsm6dsmAccCfgCali: cfgData[0]:%d, cfgData[1]:%d, cfgData[2]:%d\n",
          caliCfgPacket.caliCfgData[0], caliCfgPacket.caliCfgData[1], caliCfgPacket.caliCfgData[2]);

    mTask.sensors[ACC].staticCali[0] = (float)caliCfgPacket.caliCfgData[0] / 1000;
    mTask.sensors[ACC].staticCali[1] = (float)caliCfgPacket.caliCfgData[1] / 1000;
    mTask.sensors[ACC].staticCali[2] = (float)caliCfgPacket.caliCfgData[2] / 1000;

    sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int lsm6dsmGyroCfgCali(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
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

    osLog(LOG_INFO, "lsm6dsmGyroCfgCali: cfgData[0]:%d, cfgData[1]:%d, cfgData[2]:%d\n",
          caliCfgPacket.caliCfgData[0], caliCfgPacket.caliCfgData[1], caliCfgPacket.caliCfgData[2]);

    mTask.sensors[GYR].staticCali[0] = (float)caliCfgPacket.caliCfgData[0] / 1000;
    mTask.sensors[GYR].staticCali[1] = (float)caliCfgPacket.caliCfgData[1] / 1000;
    mTask.sensors[GYR].staticCali[2] = (float)caliCfgPacket.caliCfgData[2] / 1000;

    sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
    return 0;
}


static int lsm6dsmIntStatusCheck(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                 void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                 void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "lsm6dsmIntStatusCheck\n");
    if (mTask.sensors[ANYMO].powered)
        SPI_READ(LSM6DSM_WAKEUP_SRC_ADDR, 1, &mTask.wakeupBuffer);

    SPI_READ(LSM6DSM_FIFO_STATUS2_ADDR, 1, &mTask.statusBuffer);

    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
}

static int lsm6dsmIntHandling(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int err = 0;
    union EmbeddedDataPoint trigger_axies;
    uint8_t fifo_status_2 = mTask.statusBuffer[1];
    uint8_t regValue;
    //osLog(LOG_INFO, "lsm6dsmIntHandling, fifo_status_2:0x%x\n", fifo_status_2);

    if (mTask.sensors[ANYMO].powered) {
        uint8_t anymo_status = mTask.wakeupBuffer[1];
        if (anymo_status & LSM6DSM_WAKEUP_MASK) {
            //osLog(LOG_INFO, "Detected anymotion, status:0x%x,firstanymo:%d\n", anymo_status, mTask.firstAnymo);
            //if (mTask.firstAnymo) {
            //  mTask.firstAnymo = false;
            //} else {
            trigger_axies.idata = (anymo_status & 0x0f);
            //INFO_PRINT("Detected any motion\n");
            osEnqueueEvt(EVT_SENSOR_ANY_MOTION, trigger_axies.vptr,
                         NULL); //as other vendor do , we do not enable software mask here
            //}
        }
    }


    if ((fifo_status_2 & LSM6DSM_FIFO_STATUS2_FIFO_ERROR) == 0) {
        if (fifo_status_2 & LSM6DSM_FIFO_STATUS2_FIFO_WATERMARK) {
            //osLog(LOG_INFO, "Detected fifo watermark\n");
            accGyroInterruptOccur();
        } else {
            mt_eint_unmask(mTask.hw->eint_num); //for other interrupt
            //osLog(LOG_INFO, "anymo uneint\n");
        }
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
    } else {
        if ((fifo_status_2 & LSM6DSM_FIFO_STATUS2_FIFO_FULL_SMART)
                || (fifo_status_2 & LSM6DSM_FIFO_STATUS2_FIFO_FULL_OVERRUN)) {
            //INFO_PRINT("FIFO full\n");

            SPI_WRITE(LSM6DSM_FIFO_CTRL5_ADDR, LSM6DSM_FIFO_BYPASS_MODE, 50);

            switch (max(mTask.sensors[ACC].hwRate, mTask.sensors[GYR].hwRate)) {
                case 0:
                    regValue = LSM6DSM_FIFOOFF_CONTINUE_REG_VALUE;
                    break;
                case SENSOR_HZ(26.0f / 2.0f):
                    regValue = LSM6DSM_FIFO12HZ_CONTINUE_REG_VALUE;
                    break;
                case SENSOR_HZ(26.0f):
                    regValue = LSM6DSM_FIFO26HZ_CONTINUE_REG_VALUE;
                    break;
                case SENSOR_HZ(52.0f):
                    regValue = LSM6DSM_FIFO52HZ_CONTINUE_REG_VALUE;
                    break;
                case SENSOR_HZ(104.0f):
                    regValue = LSM6DSM_FIFO104HZ_CONTINUE_REG_VALUE;
                    break;
                case SENSOR_HZ(208.0f):
                    regValue = LSM6DSM_FIFO208HZ_CONTINUE_REG_VALUE;
                    break;
                case SENSOR_HZ(416.0f):
                    regValue = LSM6DSM_FIFO416HZ_CONTINUE_REG_VALUE;
                    break;
                default:
                    regValue = LSM6DSM_FIFOOFF_CONTINUE_REG_VALUE;
                    break;
            }

            SPI_WRITE(LSM6DSM_FIFO_CTRL5_ADDR, regValue, 2);   // set fifo continuous mode

            err = spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
        } else {
            //INFO_PRINT("FIFO empty\n");
            sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
        }
        /* If not enter accGyroInterruptOccur by INT_FIFO_WM, need unmask eint here */
        mt_eint_unmask(mTask.hw->eint_num);
    }

    return err;
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

static int lsm6dsmSample(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    //osLog(LOG_INFO, "lsm6dsmSample\n");
    ret = rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "lsm6dsmSample, rx dataInfo error\n");
        return -1;
    }

    SPI_READ(LSM6DSM_FIFO_STATUS1_ADDR, 3, &mTask.statusBuffer);

    SPI_READ(LSM6DSM_STATUS_REG_ADDR, 1, &mTask.tempStatusBuffer);

    mTask.spiCallBack = spiCallBack;

    return spiBatchTxRx(&mTask.mode, spiIsrCallBack, next_state, __FUNCTION__);
}

static int lsm6dsmReadFifo(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                           void *inBuf, uint8_t inSize, uint8_t elemInSize,
                           void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    uint8_t totalSip;

    uint8_t fifo_status_1 = mTask.statusBuffer[1];
    uint8_t fifo_status_2 = mTask.statusBuffer[2];
    uint8_t temp_status = mTask.tempStatusBuffer[1];
    //uint8_t fifo_status_3 = mTask.statusBuffer[3];
    uint16_t tmp = 0;
    uint32_t maxRate = 0;

    mTask.fifoDataToRead = ((fifo_status_2 & LSM6DSM_FIFO_CTRL2_FTH_MASK) << 8) | fifo_status_1;
    //osLog(LOG_INFO, "diff %x flag %x pattern %x\n", fifo_status_1, fifo_status_2, fifo_status_3);

    mTask.fifoDataToRead = mTask.fifoDataToRead * 2;

    totalSip = mTask.sensors[ACC].fifoSip + mTask.sensors[GYR].fifoSip;

    if (mTask.fifoDataToRead > FIFO_DATA_SIZE) {
        tmp = mTask.fifoDataToRead - FIFO_DATA_SIZE;
        mTask.fifoDataToRead = FIFO_DATA_SIZE;
        tmp = tmp / LSM6DSM_ONE_SAMPLE_BYTE;
        maxRate = max(mTask.sensors[ACC].hwRate, mTask.sensors[GYR].hwRate);
        if (maxRate) {
            mTask.swSampleTime -= 1024000000000ULL / maxRate * tmp;
            osLog(LOG_INFO, "out size tmp:%u fifo:%u\n", tmp, mTask.fifoDataToRead);
        }
    }

    if ((totalSip != 0) && mTask.fifoDataToRead >= (totalSip * LSM6DSM_ONE_SAMPLE_BYTE)) {
        tmp = mTask.fifoDataToRead % (totalSip * LSM6DSM_ONE_SAMPLE_BYTE);
        if (tmp >= LSM6DSM_ONE_SAMPLE_BYTE) {
            maxRate = max(mTask.sensors[ACC].hwRate, mTask.sensors[GYR].hwRate);
            if (maxRate) {
                mTask.swSampleTime -= 1024000000000ULL / maxRate;
                //osLog(LOG_INFO, "lsm6dsmReadFifo tmp:%d\n", tmp);
            }
        }
        mTask.fifoDataToRead -= tmp;
    } else
        mTask.fifoDataToRead = 0;

    SPI_READ(LSM6DSM_FIFO_DATA_OUT_L_ADDR, mTask.fifoDataToRead, &mTask.regBuffer);

    if (temp_status & 0x04) {
        mTask.tempReady = true;
        SPI_READ(LSM6DSM_OUT_TEMP_L_ADDR, 2, &mTask.tempBuffer);
    } else
        mTask.tempReady = false;

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
    int16_t status = 0;

    if (sensorType == SENS_TYPE_ACCEL)
        accGetCalibration(SwCali, 0);
    else if (sensorType == SENS_TYPE_GYRO)
        gyroGetCalibration(SwCali, 0);

    raw_data[AXIS_X] = (buf[0] | buf[1] << 8);
    raw_data[AXIS_Y] = (buf[2] | buf[3] << 8);
    raw_data[AXIS_Z] = (buf[4] | buf[5] << 8);

    raw_data[AXIS_X] = raw_data[AXIS_X] + SwCali[AXIS_X];
    raw_data[AXIS_Y] = raw_data[AXIS_Y] + SwCali[AXIS_Y];
    raw_data[AXIS_Z] = raw_data[AXIS_Z] + SwCali[AXIS_Z];

    remap_data[mTask.cvt.map[AXIS_X]] = mTask.cvt.sign[AXIS_X] * raw_data[AXIS_X];
    remap_data[mTask.cvt.map[AXIS_Y]] = mTask.cvt.sign[AXIS_Y] * raw_data[AXIS_Y];
    remap_data[mTask.cvt.map[AXIS_Z]] = mTask.cvt.sign[AXIS_Z] * raw_data[AXIS_Z];


    if (sensorType == SENS_TYPE_ACCEL) {
        temp_data[AXIS_X] = (float)remap_data[AXIS_X] * GRAVITY_EARTH_SCALAR / mTask.sensors[ACC].sensitivity;
        temp_data[AXIS_Y] = (float)remap_data[AXIS_Y] * GRAVITY_EARTH_SCALAR / mTask.sensors[ACC].sensitivity;
        temp_data[AXIS_Z] = (float)remap_data[AXIS_Z] * GRAVITY_EARTH_SCALAR / mTask.sensors[ACC].sensitivity;

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
                osLog(LOG_INFO,
                      "ACCEL cali done %lld:caliResult[0]:%d, caliResult[1]:%d, caliResult[2]:%d, offset[0]:%f, offset[1]:%f, offset[2]:%f\n",
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
        /*
        osLog(LOG_INFO, "ACCEL:raw_data_x=%f, raw_data_y=%f, raw_data_z=%f\n",
            (double)temp_data[AXIS_X], (double)temp_data[AXIS_Y], (double)temp_data[AXIS_Z]);
        osLog(LOG_INFO, "ACCEL:calibrated_x=%f, calibrated_y=%f, calibrated_z=%f\n",
            (double)data->x, (double)data->y, (double)data->z);
        */
        mTask.accFactoryData.ix = (int32_t)(data->x * ACCELEROMETER_INCREASE_NUM_AP);
        mTask.accFactoryData.iy = (int32_t)(data->y * ACCELEROMETER_INCREASE_NUM_AP);
        mTask.accFactoryData.iz = (int32_t)(data->z * ACCELEROMETER_INCREASE_NUM_AP);

    } else if (sensorType == SENS_TYPE_GYRO) {
        temp_data[AXIS_X] = (float)remap_data[AXIS_X] * kScale_gyr; //DEGREE_TO_RADIRAN_SCALAR / mTask.sensors[GYR].sensitivity;
        temp_data[AXIS_Y] = (float)remap_data[AXIS_Y] * kScale_gyr; //DEGREE_TO_RADIRAN_SCALAR / mTask.sensors[GYR].sensitivity;
        temp_data[AXIS_Z] = (float)remap_data[AXIS_Z] * kScale_gyr; //DEGREE_TO_RADIRAN_SCALAR / mTask.sensors[GYR].sensitivity;

        if (UNLIKELY(mTask.sensors[GYR].startCali)) {
#ifndef CFG_MTK_CALIBRATION_V1_0
            status = Gyro_run_factory_calibration_timeout(0,
                temp_data, calibrated_data_output, (int *)&mTask.sensors[GYR].accuracy, 0, rtcGetTime());
            if (status != 0) {
                mTask.sensors[GYR].startCali = false;
                if (status > 0) {
                    osLog(LOG_INFO, "GYRO cali detect shake %lld\n", rtcGetTime());
                    caliResult[AXIS_X] = (int32_t)(mTask.sensors[GYR].staticCali[AXIS_X] * 1000);
                    caliResult[AXIS_Y] = (int32_t)(mTask.sensors[GYR].staticCali[AXIS_Y] * 1000);
                    caliResult[AXIS_Z] = (int32_t)(mTask.sensors[GYR].staticCali[AXIS_Z] * 1000);
                    accGyroSendCalibrationResult(SENS_TYPE_GYRO, (int32_t *)&caliResult[0], (uint8_t)status);
                } else
                    osLog(LOG_INFO, "GYRO cali time out %lld\n", rtcGetTime());
            } else if (mTask.sensors[GYR].accuracy == 3) {
                mTask.sensors[GYR].startCali = false;
                mTask.sensors[GYR].staticCali[AXIS_X] = calibrated_data_output[AXIS_X] - temp_data[AXIS_X];
                mTask.sensors[GYR].staticCali[AXIS_Y] = calibrated_data_output[AXIS_Y] - temp_data[AXIS_Y];
                mTask.sensors[GYR].staticCali[AXIS_Z] = calibrated_data_output[AXIS_Z] - temp_data[AXIS_Z];
                caliResult[AXIS_X] = (int32_t)(mTask.sensors[GYR].staticCali[AXIS_X] * 1000);
                caliResult[AXIS_Y] = (int32_t)(mTask.sensors[GYR].staticCali[AXIS_Y] * 1000);
                caliResult[AXIS_Z] = (int32_t)(mTask.sensors[GYR].staticCali[AXIS_Z] * 1000);
                accGyroSendCalibrationResult(SENS_TYPE_GYRO, (int32_t *)&caliResult[0], (uint8_t)status);
                osLog(LOG_INFO,
                      "GYRO cali done %lld: caliResult[0]:%d, caliResult[1]:%d, caliResult[2]:%d, offset[0]:%f, offset[1]:%f, offset[2]:%f\n",
                      rtcGetTime(), caliResult[AXIS_X], caliResult[AXIS_Y], caliResult[AXIS_Z],
                      (double)mTask.sensors[GYR].staticCali[AXIS_X],
                      (double)mTask.sensors[GYR].staticCali[AXIS_Y],
                      (double)mTask.sensors[GYR].staticCali[AXIS_Z]);
            }
#else
            status = gyroFactoryCalibrateRun(&mTask.gyroFactoryCal,
                temp_data[AXIS_X], temp_data[AXIS_Y], temp_data[AXIS_Z], rtcGetTime());
            if (status == UNSTABLE_DETECT) {
                mTask.sensors[GYR].startCali = false;
                caliResult[AXIS_X] = (int32_t)(mTask.sensors[GYR].staticCali[AXIS_X] * 1000);
                caliResult[AXIS_Y] = (int32_t)(mTask.sensors[GYR].staticCali[AXIS_Y] * 1000);
                caliResult[AXIS_Z] = (int32_t)(mTask.sensors[GYR].staticCali[AXIS_Z] * 1000);
                accGyroSendCalibrationResult(SENS_TYPE_GYRO, (int32_t *)&caliResult[0], (uint8_t)status);
            } else if (status == STILL_DETECT) {
                mTask.sensors[GYR].startCali = false;
                gyroFactoryCalibrateRemoveBias(&mTask.gyroFactoryCal,
                    temp_data[AXIS_X], temp_data[AXIS_Y], temp_data[AXIS_Z],
                    &calibrated_data_output[AXIS_X], &calibrated_data_output[AXIS_Y],
                    &calibrated_data_output[AXIS_Z]);
                gyroFactoryCalibrateGetBias(&mTask.gyroFactoryCal,
                    &mTask.sensors[GYR].staticCali[AXIS_X],
                    &mTask.sensors[GYR].staticCali[AXIS_Y],
                    &mTask.sensors[GYR].staticCali[AXIS_Z],
                    (int *)&mTask.sensors[GYR].accuracy);
                caliResult[AXIS_X] = (int32_t)(mTask.sensors[GYR].staticCali[AXIS_X] * 1000);
                caliResult[AXIS_Y] = (int32_t)(mTask.sensors[GYR].staticCali[AXIS_Y] * 1000);
                caliResult[AXIS_Z] = (int32_t)(mTask.sensors[GYR].staticCali[AXIS_Z] * 1000);
                accGyroSendCalibrationResult(SENS_TYPE_GYRO, (int32_t *)&caliResult[0], (uint8_t)status);
                osLog(LOG_INFO, "gyro cali bias: [%f, %f, %f]\n",
                    (double)mTask.sensors[GYR].staticCali[AXIS_X],
                    (double)mTask.sensors[GYR].staticCali[AXIS_Y],
                    (double)mTask.sensors[GYR].staticCali[AXIS_Z]);
            }
#endif
        }

        data->sensType = sensorType;
#ifndef CFG_MTK_CALIBRATION_V1_0
        data->x = temp_data[AXIS_X] + mTask.sensors[GYR].staticCali[AXIS_X];
        data->y = temp_data[AXIS_Y] + mTask.sensors[GYR].staticCali[AXIS_Y];
        data->z = temp_data[AXIS_Z] + mTask.sensors[GYR].staticCali[AXIS_Z];
#else
        data->x = temp_data[AXIS_X] - mTask.sensors[GYR].staticCali[AXIS_X];
        data->y = temp_data[AXIS_Y] - mTask.sensors[GYR].staticCali[AXIS_Y];
        data->z = temp_data[AXIS_Z] - mTask.sensors[GYR].staticCali[AXIS_Z];
#endif
        /*
        osLog(LOG_INFO, "GYRO:raw_data_x=%f, raw_data_y=%f, raw_data_z=%f\n",
            (double)temp_data[AXIS_X], (double)temp_data[AXIS_Y], (double)temp_data[AXIS_Z]);
        osLog(LOG_INFO, "GYRO:calibrated_x=%f, calibrated_y=%f, calibrated_z=%f\n",
            (double)data->x, (double)data->y, (double)data->z);
        */
        mTask.gyroFactoryData.ix =
            (int32_t)((float)data->x * GYROSCOPE_INCREASE_NUM_AP / DEGREE_TO_RADIRAN_SCALAR);
        mTask.gyroFactoryData.iy =
            (int32_t)((float)data->y * GYROSCOPE_INCREASE_NUM_AP / DEGREE_TO_RADIRAN_SCALAR);
        mTask.gyroFactoryData.iz =
            (int32_t)((float)data->z * GYROSCOPE_INCREASE_NUM_AP / DEGREE_TO_RADIRAN_SCALAR);
    }

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

static int lsm6dsmConvert(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                          void *inBuf, uint8_t inSize, uint8_t elemInSize,
                          void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    struct accGyroData *data = mTask.accGyroPacket.outBuf;

    uint8_t accEventSize = 0;
    uint8_t gyroEventSize = 0;
    uint64_t swSampleTime = 0, realSampleTime = 0;
    uint8_t *fifo_data = &mTask.regBuffer[1];

    uint16_t fifo_offset = 0;
    uint8_t gyro_sip, accel_sip;
    int16_t temperature16 = 0;
    float temperature = 0;
    uint32_t accDelay = 0, gyroDelay = 0;
    //osLog(LOG_INFO, "lsm6dsmConvert, GYR Discard:%d, ACC Discard:%d\n",
    //mTask.sensors[GYR].samplesToDiscard, mTask.sensors[ACC].samplesToDiscard);

    while (fifo_offset < mTask.fifoDataToRead) {
        gyro_sip = mTask.sensors[GYR].fifoSip;
        accel_sip = mTask.sensors[ACC].fifoSip;

        do {
            if (gyro_sip > 0) {
                if (mTask.sensors[GYR].samplesToDiscard > 0) {
                    mTask.sensors[GYR].samplesToDiscard--;
                } else {
                    if ((accEventSize + gyroEventSize) < MAX_RECV_PACKET) {
                        parseRawData(&data[accEventSize + gyroEventSize], &fifo_data[fifo_offset], SENS_TYPE_GYRO);
                        gyroEventSize++;
                    } else
                        osLog(LOG_ERROR, "outBuf full, accEventSize = %d, gyroEventSize = %d\n", accEventSize, gyroEventSize);
                }

                fifo_offset += LSM6DSM_ONE_SAMPLE_BYTE;
                gyro_sip--;
            }

            if (accel_sip > 0) {
                if (mTask.sensors[ACC].samplesToDiscard > 0)
                    mTask.sensors[ACC].samplesToDiscard--;
                else {
                    if ((accEventSize + gyroEventSize) < MAX_RECV_PACKET) {
                        parseRawData(&data[accEventSize + gyroEventSize], &fifo_data[fifo_offset], SENS_TYPE_ACCEL);
                        accEventSize++;
                    } else
                        osLog(LOG_ERROR, "outBuf full, accEventSize = %d, gyroEventSize = %d\n", accEventSize, gyroEventSize);
                }

                fifo_offset += LSM6DSM_ONE_SAMPLE_BYTE;
                accel_sip--;
            }
        } while ((accel_sip > 0) || (gyro_sip > 0));
    }
#ifndef CFG_MTK_CALIBRATION_V1_0
    /*if factory true , can't send to runtime cali in parseRawData in accGyro*/
    if (mTask.sensors[ACC].startCali || mTask.sensors[GYR].startCali) {
        accEventSize = 0;
        gyroEventSize = 0;
        registerAccGyroFifoInfo((mTask.sensors[ACC].hwRate == 0) ? 0 : 1024000000000 / mTask.sensors[ACC].hwRate,
                                (mTask.sensors[GYR].hwRate == 0) ? 0 : 1024000000000 / mTask.sensors[GYR].hwRate);
    }
#endif
    if (mTask.tempReady) {
        uint16_t temp_h = mTask.tempBuffer[2];
        uint16_t temp_l = mTask.tempBuffer[1];
        temperature16 = (int16_t)((temp_h << 8) | temp_l);
        mTask.temperature = 25.0f + (float)temperature16 * kScale_temp;
        temperature = mTask.temperature;
    } else {
        temperature = mTask.temperature;
    }
    //osLog(LOG_INFO, "lsm6dsmConvert, fifoDataToRead:%d, accEventSize:%d, gyroEventSize:%d, temperature:%f\n",
    //mTask.fifoDataToRead, accEventSize, gyroEventSize, (double)temperature);
    //osLog(LOG_INFO, "lsm6dsmConvert, fifoDataToRead:%d, accEventSize:%d, gyroEventSize:%d\n",
    //mTask.fifoDataToRead, accEventSize, gyroEventSize);
    swSampleTime = mTask.swSampleTime;
    getRealSampleDelay(&accDelay, &gyroDelay);
    realSampleTime = calcFakeInterruptTime2(swSampleTime, mTask.hwSampleTime, mTask.lastSampleTime,
                                           accDelay, mTask.sensors[ACC].configed, accEventSize,
                                           gyroDelay, mTask.sensors[GYR].configed, gyroEventSize);

    //osLog(LOG_INFO, "lsm6dsmConvert, swSampleTime=%lld, hwSampleTime=%lld, realSampleTime=%lld, lastSampleTime=%lld, now=%lld\n",
    //swSampleTime, mTask.hwSampleTime, realSampleTime, mTask.lastSampleTime, rtcGetTime());

    mTask.hwSampleTime = realSampleTime;
    mTask.lastSampleTime = realSampleTime;

    txTransferDataInfo(&mTask.dataInfo, accEventSize, gyroEventSize, realSampleTime, data, temperature);


    mt_eint_unmask(mTask.hw->eint_num);
    sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
    return 0;
}


static int lsm6dsmSwReset(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                          void *inBuf, uint8_t inSize, uint8_t elemInSize,
                          void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    osLog(LOG_INFO, "lsm6dsmSwReset\n");

    SPI_WRITE(LSM6DSM_CTRL3_C_ADDR, LSM6DSM_SW_RESET, 50000);
    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
}

static int lsm6dsmInitReg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                          void *inBuf, uint8_t inSize, uint8_t elemInSize,
                          void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    osLog(LOG_INFO, "lsm6dsmInitReg\n");

    /* During init, reset all configurable registers to default values */
    SPI_WRITE(LSM6DSM_FUNC_CFG_ACCESS_ADDR, LSM6DSM_FUNC_CFG_ACCESS_BASE, 50); /*disable access embed reg*/
    SPI_WRITE(LSM6DSM_DRDY_PULSE_CFG_ADDR, LSM6DSM_DRDY_PULSE_CFG_BASE); /*bit 8 latched mode*/

    SPI_WRITE(LSM6DSM_CTRL1_XL_ADDR, LSM6DSM_CTRL1_XL_BASE, 5); /*acc power down +-8g, bandwidth = odr/4*/
    SPI_WRITE(LSM6DSM_CTRL2_G_ADDR, LSM6DSM_CTRL2_G_BASE, 5); /*gyro power down 2000dps*/

#ifdef CFG_HW_STEP_COUNTER_SUPPORT
    SPI_WRITE(LSM6DSM_CTRL10_C_ADDR, LSM6DSM_CTRL10_C_BASE, 5);
    SPI_WRITE(LSM6DSM_FUNC_CFG_ACCESS_ADDR, LSM6DSM_FUNC_CFG_ACCESS_FUNC_CFG_EN, 5);
    SPI_WRITE(LSM6DSM_PEDO_THS_REG, 0x8E, 5);
    SPI_WRITE(LSM6DSM_FUNC_CFG_ACCESS_ADDR, LSM6DSM_FUNC_CFG_ACCESS_BASE, 50); /*disable access embed reg*/
#endif
    SPI_WRITE(LSM6DSM_CTRL3_C_ADDR, LSM6DSM_CTRL3_C_BASE, 5); /*BDU INC*/
    SPI_WRITE(LSM6DSM_CTRL4_C_ADDR, LSM6DSM_CTRL4_C_BASE, 5); /*signal on int1, I2C disable*/
    SPI_WRITE(LSM6DSM_CTRL5_C_ADDR, LSM6DSM_CTRL5_C_BASE, 5);
    SPI_WRITE(LSM6DSM_CTRL6_C_ADDR, LSM6DSM_CTRL6_C_BASE, 25);

    SPI_WRITE(LSM6DSM_INT1_CTRL_ADDR, LSM6DSM_INT1_CTRL_BASE); /*config fifo int*/
    mTask.accGyroPowerd = true;
    SPI_WRITE(LSM6DSM_TAP_CFG_ADDR, LSM6DSM_TAP_CFG_BASE); /* INT LATCH  */

    return spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
}

static void sensorCoreRegistration(void)
{
    struct sensorCoreInfo mInfo;
    osLog(LOG_INFO, "lsm6dsmRegisterCore\n");

    /* Register sensor Core */
    memset(&mInfo, 0x00, sizeof(struct sensorCoreInfo));
    mInfo.sensType = SENS_TYPE_ACCEL;
    mInfo.gain = GRAVITY_EARTH_1000;
    mInfo.sensitivity = mTask.sensors[ACC].sensitivity;
    mInfo.cvt = mTask.cvt;
    mInfo.getCalibration = accGetCalibration;
    mInfo.setCalibration = accSetCalibration;
    mInfo.getData = accGetData;
    mInfo.setDebugTrace = lsm6dsmSetDebugTrace;
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

static int lsm6dsmSensorRegistration(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                     void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                     void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    sensorCoreRegistration();
    sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int lsm6dsmEintRegistration(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                   void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                   void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{

    mt_eint_dis_hw_debounce(mTask.hw->eint_num);
    mt_eint_registration(mTask.hw->eint_num, LEVEL_SENSITIVE, HIGH_LEVEL_TRIGGER, lsm6dsmIsr1, EINT_INT_UNMASK,
                         EINT_INT_AUTO_UNMASK_OFF);
    sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static struct sensorFsm lsm6dsmFsm[] = {
    sensorFsmCmd(STATE_HW_INT_STATUS_CHECK, STATE_HW_INT_HANDLING, lsm6dsmIntStatusCheck),
    sensorFsmCmd(STATE_HW_INT_HANDLING, STATE_HW_INT_HANDLING_DONE, lsm6dsmIntHandling),

    sensorFsmCmd(STATE_SAMPLE, STATE_FIFO, lsm6dsmSample),
    sensorFsmCmd(STATE_FIFO, STATE_CONVERT, lsm6dsmReadFifo),
    sensorFsmCmd(STATE_CONVERT, STATE_SAMPLE_DONE, lsm6dsmConvert),

    sensorFsmCmd(STATE_ACC_ENABLE, STATE_ACC_ENABLE_DONE, lsm6dsmAccPowerOn),
    sensorFsmCmd(STATE_ACC_DISABLE, STATE_ACC_DISABLE_DONE, lsm6dsmAccPowerOff),

    sensorFsmCmd(STATE_ACC_RATECHG, STATE_ACC_RATECHG_DONE, lsm6dsmAccRate),

    sensorFsmCmd(STATE_ACC_CALI, STATE_ACC_CALI_DONE, lsm6dsmAccCali),

    sensorFsmCmd(STATE_ACC_CFG, STATE_ACC_CFG_DONE, lsm6dsmAccCfgCali),

    sensorFsmCmd(STATE_GYRO_ENABLE, STATE_GYRO_ENABLE_DONE, lsm6dsmGyroPowerOn),
    sensorFsmCmd(STATE_GYRO_DISABLE, STATE_GYRO_DISABLE_DONE, lsm6dsmGyroPowerOff),

    sensorFsmCmd(STATE_GYRO_RATECHG, STATE_GYRO_RATECHG_DONE, lsm6dsmGyroRate),

    sensorFsmCmd(STATE_GYRO_CALI, STATE_GYRO_CALI_DONE, lsm6dsmGyroCali),

    sensorFsmCmd(STATE_GYRO_CFG, STATE_GYRO_CFG_DONE, lsm6dsmGyroCfgCali),

    /* For Anymotion */
    sensorFsmCmd(STATE_ANYMO_ENABLE, STATE_ANYMO_ENABLE_DONE, anyMotionPowerOn),
    sensorFsmCmd(STATE_ANYMO_DISABLE, STATE_ANYMO_DISABLE_DONE, anyMotionPowerOff),

#ifdef CFG_HW_STEP_COUNTER_SUPPORT
    /* for stepCounter */
    sensorFsmCmd(CHIP_STEP_COUNT_ENABLE, CHIP_STEP_COUNT_DONE, stepCountPowerOn),
    sensorFsmCmd(CHIP_STEP_COUNT_DISABLE, CHIP_STEP_COUNT_DISABLE_DONE, stepCountPowerOff),
    sensorFsmCmd(CHIP_STEP_COUNT_SAMPLE, CHIP_STEP_COUNT_SEND, stepCntGetData),
    sensorFsmCmd(CHIP_STEP_COUNT_SEND, CHIP_STEP_COUNT_SAMPLE_DONE, StepCntSendData),
#endif
    sensorFsmCmd(STATE_SW_RESET, STATE_INIT_REG, lsm6dsmSwReset),
    sensorFsmCmd(STATE_INIT_REG, STATE_SENSOR_REGISTRATION, lsm6dsmInitReg),
    sensorFsmCmd(STATE_SENSOR_REGISTRATION, STATE_EINT_REGISTRATION, lsm6dsmSensorRegistration),
    sensorFsmCmd(STATE_EINT_REGISTRATION, STATE_INIT_DONE, lsm6dsmEintRegistration),
};

static void initSensorStruct(struct lsm6dsmSensor *sensor, enum SensorIndex idx)
{
    sensor->powered = false;
    sensor->configed = false;
    sensor->lowPower = false;

    sensor->startCali = false;
    sensor->staticCali[AXIS_X] = 0;
    sensor->staticCali[AXIS_Y] = 0;
    sensor->staticCali[AXIS_Z] = 0;
    sensor->accuracy = 0;

    sensor->rate = 0;
    sensor->latency = SENSOR_LATENCY_NODATA;
    sensor->hwRate = 0;  //rate set in hw
    sensor->preRealRate = 0;

    sensor->samplesDecimator = 0;
    sensor->samplesDecimatorCounter = 0;
    sensor->samplesToDiscard = 0;
    sensor->decimators = 0;
    sensor->fifoSip = 0;
}

int lsm6dsmInit(void)
{
    int ret = 0;
    uint8_t txData[2] = {0}, rxData[2] = {0};
    enum SensorIndex i;

    insertMagicNum(&mTask.accGyroPacket);
    mTask.hw = get_cust_accGyro("lsm6dsm");

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

    mTask.sensors[ACC].sensitivity = (float)65536 / (8 * 2);
    mTask.sensors[GYR].sensitivity = (float)1000 / 70;

    mTask.watermark = 0;
    mTask.temperature = 25.0f;  //init with 25 ¡æ
    mTask.tempReady = false;
    mTask.fifoDataToRead = 0;
    mTask.accGyroPowerd = false;

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
    txData[0] = LSM6DSM_WAI_ADDR | 0x80;
    for (uint8_t i = 0; i < 3;) {
        ret = spiMasterRxTxSync(mTask.spiDev, rxData, txData, 2);
        if (ret >= 0 && (rxData[1] == LSM6DSM_WAI_VALUE))
            break;
        ++i;
        if (i >= 3) {
            ret = -1;
            sendSensorErrToAp(ERR_SENSOR_ACC_GYR, ERR_CASE_ACC_GYR_INIT, ACC_NAME);
            sendSensorErrToAp(ERR_SENSOR_ACC_GYR, ERR_CASE_ACC_GYR_INIT, GYRO_NAME);
            spiMasterRelease(mTask.spiDev);
            disable_latch_time(mTask.latch_time_id);
            free_latch_time(mTask.latch_time_id);
            goto err_out;
        }
    }

    osLog(LOG_INFO, "lsm6dsm: auto detect success: %02x\n", rxData[1]);
    accSensorRegister();
    gyroSensorRegister();
    anyMotionSensorRegister();
#ifdef CFG_HW_STEP_COUNTER_SUPPORT
    stepCntSensorRegister();
#endif
    registerAccGyroInterruptMode(ACC_GYRO_FIFO_INTERRUPTIBLE);
    registerAccGyroDriverFsm(lsm6dsmFsm, ARRAY_SIZE(lsm6dsmFsm));

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

    const struct GyroFactoryCalParameters gyro_factory_cal_parameters = {
        .param = {
            .var_threshold = 5e-5f,
            .bias_mean_limit = 0.1f,
            .debounce_time_duration_nanos = SEC_TO_NANOS(0.5f),
            .window_time_duration_nanos = SEC_TO_NANOS(1),
            .standard_x = 0.0f,
            .standard_y = 0.0f,
            .standard_z = 0.0f,
            .factory_calibration_enable = true,
        },
    };
    gyroFactoryCalibrateParam(&mTask.gyroFactoryCal, &gyro_factory_cal_parameters);

    const struct GyroCalParameters gyro_cal_parameters = {
        .min_still_duration_nanos = SEC_TO_NANOS(5.9f),
        .max_still_duration_nanos = SEC_TO_NANOS(6.9f),
        .calibration_time_nanos = 0,
        .window_time_duration_nanos = SEC_TO_NANOS(1.5f),
        .bias_x = 0,
        .bias_y = 0,
        .bias_z = 0,
        .stillness_threshold = 0.95f,
        .stillness_mean_delta_limit = MDEG_TO_RAD * 40.0f,
        .gyro_var_threshold = 5e-5f,
        .gyro_confidence_delta = 1e-5f,
        .accel_var_threshold = 8e-3f,
        .accel_confidence_delta = 1.6e-3f,
        .mag_var_threshold = 1.4f,
        .mag_confidence_delta = 0.25f,
        .temperature_delta_limit_celsius = 1.5f,
        .gyro_calibration_enable = true,
    };
    gyroCalInit(GetGyroCalBase(), &gyro_cal_parameters);

    const struct OverTempCalParameters gyro_otc_parameters = {
        .min_temp_update_period_nanos = MSEC_TO_NANOS(500),
        .age_limit_nanos = DAYS_TO_NANOS(2),
        .delta_temp_per_bin = 0.75f,
        .jump_tolerance = 40.0f * MDEG_TO_RAD,
        .outlier_limit = 50.0f * MDEG_TO_RAD,
        .temp_sensitivity_limit = 80.0f * MDEG_TO_RAD,
        .sensor_intercept_limit = 3.0e3f * MDEG_TO_RAD,
        .significant_offset_change = 0.1f * MDEG_TO_RAD,
        .min_num_model_pts = 5,
        .over_temp_enable = true,
    };
    overTempCalInit(GetOverTempCalBase(), &gyro_otc_parameters);
#endif
err_out:
    return ret;
}
#ifndef CFG_OVERLAY_INIT_SUPPORT
MODULE_DECLARE(lsm6dsm, SENS_TYPE_ACCEL, lsm6dsmInit);
#else
#include "mtk_overlay_init.h"
OVERLAY_DECLARE(lsm6dsm, OVERLAY_ID_ACCGYRO, lsm6dsmInit);
#endif
