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
#include <nanohub_math.h>
#include <nanohubPacket.h>
#include <plat/inc/rtc.h>
#include <sensors.h>
#include <seos.h>
#include <slab.h>
#include <spi.h>
#include <plat/inc/spichre.h>
#include <spichre-plat.h>
#include <timer.h>
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
#include "performance.h"

#define ULONG_LONG_MAX 0XFFFFFFFFFFFFFFFFULL

#define INFO_PRINT(fmt, ...) do { \
        osLog(LOG_INFO, "%s " fmt, "[BMI160]", ##__VA_ARGS__); \
    } while (0);

#define ERROR_PRINT(fmt, ...) do { \
        osLog(LOG_ERROR, "%s " fmt, "[BMI160] ERROR:", ##__VA_ARGS__); \
    } while (0);

#define DEBUG_PRINT(fmt, ...) do { \
        if (DBG_ENABLE) {  \
            INFO_PRINT(fmt,  ##__VA_ARGS__); \
        } \
    } while (0);

#define DEBUG_PRINT_IF(cond, fmt, ...) do { \
        if ((cond) && DBG_ENABLE) {  \
            INFO_PRINT(fmt,  ##__VA_ARGS__); \
        } \
    } while (0);

#define DBG_ENABLE                0
#define DBG_CHUNKED               1
#define DBG_INT                   0
#define DBG_SHALLOW_PARSE         1
#define DBG_STATE                 0
#define DBG_WM_CALC               0
#define TIMESTAMP_DBG             0
#ifdef CFG_HW_STEP_COUNTER_SUPPORT
#define STEPCNT_DBG               0
#endif
// fixme: to list required definitions for a slave mag
#ifdef USE_BMM150
#include "bosch_bmm150_slave.h"
#elif USE_AK09915
#include "akm_ak09915_slave.h"
#endif

#define ACC_NAME     "bmi160_acc"
#define GYRO_NAME     "bmi160_gyro"

#define BMI160_APP_ID APP_ID_MAKE(APP_ID_VENDOR_GOOGLE, 2)

#define BMI160_SPI_WRITE          0x00
#define BMI160_SPI_READ           0x80

#define BMI160_SPI_BUS_ID         1
#define BMI160_SPI_SPEED_HZ       8000000
#define BMI160_SPI_MODE           3

#define BMI160_INT_IRQ            EXTI9_5_IRQn
#define BMI160_INT1_PIN           GPIO_PB(6)
#define BMI160_INT2_PIN           GPIO_PB(7)

#define BMI160_ID                 0xd1

#define BMI160_REG_ID             0x00
#define BMI160_REG_ERR            0x02
#define BMI160_REG_PMU_STATUS     0x03
#define BMI160_REG_DATA_0         0x04
#define BMI160_REG_DATA_1         0x05
#define BMI160_REG_DATA_14        0x12
#define BMI160_REG_SENSORTIME_0   0x18
#define BMI160_REG_STATUS         0x1b
#define BMI160_REG_INT_STATUS_0   0x1c
#define BMI160_REG_INT_STATUS_1   0x1d
#define BMI160_REG_TEMPERATURE_0  0x20
#define BMI160_REG_TEMPERATURE_1  0x21
#define BMI160_REG_FIFO_LENGTH_0  0x22
#define BMI160_REG_FIFO_DATA      0x24
#define BMI160_REG_ACC_CONF       0x40
#define BMI160_REG_ACC_RANGE      0x41
#define BMI160_REG_GYR_CONF       0x42
#define BMI160_REG_GYR_RANGE      0x43
#define BMI160_REG_MAG_CONF       0x44
#define BMI160_REG_FIFO_DOWNS     0x45
#define BMI160_REG_FIFO_CONFIG_0  0x46
#define BMI160_REG_FIFO_CONFIG_1  0x47
#define BMI160_REG_MAG_IF_0       0x4b
#define BMI160_REG_MAG_IF_1       0x4c
#define BMI160_REG_MAG_IF_2       0x4d
#define BMI160_REG_MAG_IF_3       0x4e
#define BMI160_REG_MAG_IF_4       0x4f
#define BMI160_REG_INT_EN_0       0x50
#define BMI160_REG_INT_EN_1       0x51
#define BMI160_REG_INT_EN_2       0x52
#define BMI160_REG_INT_OUT_CTRL   0x53
#define BMI160_REG_INT_LATCH      0x54
#define BMI160_REG_INT_MAP_0      0x55
#define BMI160_REG_INT_MAP_1      0x56
#define BMI160_REG_INT_MAP_2      0x57
#define BMI160_REG_INT_DATA_0     0x58
#define BMI160_REG_INT_MOTION_0   0x5f
#define BMI160_REG_INT_MOTION_1   0x60
#define BMI160_REG_INT_MOTION_2   0x61
#define BMI160_REG_INT_MOTION_3   0x62
#define BMI160_REG_INT_TAP_0      0x63
#define BMI160_REG_INT_TAP_1      0x64
#define BMI160_REG_INT_FLAT_0     0x67
#define BMI160_REG_INT_FLAT_1     0x68
#define BMI160_REG_PMU_TRIGGER    0x6C
#define BMI160_REG_FOC_CONF       0x69
#define BMI160_REG_CONF           0x6a
#define BMI160_REG_IF_CONF        0x6b
#define BMI160_REG_SELF_TEST      0x6d
#define BMI160_REG_OFFSET_0       0x71
#define BMI160_REG_OFFSET_3       0x74
#define BMI160_REG_OFFSET_6       0x77
#define BMI160_REG_STEP_CNT_0     0x78
#define BMI160_REG_STEP_CONF_0    0x7a
#define BMI160_REG_STEP_CONF_1    0x7b
#define BMI160_REG_CMD            0x7e
#define BMI160_REG_MAGIC          0x7f

#define INT_STEP        0x01
#define INT_ANY_MOTION  0x04
#define INT_DOUBLE_TAP  0x10
#define INT_SINGLE_TAP  0x20
#define INT_ORIENT      0x40
#define INT_FLAT        0x80
#define INT_HIGH_G_Z    0x04
#define INT_LOW_G       0x08
#define INT_DATA_RDY    0x10
#define INT_FIFO_FULL   0x20
#define INT_FIFO_WM     0x40
#define INT_NO_MOTION   0x80

#define BMI160_FRAME_HEADER_INVALID  0x80   // mark the end of valid data
#define BMI160_FRAME_HEADER_SKIP     0x81   // not defined by hw, used for skip a byte in buffer

#define WATERMARK_MIN                1
#define WATERMARK_MAX                200    // must <= 255 (0xff)

#define WATERMARK_MAX_SENSOR_RATE    400    // Accel and gyro are 400 Hz max
#define WATERMARK_TIME_UNIT_NS       (1000000000ULL/(WATERMARK_MAX_SENSOR_RATE))

#define gSPI    BMI160_SPI_BUS_ID

#define ACCL_INT_LINE EXTI_LINE_P6
#define GYR_INT_LINE EXTI_LINE_P7

#define SPI_WRITE_0(addr, data) spiQueueWrite(addr, data, 2)
#define SPI_WRITE_1(addr, data, delay) spiQueueWrite(addr, data, delay)
#define GET_SPI_WRITE_MACRO(_1, _2, _3, NAME, ...) NAME
#define SPI_WRITE(...) GET_SPI_WRITE_MACRO(__VA_ARGS__, SPI_WRITE_1, SPI_WRITE_0)(__VA_ARGS__)

#define SPI_READ_0(addr, size, buf) spiQueueRead(addr, size, buf, 0)
#define SPI_READ_1(addr, size, buf, delay) spiQueueRead(addr, size, buf, delay)
#define GET_SPI_READ_MACRO(_1, _2, _3, _4, NAME, ...) NAME
#define SPI_READ(...) GET_SPI_READ_MACRO(__VA_ARGS__, SPI_READ_1, SPI_READ_0)(__VA_ARGS__)

#define EVT_SENSOR_ACC_DATA_RDY sensorGetMyEventType(SENS_TYPE_ACCEL)
#define EVT_SENSOR_GYR_DATA_RDY sensorGetMyEventType(SENS_TYPE_GYRO)
#define EVT_SENSOR_ANY_MOTION sensorGetMyEventType(SENS_TYPE_ANY_MOTION)
#define EVT_SENSOR_NO_MOTION sensorGetMyEventType(SENS_TYPE_NO_MOTION)
#ifdef CFG_HW_STEP_COUNTER_SUPPORT
#define EVT_SENSOR_STEP_COUNTER sensorGetMyEventType(SENS_TYPE_STEP_COUNT)
#endif
#define MAX_NUM_COMMS_EVENT_SAMPLES 15

#define BMI160_FS_125_LSB               2624
#define BMI160_FS_250_LSB               1312
#define BMI160_FS_500_LSB               656
#define BMI160_FS_1000_LSB              328
#define BMI160_FS_2000_LSB              164

#define kScale_acc    0.00239501953f  // ACC_range * 9.81f / 32768.0f;
#define kScale_gyr    (DEGREE_TO_RADIRAN_SCALAR / 16.4f) //0.00106472439f  // GYR_range * M_PI / (180.0f * 32768.0f);
#define kScale_temp   0.001953125f    // temperature in deg C
#define kTempInvalid  -1000.0f

#define kTimeSyncPeriodNs        100000000ull  // sync sensor and RTC time every 100ms
#define kSensorTimerIntervalUs   39ull        // bmi160 clock increaments every 39000ns

#define kMinRTCTimeIncrementNs   1250000ull  // forced min rtc time increment, 1.25ms for 400Hz
#define kMinSensorTimeIncrement  64         // forced min sensortime increment,
                                            // 64 = 2.5 msec for 400Hz

#define ACC_MIN_RATE    5
#define GYR_MIN_RATE    6
#define ACC_MAX_RATE    12
#define GYR_MAX_RATE    13
#define ACC_MAX_OSR     3
#define GYR_MAX_OSR     4
#define OSR_THRESHOLD   8

#define MOTION_ODR         7

#define RETRY_CNT_CALIBRATION 1
#define RETRY_CNT_ID 5
#define RETRY_CNT_MAG 30

#define SPI_PACKET_SIZE 30
#define FIFO_READ_SIZE  (1024+4)
#define CHUNKED_READ_SIZE (64)
#define BUF_MARGIN 32   // some extra buffer for additional reg RW when a FIFO read happens
#define SPI_BUF_SIZE (FIFO_READ_SIZE + CHUNKED_READ_SIZE + BUF_MARGIN)

enum SensorIndex {
    ACC = 0,
    GYR,
    ANYMO,
    NOMO,
#ifdef CFG_HW_STEP_COUNTER_SUPPORT
    STEPCNT,
#endif
    NUM_OF_SENSOR,
};

enum SensorEvents {
    NO_EVT = -1,
    EVT_SPI_DONE = EVT_APP_START + 1,
    EVT_SENSOR_INTERRUPT_1,
    EVT_SENSOR_INTERRUPT_2,
    EVT_TIME_SYNC,
};

enum InitState {
    RESET_BMI160,
    INIT_BMI160,
    INIT_ON_CHANGE_SENSORS,
    INIT_DONE,
};

enum CalibrationState {
    CALIBRATION_START,
    CALIBRATION_FOC,
    CALIBRATION_STATUS_CHECK,
    CALIBRATION_SET_OFFSET,
    CALIBRATION_TIMEOUT,
};

enum SensorState {
    SENSOR_BOOT,
    SENSOR_VERIFY_ID,
    SENSOR_INITIALIZING,
    SENSOR_IDLE,
    SENSOR_POWERING_UP,
    SENSOR_POWERING_DOWN,
    SENSOR_CONFIG_CHANGING,
    SENSOR_INT_1_HANDLING,
    SENSOR_INT_2_HANDLING,
    SENSOR_CALIBRATING,
    SENSOR_TIME_SYNC,
    SENSOR_SAVE_CALIBRATION,
    SENSOR_NUM_OF_STATE
};
static const char * getStateName(int32_t s) {
#if DBG_STATE
    static const char* const l[] = {"BOOT", "VERIFY_ID", "INIT", "IDLE", "PWR_UP",
            "PWR-DN", "CFG_CHANGE", "INT1", "INT2", "CALIB", "STEP_CNT", "SYNC", "SAVE_CALIB"};
    if (s >= 0 && s < SENSOR_NUM_OF_STATE) {
        return l[s];
    }
#endif
    return "???";
}

struct ConfigStat {
    uint64_t latency;
    uint32_t rate;
    bool enable;
};

struct BMI160Sensor {
    struct ConfigStat pConfig;  // pending config status request
    struct TripleAxisDataEvent *data_evt;
    uint32_t handle;
    uint32_t rate;
    uint64_t latency;
    uint64_t prev_rtc_time;
    uint32_t offset[3];
    bool powered;  // activate status
    bool configed;  // configure status
    bool offset_enable;
    bool startCali;
    uint8_t flush;
};

struct BMI160Task {
    uint32_t tid;
    struct BMI160Sensor sensors[NUM_OF_SENSOR];

    // time keeping.
    uint64_t last_sensortime;
    uint64_t frame_sensortime;
    uint64_t prev_frame_time[3];
    uint64_t time_delta[3];
    uint64_t next_delta[3];
    uint64_t tempTime;

    // spi and interrupt
    spi_cs_t cs;
    struct SpiMode mode;
    struct SpiPacket packets[SPI_PACKET_SIZE];
    struct SpiDevice *spiDev;
    time_sync_t gSensorTime2RTC;

    float tempCelsius;
    uint32_t poll_generation;
    uint32_t active_poll_generation;
    uint8_t active_oneshot_sensor_cnt;
    uint8_t interrupt_enable_0;
#ifdef CFG_HW_STEP_COUNTER_SUPPORT
    uint8_t interrupt_enable_1;
    bool step_cnt_changed;
    uint32_t total_step_cnt;
    uint32_t last_step_cnt;
#endif
    uint8_t interrupt_enable_2;
    uint8_t acc_downsample;
    uint8_t gyr_downsample;
    bool fifo_enabled[3];

    // spi buffers
    int xferCnt;
    uint8_t *dataBuffer;
    uint8_t *statusBuffer;
    uint8_t temperatureBuffer[4];
    uint8_t txrxBuffer[SPI_BUF_SIZE];

    // states
    volatile uint8_t state;  // task state, type enum SensorState, do NOT change this directly
    enum InitState init_state;
    enum CalibrationState calibration_state;

    // pending configs
    bool pending_int[2];
    bool pending_config[NUM_OF_SENSOR];
    bool pending_calibration_save;
    bool pending_time_sync;
    bool pending_delta[3];
    bool pending_dispatch;
    bool frame_sensortime_valid;

    // FIFO setting
    uint16_t chunkReadSize;
    uint16_t  watermark;

    // spi rw
    struct SlabAllocator *mDataSlab;
    uint16_t mWbufCnt;
    uint8_t mRegCnt;
    uint8_t mRetryLeft;
    bool spiInUse;

    SpiCbkF spiCallBack;
    void *next_state;

    struct transferDataInfo dataInfo;
    struct accGyroDataPacket accGyroPacket;
    uint64_t hwSampleTime;
    uint64_t swSampleTime;
    uint64_t lastSampleTime;
    struct accGyro_hw *hw;
    struct sensorDriverConvert cvt;
    /* data for factory */
    struct TripleAxisDataPoint accFactoryData;
    struct TripleAxisDataPoint gyroFactoryData;

    int32_t accSwCali[AXES_NUM];
    int32_t gyroSwCali[AXES_NUM];
    int32_t debug_trace;
    int latch_time_id;
};

static struct BMI160Task mTask;
static struct BMI160Task *bmi160DebugPoint;
typedef struct BMI160Task _Task;
#define TASK  _Task* const _task

// To get rid of static variables all task functions should have a task structure pointer input.
// This is an intermediate step.
#define TDECL()  TASK = &mTask; (void)_task

// Access task variables without explicitly specify the task structure pointer.
#define T(v)  (_task->v)

// Atomic get state
#define GET_STATE() (atomicReadByte(&(_task->state)))

// Atomic set state, this set the state to arbitrary value, use with caution
#define SET_STATE(s) do {\
        DEBUG_PRINT_IF(DBG_STATE, "set state %s\n", getStateName(s));\
        atomicWriteByte(&(_task->state), (s));\
    }while(0)

#define BMI160_TO_ANDROID_COORDINATE(x, y, z)   \
    do {                                        \
        float xi = x, yi = y, zi = z;         \
        x = -yi; y = xi; z = zi;                \
    } while (0)

#define max(x, y)   (x > y ? x : y)
// Short-hand
#define trySwitchState(s) trySwitchState_(_task, (s))

// Chunked FIFO read functions
static void chunkedReadInit_(TASK, int index, int size);
#define chunkedReadInit(a, b) chunkedReadInit_(_task, (a), (b))
static void chunkedReadSpiCallback(void *cookie, int error);
static void initiateFifoRead_(TASK, bool isInterruptContext);
#define initiateFifoRead(a) initiateFifoRead_(_task, (a))
static uint8_t* shallowParseFrame(uint8_t * buf, int size);

static void accGetCalibration(int32_t *cali, int32_t size);
static void gyroGetCalibration(int32_t *cali, int32_t size);

static uint8_t offset6Mode(void);

enum BMI160State {
    STATE_RESET_BMI160 = CHIP_RESET + 1,
    STATE_INIT_BMI160,
    STATE_INIT_ON_CHANGE_SENSORS,
    STATE_CONVERT,
    STATE_UNMASK_EINT,
    STATE_CALIBRATION_FOC,
    STATE_CALIBRATION_STATUS_CHECK,
    STATE_CALIBRATION_SET_OFFSET,
    STATE_HW_CALI_CHECK,
};

static uint8_t calcWatermark(uint32_t numOfData) {
    uint16_t watermark;
    // Data size
    watermark = numOfData * 6;

    // Header Size
    watermark += numOfData * max(mTask.sensors[ACC].rate, mTask.sensors[GYR].rate) / (mTask.sensors[ACC].rate + mTask.sensors[GYR].rate);

    // Convert to hardware unit
    watermark /= 4;

    INFO_PRINT("acc = %d(%u), gyro = %d(%u), numOfData = %u, wm = %u\n",
        mTask.sensors[ACC].configed, mTask.sensors[ACC].rate,
        mTask.sensors[GYR].configed, mTask.sensors[GYR].rate,
        numOfData, watermark);

    return watermark;
}

static void spiQueueWrite(uint8_t addr, uint8_t data, uint32_t delay) {
    TDECL();
    /*if (T(spiInUse)) {
        ERROR_PRINT("SPI in use, cannot queue write\n");
        return;
    }*/
    T(packets[T(mRegCnt)]).size = 2;
    T(packets[T(mRegCnt)]).txBuf = &T(txrxBuffer[T(mWbufCnt)]);
    T(packets[T(mRegCnt)]).rxBuf = &T(txrxBuffer[T(mWbufCnt)]);
    T(packets[T(mRegCnt)]).delay = delay * 1000;
    T(txrxBuffer[T(mWbufCnt++)]) = BMI160_SPI_WRITE | addr;
    T(txrxBuffer[T(mWbufCnt++)]) = data;
    T(mWbufCnt) = (T(mWbufCnt) + 3) & 0xFFFC;
    T(mRegCnt)++;
}

/*
 * need to be sure size of buf is larger than read size
 */
static void spiQueueRead(uint8_t addr, size_t size, uint8_t **buf, uint32_t delay) {
    TDECL();
    /*if (T(spiInUse)) {
        ERROR_PRINT("SPI in use, cannot queue read %d %d\n", (int)addr, (int)size);
        return;
    }*/

    *buf = &T(txrxBuffer[T(mWbufCnt)]);
    T(packets[T(mRegCnt)]).size = size + 1;  // first byte will not contain valid data
    T(packets[T(mRegCnt)]).txBuf = &T(txrxBuffer[T(mWbufCnt)]);
    T(packets[T(mRegCnt)]).rxBuf = *buf;
    T(packets[T(mRegCnt)]).delay = delay * 1000;
    T(txrxBuffer[T(mWbufCnt)++]) = BMI160_SPI_READ | addr;
    T(mWbufCnt) = (T(mWbufCnt) + size + 3) & 0xFFFC;
    T(mRegCnt)++;
}

static void spiBatchTxRx(struct SpiMode *mode,
        SpiCbkF callback, void *cookie, const char * src) {
    TDECL();
    if (T(mWbufCnt) > SPI_BUF_SIZE) {
        ERROR_PRINT("NO enough SPI buffer space, dropping transaction.\n");
        return;
    }
    if (T(mRegCnt) > SPI_PACKET_SIZE) {
        ERROR_PRINT("spiBatchTxRx too many packets!\n");
        return;
    }

    // T(spiInUse) = true;
    spiMasterRxTx(T(spiDev), T(cs), T(packets), T(mRegCnt), mode, callback, cookie);
    T(mRegCnt) = 0;
    T(mWbufCnt) = 0;
}

static void bmi160Isr1(int arg) {
    if (mTask.latch_time_id < 0) {
        mTask.hwSampleTime = rtcGetTime();
        //ERROR_PRINT("bmi160Isr1, real=%lld\n", mTask.hwSampleTime);
    } else {
        mTask.hwSampleTime = get_latch_time_timestamp(mTask.latch_time_id);
        //ERROR_PRINT("bmi160Isr1, fake=%lld, real=%lld\n",
            //rtcGetTime(), mTask.hwSampleTime);
    }
    mark_timestamp(SENS_TYPE_ACCEL, SENS_TYPE_ACCEL, INT_ISR, mTask.hwSampleTime);
    mark_timestamp(SENS_TYPE_GYRO, SENS_TYPE_GYRO, INT_ISR, mTask.hwSampleTime);

    accGyroHwIntCheckStatus();
}

static inline bool anyFifoEnabled(void) {
    return (mTask.fifo_enabled[ACC] || mTask.fifo_enabled[GYR]);
}

static void configFifo(void) {
    TDECL();

    uint8_t val = 0x12;
    SPI_WRITE(BMI160_REG_FIFO_CONFIG_1, val);

    // we always need to cleanup the fifo and invalidate time
    SPI_WRITE(BMI160_REG_CMD, 0xb0);
    // if ACC is configed, enable ACC bit in fifo_config reg.
    if (mTask.sensors[ACC].configed) {  // && mTask.sensors[ACC].latency != SENSOR_LATENCY_NODATA) {
        val |= 0x40;
        mTask.fifo_enabled[ACC] = true;
    } else {
        val = val & 0xBF;//set the fifo_acc_en bit 0
        mTask.fifo_enabled[ACC] = false;
    }

    // if GYR is configed, enable GYR bit in fifo_config reg.
    if (mTask.sensors[GYR].configed) {  // && mTask.sensors[GYR].latency != SENSOR_LATENCY_NODATA) {
        val |= 0x80;
        mTask.fifo_enabled[GYR] = true;
    } else {
        val = val & 0x7F;//set the fifo_gyro_en bit 0
        mTask.fifo_enabled[GYR] = false;
    }

    SPI_WRITE(BMI160_REG_FIFO_CONFIG_0, mTask.watermark);

    // config the fifo register
    SPI_WRITE(BMI160_REG_FIFO_CONFIG_1, val);
}

static int accPowerOn(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    // set ACC power mode to NORMAL
    SPI_WRITE(BMI160_REG_CMD, 0x11, 50000);
    mTask.sensors[ACC].powered = 1;
    spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);

    return 0;
}

static int accPowerOff(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    // set ACC power mode to SUSPEND
    int ret = 0;
    struct accGyroCntlPacket cntlPacket;

    ret = rxControlInfo(&cntlPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "gyrPowerOff, rx inSize and elemSize error\n");
        return -1;
    }

    mTask.sensors[ACC].configed = false;
    mTask.sensors[ACC].rate = 0;
    registerAccGyroFifoInfo((mTask.sensors[ACC].rate == 0) ? 0 : 1024000000000 / mTask.sensors[ACC].rate,
        (mTask.sensors[GYR].rate == 0) ? 0 : 1024000000000 / mTask.sensors[GYR].rate);
    mTask.watermark = calcWatermark(cntlPacket.waterMark);
    configFifo();
#ifdef CFG_HW_STEP_COUNTER_SUPPORT
    if (!mTask.sensors[ANYMO].powered && !mTask.sensors[NOMO].powered
        && !mTask.sensors[STEPCNT].powered) {
#else
    if (!mTask.sensors[ANYMO].powered && !mTask.sensors[NOMO].powered) {
#endif
        SPI_WRITE(BMI160_REG_CMD, 0x10, 5000); /* set to suspend mode */
        osLog(LOG_ERROR, "accPowerOff, enter acc suspend mode\n");
    } else {
        /* enable undersampling, odr = 50Hz */
        SPI_WRITE(BMI160_REG_ACC_CONF, 0xA0 | 0x7, 450);
        SPI_WRITE(BMI160_REG_CMD, 0x12, 5000); /* set to low power */
        osLog(LOG_ERROR, "accPowerOff, enter acc low power mode\n");
    }
    mTask.sensors[ACC].powered = 0;
    spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);

    return 0;
}

static int accGyroUnmaskEint(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    /* unmask eint to prevent eint been masked due to in time acc/gyro disable but some motion still need */
    mt_eint_unmask(mTask.hw->eint_num);

    sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int gyrPowerOn(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    // set GYR power mode to NORMAL
    SPI_WRITE(BMI160_REG_CMD, 0x15, 80000); // gyro setup time 80ms

    if (anyFifoEnabled() && (1 != mTask.sensors[GYR].powered)) {
#if TIMESTAMP_DBG
        DEBUG_PRINT("minimize_sensortime_history()\n");
#endif
    }

    mTask.sensors[GYR].powered = 1;
    spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);

    return 0;
}

static int gyrPowerOff(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    // set GYR power mode to SUSPEND
    int ret = 0;
    struct accGyroCntlPacket cntlPacket;

    ret = rxControlInfo(&cntlPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "gyrPowerOff, rx inSize and elemSize error\n");
        return -1;
    }

    mTask.sensors[GYR].configed = false;
    mTask.sensors[GYR].rate = 0;
    registerAccGyroFifoInfo((mTask.sensors[ACC].rate == 0) ? 0 : 1024000000000 / mTask.sensors[ACC].rate,
        (mTask.sensors[GYR].rate == 0) ? 0 : 1024000000000 / mTask.sensors[GYR].rate);
    mTask.watermark = calcWatermark(cntlPacket.waterMark);
    configFifo();
    SPI_WRITE(BMI160_REG_CMD, 0x14, 5000);

    if (anyFifoEnabled() && 0 != mTask.sensors[GYR].powered) {
#if TIMESTAMP_DBG
        DEBUG_PRINT("minimize_sensortime_history()\n");
#endif
    }

    mTask.sensors[GYR].powered = 0;
    spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);

    return 0;
}

static int hwIntHandling(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{

    union EmbeddedDataPoint trigger_axies;
    uint8_t int_status_0 = mTask.statusBuffer[1];
    uint8_t int_status_1 = mTask.statusBuffer[2];
    /* osLog(LOG_INFO, "hwIntHandling\n"); */

    if ((int_status_0 & INT_ANY_MOTION) && mTask.sensors[ANYMO].powered) {
        // bit [0:2] of INT_STATUS[2] is set when anymo is triggered by x, y or
        // z axies respectively. bit [3] indicates the slope.
        trigger_axies.idata = (mTask.statusBuffer[3] & 0x0f);
        INFO_PRINT("Detected any motion\n");
        osEnqueueEvt(EVT_SENSOR_ANY_MOTION, trigger_axies.vptr, NULL);
    }
    if ((int_status_1 & INT_NO_MOTION) && mTask.sensors[NOMO].powered) {
        INFO_PRINT("Detected no motion\n");
        osEnqueueEvt(EVT_SENSOR_NO_MOTION, NULL, NULL);
    }
    if (int_status_1 & INT_FIFO_WM) {
        /* INFO_PRINT("Detected fifo watermark\n"); */
        accGyroInterruptOccur();
    } else {
        /* If not enter accGyroInterruptOccur by INT_FIFO_WM, need unmask eint here */
        mt_eint_unmask(mTask.hw->eint_num);
    }

    sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int hwIntStatusCheck(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                            void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{

    SPI_READ(BMI160_REG_INT_STATUS_0, 4, &mTask.statusBuffer);
    spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
    return 0;
}

static int anyMotionPowerOn(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                            void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    INFO_PRINT("Enter anyMotionPowerOn\n");

    if (!mTask.sensors[ACC].powered && !mTask.sensors[NOMO].powered) {
        /* enable undersampling, odr = 50Hz */
        SPI_WRITE(BMI160_REG_ACC_CONF, 0xA0 | 0x7, 450);
        /* set accel to low power mode if accel is not enabled */
        SPI_WRITE(BMI160_REG_CMD, 0x12, 50000);
        osLog(LOG_ERROR, "anyMotionPowerOn, enter acc low power mode\n");
    }

    /* enable anymotion interrupt */
    mTask.interrupt_enable_0 |= 0x07;
    mTask.sensors[ANYMO].powered = true;
    SPI_WRITE(BMI160_REG_INT_EN_0, mTask.interrupt_enable_0, 450);
    spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
    return 0;
}

static int anyMotionPowerOff(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    INFO_PRINT("Enter anyMotionPowerOff\n");

    mTask.interrupt_enable_0 &= ~0x07;
    mTask.sensors[ANYMO].configed = false;
    mTask.sensors[ANYMO].powered = false;
    SPI_WRITE(BMI160_REG_INT_EN_0, mTask.interrupt_enable_0, 450);
    /* for accel disable */
#ifdef CFG_HW_STEP_COUNTER_SUPPORT
    if (!mTask.sensors[ACC].powered && !mTask.sensors[NOMO].powered
        && !mTask.sensors[STEPCNT].powered) {
#else
    if (!mTask.sensors[ACC].powered && !mTask.sensors[NOMO].powered) {
#endif
        SPI_WRITE(BMI160_REG_CMD, 0x10, 5000);
        osLog(LOG_ERROR, "anyMotionPowerOff, enter acc suspend mode\n");
    }
    spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
    return 0;
}

#ifdef CFG_HW_STEP_COUNTER_SUPPORT
static int stepCountPowerOn(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                            void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    INFO_PRINT("Enter stepCountPowerOn\n");

    if (!mTask.sensors[ACC].powered && !mTask.sensors[STEPCNT].powered) {
        /* set accel to low power mode if accel is not enabled */
        SPI_WRITE(BMI160_REG_CMD, 0x12, 50000);
        osLog(LOG_ERROR, "stepCountPowerOn, enter acc low power mode\n");
    }

    /* enable stepcount interrupt */
    // mTask.interrupt_enable_1 |= 0x08;
    mTask.sensors[STEPCNT].powered = true;
    // SPI_WRITE(BMI160_REG_INT_EN_2, mTask.interrupt_enable_1, 450);
    /* set step_cnt_en bit */
    SPI_WRITE(BMI160_REG_STEP_CONF_1, 0x08 | 0x03, 1000);
    spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
    return 0;
}

static int stepCountPowerOff(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    INFO_PRINT("Enter stepCountPowerOff\n");

   // mTask.interrupt_enable_1 &= ~0x08;
    mTask.sensors[STEPCNT].configed = false;
    mTask.sensors[STEPCNT].powered = false;

    // SPI_WRITE(BMI160_REG_INT_EN_2, mTask.interrupt_enable_1, 450);
    /* unset step_cnt_en bit */
    SPI_WRITE(BMI160_REG_STEP_CONF_1, 0x03);
    /* clear last_step_cnt when poweroff */
    mTask.last_step_cnt = 0;
    /* for accel disable */
    if (!mTask.sensors[ACC].powered && !mTask.sensors[NOMO].powered
        && !mTask.sensors[ANYMO].powered) {
        SPI_WRITE(BMI160_REG_CMD, 0x10, 5000);
        osLog(LOG_ERROR, "stepCountPowerOff, enter acc suspend mode\n");
    }
    spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
    return 0;
}
#endif

static int noMotionPowerOn(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                           void *inBuf, uint8_t inSize, uint8_t elemInSize,
                           void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    INFO_PRINT("Enter noMotionPowerOn\n");

    if (!mTask.sensors[ACC].powered && !mTask.sensors[NOMO].powered) {
        /* enable undersampling, odr = 50Hz */
        SPI_WRITE(BMI160_REG_ACC_CONF, 0xA0 | 0x7, 450);
        /* set accel to low power mode if accel is not enabled */
        SPI_WRITE(BMI160_REG_CMD, 0x12, 50000);
        osLog(LOG_ERROR, "noMotionPowerOn, enter acc low power mode\n");
    }

    /* enable nomotion interrupt */
    mTask.interrupt_enable_2 |= 0x07;
    mTask.sensors[NOMO].powered = true;
    SPI_WRITE(BMI160_REG_INT_EN_2, mTask.interrupt_enable_2, 450);
    spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
    return 0;
}

static int noMotionPowerOff(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                            void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    INFO_PRINT("Enter noMotionPowerOff\n");

    mTask.interrupt_enable_2 &= ~0x07;
    mTask.sensors[NOMO].configed = false;
    mTask.sensors[NOMO].powered = false;
    SPI_WRITE(BMI160_REG_INT_EN_2, mTask.interrupt_enable_2, 450);
    /* for accel disable */
#ifdef CFG_HW_STEP_COUNTER_SUPPORT
    if (!mTask.sensors[ACC].powered && !mTask.sensors[ANYMO].powered
        && !mTask.sensors[STEPCNT].powered) {
#else
    if (!mTask.sensors[ACC].powered && !mTask.sensors[ANYMO].powered) {
#endif
        SPI_WRITE(BMI160_REG_CMD, 0x10, 5000);
        osLog(LOG_ERROR, "noMotionPowerOff, enter acc suspend mode\n");
    }
    spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
    return 0;
}

#ifdef CFG_HW_STEP_COUNTER_SUPPORT
static int StepCntSendData(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                            void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    union EmbeddedDataPoint step_cnt;
    uint32_t cur_step_cnt;
    cur_step_cnt = (int)(mTask.dataBuffer[1] | (mTask.dataBuffer[2] << 8));
    DEBUG_PRINT_IF(STEPCNT_DBG, "StepCntSendData buf1= 0x%x,buf2=0x%x\n", mTask.dataBuffer[1], mTask.dataBuffer[2]);

    if (cur_step_cnt != mTask.last_step_cnt) {
        /* Check for possible overflow */
        if (cur_step_cnt < mTask.last_step_cnt) {
            mTask.total_step_cnt += cur_step_cnt + (0xFFFF - mTask.last_step_cnt);
        } else {
            mTask.total_step_cnt += (cur_step_cnt - mTask.last_step_cnt);
        }
        mTask.last_step_cnt = cur_step_cnt;
        step_cnt.idata = mTask.total_step_cnt;
        DEBUG_PRINT_IF(STEPCNT_DBG, "step count =%d\n", step_cnt.idata);
        osEnqueueEvt(EVT_SENSOR_STEP_COUNTER, step_cnt.vptr, NULL);
    }

   sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
   return 0;
}

static int stepCntGetData(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                            void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    DEBUG_PRINT_IF(STEPCNT_DBG, "stepCntGetData\n");
    SPI_READ(BMI160_REG_STEP_CNT_0, 2, &mTask.dataBuffer);
    spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
    return 0;
}
#endif

static void updateTimeDelta(uint8_t idx, uint8_t odr) {
}

// compute the register value from sensor rate.
static uint8_t computeOdr(uint32_t *rate) {
    uint8_t odr = 0x00;

    if (*rate > SENSOR_HZ(1600))
        *rate = SENSOR_HZ(3200);
    else if (*rate > SENSOR_HZ(800))
        *rate = SENSOR_HZ(1600);
    else if (*rate > SENSOR_HZ(400))
        *rate = SENSOR_HZ(800);
    else if (*rate > SENSOR_HZ(200))
        *rate = SENSOR_HZ(400);
    else if (*rate > SENSOR_HZ(100))
        *rate = SENSOR_HZ(200);
    else if (*rate > SENSOR_HZ(50))
        *rate = SENSOR_HZ(100);
    else if (*rate > SENSOR_HZ(25))
        *rate = SENSOR_HZ(50);
    else if (*rate > SENSOR_HZ(25.0f/2.0f))
        *rate = SENSOR_HZ(25);
    else if (*rate > SENSOR_HZ(25.0f/4.0f))
        *rate = SENSOR_HZ(25.0f/2.0f);
    else if (*rate > SENSOR_HZ(25.0f/8.0f))
        *rate = SENSOR_HZ(25.0f/4.0f);
    else if (*rate > SENSOR_HZ(25.0f/16.0f))
        *rate = SENSOR_HZ(25.0f/8.0f);
    else if (*rate > SENSOR_HZ(25.0f/32.0f))
        *rate = SENSOR_HZ(25.0f/16.0f);
    else
        *rate = SENSOR_HZ(25.0f/32.0f);

    switch (*rate) {
    // fall through intended to get the correct register value
    case SENSOR_HZ(3200): odr++;
    case SENSOR_HZ(1600): odr++;
    case SENSOR_HZ(800): odr++;
    case SENSOR_HZ(400): odr++;
    case SENSOR_HZ(200): odr++;
    case SENSOR_HZ(100): odr++;
    case SENSOR_HZ(50): odr++;
    case SENSOR_HZ(25): odr++;
    case SENSOR_HZ(25.0f/2.0f): odr++;
    case SENSOR_HZ(25.0f/4.0f): odr++;
    case SENSOR_HZ(25.0f/8.0f): odr++;
    case SENSOR_HZ(25.0f/16.0f): odr++;
    case SENSOR_HZ(25.0f/32.0f): odr++;
    default:
        return odr;
    }
}

static void configMotion(uint8_t odr) {
    // motion threshold is element * 15.63mg (for 8g range)
    static const uint8_t motion_thresholds[ACC_MAX_RATE+1] =
        {5, 5, 5, 5, 5, 5, 5, 5, 4, 3, 2, 2, 2};

    // set any_motion duration to 1 point
    // set no_motion duration to (3+1)*1.28sec=5.12sec
    // set no_motion duration to (11+1)*1.28sec=15.36sec by MTK CTO
    SPI_WRITE(BMI160_REG_INT_MOTION_0, 0x0b << 2, 450);

    // set any_motion threshold
    SPI_WRITE(BMI160_REG_INT_MOTION_1, motion_thresholds[odr], 450);

    // set no_motion threshold
    SPI_WRITE(BMI160_REG_INT_MOTION_2, motion_thresholds[odr], 450);
}

// static bool accSetRate(uint32_t rate, uint64_t latency, void *cookie)
static int accSetRate(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    TDECL();
    int odr, osr = 0;
    int ret = 0;
    struct accGyroCntlPacket cntlPacket;

    ret = rxControlInfo(&cntlPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "accSetRate, rx inSize and elemSize error\n");
        return -1;
    }

    mTask.sensors[ACC].rate = cntlPacket.rate;
    mTask.sensors[ACC].latency = SENSOR_LATENCY_NODATA;  // latency;
    // odr = computeOdr(cntlPacket.rate);
    odr = computeOdr(&mTask.sensors[ACC].rate);
    if (!odr) {
        ERROR_PRINT("invalid acc rate\n");
        return false;
    }
    registerAccGyroFifoInfo((mTask.sensors[ACC].rate == 0) ? 0 : 1024000000000 / mTask.sensors[ACC].rate,
        (mTask.sensors[GYR].rate == 0) ? 0 : 1024000000000 / mTask.sensors[GYR].rate);

    updateTimeDelta(ACC, odr);

    // minimum supported rate for ACCEL is 12.5Hz.
    // Anything lower than that shall be acheived by downsampling.
    if (odr < ACC_MIN_RATE) {
        osr = ACC_MIN_RATE - odr;
        odr = ACC_MIN_RATE;
    }

    // for high odrs, oversample to reduce hw latency and downsample
    // to get desired odr
    if (odr > OSR_THRESHOLD) {
        osr = (ACC_MAX_OSR + odr) > ACC_MAX_RATE ? (ACC_MAX_RATE - odr) : ACC_MAX_OSR;
        odr += osr;
    }

    mTask.sensors[ACC].configed = true;
    mTask.acc_downsample = osr;

    // configure ANY_MOTION and NO_MOTION based on odr
    configMotion(odr);

    // set ACC bandwidth parameter to 2 (bits[4:6])
    // set the rate (bits[0:3])
    SPI_WRITE(BMI160_REG_ACC_CONF, 0x20 | odr);

    // configure down sampling ratio, 0x88 is to specify we are using
    // filtered samples
    SPI_WRITE(BMI160_REG_FIFO_DOWNS, (mTask.acc_downsample << 4) | mTask.gyr_downsample | 0x88);

    mTask.watermark = calcWatermark(cntlPacket.waterMark);

    // flush the data and configure the fifo
    configFifo();
    mt_eint_unmask(mTask.hw->eint_num);

    spiBatchTxRx(&mTask.mode, spiCallBack/*sensorSpiCallback*/, next_state/*&mTask.sensors[ACC]*/, __FUNCTION__);
    return true;
}


static int accCali(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    TDECL();
    //osLog(LOG_ERROR, "accCali\n");
    uint8_t mode = 0;

    if ((void *)next_state == (void *)STATE_CALIBRATION_FOC)
        mTask.calibration_state = CALIBRATION_START;

    switch (mTask.calibration_state) {
    case CALIBRATION_START:

        mTask.sensors[ACC].startCali = true;
        // turn ACC to NORMAL mode
        SPI_WRITE(BMI160_REG_CMD, 0x11, 50000);
        mTask.calibration_state = CALIBRATION_FOC;

        spiBatchTxRx(&mTask.mode, spiCallBack, next_state, "accCali CALI_START");
        break;
    case CALIBRATION_FOC:

        // set accel range to +-8g
        //SPI_WRITE(BMI160_REG_ACC_RANGE, 0x08, 450);
        // enable accel fast offset compensation,
        // x: 0g, y: 0g, z: -1g ,depend on sensor position
        if (T(cvt).sign[AXIS_Z] > 0) {
            SPI_WRITE(BMI160_REG_FOC_CONF, 0x3c | (1 << 0));
        } else {
            SPI_WRITE(BMI160_REG_FOC_CONF, 0x3c | (1 << 1));
        }
        // start calibration, delay time recommend >250000(250ms)
        SPI_WRITE(BMI160_REG_CMD, 0x03, 500000);

        // poll the status reg until the calibration finishes.
        SPI_READ(BMI160_REG_STATUS, 1, &mTask.statusBuffer, 50000);

        mTask.calibration_state = CALIBRATION_STATUS_CHECK;
        spiBatchTxRx(&mTask.mode, spiCallBack, next_state, "accCali CALI_FOC");
        break;
    case CALIBRATION_STATUS_CHECK:
        // if the STATUS REG has bit 3 set, it means calbration is done.
        // otherwise, FOC failed
        if (mTask.statusBuffer[1] & 0x08) {
            //disable FOC
            SPI_WRITE(BMI160_REG_FOC_CONF, 0x00);

            //read the offset value for accel
            SPI_READ(BMI160_REG_OFFSET_0, 3, &mTask.dataBuffer);

            mTask.calibration_state = CALIBRATION_SET_OFFSET;

            spiBatchTxRx(&mTask.mode, spiCallBack, next_state, "accCali CALI_CHECK");
            INFO_PRINT("FOC set FINISHED!\n");
        } else {
            // if calbration hasn't finished  on the STATUS reg,
            // declare timeout.
            //disable FOC
            //SPI_WRITE(BMI160_REG_FOC_CONF, 0x00);

            mTask.calibration_state = CALIBRATION_TIMEOUT;
            sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);

            ERROR_PRINT("FOC set FAILED!\n");
        }

        break;
    case CALIBRATION_SET_OFFSET:
        mTask.sensors[ACC].offset[0] = mTask.dataBuffer[1];
        mTask.sensors[ACC].offset[1] = mTask.dataBuffer[2];
        mTask.sensors[ACC].offset[2] = mTask.dataBuffer[3];
        /*
             ERROR_PRINT("ACCELERATION OFFSET is %u  %u  %u\n",
                (unsigned int)mTask.sensors[ACC].offset[0],
                (unsigned int)mTask.sensors[ACC].offset[1],
                (unsigned int)mTask.sensors[ACC].offset[2]);

             ERROR_PRINT("ACCELERATION dataBuffer is %d  %d  %d\n",
                mTask.dataBuffer[1],
                mTask.dataBuffer[2],
                mTask.dataBuffer[3]);    */
        // sign extend values
        if (mTask.sensors[ACC].offset[0] & 0x80)
            mTask.sensors[ACC].offset[0] |= 0xFFFFFF00;
        if (mTask.sensors[ACC].offset[1] & 0x80)
            mTask.sensors[ACC].offset[1] |= 0xFFFFFF00;
        if (mTask.sensors[ACC].offset[2] & 0x80)
            mTask.sensors[ACC].offset[2] |= 0xFFFFFF00;

        mTask.sensors[ACC].offset_enable = true;
        INFO_PRINT("ACCELERATION OFFSET is %u  %u  %u\n",
                mTask.sensors[ACC].offset[0],
                mTask.sensors[ACC].offset[1],
                mTask.sensors[ACC].offset[2]);

        accGyroSendCalibrationResult(SENS_TYPE_ACCEL, (int32_t *)&mTask.sensors[ACC].offset[0], 0);

        // Enable offset compensation for accel
        mode = offset6Mode();
        SPI_WRITE(BMI160_REG_OFFSET_6, mode);

        if (mTask.sensors[ACC].powered) {
            // turn ACC to NORMAL mode
            SPI_WRITE(BMI160_REG_CMD, 0x11, 5000);
        }
#ifdef CFG_HW_STEP_COUNTER_SUPPORT
        else if (!mTask.sensors[ANYMO].powered && !mTask.sensors[NOMO].powered
            && !mTask.sensors[STEPCNT].powered) {
#else
        else if (!mTask.sensors[ANYMO].powered && !mTask.sensors[NOMO].powered) {
#endif
            SPI_WRITE(BMI160_REG_CMD, 0x10, 5000); /* set to suspend mode */
            osLog(LOG_ERROR, "acc cali done, enter acc suspend mode\n");
        } else {
            /* enable undersampling, odr = 50Hz */
            SPI_WRITE(BMI160_REG_ACC_CONF, 0xA0 | 0x7, 450);
            SPI_WRITE(BMI160_REG_CMD, 0x12, 5000); /* set to low power */
            osLog(LOG_ERROR, "acc cali done, enter acc low power mode\n");
        }
        mTask.sensors[ACC].startCali = false;

        spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
        //sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
        break;
    case CALIBRATION_TIMEOUT:
        //TIME OUT
        if (mTask.sensors[ACC].powered) {
            // turn ACC to NORMAL mode
            SPI_WRITE(BMI160_REG_CMD, 0x11, 5000);
        }
#ifdef CFG_HW_STEP_COUNTER_SUPPORT
        else if (!mTask.sensors[ANYMO].powered && !mTask.sensors[NOMO].powered
            && !mTask.sensors[STEPCNT].powered) {
#else
        else if (!mTask.sensors[ANYMO].powered && !mTask.sensors[NOMO].powered) {
#endif
            SPI_WRITE(BMI160_REG_CMD, 0x10, 5000); /* set to suspend mode */
            osLog(LOG_ERROR, "acc cali done, enter acc suspend mode\n");
        } else {
            /* enable undersampling, odr = 50Hz */
            SPI_WRITE(BMI160_REG_ACC_CONF, 0xA0 | 0x7, 450);
            SPI_WRITE(BMI160_REG_CMD, 0x12, 5000); /* set to low power */
            osLog(LOG_ERROR, "acc cali done, enter acc low power mode\n");
        }
        mTask.sensors[ACC].startCali = false;

        spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);

        ERROR_PRINT("Calibration TIMED OUT\n");
        break;
    default:
        ERROR_PRINT("Invalid calibration state\n");
        break;
    }
    return true;
}

static int accHwCaliCheck(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    TDECL();
    int ret = 0;
    struct accGyroCaliCfgPacket caliCfgPacket;

    //osLog(LOG_ERROR, "accHwCaliCheck\n");

    ret = rxCaliCfgInfo(&caliCfgPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);

    if (ret < 0) {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "accHwCaliCheck, rx inSize and elemSize error\n");
        return -1;
    }

    mTask.sensors[ACC].offset[0] = caliCfgPacket.caliCfgData[0];
    mTask.sensors[ACC].offset[1] = caliCfgPacket.caliCfgData[1];
    mTask.sensors[ACC].offset[2] = caliCfgPacket.caliCfgData[2];
    /*
   osLog(LOG_ERROR, "accHwCaliCheck offset [0]:%d, [1]:%d, [2]:%d\n",
                   mTask.sensors[ACC].offset[0],
                   mTask.sensors[ACC].offset[1],
                   mTask.sensors[ACC].offset[2]);
    */
    mTask.sensors[ACC].offset_enable = true;

    //read gyro_off_en and gyro_off_xyz<9:8>
    SPI_READ(BMI160_REG_OFFSET_6, 1, &mTask.dataBuffer);

    spiBatchTxRx(&mTask.mode, spiCallBack/*sensorSpiCallback*/, next_state, __FUNCTION__);
    return true;
}

static int accCaliSave(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    TDECL();

    uint8_t mode = 0;

    //clear acc_off_en bit and reset
    mode = mTask.dataBuffer[1] & 0xbf;
    if (mTask.sensors[ACC].offset_enable)
        mode |= 0x01 << 6;

    SPI_WRITE(BMI160_REG_OFFSET_0, mTask.sensors[ACC].offset[0] & 0xFF, 450);
    SPI_WRITE(BMI160_REG_OFFSET_0 + 1, mTask.sensors[ACC].offset[1] & 0xFF, 450);
    SPI_WRITE(BMI160_REG_OFFSET_0 + 2, mTask.sensors[ACC].offset[2] & 0xFF, 450);

    SPI_WRITE(BMI160_REG_OFFSET_6, mode, 450);
    SPI_READ(BMI160_REG_OFFSET_0, 7, &mTask.dataBuffer);

    spiBatchTxRx(&mTask.mode, spiCallBack/*sensorSpiCallback*/, next_state, __FUNCTION__);
    return true;
}

// static bool gyrSetRate(uint32_t rate, uint64_t latency, void *cookie)
static int gyrSetRate(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    TDECL();
    int odr, osr = 0;
    int ret = 0;
    struct accGyroCntlPacket cntlPacket;

    ret = rxControlInfo(&cntlPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "gyrSetRate, rx inSize and elemSize error\n");
        return -1;
    }

    mTask.sensors[GYR].rate = cntlPacket.rate;
    mTask.sensors[GYR].latency = SENSOR_LATENCY_NODATA;  // latency;
    // odr = computeOdr(cntlPacket.rate);
    odr = computeOdr(&mTask.sensors[GYR].rate);
    if (!odr) {
        ERROR_PRINT("invalid gyr rate\n");
        return false;
    }
    registerAccGyroFifoInfo((mTask.sensors[ACC].rate == 0) ? 0 : 1024000000000 / mTask.sensors[ACC].rate,
        (mTask.sensors[GYR].rate == 0) ? 0 : 1024000000000 / mTask.sensors[GYR].rate);

    updateTimeDelta(GYR, odr);

    // minimum supported rate for GYRO is 25.0Hz.
    // Anything lower than that shall be acheived by downsampling.
    if (odr < GYR_MIN_RATE) {
        osr = GYR_MIN_RATE - odr;
        odr = GYR_MIN_RATE;
    }

    mTask.sensors[GYR].configed = true;
    mTask.gyr_downsample = osr;

    // set GYR bandwidth parameter to 2 (bits[4:6])
    // set the rate (bits[0:3])
    SPI_WRITE(BMI160_REG_GYR_CONF, 0x20 | odr);

    // configure down sampling ratio, 0x88 is to specify we are using
    // filtered samples
    SPI_WRITE(BMI160_REG_FIFO_DOWNS, (mTask.acc_downsample << 4) | mTask.gyr_downsample | 0x88);

    mTask.watermark = calcWatermark(cntlPacket.waterMark);

    // flush the data and configure the fifo
    configFifo();
    mt_eint_unmask(mTask.hw->eint_num);

    spiBatchTxRx(&mTask.mode, spiCallBack/*sensorSpiCallback*/, next_state/*&mTask.sensors[GYR]*/, __FUNCTION__);
    return true;
}
static int gyroCali(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    TDECL();
    //osLog(LOG_ERROR, "gyroCali\n");
    //uint32_t tempData[4] = {0};
    uint8_t mode = 0;

    if ((void *)next_state == (void *)STATE_CALIBRATION_FOC)
        mTask.calibration_state = CALIBRATION_START;

    switch (mTask.calibration_state) {
    case CALIBRATION_START:

        mTask.sensors[GYR].startCali = true;
        // turn gyro to NORMAL mode
        SPI_WRITE(BMI160_REG_CMD, 0x15, 50000);

        mTask.calibration_state = CALIBRATION_FOC;

        spiBatchTxRx(&mTask.mode, spiCallBack, next_state, "gyroCali CALI_START");
        break;
    case CALIBRATION_FOC:

        // set gyro range to +-2000 deg/sec
        //SPI_WRITE(BMI160_REG_GYR_RANGE, 0x00);
        // enable gyro fast offset compensation
        SPI_WRITE(BMI160_REG_FOC_CONF, 0x40);

        // start calibration, delay time recommend >250000(250ms)
        SPI_WRITE(BMI160_REG_CMD, 0x03, 500000);

        // poll the status reg until the calibration finishes.
        SPI_READ(BMI160_REG_STATUS, 1, &mTask.statusBuffer, 50000);

        mTask.calibration_state = CALIBRATION_STATUS_CHECK;
        spiBatchTxRx(&mTask.mode, spiCallBack, next_state, "gyroCali CALI_FOC");
        break;
    case CALIBRATION_STATUS_CHECK:
        // if the STATUS REG has bit 3 set, it means calbration is done.
        // otherwise, FOC failed
        if (mTask.statusBuffer[1] & 0x08) {
            //disable FOC
            SPI_WRITE(BMI160_REG_FOC_CONF, 0x00);

            //read the offset value for gyro
            SPI_READ(BMI160_REG_OFFSET_3, 4, &mTask.dataBuffer);

            mTask.calibration_state = CALIBRATION_SET_OFFSET;

            spiBatchTxRx(&mTask.mode, spiCallBack, next_state, "gyroCali CALI_CHECK");
            INFO_PRINT("FOC set FINISHED!\n");
        } else {
            // if calbration hasn't finished  on the STATUS reg,
            // declare timeout.
            //disable FOC
            //SPI_WRITE(BMI160_REG_FOC_CONF, 0x00);
            mTask.calibration_state = CALIBRATION_TIMEOUT;
            sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
            ERROR_PRINT("FOC set FAILED!\n");
        }

        break;
    case CALIBRATION_SET_OFFSET:
        //tempData[0] = mTask.dataBuffer[1] & 0x000000FF;
        //tempData[1] = mTask.dataBuffer[2] & 0x000000FF;
        //tempData[2] = mTask.dataBuffer[3] & 0x000000FF;
        //tempData[3] = mTask.dataBuffer[4] & 0x000000FF;

        mTask.sensors[GYR].offset[0] = ((mTask.dataBuffer[4] & 0x03) << 8) | mTask.dataBuffer[1];
        mTask.sensors[GYR].offset[1] = ((mTask.dataBuffer[4] & 0x0C) << 6) | mTask.dataBuffer[2];
        mTask.sensors[GYR].offset[2] = ((mTask.dataBuffer[4] & 0x30) << 4) | mTask.dataBuffer[3];

        // sign extend values
        if (mTask.sensors[GYR].offset[0] & 0x200)
            mTask.sensors[GYR].offset[0] |= 0xFFFFFC00;
        if (mTask.sensors[GYR].offset[1] & 0x200)
            mTask.sensors[GYR].offset[1] |= 0xFFFFFC00;
        if (mTask.sensors[GYR].offset[2] & 0x200)
            mTask.sensors[GYR].offset[2] |= 0xFFFFFC00;

        mTask.sensors[GYR].offset_enable = true;
        INFO_PRINT("GYRO OFFSET is %u  %u  %u\n",
                mTask.sensors[GYR].offset[0],
                mTask.sensors[GYR].offset[1],
                mTask.sensors[GYR].offset[2]);

        accGyroSendCalibrationResult(SENS_TYPE_GYRO, (int32_t *)&mTask.sensors[GYR].offset[0], 0);

        // Enable offset compensation for gyro
        mode = offset6Mode();
        SPI_WRITE(BMI160_REG_OFFSET_6, mode);

        if (mTask.sensors[GYR].powered) {
            // turn GYRO to NORMAL mode
            SPI_WRITE(BMI160_REG_CMD, 0x15, 5000);
        } else
            SPI_WRITE(BMI160_REG_CMD, 0x14, 5000);
        mTask.sensors[GYR].startCali = false;

        spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
        break;
    case CALIBRATION_TIMEOUT:
        //TIME OUT
        if (mTask.sensors[GYR].powered) {
            // turn GYRO to NORMAL mode
            SPI_WRITE(BMI160_REG_CMD, 0x15, 5000);
        } else
            SPI_WRITE(BMI160_REG_CMD, 0x14, 5000);
        mTask.sensors[GYR].startCali = false;

        spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
        ERROR_PRINT("Calibration TIMED OUT\n");
        break;
    default:
        ERROR_PRINT("Invalid calibration state\n");
        break;
    }
    return true;
}

static int gyroHwCaliCheck(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    TDECL();
    int ret = 0;
    struct accGyroCaliCfgPacket caliCfgPacket;

    //osLog(LOG_ERROR, "gyroHwCaliCheck\n");

    ret = rxCaliCfgInfo(&caliCfgPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);

    if (ret < 0) {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "gyroHwCaliCheck, rx inSize and elemSize error\n");
        return -1;
    }

    mTask.sensors[GYR].offset[0] = caliCfgPacket.caliCfgData[0];
    mTask.sensors[GYR].offset[1] = caliCfgPacket.caliCfgData[1];
    mTask.sensors[GYR].offset[2] = caliCfgPacket.caliCfgData[2];
    /*
      osLog(LOG_ERROR, "gyroHwCaliCheck offset [0]:%d, [1]:%d, [2]:%d\n",
                   mTask.sensors[GYR].offset[0],
                   mTask.sensors[GYR].offset[1],
                   mTask.sensors[GYR].offset[2]);
      */
    mTask.sensors[GYR].offset_enable = true;

    //read acc_off_en
    SPI_READ(BMI160_REG_OFFSET_6, 1, &mTask.dataBuffer);

    spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
    return true;
}

static int gyroCaliSave(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    TDECL();

    uint8_t mode = 0;

    //get acc_off_en bit
    mode = mTask.dataBuffer[1] & 0x40;

    if (mTask.sensors[GYR].offset_enable)
        mode |= 0x01 << 7;

    mode |= (mTask.sensors[GYR].offset[2] & 0x0300) >> 4;
    mode |= (mTask.sensors[GYR].offset[1] & 0x0300) >> 6;
    mode |= (mTask.sensors[GYR].offset[0] & 0x0300) >> 8;

    SPI_WRITE(BMI160_REG_OFFSET_3, mTask.sensors[GYR].offset[0] & 0xFF, 450);
    SPI_WRITE(BMI160_REG_OFFSET_3 + 1, mTask.sensors[GYR].offset[1] & 0xFF, 450);
    SPI_WRITE(BMI160_REG_OFFSET_3 + 2, mTask.sensors[GYR].offset[2] & 0xFF, 450);

    SPI_WRITE(BMI160_REG_OFFSET_6, mode, 450);
    SPI_READ(BMI160_REG_OFFSET_0, 7, &mTask.dataBuffer);

    spiBatchTxRx(&mTask.mode, spiCallBack, next_state, __FUNCTION__);
    return true;
}

static int bmi160Sample(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    TDECL();
    int ret = 0;

    mark_timestamp(SENS_TYPE_ACCEL, SENS_TYPE_ACCEL, SAMPLE_BEGIN, rtcGetTime());
    mark_timestamp(SENS_TYPE_GYRO, SENS_TYPE_GYRO, SAMPLE_BEGIN, rtcGetTime());
    ret = rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "bmi160Sample, rx dataInfo error\n");
        return -1;
    }

    T(spiCallBack) = spiCallBack;
    T(next_state) = next_state;
    initiateFifoRead(false /*isInterruptContext*/);

    return 0;
}

static void parseRawData(struct accGyroData *data, uint8_t *buf, float kScale, uint8_t sensorType) {
    int16_t raw_data[AXES_NUM];
    int16_t remap_data[AXES_NUM];
    int32_t SwCali[AXES_NUM];
    memset(SwCali, 0, sizeof(SwCali));

    if (sensorType == SENS_TYPE_ACCEL)
        accGetCalibration(SwCali, 0);
    else if (sensorType == SENS_TYPE_GYRO)
        gyroGetCalibration(SwCali, 0);

    raw_data[AXIS_X] = (buf[0] | buf[1] << 8);
    raw_data[AXIS_Y] = (buf[2] | buf[3] << 8);
    raw_data[AXIS_Z] = (buf[4] | buf[5] << 8);
    /*
      osLog(LOG_ERROR,"ACCEL:raw_data_x=%d, raw_data_y=%d, raw_data_z=%d\n",
                    raw_data[AXIS_X], raw_data[AXIS_Y], raw_data[AXIS_Z]);
      */
    raw_data[AXIS_X] = raw_data[AXIS_X] + SwCali[AXIS_X];
    raw_data[AXIS_Y] = raw_data[AXIS_Y] + SwCali[AXIS_Y];
    raw_data[AXIS_Z] = raw_data[AXIS_Z] + SwCali[AXIS_Z];

    remap_data[mTask.cvt.map[AXIS_X]] = mTask.cvt.sign[AXIS_X] * raw_data[AXIS_X];
    remap_data[mTask.cvt.map[AXIS_Y]] = mTask.cvt.sign[AXIS_Y] * raw_data[AXIS_Y];
    remap_data[mTask.cvt.map[AXIS_Z]] = mTask.cvt.sign[AXIS_Z] * raw_data[AXIS_Z];
    /*
      osLog(LOG_ERROR,"ACCEL:remap_data=%d, remap_data=%d, remap_data=%d\n",
                    remap_data[AXIS_X], remap_data[AXIS_Y], remap_data[AXIS_Z]);
      */
    data->sensType = sensorType;
    data->x = (float)remap_data[AXIS_X] * kScale;
    data->y = (float)remap_data[AXIS_Y] * kScale;
    data->z = (float)remap_data[AXIS_Z] * kScale;

    if (mTask.debug_trace) {
        switch(sensorType) {
            case SENS_TYPE_ACCEL:
                osLog(LOG_ERROR,"ACCEL:raw_data_x=%f, raw_data_y=%f, raw_data_z=%f\n",
                    (double)data->x, (double)data->y, (double)data->z);
                break;
            case SENS_TYPE_GYRO:
                osLog(LOG_ERROR,"GYRO:raw_data_x=%f, raw_data_y=%f, raw_data_z=%f\n",
                    (double)data->x, (double)data->y, (double)data->z);
                break;
            default:
                break;
        }
    }

    if (sensorType == SENS_TYPE_ACCEL) {
        mTask.accFactoryData.ix = (int32_t)(data->x * ACCELEROMETER_INCREASE_NUM_AP);
        mTask.accFactoryData.iy = (int32_t)(data->y * ACCELEROMETER_INCREASE_NUM_AP);
        mTask.accFactoryData.iz = (int32_t)(data->z * ACCELEROMETER_INCREASE_NUM_AP);
    } else if (sensorType == SENS_TYPE_GYRO) {
        mTask.gyroFactoryData.ix =
            (int32_t)((float)remap_data[AXIS_X] * BMI160_FS_250_LSB / BMI160_FS_2000_LSB);
        mTask.gyroFactoryData.iy =
            (int32_t)((float)remap_data[AXIS_Y] * BMI160_FS_250_LSB / BMI160_FS_2000_LSB);
        mTask.gyroFactoryData.iz =
            (int32_t)((float)remap_data[AXIS_Z] * BMI160_FS_250_LSB / BMI160_FS_2000_LSB);
    }
}

static int bmi160Convert(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    size_t i = 1, j;
    size_t size = mTask.xferCnt;
    int fh_mode, fh_param;
    uint8_t *buf = mTask.dataBuffer;

    uint64_t min_delta = ULONG_LONG_MAX;
    uint32_t sensor_time24;
    /* uint64_t full_sensor_time; */
    uint64_t frame_sensor_time = mTask.frame_sensortime;
    bool observed[2] = {false, false};
    uint64_t tmp_frame_time, tmp_time[3];
    bool frame_sensor_time_valid = true;  // mTask.frame_sensortime_valid;
#if TIMESTAMP_DBG
    int frame_num = -1;
#endif
    struct accGyroData *data = mTask.accGyroPacket.outBuf;
    uint8_t accEventSize = 0;
    uint8_t gyroEventSize = 0;
    uint64_t swSampleTime = 0, realSampleTime = 0;
    int16_t temperature16 = 0;
    float temperature = 0;

    if (!mTask.frame_sensortime_valid) {
        // This is the first FIFO delivery after any sensor is enabled in
        // bmi160. Sensor time reference is not establised until end of this
        // FIFO frame. Assume time start from zero and do a dry run to estimate
        // the time and then go through this FIFO again.
        frame_sensor_time = 0ull;
    }

    while (size > 0) {
        if (buf[i] == BMI160_FRAME_HEADER_INVALID) {
            // reaching invalid header means no more data
            break;
        } else if (buf[i] == BMI160_FRAME_HEADER_SKIP) {
            // manually injected skip header
            DEBUG_PRINT_IF(DBG_CHUNKED, "skip nop header");
            i++;
            size--;
            continue;
        }

        fh_mode = buf[i] >> 6;
        fh_param = (buf[i] >> 2) & 0xf;

        i++;
        size--;
#if TIMESTAMP_DBG
        ++frame_num;
#endif
        if (fh_mode == 1) {
            // control frame.
            if (fh_param == 0) {
                // skip frame, we skip it
                if (size >= 1) {
                    i++;
                    size--;
                } else {
                    size = 0;
                }
            } else if (fh_param == 1) {
                // sensortime frame
                if (size >= 3) {
                    // The active sensor with the highest odr/lowest delta is the one that
                    // determines the sensor time increments.
                    for (j = ACC; j <= GYR; j++) {
                        if (mTask.sensors[j].configed &&
                                mTask.sensors[j].latency != SENSOR_LATENCY_NODATA) {
                            min_delta = min_delta < mTask.time_delta[j] ? min_delta :
                                    mTask.time_delta[j];
                        }
                    }
                    sensor_time24 = buf[i + 2] << 16 | buf[i + 1] << 8 | buf[i];

                    // clear lower bits that measure time from taking the sample to reading the
                    // FIFO, something we're not interested in.
                    sensor_time24 &= ~(min_delta - 1);
                    i += 3;
                    size -= 3;
                } else {
                    size = 0;
                }
            } else if (fh_param == 2) {
                // fifo_input config frame
#if TIMESTAMP_DBG
                DEBUG_PRINT("frame %d config change 0x%02x\n", frame_num, buf[i]);
#endif
                if (size >= 1) {
                    i++;
                    size--;
                } else {
                    size = 0;
                }
            } else {
                size = 0;  // drop this batch
                ERROR_PRINT("Invalid fh_param in conttrol frame\n");
            }
        } else if (fh_mode == 2) {
            // Calcutate candidate frame time (tmp_frame_time):
            // 1) When sensor is first enabled, reference from other sensors if possible.
            // Otherwise, add the smallest increment to the previous data frame time.
            // 2) The newly enabled sensor could only underestimate its
            // frame time without reference from other sensors.
            // 3) The underestimated frame time of a newly enabled sensor will be corrected
            // as soon as it shows up in the same frame with another sensor.
            // 4) (prev_frame_time == ULONG_LONG_MAX) means the sensor wasn't enabled.
            // 5) (prev_frame_time == ULONG_LONG_MAX -1) means the sensor didn't appear in the last
            // data frame of the previous fifo read.  So it won't be used as a frame time reference.

            tmp_frame_time = 0;
            for (j = ACC; j <= GYR; j++) {
                observed[j] = false;  // reset at each data frame
                tmp_time[j] = 0;
                if ((mTask.prev_frame_time[j] < ULONG_LONG_MAX - 1) && (fh_param & (1 << j))) {
                    tmp_time[j] = mTask.prev_frame_time[j] + mTask.time_delta[j];
                    tmp_frame_time = (tmp_time[j] > tmp_frame_time) ? tmp_time[j] : tmp_frame_time;
                }
            }
            tmp_frame_time = (frame_sensor_time + kMinSensorTimeIncrement > tmp_frame_time)
                ? (frame_sensor_time + kMinSensorTimeIncrement) : tmp_frame_time;

            // regular frame, dispatch data to each sensor's own fifo
            if (fh_param & 4) {  // have mag data
                ERROR_PRINT("Not support, should not enter here!\n");
            }
            if (fh_param & 2) {  // have gyro data
                if (size >= 6) {
                    if (frame_sensor_time_valid) {
                        if ((accEventSize + gyroEventSize) < MAX_RECV_PACKET) {
                            parseRawData(&data[accEventSize + gyroEventSize], &buf[i], kScale_gyr, SENS_TYPE_GYRO);
                            gyroEventSize++;
                        } else
                            ERROR_PRINT("outBuf full, accEventSize = %d, gyroEventSize = %d\n",
                                accEventSize, gyroEventSize);
                    }
                    mTask.prev_frame_time[GYR] = tmp_frame_time;
                    i += 6;
                    size -= 6;
                    observed[GYR] = true;
                } else {
                    size = 0;
                }
            }
            if (fh_param & 1) {  // have accel data
                if (size >= 6) {
                    if (frame_sensor_time_valid) {
                        if ((accEventSize + gyroEventSize) < MAX_RECV_PACKET) {
                            parseRawData(&data[accEventSize + gyroEventSize], &buf[i], kScale_acc, SENS_TYPE_ACCEL);
                            accEventSize++;
                        } else
                            ERROR_PRINT("outBuf full, accEventSize = %d, gyroEventSize = %d\n",
                                accEventSize, gyroEventSize);
                    }
                    mTask.prev_frame_time[ACC] = tmp_frame_time;
                    i += 6;
                    size -= 6;
                    observed[ACC] = true;
                } else {
                    size = 0;
                }
            }

            if (observed[ACC] || observed[GYR])
                frame_sensor_time = tmp_frame_time;
        } else {
            size = 0;  // drop this batch
            ERROR_PRINT("Invalid fh_mode\n");
        }
    } /* while (size > 0) { */
    /*if factory true , can't send data*/
    if (mTask.sensors[ACC].startCali || mTask.sensors[GYR].startCali) {
        accEventSize = 0;
        gyroEventSize = 0;
    }

    swSampleTime = mTask.swSampleTime;
    realSampleTime = calcFakeInterruptTime(swSampleTime, mTask.hwSampleTime, mTask.lastSampleTime,
        mTask.sensors[ACC].rate, mTask.sensors[ACC].configed, accEventSize,
        mTask.sensors[GYR].rate, mTask.sensors[GYR].configed, gyroEventSize);
    DEBUG_PRINT("bmi160Convert, swSampleTime=%lld, hwSampleTime=%lld, realSampleTime=%lld, lastSampleTime=%lld, now=%lld\n",
        swSampleTime, mTask.hwSampleTime, realSampleTime, mTask.lastSampleTime, rtcGetTime());
    mTask.hwSampleTime = realSampleTime;
    mTask.lastSampleTime = realSampleTime;

    temperature16 = (mTask.temperatureBuffer[1] | (mTask.temperatureBuffer[2] << 8));
    if (temperature16 == 0x8000) {
        temperature = kTempInvalid;
    } else {
        temperature = 23.0f + temperature16 * kScale_temp;
    }
    txTransferDataInfo(&mTask.dataInfo, accEventSize, gyroEventSize, realSampleTime, data, temperature);
    sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
    mt_eint_unmask(mTask.hw->eint_num);
    return 0;
}

// bits[6:7] in OFFSET[6] to enable/disable gyro/accel offset.
// bits[0:5] in OFFSET[6] stores the most significant 2 bits of gyro offset at
// its x, y, z axies.
// Calculate the stored gyro offset and compose it with the intended
// enable/disable mode for gyro/accel offset to determine the value for
// OFFSET[6].
static uint8_t offset6Mode(void) {
    uint8_t mode = 0;
    if (mTask.sensors[GYR].offset_enable)
        mode |= 0x01 << 7;
    if (mTask.sensors[ACC].offset_enable)
        mode |= 0x01 << 6;
    mode |= (mTask.sensors[GYR].offset[2] & 0x0300) >> 4;
    mode |= (mTask.sensors[GYR].offset[1] & 0x0300) >> 6;
    mode |= (mTask.sensors[GYR].offset[0] & 0x0300) >> 8;
    DEBUG_PRINT("OFFSET_6_MODE is: %02x\n", mode);
    return mode;
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
static void gyroGetCalibration(int32_t *cali, int32_t size) {
    cali[AXIS_X] = mTask.gyroSwCali[AXIS_X];
    cali[AXIS_Y] = mTask.gyroSwCali[AXIS_Y];
    cali[AXIS_Z] = mTask.gyroSwCali[AXIS_Z];
    /* osLog(LOG_INFO, "gyroGetCalibration cali x:%d, y:%d, z:%d\n", cali[AXIS_X], cali[AXIS_Y], cali[AXIS_Z]); */
}
static void gyroSetCalibration(int32_t *cali, int32_t size) {
    mTask.gyroSwCali[AXIS_X] = cali[AXIS_X];
    mTask.gyroSwCali[AXIS_Y] = cali[AXIS_Y];
    mTask.gyroSwCali[AXIS_Z] = cali[AXIS_Z];
    /* osLog(LOG_INFO, "gyroSetCalibration cali x:%d, y:%d, z:%d\n", mTask.gyroSwCali[AXIS_X],
        mTask.gyroSwCali[AXIS_Y], mTask.gyroSwCali[AXIS_Z]); */
}
static void gyroGetData(void *sample) {
    struct TripleAxisDataPoint *tripleSample = (struct TripleAxisDataPoint *)sample;
    tripleSample->ix = mTask.gyroFactoryData.ix;
    tripleSample->iy = mTask.gyroFactoryData.iy;
    tripleSample->iz = mTask.gyroFactoryData.iz;
}

static void bmi160SetDebugTrace(int32_t trace) {
    TDECL();

    T(debug_trace) = trace;
    osLog(LOG_ERROR, "%s ==> trace:%d\n", __func__, T(debug_trace));
}

static void accGetSensorInfo(struct sensorInfo_t *data)
{
    strncpy(data->name, ACC_NAME, sizeof(data->name));
}

static void gyroGetSensorInfo(struct sensorInfo_t *data)
{
    strncpy(data->name, GYRO_NAME, sizeof(data->name));
}

static int bmi160RegisterCore(void) {
    struct sensorCoreInfo mInfo;

    /* Register sensor Core */
    memset(&mInfo, 0x00, sizeof(struct sensorCoreInfo));
    mInfo.sensType = SENS_TYPE_ACCEL;
    mInfo.gain = GRAVITY_EARTH_1000;
    mInfo.sensitivity = 4096;  // mTask.accReso->sensitivity;
    mInfo.cvt = mTask.cvt;
    mInfo.getCalibration = accGetCalibration;
    mInfo.setCalibration = accSetCalibration;
    mInfo.getData = accGetData;
    mInfo.setDebugTrace = bmi160SetDebugTrace;
    mInfo.getSensorInfo = accGetSensorInfo;
    sensorCoreRegister(&mInfo);

    memset(&mInfo, 0x00, sizeof(struct sensorCoreInfo));
    mInfo.sensType = SENS_TYPE_GYRO;
    mInfo.gain = BMI160_FS_250_LSB;  // GYROSCOPE_INCREASE_NUM_AP;
    mInfo.sensitivity = BMI160_FS_2000_LSB;  // mTask.gyroReso->sensitivity;
    mInfo.cvt = mTask.cvt;
    mInfo.getCalibration = gyroGetCalibration;
    mInfo.setCalibration = gyroSetCalibration;
    mInfo.getData = gyroGetData;
    mInfo.getSensorInfo = gyroGetSensorInfo;
    sensorCoreRegister(&mInfo);
    return 0;
}

static int sensorInit(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    TDECL();

    switch (mTask.init_state) {
    case RESET_BMI160:
        printf("RESET_BMI160\n");
        DEBUG_PRINT("Performing soft reset\n");
        // perform soft reset and wait for 100ms
        SPI_WRITE(BMI160_REG_CMD, 0xb6, 100000);
        // dummy reads after soft reset, wait 100us
        SPI_READ(BMI160_REG_MAGIC, 1, &mTask.dataBuffer, 100);

        mTask.init_state = INIT_BMI160;
        spiBatchTxRx(&mTask.mode, spiCallBack, next_state, "sensorInit RESET");
        break;

    case INIT_BMI160:
        printf("INIT_BMI160\n");
        // Read any pending interrupts to reset them
        SPI_READ(BMI160_REG_INT_STATUS_0, 4, &mTask.statusBuffer);

        // disable accel, gyro and mag data in FIFO, enable header, enable time.
        SPI_WRITE(BMI160_REG_FIFO_CONFIG_1, 0x12, 450);

        // set the watermark to 24 byte
        SPI_WRITE(BMI160_REG_FIFO_CONFIG_0, 0x06, 450);

        // FIFO watermark and fifo_full interrupt enabled
        SPI_WRITE(BMI160_REG_INT_EN_0, 0x00, 450);
        SPI_WRITE(BMI160_REG_INT_EN_1, 0x60, 450);
        SPI_WRITE(BMI160_REG_INT_EN_2, 0x00, 450);

        // INT1 enabled, high-level (push-pull) triggered.
        SPI_WRITE(BMI160_REG_INT_OUT_CTRL, 0x0a, 450);

        // INT1, INT2 input disabled, interrupt mode: non-latched
        SPI_WRITE(BMI160_REG_INT_LATCH, 0x00, 450);

        // Map all interrupts to INT1
        SPI_WRITE(BMI160_REG_INT_MAP_0, 0xFF, 450);
        SPI_WRITE(BMI160_REG_INT_MAP_1, 0xF0, 450);
        SPI_WRITE(BMI160_REG_INT_MAP_2, 0x00, 450);

        // Use pre-filtered data for tap interrupt
        SPI_WRITE(BMI160_REG_INT_DATA_0, 0x08);

        // Disable PMU_TRIGGER
        SPI_WRITE(BMI160_REG_PMU_TRIGGER, 0x00, 450);

        // tell gyro and accel to NOT use the FOC offset.
        mTask.sensors[ACC].offset_enable = false;
        mTask.sensors[GYR].offset_enable = false;
        SPI_WRITE(BMI160_REG_OFFSET_6, offset6Mode(), 450);

        // initial range for accel (+-8g) and gyro (+-2000 degree).
        SPI_WRITE(BMI160_REG_ACC_RANGE, 0x08, 450);
        SPI_WRITE(BMI160_REG_GYR_RANGE, 0x00, 450);

        // Reset step counter
        SPI_WRITE(BMI160_REG_CMD, 0xB2, 10000);
        // Reset interrupt
        SPI_WRITE(BMI160_REG_CMD, 0xB1, 10000);
        // Reset fifo
        SPI_WRITE(BMI160_REG_CMD, 0xB0, 10000);

        // no mag connected to secondary interface
        mTask.init_state = INIT_ON_CHANGE_SENSORS;
        spiBatchTxRx(&mTask.mode, spiCallBack, next_state, "sensorInit INIT");
        break;

    case INIT_ON_CHANGE_SENSORS:
        printf("INIT_ON_CHANGE_SENSORS\n");
        // configure any_motion and no_motion for 50Hz accel samples
        configMotion(MOTION_ODR);

        // select no_motion over slow_motion
        // select any_motion over significant motion
        SPI_WRITE(BMI160_REG_INT_MOTION_3, 0x15, 450);

        // int_tap_quiet=30ms, int_tap_shock=75ms, int_tap_dur=150ms
        SPI_WRITE(BMI160_REG_INT_TAP_0, 0x42, 450);

        // int_tap_th = 7 * 250 mg (8-g range)
        SPI_WRITE(BMI160_REG_INT_TAP_1, 0x01/*TAP_THRESHOLD*/, 450);

        // config step detector
        SPI_WRITE(BMI160_REG_STEP_CONF_0, 0x15, 450);
        SPI_WRITE(BMI160_REG_STEP_CONF_1, 0x03, 450);

        // int_flat_theta = 44.8 deg * (16/64) = 11.2 deg
        SPI_WRITE(BMI160_REG_INT_FLAT_0, 0x10, 450);

        // int_flat_hold_time = (640 msec)
        // int_flat_hy = 44.8 * 4 / 64 = 2.8 deg
        SPI_WRITE(BMI160_REG_INT_FLAT_1, 0x14, 450);

        mTask.init_state = INIT_DONE;
        spiBatchTxRx(&mTask.mode, spiCallBack, next_state, "sensorInit INIT_ONC");
        break;

    case INIT_DONE:
        mt_eint_dis_hw_debounce(mTask.hw->eint_num);
        mt_eint_registration(mTask.hw->eint_num, LEVEL_SENSITIVE, HIGH_LEVEL_TRIGGER, bmi160Isr1, EINT_INT_UNMASK,
            EINT_INT_AUTO_UNMASK_OFF);
        printf("INIT_DONE\n");
        bmi160RegisterCore();
        SET_STATE(SENSOR_IDLE);
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
        break;

    default:
        INFO_PRINT("Invalid init_state.\n");
    }

    return 0;
}


static void initSensorStruct(struct BMI160Sensor *sensor, enum SensorIndex idx) {
    sensor->powered = false;
    sensor->configed = false;
    sensor->rate = 0;
    sensor->offset[0] = 0;
    sensor->offset[1] = 0;
    sensor->offset[2] = 0;
    sensor->latency = 0;
    sensor->data_evt = NULL;
    sensor->flush = 0;
    sensor->prev_rtc_time = 0;
    sensor->startCali = false;
}

static struct sensorFsm bmi160Fsm[] = {
    /* Int Status Check */
    sensorFsmCmd(CHIP_HW_INT_STATUS_CHECK, CHIP_HW_INT_HANDLING, hwIntStatusCheck),
    sensorFsmCmd(CHIP_HW_INT_HANDLING, CHIP_HW_INT_HANDLING_DONE, hwIntHandling),
    /* smple */
    sensorFsmCmd(CHIP_SAMPLING, STATE_CONVERT, bmi160Sample),
    sensorFsmCmd(STATE_CONVERT, CHIP_SAMPLING_DONE, bmi160Convert),
    /* acc enable state */
    sensorFsmCmd(CHIP_ACC_ENABLE, CHIP_ACC_ENABLE_DONE, accPowerOn),
    /* acc disable state */
    sensorFsmCmd(CHIP_ACC_DISABLE, STATE_UNMASK_EINT, accPowerOff),
    /* acc disable unmask eint state */
    sensorFsmCmd(STATE_UNMASK_EINT, CHIP_ACC_DISABLE_DONE, accGyroUnmaskEint),
    /* acc rate state */
    sensorFsmCmd(CHIP_ACC_RATECHG, CHIP_ACC_RATECHG_DONE, accSetRate),
    /* acc cali state */
    sensorFsmCmd(CHIP_ACC_CALI, STATE_CALIBRATION_FOC, accCali),
    sensorFsmCmd(STATE_CALIBRATION_FOC, STATE_CALIBRATION_STATUS_CHECK, accCali),
    sensorFsmCmd(STATE_CALIBRATION_STATUS_CHECK, STATE_CALIBRATION_SET_OFFSET, accCali),
    sensorFsmCmd(STATE_CALIBRATION_SET_OFFSET, CHIP_ACC_CALI_DONE, accCali),
    /* acc cfg state */
    sensorFsmCmd(CHIP_ACC_CFG, STATE_HW_CALI_CHECK, accHwCaliCheck),
    sensorFsmCmd(STATE_HW_CALI_CHECK, CHIP_ACC_CFG_DONE, accCaliSave),
    /* gyro enable state */
    sensorFsmCmd(CHIP_GYRO_ENABLE, CHIP_GYRO_ENABLE_DONE, gyrPowerOn),
    /* gyro disable state */
    sensorFsmCmd(CHIP_GYRO_DISABLE, STATE_UNMASK_EINT, gyrPowerOff),
    /* gyro disable unmask eint state */
    sensorFsmCmd(STATE_UNMASK_EINT, CHIP_GYRO_DISABLE_DONE, accGyroUnmaskEint),
    /* gyro rate state */
    sensorFsmCmd(CHIP_GYRO_RATECHG, CHIP_GYRO_RATECHG_DONE, gyrSetRate),
    /* gyro cali state */
    sensorFsmCmd(CHIP_GYRO_CALI, STATE_CALIBRATION_FOC, gyroCali),
    sensorFsmCmd(STATE_CALIBRATION_FOC, STATE_CALIBRATION_STATUS_CHECK, gyroCali),
    sensorFsmCmd(STATE_CALIBRATION_STATUS_CHECK, STATE_CALIBRATION_SET_OFFSET, gyroCali),
    sensorFsmCmd(STATE_CALIBRATION_SET_OFFSET, CHIP_GYRO_CALI_DONE, gyroCali),
    /* gyro cfg state */
    sensorFsmCmd(CHIP_GYRO_CFG, STATE_HW_CALI_CHECK, gyroHwCaliCheck),
    sensorFsmCmd(STATE_HW_CALI_CHECK, CHIP_GYRO_CFG_DONE, gyroCaliSave),
    /* init state */
    sensorFsmCmd(CHIP_RESET, STATE_RESET_BMI160, sensorInit),
    sensorFsmCmd(STATE_RESET_BMI160, STATE_INIT_BMI160, sensorInit),
    sensorFsmCmd(STATE_INIT_BMI160, STATE_INIT_ON_CHANGE_SENSORS, sensorInit),
    sensorFsmCmd(STATE_INIT_ON_CHANGE_SENSORS, CHIP_INIT_DONE, sensorInit),
    /* For Anymotion */
    sensorFsmCmd(CHIP_ANYMO_ENABLE, CHIP_ANYMO_ENABLE_DONE, anyMotionPowerOn),
    sensorFsmCmd(CHIP_ANYMO_DISABLE, CHIP_ANYMO_DISABLE_DONE, anyMotionPowerOff),
    /* For Nomotion */
    sensorFsmCmd(CHIP_NOMO_ENABLE, CHIP_NOMO_ENABLE_DONE, noMotionPowerOn),
    sensorFsmCmd(CHIP_NOMO_DISABLE, CHIP_NOMO_DISABLE_DONE, noMotionPowerOff),
#ifdef CFG_HW_STEP_COUNTER_SUPPORT
    /* for stepCounter */
    sensorFsmCmd(CHIP_STEP_COUNT_ENABLE, CHIP_STEP_COUNT_DONE, stepCountPowerOn),
    sensorFsmCmd(CHIP_STEP_COUNT_DISABLE, CHIP_STEP_COUNT_DISABLE_DONE, stepCountPowerOff),
    sensorFsmCmd(CHIP_STEP_COUNT_SAMPLE, CHIP_STEP_COUNT_SEND, stepCntGetData),
    sensorFsmCmd(CHIP_STEP_COUNT_SEND, CHIP_STEP_COUNT_SAMPLE_DONE, StepCntSendData),
#endif
};

static int bmi160Init(void)
{
    int ret = 0;
    uint8_t txData[2] = {0}, rxData[2] = {0};
    TDECL();

    enum SensorIndex i;
    osLog(LOG_ERROR, "bmi160Init\n");
    bmi160DebugPoint = &mTask;

    insertMagicNum(&mTask.accGyroPacket);
    T(hw) = get_cust_accGyro("bmi160");
    if (NULL == T(hw)) {
        osLog(LOG_ERROR, "get_cust_acc_hw fail\n");
        ret = -1;
        goto err_out;
    }
    osLog(LOG_ERROR, "acc spi_num: %d\n", T(hw)->i2c_num);

    if (0 != (ret = sensorDriverGetConvert(T(hw)->direction, &T(cvt)))) {
        osLog(LOG_ERROR, "invalid direction: %d\n", T(hw)->direction);
    }
    osLog(LOG_ERROR, "acc map[0]:%d, map[1]:%d, map[2]:%d, sign[0]:%d, sign[1]:%d, sign[2]:%d\n\r",
        T(cvt).map[AXIS_X], T(cvt).map[AXIS_Y], T(cvt).map[AXIS_Z],
        T(cvt).sign[AXIS_X], T(cvt).sign[AXIS_Y], T(cvt).sign[AXIS_Z]);

    T(pending_int[0]) = false;
    T(pending_int[1]) = false;
    T(pending_dispatch) = false;
    T(frame_sensortime_valid) = false;
    T(poll_generation) = 0;
    T(tempCelsius) = kTempInvalid;
    T(tempTime) = 0;

    T(mode).speed = 8000000;    //8Mhz
    T(mode).bitsPerWord = 8;
    T(mode).cpol = SPI_CPOL_IDLE_LO;
    T(mode).cpha = SPI_CPHA_LEADING_EDGE;
    T(mode).nssChange = true;
    T(mode).format = SPI_FORMAT_MSB_FIRST;

    T(watermark) = 0;

    spiMasterRequest(T(hw)->i2c_num, &T(spiDev));

    mTask.latch_time_id = alloc_latch_time();
    enable_latch_time(mTask.latch_time_id, mTask.hw->eint_num);

    for (i = ACC; i < NUM_OF_SENSOR; i++) {
        initSensorStruct(&T(sensors[i]), i);
        T(pending_config[i]) = false;
    }

    T(mWbufCnt) = 0;
    T(mRegCnt) = 0;
    // T(spiInUse) = false;

    T(interrupt_enable_0) = 0x00;
    T(interrupt_enable_2) = 0x00;

    // initialize the last bmi160 time to be ULONG_MAX, so that we know it's
    // not valid yet.
    T(last_sensortime) = 0;
    T(frame_sensortime) = ULONG_LONG_MAX;
    /* init the debug_trace, default 0 */
    T(debug_trace) = 0;

    // it's ok to leave interrupt open all the time.

    SET_STATE(SENSOR_VERIFY_ID);
    // dummy reads after boot, wait 100us
    txData[0] = BMI160_REG_MAGIC | 0x80;
    ret = spiMasterRxTxSync(T(spiDev), rxData, txData, 2);
    udelay(100);
    // read the device ID for bmi160
    txData[0] = BMI160_REG_ID | 0x80;

    for (uint8_t i = 0; i < 3;) {
        ret = spiMasterRxTxSync(T(spiDev), rxData, txData, 2);
        if (ret >= 0 && (rxData[1] == BMI160_ID))
            break;
        ++i;
        if (i >= 3) {
            ret = -1;
            sendSensorErrToAp(ERR_SENSOR_ACC_GYR, ERR_CASE_ACC_GYR_INIT, ACC_NAME);
            sendSensorErrToAp(ERR_SENSOR_ACC_GYR, ERR_CASE_ACC_GYR_INIT, GYRO_NAME);
            spiMasterRelease(T(spiDev));
            disable_latch_time(mTask.latch_time_id);
            free_latch_time(mTask.latch_time_id);
            goto err_out;
        }
    }

    INFO_PRINT("bmi160 auto detect success: %02x\n", rxData[1]);
    SET_STATE(SENSOR_INITIALIZING);
    mTask.init_state = RESET_BMI160;
    accSensorRegister();
    gyroSensorRegister();
    anyMotionSensorRegister();
    noMotionSensorRegister();
#ifdef CFG_HW_STEP_COUNTER_SUPPORT
    stepCntSensorRegister();
#endif
    registerAccGyroInterruptMode(ACC_GYRO_FIFO_INTERRUPTIBLE);
    registerAccGyroDriverFsm(bmi160Fsm, ARRAY_SIZE(bmi160Fsm));

#ifdef CFG_MTK_CALIBRATION_V1_0
    const struct GyroCalParameters gyro_cal_parameters = {
        .min_still_duration_nanos        = SEC_TO_NANOS(5),
        .max_still_duration_nanos        = SEC_TO_NANOS(5.9f),
        .calibration_time_nanos          = 0,
        .window_time_duration_nanos      = SEC_TO_NANOS(1.5f),
        .bias_x                          = 0,
        .bias_y                          = 0,
        .bias_z                          = 0,
        .stillness_threshold             = 0.95f,
        .stillness_mean_delta_limit      = MDEG_TO_RAD * 40.0f,
        .gyro_var_threshold              = 7.5e-5f,
        .gyro_confidence_delta           = 1.5e-5f,
        .accel_var_threshold             = 4.5e-3f,
        .accel_confidence_delta          = 9e-4f,
        .mag_var_threshold               = 1.4f,
        .mag_confidence_delta            = 0.25f,
        .temperature_delta_limit_celsius = 1.5f,
        .gyro_calibration_enable         = true
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
        .over_temp_enable = true
    };
    overTempCalInit(GetOverTempCalBase(), &gyro_otc_parameters);
#endif
err_out:
    return ret;
}

#ifndef CFG_OVERLAY_INIT_SUPPORT
MODULE_DECLARE(bmi160, SENS_TYPE_ACCEL, bmi160Init);
#else
#include "mtk_overlay_init.h"
OVERLAY_DECLARE(bmi160, OVERLAY_ID_ACCGYRO, bmi160Init);
#endif
/**
 * Parse BMI160 FIFO frame without side effect.
 *
 * The major purpose of this function is to determine if FIFO content is received completely (start
 * to see invalid headers). If not, return the pointer to the beginning last incomplete frame so
 * additional read can use this pointer as start of read buffer.
 *
 * @param buf  buffer location
 * @param size size of data to be parsed
 *
 * @return NULL if the FIFO is received completely; or pointer to the beginning of last incomplete
 * frame for additional read.
 */
static uint8_t* shallowParseFrame(uint8_t * buf, int size) {
    int i = 0;
    int iLastFrame = 0;  // last valid frame header index

    DEBUG_PRINT_IF(DBG_SHALLOW_PARSE, "spf start %p: %x %x %x\n", buf, buf[0], buf[1], buf[2]);
    while (size > 0) {
        int fh_mode, fh_param;
        iLastFrame = i;

        if (buf[i] == BMI160_FRAME_HEADER_INVALID) {
            // no more data
            DEBUG_PRINT_IF(DBG_SHALLOW_PARSE, "spf:at%d=0x80\n", iLastFrame);
            return NULL;
        } else if (buf[i] == BMI160_FRAME_HEADER_SKIP) {
            // artifically added nop frame header, skip
            DEBUG_PRINT_IF(DBG_SHALLOW_PARSE, "at %d, skip header\n", i);
            i++;
            size--;
            continue;
        }

        // ++frame_num;

        fh_mode = buf[i] >> 6;
        fh_param = (buf[i] >> 2) & 0xf;

        i++;
        size--;

        if (fh_mode == 1) {
            // control frame.
            if (fh_param == 0) {
                // skip frame, we skip it (1 byte)
                i++;
                size--;
                DEBUG_PRINT_IF(DBG_SHALLOW_PARSE, "at %d, a skip frame\n", iLastFrame);
            } else if (fh_param == 1) {
                // sensortime frame  (3 bytes)
                i += 3;
                size -= 3;
                DEBUG_PRINT_IF(DBG_SHALLOW_PARSE, "at %d, a sensor_time frame\n", iLastFrame);
            } else if (fh_param == 2) {
                // fifo_input config frame (1byte)
                i++;
                size--;
                DEBUG_PRINT_IF(DBG_SHALLOW_PARSE, "at %d, a fifo cfg frame\n", iLastFrame);
            } else {
                size = 0;  // drop this batch
                DEBUG_PRINT_IF(DBG_SHALLOW_PARSE, "Invalid fh_param in control frame!!\n");
                // mark invalid
                buf[iLastFrame] = BMI160_FRAME_HEADER_INVALID;
                return NULL;
            }
        } else if (fh_mode == 2) {
            // regular frame, dispatch data to each sensor's own fifo
            if (fh_param & 4) {  // have accel data
                ERROR_PRINT("Should not have mag data\n");
                size = 0;  // drop this batch
                // mark invalid
                buf[iLastFrame] = BMI160_FRAME_HEADER_INVALID;
                return NULL;
            }
            if (fh_param & 2) {  // have gyro data
                i += 6;
                size -= 6;
            }
            if (fh_param & 1) {  // have accel data
                i += 6;
                size -= 6;
            }
            DEBUG_PRINT_IF(DBG_SHALLOW_PARSE, "at %d, a reg frame acc %d, gyro %d, mag %d\n",
                       iLastFrame, fh_param &1 ? 1:0, fh_param&2?1:0, fh_param&4?1:0);
        } else {
            size = 0;  // drop this batch
            DEBUG_PRINT_IF(DBG_SHALLOW_PARSE, "spf: Invalid fh_mode %d!!\n", fh_mode);
            // mark invalid
            buf[iLastFrame] = BMI160_FRAME_HEADER_INVALID;
            return NULL;
        }
    }

    if (size == 0)
        iLastFrame = i;

    // there is a partial frame, return where to write next chunck of data
    DEBUG_PRINT_IF(DBG_SHALLOW_PARSE, "partial frame ends %p\n", buf + iLastFrame);

    while ((int)(buf + iLastFrame) & 0x3) {
        buf[iLastFrame++] = BMI160_FRAME_HEADER_SKIP;
    }

    return buf + iLastFrame;
}

/**
 * Intialize the first read of chunked SPI read sequence.
 *
 * @param index starting index of the txrxBuffer in which the data will be write into.
 */
static void chunkedReadInit_(TASK, int index, int size) {
    T(mWbufCnt) = index;
    if (T(mWbufCnt) > FIFO_READ_SIZE) {
        // drop data to prevent bigger issue
        T(mWbufCnt) = 0;
    }
    T(chunkReadSize) = size > CHUNKED_READ_SIZE ? size : CHUNKED_READ_SIZE;
    DEBUG_PRINT_IF(DBG_CHUNKED, "crd %d>>%d\n", T(chunkReadSize), index);
    SPI_READ(BMI160_REG_FIFO_DATA, T(chunkReadSize), &T(dataBuffer));
    spiBatchTxRx(&T(mode), chunkedReadSpiCallback, _task, __FUNCTION__);
}

/**
 * Chunked SPI read callback.
 *
 * Handles the chunked read logic: issue additional read if necessary, or calls sensorSpiCallback()
 * if the entire FIFO is read.
 *
 * @param cookie extra data
 * @param err    error
 *
 * @see sensorSpiCallback()
 */
static void chunkedReadSpiCallback(void *cookie, int err) {
    TASK = (_Task*) cookie;
    // T(spiInUse) = false;
    if (err != 0) {
        DEBUG_PRINT_IF(DBG_CHUNKED, "crd retry");
        // read full fifo length to be safe
        chunkedReadInit(0, FIFO_READ_SIZE);
        return;
    }

    *T(dataBuffer) = BMI160_FRAME_HEADER_SKIP;  // fill the 0x00/0xff hole at the first byte
    uint8_t* end = shallowParseFrame(T(dataBuffer), T(chunkReadSize));

    if (end == NULL) {
        // if interrupt is still set after read for some reason, set the pending interrupt
        // to handle it immediately after data is handled.

        // recover the buffer and valid data size to make it looks like a single read so that
        // real frame parse works properly
        T(dataBuffer) = T(txrxBuffer);
        T(xferCnt) = FIFO_READ_SIZE;
        mTask.swSampleTime = rtcGetTime();

        /*
         * read temperature data, we can't use spiQueueRead and spiBatchTxRx instead follows,
         * if use, may corrupt data we read from fifo
         */
        T(temperatureBuffer[0]) = BMI160_SPI_READ | BMI160_REG_TEMPERATURE_0;
        T(packets[0]).size = 1 + 2 ;  // first byte invalid data, follows 2 bytes temperature data
        T(packets[0]).txBuf = &T(temperatureBuffer[0]);
        T(packets[0]).rxBuf = &T(temperatureBuffer[0]);
        T(packets[0]).delay = 0;
        spiMasterRxTx(T(spiDev), T(cs), T(packets), 1, &T(mode), T(spiCallBack), T(next_state));
    } else {
        DEBUG_PRINT_IF(DBG_CHUNKED, "crd cont");
        chunkedReadInit(end - T(txrxBuffer), CHUNKED_READ_SIZE);
    }
}

/**
 * Initiate read of sensor fifo.
 *
 * If task is in idle state, init chunked FIFO read; otherwise, submit an interrupt message or mark
 * the read pending depending if it is called in interrupt context.
 *
 * @param isInterruptContext true if called from interrupt context; false otherwise.
 *
 */
static void initiateFifoRead_(TASK, bool isInterruptContext) {
    // estimate first read size to be watermark + 1 more sample + some extra
    int firstReadSize = T(watermark) * 4 + 32;  // 1+6+6+8+1+3 + extra = 25 + extra = 32
    if (firstReadSize < CHUNKED_READ_SIZE) {
        firstReadSize = CHUNKED_READ_SIZE;
    }
    // mTask.hwSampleTime = rtcGetTime();
    chunkedReadInit(0, firstReadSize);
}
