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
#include "ap3426.h"
#include "alsps.h"

#define ALS_NAME     "ap3426_l"
#define PS_NAME      "ap3426_p"
#define I2C_SPEED                       400000

#define UNIT_MS_TO_NS   1000000

enum ap3426State {
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
    STATE_INIT_DONE = CHIP_INIT_DONE,
    STATE_IDLE = CHIP_IDLE,
    STATE_RESET = CHIP_RESET,

    STATE_SET_ALS_CONF,
    STATE_SET_PS_CONF,
    STATE_GET_ALS_VAL,
    STATE_ALS_SET_DEBOUNCE,
    STATE_GET_PS_STAT,
    STATE_GET_PS_RAW_DATA,
    STATE_PS_SET_DEBOUNCE,
    STATE_ALS_POWER_ON,
    STATE_ALS_POWER_OFF,
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

#define MAX_I2C_PER_PACKET  8
#define ICM20645_DATA_LEN   6
#define MAX_RXBUF 512
#define MAX_TXBUF (MAX_RXBUF / MAX_I2C_PER_PACKET)
SRAM_REGION_BSS static struct ap3426Task {
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
    uint32_t deviceId;
    /* data for factory */
    struct alsps_hw *hw;
    uint8_t i2c_addr;
    struct transferDataInfo dataInfo;
    struct AlsPsData data;
    uint32_t als_raw_data;
    uint32_t prox_raw_data;
    int32_t psCali;
    int32_t alsCali;
    uint32_t    ps_threshold_high;
    uint32_t    ps_threshold_low;
    uint32_t    als_threshold_high;
    uint32_t    als_threshold_low;
} mTask;
static struct ap3426Task *ap3426DebugPoint;

static void cal_end_time(unsigned int ms, uint64_t *end_tick)
{
    *end_tick = rtcGetTime() +
                ((uint64_t)ms * UNIT_MS_TO_NS);
}


static int ap3426_read_als(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                            void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    if (rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize)) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "als rx error\n");
        return -1;
    }
    mTask.txBuf[0] = AP3426_ADATA_L;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 2, i2cCallBack,
                         next_state);
}

static int ap3426_get_als_value(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                 void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                 void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.als_raw_data = ((mTask.rxBuf[1] << 8) | mTask.rxBuf[0]);
    mTask.data.als_data =
         ((AP3426_ALS_GAIN_A_RATION * (int)mTask.als_raw_data))/AP3426_FX;
    mTask.data.sensType = SENS_TYPE_ALS;
    txTransferDataInfo(&mTask.dataInfo, 1, &mTask.data);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static void alsGetData(void *sample)
{
    struct SingleAxisDataPoint *singleSample = (struct SingleAxisDataPoint *)sample;
    singleSample->idata = mTask.data.als_data;
}

static int ap3426_read_ps(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                           void *inBuf, uint8_t inSize, uint8_t elemInSize,
                           void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    if (rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize)) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "ps rx error\n");
        return -1;
    }
    mTask.txBuf[0] = AP3426_INT_STATUS;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,
                         next_state);
}


static int ap3426_get_ps_status(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                 void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                 void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    if (!(((int)mTask.rxBuf[0] & 0x10) >> 4)) {
        mTask.data.prox_state = PROX_STATE_FAR; /* far */
        mTask.data.prox_data = 1; /* far state distance is 1cm */
    } else {
        mTask.data.prox_state = PROX_STATE_NEAR; /* near */
        mTask.data.prox_data = 0; /* near state distance is 0cm */
    }
    mTask.txBuf[0] = AP3426_PDATA_L;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 2, i2cCallBack,
                         next_state);
}

static int ap3426_get_ps_raw_data(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                   void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                   void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.prox_raw_data = (mTask.rxBuf[1] & 0x3) << 8 | mTask.rxBuf[0];
    if (mTask.prox_raw_data < mTask.psCali)
        mTask.prox_raw_data = 0;
    else
        mTask.prox_raw_data -= mTask.psCali;
    mt_eint_unmask(mTask.hw->eint_num); /* MT6797 use 11 */
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

    void get_ps_data(void *cookie, size_t tx, size_t rx, int err) {
        char *rxBuf = cookie;
        if (err == 0) {
            mTask.prox_raw_data = (rxBuf[1] & 0x3) << 8 | rxBuf[0];
            if (mTask.prox_raw_data < mTask.psCali)
                mTask.prox_raw_data = 0;
            else
                mTask.prox_raw_data -= mTask.psCali;
        } else
            osLog(LOG_INFO, "psGetData error (%d)\n", err);
    }
    txBuf[0] = AP3426_PDATA_L;
    i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, txBuf, 1,
                  rxBuf, 2, get_ps_data, rxBuf);
}

void ap3426_ps_set_threshold(uint32_t threshold_high, uint32_t threshold_low)
{
    int ret;
    char txBuf[6];

    if (threshold_high == 0 && threshold_low == 0)
        return;
    txBuf[0] = AP3426_INT_PS_LOW_THD_LOW;
    txBuf[1] = (unsigned char)(threshold_high & 0xFF);
    txBuf[2] = (unsigned char)(threshold_high >> 8);
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, txBuf, 3,
                      NULL, 0);
    if (ret) {
        osLog(LOG_INFO, "ps thr_h error (%d)\n", ret);
        return;
    }
    txBuf[3] = AP3426_INT_PS_HIGH_THD_LOW;
    txBuf[4] = (unsigned char)(threshold_low & 0xFF);
    txBuf[5] = (unsigned char)(threshold_low >> 8);
    i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, &txBuf[3], 3,
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
    ap3426_ps_set_threshold(mTask.ps_threshold_high + mTask.psCali,
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
    ap3426_ps_set_threshold(mTask.ps_threshold_high, mTask.ps_threshold_low);
}

static void psGetSensorInfo(struct sensorInfo_t *data)
{
    strncpy(data->name, PS_NAME, sizeof(data->name));
}

static void alsGetSensorInfo(struct sensorInfo_t *data)
{
    strncpy(data->name, ALS_NAME, sizeof(data->name));
}

static int ap3426_set_als_debounce_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                       void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                       void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.als_debounce_on = 1;
    cal_end_time(mTask.als_debounce_time, &mTask.als_debounce_end);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int ap3426_set_ps_debounce_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                      void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                      void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.ps_debounce_on = 1;
    cal_end_time(mTask.ps_debounce_time, &mTask.ps_debounce_end);
    mTask.data.prox_state = PROX_STATE_INIT;
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int ap3426_set_als_power_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                    void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.txBuf[0] = AP3426_ENABLE;
    mTask.txBuf[1] = mTask.rxBuf[0] | 0x01;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int ap3426_set_als_power_off(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                     void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                     void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.als_debounce_on = 0;
    mTask.txBuf[0] = AP3426_ENABLE;
    mTask.txBuf[1] = mTask.rxBuf[0] & 0xFE;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int ap3426_als_ratechg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                               void *inBuf, uint8_t inSize, uint8_t elemInSize,
                               void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int ap3426_als_cali(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
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

static int ap3426_als_cfg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    struct alspsCaliCfgPacket caliCfgPacket;
    double  alsCali_mi;

    ret = rxCaliCfgInfo(&caliCfgPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);

    if (ret < 0) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "ap3426_als_cfg, rx inSize and elemSize error\n");
        return -1;
    }
    alsCali_mi = (double)((float)caliCfgPacket.caliCfgData[0] / ALS_INCREASE_NUM_AP);
    osLog(LOG_INFO, "ap3426_als_cfg: [%f]\n", alsCali_mi);
    mTask.alsCali = caliCfgPacket.caliCfgData[0];
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int ap3426_mask_eint(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mt_eint_mask(mTask.hw->eint_num);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

/*  Need add unmask operation to ratechg, if sensor has setting rate */
static int ap3426_unmask_eint(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mt_eint_unmask(mTask.hw->eint_num);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int ap3426_set_ps_power_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                   void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                   void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.txBuf[0] = AP3426_ENABLE;
    mTask.txBuf[1] = mTask.rxBuf[0] | 0x02;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int ap3426_set_ps_power_off(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                    void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.ps_debounce_on = 0;
    mTask.data.prox_state = PROX_STATE_FAR; /* default value */
    mTask.txBuf[0] = AP3426_ENABLE;
    mTask.txBuf[1] = mTask.rxBuf[0] & 0xFD;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int ap3426_ps_ratechg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int ap3426_ps_cali(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
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

static int ap3426_ps_cfg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    struct alspsCaliCfgPacket caliCfgPacket;

    ret = rxCaliCfgInfo(&caliCfgPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);

    if (ret < 0) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "ap3426_ps_cfg, rx inSize and elemSize error\n");
        return -1;
    }
    osLog(LOG_INFO, "ap3426_ps_cfg, [high, low]: [%d, %d]\n",
        caliCfgPacket.caliCfgData[0], caliCfgPacket.caliCfgData[1]);
    ap3426_ps_set_threshold(caliCfgPacket.caliCfgData[0], caliCfgPacket.caliCfgData[1]);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int ap3426_get_conf(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                               void *inBuf, uint8_t inSize, uint8_t elemInSize,
                               void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.txBuf[0] = AP3426_ENABLE;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,
                         next_state);
}

static int ap3426_reset(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;

    mTask.txBuf[0] = AP3426_ENABLE;
    mTask.txBuf[1] = 0x0;
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, (void *)STATE_IDLE);
    mTask.txBuf[2] = 0x01;
    mTask.txBuf[3] = 0x0;
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr,
                        (uint8_t*)&mTask.txBuf[2], 2,
                         i2cCallBack, next_state);
    return ret;
}

static int ap3426_set_als_conf(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;

    if (0 == mTask.hw->polling_mode_als) {
        mTask.txBuf[0] = AP3426_INT_ALS_LOW_THD_LOW;
        mTask.txBuf[1] = mTask.als_threshold_low & 0xFF;
        mTask.txBuf[2] = mTask.als_threshold_low >> 8;
        mTask.txBuf[3] = mTask.als_threshold_high & 0xFF;
        mTask.txBuf[4] = mTask.als_threshold_high >> 8;
        ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 5,
                       i2cCallBack, (void *)STATE_IDLE);
    }
    mTask.txBuf[5] = 0x10;
    mTask.txBuf[6] = 0x00;
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, (uint8_t*)&mTask.txBuf[5], 2,
                       i2cCallBack, next_state);
    return ret;
}
static int ap3426_set_ps_conf(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;

    if (0 == mTask.hw->polling_mode_ps) {
        mTask.txBuf[0] = AP3426_INT_PS_LOW_THD_LOW;
        mTask.txBuf[1] = mTask.ps_threshold_low & 0xFF;
        mTask.txBuf[2] = mTask.ps_threshold_low >> 8;
        mTask.txBuf[3] = mTask.ps_threshold_high & 0xFF;
        mTask.txBuf[4] = mTask.ps_threshold_high >> 8;
        ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 5,
                        i2cCallBack, (void *)STATE_IDLE);
    }
    mTask.txBuf[5] = 0x20;
    mTask.txBuf[6] = 0x01;
    mTask.txBuf[7] = 0x03;
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, (uint8_t*)&mTask.txBuf[5], 3,
                        i2cCallBack, (void *)STATE_IDLE);
    mTask.txBuf[8] = 0x23;
    mTask.txBuf[9] = 0x01;
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, (uint8_t*)&mTask.txBuf[8], 2,
                        i2cCallBack, (void *)STATE_IDLE);
    mTask.txBuf[10] = 0x25;
    mTask.txBuf[11] = 0x08;
    mTask.txBuf[12] = 0x01;
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, (uint8_t*)&mTask.txBuf[10], 3,
                        i2cCallBack, next_state);
    return ret;
}

SRAM_REGION_FUNCTION void ap3426_eint_handler(int arg)
{
    alsPsInterruptOccur();
}

static int ap3426_setup_eint(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
#if 1
    mt_eint_dis_hw_debounce(mTask.hw->eint_num); /* MT6797 use 11 */
    mt_eint_registration(mTask.hw->eint_num, LEVEL_SENSITIVE, LOW_LEVEL_TRIGGER, ap3426_eint_handler, EINT_INT_UNMASK,
                         EINT_INT_AUTO_UNMASK_OFF); /* MT6797 use 11 */
#endif

    mTask.txBuf[0] = 0x02;
    mTask.txBuf[1] = 0x00;
    if (0 == mTask.hw->polling_mode_ps)
        mTask.txBuf[1] |= 0x80;
    if (0 == mTask.hw->polling_mode_als)
        mTask.txBuf[1] |= 0x08;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                     i2cCallBack, next_state);
}

static int ap3426_register_core(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
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

static struct sensorFsm ap3426Fsm[] = {
    sensorFsmCmd(STATE_SAMPLE_ALS, STATE_GET_ALS_VAL, ap3426_read_als),
    sensorFsmCmd(STATE_GET_ALS_VAL, STATE_SAMPLE_ALS_DONE, ap3426_get_als_value),
    sensorFsmCmd(STATE_SAMPLE_PS, STATE_GET_PS_STAT, ap3426_read_ps),
    sensorFsmCmd(STATE_GET_PS_STAT, STATE_GET_PS_RAW_DATA, ap3426_get_ps_status),
    sensorFsmCmd(STATE_GET_PS_RAW_DATA, STATE_SAMPLE_PS_DONE, ap3426_get_ps_raw_data),
    /* als enable state */
    sensorFsmCmd(STATE_ALS_ENABLE, STATE_ALS_POWER_ON, ap3426_get_conf),
    sensorFsmCmd(STATE_ALS_POWER_ON, STATE_ALS_SET_DEBOUNCE, ap3426_set_als_power_on),
    sensorFsmCmd(STATE_ALS_SET_DEBOUNCE, STATE_ALS_ENABLE_DONE, ap3426_set_als_debounce_on),
    /* als disable state */
    sensorFsmCmd(STATE_ALS_DISABLE, STATE_ALS_POWER_OFF, ap3426_get_conf),
    sensorFsmCmd(STATE_ALS_POWER_OFF, STATE_ALS_DISABLE_DONE, ap3426_set_als_power_off),
    /* als rate state */
    sensorFsmCmd(STATE_ALS_RATECHG, CHIP_ALS_RATECHG_DONE, ap3426_als_ratechg),
    /* als cali state */
    sensorFsmCmd(STATE_ALS_CALI, CHIP_ALS_CALI_DONE, ap3426_als_cali),
    /* als cfg state */
    sensorFsmCmd(STATE_ALS_CFG, CHIP_ALS_CFG_DONE, ap3426_als_cfg),
    /* ps enable state */
    sensorFsmCmd(STATE_PS_ENABLE, STATE_PS_POWER_ON, ap3426_get_conf),
    sensorFsmCmd(STATE_PS_POWER_ON, STATE_PS_SET_DEBOUNCE, ap3426_set_ps_power_on),
    sensorFsmCmd(STATE_PS_SET_DEBOUNCE, STATE_PS_UNMASK_EINT, ap3426_set_ps_debounce_on),
    sensorFsmCmd(STATE_PS_UNMASK_EINT, STATE_PS_ENABLE_DONE, ap3426_unmask_eint),
    /* ps disable state */
    sensorFsmCmd(STATE_PS_DISABLE, STATE_PS_GET_CONF, ap3426_mask_eint),
    sensorFsmCmd(STATE_PS_GET_CONF, STATE_PS_POWER_OFF, ap3426_get_conf),
    sensorFsmCmd(STATE_PS_POWER_OFF, STATE_PS_DISABLE_DONE, ap3426_set_ps_power_off),
    /* ps rate state */
    sensorFsmCmd(STATE_PS_RATECHG, CHIP_PS_RATECHG_DONE, ap3426_ps_ratechg),
    /* ps cali state */
    sensorFsmCmd(STATE_PS_CALI, CHIP_PS_CALI_DONE, ap3426_ps_cali),
    /* ps cfg state */
    sensorFsmCmd(STATE_PS_CFG, CHIP_PS_CFG_DONE, ap3426_ps_cfg),
    /* init state */
    sensorFsmCmd(STATE_RESET, STATE_SET_ALS_CONF, ap3426_reset),
    sensorFsmCmd(STATE_SET_ALS_CONF, STATE_SET_PS_CONF, ap3426_set_als_conf),
    sensorFsmCmd(STATE_SET_PS_CONF, STATE_SETUP_EINT, ap3426_set_ps_conf),
    sensorFsmCmd(STATE_SETUP_EINT, STATE_CORE, ap3426_setup_eint),
    sensorFsmCmd(STATE_CORE, STATE_INIT_DONE, ap3426_register_core),
};

static int ap3426Init(void)
{
    int ret = 0;
    osLog(LOG_INFO, "%s: task starting\n", __func__);
    ap3426DebugPoint = &mTask;

    mTask.hw = get_cust_alsps("AP3426");
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
    osLog(LOG_ERROR, "ap3426 i2c_num: %d, i2c_addr: 0x%x\n", mTask.hw->i2c_num, mTask.i2c_addr);

    i2cMasterRequest(mTask.hw->i2c_num, I2C_SPEED);
    mTask.txBuf[0] = 0x03;

    for (uint8_t i = 0; i < 3; i++) {
        ret = i2cMasterTxRxSync(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
            mTask.rxBuf, 3, NULL, NULL);
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

    mTask.deviceId = mTask.rxBuf[0] | (mTask.rxBuf[1] << 8) | (mTask.rxBuf[2] << 16);
    osLog(LOG_INFO, "ap3426: auto detect success:0x%x\n", mTask.deviceId);

    alsSensorRegister();
    psSensorRegister();
    registerAlsPsDriverFsm(ap3426Fsm, ARRAY_SIZE(ap3426Fsm));
err_out:
    return ret;
}

#ifndef CFG_OVERLAY_INIT_SUPPORT
MODULE_DECLARE(ap3426, SENS_TYPE_ALS, ap3426Init);
#else
#include "mtk_overlay_init.h"
OVERLAY_DECLARE(ap3426, OVERLAY_ID_ALSPS, ap3426Init);
#endif

