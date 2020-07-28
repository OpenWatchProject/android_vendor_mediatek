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
//#include <stdbool.h>
//#include <stdint.h>
#include <seos.h>
#include <util.h>
#include <sensors.h>
#include <plat/inc/rtc.h>
#include <contexthub_core.h>
#include <mt_gpt.h>
#include "eint.h"
#include "cm36558.h"
#include "alsps.h"
#include "cache_internal.h"

#define ALS_NAME     "cm36558_l"
#define PS_NAME      "cm36558_p"
#define I2C_SPEED                       400000

#define UNIT_MS_TO_NS 1000000

enum cm36558State {
    STATE_SAMPLE_ALS = CHIP_SAMPLING_ALS,
    STATE_SAMPLE_ALS_DONE = CHIP_SAMPLING_ALS_DONE,
    STATE_SAMPLE_PS = CHIP_SAMPLING_PS,
    STATE_SAMPLE_PS_ONCE = CHIP_SAMPLING_PS_ONCE,
    STATE_SAMPLE_PS_DONE = CHIP_SAMPLING_PS_DONE,
    STATE_ALS_ENABLE = CHIP_ALS_ENABLE,
    STATE_ALS_ENABLE_DONE = CHIP_ALS_ENABLE_DONE,
    STATE_ALS_DISABLE = CHIP_ALS_DISABLE,
    STATE_ALS_DISABLE_DONE = CHIP_ALS_DISABLE_DONE,
    STATE_ALS_RATECHG = CHIP_ALS_RATECHG,
    STATE_ALS_RATECHG_DONE = CHIP_ALS_RATECHG_DONE,
    STATE_ALS_CALI = CHIP_ALS_CALI,
    STATE_ALS_CALI_DONE = CHIP_ALS_CALI_DONE,
    STATE_ALS_CFG = CHIP_ALS_CFG,
    STATE_ALS_CFG_DONE = CHIP_ALS_CFG_DONE,
    STATE_PS_ENABLE = CHIP_PS_ENABLE,
    STATE_PS_ENABLE_DONE = CHIP_PS_ENABLE_DONE,
    STATE_PS_DISABLE = CHIP_PS_DISABLE,
    STATE_PS_DISABLE_DONE = CHIP_PS_DISABLE_DONE,
    STATE_PS_RATECHG = CHIP_PS_RATECHG,
    STATE_PS_RATECHG_DONE = CHIP_PS_RATECHG_DONE,
    STATE_PS_CALI = CHIP_PS_CALI,
    STATE_PS_CALI_DONE = CHIP_PS_CALI_DONE,
    STATE_PS_CFG = CHIP_PS_CFG,
    STATE_PS_CFG_DONE = CHIP_PS_CFG_DONE,
    STATE_RGBW_ENABLE = CHIP_RGBW_ENABLE,
    STATE_RGBW_ENABLE_DONE = CHIP_RGBW_ENABLE_DONE,
    STATE_RGBW_DISABLE = CHIP_RGBW_DISABLE,
    STATE_RGBW_DISABLE_DONE = CHIP_RGBW_DISABLE_DONE,
    STATE_RGBW_RATECHG = CHIP_RGBW_RATECHG,
    STATE_RGBW_RATECHG_DONE = CHIP_RGBW_RATECHG_DONE,
    STATE_INIT_DONE = CHIP_INIT_DONE,
    STATE_IDEL = CHIP_IDLE,
    STATE_RESET = CHIP_RESET,

    STATE_GET_ALS_VAL,
    STATE_ALS_SET_DEBOUNCE,
    STATE_GET_PS_STAT,
    STATE_GET_PS_RAW_DATA,
    STATE_GET_PS_RAW_DATA_ONCE,
    STATE_PS_SET_DEBOUNCE,
    STATE_ALS_CONF_R,
    STATE_ALS_THDH_W,
    STATE_ALS_THDL_W,
    STATE_ALS_POWER_ON,
    STATE_ALS_POWER_OFF,
    STATE_PS_CONF_R,
    STATE_PS_CONF1_2_W,
    STATE_PS_CONF3_MS_W,
    STATE_PS_CANC_W,
    STATE_PS_THDH_W,
    STATE_PS_THDL_W,
    STATE_PS_POWER_ON,
    STATE_PS_POWER_OFF,
    STATE_PS_GET_CONF,
    STATE_PS_UNMASK_EINT,
    STATE_SETUP_EINT,
    STATE_CORE
};

struct scale_factor {
    unsigned char  whole;
    unsigned char  fraction;
};

#define MAX_RXBUF 8
#define MAX_TXBUF 8
SRAM_REGION_BSS static struct cm36558Task {
    /* txBuf for i2c operation, fill register and fill value */
    bool alsPowerOn;
    bool psPowerOn;
    unsigned int    als_debounce_time;
    unsigned int    als_debounce_on;    /*indicates if the debounce is on*/
    uint64_t        als_debounce_end;
    unsigned int    ps_debounce_time;
    unsigned int    ps_debounce_on;     /*indicates if the debounce is on*/
    uint64_t        ps_debounce_end;
    unsigned int    ps_suspend;
    uint8_t txBuf[MAX_TXBUF];
    /* rxBuf for i2c operation, receive rawdata */
    uint8_t rxBuf[MAX_RXBUF];
    uint8_t deviceId;
    /* data for factory */
    struct alsps_hw *hw;
    uint8_t i2c_addr;
    struct transferDataInfo dataInfo;
    struct AlsPsData data[2];
    uint32_t als_raw_data;
    uint32_t prox_raw_data;
    int32_t psCali;
    int32_t alsCali;
    uint32_t    ps_threshold_high;
    uint32_t    ps_threshold_low;
    uint32_t    als_threshold_high;
    uint32_t    als_threshold_low;
    bool rgbw_enabled;
} mTask;

static struct cm36558Task *cm36558DebugPoint;

static int check_timeout(uint64_t *end_tick)
{
    return rtcGetTime() > *end_tick ? 1 : 0;
}

static void cal_end_time(unsigned int ms, uint64_t *end_tick)
{
    *end_tick = rtcGetTime() +
            ((uint64_t)ms * UNIT_MS_TO_NS);
}


static int cm36558_read_als(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                            void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    if (rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize)) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "als rx error\n");
        return -1;
    }
    mTask.txBuf[0] = CM36558_REG_ALS_DATA;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 2, i2cCallBack,
                         next_state);
}

int getLuxFromAlsData(void)
{
    int idx;
    int invalid = 0;
    int als_level_num, als_value_num, als_diff;

    als_level_num = sizeof(mTask.hw->als_level) / sizeof(mTask.hw->als_level[0]);
    als_value_num = sizeof(mTask.hw->als_value) / sizeof(mTask.hw->als_value[0]);
    als_diff = als_value_num - als_level_num;

    for (idx = 0; idx < als_level_num; idx++) {
        if (mTask.als_raw_data < mTask.hw->als_level[idx]) {
            break;
        }
    }

    if (idx >= als_value_num) {
        idx = als_value_num - 1;
    }
    if (1 == mTask.als_debounce_on) {
        if (check_timeout(&mTask.als_debounce_end)) {
            mTask.als_debounce_on = 0;
        }
        if (1 == mTask.als_debounce_on) {
            invalid = 1;
        }
    }
    if (!invalid) {
        int level_high, level_low, level_diff, value_high, value_low, value_diff, value = 0;

        if (idx >= als_level_num) {
            level_high = mTask.hw->als_level[idx - als_diff];
            level_low = (idx > 0) ? mTask.hw->als_level[idx - 1 - als_diff] : 0;
        } else {
            level_high = mTask.hw->als_level[idx];
            level_low = (idx > 0) ? mTask.hw->als_level[idx - 1] : 0;
        }
        level_diff = level_high - level_low;
        value_high = mTask.hw->als_value[idx];
        value_low = (idx > 0) ? mTask.hw->als_value[idx - 1] : 0;
        value_diff = value_high - value_low;

        if ((level_low >= level_high) || (value_low >= value_high))
            value = value_low;
        else
            value = (level_diff * value_low + (mTask.als_raw_data - level_low) * value_diff + ((
                         level_diff + 1) >> 1)) / level_diff;

        ///APS_LOG("ALS: %d [%d, %d] => %d [%d, %d] \n\r", als, level_low, level_high, value, value_low, value_high);
        return value;
    } else {
        /*
        if (mTask.trace & CMC_TRC_CVT_ALS) {
            APS_LOG("ALS: %05d => %05d (-1)\n\r", als, mTask.hw->als_value[idx]);
        }*/
        return -1;
    }
}

static int cm36558_get_als_value(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                 void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                 void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int count = 0;
    int als_data = 0;
    static uint32_t a = 0;
    mTask.als_raw_data = ((mTask.rxBuf[1] << 8) | mTask.rxBuf[0]);
    als_data = getLuxFromAlsData();  //aal mapping value

    if (als_data < 0)
        mTask.data[0].sensType = SENS_TYPE_INVALID; /*invalid lux value when debounce is on*/
    else {
        mTask.data[0].als_data = als_data;
        mTask.data[0].sensType = SENS_TYPE_ALS;
    }

    count++;
    if (mTask.rgbw_enabled) {
        mTask.data[1].r = a++;
        mTask.data[1].g = a++;
        mTask.data[1].b = a++;
        mTask.data[1].w = a++;
        mTask.data[1].sensType = SENS_TYPE_RGBW;
        count++;
    }
    txTransferDataInfo(&mTask.dataInfo, count, &mTask.data[0]);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static void alsGetData(void *sample)
{
    struct SingleAxisDataPoint *singleSample = (struct SingleAxisDataPoint *)sample;
    singleSample->idata = getLuxFromAlsData();
}

static int cm36558_read_ps(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                           void *inBuf, uint8_t inSize, uint8_t elemInSize,
                           void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    if (rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize)) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "ps rx error\n");
        return -1;
    }
    mTask.txBuf[0] = CM36558_REG_INT_FLAG;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 2, i2cCallBack,
                         next_state);
}

static int cm36558_get_ps_status(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                 void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                 void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    if (mTask.rxBuf[1] & 0x1) {
        mTask.data[0].prox_state = PROX_STATE_FAR; /* far */
        mTask.data[0].prox_data = 1; /* far state distance is 1cm */
    } else if (mTask.rxBuf[1] & 0x2) {
        mTask.data[0].prox_state = PROX_STATE_NEAR; /* near */
        mTask.data[0].prox_data = 0; /* near state distance is 0cm */
    } else {
        osLog(LOG_INFO, "%s: Interrupt flag error\n", __func__);
        return -1;
    }
    mTask.txBuf[0] = CM36558_REG_PS_DATA;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 2, i2cCallBack,
                         next_state);
}

static int cm36558_get_ps_raw_data(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                   void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                   void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.prox_raw_data = mTask.rxBuf[1] << 8 | mTask.rxBuf[0];
    if (mTask.prox_raw_data < mTask.psCali)
        mTask.prox_raw_data = 0;
    mTask.prox_raw_data -= mTask.psCali;
    mt_eint_unmask(mTask.hw->eint_num); /* MT6797 use 11 */

    mTask.data[0].sensType = SENS_TYPE_PROX;
    txTransferDataInfo(&mTask.dataInfo, 1, &mTask.data[0]);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int cm36558_read_ps_once(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                           void *inBuf, uint8_t inSize, uint8_t elemInSize,
                           void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    if (rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize)) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "ps rx error\n");
        return -1;
    }
    mTask.txBuf[0] = CM36558_REG_PS_DATA;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 2, i2cCallBack,
                         next_state);
}

static int cm36558_get_ps_raw_data_once(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                   void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                   void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.prox_raw_data = mTask.rxBuf[1] << 8 | mTask.rxBuf[0];
    if (mTask.prox_raw_data < mTask.psCali)
        mTask.prox_raw_data = 0;
    mTask.prox_raw_data -= mTask.psCali;
/* for the first state reporting*/
    uint32_t ps_flag = 0;
    if (mTask.prox_raw_data >= mTask.ps_threshold_high) {
        ps_flag = 1;
    } else if (mTask.prox_raw_data <= mTask.ps_threshold_low) {
        ps_flag = 0;
    }
    if (ps_flag == 0) { /* 0 is far */
        mTask.data[0].prox_state = PROX_STATE_FAR; /* far */
        mTask.data[0].prox_data = 1; /* far state distance is 1cm */
    } else {
        mTask.data[0].prox_state = PROX_STATE_NEAR; /* near */
        mTask.data[0].prox_data = 0; /* near state distance is 0cm */
    }

    mTask.data[0].sensType = SENS_TYPE_PROX;
    txTransferDataInfo(&mTask.dataInfo, 1, &mTask.data[0]);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static void psGetData(void *sample)
{
    char txBuf[1];
    static char rxBuf[2];
    struct TripleAxisDataPoint *tripleSample = (struct TripleAxisDataPoint *)sample;
    tripleSample->ix = mTask.prox_raw_data;
    tripleSample->iy = (mTask.data[0].prox_state == PROX_STATE_NEAR ? 0 : 1);

    void get_ps_data(void *cookie, size_t tx, size_t rx, int err) {
        char *rxBuf = cookie;
        if (err == 0) {
            mTask.prox_raw_data = rxBuf[1] << 8 | rxBuf[0];
            if (mTask.prox_raw_data < mTask.psCali)
                mTask.prox_raw_data = 0;
            mTask.prox_raw_data -= mTask.psCali;
        } else
            osLog(LOG_INFO, "psGetData error (%d)\n", err);
    }
    txBuf[0] = CM36558_REG_PS_DATA;
    i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, txBuf, 1,
                  rxBuf, 2, get_ps_data, rxBuf);
}

void cm36558_ps_set_threshold(uint32_t threshold_high, uint32_t threshold_low)
{
    int ret;
    char txBuf[6];

    if (threshold_high <= 0 || threshold_high >= 0xffff
            || threshold_low <= 0 || threshold_low >= 0xffff
            || threshold_high < threshold_low) {
        osLog(LOG_INFO, "%s: Invalid thresholds[%u, %u]\n",
            __func__, threshold_high, threshold_low);
        return;
    }
    txBuf[0] = CM36558_REG_PS_THDH;
    txBuf[1] = (unsigned char)(threshold_high & 0xFF);
    txBuf[2] = (unsigned char)(threshold_high >> 8);
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, txBuf, 3,
                      NULL, 0);
    if (ret) {
        osLog(LOG_INFO, "ps thr_h error (%d)\n", ret);
        return;
    }
    txBuf[3] = CM36558_REG_PS_THDL;
    txBuf[4] = (unsigned char)(threshold_low & 0xFF);
    txBuf[5] = (unsigned char)(threshold_low >> 8);
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, &txBuf[3], 3,
                NULL, 0);
    if (ret) {
        osLog(LOG_INFO, "ps thr_l error (%d)\n", ret);
        return;
    }
}

static void psGetCalibration(int32_t *cali, int32_t size)
{
    cali[0] = mTask.psCali;
}

static void psSetCalibration(int32_t *cali, int32_t size)
{
    mTask.psCali = cali[0];
    cm36558_ps_set_threshold(mTask.ps_threshold_high + mTask.psCali,
                             mTask.ps_threshold_low + mTask.psCali);
}

static void psGetThreshold(uint32_t *threshold_high, uint32_t *threshold_low)
{
    *threshold_high = mTask.ps_threshold_high;
    *threshold_low = mTask.ps_threshold_low;
}

static void psSetThreshold(uint32_t threshold_high, uint32_t threshold_low)
{
    osLog(LOG_ERROR, "%s threshold_high:%lu threshold_low:%lu\n", __func__, threshold_high, threshold_low);
    mTask.ps_threshold_high = threshold_high;
    mTask.ps_threshold_low = threshold_low;
    cm36558_ps_set_threshold(mTask.ps_threshold_high, mTask.ps_threshold_low);
}

static void psGetSensorInfo(struct sensorInfo_t *data)
{
    strncpy(data->name, PS_NAME, sizeof(data->name));
}

static void alsGetSensorInfo(struct sensorInfo_t *data)
{
    strncpy(data->name, ALS_NAME, sizeof(data->name));
}

static int cm36558_set_als_debounce_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                       void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                       void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.als_debounce_on = 1;
    cal_end_time(mTask.als_debounce_time, &mTask.als_debounce_end);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int cm36558_set_ps_debounce_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                      void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                      void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.ps_debounce_on = 1;
    cal_end_time(mTask.ps_debounce_time, &mTask.ps_debounce_end);
    mTask.data[0].prox_state = PROX_STATE_INIT;
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int cm36558_set_als_power_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                    void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.txBuf[0] = CM36558_REG_ALS_CONF;
    mTask.txBuf[1] = mTask.rxBuf[0] & 0xFE;
    mTask.txBuf[2] = mTask.rxBuf[1];
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3,
                       i2cCallBack, next_state);
}

static int cm36558_set_als_power_off(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                     void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                     void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.als_debounce_on = 0;
    mTask.txBuf[0] = CM36558_REG_ALS_CONF;
    mTask.txBuf[1] = mTask.rxBuf[0] | 0x01;
    mTask.txBuf[2] = mTask.rxBuf[1];
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3,
                       i2cCallBack, next_state);
}

static int cm36558_als_ratechg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                               void *inBuf, uint8_t inSize, uint8_t elemInSize,
                               void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int cm36558_als_cali(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    int32_t alsCali[2];
    float   alsCali_mi;

    alsCali_mi = 0.345;
    alsCali[0] = alsCali_mi * ALS_INCREASE_NUM_AP;
    alsCali[1] = 0;

    alsPsSendCalibrationResult(SENS_TYPE_ALS, alsCali);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int cm36558_als_cfg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    struct alspsCaliCfgPacket caliCfgPacket;

    ret = rxCaliCfgInfo(&caliCfgPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);

    if (ret < 0) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "cm36558_als_cfg, rx inSize and elemSize error\n");
        return -1;
    }
    mTask.alsCali = caliCfgPacket.caliCfgData[0];
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int cm36558_mask_eint(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mt_eint_mask(mTask.hw->eint_num);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

/*  Need add unmask operation to ratechg, if sensor has setting rate */
static int cm36558_unmask_eint(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mt_eint_unmask(mTask.hw->eint_num);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int cm36558_set_ps_power_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                   void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                   void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.txBuf[0] = CM36558_REG_PS_CONF1_2;
    mTask.txBuf[1] = mTask.rxBuf[0] & 0xFE;
    mTask.txBuf[2] = mTask.rxBuf[1];
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3,
                       i2cCallBack, next_state);
}

static int cm36558_set_ps_power_off(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                    void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.ps_debounce_on = 0;
    mTask.data[0].prox_state = PROX_STATE_FAR; /* default value */
    mTask.txBuf[0] = CM36558_REG_PS_CONF1_2;
    mTask.txBuf[1] = mTask.rxBuf[0] | 0x01;
    mTask.txBuf[2] = mTask.rxBuf[1];
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3,
                       i2cCallBack, next_state);
}

static int cm36558_ps_ratechg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int cm36558_ps_cali(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    int32_t threshold[2];
    threshold[0] = 2; // high simulation data
    threshold[1] = 3; // low simulation data
    alsPsSendCalibrationResult(SENS_TYPE_PROX, threshold);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int cm36558_ps_cfg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    struct alspsCaliCfgPacket caliCfgPacket;

    ret = rxCaliCfgInfo(&caliCfgPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);

    if (ret < 0) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "cm36558_ps_cfg, rx inSize and elemSize error\n");
        return -1;
    }
    cm36558_ps_set_threshold(caliCfgPacket.caliCfgData[0], caliCfgPacket.caliCfgData[1]);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int cm36558_get_ps_conf(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                               void *inBuf, uint8_t inSize, uint8_t elemInSize,
                               void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.txBuf[0] = CM36558_REG_PS_CONF1_2;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 2, i2cCallBack,
                         next_state);
}

static int cm36558_get_als_conf(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.txBuf[0] = CM36558_REG_ALS_CONF;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 2, i2cCallBack,
                         next_state);
}

static int cm36558_set_als_conf(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.txBuf[0] = CM36558_REG_ALS_CONF;
    if (1 == mTask.hw->polling_mode_als)
        mTask.txBuf[1] = 0b00000001;
    else
        mTask.txBuf[1] = 0b00000011;
    mTask.txBuf[2] = 0b00000001;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3,
                       i2cCallBack, next_state);
}

static int cm36558_set_als_thdh(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //use polling
    mTask.txBuf[0] = CM36558_REG_ALS_THDH;
    mTask.txBuf[1] = (unsigned char)(mTask.als_threshold_high & 0xFF);
    mTask.txBuf[2] = (unsigned char)(mTask.als_threshold_high >> 8);
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3,
                       i2cCallBack, next_state);
}

static int cm36558_set_als_thdl(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //use polling
    mTask.txBuf[0] = CM36558_REG_ALS_THDL;
    mTask.txBuf[1] = (unsigned char)(mTask.als_threshold_low & 0xFF);
    mTask.txBuf[2] = (unsigned char)(mTask.als_threshold_low >> 8);
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3,
                       i2cCallBack, next_state);
}

static int cm36558_set_ps_conf1_2(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                  void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                  void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.txBuf[0] = CM36558_REG_PS_CONF1_2;
    mTask.txBuf[1] = 0b00010001;
    /* use interrupt */
    mTask.txBuf[2] = 0b00000011;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3,
                       i2cCallBack, next_state);
}

static int cm36558_set_ps_conf3_ms(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                   void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                   void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.txBuf[0] = CM36558_REG_PS_CONF3_MS;
    mTask.txBuf[1] = 0b00010000;
    mTask.txBuf[2] = 0b00000010;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3,
                       i2cCallBack, next_state);
}

static int cm36558_set_ps_canc(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                               void *inBuf, uint8_t inSize, uint8_t elemInSize,
                               void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.txBuf[0] = CM36558_REG_PS_CANC;
    mTask.txBuf[1] = 0x00;
    mTask.txBuf[2] = 0x00;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3,
                       i2cCallBack, next_state);
}

static int cm36558_set_ps_thdh(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                               void *inBuf, uint8_t inSize, uint8_t elemInSize,
                               void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.txBuf[0] = CM36558_REG_PS_THDH;
    mTask.txBuf[1] = (unsigned char)((mTask.ps_threshold_high + mTask.psCali) & 0xFF);
    mTask.txBuf[2] = (unsigned char)((mTask.ps_threshold_high + mTask.psCali) >> 8);
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3,
                       i2cCallBack, next_state);
}

static int cm36558_set_ps_thdl(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                               void *inBuf, uint8_t inSize, uint8_t elemInSize,
                               void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.txBuf[0] = CM36558_REG_PS_THDL;
    mTask.txBuf[1] = (unsigned char)((mTask.ps_threshold_low + mTask.psCali) & 0xFF);
    mTask.txBuf[2] = (unsigned char)((mTask.ps_threshold_low + mTask.psCali) >> 8);
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3,
                       i2cCallBack, next_state);
}

static int cm36558_rgbw_enable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                               void *inBuf, uint8_t inSize, uint8_t elemInSize,
                               void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.rgbw_enabled = true;
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int cm36558_rgbw_disable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                               void *inBuf, uint8_t inSize, uint8_t elemInSize,
                               void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.rgbw_enabled = false;
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int cm36558_rgbw_ratechg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                               void *inBuf, uint8_t inSize, uint8_t elemInSize,
                               void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

SRAM_REGION_FUNCTION void cm36558_eint_handler(int arg)
{
    alsPsInterruptOccur();
}

static int cm36558_setup_eint(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
#if 1
    mt_eint_dis_hw_debounce(mTask.hw->eint_num); /* MT6797 use 11 */
    mt_eint_registration(mTask.hw->eint_num, LEVEL_SENSITIVE, LOW_LEVEL_TRIGGER, cm36558_eint_handler, EINT_INT_UNMASK,
                         EINT_INT_AUTO_UNMASK_OFF); /* MT6797 use 11 */
#endif
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int cm36558_register_core(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                 void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                 void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    struct sensorCoreInfo mInfo;
    int ret = 0;

    memset(&mInfo, 0x00, sizeof(struct sensorCoreInfo));
    /* Register sensor Core */
    mInfo.sensType = SENS_TYPE_ALS;
    ///mInfo.gain = GRAVITY_EARTH_1000;
    ///mInfo.cvt = mTask.cvt;
    ///mInfo.getCalibration = accGetCalibration;
    ///mInfo.setCalibration = accSetCalibration;
    mInfo.getData = alsGetData;
    mInfo.getSensorInfo = alsGetSensorInfo;
    sensorCoreRegister(&mInfo);

    mInfo.sensType = SENS_TYPE_PROX,
    mInfo.gain = 1;
    mInfo.sensitivity = 1;
    ret = sensorDriverGetConvert(0, &mInfo.cvt);
    if (ret != 0)
        osLog(LOG_ERROR, "alsps: invalid direction!\n");
    mInfo.getCalibration = psGetCalibration;
    mInfo.setCalibration = psSetCalibration;
    mInfo.getThreshold = psGetThreshold;
    mInfo.setThreshold = psSetThreshold;
    mInfo.getData = psGetData;
    mInfo.getSensorInfo = psGetSensorInfo;
    sensorCoreRegister(&mInfo);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static struct sensorFsm cm36558Fsm[] = {
    sensorFsmCmd(STATE_SAMPLE_ALS, STATE_GET_ALS_VAL, cm36558_read_als),
    sensorFsmCmd(STATE_GET_ALS_VAL, STATE_SAMPLE_ALS_DONE, cm36558_get_als_value),

    sensorFsmCmd(STATE_SAMPLE_PS, STATE_GET_PS_STAT, cm36558_read_ps),
    sensorFsmCmd(STATE_GET_PS_STAT, STATE_GET_PS_RAW_DATA, cm36558_get_ps_status),
    sensorFsmCmd(STATE_GET_PS_RAW_DATA, STATE_SAMPLE_PS_DONE, cm36558_get_ps_raw_data),
    /*ps sample data once, for interrupt mode*/
    sensorFsmCmd(STATE_SAMPLE_PS_ONCE, STATE_GET_PS_RAW_DATA_ONCE, cm36558_read_ps_once),
    sensorFsmCmd(STATE_GET_PS_RAW_DATA_ONCE, STATE_SAMPLE_PS_DONE, cm36558_get_ps_raw_data_once),
    /* als enable state */
    sensorFsmCmd(STATE_ALS_ENABLE, STATE_ALS_POWER_ON, cm36558_get_als_conf),
    sensorFsmCmd(STATE_ALS_POWER_ON, STATE_ALS_SET_DEBOUNCE, cm36558_set_als_power_on),
    sensorFsmCmd(STATE_ALS_SET_DEBOUNCE, STATE_ALS_ENABLE_DONE, cm36558_set_als_debounce_on),
    /* als disable state */
    sensorFsmCmd(STATE_ALS_DISABLE, STATE_ALS_POWER_OFF, cm36558_get_als_conf),
    sensorFsmCmd(STATE_ALS_POWER_OFF, STATE_ALS_DISABLE_DONE, cm36558_set_als_power_off),
    /* als rate state */
    sensorFsmCmd(STATE_ALS_RATECHG, CHIP_ALS_RATECHG_DONE, cm36558_als_ratechg),
    /* als cali state */
    sensorFsmCmd(STATE_ALS_CALI, CHIP_ALS_CALI_DONE, cm36558_als_cali),
    /* als cfg state */
    sensorFsmCmd(STATE_ALS_CFG, CHIP_ALS_CFG_DONE, cm36558_als_cfg),
    /* ps enable state */
    sensorFsmCmd(STATE_PS_ENABLE, STATE_PS_POWER_ON, cm36558_get_ps_conf),
    sensorFsmCmd(STATE_PS_POWER_ON, STATE_PS_SET_DEBOUNCE, cm36558_set_ps_power_on),
    sensorFsmCmd(STATE_PS_SET_DEBOUNCE, STATE_PS_UNMASK_EINT, cm36558_set_ps_debounce_on),
    sensorFsmCmd(STATE_PS_UNMASK_EINT, STATE_PS_ENABLE_DONE, cm36558_unmask_eint),
    /* ps disable state */
    sensorFsmCmd(STATE_PS_DISABLE, STATE_PS_GET_CONF, cm36558_mask_eint),
    sensorFsmCmd(STATE_PS_GET_CONF, STATE_PS_POWER_OFF, cm36558_get_ps_conf),
    sensorFsmCmd(STATE_PS_POWER_OFF, STATE_PS_DISABLE_DONE, cm36558_set_ps_power_off),
    /* ps rate state */
    sensorFsmCmd(STATE_PS_RATECHG, CHIP_PS_RATECHG_DONE, cm36558_ps_ratechg),
    /* ps cali state */
    sensorFsmCmd(STATE_PS_CALI, CHIP_PS_CALI_DONE, cm36558_ps_cali),
    /* ps cfg state */
    sensorFsmCmd(STATE_PS_CFG, CHIP_PS_CFG_DONE, cm36558_ps_cfg),
    /* rgbw enable state */
    sensorFsmCmd(STATE_RGBW_ENABLE, STATE_RGBW_ENABLE_DONE, cm36558_rgbw_enable),
    /* rgbw disable state */
    sensorFsmCmd(STATE_RGBW_DISABLE, STATE_RGBW_DISABLE_DONE, cm36558_rgbw_disable),
    /* rgbw rate state */
    sensorFsmCmd(STATE_RGBW_RATECHG, CHIP_RGBW_RATECHG_DONE, cm36558_rgbw_ratechg),
    /* init state */
    sensorFsmCmd(STATE_RESET, STATE_ALS_THDH_W, cm36558_set_als_conf),
    sensorFsmCmd(STATE_ALS_THDH_W, STATE_ALS_THDL_W, cm36558_set_als_thdh),
    sensorFsmCmd(STATE_ALS_THDL_W, STATE_PS_CONF1_2_W, cm36558_set_als_thdl),
    sensorFsmCmd(STATE_PS_CONF1_2_W, STATE_PS_CONF3_MS_W, cm36558_set_ps_conf1_2),
    sensorFsmCmd(STATE_PS_CONF3_MS_W, STATE_PS_CANC_W, cm36558_set_ps_conf3_ms),
    sensorFsmCmd(STATE_PS_CANC_W, STATE_PS_THDH_W, cm36558_set_ps_canc),
    sensorFsmCmd(STATE_PS_THDH_W, STATE_PS_THDL_W, cm36558_set_ps_thdh),
    sensorFsmCmd(STATE_PS_THDL_W, STATE_SETUP_EINT, cm36558_set_ps_thdl),
    sensorFsmCmd(STATE_SETUP_EINT, STATE_CORE, cm36558_setup_eint),
    sensorFsmCmd(STATE_SETUP_EINT, STATE_INIT_DONE, cm36558_register_core),
};

static int cm36558Init(void)
{
    int ret = 0;
    cm36558DebugPoint = &mTask;

    mTask.hw = get_cust_alsps("cm36558");
    if (NULL == mTask.hw) {
        osLog(LOG_ERROR, "get_cust_acc_hw fail\n");
        return 0;
    }
    mTask.i2c_addr = mTask.hw->i2c_addr[0];
    mTask.als_debounce_time = 200;
    mTask.als_debounce_on = 0;
    mTask.ps_debounce_time = 200;
    mTask.ps_debounce_on = 0;
    mTask.psCali = 0;
    mTask.alsCali = 0;
    mTask.als_threshold_high = mTask.hw->als_threshold_high;
    mTask.als_threshold_low = mTask.hw->als_threshold_low;
    mTask.ps_threshold_high = mTask.hw->ps_threshold_high;
    mTask.ps_threshold_low = mTask.hw->ps_threshold_low;
    osLog(LOG_ERROR, "acc i2c_num: %d, i2c_addr: 0x%x\n", mTask.hw->i2c_num, mTask.i2c_addr);

    i2cMasterRequest(mTask.hw->i2c_num, I2C_SPEED);

    mTask.txBuf[0] = CM36558_REG_ID;

    for (uint8_t i = 0; i < 3; i++) {
        ret = i2cMasterTxRxSync(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
            &mTask.deviceId, 1, NULL, NULL);
        if (ret >= 0)
            break;
    }

    if (ret < 0) {
        ret = -1;
        sendSensorErrToAp(ERR_SENSOR_ALS_PS, ERR_CASE_ALS_PS_INIT, ALS_NAME);
        sendSensorErrToAp(ERR_SENSOR_ALS_PS, ERR_CASE_ALS_PS_INIT, PS_NAME);
        i2cMasterRelease(mTask.hw->i2c_num);
        goto err_out;
    }

    if (mTask.deviceId == 0x58) {
        osLog(LOG_INFO, "cm36558 auto detect success %x\n", mTask.deviceId);
        goto success_out;
    } else {
        ret = -1;
        sendSensorErrToAp(ERR_SENSOR_ALS_PS, ERR_CASE_ALS_PS_INIT, ALS_NAME);
        sendSensorErrToAp(ERR_SENSOR_ALS_PS, ERR_CASE_ALS_PS_INIT, PS_NAME);
        i2cMasterRelease(mTask.hw->i2c_num);
        goto err_out;
    }

success_out:
    alsSensorRegister();
    psSensorRegister();
    registerAlsPsDriverFsm(cm36558Fsm, ARRAY_SIZE(cm36558Fsm));
err_out:
    return ret;
}

#ifndef CFG_OVERLAY_INIT_SUPPORT
MODULE_DECLARE(cm36558, SENS_TYPE_ALS, cm36558Init);
#else
#include "mtk_overlay_init.h"
OVERLAY_DECLARE(cm36558, OVERLAY_ID_ALSPS, cm36558Init);
#endif
