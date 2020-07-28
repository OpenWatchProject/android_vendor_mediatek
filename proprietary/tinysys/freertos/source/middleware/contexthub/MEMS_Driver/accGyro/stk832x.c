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
#include <performance.h>
#include <API_sensor_calibration.h>

#include "stk832x.h"

#define STK832X_VERSION     "0.2.12"

//#define STK_ENV_LACK
//#define STK_DEBUG_MODE
//#define STK_HAND_DETECT
//#define STK_FIR
//#define STK_TIMEESTIMATE

#ifdef STK_HAND_DETECT
    #include "stk_hand_detect.h"
    #define STK_HAND_DETECT_CNT_THRESHOLD   9
#endif /* STK_HAND_DETECT */

#ifdef STK_FIR
    #define STK_FIR_LEN     5 /* !!! Can't 0, must more than 0.!!! */
    struct data_fir
    {
        int xyz[STK_FIR_LEN][AXES_NUM];
        int sum[AXES_NUM];
        int idx;
        int count;
    };
#endif /* STK_FIR */

#define STK_SPI_WRITE_DELAY         5 /* Perform spi write process and wait for 5usec */
#define STK_SIZE_PER_FIFO_DATA      6 /* Each fifo data involves XYZ data */

#define SPI_PACKET_SIZE             30
#define SPI_BUF_SIZE                (1024 + 4)
#define SPI_WRITE_0(addr, data) spiQueueWrite(addr, data, 2)
#define SPI_WRITE_1(addr, data, delay) spiQueueWrite(addr, data, delay)
#define GET_SPI_WRITE_MACRO(_1, _2, _3, NAME, ...) NAME
#define SPI_WRITE(...) GET_SPI_WRITE_MACRO(__VA_ARGS__, SPI_WRITE_1, SPI_WRITE_0)(__VA_ARGS__)
#define SPI_READ_0(addr, size, buf) spiQueueRead(addr, size, buf, 0)
#define SPI_READ_1(addr, size, buf, delay) spiQueueRead(addr, size, buf, delay)
#define GET_SPI_READ_MACRO(_1, _2, _3, _4, NAME, ...) NAME
#define SPI_READ(...) GET_SPI_READ_MACRO(__VA_ARGS__, SPI_READ_1, SPI_READ_0)(__VA_ARGS__)

#define EVT_SENSOR_ANY_MOTION sensorGetMyEventType(SENS_TYPE_ANY_MOTION)

#ifdef STK_TIMEESTIMATE
static inline uint64_t stk_get_deviation(uint64_t a, uint64_t b)
{
    if (a > b)
        return a - b;
    else
        return b - a;
}
#endif /* STK_TIMEESTIMATE */

enum stk832xState
{
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
    /* stk832x state */
    STK_INITIALIZE_01,
    STK_INITIALIZE_02,
    STK_INITIALIZE_03,
    STK_INITIALIZE_04,
    STK_INITIALIZE_05,
    STK_INITIALIZE_06,
    STK_INITIALIZE_07,
    STK_INITIALIZE_08,
    STK_INITIALIZE_09,
    STK_ACC_ENABLE_01,
    STK_ACC_DISABLE_01,
    STK_ACC_RATECHG_01,
    STK_ACC_RATECHG_02,
    STK_INT_HANDLING_01,
    STK_INT_HANDLING_02,
    STK_ANYMO_ENABLE_01,
    STK_ANYMO_ENABLE_02,
    STK_ANYMO_ENABLE_03,
    STK_ANYMO_DISABLE_01,
    STK_ANYMO_DISABLE_02,
    STK_ANYMO_DISABLE_03,
};

enum sensorHandleState
{
    ACC = 0,
    ANY_MOTION,
    MAX_HANDLE,
};

static struct stk832xTask
{
    /* mtk related */
    struct accGyroDataPacket accGyroPacket;
    struct accGyro_hw *hw;
    struct sensorDriverConvert cvt;
    struct SpiMode spiMode;
    struct SpiDevice *spiDev;
    SpiCbkF spiCallBack;
    struct transferDataInfo dataInfo;
    spi_cs_t cs;
    struct SpiPacket packets[SPI_PACKET_SIZE];
    uint8_t txrxBuffer[SPI_BUF_SIZE];
    uint16_t mWbufCnt;
    uint8_t mRegCnt;
    /* system related */
    int latch_time_id;
    /* accel parameter */
    bool powered;
    float sensitivity;
    uint32_t sampleRate; /* Hz */
#ifdef STK_TIMEESTIMATE
    uint32_t ts_itvl_ideal;
    uint32_t ts_tolerance;
#endif /* STK_TIMEESTIMATE */
    int drop;
    int dropCnt;
    int dropSum[AXES_NUM];
    uint64_t latency;
    bool enableAnyMotion;
    uint16_t watermark;
    uint32_t fifoDataToRead;
    uint64_t swSampleTime;
    uint64_t hwSampleTime;
    uint64_t lastSampleTime;
    uint8_t *statusBuffer;
    uint8_t *fifoBuffer;
    bool startCali;
    float staticCali[AXES_NUM];
    /* data for factory */
    struct TripleAxisDataPoint accFactoryData;
    int32_t accSwCali[AXES_NUM];
    int32_t debug_trace;
#ifdef STK_HAND_DETECT
    int32_t hd_cnt;
    int32_t hd_cnt_rls;
    int32_t hd_cnt_fbl;
    int32_t hd_cnt_rll;
    /**
     * hd_status
     * false: no hand detect alarm yet.
     * true: Start to calculate hand detect algorithm.
     */
    bool hd_status;
#endif /* STK_HAND_DETECT */
#ifdef STK_FIR
    struct data_fir     fir;
#endif /* STK_FIR */
} mTask;

typedef struct
{
    uint8_t     regBwsel;
    uint32_t    sampleRate;
    int         drop;
} _stkOdrMap;

const  static _stkOdrMap stkODRTable[] =
{
    /* 0: ODR = 1Hz */
    {
        .regBwsel   = STK832X_BWSEL_BW_7_81,
        .sampleRate = SENSOR_HZ(1.0f),
        .drop       = 15
    },
    /* 1: ODR = 15.62Hz */
    {
        .regBwsel   = STK832X_BWSEL_BW_7_81,
        .sampleRate = SENSOR_HZ(15.62f),
        .drop       = 0
    },
    /* 2: ODR = 31.26Hz */
    {
        .regBwsel   = STK832X_BWSEL_BW_15_63,
        .sampleRate = SENSOR_HZ(31.26f),
        .drop       = 0
    },
    /* 3: ODR = 62.5Hz */
    {
        .regBwsel   = STK832X_BWSEL_BW_31_25,
        .sampleRate = SENSOR_HZ(62.5f),
        .drop       = 0
    },
    /* 4: ODR = 125Hz */
    {
        .regBwsel   = STK832X_BWSEL_BW_62_5,
        .sampleRate = SENSOR_HZ(125.0f),
        .drop       = 0
    },
    /* 5: ODR = 200Hz */
    {
        .regBwsel   = STK832X_BWSEL_BW_500,
        .sampleRate = SENSOR_HZ(200.0f),
        .drop       = 5
    },
    /* 6: ODR = 250Hz */
    {
        .regBwsel   = STK832X_BWSEL_BW_125,
        .sampleRate = SENSOR_HZ(250.0f),
        .drop       = 0
    },
};

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

static int spiBatchTxRx(struct SpiMode *mode, SpiCbkF callback,
                        void *cookie, const char *src)
{
    int err = 0;

    if (mTask.mWbufCnt > SPI_BUF_SIZE)
    {
        osLog(LOG_INFO, "NO enough SPI buffer space, dropping transaction.\n");
        return -1;
    }

    if (mTask.mRegCnt > SPI_PACKET_SIZE)
    {
        osLog(LOG_INFO, "spiBatchTxRx too many packets!\n");
        return -1;
    }

    err = spiMasterRxTx(mTask.spiDev, mTask.cs, mTask.packets, mTask.mRegCnt, mode, callback, cookie);
    mTask.mRegCnt = 0;
    mTask.mWbufCnt = 0;
    return err;
}

static int stkSetDelay(int rate)
{
    int i = -1;

    for (i = 0; i < ARRAY_SIZE(stkODRTable); i++)
    {
        if (rate <= stkODRTable[i].sampleRate)
        {
            break;
        }
    }

    if (rate > stkODRTable[ARRAY_SIZE(stkODRTable) - 1].sampleRate)
    {
        i = ARRAY_SIZE(stkODRTable) - 1;
    }

    return i;
}

static void stkGetCalibration(int32_t *cali, int32_t size)
{
    cali[AXIS_X] = mTask.accSwCali[AXIS_X];
    cali[AXIS_Y] = mTask.accSwCali[AXIS_Y];
    cali[AXIS_Z] = mTask.accSwCali[AXIS_Z];
}

static void stkSetCalibration(int32_t *cali, int32_t size)
{
    mTask.accSwCali[AXIS_X] = cali[AXIS_X];
    mTask.accSwCali[AXIS_Y] = cali[AXIS_Y];
    mTask.accSwCali[AXIS_Z] = cali[AXIS_Z];
}

static void stkAccGetData(void *sample)
{
    struct TripleAxisDataPoint *tripleSample = (struct TripleAxisDataPoint *)sample;
    tripleSample->ix = mTask.accFactoryData.ix;
    tripleSample->iy = mTask.accFactoryData.iy;
    tripleSample->iz = mTask.accFactoryData.iz;
}

static void stkSetDebugTrace(int32_t trace)
{
    mTask.debug_trace = trace;
    osLog(LOG_ERROR, "%s ==> trace:%ld \n", __func__, mTask.debug_trace);
}

#ifndef STK_ENV_LACK
static void stkGetSensorInfo(struct sensorInfo_t *data)
{
    strncpy(data->name, "stk832x", sizeof(data->name));
}
#endif /* no STK_ENV_LACK */

static void stkSensorCoreRegistration(void)
{
    struct sensorCoreInfo mInfo;
#ifdef STK_DEBUG_MODE
    osLog(LOG_INFO, "%s \n", __FUNCTION__);
#endif /* STK_DEBUG_MODE */
    memset(&mInfo, 0x00, sizeof(struct sensorCoreInfo));
    /* Register sensor core */
    mInfo.sensType = SENS_TYPE_ACCEL;
    mInfo.gain = GRAVITY_EARTH_1000;
    mInfo.sensitivity = mTask.sensitivity;
    mInfo.cvt = mTask.cvt;
    mInfo.getCalibration = stkGetCalibration;
    mInfo.setCalibration = stkSetCalibration;
    mInfo.getData = stkAccGetData;
    mInfo.setDebugTrace = stkSetDebugTrace;
#ifndef STK_ENV_LACK
    mInfo.getSensorInfo = stkGetSensorInfo;
#endif /* no STK_ENV_LACK */
    sensorCoreRegister(&mInfo);
}

static void stkInitializeParameter(void)
{
    mTask.powered = false;
//    mTask.sensitivity = (float)65536 / (4 * 2); /* 0x10000 = 65536. +/-4G */
    mTask.sensitivity = 512; /* 4G */
    mTask.sampleRate = 0;
#ifdef STK_TIMEESTIMATE
    mTask.ts_itvl_ideal = 0;
    mTask.ts_tolerance = 0;
#endif /* STK_TIMEESTIMATE */
    mTask.drop = 0;
    mTask.dropCnt = 0;
    mTask.dropSum[AXIS_X] = 0;
    mTask.dropSum[AXIS_Y] = 0;
    mTask.dropSum[AXIS_Z] = 0;
    mTask.latency = SENSOR_LATENCY_NODATA;
    mTask.watermark = 1;
    mTask.enableAnyMotion = false;
    mTask.startCali = false;
    mTask.staticCali[AXIS_X] = 0;
    mTask.staticCali[AXIS_Y] = 0;
    mTask.staticCali[AXIS_Z] = 0;
    mTask.swSampleTime = 0;
    mTask.hwSampleTime = 0;
    mTask.lastSampleTime = 0;
#ifdef STK_HAND_DETECT
    mTask.hd_cnt = 0;
    mTask.hd_cnt_rls = 0;
    mTask.hd_cnt_fbl = 0;
    mTask.hd_cnt_rll = 0;
    mTask.hd_status = false;
#endif /* STK_HAND_DETECT */
#ifdef STK_FIR
    mTask.fir.count = 0;
    mTask.fir.idx = 0;
#endif /* STK_FIR */
}

static int stkFsmSwReset(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
                         void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    osLog(LOG_INFO, "%s \n", __FUNCTION__);
    /* Perform spi write process and wait for 50msec */
    SPI_WRITE(STK832X_REG_SWRST, STK832X_SWRST_VAL, 50000);
    /* accel parameter initialize */
    stkInitializeParameter();
    mTask.powered = true;
    return spiBatchTxRx(&mTask.spiMode, spiCallBack, next_state, __FUNCTION__);
}

static int stkFsmSetRange4G(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
                            void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    SPI_WRITE(STK832X_REG_RANGESEL, STK832X_RANGESEL_4G, STK_SPI_WRITE_DELAY);
#ifdef STK_DEBUG_MODE
    osLog(LOG_INFO, "%s \n", __func__);
#endif /* STK_DEBUG_MODE */
    return spiBatchTxRx(&mTask.spiMode, spiCallBack, next_state, __FUNCTION__);
}

static int stkFsmSetSlopeThd4G(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
                            void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    SPI_WRITE(STK832X_REG_SLOPETHD, STK832X_SLOPETHD_DEF >> 1, STK_SPI_WRITE_DELAY);
#ifdef STK_DEBUG_MODE
    osLog(LOG_INFO, "%s \n", __func__);
#endif /* STK_DEBUG_MODE */
    return spiBatchTxRx(&mTask.spiMode, spiCallBack, next_state, __FUNCTION__);
}

static int stkFsmSetODR32(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
                          void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
                          void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int odr = 2;
    SPI_WRITE(STK832X_REG_BWSEL, stkODRTable[odr].regBwsel, STK_SPI_WRITE_DELAY);
    mTask.sampleRate = stkODRTable[odr].sampleRate;
#ifdef STK_TIMEESTIMATE
    mTask.ts_itvl_ideal = 1024000000000ULL / mTask.sampleRate;
    mTask.ts_tolerance = mTask.ts_itvl_ideal / 10;
#endif /* STK_TIMEESTIMATE */
    mTask.drop = stkODRTable[odr].drop;
    mTask.dropCnt = 0;
    mTask.dropSum[AXIS_X] = 0;
    mTask.dropSum[AXIS_Y] = 0;
    mTask.dropSum[AXIS_Z] = 0;
#ifdef STK_FIR
    mTask.fir.count = 0;
    mTask.fir.idx = 0;
#endif /* STK_FIR */
#ifdef STK_DEBUG_MODE
    osLog(LOG_INFO, "%s \n", __func__);
#endif /* STK_DEBUG_MODE */
    return spiBatchTxRx(&mTask.spiMode, spiCallBack, next_state, __FUNCTION__);
}

/* map ANY interrupt to int1 */
static int stkFsmMapAmdIntTo1(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
        void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
        void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    SPI_WRITE(STK832X_REG_INTMAP1, STK832X_INTMAP1_ANYMOT2INT1, STK_SPI_WRITE_DELAY);
#ifdef STK_DEBUG_MODE
    osLog(LOG_INFO, "%s \n", __func__);
#endif /* STK_DEBUG_MODE */
    return spiBatchTxRx(&mTask.spiMode, spiCallBack, next_state, __FUNCTION__);
}

/* map FIFO interrupt to int1 */
static int stkFsmMapFifoIntTo1(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
        void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
        void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    SPI_WRITE(STK832X_REG_INTMAP2, STK832X_INTMAP2_FWM2INT1, STK_SPI_WRITE_DELAY);
#ifdef STK_DEBUG_MODE
    osLog(LOG_INFO, "%s \n", __func__);
#endif /* STK_DEBUG_MODE */
    return spiBatchTxRx(&mTask.spiMode, spiCallBack, next_state, __FUNCTION__);
}

static int stkFsmSensorRegistration(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
                                    void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    stkSensorCoreRegistration();
    sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
#ifdef STK_DEBUG_MODE
    osLog(LOG_INFO, "%s \n", __func__);
#endif /* STK_DEBUG_MODE */
    return 0;
}

static void stkIsr (int arg)
{
    if (mTask.latch_time_id < 0)
    {
        mTask.hwSampleTime = rtcGetTime();
    }
    else
    {
        mTask.hwSampleTime = get_latch_time_timestamp(mTask.latch_time_id);
    }

    accGyroHwIntCheckStatus();
}

static int stkFsmEintRegistration(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
                                  void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                  void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
#ifdef STK_DEBUG_MODE
    osLog(LOG_INFO, "%s \n", __func__);
#endif /* STK_DEBUG_MODE */
    mt_eint_dis_hw_debounce(mTask.hw->eint_num);
    mt_eint_registration(mTask.hw->eint_num, LEVEL_SENSITIVE, HIGH_LEVEL_TRIGGER,
                         stkIsr, EINT_INT_UNMASK, EINT_INT_AUTO_UNMASK_OFF);
    sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
    return 0;
}

/**
 * Power ON
 */
static int stkFsmPwmdNormalForAccEnableFlow(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
        void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
        void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
#ifdef STK_DEBUG_MODE
    osLog(LOG_INFO, "%s, any motion status=%d\n", __func__, mTask.enableAnyMotion);
#endif /* STK_DEBUG_MODE */
    SPI_WRITE(STK832X_REG_POWMODE, STK832X_PWMD_NORMAL, STK_SPI_WRITE_DELAY);
    mTask.powered = true;
    return spiBatchTxRx(&mTask.spiMode, spiCallBack, next_state, __FUNCTION__);
}

/* enable FIFO interrupt */
static int stkFsmEnableAccelInt(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
        void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
        void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
#ifdef STK_DEBUG_MODE
    osLog(LOG_INFO, "%s \n", __func__);
#endif /* STK_DEBUG_MODE */
    SPI_WRITE(STK832X_REG_INTEN2, STK832X_INTEN2_FWM_EN, STK_SPI_WRITE_DELAY);
    return spiBatchTxRx(&mTask.spiMode, spiCallBack, next_state, __FUNCTION__);
}

/* enable FIFO interrupt */
static int stkFsmDisableAccelInt(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
        void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
        void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
#ifdef STK_DEBUG_MODE
    osLog(LOG_INFO, "%s \n", __func__);
#endif /* STK_DEBUG_MODE */
    SPI_WRITE(STK832X_REG_INTEN2, 0, STK_SPI_WRITE_DELAY);
    return spiBatchTxRx(&mTask.spiMode, spiCallBack, next_state, __FUNCTION__);
}

/* Power OFF */
static int stkFsmPwmdSuspend(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
                             void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
#ifdef STK_DEBUG_MODE
    osLog(LOG_INFO, "%s, any motion status=%d \n", __func__, mTask.enableAnyMotion);
#endif /* STK_DEBUG_MODE */

    mTask.powered = false;
    mTask.latency = SENSOR_LATENCY_NODATA;
    registerAccGyroFifoInfo(0, 0);
    if (!mTask.enableAnyMotion)
    {
        SPI_WRITE(STK832X_REG_POWMODE, STK832X_PWMD_SUSPEND, STK_SPI_WRITE_DELAY);
        return spiBatchTxRx(&mTask.spiMode, spiCallBack, next_state, __FUNCTION__);
    }
    else
    {
        osLog(LOG_INFO, "%s: Do not turn off power, any motion status=%d. \n", __func__, mTask.enableAnyMotion);
        mt_eint_unmask(mTask.hw->eint_num);
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
        return 0;
    }
}

static uint16_t stkCalcuWm(void)
{
    uint64_t min_latency = SENSOR_LATENCY_NODATA;
    uint8_t min_watermark = 1;
    uint8_t max_watermark = MAX_RECV_PACKET;
    uint16_t watermark = 0;
    uint32_t temp_delay = 0, temp_cnt = 0, total_cnt = 0;

    if (mTask.powered && SENSOR_LATENCY_NODATA != mTask.latency)
    {
        min_latency =
            (SENSOR_LATENCY_NODATA > mTask.latency) ? mTask.latency : SENSOR_LATENCY_NODATA;
    }

    /* If acc OFF, or latency = SENSOR_LATENCY_NODATA, watermark = 0, 1, or 2 */
    if (SENSOR_LATENCY_NODATA == min_latency)
    {
        watermark = 1;
    }
    else
    {
        if (mTask.powered)
        {
            temp_delay = (1000000000ULL / mTask.sampleRate) << 10;
            temp_cnt = min_latency / temp_delay;
            min_watermark = mTask.sampleRate / SENSOR_HZ(400.0f);
            total_cnt = (temp_cnt > min_watermark) ? temp_cnt : min_watermark;
#ifdef STK_DEBUG_MODE
            osLog(LOG_INFO, "%s: delay=%d, latency:%lld, min_wm=%d, total_cnt=%d\n",
                __func__, temp_delay, min_latency, min_watermark, total_cnt);
#endif /* STK_DEBUG_MODE */
        }
        watermark = total_cnt;
        watermark = (watermark < min_watermark) ? min_watermark : watermark;
        watermark = (watermark > max_watermark) ? max_watermark : watermark;
    }

    watermark = (STK832X_CFG1_XYZ_FRAME_MAX < watermark) ? STK832X_CFG1_XYZ_FRAME_MAX : watermark;
#ifdef STK_DEBUG_MODE
    osLog(LOG_INFO, "%s: real watermark=%d \n", __func__, watermark);
#endif /* STK_DEBUG_MODE */

    return watermark;
}

/**
 * This function can only be used in STATE_ACC_RATECHG to STATE_ACC_RATECHG_DONE flow.
 */
static int stkFsmChangeAccRate(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
                               void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
                               void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    struct accGyroCntlPacket cntlPacket;
    int odr = 0;
    uint8_t regValue = 0x0;

    if ( 0 > rxControlInfo(&cntlPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize))
    {
        osLog(LOG_ERROR, "rxControlInfo failed \n");
//        sensorFsmEnqueueFakeSpiEvt(spiCallBack, STATE_ACC_RATECHG_DONE, ERROR_EVT);
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
        return -1;
    }

    osLog(LOG_INFO, "%s acc rate:%ld, latency:%lld, watermark:%ld \n",
          __FUNCTION__, cntlPacket.rate, cntlPacket.latency, cntlPacket.waterMark);

#ifdef STK_TIMEESTIMATE
    mTask.lastSampleTime = 0;
#endif /* STK_TIMEESTIMATE */

    /* Set sample rate */
    if (mTask.sampleRate != cntlPacket.rate)
    {
        odr = stkSetDelay(cntlPacket.rate);

        if (0 > odr)
        {
            osLog(LOG_ERROR, "stkSetDelay failed, odr:%d \n", odr);
            sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
            return -1;
        }

        mTask.sampleRate = stkODRTable[odr].sampleRate;
#ifdef STK_TIMEESTIMATE
        mTask.ts_itvl_ideal = 1024000000000ULL / mTask.sampleRate;
        mTask.ts_tolerance = mTask.ts_itvl_ideal / 10;
#endif /* STK_TIMEESTIMATE */
        mTask.drop = stkODRTable[odr].drop;
        mTask.dropCnt = 0;
        mTask.dropSum[AXIS_X] = 0;
        mTask.dropSum[AXIS_Y] = 0;
        mTask.dropSum[AXIS_Z] = 0;
#ifdef STK_FIR
        mTask.fir.count = 0;
        mTask.fir.idx = 0;
#endif /* STK_FIR */
#ifdef STK_DEBUG_MODE
        osLog(LOG_INFO, "%s: current sample rate:%d, drop=%d idx=%d\n", __func__, mTask.sampleRate, mTask.drop, odr);
#endif /* STK_DEBUG_MODE */
        regValue = stkODRTable[odr].regBwsel;
        SPI_WRITE(STK832X_REG_BWSEL, regValue, 1024000000 / mTask.sampleRate);
        mTask.latency = cntlPacket.latency;
        registerAccGyroFifoInfo(
            (0 == mTask.sampleRate) ? 0 : 1024000000000 / mTask.sampleRate,
            0);
        mTask.watermark= stkCalcuWm();
        return spiBatchTxRx(&mTask.spiMode, spiCallBack, next_state, __FUNCTION__);
    }
    else
    {
#ifdef STK_DEBUG_MODE
        osLog(LOG_INFO, "No change on accel sample rate. \n");
#endif /* STK_DEBUG_MODE */
        mTask.latency = cntlPacket.latency;
        registerAccGyroFifoInfo(
            (0 == mTask.sampleRate) ? 0 : 1024000000000 / mTask.sampleRate,
            0);
        mTask.watermark= stkCalcuWm();
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
        return 0;
    }
}

static int stkFsmChangeFifoMode(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
                                void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
#ifdef STK_DEBUG_MODE
    osLog(LOG_INFO, "%s wm=%d \n", __func__, mTask.watermark);
#endif /* STK_DEBUG_MODE */
    if (STK832X_CFG1_XYZ_FRAME_MAX < mTask.watermark)
    {
        osLog(LOG_ERROR, "water mark out of range (%d) \n", mTask.watermark);
        mTask.watermark = 0;
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
        return -1;
    }

    /* FIFO settings: FIFO mode + XYZ per frame */
    SPI_WRITE(STK832X_REG_CFG2,
              (STK832X_CFG2_FIFO_MODE_FIFO << STK832X_CFG2_FIFO_MODE_SHIFT)
              | STK832X_CFG2_FIFO_DATA_SEL_XYZ,
              STK_SPI_WRITE_DELAY);

    return spiBatchTxRx(&mTask.spiMode, spiCallBack, next_state, __FUNCTION__);
}

static int stkFsmChangeWatermarkLevel(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
                                      void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                      void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    uint16_t watermarkReg = mTask.watermark;
    uint8_t regValue = *((uint8_t *)&watermarkReg);

    SPI_WRITE(STK832X_REG_CFG1, regValue, STK_SPI_WRITE_DELAY);
    mt_eint_unmask(mTask.hw->eint_num); /* for other interrupt */
    return spiBatchTxRx(&mTask.spiMode, spiCallBack, next_state, __FUNCTION__);
}

static int stkFsmReadIntsts1(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
                             void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
#ifdef STK_DEBUG_MODE
    osLog(LOG_INFO, "%s, any motion status=%d \n", __func__, mTask.enableAnyMotion);
#endif /* STK_DEBUG_MODE */
    if (mTask.enableAnyMotion)
    {
        SPI_READ(STK832X_REG_INTSTS1, 1, &mTask.statusBuffer);
        return spiBatchTxRx(&mTask.spiMode, spiCallBack, next_state, __FUNCTION__);
    }
    else
    {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
        return 0;
    }
}

static int stkFsmResetLatch(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
        void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
        void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    union EmbeddedDataPoint trigger_axies;

#ifdef STK_DEBUG_MODE
    osLog(LOG_INFO, "%s, any motion status=%d \n", __func__, mTask.enableAnyMotion);
#endif /* STK_DEBUG_MODE */
    if (mTask.enableAnyMotion)
    {
#ifdef STK_DEBUG_MODE
        osLog(LOG_INFO, "%s, INTSTS1=0x%X \n", __func__, mTask.statusBuffer[1]);
#endif /* STK_DEBUG_MODE */
        /* Check any motion intsts */
        if (STK832X_INTSTS1_ANY_MOT_STS & mTask.statusBuffer[1])
        {
            trigger_axies.idata = 0x80;
            osLog(LOG_INFO, "Detected any motion \n");
#ifdef STK_HAND_DETECT
            if (!mTask.hd_status)
            {
                mTask.hd_cnt = 3;
                mTask.hd_cnt_rls = 3;
                mTask.hd_cnt_fbl = 3;
                mTask.hd_cnt_rll = 3;
                mTask.hd_status = true;
            }
#endif /* STK_HAND_DETECT */
            osEnqueueEvt(EVT_SENSOR_ANY_MOTION, trigger_axies.vptr, NULL);
        }

        /* reset latch */
        SPI_WRITE(STK832X_REG_INTCFG2,
                STK832X_INTCFG2_INT_RST | STK832X_INTCFG2_LATCHED, STK_SPI_WRITE_DELAY);
        return spiBatchTxRx(&mTask.spiMode, spiCallBack, next_state, __FUNCTION__);
    }
    else
    {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
        return 0;
    }
}

static int stkFsmReadFifoCnt(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
        void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
        void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
#ifdef STK_DEBUG_MODE
    osLog(LOG_INFO, "%s, current wm=%d \n", __func__, mTask.watermark);
#endif /* STK_DEBUG_MODE */
    if (0 < mTask.watermark)
    {
        SPI_READ(STK832X_REG_INTSTS2, 1, &mTask.statusBuffer);
        return spiBatchTxRx(&mTask.spiMode, spiCallBack, next_state, __FUNCTION__);
    }
    else
    {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
        return 0;
    }
}

static int stkFsmCheckFifo(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
                           void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
                           void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
#ifdef STK_DEBUG_MODE
    osLog(LOG_INFO, "%s \n", __func__);
#endif /* STK_DEBUG_MODE */
    if (0 < mTask.watermark)
    {
        /* Check Fifo frame cnt */
        if (STK832X_INTSTS2_FWM_STS_MASK & mTask.statusBuffer[1])
        {	
            accGyroInterruptOccur();
        }
        else
        {
            mt_eint_unmask(mTask.hw->eint_num); /* for other interrupt */
        }
    }

    sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static void stkSpiCallBackGetSwSampleTime(void *cookie, int err)
{
    if (0 != err)
    {
        osLog(LOG_ERROR, "%s error \n", __FUNCTION__);
        sensorFsmEnqueueFakeSpiEvt(mTask.spiCallBack, cookie, ERROR_EVT);
    }
    else
    {
        mTask.swSampleTime = rtcGetTime();
        sensorFsmEnqueueFakeSpiEvt(mTask.spiCallBack, cookie, SUCCESS_EVT);
    }
}

static int stkFsmGetFifoCnt(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
                            void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
#ifdef STK_DEBUG_MODE
    osLog(LOG_INFO, "%s, wm=%d \n", __func__, mTask.watermark);
#endif /* STK_DEBUG_MODE */
    if (0 < mTask.watermark)
    {
        if (0 > rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize,
                               outBuf, outSize, elemOutSize))
        {
            osLog(LOG_ERROR, "rx dataInfo error\n");
            sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
            return -1;
        }

        SPI_READ(STK832X_REG_FIFOSTS, 1, &mTask.statusBuffer);
        mTask.spiCallBack = spiCallBack;
        return spiBatchTxRx(&mTask.spiMode, stkSpiCallBackGetSwSampleTime,
                        next_state, __FUNCTION__);
    }
    else
    {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
        return 0;
    }
}

static int stkFsmReadFifoData(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
                              void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
#ifdef STK_DEBUG_MODE
    osLog(LOG_INFO, "%s, wm=%d, FIFOSTS=0x%X \n", __func__, mTask.watermark, mTask.statusBuffer[1]);
#endif /* STK_DEBUG_MODE */
    if (0 < mTask.watermark)
    {
        if (STK832X_FIFOSTS_FIFO_OVER & mTask.statusBuffer[1])
            osLog(LOG_INFO, "FIFO OVERFLOW \n");
        mTask.fifoDataToRead = mTask.statusBuffer[1] & STK832X_FIFOSTS_FIFO_FRAME_CNT_MASK;
        mTask.fifoDataToRead *= STK_SIZE_PER_FIFO_DATA;
        SPI_READ(STK832X_REG_FIFOOUT, mTask.fifoDataToRead, &mTask.fifoBuffer);
        return spiBatchTxRx(&mTask.spiMode, spiCallBack, next_state, __FUNCTION__);
    }
    else
    {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
        return 0;
    }
}

#ifdef STK_FIR
/**
 * @brief: FIR operation
 *
 * @param[in/out]: acc xyz data
 */
void stk_fir(int16_t *x, int16_t *y, int16_t *z)
{
    if (0 == STK_FIR_LEN)
        return;

    if (STK_FIR_LEN > mTask.fir.count)
    {
        mTask.fir.xyz[mTask.fir.idx][AXIS_X] = *x;
        mTask.fir.xyz[mTask.fir.idx][AXIS_Y] = *y;
        mTask.fir.xyz[mTask.fir.idx][AXIS_Z] = *z;
        mTask.fir.sum[AXIS_X] += *x;
        mTask.fir.sum[AXIS_Y] += *y;
        mTask.fir.sum[AXIS_Z] += *z;
        mTask.fir.count++;
        mTask.fir.idx++;
    }
    else
    {
        if (STK_FIR_LEN <= mTask.fir.idx)
            mTask.fir.idx = 0;

        mTask.fir.sum[AXIS_X] -= mTask.fir.xyz[mTask.fir.idx][AXIS_X];
        mTask.fir.sum[AXIS_Y] -= mTask.fir.xyz[mTask.fir.idx][AXIS_Y];
        mTask.fir.sum[AXIS_Z] -= mTask.fir.xyz[mTask.fir.idx][AXIS_Z];
        mTask.fir.xyz[mTask.fir.idx][AXIS_X] = *x;
        mTask.fir.xyz[mTask.fir.idx][AXIS_Y] = *y;
        mTask.fir.xyz[mTask.fir.idx][AXIS_Z] = *z;
        mTask.fir.sum[AXIS_X] += *x;
        mTask.fir.sum[AXIS_Y] += *y;
        mTask.fir.sum[AXIS_Z] += *z;
        mTask.fir.idx++;
        *x = mTask.fir.sum[AXIS_X] / STK_FIR_LEN;
        *y = mTask.fir.sum[AXIS_Y] / STK_FIR_LEN;
        *z = mTask.fir.sum[AXIS_Z] / STK_FIR_LEN;
    }
}
#endif /* STK_FIR */

static void stkAddDropSum(uint8_t *buf)
{
    int16_t raw_data[AXES_NUM] = {0};

    raw_data[AXIS_X] = (buf[0] | buf[1] << 8);
    raw_data[AXIS_X] >>= 4;
    raw_data[AXIS_Y] = (buf[2] | buf[3] << 8);
    raw_data[AXIS_Y] >>= 4;
    raw_data[AXIS_Z] = (buf[4] | buf[5] << 8);
    raw_data[AXIS_Z] >>= 4;
#ifdef STK_DEBUG_MODE
    osLog(LOG_INFO, "%s, raw_data:0x%08X 0x%08X 0x%08X\n", __func__, raw_data[AXIS_X], raw_data[AXIS_Y], raw_data[AXIS_Z]);
#endif /* STK_DEBUG_MODE */

    mTask.dropSum[AXIS_X] += (int)raw_data[AXIS_X];
    mTask.dropSum[AXIS_Y] += (int)raw_data[AXIS_Y];
    mTask.dropSum[AXIS_Z] += (int)raw_data[AXIS_Z];
}

static void stkParseRawData(struct accGyroData *data, uint8_t *buf)
{
    int16_t raw_data[AXES_NUM] = {0};
    int16_t remap_data[AXES_NUM] = {0};
    int32_t swCali[AXES_NUM] = {0};
    int32_t caliResult[AXES_NUM] = {0};
    float temp_data[AXES_NUM] = {0};
    stkGetCalibration(swCali, 0);

    raw_data[AXIS_X] = (buf[0] | buf[1] << 8);
    raw_data[AXIS_X] >>= 4;
    raw_data[AXIS_Y] = (buf[2] | buf[3] << 8);
    raw_data[AXIS_Y] >>= 4;
    raw_data[AXIS_Z] = (buf[4] | buf[5] << 8);
    raw_data[AXIS_Z] >>= 4;
#ifdef STK_DEBUG_MODE
    osLog(LOG_INFO, "%s, raw_data:0x%08X 0x%08X 0x%08X\n", __func__, raw_data[AXIS_X], raw_data[AXIS_Y], raw_data[AXIS_Z]);
#endif /* STK_DEBUG_MODE */

    if (0 != mTask.drop)
    {
        mTask.dropSum[AXIS_X] += (int)raw_data[AXIS_X];
        mTask.dropSum[AXIS_Y] += (int)raw_data[AXIS_Y];
        mTask.dropSum[AXIS_Z] += (int)raw_data[AXIS_Z];

        raw_data[AXIS_X] = (int16_t)(mTask.dropSum[AXIS_X] / mTask.drop);
        raw_data[AXIS_Y] = (int16_t)(mTask.dropSum[AXIS_Y] / mTask.drop);
        raw_data[AXIS_Z] = (int16_t)(mTask.dropSum[AXIS_Z] / mTask.drop);
#ifdef STK_DEBUG_MODE
        osLog(LOG_INFO, "%s, moving average:0x%08X 0x%08X 0x%08X\n", __func__, raw_data[AXIS_X], raw_data[AXIS_Y], raw_data[AXIS_Z]);
#endif /* STK_DEBUG_MODE */
    }

#ifdef STK_FIR
    if (mTask.startCali)
        stk_fir(&raw_data[AXIS_X], &raw_data[AXIS_Y], &raw_data[AXIS_Z]);
#endif /* STK_FIR */

    raw_data[AXIS_X] += swCali[AXIS_X];
    raw_data[AXIS_Y] += swCali[AXIS_Y];
    raw_data[AXIS_Z] += swCali[AXIS_Z];
    remap_data[mTask.cvt.map[AXIS_X]] = mTask.cvt.sign[AXIS_X] * raw_data[AXIS_X];
    remap_data[mTask.cvt.map[AXIS_Y]] = mTask.cvt.sign[AXIS_Y] * raw_data[AXIS_Y];
    remap_data[mTask.cvt.map[AXIS_Z]] = mTask.cvt.sign[AXIS_Z] * raw_data[AXIS_Z];
#ifdef STK_HAND_DETECT
    int hd_action = HD_NONE_ACTION;

    if (mTask.hd_status)
    {
        hd_action = stk_hand_detect((int16_t *)remap_data);

        switch (hd_action)
        {
            case HD_WAKEUP_SCREENON_ACTION:
                mTask.hd_cnt++;
                if (0 < mTask.hd_cnt_rls)
                    mTask.hd_cnt_rls--;
                if (0 < mTask.hd_cnt_fbl)
                    mTask.hd_cnt_fbl--;
                if (0 < mTask.hd_cnt_rll)
                    mTask.hd_cnt_rll--;
                break;
            case HD_RIGHTLEFT_SIDE_SCREENOFF_ACTION:
                mTask.hd_cnt_rls++;
                if (0 < mTask.hd_cnt)
                    mTask.hd_cnt--;
                if (0 < mTask.hd_cnt_fbl)
                    mTask.hd_cnt_fbl--;
				if (0 < mTask.hd_cnt_rll)
                    mTask.hd_cnt_rll--;
                break;
            case HD_FRONTBACK_LIE_SCREENOFF_ACTION:
                mTask.hd_cnt_fbl++;
                if (0 < mTask.hd_cnt)
                    mTask.hd_cnt--;
                if (0 < mTask.hd_cnt_rls)
                    mTask.hd_cnt_rls--;
                if (0 < mTask.hd_cnt_rll)
                    mTask.hd_cnt_rll--;
                break;
            case HD_RIGHTLEFT_LIE_SCREENOFF_ACTION:
                mTask.hd_cnt_rll++;
                if (0 < mTask.hd_cnt)
                    mTask.hd_cnt--;
                if (0 < mTask.hd_cnt_rls)
                   mTask.hd_cnt_rls--;
                if (0 < mTask.hd_cnt_fbl)
                    mTask.hd_cnt_fbl--;
                break;
            default:
                mTask.hd_cnt = 0;
                mTask.hd_cnt_rls = 0;
                mTask.hd_cnt_fbl = 0;
                mTask.hd_cnt_rll = 0;
				break;
        }


        if (STK_HAND_DETECT_CNT_THRESHOLD <= mTask.hd_cnt)
        {
            osLog(LOG_INFO, "Detected hand detect with wake up, screen on \n");
            mTask.hd_cnt = 0;
            mTask.hd_status = false;
        }
        else if (STK_HAND_DETECT_CNT_THRESHOLD <= mTask.hd_cnt_rls)
        {
            osLog(LOG_INFO, "Detected hand detect with right/left side, screen off \n");
            mTask.hd_cnt_rls = 0;
            mTask.hd_status = false;
        }
        else if (STK_HAND_DETECT_CNT_THRESHOLD <= mTask.hd_cnt_fbl)
        {
            osLog(LOG_INFO, "Detected hand detect with front/back lie, screen off \n");
            mTask.hd_cnt_fbl = 0;
            mTask.hd_status = false;
        }
        else if (STK_HAND_DETECT_CNT_THRESHOLD <= mTask.hd_cnt_rll)
        {
            osLog(LOG_INFO, "Detected hand detect with right/left lie, screen off \n");
            mTask.hd_cnt_rll = 0;
            mTask.hd_status = false;
        }
    }
#endif /* STK_HAND_DETECT */
    temp_data[AXIS_X] = (float)remap_data[AXIS_X] * GRAVITY_EARTH_SCALAR / mTask.sensitivity;
    temp_data[AXIS_Y] = (float)remap_data[AXIS_Y] * GRAVITY_EARTH_SCALAR / mTask.sensitivity;
    temp_data[AXIS_Z] = (float)remap_data[AXIS_Z] * GRAVITY_EARTH_SCALAR / mTask.sensitivity;

    if (UNLIKELY(mTask.startCali))
    {
        int32_t delta_time = 0;
        float calibrated_data_output[AXES_NUM] = {0};
        int accuracy = 0;
        int16_t status =
            Acc_run_factory_calibration_timeout(delta_time, temp_data,
                                                calibrated_data_output, &accuracy, rtcGetTime());

        if (0 != status)
        {
            mTask.startCali = false;

            if (0 < status)
            {
                osLog(LOG_INFO, "ACC cali detect shake \n");
                caliResult[AXIS_X] = (int32_t)(mTask.staticCali[AXIS_X] * 1000);
                caliResult[AXIS_Y] = (int32_t)(mTask.staticCali[AXIS_Y] * 1000);
                caliResult[AXIS_Z] = (int32_t)(mTask.staticCali[AXIS_Z] * 1000);
#ifndef STK_ENV_LACK
                accGyroSendCalibrationResult(SENS_TYPE_ACCEL,
                                          (int32_t *)&caliResult[0], (uint8_t)status);
#endif /* no STK_ENV_LACK */
            }
            else
            {
                osLog(LOG_ERROR, "ACC cali time out \n");
            }
        }
        else if (3 == accuracy)
        {
            mTask.startCali = false;
            mTask.staticCali[AXIS_X] = calibrated_data_output[AXIS_X] - temp_data[AXIS_X];
            mTask.staticCali[AXIS_Y] = calibrated_data_output[AXIS_Y] - temp_data[AXIS_Y];
            mTask.staticCali[AXIS_Z] = calibrated_data_output[AXIS_Z] - temp_data[AXIS_Z];
            caliResult[AXIS_X] = (int32_t)(mTask.staticCali[AXIS_X] * 1000);
            caliResult[AXIS_Y] = (int32_t)(mTask.staticCali[AXIS_Y] * 1000);
            caliResult[AXIS_Z] = (int32_t)(mTask.staticCali[AXIS_Z] * 1000);
#ifndef STK_ENV_LACK
            accGyroSendCalibrationResult(SENS_TYPE_ACCEL,
                                      (int32_t *)&caliResult[0], (uint8_t)status);
#endif /* no STK_ENV_LACK */
            osLog(LOG_INFO, "ACC cali done, caliResult:%ld %ld %ld, offset:%f %f %f \n",
                  caliResult[AXIS_X], caliResult[AXIS_Y], caliResult[AXIS_Z],
                  (double)mTask.staticCali[AXIS_X],
                  (double)mTask.staticCali[AXIS_Y],
                  (double)mTask.staticCali[AXIS_Z]);
        }
    }

    data->sensType = SENS_TYPE_ACCEL;
    data->x = temp_data[AXIS_X] + mTask.staticCali[AXIS_X];
    data->y = temp_data[AXIS_Y] + mTask.staticCali[AXIS_Y];
    data->z = temp_data[AXIS_Z] + mTask.staticCali[AXIS_Z];
    mTask.accFactoryData.ix = (int32_t)(data->x * ACCELEROMETER_INCREASE_NUM_AP);
    mTask.accFactoryData.iy = (int32_t)(data->y * ACCELEROMETER_INCREASE_NUM_AP);
    mTask.accFactoryData.iz = (int32_t)(data->z * ACCELEROMETER_INCREASE_NUM_AP);

#ifdef STK_DEBUG_MODE
//    osLog(LOG_INFO, "%s/ACC: raw_data_x:%f, raw_data_y:%f, raw_data_z:%f \n",
//            __func__, (double)data->x, (double)data->y, (double)data->z);
#endif /* STK_DEBUG_MODE */
    if (mTask.debug_trace)
    {
        osLog(LOG_ERROR, "ACC: raw_data_x:%f, raw_data_y:%f, raw_data_z:%f \n",
              (double)data->x, (double)data->y, (double)data->z);
    }
}

#ifdef STK_TIMEESTIMATE
static uint64_t stk_time_estimate(uint64_t currentTime, uint8_t accEventSize)
{
    if (0 == mTask.lastSampleTime)
    {
        return currentTime;
    }
    else
    {
        uint64_t ts_out_of_range = mTask.ts_itvl_ideal * 100;
        uint64_t ts_est = mTask.lastSampleTime + accEventSize * mTask.ts_itvl_ideal;
        uint64_t ts_deviation = stk_get_deviation(currentTime, ts_est);

        if (ts_deviation > ts_out_of_range)
        {
            osLog(LOG_ERROR, "ts itvl no regulator. previous:%lld, current:%lld, tolerance:%d\n",
                    mTask.lastSampleTime, currentTime, mTask.ts_tolerance);
            return currentTime;
        }
        else if (ts_deviation >= mTask.ts_tolerance)
        {
#ifdef STK_DEBUG_MODE
            osLog(LOG_INFO, "ts itvl not regulator, previous:%lld, current:%lld, tolerance:%d\n",
                    mTask.lastSampleTime, currentTime, mTask.ts_tolerance);
#endif /* STK_DEBUG_MODE */
            if (currentTime > ts_est)
                return ts_est + mTask.ts_tolerance - 1;
            else
                return ts_est - mTask.ts_tolerance + 1;
        }
        else
            return currentTime;
    }
}
#endif /* STK_TIMEESTIMATE */

static int stkFsmFifoConvert(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
        void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
        void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
#ifdef STK_DEBUG_MODE
    osLog(LOG_INFO, "%s, wm=%d, fifoDataToRead=%d \n", __func__, mTask.watermark, mTask.fifoDataToRead);
#endif /* STK_DEBUG_MODE */
    if (0 < mTask.watermark)
    {
        struct accGyroData *data = mTask.accGyroPacket.outBuf;
        uint8_t *fifo_data = &mTask.fifoBuffer[1];
        uint16_t fifo_offset = 0;
        uint8_t accEventSize = 0;
        uint64_t swSampleTime = 0, realSampleTime = 0;

        while (fifo_offset < mTask.fifoDataToRead)
        {
            if (0 == mTask.drop || mTask.dropCnt >= (mTask.drop - 1))
            {
                if (accEventSize < MAX_RECV_PACKET)
                {
                    stkParseRawData(&data[accEventSize], &fifo_data[fifo_offset]);
                    accEventSize++;
                    mTask.dropCnt = 0;
                    mTask.dropSum[AXIS_X] = 0;
                    mTask.dropSum[AXIS_Y] = 0;
                    mTask.dropSum[AXIS_Z] = 0;
                }
                else
                {
                    osLog(LOG_ERROR, "outBuf full, accEventSize = %d\n", accEventSize);
                }
            }
            else
            {
                mTask.dropCnt++;
                stkAddDropSum(&fifo_data[fifo_offset]);
            }

            fifo_offset += STK_SIZE_PER_FIFO_DATA;
        }

        /* If startCali is true, can't send to runtime cali in parseRawData to accGyro */
        if (mTask.startCali)
        {
            accEventSize = 0;
#ifdef STK_DEBUG_MODE
            osLog(LOG_INFO, "%s: accEventSize=0 \n", __func__);
#endif /* STK_DEBUG_MODE */
        }

        swSampleTime = mTask.swSampleTime;
#ifdef STK_TIMEESTIMATE
        realSampleTime = stk_time_estimate(swSampleTime, accEventSize);
#else /* STK_TIMEESTIMATE */
        realSampleTime = calcFakeInterruptTime(swSampleTime, mTask.hwSampleTime, mTask.lastSampleTime,
                mTask.sampleRate, mTask.powered, accEventSize, 0, 0, 0);
#endif /* STK_TIMEESTIMATE */

        mTask.hwSampleTime = realSampleTime;
        mTask.lastSampleTime = realSampleTime;
        txTransferDataInfo(&mTask.dataInfo, accEventSize, 0, realSampleTime, data, 0);
    }

    mt_eint_unmask(mTask.hw->eint_num);
    sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int stkFsmAccCali(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
                         void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    float bias[AXES_NUM] = {0};
#ifdef STK_DEBUG_MODE
    osLog(LOG_INFO, "%s \n", __FUNCTION__);
#endif /* STK_DEBUG_MODE */
    mTask.startCali = true;
#ifdef STK_FIR
    mTask.fir.count = 0;
    mTask.fir.idx = 0;
#endif /* STK_FIR */

    Acc_init_calibration(bias);
    sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int stkFsmAccCfgCali(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
                            void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    struct accGyroCaliCfgPacket caliCfgPacket;
#ifdef STK_DEBUG_MODE
    osLog(LOG_INFO, "%s \n", __func__);
#endif /* STK_DEBUG_MODE */

    if (0 > rxCaliCfgInfo(&caliCfgPacket, inBuf, inSize, elemInSize,
                          outBuf, outSize, elemOutSize))
    {
        osLog(LOG_ERROR, "%s, rx inSize and elemSize error\n", __FUNCTION__);
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, ERROR_EVT);
        return -1;
    }

#ifdef STK_DEBUG_MODE
    osLog(LOG_INFO, "%s, cfgData[0]:%d, cfgData[1]:%d, cfgData[2]:%d \n", __func__,
          caliCfgPacket.caliCfgData[0], caliCfgPacket.caliCfgData[1], caliCfgPacket.caliCfgData[2]);
#endif /* STK_DEBUG_MODE */
    mTask.staticCali[0] = (float)caliCfgPacket.caliCfgData[0] / 1000;
    mTask.staticCali[1] = (float)caliCfgPacket.caliCfgData[1] / 1000;
    mTask.staticCali[2] = (float)caliCfgPacket.caliCfgData[2] / 1000;
    sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int stkFsmPowerUpForAnymotion(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
        void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
        void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
#ifdef STK_DEBUG_MODE
    osLog(LOG_INFO, "%s, power status:%d \n", __func__, mTask.powered);
#endif /* STK_DEBUG_MODE */
    SPI_WRITE(STK832X_REG_POWMODE, STK832X_PWMD_NORMAL, STK_SPI_WRITE_DELAY);
    return spiBatchTxRx(&mTask.spiMode, spiCallBack, next_state, __FUNCTION__);
}

/* enable new data interrupt for any motion */
static int stkFsmEnableAnymotionNewDataInt(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
        void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
        void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
#ifdef STK_DEBUG_MODE
    osLog(LOG_INFO, "%s \n", __func__);
#endif /* STK_DEBUG_MODE */
    SPI_WRITE(STK832X_REG_INTEN1, STK832X_INTEN1_SLP_EN_XYZ, STK_SPI_WRITE_DELAY);
    return spiBatchTxRx(&mTask.spiMode, spiCallBack, next_state, __FUNCTION__);
}

static int stkFsmEnableLatchMode(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
                                void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
#ifdef STK_DEBUG_MODE
    osLog(LOG_INFO, "%s \n", __func__);
#endif /* STK_DEBUG_MODE */
    SPI_WRITE(STK832X_REG_INTCFG2,
              STK832X_INTCFG2_INT_RST | STK832X_INTCFG2_LATCHED, STK_SPI_WRITE_DELAY);
    return spiBatchTxRx(&mTask.spiMode, spiCallBack, next_state, __FUNCTION__);
}

static int stkFsmEnableAnymotion(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
                                 void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                 void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
#ifdef STK_DEBUG_MODE
    osLog(LOG_INFO, "%s \n", __func__);
#endif /* STK_DEBUG_MODE */
    SPI_WRITE(STK832X_REG_SIGMOT2, STK832X_SIGMOT2_ANY_MOT_EN, STK_SPI_WRITE_DELAY);
    mTask.enableAnyMotion = true;
    return spiBatchTxRx(&mTask.spiMode, spiCallBack, next_state, __FUNCTION__);
}

static int stkFsmPowerDownForAnymotion(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
                                  void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                  void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
#ifdef STK_DEBUG_MODE
    osLog(LOG_INFO, "%s, power status:%d \n", __func__, mTask.powered);
#endif /* STK_DEBUG_MODE */
    if (!mTask.powered)
    {
        registerAccGyroFifoInfo(0, 0);
        SPI_WRITE(STK832X_REG_POWMODE, STK832X_PWMD_SUSPEND, STK_SPI_WRITE_DELAY);
        return spiBatchTxRx(&mTask.spiMode, spiCallBack, next_state, __FUNCTION__);
    }
    else
    {
        sensorFsmEnqueueFakeSpiEvt(spiCallBack, next_state, SUCCESS_EVT);
        return 0;
    }
}

static int stkFsmDisableAnymotion(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
                                  void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                  void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
#ifdef STK_DEBUG_MODE
    osLog(LOG_INFO, "%s, power status:%d \n", __func__, mTask.powered);
#endif /* STK_DEBUG_MODE */
    SPI_WRITE(STK832X_REG_SIGMOT2, 0, STK_SPI_WRITE_DELAY);
    mTask.enableAnyMotion = false;
    return spiBatchTxRx(&mTask.spiMode, spiCallBack, next_state, __FUNCTION__);
}

static int stkFsmDisableLatchMode(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
                                void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    SPI_WRITE(STK832X_REG_INTCFG2, STK832X_INTCFG2_INT_RST, STK_SPI_WRITE_DELAY);
    return spiBatchTxRx(&mTask.spiMode, spiCallBack, next_state, __FUNCTION__);
}

/* disable new data interrupt for any motion */
static int stkFsmDisableAnymotionNewDataInt(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
        void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
        void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
#ifdef STK_DEBUG_MODE
    osLog(LOG_INFO, "%s \n", __func__);
#endif /* STK_DEBUG_MODE */
    SPI_WRITE(STK832X_REG_INTEN1, 0, STK_SPI_WRITE_DELAY);
    mTask.enableAnyMotion = false;
    return spiBatchTxRx(&mTask.spiMode, spiCallBack, next_state, __FUNCTION__);
}

static struct sensorFsm stk832xFsm[] =
{
    /* Initialize: STATE_SW_RESET to STATE_INIT_DONE */
    sensorFsmCmd(STATE_SW_RESET, STK_INITIALIZE_01, stkFsmSwReset),
    sensorFsmCmd(STK_INITIALIZE_01, STK_INITIALIZE_02, stkFsmSetRange4G),
    sensorFsmCmd(STK_INITIALIZE_02, STK_INITIALIZE_03, stkFsmSetSlopeThd4G),
    sensorFsmCmd(STK_INITIALIZE_03, STK_INITIALIZE_04, stkFsmSetODR32),
    sensorFsmCmd(STK_INITIALIZE_04, STK_INITIALIZE_05, stkFsmChangeFifoMode),
    sensorFsmCmd(STK_INITIALIZE_05, STK_INITIALIZE_06, stkFsmChangeWatermarkLevel),
    sensorFsmCmd(STK_INITIALIZE_06, STK_INITIALIZE_07, stkFsmMapAmdIntTo1),
    sensorFsmCmd(STK_INITIALIZE_07, STK_INITIALIZE_08, stkFsmMapFifoIntTo1),
    sensorFsmCmd(STK_INITIALIZE_08, STK_INITIALIZE_09, stkFsmSensorRegistration),
    sensorFsmCmd(STK_INITIALIZE_09, STATE_INIT_DONE, stkFsmEintRegistration),

    /* Enable accel: STATE_ACC_ENABLE to STATE_ACC_ENABLE_DONE */
    sensorFsmCmd(STATE_ACC_ENABLE, STK_ACC_ENABLE_01, stkFsmPwmdNormalForAccEnableFlow),
    sensorFsmCmd(STK_ACC_ENABLE_01, STATE_ACC_ENABLE_DONE, stkFsmEnableAccelInt),

    /* Disable accel: STATE_ACC_DISABLE to STATE_ACC_DISABLE_DONE */
    sensorFsmCmd(STATE_ACC_DISABLE, STK_ACC_DISABLE_01, stkFsmDisableAccelInt),
    sensorFsmCmd(STK_ACC_DISABLE_01, STATE_ACC_DISABLE_DONE, stkFsmPwmdSuspend),

    /* Change rate and fifo config: STATE_ACC_RATECHG to STATE_ACC_RATECHG_DONE */
    sensorFsmCmd(STATE_ACC_RATECHG, STK_ACC_RATECHG_01, stkFsmChangeAccRate),
    sensorFsmCmd(STK_ACC_RATECHG_01, STK_ACC_RATECHG_02, stkFsmChangeFifoMode),
    sensorFsmCmd(STK_ACC_RATECHG_02, STATE_ACC_RATECHG_DONE, stkFsmChangeWatermarkLevel),

    /**
     * Int handling:
     *  STATE_HW_INT_STATUS_CHECK -> STATE_HW_INT_HANDLING -> STATE_HW_INT_HANDLING_DONE
     */
    sensorFsmCmd(STATE_HW_INT_STATUS_CHECK, STATE_HW_INT_HANDLING, stkFsmReadIntsts1),
    sensorFsmCmd(STATE_HW_INT_HANDLING, STK_INT_HANDLING_01, stkFsmResetLatch),
    sensorFsmCmd(STK_INT_HANDLING_01, STK_INT_HANDLING_02, stkFsmReadFifoCnt),
    sensorFsmCmd(STK_INT_HANDLING_02, STATE_HW_INT_HANDLING_DONE, stkFsmCheckFifo),

    /**
     * Get fifo data:
     *  STATE_SAMPLE -> STATE_FIFO -> STATE_CONVERT -> STATE_SAMPLE_DONE
     */
    sensorFsmCmd(STATE_SAMPLE, STATE_FIFO, stkFsmGetFifoCnt),
    sensorFsmCmd(STATE_FIFO, STATE_CONVERT, stkFsmReadFifoData),
    sensorFsmCmd(STATE_CONVERT, STATE_SAMPLE_DONE, stkFsmFifoConvert),

    /* Init calibration: STATE_ACC_CALI to STATE_ACC_CALI_DONE */
    sensorFsmCmd(STATE_ACC_CALI, STATE_ACC_CALI_DONE, stkFsmAccCali),

    /**
     * Get calibration data and store to mTask:
     *  STATE_ACC_CFG to STATE_ACC_CFG_DONE
     */
    sensorFsmCmd(STATE_ACC_CFG, STATE_ACC_CFG_DONE, stkFsmAccCfgCali),

    /* Enable any motion: STATE_ANYMO_ENABLE to STATE_ANYMO_ENABLE_DONE */
    sensorFsmCmd(STATE_ANYMO_ENABLE, STK_ANYMO_ENABLE_01, stkFsmPowerUpForAnymotion),
    sensorFsmCmd(STK_ANYMO_ENABLE_01, STK_ANYMO_ENABLE_02, stkFsmEnableAnymotionNewDataInt),
    sensorFsmCmd(STK_ANYMO_ENABLE_02, STK_ANYMO_ENABLE_03, stkFsmEnableLatchMode),
    sensorFsmCmd(STK_ANYMO_ENABLE_03, STATE_ANYMO_ENABLE_DONE, stkFsmEnableAnymotion),

    /* Disable any motion: STATE_ANYMO_DISABLE to STATE_ANYMO_DISABLE_DONE */
    sensorFsmCmd(STATE_ANYMO_DISABLE, STK_ANYMO_DISABLE_01, stkFsmPowerDownForAnymotion),
    sensorFsmCmd(STK_ANYMO_DISABLE_01, STK_ANYMO_DISABLE_02, stkFsmDisableAnymotion),
    sensorFsmCmd(STK_ANYMO_DISABLE_02, STK_ANYMO_DISABLE_03, stkFsmDisableLatchMode),
    sensorFsmCmd(STK_ANYMO_DISABLE_03, STATE_ANYMO_DISABLE_DONE, stkFsmDisableAnymotionNewDataInt),
};

int stk832xInit(void)
{
    int ret = 0;
    uint8_t txData[2] = {0}, rxData[2] = {0};
    /* Get accGyro_hw */
    insertMagicNum(&mTask.accGyroPacket);
    mTask.hw = get_cust_accGyro("stk832x");

    if (NULL == mTask.hw)
    {
        osLog(LOG_ERROR, "get_cust_accGyro fail\n");
        return -1;
    }

    osLog(LOG_INFO, "acc spi_num:%d, stk version:%s\n", mTask.hw->i2c_num, STK832X_VERSION);

    /* Get sensorDriverConvert */
    if (0 != sensorDriverGetConvert(mTask.hw->direction, &mTask.cvt))
    {
        osLog(LOG_ERROR, "invalid direction:%d \n", mTask.hw->direction);
    }

#ifdef STK_DEBUG_MODE
    osLog(LOG_INFO,
          "acc map[0]:%d map[1]:%d map[2]:%d, sign[0]:%d sign[1]:%d sign[2]:%d \n\r",
          mTask.cvt.map[AXIS_X], mTask.cvt.map[AXIS_Y], mTask.cvt.map[AXIS_Z],
          mTask.cvt.sign[AXIS_X], mTask.cvt.sign[AXIS_Y], mTask.cvt.sign[AXIS_Z]);
#endif /* STK_DEBUG_MODE */
    /* accel parameter initialize */
    stkInitializeParameter();
    mTask.latch_time_id = alloc_latch_time();
    enable_latch_time(mTask.latch_time_id, mTask.hw->eint_num);
    /* SPI parameter */
    mTask.spiMode.speed = 8000000; /* 8Mhz */
    mTask.spiMode.bitsPerWord = 8;
    mTask.spiMode.cpol = SPI_CPOL_IDLE_LO;
    mTask.spiMode.cpha = SPI_CPHA_LEADING_EDGE;
    mTask.spiMode.nssChange = true;
    mTask.spiMode.format = SPI_FORMAT_MSB_FIRST;
    spiMasterRequest(mTask.hw->i2c_num, &mTask.spiDev);
    /* Check PID */
    txData[0] = STK832X_REG_CHIPID | 0x80;
    ret = spiMasterRxTxSync(mTask.spiDev, rxData, txData, 2);

    if (0 > ret)
    {
        osLog(LOG_ERROR, "fail in spiMasterRxTxSync, ret=%d \n", ret);
        goto exit_error;
    }
    else if (STK8323_ID != rxData[1] && STK8325_ID != rxData[1])
    {
        osLog(LOG_ERROR, "PID not match, pid:0x%02X \n", rxData[1]);
        goto exit_error;
    }

    osLog(LOG_INFO, "Get PID:0x%02X \n", rxData[1]);
    accSensorRegister();
#ifndef STK_ENV_LACK
    anyMotionSensorRegister();
#endif /* no STK_ENV_LACK */
    registerAccGyroInterruptMode(ACC_GYRO_FIFO_INTERRUPTIBLE);
    registerAccGyroDriverFsm(stk832xFsm, ARRAY_SIZE(stk832xFsm));
    return ret;
exit_error:
    spiMasterRelease(mTask.spiDev);
    disable_latch_time(mTask.latch_time_id);
    free_latch_time(mTask.latch_time_id);
    return -1;
}

#ifndef CFG_OVERLAY_INIT_SUPPORT
MODULE_DECLARE(stk832x, SENS_TYPE_ACCEL, stk832xInit);
#else
#include "mtk_overlay_init.h"
OVERLAY_DECLARE(stk832x, OVERLAY_WORK_00, stk832xInit);
#endif
