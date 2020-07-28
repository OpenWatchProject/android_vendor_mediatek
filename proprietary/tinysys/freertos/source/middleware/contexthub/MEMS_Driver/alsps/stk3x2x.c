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
#include <timer.h>
#include "eint.h"
#include "stk3x2x.h"
#include "alsps.h"

enum stk3x2xState {
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
    /* Alsps sample */
    STATE_GET_ALS_DATA,
    STATE_ALS_SET_DEBOUNCE,
    STATE_GET_PS_FLG,
    STATE_GET_PS_RAW_DATA,
    STATE_PS_SET_DEBOUNCE,
    /* Power on & off */
    STATE_GET_ALSPS_STATE,
    STATE_ALS_POWER_ON,
    STATE_ALS_POWER_OFF,
    STATE_PS_POWER_ON,
    STATE_PS_POWER_OFF,
    STATE_PS_UNMASK_EINT,
    STATE_CLR_INT,
    STATE_EN_EINT,
    /* Init state */
    STATE_SET_SW_RST,
    STATE_SET_ALSPS_CTRL,
    STATE_SET_WAIT,
    STATE_SET_PS_THDH,
    STATE_SET_PS_THDL,
    STATE_SET_SPEC_ADDR,
    STATE_SETUP_EINT,
    STATE_CORE
};

#define I2C_SPEED                       400000
#define ALS_NAME                        "stk3x2x_l"
#define PS_NAME                         "stk3x2x_p"
#define MAX_RXBUF 8
#define MAX_TXBUF 8

#define UNIT_MS_TO_NS   1000000

SRAM_REGION_BSS static struct stk3x2xTask {
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
    struct AlsPsData data;
    uint32_t als_raw_data;
    uint32_t prox_raw_data;
    int32_t psCali;
    uint32_t    ps_threshold_high;
    uint32_t    ps_threshold_low;
    uint32_t    als_threshold_high;
    uint32_t    als_threshold_low;
    uint8_t     ledctrl_val;
    uint8_t     state_val;
} mTask;

static struct stk3x2xTask *stk3x2xDebugPoint;
static int getLuxFromAlsData(void);

static int check_timeout(uint64_t *end_tick) {
    return rtcGetTime() > *end_tick ? 1 : 0;
}

static void cal_end_time(unsigned int ms, uint64_t *end_tick) {
    *end_tick = rtcGetTime() +
                ((uint64_t)ms * UNIT_MS_TO_NS);
}

static int stk3x2x_read_als_data(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    if (rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize)) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "als rx error\n");
        return -1;
    }
    mTask.txBuf[0] = STK_DATA1_ALS_REG;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 2, i2cCallBack,
                         next_state);
}

static int stk3x2x_get_als_value(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    int als_data = 0;
    mTask.als_raw_data = ((mTask.rxBuf[0] << 8) | mTask.rxBuf[1]);
    als_data = getLuxFromAlsData();

    if (als_data < 0)
        mTask.data.sensType = SENS_TYPE_INVALID;
    else {
        mTask.data.als_data = als_data;
        mTask.data.sensType = SENS_TYPE_ALS;
    }

    txTransferDataInfo(&mTask.dataInfo, 1, &mTask.data);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int getLuxFromAlsData(void) {
    int idx;
    int invalid = 0;
    int als_level_num, als_value_num;

    als_level_num = sizeof(mTask.hw->als_level) / sizeof(mTask.hw->als_level[0]);
    als_value_num = sizeof(mTask.hw->als_value) / sizeof(mTask.hw->als_value[0]);

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
        int level_high = mTask.hw->als_level[idx];
        int level_low = (idx > 0) ? mTask.hw->als_level[idx - 1] : 0;
        int level_diff = level_high - level_low;
        int value_high = mTask.hw->als_value[idx];
        int value_low = (idx > 0) ? mTask.hw->als_value[idx - 1] : 0;
        int value_diff = value_high - value_low;
        int value = 0;

        if ((level_low >= level_high) || (value_low >= value_high))
            value = value_low;
        else
            value = (level_diff * value_low + (mTask.als_raw_data - level_low) * value_diff + ((
                         level_diff + 1) >> 1)) / level_diff;

        return value;
    }

    return -1;
}


static void alsGetData(void *sample) {
    struct SingleAxisDataPoint *singleSample = (struct SingleAxisDataPoint *)sample;
    singleSample->idata = getLuxFromAlsData();
}

static int stk3x2x_read_ps(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                          void *inBuf, uint8_t inSize, uint8_t elemInSize,
                          void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    if (rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize)) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "stk3x2x ps, rx dataInfo error\n");
        return -1;
    }
    mTask.txBuf[0] = STK_FLAG_REG;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,
                         next_state);
}

static int stk3x2x_get_ps_status(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    int ps_flag = mTask.rxBuf[0] & 0x01;
    if (ps_flag == 1) { /* 1 is far */
        mTask.data.prox_state = PROX_STATE_FAR; /* far */
        mTask.data.prox_data = 1; /* far state distance is 1cm */
    } else if (ps_flag == 0) {
        mTask.data.prox_state = PROX_STATE_NEAR; /* near */
        mTask.data.prox_data = 0; /* near state distance is 0cm */
    } else
        return -1;

    mTask.txBuf[0] = STK_DATA1_PS_REG;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 2, i2cCallBack,
                         next_state);
}

static int stk3x2x_get_ps_raw_data(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                  void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                  void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.prox_raw_data = (mTask.rxBuf[0] << 8) | mTask.rxBuf[1];
    if (mTask.prox_raw_data < mTask.psCali)
        mTask.prox_raw_data = 0;
    mTask.prox_raw_data -= mTask.psCali;

    mTask.txBuf[0] = STK_FLAG_REG;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                       mTask.rxBuf, 1, i2cCallBack,
                       next_state);
}

static int stk3x2x_clr_int(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                  void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                  void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.txBuf[0] = STK_FLAG_REG;
    mTask.txBuf[1] = mTask.rxBuf[0] & ~STK_FLG_PSINT_MASK;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int stk3x2x_enable_eint(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                  void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                  void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mt_eint_unmask(mTask.hw->eint_num);
    mTask.data.sensType = SENS_TYPE_PROX;
    txTransferDataInfo(&mTask.dataInfo, 1, &mTask.data);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static void psGetData(void *sample) {
    char txBuf[1];
    static char rxBuf[2];
    struct TripleAxisDataPoint *tripleSample = (struct TripleAxisDataPoint *)sample;
    tripleSample->ix = mTask.prox_raw_data;
    tripleSample->iy = (mTask.data.prox_state == PROX_STATE_NEAR ? 0 : 1);

    void get_ps_data(void *cookie, size_t tx, size_t rx, int err) {
        char *rxBuf = cookie;
        if (err == 0) {
            mTask.prox_raw_data = (rxBuf[0] << 8) | rxBuf[1];
            if (mTask.prox_raw_data < mTask.psCali)
                mTask.prox_raw_data = 0;
            mTask.prox_raw_data -= mTask.psCali;
        } else
            osLog(LOG_INFO, "stk3x2x: read ps data i2c error (%d)\n", err);
    }
    txBuf[0] = STK_DATA1_PS_REG;
    i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, txBuf, 1,
                  rxBuf, 2, get_ps_data, rxBuf);
}

static void stk3x2x_ps_set_threshold(uint32_t threshold_high, uint32_t threshold_low) {
    int ret;
    char txBuf[6];

    txBuf[0] = STK_THDH1_PS_REG;
    txBuf[1] = (uint8_t)((threshold_high & 0xFF00) >> 8);
    txBuf[2] = (uint8_t)(threshold_high & 0xFF);
    txBuf[3] = (uint8_t)((threshold_low & 0xFF00) >> 8);
    txBuf[4] = (uint8_t)(threshold_low & 0xFF);
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, &txBuf[0], 5,
                NULL, 0);
    if (ret) {
        osLog(LOG_INFO, "stk3x2x: set ps threshold i2c error (%d)\n", ret);
        return;
    }
}

static void psGetCalibration(int32_t *cali, int32_t size) {
    cali[0] = mTask.psCali;
}

static void psSetCalibration(int32_t *cali, int32_t size) {
    mTask.psCali = cali[0];
    stk3x2x_ps_set_threshold(mTask.ps_threshold_high + mTask.psCali,
                            mTask.ps_threshold_low + mTask.psCali);
}

static void psGetThreshold(uint32_t *threshold_high, uint32_t *threshold_low) {
    *threshold_high = mTask.ps_threshold_high;
    *threshold_low = mTask.ps_threshold_low;
    osLog(LOG_INFO, "%s ==>threshold_high:%ld threshold_low:%ld \n", __func__, *threshold_high, *threshold_low);
}

static void psSetThreshold(uint32_t threshold_high, uint32_t threshold_low) {
    osLog(LOG_INFO, "%s ==>threshold_high:%ld threshold_low:%ld \n", __func__, threshold_high, threshold_low);
    mTask.ps_threshold_high = threshold_high;
    mTask.ps_threshold_low = threshold_low;
    stk3x2x_ps_set_threshold(mTask.ps_threshold_high, mTask.ps_threshold_low);
}

static void psGetSensorInfo(struct sensorInfo_t *data)
{
    strncpy(data->name, PS_NAME, sizeof(data->name));
}

static void alsGetSensorInfo(struct sensorInfo_t *data)
{
    strncpy(data->name, ALS_NAME, sizeof(data->name));
}

static int stk3x2x_mask_eint(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mt_eint_mask(mTask.hw->eint_num);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int stk3x2x_unmask_eint(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mt_eint_unmask(mTask.hw->eint_num);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int stk3x2x_set_als_debounce_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                      void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                      void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.als_debounce_on = 1;
    cal_end_time(mTask.als_debounce_time, &mTask.als_debounce_end);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int stk3x2x_set_ps_debounce_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                     void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                     void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.ps_debounce_on = 1;
    cal_end_time(mTask.ps_debounce_time, &mTask.ps_debounce_end);
    mTask.data.prox_state = PROX_STATE_INIT;
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int stk3x2x_set_als_power_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                   void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                   void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.txBuf[0] = STK_STATE_REG;
    mTask.txBuf[1] = mTask.rxBuf[0] | STK_STATE_EN_ALS_MASK;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int stk3x2x_set_als_power_off(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                    void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.als_debounce_on = 0;
    mTask.txBuf[0] = STK_STATE_REG;
    mTask.txBuf[1] = mTask.rxBuf[0] & ~STK_STATE_EN_ALS_MASK;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int stk3x2x_als_ratechg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int stk3x2x_set_ps_power_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                  void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                  void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.txBuf[0] = STK_STATE_REG;
    mTask.txBuf[1] = mTask.rxBuf[0] | STK_STATE_EN_PS_MASK;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int stk3x2x_set_ps_power_off(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                   void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                   void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.ps_debounce_on = 0;
    mTask.data.prox_state = PROX_STATE_FAR;
    mTask.txBuf[0] = STK_STATE_REG;
    mTask.txBuf[1] = mTask.rxBuf[0] & ~STK_STATE_EN_PS_MASK;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int stk3x2x_ps_ratechg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int stk3x2x_get_alsps_state(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                               void *inBuf, uint8_t inSize, uint8_t elemInSize,
                               void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.txBuf[0] = STK_STATE_REG;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,
                         next_state);
}

static int stk3x2x_set_wait_time(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.txBuf[0] = STK_WAIT_REG;
    mTask.txBuf[1] = STK_WAIT_RST;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int stk3x2x_set_sw_reset(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.txBuf[0] = STK_SW_RESET_REG;
    mTask.txBuf[1] = 0;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int stk3x2x_set_alsps_ctrl(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.txBuf[0] = STK_STATE_REG;
    mTask.txBuf[1] = mTask.state_val;
    mTask.txBuf[2] = STK_PSCTRL_4X_64X;
    mTask.txBuf[3] = STK_ALSCTRL_4X_4X;
    mTask.txBuf[4] = mTask.ledctrl_val;
    mTask.txBuf[5] = STK_INT_CTRL;
    if (0 == mTask.hw->polling_mode_ps)
        mTask.txBuf[5] |= 1 << STK_INT_PS_SHIFT;
    else if (0 == mTask.hw->polling_mode_als)
        mTask.txBuf[5] |= 1 << STK_INT_ALS_SHIFT;
        mTask.txBuf[6] = STK_WAIT_100MS;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 7,
                       i2cCallBack, next_state);
}

static int stk3x2x_set_spec_addr(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.txBuf[0] = STK_SPEC1_REG;
    mTask.txBuf[1] = STK_SPEC1_ADDR;
    mTask.txBuf[2] = STK_INT_CTRL2;
    mTask.txBuf[3] = STK_SPEC2_ADDR;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 4,
                       i2cCallBack, next_state);
}

static int stk3x2x_set_ps_thdl(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    if (mTask.hw->polling_mode_ps == 0) {
        mTask.txBuf[0] = STK_THDL1_PS_REG;
        mTask.txBuf[1] = (uint8_t)(((mTask.ps_threshold_low + mTask.psCali) & 0xFF00) >> 8);
        mTask.txBuf[2] = (uint8_t)((mTask.ps_threshold_low + mTask.psCali) & 0x00FF);

        return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3,
                           i2cCallBack, next_state);
    } else {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
        return 0;
    }
}

static int stk3x2x_set_ps_thdh(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    if (mTask.hw->polling_mode_ps == 0) {
        mTask.txBuf[0] = STK_THDH1_PS_REG;
        mTask.txBuf[1] = (uint8_t)(((mTask.ps_threshold_high + mTask.psCali) & 0xFF00) >> 8);
        mTask.txBuf[2] = (uint8_t)((mTask.ps_threshold_high + mTask.psCali) & 0x00FF);

        return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3,
                           i2cCallBack, next_state);
    } else {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
        return 0;
    }
}

SRAM_REGION_FUNCTION static void stk3x2x_eint_handler(int arg) {
    alsPsInterruptOccur();
}

static int stk3x2x_setup_eint(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mt_eint_dis_hw_debounce(mTask.hw->eint_num);
    mt_eint_registration(mTask.hw->eint_num, LEVEL_SENSITIVE, LOW_LEVEL_TRIGGER, stk3x2x_eint_handler, EINT_INT_UNMASK,
                         EINT_INT_AUTO_UNMASK_OFF);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int stk3x2x_register_core(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
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

static struct sensorFsm stk3x2xFsm[] = {
    /* sample als */
    sensorFsmCmd(STATE_SAMPLE_ALS, STATE_GET_ALS_DATA, stk3x2x_read_als_data),
    sensorFsmCmd(STATE_GET_ALS_DATA, STATE_SAMPLE_ALS_DONE, stk3x2x_get_als_value),

    /* sample ps */
    sensorFsmCmd(STATE_SAMPLE_PS, STATE_GET_PS_FLG, stk3x2x_read_ps),
    sensorFsmCmd(STATE_GET_PS_FLG, STATE_GET_PS_RAW_DATA, stk3x2x_get_ps_status),
    sensorFsmCmd(STATE_GET_PS_RAW_DATA, STATE_CLR_INT, stk3x2x_get_ps_raw_data),
    sensorFsmCmd(STATE_CLR_INT, STATE_EN_EINT, stk3x2x_clr_int),
    sensorFsmCmd(STATE_EN_EINT, STATE_SAMPLE_PS_DONE, stk3x2x_enable_eint),

    /* als enable state */
    sensorFsmCmd(STATE_ALS_ENABLE, STATE_ALS_POWER_ON, stk3x2x_get_alsps_state),
    sensorFsmCmd(STATE_ALS_POWER_ON, STATE_ALS_SET_DEBOUNCE, stk3x2x_set_als_power_on),
    sensorFsmCmd(STATE_ALS_SET_DEBOUNCE, STATE_ALS_ENABLE_DONE, stk3x2x_set_als_debounce_on),

    /* als disable state */
    sensorFsmCmd(STATE_ALS_DISABLE, STATE_ALS_POWER_OFF, stk3x2x_get_alsps_state),
    sensorFsmCmd(STATE_ALS_POWER_OFF, STATE_ALS_DISABLE_DONE, stk3x2x_set_als_power_off),

    /* ps enable state */
    sensorFsmCmd(STATE_PS_ENABLE, STATE_PS_POWER_ON, stk3x2x_get_alsps_state),
    sensorFsmCmd(STATE_PS_POWER_ON, STATE_PS_SET_DEBOUNCE, stk3x2x_set_ps_power_on),
    sensorFsmCmd(STATE_PS_SET_DEBOUNCE, STATE_PS_UNMASK_EINT, stk3x2x_set_ps_debounce_on),
    sensorFsmCmd(STATE_PS_UNMASK_EINT, STATE_PS_ENABLE_DONE, stk3x2x_unmask_eint),
    /* ps disable state */
    sensorFsmCmd(STATE_PS_DISABLE, STATE_GET_ALSPS_STATE, stk3x2x_mask_eint),
    sensorFsmCmd(STATE_GET_ALSPS_STATE, STATE_PS_POWER_OFF, stk3x2x_get_alsps_state),
    sensorFsmCmd(STATE_PS_POWER_OFF, STATE_PS_DISABLE_DONE, stk3x2x_set_ps_power_off),

    /* als rate change */
    sensorFsmCmd(STATE_ALS_RATECHG, STATE_ALS_RATECHG_DONE, stk3x2x_als_ratechg),
    /* ps rate change */
    sensorFsmCmd(STATE_PS_RATECHG, STATE_PS_RATECHG_DONE, stk3x2x_ps_ratechg),
    /* init state */
    sensorFsmCmd(STATE_RESET, STATE_SET_SW_RST, stk3x2x_set_wait_time),
    sensorFsmCmd(STATE_SET_SW_RST, STATE_SET_ALSPS_CTRL, stk3x2x_set_sw_reset),
    sensorFsmCmd(STATE_SET_ALSPS_CTRL, STATE_SET_PS_THDH, stk3x2x_set_alsps_ctrl),
    sensorFsmCmd(STATE_SET_PS_THDH, STATE_SET_PS_THDL, stk3x2x_set_ps_thdh),
    sensorFsmCmd(STATE_SET_PS_THDL, STATE_SET_SPEC_ADDR, stk3x2x_set_ps_thdl),
    sensorFsmCmd(STATE_SET_SPEC_ADDR, STATE_SETUP_EINT, stk3x2x_set_spec_addr),
    sensorFsmCmd(STATE_SETUP_EINT, STATE_CORE, stk3x2x_setup_eint),
    sensorFsmCmd(STATE_CORE, STATE_INIT_DONE, stk3x2x_register_core),
};

static int stk3x2xInit(void) {
    int ret = 0;
    osLog(LOG_INFO, "%s: task starting\n", __func__);
    stk3x2xDebugPoint = &mTask;

    mTask.hw = get_cust_alsps("stk3x2x");
    if (NULL == mTask.hw) {
        osLog(LOG_ERROR, "get_cust_acc_hw fail\n");
        return -1;
    }
    mTask.i2c_addr = mTask.hw->i2c_addr[0];
    mTask.als_debounce_time = 200;
    mTask.als_debounce_on = 0;
    mTask.ps_debounce_time = 10;
    mTask.ps_debounce_on = 0;
    mTask.psCali = 0;
    mTask.als_threshold_high = mTask.hw->als_threshold_high;
    mTask.als_threshold_low = mTask.hw->als_threshold_low;
    mTask.ps_threshold_high = mTask.hw->ps_threshold_high;
    mTask.ps_threshold_low = mTask.hw->ps_threshold_low;
    mTask.state_val = 0; /* Standby */

    i2cMasterRequest(mTask.hw->i2c_num, I2C_SPEED);

    mTask.txBuf[0] = STK_PDT_ID_REG;
    ret = i2cMasterTxRxSync(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
        &mTask.deviceId, 1, NULL, NULL);
    if (ret < 0) {
        osLog(LOG_ERROR, "stk3x2x i2cMasterTxRxSync fail!!!\n");
        ret = -1;
        i2cMasterRelease(mTask.hw->i2c_num);
        goto err_out;
    }
    osLog(LOG_INFO, "stk3x2x: auto detect success:0x%x\n", mTask.deviceId);

    if (mTask.deviceId == STK3325_PID || mTask.deviceId == STK3327_PID)
        mTask.ledctrl_val = 0x20;
    else if (mTask.deviceId == STK3220_PID)
       mTask.ledctrl_val = 0x80;
    else
       mTask.ledctrl_val = 0x60; /* PID is 0x15 */

    alsSensorRegister();
    psSensorRegister();
    registerAlsPsDriverFsm(stk3x2xFsm, ARRAY_SIZE(stk3x2xFsm));
err_out:
    return ret;
}

#ifndef CFG_OVERLAY_INIT_SUPPORT
MODULE_DECLARE(stk3x2x, SENS_TYPE_ALS, stk3x2xInit);
#else
#include "mtk_overlay_init.h"
OVERLAY_DECLARE(stk3x2x, OVERLAY_ID_ALSPS, stk3x2xInit);
#endif
