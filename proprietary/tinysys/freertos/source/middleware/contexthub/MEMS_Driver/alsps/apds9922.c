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
#include <seos.h>
#include <util.h>
#include <sensors.h>
#include <plat/inc/rtc.h>
#include <contexthub_core.h>
#include <mt_gpt.h>
#include <timer.h>
#include "eint.h"
#include "apds9922.h"
#include "alsps.h"

enum apds9922State {
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
    STATE_INIT_DONE = CHIP_INIT_DONE,
    STATE_IDEL = CHIP_IDLE,
    STATE_RESET = CHIP_RESET,

    STATE_GET_ALS_GAIN,
    STATE_GET_ALS_DATA,
    STATE_GET_ALS_MEAS_RATE,
    STATE_ALS_SET_DEBOUNCE,
    STATE_GET_PS_STAT,
    STATE_GET_PS_RAW_DATA,
    STATE_PS_SET_DEBOUNCE,
    STATE_SET_MAIN_CTRL,
    STATE_SET_PS_LED,
    STATE_ALS_POWER_ON,
    STATE_ALS_POWER_OFF,
    STATE_SET_PS_PULSES,
    STATE_SET_PS_MEAS_RATE,
    STATE_SET_ALS_MEAS_RATE,
    STATE_SET_ALS_GAIN,
    STATE_SET_INT_PERSIST,
    STATE_SET_INT_CFG,
    STATE_PS_POWER_ON,
    STATE_PS_POWER_OFF,
    STATE_SET_THDL,
    STATE_SET_THDH,
    STATE_PS_UNMASK_EINT,
    STATE_PS_GET_CONF,
    STATE_SETUP_EINT,
    STATE_CORE
};

#define I2C_SPEED                       400000
#define ALS_NAME                        "apds9922_l"
#define PS_NAME                         "apds9922_p"

#define MAX_RXBUF 8
#define MAX_TXBUF 8

#define UNIT_MS_TO_NS   1000000
struct als_reg {
    unsigned int als_value;
    uint16_t als_time;
    uint16_t als_gain;
};

SRAM_REGION_BSS static struct apds9922Task {
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
    struct als_reg als_reg_val;
    int32_t psCali;
    uint32_t    als_raw_data;
    uint32_t    prox_raw_data;
    uint32_t    ps_threshold_high;
    uint32_t    ps_threshold_low;
    uint32_t    als_threshold_high;
    uint32_t    als_threshold_low;
} mTask;

static struct apds9922Task *apds9922DebugPoint;

static int check_timeout(uint64_t *end_tick) {
    return rtcGetTime() > *end_tick ? 1 : 0;
}

static void cal_end_time(unsigned int ms, uint64_t *end_tick) {
    *end_tick = rtcGetTime() +
                ((uint64_t)ms * UNIT_MS_TO_NS);
}

static int apds9922_read_als_gain(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    if (rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize)) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "apds9922 als, rx dataInfo error\n");
        return -1;
    }
    mTask.txBuf[0] = APDS9922_DD_ALS_GAIN_ADDR;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,
                         next_state);
}

static int apds9922_read_als_meas_rate(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                     void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                     void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.als_reg_val.als_gain = mTask.rxBuf[0] & 0x07;

    mTask.txBuf[0] = APDS9922_DD_ALS_MEAS_RATE_ADDR;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,
                         next_state);
}

static int apds9922_read_als_data(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.als_reg_val.als_time = (mTask.rxBuf[0] >> 4) & 0x07;

    mTask.txBuf[0] = APDS9922_DD_ALS_DATA_ADDR;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 3, i2cCallBack,
                         next_state);
}

static int getLuxFromAlsData(void) {
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
    }

    return -1;
}

static int apds9922_read_als(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                           void *inBuf, uint8_t inSize, uint8_t elemInSize,
                           void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    int als_data = 0;
    mTask.als_reg_val.als_value = mTask.rxBuf[0] | (mTask.rxBuf[1] << 8) | (mTask.rxBuf[2] << 16);

    mTask.als_raw_data = (mTask.als_reg_val.als_value * APDS9922_DD_LUX_FACTOR) / (mTask.als_reg_val.als_gain *
                              mTask.als_reg_val.als_time);
    als_data = getLuxFromAlsData();
    if (als_data < 0)
        mTask.data[0].sensType = SENS_TYPE_INVALID;
    else {
        mTask.data[0].als_data = als_data;
        mTask.data[0].sensType = SENS_TYPE_ALS;
    }
    txTransferDataInfo(&mTask.dataInfo, 1, &mTask.data[0]);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static void alsGetData(void *sample) {
    struct SingleAxisDataPoint *singleSample = (struct SingleAxisDataPoint *)sample;
    singleSample->idata = getLuxFromAlsData();
}

static int apds9922_read_ps(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                          void *inBuf, uint8_t inSize, uint8_t elemInSize,
                          void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    if (rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize)) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "apds9922 ps, rx dataInfo error\n");
        return -1;
    }
    mTask.txBuf[0] = APDS9922_DD_MAIN_STATUS_ADDR;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,
                         next_state);
}

static int apds9922_get_ps_status(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    int ps_flag = (mTask.rxBuf[0] & 0x04) >> 2;
    osLog(LOG_ERROR, "prox ===========================> intr occur, is %s\n", ps_flag == 0 ? "far" : "near");
    if (ps_flag == 0) { /* 0 is far */
        mTask.data[0].prox_state = PROX_STATE_FAR; /* far */
        mTask.data[0].prox_data = 1; /* far state distance is 1cm */
    } else if (ps_flag == 1) {
        mTask.data[0].prox_state = PROX_STATE_NEAR; /* near */
        mTask.data[0].prox_data = 0; /* near state distance is 0cm */
    } else
        return -1;

    mTask.txBuf[0] = APDS9922_DD_PRX_DATA_ADDR;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 2, i2cCallBack,
                         next_state);
}

static int apds9922_get_ps_raw_data(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                  void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                  void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.prox_raw_data = ((mTask.rxBuf[1] << 8) & 0x7ff) | mTask.rxBuf[0];
    if (mTask.prox_raw_data < mTask.psCali)
        mTask.prox_raw_data = 0;
    mTask.prox_raw_data -= mTask.psCali;
    osLog(LOG_ERROR, "%s =====> ps_raw_data:%ld\n", __func__, mTask.prox_raw_data);
    mt_eint_unmask(mTask.hw->eint_num);

    mTask.data[0].sensType = SENS_TYPE_PROX;
    txTransferDataInfo(&mTask.dataInfo, 1, &mTask.data[0]);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static void psGetData(void *sample) {
    char txBuf[1];
    static char rxBuf[2];
    struct TripleAxisDataPoint *tripleSample = (struct TripleAxisDataPoint *)sample;
    tripleSample->ix = mTask.prox_raw_data;
    tripleSample->iy = (mTask.data[0].prox_state == PROX_STATE_NEAR ? 0 : 1);

    osLog(LOG_ERROR, "%s ==> ps_raw_data:%ld\n", __func__, mTask.prox_raw_data);
    void get_ps_data(void *cookie, size_t tx, size_t rx, int err) {
        char *rxBuf = cookie;
        if (err == 0) {
            mTask.prox_raw_data = ((rxBuf[1] << 8) & 0x7ff) | rxBuf[0];
            if (mTask.prox_raw_data < mTask.psCali)
                mTask.prox_raw_data = 0;
            mTask.prox_raw_data -= mTask.psCali;
        } else
            osLog(LOG_INFO, "apds9922: read ps data i2c error (%d)\n", err);
    }
    txBuf[0] = APDS9922_DD_PRX_DATA_ADDR;
    i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, txBuf, 1,
                  rxBuf, 2, get_ps_data, rxBuf);
}

static void apds9922_ps_set_threshold(uint32_t threshold_high, uint32_t threshold_low) {
    int ret;
    char txBuf[6];

    txBuf[0] = APDS9922_DD_PRX_THRES_UP_0_ADDR;
    txBuf[1] = (uint8_t)(threshold_high & 0xFF);
    txBuf[2] = (uint8_t)((threshold_high & 0xFF00) >> 8);
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, txBuf, 3,
                      NULL, 0);
    if (ret) {
        osLog(LOG_INFO, "apds9922: set ps threshold_high i2c error (%d)\n", ret);
        return;
    }
    txBuf[3] = APDS9922_DD_PRX_THRES_LOW_0_ADDR;
    txBuf[4] = (uint8_t)(threshold_low & 0xFF);
    txBuf[5] = (uint8_t)((threshold_low & 0xFF00) >> 8);
    i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, &txBuf[3], 3,
                NULL, 0);
    if (ret) {
        osLog(LOG_INFO, "apds9922: set ps threshold_low i2c error (%d)\n", ret);
        return;
    }
}

static void psGetCalibration(int32_t *cali, int32_t size) {
    cali[0] = mTask.psCali;
}

static void psSetCalibration(int32_t *cali, int32_t size) {
    mTask.psCali = cali[0];
    apds9922_ps_set_threshold(mTask.ps_threshold_high + mTask.psCali,
                            mTask.ps_threshold_low + mTask.psCali);
}

static void psGetThreshold(uint32_t *threshold_high, uint32_t *threshold_low) {
    *threshold_high = mTask.ps_threshold_high;
    *threshold_low = mTask.ps_threshold_low;
    osLog(LOG_ERROR, "%s ==>threshold_high:%ld threshold_low:%ld \n", __func__, *threshold_high, *threshold_low);
}

static void psSetThreshold(uint32_t threshold_high, uint32_t threshold_low) {
    osLog(LOG_ERROR, "%s ==>threshold_high:%ld threshold_low:%ld \n", __func__, threshold_high, threshold_low);
    mTask.ps_threshold_high = threshold_high;
    mTask.ps_threshold_low = threshold_low;
    apds9922_ps_set_threshold(mTask.ps_threshold_high, mTask.ps_threshold_low);
}

static void psGetSensorInfo(struct sensorInfo_t *data)
{
    strncpy(data->name, PS_NAME, sizeof(data->name));
}

static void alsGetSensorInfo(struct sensorInfo_t *data)
{
    strncpy(data->name, ALS_NAME, sizeof(data->name));
}

static int apds9922_mask_eint(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mt_eint_mask(mTask.hw->eint_num);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int apds9922_unmask_eint(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mt_eint_unmask(mTask.hw->eint_num);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int apds9922_set_als_debounce_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                      void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                      void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.als_debounce_on = 1;
    cal_end_time(mTask.als_debounce_time, &mTask.als_debounce_end);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int apds9922_set_ps_debounce_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                     void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                     void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.ps_debounce_on = 1;
    cal_end_time(mTask.ps_debounce_time, &mTask.ps_debounce_end);
    mTask.data[0].prox_state = PROX_STATE_INIT;
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int apds9922_set_als_power_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                   void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                   void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.txBuf[0] = APDS9922_DD_MAIN_CTRL_ADDR;
    mTask.txBuf[1] = mTask.rxBuf[0] | APDS9922_DD_ALS_EN;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int apds9922_set_als_power_off(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                    void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.als_debounce_on = 0;
    mTask.txBuf[0] = APDS9922_DD_MAIN_CTRL_ADDR;
    mTask.txBuf[1] = mTask.rxBuf[0] & ~APDS9922_DD_ALS_EN;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int apds9922_als_ratechg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int apds9922_set_ps_power_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                  void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                  void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.txBuf[0] = APDS9922_DD_MAIN_CTRL_ADDR;
    mTask.txBuf[1] = mTask.rxBuf[0] | APDS9922_DD_PRX_EN;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int apds9922_set_ps_power_off(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                   void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                   void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.ps_debounce_on = 0;
    mTask.data[0].prox_state = PROX_STATE_FAR;
    mTask.txBuf[0] = APDS9922_DD_MAIN_CTRL_ADDR;
    mTask.txBuf[1] = mTask.rxBuf[0] & ~APDS9922_DD_PRX_EN;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int apds9922_ps_ratechg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int apds9922_get_ps_conf(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.txBuf[0] = APDS9922_DD_MAIN_CTRL_ADDR;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,
                         next_state);
}

static int apds9922_get_als_conf(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                               void *inBuf, uint8_t inSize, uint8_t elemInSize,
                               void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.txBuf[0] = APDS9922_DD_MAIN_CTRL_ADDR;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,
                         next_state);
}

static int apds9922_set_main_ctrl(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.txBuf[0] = APDS9922_DD_MAIN_CTRL_ADDR;
    mTask.txBuf[1] = 0;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int apds9922_set_ps_led(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.txBuf[0] = APDS9922_DD_PRX_LED_ADDR;
    mTask.txBuf[1] = APDS9922_DD_PRX_DEFAULT_LED_FREQ | APDS9922_DD_PRX_DEFAULT_LED_CURRENT;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int apds9922_set_ps_pulses(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.txBuf[0] = APDS9922_DD_PRX_PULSES_ADDR;
    mTask.txBuf[1] = APDS9922_DD_PRX_DEFAULT_PULSE;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int apds9922_set_ps_meas_rate(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                   void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                   void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.txBuf[0] = APDS9922_DD_PRX_MEAS_RATE_ADDR;
    mTask.txBuf[1] = 0x40 | APDS9922_DD_PRX_DEFAULT_RES | APDS9922_DD_PRX_DEFAULT_MEAS_RATE;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int apds9922_set_als_meas_rate(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                    void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.txBuf[0] = APDS9922_DD_ALS_MEAS_RATE_ADDR;
    mTask.txBuf[1] = APDS9922_DD_ALS_DEFAULT_RES | APDS9922_DD_ALS_DEFAULT_MEAS_RATE;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int apds9922_set_als_gain(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                               void *inBuf, uint8_t inSize, uint8_t elemInSize,
                               void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.txBuf[0] = APDS9922_DD_ALS_GAIN_ADDR;
    mTask.txBuf[1] = APDS9922_DD_ALS_DEFAULT_GAIN;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int apds9922_set_int_persist(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                  void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                  void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.txBuf[0] = APDS9922_DD_INT_PERSISTENCE_ADDR;
    mTask.txBuf[1] = APDS9922_DD_PRX_PERS_4;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int apds9922_set_int_cfg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.txBuf[0] = APDS9922_DD_INT_CFG_ADDR;
    if (1 == mTask.hw->polling_mode_ps)
        mTask.txBuf[1] = 0x10;
    else
        mTask.txBuf[1] = 0x11;

    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int apds9922_set_ps_thdl(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    if (mTask.hw->polling_mode_ps == 0) {
        mTask.txBuf[0] = APDS9922_DD_PRX_THRES_LOW_0_ADDR;
        mTask.txBuf[1] = (uint8_t)((mTask.ps_threshold_low + mTask.psCali) & 0x00FF);
        mTask.txBuf[2] = (uint8_t)(((mTask.ps_threshold_low + mTask.psCali) & 0xFF00) >> 8);
        return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3,
                           i2cCallBack, next_state);
    } else {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
        return 0;
    }
}

static int apds9922_set_ps_thdh(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    if (mTask.hw->polling_mode_ps == 0) {
        mTask.txBuf[0] = APDS9922_DD_PRX_THRES_UP_0_ADDR;
        mTask.txBuf[1] = (uint8_t)((mTask.ps_threshold_high + mTask.psCali) & 0x00FF);
        mTask.txBuf[2] = (uint8_t)(((mTask.ps_threshold_high + mTask.psCali) & 0xFF00) >> 8);

        return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3,
                           i2cCallBack, next_state);
    } else {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
        return 0;
    }
}

SRAM_REGION_FUNCTION static void apds9922_eint_handler(int arg) {
    alsPsInterruptOccur();
}

static int apds9922_setup_eint(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mt_eint_dis_hw_debounce(mTask.hw->eint_num);
    mt_eint_registration(mTask.hw->eint_num, LEVEL_SENSITIVE, LOW_LEVEL_TRIGGER, apds9922_eint_handler, EINT_INT_UNMASK,
                         EINT_INT_AUTO_UNMASK_OFF);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int apds9922_register_core(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
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

static struct sensorFsm apds9922Fsm[] = {
    /* sample als */
    sensorFsmCmd(STATE_SAMPLE_ALS, STATE_GET_ALS_GAIN, apds9922_read_als_gain),
    sensorFsmCmd(STATE_GET_ALS_GAIN, STATE_GET_ALS_MEAS_RATE, apds9922_read_als_meas_rate),
    sensorFsmCmd(STATE_GET_ALS_MEAS_RATE, STATE_GET_ALS_DATA, apds9922_read_als_data),
    sensorFsmCmd(STATE_GET_ALS_DATA, STATE_SAMPLE_ALS_DONE, apds9922_read_als),
    /* sample ps */
    sensorFsmCmd(STATE_SAMPLE_PS, STATE_GET_PS_STAT, apds9922_read_ps),
    sensorFsmCmd(STATE_GET_PS_STAT, STATE_GET_PS_RAW_DATA, apds9922_get_ps_status),
    sensorFsmCmd(STATE_GET_PS_RAW_DATA, STATE_SAMPLE_PS_DONE, apds9922_get_ps_raw_data),
    /* als enable state */
    sensorFsmCmd(STATE_ALS_ENABLE, STATE_ALS_POWER_ON, apds9922_get_als_conf),
    sensorFsmCmd(STATE_ALS_POWER_ON, STATE_ALS_SET_DEBOUNCE, apds9922_set_als_power_on),
    sensorFsmCmd(STATE_ALS_SET_DEBOUNCE, STATE_ALS_ENABLE_DONE, apds9922_set_als_debounce_on),
    /* als disable state */
    sensorFsmCmd(STATE_ALS_DISABLE, STATE_ALS_POWER_OFF, apds9922_get_als_conf),
    sensorFsmCmd(STATE_ALS_POWER_OFF, STATE_ALS_DISABLE_DONE, apds9922_set_als_power_off),
    /* ps enable state */
    sensorFsmCmd(STATE_PS_ENABLE, STATE_PS_POWER_ON, apds9922_get_ps_conf),
    sensorFsmCmd(STATE_PS_POWER_ON, STATE_PS_SET_DEBOUNCE, apds9922_set_ps_power_on),
    sensorFsmCmd(STATE_PS_SET_DEBOUNCE, STATE_PS_UNMASK_EINT, apds9922_set_ps_debounce_on),
    sensorFsmCmd(STATE_PS_UNMASK_EINT, STATE_PS_ENABLE_DONE, apds9922_unmask_eint),
    /* ps disable state */
    sensorFsmCmd(STATE_PS_DISABLE, STATE_PS_GET_CONF, apds9922_mask_eint),
    sensorFsmCmd(STATE_PS_GET_CONF, STATE_PS_POWER_OFF, apds9922_get_ps_conf),
    sensorFsmCmd(STATE_PS_POWER_OFF, STATE_PS_DISABLE_DONE, apds9922_set_ps_power_off),
    /* als rate change */
    sensorFsmCmd(STATE_ALS_RATECHG, STATE_ALS_RATECHG_DONE, apds9922_als_ratechg),
    /* ps rate change */
    sensorFsmCmd(STATE_PS_RATECHG, STATE_PS_RATECHG_DONE, apds9922_ps_ratechg),
    /* init state */
    sensorFsmCmd(STATE_RESET, STATE_SET_MAIN_CTRL, apds9922_set_main_ctrl),
    sensorFsmCmd(STATE_SET_MAIN_CTRL, STATE_SET_PS_LED, apds9922_set_ps_led),
    sensorFsmCmd(STATE_SET_PS_LED, STATE_SET_PS_PULSES, apds9922_set_ps_pulses),
    sensorFsmCmd(STATE_SET_PS_PULSES, STATE_SET_PS_MEAS_RATE, apds9922_set_ps_meas_rate),
    sensorFsmCmd(STATE_SET_PS_MEAS_RATE, STATE_SET_ALS_MEAS_RATE, apds9922_set_als_meas_rate),
    sensorFsmCmd(STATE_SET_ALS_MEAS_RATE, STATE_SET_ALS_GAIN, apds9922_set_als_gain),
    sensorFsmCmd(STATE_SET_ALS_GAIN, STATE_SET_INT_PERSIST, apds9922_set_int_persist),
    sensorFsmCmd(STATE_SET_INT_PERSIST, STATE_SET_INT_CFG, apds9922_set_int_cfg),
    sensorFsmCmd(STATE_SET_INT_CFG, STATE_SET_THDL, apds9922_set_ps_thdl),
    sensorFsmCmd(STATE_SET_THDL, STATE_SET_THDH, apds9922_set_ps_thdh),
    sensorFsmCmd(STATE_SET_THDH, STATE_SETUP_EINT, apds9922_setup_eint),
    sensorFsmCmd(STATE_SETUP_EINT, STATE_INIT_DONE, apds9922_register_core),
};

static int apds9922Init(void) {
    int ret = 0;

    osLog(LOG_INFO, "%s: task starting\n", __func__);
    apds9922DebugPoint = &mTask;

    mTask.hw = get_cust_alsps("apds9922");
    if (NULL == mTask.hw) {
        osLog(LOG_ERROR, "get_cust_alsps_hw fail\n");
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
    osLog(LOG_ERROR, "alsps i2c_num: %d, i2c_addr: 0x%x\n", mTask.hw->i2c_num, mTask.i2c_addr);

    i2cMasterRequest(mTask.hw->i2c_num, I2C_SPEED);

    mTask.txBuf[0] = APDS9922_DD_PART_ID_ADDR;

    ret = i2cMasterTxRxSync(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                            &mTask.deviceId, 1, NULL, NULL);
    if (ret < 0) {
        osLog(LOG_ERROR, "apds9922 i2cMasterTxRxSync fail!!!\n");
        ret = -1;
        i2cMasterRelease(mTask.hw->i2c_num);
        goto err_out;
    }
    if ((mTask.deviceId & 0xF0) == 0xB0) {  // apdb9922 part id[7:4] is fixed
        goto success_out;
    } else {
        i2cMasterRelease(mTask.hw->i2c_num);
        osLog(LOG_ERROR, "apds9922: read id fail!!!\n");
        ret = -1;
        goto err_out;
    }
success_out:
    osLog(LOG_INFO, "apds9922: auto detect success:0x%x\n", mTask.deviceId);
    alsSensorRegister();
    psSensorRegister();
    registerAlsPsDriverFsm(apds9922Fsm, ARRAY_SIZE(apds9922Fsm));
err_out:
    return ret;
}

#ifndef CFG_OVERLAY_INIT_SUPPORT
MODULE_DECLARE(apds9922, SENS_TYPE_ALS, apds9922Init);
#else
#include "mtk_overlay_init.h"
OVERLAY_DECLARE(apds9922, OVERLAY_ID_ALSPS, apds9922Init);
#endif
