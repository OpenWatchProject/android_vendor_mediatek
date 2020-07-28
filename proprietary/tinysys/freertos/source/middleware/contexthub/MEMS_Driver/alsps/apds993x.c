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
#include "apds993x.h"
#include "alsps.h"
/* #include "cust_alsps.h" */
/* #include <FreeRTOS.h> */
/* #include <semphr.h> */

/*
 * Global data
 */
static unsigned char apds993x_als_atime_tb[] = { 0xF6, 0xED, 0xDB };
static unsigned short apds993x_als_integration_tb[] = { 2720, 5168, 10064 };
static unsigned char apds993x_als_again_tb[] = { 1, 8, 16, 120 };
static unsigned char apds993x_als_again_bit_tb[] = { 0x00, 0x01, 0x02, 0x03 };

/*===========================================================================================================================*/
enum apds993xState {
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
    STATE_ALS_CFG = CHIP_ALS_CFG,
    STATE_ALS_CFG_DONE = CHIP_ALS_CFG_DONE,
    STATE_PS_ENABLE = CHIP_PS_ENABLE,
    STATE_PS_ENABLE_DONE = CHIP_PS_ENABLE_DONE,
    STATE_PS_DISABLE = CHIP_PS_DISABLE,
    STATE_PS_DISABLE_DONE = CHIP_PS_DISABLE_DONE,
    STATE_PS_RATECHG = CHIP_PS_RATECHG,
    STATE_PS_RATECHG_DONE = CHIP_PS_RATECHG_DONE,
    STATE_PS_CFG = CHIP_PS_CFG,
    STATE_PS_CFG_DONE = CHIP_PS_CFG_DONE,
    STATE_INIT_DONE = CHIP_INIT_DONE,
    STATE_IDLE = CHIP_IDLE,
    STATE_RESET = CHIP_RESET,

    STATE_GET_ALS_DATA,

    STATE_GET_PS_DATA,
    STATE_GET_PS_STATUS,
    STATE_SET_PS_INT_CLEAR,

    STATE_ALS_POWER_ON,
    STATE_ALS_EN_POWER_OFF,
    STATE_ALS_SET_DEBOUNCE,

    STATE_PS_SET_ENABLE_FLAG,
    STATE_PS_EN_POWER_OFF,
    STATE_PS_SET_THRES,
    STATE_PS_EN_POWER_ON,

    STATE_PS_MASK_EINT,

    STATE_SET_INIT_POWER_OFF,
    STATE_SET_TIME,
    STATE_SET_PPCOUNT,
    STATE_SET_CONFIG,
    STATE_SET_CONTROL,
    STATE_SET_PS_THRES_INIT,
    STATE_SET_AILT,
    STATE_SET_AIHT,
    STATE_SET_PERS,
    STATE_SETUP_EINT
};

#define I2C_SPEED           400000
#define ALS_NAME                    "ap993x_l"
#define PS_NAME                     "ap993x_p"

#define MAX_RXBUF 8
#define MAX_TXBUF 8

#define UNIT_MS_TO_NS   1000000

struct als_reg {
    int prev_als_value;
    uint16_t ch0data;
    uint16_t ch1data;
};

SRAM_REGION_BSS static struct apds993xTask {
    /* txBuf for i2c operation, fill register and fill value */
    bool alsPowerOn;
    bool psPowerOn;
    unsigned int als_debounce_time;
    unsigned int als_debounce_on;   /*indicates if the debounce is on */
    uint64_t     als_debounce_end;
    unsigned int ps_debounce_time;
    unsigned int ps_debounce_on;    /*indicates if the debounce is on */
    uint64_t     ps_debounce_end;
    unsigned int ps_suspend;
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
    struct als_reg als_reg_val;
    int32_t psCali;
    uint32_t ps_threshold_high;
    uint32_t ps_threshold_low;
    uint32_t als_threshold_high;
    uint32_t als_threshold_low;
} mTask;

static struct apds993xTask *apds993xDebugPoint;

static void cal_end_time(unsigned int ms, uint64_t *end_tick)
{
    *end_tick = rtcGetTime() +
                ((uint64_t)ms * UNIT_MS_TO_NS);
}

static int LuxCalculation(void)
{
    int luxValue = 0;
    int IAC1 = 0;
    int IAC2 = 0;
    int IAC = 0;

    /* re-adjust COE_B to avoid 2 decimal point */
    IAC1 = (mTask.als_reg_val.ch0data - (APDS993X_COE_B * mTask.als_reg_val.ch1data) / 100);
    /* re-adjust COE_C and COE_D to void 2 decimal point */
    IAC2 =
        ((APDS993X_COE_C * mTask.als_reg_val.ch0data) / 100 -
         (APDS993X_COE_D * mTask.als_reg_val.ch1data) / 100);

    if (IAC1 > IAC2)
        IAC = IAC1;
    else if (IAC1 <= IAC2)
        IAC = IAC2;
    else
        IAC = 0;

    if (IAC1 < 0 && IAC2 < 0) {
        IAC = 0;    /* cdata and irdata saturated */
        return -1;  /* don't report first, change gain may help */
    }

    luxValue =
        ((IAC * APDS993X_GA * APDS993X_DF) / 100) /
        ((apds993x_als_integration_tb[APDS993X_ALS_RES_37888] / 100) *
         apds993x_als_again_tb[APDS993X_ALS_GAIN_8X]);

    return luxValue;
}

static int apds993x_calculate_als(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                void *inBuf, uint8_t inSize, uint8_t elemInSize, void *outBuf,
                uint8_t *outSize, uint8_t *elemOutSize)
{
    bool is_als_valid = 1;

    mTask.als_reg_val.ch0data = mTask.rxBuf[0] | (mTask.rxBuf[1] << 8);
    mTask.als_reg_val.ch1data = mTask.rxBuf[2] | (mTask.rxBuf[3] << 8);

    mTask.als_raw_data = LuxCalculation();

    if (mTask.als_raw_data >= 0) {
        mTask.als_raw_data =
            mTask.als_raw_data < 30000 ? mTask.als_raw_data : 30000;
        mTask.als_reg_val.prev_als_value = mTask.als_raw_data;
    } else {
        is_als_valid = 0;
        mTask.als_raw_data = mTask.als_reg_val.prev_als_value;
    }

    mTask.data.als_data = mTask.als_raw_data;
    mTask.data.sensType = SENS_TYPE_ALS;

    if (is_als_valid)
        txTransferDataInfo(&mTask.dataInfo, 1, &mTask.data);

    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int apds993x_read_als(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
                void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    if (rxTransferDataInfo
        (&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize)) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "apds993x als, rx dataInfo error\n");
        return -1;
    }

    mTask.txBuf[0] = CMD_BYTE | APDS993X_CH0DATAL_REG;
    i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[0], 1,
              &mTask.rxBuf[0], 1, i2cCallBack, (void *)STATE_IDLE);

    mTask.txBuf[1] = CMD_BYTE | APDS993X_CH0DATAH_REG;
    i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[1], 1,
              &mTask.rxBuf[1], 1, i2cCallBack, (void *)STATE_IDLE);

    mTask.txBuf[2] = CMD_BYTE | APDS993X_CH1DATAL_REG;
    i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[2], 1,
              &mTask.rxBuf[2], 1, i2cCallBack, (void *)STATE_IDLE);

    mTask.txBuf[3] = CMD_BYTE | APDS993X_CH1DATAH_REG;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[3], 1,
              &mTask.rxBuf[3], 1, i2cCallBack, next_state);
}

static void alsGetData(void *sample)
{
    struct SingleAxisDataPoint *singleSample = (struct SingleAxisDataPoint *)sample;

    singleSample->idata = LuxCalculation();
}

static int apds993x_read_ps(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                     void *inBuf, uint8_t inSize, uint8_t elemInSize,
                     void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    if (rxTransferDataInfo
        (&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize)) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "apds3x ps, rx dataInfo error\n");
        return -1;
    }

    mt_eint_mask(mTask.hw->eint_num);

    mTask.txBuf[0] = CMD_BYTE | APDS993X_PDATAL_REG;
    i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[0], 1,
              &mTask.rxBuf[0], 1, i2cCallBack, (void *)STATE_IDLE);

    mTask.txBuf[1] = CMD_BYTE | APDS993X_PDATAH_REG;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[1], 1,
              &mTask.rxBuf[1], 1, i2cCallBack, next_state);
}

static int apds993x_get_ps_status(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                    void *inBuf, uint8_t inSize, uint8_t elemInSize,
                    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ps_flag = 0;

    mTask.prox_raw_data = mTask.rxBuf[1] << 8 | mTask.rxBuf[0];
    /* osLog(LOG_ERROR, "ps rawdata 0x%x\n", mTask.prox_raw_data); */

    if ((mTask.prox_raw_data > mTask.ps_threshold_low) &&
        (mTask.prox_raw_data >= mTask.ps_threshold_high))
        ps_flag = 1;
    else if ((mTask.prox_raw_data <= mTask.ps_threshold_low) &&
        (mTask.prox_raw_data < mTask.ps_threshold_high))
        ps_flag = 0;
    else
        osLog(LOG_ERROR, "rawData (0x%x)\n", mTask.prox_raw_data);

    /* osLog(LOG_ERROR, "ps int! %s\n", ps_flag == 0 ? "far" : "near"); */
    if (ps_flag == 0) { /* 0 is far */
        mTask.data.prox_state = PROX_STATE_FAR;   /* far */
        mTask.data.prox_data = 1;  /* far state distance is 1cm */
        mTask.ps_threshold_low = 0;
        mTask.ps_threshold_high = APDS993X_PS_DETECTION_THRESHOLD;
    } else if (ps_flag == 1) {
        mTask.data.prox_state = PROX_STATE_NEAR;  /* near */
        mTask.data.prox_data = 0;  /* near state distance is 0cm */
        mTask.ps_threshold_low = APDS993X_PS_HSYTERESIS_THRESHOLD;
        mTask.ps_threshold_high = 1023;
    }

    mTask.txBuf[0] = CMD_BYTE | APDS993X_PIHTL_REG;
    mTask.txBuf[1] = mTask.ps_threshold_high & 0xFF;
    i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
            i2cCallBack, (void *)STATE_IDLE);

    mTask.txBuf[0] = CMD_BYTE | APDS993X_PIHTH_REG;
    mTask.txBuf[1] = mTask.ps_threshold_high >> 8;
    i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
            i2cCallBack, (void *)STATE_IDLE);

    mTask.txBuf[0] = CMD_BYTE | APDS993X_PILTL_REG;
    mTask.txBuf[1] = mTask.ps_threshold_low & 0xFF;
    i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
            i2cCallBack, (void *)STATE_IDLE);

    mTask.txBuf[0] = CMD_BYTE | APDS993X_PILTH_REG;
    mTask.txBuf[1] = mTask.ps_threshold_low >> 8;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
            i2cCallBack, next_state);
}

static int apds993x_set_ps_int_clear(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
                    void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
                    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.txBuf[0] = CMD_CLR_PS_INT;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1, i2cCallBack,
            next_state);
}

static int apds993x_sample_ps_done(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                    void *inBuf, uint8_t inSize, uint8_t elemInSize,
                    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    if (mTask.prox_raw_data < mTask.psCali)
        mTask.prox_raw_data = 0;
    mTask.prox_raw_data -= mTask.psCali;
    /* osLog(LOG_ERROR, "ps raw_data:%ld\n", mTask.prox_raw_data); */
    mt_eint_unmask(mTask.hw->eint_num);

    mTask.data.sensType = SENS_TYPE_PROX;
    txTransferDataInfo(&mTask.dataInfo, 1, &mTask.data);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);

    return 0;
}

static void psGetData(void *sample)
{
    char txBuf[1];
    static char rxBuf[2];
    struct TripleAxisDataPoint *tripleSample = (struct TripleAxisDataPoint *)sample;

    tripleSample->ix = mTask.prox_raw_data;
    tripleSample->iy = (mTask.data.prox_state == PROX_STATE_NEAR ? 0 : 1);

    /* osLog(LOG_ERROR, "%s ==> ps_raw_data:%ld\n", __func__, mTask.prox_raw_data); */
    void get_ps_data(void *cookie, size_t tx, size_t rx, int err)
    {
        char *rxBuf = cookie;

        if (err == 0) {
            mTask.prox_raw_data = ((rxBuf[1] << 8) & 0x7ff) | rxBuf[0];
            if (mTask.prox_raw_data < mTask.psCali)
                mTask.prox_raw_data = 0;
            mTask.prox_raw_data -= mTask.psCali;
        } else
            osLog(LOG_INFO, "apds993x: read ps data i2c error (%d)\n", err);
    }
    txBuf[0] = CMD_BYTE | APDS993X_PDATAL_REG;
    i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, txBuf, 1, rxBuf, 2, get_ps_data, rxBuf);
}

static void apds993x_ps_set_threshold(uint32_t threshold_high, uint32_t threshold_low)
{
    char txBuf[2];

    mTask.ps_threshold_high = threshold_high;
    mTask.ps_threshold_low = threshold_low;

    txBuf[0] = CMD_BYTE | APDS993X_PIHTL_REG;
    txBuf[1] = mTask.ps_threshold_high & 0xFF;
    i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, txBuf, 2, NULL, 0);

    txBuf[0] = CMD_BYTE | APDS993X_PIHTH_REG;
    txBuf[1] = mTask.ps_threshold_high >> 8;
    i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, txBuf, 2, NULL, 0);

    txBuf[0] = CMD_BYTE | APDS993X_PILTL_REG;
    txBuf[1] = mTask.ps_threshold_low & 0xFF;
    i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, txBuf, 2, NULL, 0);

    txBuf[0] = CMD_BYTE | APDS993X_PILTH_REG;
    txBuf[1] = mTask.ps_threshold_low >> 8;
    i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, txBuf, 2, NULL, 0);
}

static void psGetCalibration(int32_t *cali, int32_t size)
{
    cali[0] = mTask.psCali;
}

static void psSetCalibration(int32_t *cali, int32_t size)
{
    mTask.psCali = cali[0];
    apds993x_ps_set_threshold(mTask.ps_threshold_high + mTask.psCali,
                    mTask.ps_threshold_low + mTask.psCali);
}

static void psGetThreshold(uint32_t *threshold_high, uint32_t *threshold_low)
{
    *threshold_high = mTask.ps_threshold_high;
    *threshold_low = mTask.ps_threshold_low;
}

static void psSetThreshold(uint32_t threshold_high, uint32_t threshold_low)
{
    apds993x_ps_set_threshold(mTask.ps_threshold_high, mTask.ps_threshold_low);
}

static void psGetSensorInfo(struct sensorInfo_t *data)
{
    strncpy(data->name, PS_NAME, sizeof(data->name));
}

static void alsGetSensorInfo(struct sensorInfo_t *data)
{
    strncpy(data->name, ALS_NAME, sizeof(data->name));
}

static int apds993x_mask_eint(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
    void *inBuf, uint8_t inSize, uint8_t elemInSize, void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mt_eint_mask(mTask.hw->eint_num);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int apds993x_unmask_eint(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
    void *inBuf, uint8_t inSize, uint8_t elemInSize, void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mt_eint_unmask(mTask.hw->eint_num);

    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int apds993x_set_ps_enable_flag(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
            void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.psPowerOn = 1;
    mt_eint_mask(mTask.hw->eint_num);

    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int apds993x_set_apds_power_off(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
            void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.txBuf[0] = CMD_BYTE | APDS993X_ENABLE_REG;
    mTask.txBuf[1] = 0;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack, next_state);
}

static int apds993x_set_als_debounce_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
            void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.als_debounce_on = 1;
    cal_end_time(mTask.als_debounce_time, &mTask.als_debounce_end);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int apds993x_set_als_enable_flag(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
            void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.alsPowerOn = 1;

    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int apds993x_set_als_power_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
                void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.txBuf[0] = CMD_BYTE | APDS993X_ENABLE_REG;

    if (mTask.psPowerOn)
        mTask.txBuf[1] = APDS993x_DD_PIEN | APDS993x_DD_PEN |
            APDS993x_DD_AEN | APDS993x_DD_PON;
    else
        mTask.txBuf[1] = APDS993x_DD_AEN | APDS993x_DD_PON;

    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                i2cCallBack, next_state);
}

static int apds993x_set_als_disable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
                void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    if (mTask.psPowerOn) {
        /* PS is on. */
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
        return 0;
    }

    mTask.txBuf[0] = CMD_BYTE | APDS993X_ENABLE_REG;
    mTask.txBuf[1] = 0;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                i2cCallBack, next_state);
}

static int apds993x_als_cfg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int apds993x_ps_cfg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int apds993x_als_ratechg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int apds993x_set_ps_disable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.psPowerOn = 0;

    mTask.txBuf[0] = CMD_BYTE | APDS993X_ENABLE_REG;
    if (mTask.alsPowerOn)
        mTask.txBuf[1] = APDS993x_DD_AEN | APDS993x_DD_PON;
    else
        mTask.txBuf[1] = 0;

    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack, next_state);
}

static int apds993x_set_ps_thres_init(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack,
                void *next_state, void *inBuf, uint8_t inSize, uint8_t elemInSize,
                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.ps_threshold_high = APDS993X_PS_DETECTION_THRESHOLD;
    mTask.ps_threshold_low = 0;

    mTask.txBuf[0] = CMD_BYTE | APDS993X_PIHTL_REG;
    mTask.txBuf[1] = mTask.ps_threshold_high & 0xFF;
    i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack, (void *)STATE_IDLE);

    mTask.txBuf[0] = CMD_BYTE | APDS993X_PIHTH_REG;
    mTask.txBuf[1] = mTask.ps_threshold_high >> 8;
    i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack, (void *)STATE_IDLE);

    mTask.txBuf[0] = CMD_BYTE | APDS993X_PILTL_REG;
    mTask.txBuf[1] = mTask.ps_threshold_low & 0xFF;
    i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack, (void *)STATE_IDLE);

    mTask.txBuf[0] = CMD_BYTE | APDS993X_PILTH_REG;
    mTask.txBuf[1] = mTask.ps_threshold_low >> 8;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack, next_state);
}

static int apds993x_set_ps_enable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.txBuf[0] = CMD_BYTE | APDS993X_ENABLE_REG;
    mTask.txBuf[1] =
        mTask.rxBuf[0] | APDS993x_DD_PIEN | APDS993x_DD_PEN | APDS993x_DD_AEN | APDS993x_DD_PON;

    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack, next_state);
}

static int apds993x_ps_ratechg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int apds993x_set_time(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;

    mTask.txBuf[0] = CMD_BYTE | APDS993X_ATIME_REG;
    mTask.txBuf[1] = apds993x_als_atime_tb[APDS993X_ALS_RES_37888];

    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack, (void *)STATE_IDLE);

    mTask.txBuf[2] = CMD_BYTE | APDS993X_PTIME_REG;
    mTask.txBuf[3] = 0xFF;
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[2], 2, i2cCallBack, (void *)STATE_IDLE);

    mTask.txBuf[4] = CMD_BYTE | APDS993X_WTIME_REG;
    mTask.txBuf[5] = 0xFF;
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[4], 2, i2cCallBack, next_state);

    return ret;
}

static int apds993x_set_ppcount(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                    void *inBuf, uint8_t inSize, uint8_t elemInSize,
                    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.txBuf[0] = CMD_BYTE | APDS993X_PPCOUNT_REG;
    mTask.txBuf[1] = APDS993X_PS_PULSE_NUMBER;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack, next_state);
}

static int apds993x_set_config(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                    void *inBuf, uint8_t inSize, uint8_t elemInSize,
                    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.txBuf[0] = CMD_BYTE | APDS993X_CONFIG_REG;
    mTask.txBuf[1] = 0;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack, next_state);
}

static int apds993x_set_control(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                    void *inBuf, uint8_t inSize, uint8_t elemInSize,
                    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.txBuf[0] = CMD_BYTE | APDS993X_CONTROL_REG;
    mTask.txBuf[1] =
        APDS993X_PDRVIE_100MA | APDS993X_PRX_IR_DIOD | APDS993X_PGAIN_4X |
        apds993x_als_again_bit_tb[APDS993X_ALS_GAIN_8X];
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack, next_state);
}

static int apds993x_set_ailt(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                    void *inBuf, uint8_t inSize, uint8_t elemInSize,
                    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.txBuf[0] = CMD_BYTE | APDS993X_AILTL_REG;
    mTask.txBuf[1] = 0xFF;

    i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack, (void *)STATE_IDLE);

    mTask.txBuf[0] = CMD_BYTE | APDS993X_AILTH_REG;
    mTask.txBuf[1] = 0xFF;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack, next_state);
}

static int apds993x_set_aiht(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                 void *inBuf, uint8_t inSize, uint8_t elemInSize,
                 void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.txBuf[0] = CMD_BYTE | APDS993X_AIHTL_REG;
    mTask.txBuf[1] = 0;
    i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack, (void *)STATE_IDLE);

    mTask.txBuf[0] = CMD_BYTE | APDS993X_AIHTH_REG;
    mTask.txBuf[1] = 0;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack, next_state);
}

static int apds993x_set_pers(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.txBuf[0] = CMD_BYTE | APDS993X_PERS_REG;
    mTask.txBuf[1] = APDS993X_PPERS_2 | APDS993X_APERS_2;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack, next_state);
}

SRAM_REGION_FUNCTION static void apds993x_eint_handler(int arg)
{
    /* osLog(LOG_ERROR, "(%s)\n", __func__); */
    alsPsInterruptOccur();
}

static int apds993x_setup_eint(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mt_eint_dis_hw_debounce(mTask.hw->eint_num);

    mt_eint_registration(mTask.hw->eint_num, LEVEL_SENSITIVE, LOW_LEVEL_TRIGGER,
                     apds993x_eint_handler, EINT_INT_UNMASK, EINT_INT_AUTO_UNMASK_OFF);

    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);

    return 0;
}

static int apds993x_register_core(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    struct sensorCoreInfo mInfo;
    int ret = 0;

    /* Register sensor Core */
    memset(&mInfo, 0x00, sizeof(struct sensorCoreInfo));
    mInfo.sensType = SENS_TYPE_ALS;
    mInfo.getData = alsGetData;
    mInfo.getSensorInfo = alsGetSensorInfo;
    sensorCoreRegister(&mInfo);
    memset(&mInfo, 0x00, sizeof(struct sensorCoreInfo));
    mInfo.sensType = SENS_TYPE_PROX, mInfo.gain = 1;
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

static struct sensorFsm apds993xFsm[] = {
    /* sample als */
    sensorFsmCmd(STATE_SAMPLE_ALS, STATE_GET_ALS_DATA, apds993x_read_als),
    sensorFsmCmd(STATE_GET_ALS_DATA, STATE_SAMPLE_ALS_DONE, apds993x_calculate_als),
    /* sample ps */
    sensorFsmCmd(STATE_SAMPLE_PS, STATE_GET_PS_DATA, apds993x_read_ps),
    sensorFsmCmd(STATE_GET_PS_DATA, STATE_GET_PS_STATUS, apds993x_get_ps_status),
    sensorFsmCmd(STATE_GET_PS_STATUS, STATE_SET_PS_INT_CLEAR, apds993x_set_ps_int_clear),
    sensorFsmCmd(STATE_SET_PS_INT_CLEAR, STATE_SAMPLE_PS_DONE, apds993x_sample_ps_done),
    /* als enable state */
    sensorFsmCmd(STATE_ALS_ENABLE, STATE_ALS_POWER_ON, apds993x_set_als_enable_flag),
    sensorFsmCmd(STATE_ALS_POWER_ON, STATE_ALS_EN_POWER_OFF, apds993x_set_apds_power_off),
    sensorFsmCmd(STATE_ALS_EN_POWER_OFF, STATE_ALS_SET_DEBOUNCE, apds993x_set_als_power_on),
    sensorFsmCmd(STATE_ALS_SET_DEBOUNCE, STATE_ALS_ENABLE_DONE, apds993x_set_als_debounce_on),
    /* als disable state */
    sensorFsmCmd(STATE_ALS_DISABLE, STATE_ALS_DISABLE_DONE, apds993x_set_als_disable),
    /* ps enable state */
    sensorFsmCmd(STATE_PS_ENABLE, STATE_PS_SET_ENABLE_FLAG, apds993x_set_ps_enable_flag),
    sensorFsmCmd(STATE_PS_SET_ENABLE_FLAG, STATE_PS_EN_POWER_OFF, apds993x_set_apds_power_off),
    sensorFsmCmd(STATE_PS_EN_POWER_OFF, STATE_PS_SET_THRES, apds993x_set_ps_thres_init),
    sensorFsmCmd(STATE_PS_SET_THRES, STATE_PS_EN_POWER_ON, apds993x_set_ps_enable),
    sensorFsmCmd(STATE_PS_EN_POWER_ON, STATE_PS_ENABLE_DONE, apds993x_unmask_eint),
    /* ps disable state */
    sensorFsmCmd(STATE_PS_DISABLE, STATE_PS_MASK_EINT, apds993x_mask_eint),
    sensorFsmCmd(STATE_PS_MASK_EINT, STATE_PS_DISABLE_DONE, apds993x_set_ps_disable),
    /* als cfg state */
    sensorFsmCmd(STATE_ALS_CFG, STATE_ALS_CFG_DONE, apds993x_als_cfg),
    /* ps cfg state */
    sensorFsmCmd(STATE_PS_CFG, STATE_PS_CFG_DONE, apds993x_ps_cfg),
    /* als rate change */
    sensorFsmCmd(STATE_ALS_RATECHG, STATE_ALS_RATECHG_DONE, apds993x_als_ratechg),
    /* ps rate change */
    sensorFsmCmd(STATE_PS_RATECHG, STATE_PS_RATECHG_DONE, apds993x_ps_ratechg),

    /* init state */
    sensorFsmCmd(STATE_RESET, STATE_SET_INIT_POWER_OFF, apds993x_set_apds_power_off),
    sensorFsmCmd(STATE_SET_INIT_POWER_OFF, STATE_SET_TIME, apds993x_set_time),
    sensorFsmCmd(STATE_SET_TIME, STATE_SET_PPCOUNT, apds993x_set_ppcount),
    sensorFsmCmd(STATE_SET_PPCOUNT, STATE_SET_CONFIG, apds993x_set_config),
    sensorFsmCmd(STATE_SET_CONFIG, STATE_SET_CONTROL, apds993x_set_control),
    sensorFsmCmd(STATE_SET_CONTROL, STATE_SET_PS_THRES_INIT, apds993x_set_ps_thres_init),
    sensorFsmCmd(STATE_SET_PS_THRES_INIT, STATE_SET_AILT, apds993x_set_ailt),
    sensorFsmCmd(STATE_SET_AILT, STATE_SET_AIHT, apds993x_set_aiht),
    sensorFsmCmd(STATE_SET_AIHT, STATE_SET_PERS, apds993x_set_pers),
    sensorFsmCmd(STATE_SET_PERS, STATE_SETUP_EINT, apds993x_setup_eint),
    sensorFsmCmd(STATE_SETUP_EINT, STATE_INIT_DONE, apds993x_register_core),
};

static int apds993xInit(void)
{
    int ret = 0;

    osLog(LOG_INFO, "%s: task starting\n", __func__);
    apds993xDebugPoint = &mTask;

    mTask.hw = get_cust_alsps("ap9930");
    if (NULL == mTask.hw) {
        osLog(LOG_ERROR, "get_cust_acc_hw fail\n");
        ret = -1;
        goto err_out;
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

    mTask.txBuf[0] = CMD_BYTE | APDS993X_ID_REG;

    ret = i2cMasterTxRxSync(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                &mTask.deviceId, 1, NULL, NULL);
    if (ret < 0) {
        osLog(LOG_ERROR, "apds993x i2cMasterTxRxSync fail!!!\n");
        ret = -1;
        i2cMasterRelease(mTask.hw->i2c_num);
        goto err_out;
    }

    if (mTask.deviceId == 0x39) {
        osLog(LOG_INFO, "apds9930: detect success.\n");
    } else if (mTask.deviceId == 0x30) {
        osLog(LOG_INFO, "apds9931: detect success.\n");
    } else {
        osLog(LOG_ERROR, "apds993x: detect failed:0x%x\n", mTask.deviceId);
        ret = -1;
        goto err_out;
    }

    alsSensorRegister();
    psSensorRegister();
    registerAlsPsDriverFsm(apds993xFsm, ARRAY_SIZE(apds993xFsm));
err_out:
    return ret;
}

#ifndef CFG_OVERLAY_INIT_SUPPORT
MODULE_DECLARE(apds9930, SENS_TYPE_ALS, apds993xInit);
#else
#include "mtk_overlay_init.h"
OVERLAY_DECLARE(apds9930, OVERLAY_WORK_02, apds993xInit);
#endif
