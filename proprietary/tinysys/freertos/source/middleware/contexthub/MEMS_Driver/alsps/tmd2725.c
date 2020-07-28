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
#include "tmd2725.h"
#include "alsps.h"

#define I2C_SPEED                       400000
#define ALS_NAME                         "tmd2725_l"
#define PS_NAME                          "tmd2725_p"

#define UNIT_MS_TO_NS   1000000


enum tmd2725State {
    STATE_SAMPLE_ALS       = CHIP_SAMPLING_ALS,
    STATE_SAMPLE_ALS_DONE  = CHIP_SAMPLING_ALS_DONE,
    STATE_SAMPLE_PS        = CHIP_SAMPLING_PS,
    STATE_SAMPLE_PS_DONE   = CHIP_SAMPLING_PS_DONE,
    STATE_ALS_ENABLE       = CHIP_ALS_ENABLE,
    STATE_ALS_ENABLE_DONE  = CHIP_ALS_ENABLE_DONE,
    STATE_ALS_DISABLE      = CHIP_ALS_DISABLE,
    STATE_ALS_DISABLE_DONE = CHIP_ALS_DISABLE_DONE,
    STATE_ALS_RATECHG      = CHIP_ALS_RATECHG,
    STATE_ALS_RATECHG_DONE = CHIP_ALS_RATECHG_DONE,
    STATE_PS_ENABLE        = CHIP_PS_ENABLE,
    STATE_PS_ENABLE_DONE   = CHIP_PS_ENABLE_DONE,
    STATE_PS_DISABLE       = CHIP_PS_DISABLE,
    STATE_PS_DISABLE_DONE  = CHIP_PS_DISABLE_DONE,
    STATE_PS_RATECHG       = CHIP_PS_RATECHG,
    STATE_PS_RATECHG_DONE  = CHIP_PS_RATECHG_DONE,
    STATE_INIT_DONE        = CHIP_INIT_DONE,
    STATE_IDEL             = CHIP_IDLE,
    STATE_RESET            = CHIP_RESET,

    STATE_GET_ALS_GAIN,
    STATE_GET_ALS_DATA,
    STATE_GET_ALS_MEAS_RATE,
    STATE_ALS_SET_DEBOUNCE,
    STATE_GET_PS_INT_STATUS,
    STATE_GET_PS_RAW_DATA,
    STATE_GET_PS_STATUS,
    STATE_GET_PS_STAT,
    STATE_PS_SET_DEBOUNCE,
    STATE_SET_MAIN_CTRL,
    STATE_SET_PS_LED,
    STATE_ALS_POWER_ON,
    STATE_ALS_POWER_OFF,
    STATE_SET_PS_PULSES,
    STATE_SET_PS_MEAS_RATE,
    STATE_SET_ALS_MEAS_RATE,
    STATE_SET_WTIME,
    STATE_SET_ALS_GAIN,
    STATE_SET_INT_PERSIST,
    STATE_SET_INT_CFG,
    STATE_PS_POWER_ON,
    STATE_PS_POWER_OFF,
    STATE_SET_THDL,
    STATE_SET_THDH,
    STATE_SETUP_EINT,
    STATE_CORE
};

#define MAX_RXBUF 8
#define MAX_TXBUF 8

struct als_reg {
    u32 als_value;
    u32 photopic;
    u32 tgproduct;
    u32 saturation;
    u32 ir;
    u16 als_factor;
    u16 als_time;
    u16 als_gain;
};

SRAM_REGION_BSS static struct tmd2725Task {
    /* txBuf for i2c operation, fill register and fill value */
    bool                        alsPowerOn;
    bool                        psPowerOn;
    bool                        ps_cali;              /*ps is calibration*/
    unsigned int                als_debounce_time;
    unsigned int                als_debounce_on;    /*indicates if the debounce is on*/
    uint64_t                    als_debounce_end;
    unsigned int                ps_debounce_time;
    unsigned int                ps_debounce_on;     /*indicates if the debounce is on*/
    uint64_t                    ps_debounce_end;
    unsigned int                ps_suspend;
    uint8_t                     txBuf[MAX_TXBUF];
    /* rxBuf for i2c operation, receive rawdata */
    uint8_t                     rxBuf[MAX_RXBUF];
    uint8_t                     deviceId;
    /* data for factory */
    struct alsps_hw             *hw;
    uint8_t                     i2c_addr;
    struct transferDataInfo     dataInfo;
    struct AlsPsData            data[2];
    struct als_reg              als_reg_val;
    uint32_t                    als_raw_data;
    uint32_t                    prox_raw_data;
    int32_t                     psCali;
    uint32_t                    ps_threshold_high;
    uint32_t                    ps_threshold_low;
    uint32_t                    als_threshold_high;
    uint32_t                    als_threshold_low;
    uint8_t                     int_status;
} mTask;
static struct tmd2725Task *tmd2725DebugPoint;

static int check_timeout(uint64_t *end_tick) {
    return rtcGetTime() > *end_tick ? 1 : 0;
}

static void cal_end_time(unsigned int ms, uint64_t *end_tick) {
    *end_tick = rtcGetTime() +
                ((uint64_t)ms * UNIT_MS_TO_NS);
}


static int tmd2725_read_als_gain(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    if (rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize)) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "tmd2725 als, rx dataInfo error\n");
        return -1;
    }
    mTask.txBuf[0] = TMD2725_REG_CFG1;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,
                         next_state);
}

static int tmd2725_read_als_meas_rate(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                     void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                     void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.als_reg_val.als_gain = mTask.rxBuf[0] & AGAIN_MASK;

    mTask.txBuf[0] = TMD2725_REG_ATIME;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,
                         next_state);
}

static int tmd2725_read_als_data(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.als_reg_val.als_time = mTask.rxBuf[0];

    mTask.txBuf[0] = TMD2725_REG_CH0DATA;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 4, i2cCallBack,
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
    } else {
        /*
        if (mTask.trace & CMC_TRC_CVT_ALS) {
            APS_LOG("ALS: %05d => %05d (-1)\n\r", als, mTask.hw->als_value[idx]);
        }*/
        return -1;
    }
}

static void tmd2725_hub_calc_tgp(void) {
    uint32_t sat, tgp;
    uint32_t tmp1, tmp2;
    uint32_t atime = mTask.als_reg_val.als_time + 1;

    /* Calculate the saturation level */
    tmp1 = MAX_ALS_VALUE;
    tmp2 = (atime << 10) - 1;
    sat  = tmp1 < tmp2 ? tmp1:tmp2;
    sat  = sat * 8 / 10;

    /* Calculate product of ALS time and gain */
    atime *= ATIME_PER_STEP_X100;
    atime /= 100;

    tgp = als_gains[mTask.als_reg_val.als_gain];
    tgp *= atime;

    mTask.als_reg_val.saturation = sat;
    mTask.als_reg_val.tgproduct = tgp;
}

static uint32_t tmd2725_hub_cal_lux(void) {
    int32_t lux1 = 0, lux2 = 0;
    int32_t lux;
    uint32_t tgp = 0;

    /* Lux equation */
    lux1 = mTask.als_reg_val.photopic * 1000 -  mTask.als_reg_val.ir * B_Coef;
    lux2 = mTask.als_reg_val.photopic * C_Coef -  mTask.als_reg_val.ir * D_Coef;

    if (USE_MAX) {
        lux = lux1 > lux2 ? lux1 : lux2;
    } else {
        lux = lux1 < lux2 ? lux1 : lux2;
    }

    if (lux < 0) {
        lux = 0;
    }

    tgp = mTask.als_reg_val.tgproduct;
    if (lux < (100 * tgp)) {
        lux *= D_Factor;
        lux /= tgp;
    } else {
        lux /= tgp;
        lux *= D_Factor;
    }

    lux += 500;

    /* Scalling */
    if (lux > 1000 * 1000) {
        lux /= 1000;
        lux *= mTask.als_reg_val.als_factor;
    } else {
        lux *= mTask.als_reg_val.als_factor;
        lux /= 1000;
    }

    if (lux > SNS_DD_ALS_RANGE_MAX) {
        lux = SNS_DD_ALS_RANGE_MAX;
    }

    return lux;
}

static int tmd2725_read_als(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                           void *inBuf, uint8_t inSize, uint8_t elemInSize,
                           void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    int32_t lux = 0;
    int als_data = 0;
    mTask.als_reg_val.photopic = mTask.rxBuf[0] | (mTask.rxBuf[1] << 8);
    mTask.als_reg_val.ir = mTask.rxBuf[2] | (mTask.rxBuf[3] << 8);

    tmd2725_hub_calc_tgp();

    if (mTask.als_reg_val.photopic >= MAX_ALS_VALUE)
        lux = SNS_DD_ALS_RANGE_MAX;

    else if (mTask.als_reg_val.photopic <= MIN_ALS_VALUE)
        lux = 0;
    else
        lux = tmd2725_hub_cal_lux();

    mTask.als_raw_data = lux;
    als_data = getLuxFromAlsData();  // aal mapping value

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
    osLog(LOG_INFO, "tmd2725: %s \n", __func__);
    struct SingleAxisDataPoint *singleSample = (struct SingleAxisDataPoint *)sample;
    singleSample->idata = getLuxFromAlsData();
}

static int tmd2725_read_ps_interrupt_status(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                          void *inBuf, uint8_t inSize, uint8_t elemInSize,
                          void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    if (rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize)) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "tmd2725 ps, rx dataInfo error\n");
        return -1;
    }
    mTask.txBuf[0] = TMD2725_REG_STATUS;

    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         &mTask.int_status, 1, i2cCallBack,
                         next_state);
}

static int tmd2725_read_ps_raw_data(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                          void *inBuf, uint8_t inSize, uint8_t elemInSize,
                          void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.txBuf[0] = TMD2725_REG_PDATA;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,
                         next_state);
}

static void tmd2725_ps_set_threshold(uint32_t threshold_high, uint32_t threshold_low);
static int tmd2725_get_ps_status(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    bool ps_flag = 0;
    int ps_threshold_high = 80, ps_threshold_low = 30;

    if (mTask.rxBuf[0] >= mTask.ps_threshold_high) {
        ps_flag = 1;
        ps_threshold_high = 0xFF;
        ps_threshold_low = mTask.ps_threshold_low;
    } else if (mTask.rxBuf[0] <= mTask.ps_threshold_low) {
        ps_flag = 0;
        ps_threshold_low = 0;
        ps_threshold_high = mTask.ps_threshold_high;
    }
    if (ps_flag == 0) { /* 0 is far */
        mTask.data[0].prox_state = PROX_STATE_FAR; /* far */
        mTask.data[0].prox_data = 1; /* far state distance is 1cm */
    } else {
        mTask.data[0].prox_state = PROX_STATE_NEAR; /* near */
        mTask.data[0].prox_data = 0; /* near state distance is 0cm */
    }

    mTask.prox_raw_data = mTask.rxBuf[0];
    if (mTask.prox_raw_data < mTask.psCali)
        mTask.prox_raw_data = 0;
    mTask.prox_raw_data -= mTask.psCali;

    tmd2725_ps_set_threshold(ps_threshold_high, ps_threshold_low);

    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int tmd2725_clear_ps_interrupt(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                  void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                  void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.data[0].sensType = SENS_TYPE_PROX;
    txTransferDataInfo(&mTask.dataInfo, 1, &mTask.data[0]);

    mt_eint_unmask(mTask.hw->eint_num); /* MT6797 use 11 */

    mTask.txBuf[0] = TMD2725_REG_STATUS;
    mTask.txBuf[1] = 0xFF;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static void psGetData(void *sample) {
    char txBuf[1];
    static char rxBuf[2];
    struct TripleAxisDataPoint *tripleSample = (struct TripleAxisDataPoint *)sample;
    tripleSample->ix = mTask.prox_raw_data;
    tripleSample->iy = (mTask.data[0].prox_state == PROX_STATE_NEAR ? 0 : 1);

    osLog(LOG_ERROR, "%s ==> ps_raw_data:%x\n", __func__, mTask.prox_raw_data);
    void get_ps_data(void *cookie, size_t tx, size_t rx, int err) {
        char *rxBuf = cookie;
        if (err == 0) {
            mTask.prox_raw_data = rxBuf[0];
            if (mTask.prox_raw_data < mTask.psCali)
                mTask.prox_raw_data = 0;
            mTask.prox_raw_data -= mTask.psCali;
        } else
            osLog(LOG_INFO, "tmd2725: read ps data i2c error (%d)\n", err);
    }
    txBuf[0] = TMD2725_REG_PDATA;
    i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, txBuf, 1,
                  rxBuf, 2, get_ps_data, rxBuf);
}

static void tmd2725_ps_set_threshold(uint32_t threshold_high, uint32_t threshold_low) {
    int ret;
    char txBuf[6];

    txBuf[0] = TMD2725_REG_PIHT;
    txBuf[1] = (unsigned char)(threshold_high & 0xFF);
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, txBuf, 2,
                      NULL, 0);
    if (ret) {
        osLog(LOG_INFO, "tmd2725: set ps threshold_high i2c error (%d)\n", ret);
        return;
    }
    txBuf[2] = TMD2725_REG_PILT;
    txBuf[3] = (unsigned char)(threshold_low & 0xFF);
    i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, &txBuf[2], 2,
                NULL, 0);
    if (ret) {
        osLog(LOG_INFO, "tmd2725: set ps threshold_low i2c error (%d)\n", ret);
        return;
    }
}

static void psGetCalibration(int32_t *cali, int32_t size) {
    osLog(LOG_INFO, "tmd2725: %s \n", __func__);
    cali[0] = mTask.psCali;
}

static void psSetCalibration(int32_t *cali, int32_t size) {
    osLog(LOG_INFO, "tmd2725: %s \n", __func__);
    mTask.psCali = cali[0];
    tmd2725_ps_set_threshold(mTask.ps_threshold_high + mTask.psCali,
                            mTask.ps_threshold_low + mTask.psCali);
}

static void psGetThreshold(uint32_t *threshold_high, uint32_t *threshold_low) {
    *threshold_high = mTask.ps_threshold_high;
    *threshold_low = mTask.ps_threshold_low;
    osLog(LOG_ERROR, "%s ==>threshold_high:%d threshold_low:%d \n", __func__, *threshold_high, *threshold_low);
}

static void psSetThreshold(uint32_t threshold_high, uint32_t threshold_low) {
    osLog(LOG_ERROR, "%s ==>threshold_high:%d threshold_low:%d \n", __func__, threshold_high, threshold_low);
    mTask.ps_threshold_high = threshold_high;
    mTask.ps_threshold_low = threshold_low;
    tmd2725_ps_set_threshold(mTask.ps_threshold_high, mTask.ps_threshold_low);
}

static void psGetSensorInfo(struct sensorInfo_t *data)
{
    strncpy(data->name, PS_NAME, sizeof(data->name));
}

static void alsGetSensorInfo(struct sensorInfo_t *data)
{
    strncpy(data->name, ALS_NAME, sizeof(data->name));
}

static int tmd2725_set_als_debounce_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                      void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                      void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    osLog(LOG_INFO, "tmd2725: %s \n", __func__);
    mTask.als_debounce_on = 1;
    cal_end_time(mTask.als_debounce_time, &mTask.als_debounce_end);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

uint32_t timerHandle;  // fate psensor interrupt( using timer )
static void psInterruptCallback(uint32_t timerId, void *cookie) {
//  alsPsInterruptOccur();
}

static int tmd2725_set_ps_debounce_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                     void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                     void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    osLog(LOG_INFO, "tmd2725: %s \n", __func__);
    mTask.ps_debounce_on = 1;
    cal_end_time(mTask.ps_debounce_time, &mTask.ps_debounce_end);
    mTask.data[0].prox_state = PROX_STATE_INIT;
    timerHandle = timTimerSet(1000000000ULL, 0, 50, psInterruptCallback, NULL, false);  ///// fake timer
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int tmd2725_set_als_power_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                   void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                   void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    osLog(LOG_INFO, "tmd2725: %s \n", __func__);
    mTask.alsPowerOn = 1;
    if (!mTask.ps_cali) {
        mTask.txBuf[0] = TMD2725_REG_ENABLE;
        mTask.txBuf[1] = mTask.rxBuf[0] | TMD2725_AEN;

        return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
        i2cCallBack, next_state);
    } else
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);

    return 0;
}

static int tmd2725_set_als_power_off(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                    void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    osLog(LOG_INFO, "tmd2725: %s \n", __func__);
    mTask.als_debounce_on = 0;
    mTask.alsPowerOn = 0;
    mTask.txBuf[0] = TMD2725_REG_ENABLE;
    mTask.txBuf[1] = mTask.rxBuf[0] & ~TMD2725_AEN;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int tmd2725_als_ratechg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    osLog(LOG_INFO, "tmd2725: %s \n", __func__);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int tmd2725_set_ps_power_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                  void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                  void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    osLog(LOG_INFO, "tmd2725: %s \n", __func__);
    mTask.txBuf[0] = TMD2725_REG_ENABLE;
    mTask.txBuf[1] = mTask.rxBuf[0] | TMD2725_PEN;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int tmd2725_set_ps_power_off(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                   void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                   void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    osLog(LOG_INFO, "tmd2725: %s \n", __func__);
    mTask.ps_debounce_on = 0;
    mTask.data[0].prox_state = PROX_STATE_FAR; /* default value */
    mTask.txBuf[0] = TMD2725_REG_ENABLE;
    mTask.txBuf[1] = mTask.rxBuf[0] & ~TMD2725_PEN;
    timTimerCancel(timerHandle);  // disable the fake interrupt(timer)
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int tmd2725_ps_ratechg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    osLog(LOG_INFO, "tmd2725: %s \n", __func__);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int tmd2725_get_ps_conf(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    osLog(LOG_INFO, "tmd2725: %s \n", __func__);
    mTask.txBuf[0] = TMD2725_REG_ENABLE;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,
                         next_state);
}

static int tmd2725_get_als_conf(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                               void *inBuf, uint8_t inSize, uint8_t elemInSize,
                               void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    osLog(LOG_INFO, "tmd2725: %s \n", __func__);
    mTask.txBuf[0] = TMD2725_REG_ENABLE;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,
                         next_state);
}

static int tmd2725_set_main_ctrl(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    osLog(LOG_INFO, "tmd2725: %s \n", __func__);
    mTask.txBuf[0] = TMD2725_REG_ENABLE;
    mTask.txBuf[1] = TMD2725_PON;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int tmd2725_set_ps_led(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    // use polling
    osLog(LOG_INFO, "tmd2725: %s \n", __func__);
    mTask.txBuf[0] = TMD2725_REG_PGCFG1;
    mTask.txBuf[1] = PGAIN_1X | PLDRIVE_MA(100);
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int tmd2725_set_ps_pulses(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    // use polling
    osLog(LOG_INFO, "tmd2725: %s \n", __func__);
    mTask.txBuf[0] = TMD2725_REG_PGCFG0;
    mTask.txBuf[1] = PPLEN_8US | PPULSES(8);
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int tmd2725_set_ps_meas_rate(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                   void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                   void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    osLog(LOG_INFO, "tmd2725: %s \n", __func__);
    mTask.txBuf[0] = TMD2725_REG_PTIME;
    mTask.txBuf[1] = PTIME_US(4000);
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int tmd2725_set_als_meas_rate(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                    void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    osLog(LOG_INFO, "tmd2725: %s \n", __func__);
    mTask.txBuf[0] = TMD2725_REG_ATIME;
    mTask.txBuf[1] = ATIME_MS(50);
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int tmd2725_set_wtime(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                    void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    osLog(LOG_INFO, "tmd2725: %s \n", __func__);
    mTask.txBuf[0] = TMD2725_REG_WTIME;
    mTask.txBuf[1] = WTIME_MS(50);
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int tmd2725_set_als_gain(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                               void *inBuf, uint8_t inSize, uint8_t elemInSize,
                               void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    osLog(LOG_INFO, "tmd2725: %s \n", __func__);
    mTask.txBuf[0] = TMD2725_REG_CFG1;
    mTask.txBuf[1] = AGAIN_16X;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int tmd2725_set_int_persist(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                  void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                  void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    osLog(LOG_INFO, "tmd2725: %s \n", __func__);
    mTask.txBuf[0] = TMD2725_REG_INT_PERS;
    mTask.txBuf[1] = PROX_PERSIST(1);
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int tmd2725_set_int_cfg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    osLog(LOG_INFO, "tmd2725: %s \n", __func__);
    mTask.txBuf[0] = TMD2725_REG_INTENAB;
    mTask.txBuf[1] = 0;
    if (0 == mTask.hw->polling_mode_ps)
        mTask.txBuf[1] |= TMD2725_PIEN;  // TMD2725_PIEN | TMD2725_PSIEN;
    if (0 == mTask.hw->polling_mode_als)
        mTask.txBuf[1] |= TMD2725_AIEN | TMD2725_ASIEN;

    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int tmd2725_set_ps_thdl(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    ///////// becareful about this
    osLog(LOG_INFO, "tmd2725: %s \n", __func__);
    if (mTask.hw->polling_mode_ps == 0) {
        mTask.txBuf[0] = TMD2725_REG_PILT;
        mTask.txBuf[1] = (u8)(mTask.ps_threshold_low + mTask.psCali);
        return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                           i2cCallBack, next_state);
    } else {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
        return 0;
    }
}

static int tmd2725_set_ps_thdh(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    ///////// becareful about this
    osLog(LOG_INFO, "tmd2725: %s \n", __func__);
    if (mTask.hw->polling_mode_ps == 0) {
        mTask.txBuf[0] = TMD2725_REG_PIHT;
        mTask.txBuf[1] = (u8)(mTask.ps_threshold_high + mTask.psCali);
        return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                           i2cCallBack, next_state);
    } else {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
        return 0;
    }
}

SRAM_REGION_FUNCTION static void tmd2725_eint_handler(int arg) {
    alsPsInterruptOccur();
}

static int tmd2725_setup_eint(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mt_eint_dis_hw_debounce(mTask.hw->eint_num);

    mt_eint_registration(mTask.hw->eint_num, EDGE_SENSITIVE, LOW_LEVEL_TRIGGER, tmd2725_eint_handler, EINT_INT_UNMASK,
                         EINT_INT_AUTO_UNMASK_OFF);

    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int tmd2725_register_core(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
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

static struct sensorFsm tmd2725Fsm[] = {
    /* sample als */
    sensorFsmCmd(STATE_SAMPLE_ALS, STATE_GET_ALS_GAIN, tmd2725_read_als_gain),
    sensorFsmCmd(STATE_GET_ALS_GAIN, STATE_GET_ALS_MEAS_RATE, tmd2725_read_als_meas_rate),
    sensorFsmCmd(STATE_GET_ALS_MEAS_RATE, STATE_GET_ALS_DATA, tmd2725_read_als_data),
    sensorFsmCmd(STATE_GET_ALS_DATA, STATE_SAMPLE_ALS_DONE, tmd2725_read_als),
    /* sample ps */
    sensorFsmCmd(STATE_SAMPLE_PS, STATE_GET_PS_INT_STATUS, tmd2725_read_ps_interrupt_status),
    sensorFsmCmd(STATE_GET_PS_INT_STATUS, STATE_GET_PS_RAW_DATA, tmd2725_read_ps_raw_data),
    sensorFsmCmd(STATE_GET_PS_RAW_DATA, STATE_GET_PS_STATUS, tmd2725_get_ps_status),
    sensorFsmCmd(STATE_GET_PS_STATUS, STATE_SAMPLE_PS_DONE, tmd2725_clear_ps_interrupt),
    /* als enable state */
    sensorFsmCmd(STATE_ALS_ENABLE, STATE_ALS_POWER_ON, tmd2725_get_als_conf),
    sensorFsmCmd(STATE_ALS_POWER_ON, STATE_ALS_SET_DEBOUNCE, tmd2725_set_als_power_on),
    sensorFsmCmd(STATE_ALS_SET_DEBOUNCE, STATE_ALS_ENABLE_DONE, tmd2725_set_als_debounce_on),
    /* als disable state */
    sensorFsmCmd(STATE_ALS_DISABLE, STATE_ALS_POWER_OFF, tmd2725_get_als_conf),
    sensorFsmCmd(STATE_ALS_POWER_OFF, STATE_ALS_DISABLE_DONE, tmd2725_set_als_power_off),
    /* ps enable state */
    sensorFsmCmd(STATE_PS_ENABLE, STATE_PS_POWER_ON, tmd2725_get_ps_conf),
    sensorFsmCmd(STATE_PS_POWER_ON, STATE_PS_SET_DEBOUNCE, tmd2725_set_ps_power_on),
    sensorFsmCmd(STATE_PS_SET_DEBOUNCE, STATE_PS_ENABLE_DONE, tmd2725_set_ps_debounce_on),
    /* ps disable state */
    sensorFsmCmd(STATE_PS_DISABLE, STATE_PS_POWER_OFF, tmd2725_get_ps_conf),
    sensorFsmCmd(STATE_PS_POWER_OFF, STATE_PS_DISABLE_DONE, tmd2725_set_ps_power_off),
    /* als rate change */
    sensorFsmCmd(STATE_ALS_RATECHG, STATE_ALS_RATECHG_DONE, tmd2725_als_ratechg),
    /* ps rate change */
    sensorFsmCmd(STATE_PS_RATECHG, STATE_PS_RATECHG_DONE, tmd2725_ps_ratechg),
    /* init state */
    sensorFsmCmd(STATE_RESET, STATE_SET_MAIN_CTRL, tmd2725_set_main_ctrl),
    sensorFsmCmd(STATE_SET_MAIN_CTRL, STATE_SET_PS_LED, tmd2725_set_ps_led),
    sensorFsmCmd(STATE_SET_PS_LED, STATE_SET_PS_PULSES, tmd2725_set_ps_pulses),
    sensorFsmCmd(STATE_SET_PS_PULSES, STATE_SET_PS_MEAS_RATE, tmd2725_set_ps_meas_rate),
    sensorFsmCmd(STATE_SET_PS_MEAS_RATE, STATE_SET_ALS_MEAS_RATE, tmd2725_set_als_meas_rate),
    sensorFsmCmd(STATE_SET_ALS_MEAS_RATE, STATE_SET_WTIME, tmd2725_set_wtime),
    sensorFsmCmd(STATE_SET_WTIME, STATE_SET_ALS_GAIN, tmd2725_set_als_gain),
    sensorFsmCmd(STATE_SET_ALS_GAIN, STATE_SET_INT_PERSIST, tmd2725_set_int_persist),
    sensorFsmCmd(STATE_SET_INT_PERSIST, STATE_SET_INT_CFG, tmd2725_set_int_cfg),
    sensorFsmCmd(STATE_SET_INT_CFG, STATE_SET_THDL, tmd2725_set_ps_thdl),
    sensorFsmCmd(STATE_SET_THDL, STATE_SET_THDH, tmd2725_set_ps_thdh),
    sensorFsmCmd(STATE_SET_THDH, STATE_SETUP_EINT, tmd2725_setup_eint),
    sensorFsmCmd(STATE_SETUP_EINT, STATE_INIT_DONE, tmd2725_register_core),
};

static int tmd2725Init(void) {
    int ret = 0;

    osLog(LOG_INFO, "%s: task starting\n", __func__);
    tmd2725DebugPoint = &mTask;

    mTask.hw = get_cust_alsps("tmd2725");
    if (NULL == mTask.hw) {
        osLog(LOG_ERROR, "get_cust_acc_hw fail\n");
        ret = -1;
        goto err_out;
    }
    mTask.i2c_addr = mTask.hw->i2c_addr[0];
    mTask.alsPowerOn = 0;
    mTask.psPowerOn = 0;
    mTask.ps_cali = 0;
    mTask.als_debounce_time = 200;
    mTask.als_debounce_on = 0;
    mTask.ps_debounce_time = 200;
    mTask.ps_debounce_on = 0;
    mTask.psCali = 0;
    mTask.als_threshold_high = mTask.hw->als_threshold_high;
    mTask.als_threshold_low = mTask.hw->als_threshold_low;
    mTask.ps_threshold_high = mTask.hw->ps_threshold_high;
    mTask.ps_threshold_low = mTask.hw->ps_threshold_low;
    mTask.als_reg_val.als_factor = 1;
    osLog(LOG_ERROR, "alsps i2c_num: %d, i2c_addr: 0x%x\n", mTask.hw->i2c_num, mTask.i2c_addr);

    i2cMasterRequest(mTask.hw->i2c_num, I2C_SPEED);

    mTask.txBuf[0] = TMD2725_REG_ID;
    ret = i2cMasterTxRxSync(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                    &mTask.deviceId, 1, NULL, NULL);
    if (ret < 0) {
        osLog(LOG_ERROR, "tmd2725 i2cMasterTxRxSync fail!!!\n");
        ret = -1;
        i2cMasterRelease(mTask.hw->i2c_num);
        goto err_out;
    }

    if (mTask.deviceId == 0xE4) {  // tmd2725 device id is fixed
        goto success_out;
    } else {
        i2cMasterRelease(mTask.hw->i2c_num);
        osLog(LOG_ERROR, "tmd2725: read id fail!!!\n");
        ret = -1;
        goto err_out;
    }
success_out:
    osLog(LOG_INFO, "tmd2725: auto detect success:0x%x\n", mTask.deviceId);
    alsSensorRegister();
    psSensorRegister();
    registerAlsPsDriverFsm(tmd2725Fsm, ARRAY_SIZE(tmd2725Fsm));

err_out:
    return ret;
}
#ifndef CFG_OVERLAY_INIT_SUPPORT
MODULE_DECLARE(tmd2725, SENS_TYPE_ALS, tmd2725Init);
#else
#include "mtk_overlay_init.h"
OVERLAY_DECLARE(tmd2725, OVERLAY_ID_ALSPS, tmd2725Init);
#endif
