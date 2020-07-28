/*
 * Copyright (C) 2017 The Android Open Source Project
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
#include <seos.h>
#include <util.h>
#include <sensors.h>
#include <plat/inc/rtc.h>
#include <contexthub_core.h>
#include <mt_gpt.h>
#include "eint.h"
#include "cm36652.h"
#include "alsps.h"

#define I2C_SPEED                       400000
#define ALS_NAME                        "cm36652_l"
#define PS_NAME                         "cm36652_p"

#define UNIT_MS_TO_NS   1000000

enum cm36652State {
    STATE_SAMPLE_ALS = CHIP_SAMPLING_ALS,
    STATE_SAMPLE_ALS_DONE = CHIP_SAMPLING_ALS_DONE,
    STATE_SAMPLE_PS = CHIP_SAMPLING_PS,
    STATE_SAMPLE_PS_DONE = CHIP_SAMPLING_PS_DONE,
    STATE_ALS_ENABLE = CHIP_ALS_ENABLE,
    STATE_ALS_ENABLE_DONE = CHIP_ALS_ENABLE_DONE,
    STATE_ALS_DISABLE = CHIP_ALS_DISABLE,
    STATE_ALS_DISABLE_DONE = CHIP_ALS_DISABLE_DONE,
    STATE_ALS_RATECHG = CHIP_ALS_RATECHG,
    STATE_ALS_RATECHG_DONE = CHIP_ALS_RATECHG_DONE,
    STATE_PS_ENABLE = CHIP_PS_ENABLE,
    STATE_PS_ENABLE_DONE = CHIP_PS_ENABLE_DONE,
    STATE_PS_DISABLE = CHIP_PS_DISABLE,
    STATE_PS_DISABLE_DONE = CHIP_PS_DISABLE_DONE,
    STATE_PS_RATECHG = CHIP_PS_RATECHG,
    STATE_PS_RATECHG_DONE = CHIP_PS_RATECHG_DONE,
    STATE_RGBW_ENABLE = CHIP_RGBW_ENABLE,
    STATE_RGBW_ENABLE_DONE = CHIP_RGBW_ENABLE_DONE,
    STATE_RGBW_DISABLE = CHIP_RGBW_DISABLE,
    STATE_RGBW_DISABLE_DONE = CHIP_RGBW_DISABLE_DONE,
    STATE_RGBW_RATECHG = CHIP_RGBW_RATECHG,
    STATE_RGBW_RATECHG_DONE = CHIP_RGBW_RATECHG_DONE,
    STATE_INIT_DONE = CHIP_INIT_DONE,
    STATE_IDLE = CHIP_IDLE,
    STATE_RESET = CHIP_RESET,

    STATE_GET_ALS_VAL,
    STATE_ALS_SET_DEBOUNCE,
    STATE_GET_PS_STAT,
    STATE_GET_PS_RAW_DATA,
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
    STATE_PS_THD,
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

SRAM_REGION_BSS static struct cm36652Task {
    /* txBuf for i2c operation, fill register and fill value */
    bool alsPowerOn;
    bool psPowerOn;
    unsigned int    als_debounce_time;
    unsigned int    als_debounce_on;    /*indicates if the debounce is on*/
    uint64_t    als_debounce_end;
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
    uint32_t    ps_threshold_high;
    uint32_t    ps_threshold_low;
    uint32_t    als_threshold_high;
    uint32_t    als_threshold_low;
    bool rgbw_enabled;
    bool als_enabled;
} mTask;
static struct cm36652Task *cm36652DebugPoint;

static int check_timeout(uint64_t *end_tick)
{
    return rtcGetTime() > *end_tick ? 1 : 0;
}

static void cal_end_time(unsigned int ms, uint64_t *end_tick)
{
    *end_tick = rtcGetTime() +
                ((uint64_t)ms * UNIT_MS_TO_NS);
}

static int cm36652_read_als(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                            void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    if (rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize)) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "als rx error\n");
        return -1;
    }
    if (mTask.rgbw_enabled) {
        mTask.txBuf[0] = CM36652_REG_CS_R_DATA;
        ret |= i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[0], 1,
                             &mTask.rxBuf[0], 2, i2cCallBack, (void *)STATE_IDLE);

        mTask.txBuf[0] = CM36652_REG_CS_G_DATA;
        ret |= i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[0], 1,
                             &mTask.rxBuf[2], 2, i2cCallBack, (void *)STATE_IDLE);

        mTask.txBuf[0] = CM36652_REG_CS_B_DATA;
        ret |= i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[0], 1,
                             &mTask.rxBuf[4], 2, i2cCallBack, (void *)STATE_IDLE);

        mTask.txBuf[0] = CM36652_REG_White_DATA;
        ret |= i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[0], 1,
                             &mTask.rxBuf[6], 2, i2cCallBack, next_state);
    } else {
        mTask.txBuf[0] = CM36652_REG_CS_G_DATA;
        ret = i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[0], 1,
                             &mTask.rxBuf[2], 2, i2cCallBack, next_state);
    }
    return ret;
}

static int getLuxFromAlsData(void)
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
        return value;
    } else {
        return -1;
    }
}

static int cm36652_get_als_value(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                 void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                 void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int als_data = 0;
    if (mTask.als_enabled) {
        mTask.als_raw_data = ((mTask.rxBuf[3] << 8) | mTask.rxBuf[2]);
        als_data = getLuxFromAlsData();  //aal mapping value
        if (als_data < 0)
            mTask.data[0].sensType = SENS_TYPE_INVALID; /*invalid value when debounce is on*/
        else {
            mTask.data[0].als_data = als_data;
            mTask.data[0].sensType = SENS_TYPE_ALS;
        }
    }
    if (mTask.rgbw_enabled) {
        mTask.data[1].r = ((mTask.rxBuf[1] << 8) | mTask.rxBuf[0]);
        mTask.data[1].g = ((mTask.rxBuf[3] << 8) | mTask.rxBuf[2]);
        mTask.data[1].b = ((mTask.rxBuf[5] << 8) | mTask.rxBuf[4]);
        mTask.data[1].w = ((mTask.rxBuf[7] << 8) | mTask.rxBuf[6]);
        mTask.data[1].sensType = SENS_TYPE_RGBW;
    }

    if (mTask.als_enabled && mTask.rgbw_enabled)
        txTransferDataInfo(&mTask.dataInfo, 2, &mTask.data[0]);
    else if (mTask.als_enabled)
        txTransferDataInfo(&mTask.dataInfo, 1, &mTask.data[0]);
    else
        txTransferDataInfo(&mTask.dataInfo, 1, &mTask.data[1]);

    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static void alsGetData(void *sample)
{
    struct SingleAxisDataPoint *singleSample = (struct SingleAxisDataPoint *)sample;
    singleSample->idata = getLuxFromAlsData();
}


static int cm36652_read_ps(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                           void *inBuf, uint8_t inSize, uint8_t elemInSize,
                           void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    if (rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize)) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "ps rx error\n");
        return -1;
    }
    mTask.txBuf[0] = CM36652_REG_INT_FLAG;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 2, i2cCallBack,
                         next_state);
}


static int cm36652_get_ps_status(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                 void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                 void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    if (mTask.rxBuf[1] & 0x1) {
        mTask.data[0].prox_state = PROX_STATE_FAR; /* far */
        mTask.data[0].prox_data = 1; /* far state distance is 1cm */
    } else if (mTask.rxBuf[1] & 0x2) {
        mTask.data[0].prox_state = PROX_STATE_NEAR; /* near */
        mTask.data[0].prox_data = 0; /* near state distance is 0cm */
    } else
        return -1;

    mTask.txBuf[0] = CM36652_REG_PS_DATA;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 2, i2cCallBack,
                         next_state);
}

static int cm36652_get_ps_raw_data(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
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
    txBuf[0] = CM36652_REG_PS_DATA;
    i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, txBuf, 1,
                  rxBuf, 2, get_ps_data, rxBuf);
}

void cm36652_ps_set_threshold(uint32_t threshold_high, uint32_t threshold_low)
{
    int ret;
    char txBuf[3];

    txBuf[0] = CM36652_REG_PS_THD;
    txBuf[1] = (unsigned char)threshold_low;
    txBuf[2] = (unsigned char)threshold_high;
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, txBuf, 3,
                      NULL, 0);
    if (ret) {
        osLog(LOG_INFO, "%s: ps thrd error (%d)\n", __func__, ret);
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
    cm36652_ps_set_threshold(mTask.ps_threshold_high + mTask.psCali,
                             mTask.ps_threshold_low + mTask.psCali);
}

static void psGetThreshold(uint32_t *threshold_high, uint32_t *threshold_low)
{
    *threshold_high = mTask.ps_threshold_high;
    *threshold_low = mTask.ps_threshold_low;
}

static void psSetThreshold(uint32_t threshold_high, uint32_t threshold_low)
{
    osLog(LOG_ERROR, "%s threshold_high:%d threshold_low:%d \n", __func__, threshold_high, threshold_low);
    mTask.ps_threshold_high = threshold_high;
    mTask.ps_threshold_low = threshold_low;
    cm36652_ps_set_threshold(mTask.ps_threshold_high, mTask.ps_threshold_low);
}

static void psGetSensorInfo(struct sensorInfo_t *data)
{
    strncpy(data->name, PS_NAME, sizeof(data->name));
}

static void alsGetSensorInfo(struct sensorInfo_t *data)
{
    strncpy(data->name, ALS_NAME, sizeof(data->name));
}


static int cm36652_set_als_debounce_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                       void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                       void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.als_enabled = true;
    mTask.als_debounce_on = 1;
    cal_end_time(mTask.als_debounce_time, &mTask.als_debounce_end);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int cm36652_set_ps_debounce_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                      void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                      void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.ps_debounce_on = 1;
    cal_end_time(mTask.ps_debounce_time, &mTask.ps_debounce_end);
    mTask.data[0].prox_state = PROX_STATE_INIT;
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int cm36652_set_als_power_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                    void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.txBuf[0] = CM36652_REG_CS_CONF;
    mTask.txBuf[1] = mTask.rxBuf[0] & 0xFE;
    mTask.txBuf[2] = mTask.rxBuf[1];
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3,
                       i2cCallBack, next_state);
}

static int cm36652_set_als_power_off(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                     void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                     void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.als_enabled = false;
    if (mTask.rgbw_enabled == true) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
        return 0;
    }
    mTask.als_debounce_on = 0;
    mTask.txBuf[0] = CM36652_REG_CS_CONF;
    mTask.txBuf[1] = mTask.rxBuf[0] | 0x01;
    mTask.txBuf[2] = mTask.rxBuf[1];
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3,
                       i2cCallBack, next_state);
}

static int cm36652_als_ratechg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                               void *inBuf, uint8_t inSize, uint8_t elemInSize,
                               void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int cm36652_mask_eint(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mt_eint_mask(mTask.hw->eint_num);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

/*  Need add unmask operation to ratechg, if sensor has setting rate */
static int cm36652_unmask_eint(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mt_eint_unmask(mTask.hw->eint_num);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int cm36652_set_ps_power_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                   void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                   void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.txBuf[0] = CM36652_REG_PS_CONF1_2;
    mTask.txBuf[1] = mTask.rxBuf[0] & 0xFE;
    mTask.txBuf[2] = mTask.rxBuf[1];
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3,
                       i2cCallBack, next_state);
}

static int cm36652_set_ps_power_off(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                    void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.ps_debounce_on = 0;
    mTask.data[0].prox_state = PROX_STATE_FAR; /* default value */
    mTask.txBuf[0] = CM36652_REG_PS_CONF1_2;
    mTask.txBuf[1] = mTask.rxBuf[0] | 0x01;
    mTask.txBuf[2] = mTask.rxBuf[1];
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3,
                       i2cCallBack, next_state);
}

static int cm36652_ps_ratechg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int cm36652_get_ps_conf(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                               void *inBuf, uint8_t inSize, uint8_t elemInSize,
                               void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.txBuf[0] = CM36652_REG_PS_CONF1_2;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 2, i2cCallBack,
                         next_state);
}

static int cm36652_get_als_conf(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.txBuf[0] = CM36652_REG_CS_CONF;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 2, i2cCallBack,
                         next_state);
}

static int cm36652_set_als_conf(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.txBuf[0] = CM36652_REG_CS_CONF;
    if (1 == mTask.hw->polling_mode_als)
        mTask.txBuf[1] = 0x11;
    else
        mTask.txBuf[1] = 0x17;

    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int cm36652_set_als_thdh(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //use polling
    mTask.txBuf[0] = CM36652_REG_ALS_THDH;
    if (!mTask.hw->polling_mode_als) {
        mTask.txBuf[1] = 0x00;
        mTask.txBuf[2] = mTask.als_threshold_high;
    } else {
        mTask.txBuf[1] = (unsigned char)(mTask.als_threshold_high & 0xFF);
        mTask.txBuf[2] = (unsigned char)(mTask.als_threshold_high >> 8);
    }

    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3,
                       i2cCallBack, next_state);
}

static int cm36652_set_als_thdl(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //use polling
    mTask.txBuf[0] = CM36652_REG_ALS_THDL;
    if (!mTask.hw->polling_mode_als) {
        mTask.txBuf[1] = 0x00;
        mTask.txBuf[2] = mTask.als_threshold_low;
    } else {
        mTask.txBuf[1] = (unsigned char)(mTask.als_threshold_low & 0xFF);
        mTask.txBuf[2] = (unsigned char)(mTask.als_threshold_low >> 8);
    }

    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3,
                       i2cCallBack, next_state);
}

static int cm36652_set_ps_conf1_2(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                  void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                  void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.txBuf[0] = CM36652_REG_PS_CONF1_2;
    mTask.txBuf[1] = 0x19;
    /* use interrupt */
    mTask.txBuf[2] = 0x62;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3,
                       i2cCallBack, next_state);
}

static int cm36652_set_ps_canc(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                               void *inBuf, uint8_t inSize, uint8_t elemInSize,
                               void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.txBuf[0] = CM36652_REG_PS_CANC;
    mTask.txBuf[1] = 0x00;

    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int cm36652_set_ps_thd(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                               void *inBuf, uint8_t inSize, uint8_t elemInSize,
                               void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.txBuf[0] = CM36652_REG_PS_THD;
    mTask.txBuf[1] = (unsigned char)(mTask.ps_threshold_low + mTask.psCali);
    mTask.txBuf[2] = (unsigned char)(mTask.ps_threshold_high + mTask.psCali);

    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3,
                       i2cCallBack, next_state);
}

static int cm36652_rgbw_enable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                               void *inBuf, uint8_t inSize, uint8_t elemInSize,
                               void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.rgbw_enabled = true;
    mTask.als_debounce_on = 1;
    cal_end_time(mTask.als_debounce_time, &mTask.als_debounce_end);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int cm36652_rgbw_disable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                               void *inBuf, uint8_t inSize, uint8_t elemInSize,
                               void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.rgbw_enabled = false;
    if (mTask.als_enabled == true) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
        return 0;
    }
    mTask.als_debounce_on = 0;
    mTask.txBuf[0] = CM36652_REG_CS_CONF;
    mTask.txBuf[1] = mTask.rxBuf[0] | 0x01;
    mTask.txBuf[2] = mTask.rxBuf[1];
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3,
                       i2cCallBack, next_state);
}

static int cm36652_rgbw_ratechg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                               void *inBuf, uint8_t inSize, uint8_t elemInSize,
                               void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

SRAM_REGION_FUNCTION void cm36652_eint_handler(int arg)
{
    alsPsInterruptOccur();
}

static int cm36652_setup_eint(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mt_eint_dis_hw_debounce(mTask.hw->eint_num); /* MT6797 use 11 */
    mt_eint_registration(mTask.hw->eint_num, LEVEL_SENSITIVE, LOW_LEVEL_TRIGGER, cm36652_eint_handler, EINT_INT_UNMASK,
                         EINT_INT_AUTO_UNMASK_OFF); /* MT6797 use 11 */
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}


static int cm36652_register_core(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                 void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                 void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    struct sensorCoreInfo mInfo;
    int ret = 0;

    /* Register sensor Core */
    mInfo.sensType = SENS_TYPE_ALS;
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

static struct sensorFsm cm36652Fsm[] = {
    sensorFsmCmd(STATE_SAMPLE_ALS, STATE_GET_ALS_VAL, cm36652_read_als),
    sensorFsmCmd(STATE_GET_ALS_VAL, STATE_SAMPLE_ALS_DONE, cm36652_get_als_value),

    sensorFsmCmd(STATE_SAMPLE_PS, STATE_GET_PS_STAT, cm36652_read_ps),
    sensorFsmCmd(STATE_GET_PS_STAT, STATE_GET_PS_RAW_DATA, cm36652_get_ps_status),
    sensorFsmCmd(STATE_GET_PS_RAW_DATA, STATE_SAMPLE_PS_DONE, cm36652_get_ps_raw_data),
    /* als enable state */
    sensorFsmCmd(STATE_ALS_ENABLE, STATE_ALS_POWER_ON, cm36652_get_als_conf),
    sensorFsmCmd(STATE_ALS_POWER_ON, STATE_ALS_SET_DEBOUNCE, cm36652_set_als_power_on),
    sensorFsmCmd(STATE_ALS_SET_DEBOUNCE, STATE_ALS_ENABLE_DONE, cm36652_set_als_debounce_on),
    /* als disable state */
    sensorFsmCmd(STATE_ALS_DISABLE, STATE_ALS_POWER_OFF, cm36652_get_als_conf),
    sensorFsmCmd(STATE_ALS_POWER_OFF, STATE_ALS_DISABLE_DONE, cm36652_set_als_power_off),
    /* als rate state */
    sensorFsmCmd(STATE_ALS_RATECHG, CHIP_ALS_RATECHG_DONE, cm36652_als_ratechg),
    /* ps enable state */
    sensorFsmCmd(STATE_PS_ENABLE, STATE_PS_POWER_ON, cm36652_get_ps_conf),
    sensorFsmCmd(STATE_PS_POWER_ON, STATE_PS_SET_DEBOUNCE, cm36652_set_ps_power_on),
    sensorFsmCmd(STATE_PS_SET_DEBOUNCE, STATE_PS_UNMASK_EINT, cm36652_set_ps_debounce_on),
    sensorFsmCmd(STATE_PS_UNMASK_EINT, STATE_PS_ENABLE_DONE, cm36652_unmask_eint),
    /* ps disable state */
    sensorFsmCmd(STATE_PS_DISABLE, STATE_PS_GET_CONF, cm36652_mask_eint),
    sensorFsmCmd(STATE_PS_GET_CONF, STATE_PS_POWER_OFF, cm36652_get_ps_conf),
    sensorFsmCmd(STATE_PS_POWER_OFF, STATE_PS_DISABLE_DONE, cm36652_set_ps_power_off),
    /* ps rate state */
    sensorFsmCmd(STATE_PS_RATECHG, CHIP_PS_RATECHG_DONE, cm36652_ps_ratechg),
    // rgbw enable
    sensorFsmCmd(STATE_RGBW_ENABLE, STATE_ALS_POWER_ON, cm36652_get_als_conf),
    sensorFsmCmd(STATE_ALS_POWER_ON, STATE_ALS_SET_DEBOUNCE, cm36652_set_als_power_on),
    sensorFsmCmd(STATE_ALS_SET_DEBOUNCE, STATE_RGBW_ENABLE_DONE, cm36652_rgbw_enable),
    // rgbw disable
    sensorFsmCmd(STATE_RGBW_DISABLE, STATE_ALS_POWER_OFF, cm36652_get_als_conf),
    sensorFsmCmd(STATE_ALS_POWER_OFF, STATE_RGBW_DISABLE_DONE, cm36652_rgbw_disable),
    /* rgbw rate state */
    sensorFsmCmd(STATE_RGBW_RATECHG, CHIP_RGBW_RATECHG_DONE, cm36652_rgbw_ratechg),
    /* init state */
    sensorFsmCmd(STATE_RESET, STATE_ALS_THDH_W, cm36652_set_als_conf),
    sensorFsmCmd(STATE_ALS_THDH_W, STATE_ALS_THDL_W, cm36652_set_als_thdh),
    sensorFsmCmd(STATE_ALS_THDL_W, STATE_PS_CONF1_2_W, cm36652_set_als_thdl),
    sensorFsmCmd(STATE_PS_CONF1_2_W, STATE_PS_CANC_W, cm36652_set_ps_conf1_2),
    sensorFsmCmd(STATE_PS_CANC_W, STATE_PS_THD, cm36652_set_ps_canc),
    sensorFsmCmd(STATE_PS_THD, STATE_SETUP_EINT, cm36652_set_ps_thd),
    sensorFsmCmd(STATE_SETUP_EINT, STATE_CORE, cm36652_setup_eint),
    sensorFsmCmd(STATE_SETUP_EINT, STATE_INIT_DONE, cm36652_register_core),
};

static int cm36652Init(void)
{
    int ret = 0;
    osLog(LOG_INFO, "%s: task starting\n", __func__);
    cm36652DebugPoint = &mTask;

    mTask.hw = get_cust_alsps("cm36652");
    if (NULL == mTask.hw) {
        osLog(LOG_ERROR, "get_cust_rgbw_hw fail\n");
        return 0;
    }
    mTask.i2c_addr = mTask.hw->i2c_addr[0];
    mTask.als_debounce_time = 200;
    mTask.als_debounce_on = 0;
    mTask.ps_debounce_time = 200;
    mTask.ps_debounce_on = 0;
    mTask.psCali = 0;
    mTask.als_threshold_high = mTask.hw->als_threshold_high;
    mTask.als_threshold_low = mTask.hw->als_threshold_low;
    mTask.ps_threshold_high = mTask.hw->ps_threshold_high;
    mTask.ps_threshold_low = mTask.hw->ps_threshold_low;
    osLog(LOG_ERROR, "rgbw i2c_num: %d, i2c_addr: 0x%x\n", mTask.hw->i2c_num, mTask.i2c_addr);

    i2cMasterRequest(mTask.hw->i2c_num, I2C_SPEED);

    mTask.txBuf[0] = CM36652_REG_ID;

    ret = i2cMasterTxRxSync(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
        &mTask.deviceId, 2, NULL, NULL);
    if (ret < 0) {
        osLog(LOG_ERROR, "%s: i2cMasterTxRxSync fail!!!\n", __func__);
        ret = -1;
        i2cMasterRelease(mTask.hw->i2c_num);
        goto err_out;
    }

    osLog(LOG_INFO, "cm36652: auto detect success:0x%x\n", mTask.deviceId);
    alsSensorRegister();
    psSensorRegister();
    rgbwSensorRegister();
    registerAlsPsDriverFsm(cm36652Fsm, ARRAY_SIZE(cm36652Fsm));
err_out:
    return ret;
}
#ifndef CFG_OVERLAY_INIT_SUPPORT
MODULE_DECLARE(cm36652, SENS_TYPE_ALS, cm36652Init);
#else
#include "mtk_overlay_init.h"
OVERLAY_DECLARE(cm36652, OVERLAY_ID_ALSPS, cm36652Init);
#endif
