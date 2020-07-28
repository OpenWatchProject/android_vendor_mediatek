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
//#include "eint.h"
#include "vcnl36832.h"
#include "alsps_separate.h"
#include "alsps.h"

#define I2C_SPEED   400000
#define PS_NAME		"vcnl36832"

#define VCNL36832_DEVICE_ID_CHECK //check VCNL36832 DEVICE ID

#define PS_DEBOUNCE_TIME    20000000 /*20ms*/
#define PS_CALI_ARR_NUM     3


//temporary use
//no glass cover
#define PS_CODE_NEAR_THD 900
#define PS_CODE_FAR_THD  320

enum vcnl36832State {
    STATE_SAMPLE_PS = CHIP_SAMPLING_PS,
    STATE_SAMPLE_PS_DONE = CHIP_SAMPLING_PS_DONE,
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
    STATE_IDEL = CHIP_IDLE,
    STATE_RESET = CHIP_PS_RESET,

    STATE_GET_PS_STAT,
    STATE_GET_PS_RAW_DATA,
    STATE_PS_CONF_R,
    STATE_PS_CONF1_2_W,
    STATE_PS_CONF3_MS_W,
    STATE_PS_CANC_W,
    STATE_PS_POWER_ON,
    STATE_PS_POWER_OFF,
    STATE_CORE
};

#define MAX_RXBUF 8
#define MAX_TXBUF 8
SRAM_REGION_BSS static struct vcnl36832Task {
    /* txBuf for i2c operation, fill register and fill value */
    uint64_t    ps_debounce_time;
    uint64_t    ps_enable_time;
    uint64_t    ps_sample_time;
    unsigned int    ps_debounce_on;     /*indicates if the debounce is on*/
    bool startCali;
    uint8_t txBuf[MAX_TXBUF];
    /* rxBuf for i2c operation, receive rawdata */
    uint8_t rxBuf[MAX_RXBUF];
    uint32_t deviceId;
    /* data for factory */
    struct alsps_hw *hw;
    uint8_t i2c_addr;
    struct transferDataInfo dataInfo;
    struct AlsPsData data[1];
    uint32_t prox_raw_data;
    int32_t psCali;
    uint32_t caliCount;
    int32_t caliResult;
    uint32_t ps_threshold_high;
    uint32_t ps_threshold_low;

    uint8_t ps_conf1[2];
} mTask;

static struct vcnl36832Task *vcnl36832DebugPoint;


static int vcnl36832_get_ps_raw_data(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                   void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                   void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    if (rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize)) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "vcnl36832 ps, rx dataInfo error\n");
        return -1;
    }
    /*debounce finish check*/
    mTask.ps_sample_time = rtcGetTime();
    mTask.ps_debounce_time = mTask.ps_sample_time - mTask.ps_enable_time;
    if (mTask.ps_debounce_time > PS_DEBOUNCE_TIME)
        mTask.ps_debounce_on = 0;
    //ps polling mode read VCNL36832_REG_PS_DATA
    mTask.txBuf[0] = VCNL36832_REG_PS_DATA;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 2, i2cCallBack,
                         next_state);
}

static int vcnl36832_get_ps_status(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                 void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                 void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int psDataSize = 0;
    int32_t threshold[2];
    mTask.prox_raw_data = (mTask.rxBuf[1]<<8)| mTask.rxBuf[0];
    /*using for factory cali*/
    if (mTask.startCali) {
        if ((mTask.caliCount < PS_CALI_ARR_NUM) && (mTask.ps_debounce_on == 0)) {
            mTask.caliResult +=  mTask.prox_raw_data;
            mTask.caliCount ++;
            if (mTask.caliCount == PS_CALI_ARR_NUM - 1) {
                osLog(LOG_INFO, "Ps cali start, caliresult=%d\n", mTask.caliResult);
                mTask.caliResult /= PS_CALI_ARR_NUM;
                mTask.startCali = false;
                mTask.caliCount = 0;
                mTask.ps_threshold_high += mTask.caliResult;
                mTask.ps_threshold_low += mTask.caliResult;
                threshold[0] = mTask.ps_threshold_high;
                threshold[1] = mTask.ps_threshold_low;
                alsPsSendCalibrationResult(SENS_TYPE_PROX, threshold);
            }
        }
    }
    if ( mTask.prox_raw_data <= mTask.ps_threshold_low &&  mTask.data[0].prox_state == PROX_STATE_NEAR ) {
        mTask.data[0].prox_state = PROX_STATE_FAR; /* far */
        mTask.data[0].prox_data = 1; /* far state distance is 1cm */
    } else if ( mTask.prox_raw_data >= mTask.ps_threshold_high &&  mTask.data[0].prox_state == PROX_STATE_FAR) {
        mTask.data[0].prox_state = PROX_STATE_NEAR; /* near */
        mTask.data[0].prox_data = 0; /* near state distance is 0cm */
    }

    if (mTask.ps_debounce_on == 0) {
        psDataSize = 1;
    }
    mTask.data[0].sensType = SENS_TYPE_PROX;
    txTransferDataInfo(&mTask.dataInfo, psDataSize, &mTask.data[0]);
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
    txBuf[0] = VCNL36832_REG_PS_DATA;
    i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, txBuf, 1,
                  rxBuf, 2, get_ps_data, rxBuf);
}

static void vcnl36832_ps_set_threshold(uint32_t threshold_high, uint32_t threshold_low) {

    if ((threshold_high == 0 && threshold_low == 0) || threshold_high < threshold_low){
        osLog(LOG_ERROR, "Invalid threshold value: all zero or High < low!\n");
        return;
    }
    mTask.ps_threshold_high = threshold_high;
    mTask.ps_threshold_low = threshold_low;
    return;
}

static void psGetSensorInfo(struct sensorInfo_t *data)
{
    strncpy(data->name, PS_NAME, sizeof(data->name));
}

static void psGetCalibration(int32_t *cali, int32_t size)
{
    cali[0] = mTask.psCali;
}

static void psSetCalibration(int32_t *cali, int32_t size)
{
    mTask.psCali = cali[0];
    vcnl36832_ps_set_threshold(mTask.ps_threshold_high + mTask.psCali,
                             mTask.ps_threshold_low + mTask.psCali);
}

static void psGetThreshold(uint32_t *threshold_high, uint32_t *threshold_low)
{
    *threshold_high = mTask.ps_threshold_high;
    *threshold_low = mTask.ps_threshold_low;
}

static void psSetThreshold(uint32_t threshold_high, uint32_t threshold_low)
{
    vcnl36832_ps_set_threshold(threshold_high, threshold_low);
}

static int vcnl36832_set_ps_power_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                   void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                   void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.ps_enable_time = rtcGetTime();
    mTask.ps_debounce_on = 1;
    mTask.data[0].prox_state = PROX_STATE_FAR; //default setting
    mTask.data[0].prox_data = 1; //default setting
    // Startup sequence
    // ALSCONF 0x0201|(1 << 1)
    mTask.txBuf[0] = VCNL36832_REG_ALS_CONF;
    mTask.txBuf[1] = 0x01|(1 << 1);
    mTask.txBuf[2] = 0x02;
    i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3,
                       NULL, NULL);

    // ALSCONF = 0x0203 | VCNL36832_ALS_START
    mTask.txBuf[1] = 0x03|(1 << 7);
    mTask.txBuf[2] = 0x02;
    i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3,
                       NULL, NULL);
    //power on ps
    mTask.ps_conf1[0] = mTask.rxBuf[0];
    mTask.ps_conf1[1] = mTask.rxBuf[1];

    mTask.ps_conf1[0] &= 0xFE;
    //mTask.ps_conf1[0] = 0b10001100;
    mTask.txBuf[0] = VCNL36832_REG_PS_CONF1_2;
    mTask.txBuf[1] = mTask.ps_conf1[0];
    mTask.txBuf[2] = mTask.ps_conf1[1];

    i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3,
                       i2cCallBack, next_state);
    return 0;
}

static int vcnl36832_set_ps_power_off(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                    void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    /*debounce end*/
    mTask.ps_enable_time = 0;
    mTask.ps_sample_time = 0;
    mTask.ps_debounce_time = 0;
    mTask.ps_debounce_on = 0;

    mTask.ps_conf1[0] = mTask.rxBuf[0];
    mTask.ps_conf1[1] = mTask.rxBuf[1];
    mTask.ps_conf1[0] |= 0x01;
    mTask.data[0].prox_state = PROX_STATE_FAR; /* default value */
    mTask.data[0].prox_data = 1; //default setting

    //power off ps
    mTask.txBuf[0] = VCNL36832_REG_PS_CONF1_2;
    mTask.txBuf[1] = mTask.ps_conf1[0];
    mTask.txBuf[2] = mTask.ps_conf1[1];

    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3,
                       i2cCallBack, next_state);
    // power off sequence
    // ALSCONF = 0x0283 & (~(1 << 1))
    mTask.txBuf[0] = VCNL36832_REG_ALS_CONF;
    mTask.txBuf[1] = 0x83&(~(1 << 1));
    mTask.txBuf[2] = 0x02;

    i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3,
                       i2cCallBack, next_state);
}

static int vcnl36832_ps_cali(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    /*Not sure now!*/
    mTask.startCali = true;
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int vcnl36832_ps_cfg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    int ret = 0;
    struct alspsCaliCfgPacket caliCfgPacket;

    ret = rxCaliCfgInfo(&caliCfgPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);

    if (ret < 0) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "tmd2702_ps_cfg, rx inSize and elemSize error\n");
        return -1;
    }
    vcnl36832_ps_set_threshold(caliCfgPacket.caliCfgData[0], caliCfgPacket.caliCfgData[1]);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int vcnl36832_ps_ratechg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int vcnl36832_get_ps_conf(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                               void *inBuf, uint8_t inSize, uint8_t elemInSize,
                               void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.txBuf[0] = VCNL36832_REG_PS_CONF1_2;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 2, i2cCallBack,
                         next_state);
}

static int vcnl36832_set_ps_conf1_2(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                  void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                  void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //4T, ps polling mode
    //polling (reg,command) = (0x03, 0x8C00)
    mTask.ps_conf1[1] = 0b00000000;
    mTask.ps_conf1[0] = 0b10001100;

    mTask.txBuf[0] = VCNL36832_REG_PS_CONF1_2;
    mTask.txBuf[1] = mTask.ps_conf1[0];
    mTask.txBuf[2] = mTask.ps_conf1[1];
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3,
                       i2cCallBack, next_state);
}

static int vcnl36832_set_ps_conf3_ms(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                   void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                   void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //(reg,command) = (0x04, 0xe408)
    mTask.txBuf[0] = VCNL36832_REG_PS_CONF3_4;
    mTask.txBuf[1] = 0b00001000;
    mTask.txBuf[2] = 0b11100100;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3,
                       i2cCallBack, next_state);
}

static int vcnl36832_set_ps_canc(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                               void *inBuf, uint8_t inSize, uint8_t elemInSize,
                               void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.txBuf[0] = VCNL36832_REG_PS_CANC;
    mTask.txBuf[1] = 0x00;
    mTask.txBuf[2] = 0x00;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3,
                       i2cCallBack, next_state);
}

static int vcnl36832_register_core(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                 void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                 void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    struct sensorCoreInfo mInfo;
    int ret = 0;

    /* Register sensor Core */
    memset(&mInfo, 0x00, sizeof(struct sensorCoreInfo));
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

static struct sensorFsm vcnl36832Fsm[] = {
    sensorFsmCmd(STATE_SAMPLE_PS, STATE_GET_PS_RAW_DATA, vcnl36832_get_ps_raw_data),
    sensorFsmCmd(STATE_GET_PS_RAW_DATA, STATE_SAMPLE_PS_DONE, vcnl36832_get_ps_status),
    /* ps cali */
    sensorFsmCmd(STATE_PS_CALI, STATE_PS_CALI_DONE, vcnl36832_ps_cali),
    /*ps cfg*/
    sensorFsmCmd(STATE_PS_CFG, STATE_PS_CFG_DONE, vcnl36832_ps_cfg),
    /* ps enable state */
    sensorFsmCmd(STATE_PS_ENABLE, STATE_PS_POWER_ON, vcnl36832_get_ps_conf),
    sensorFsmCmd(STATE_PS_POWER_ON, STATE_PS_ENABLE_DONE, vcnl36832_set_ps_power_on),
    /* ps disable state */
    sensorFsmCmd(STATE_PS_DISABLE, STATE_PS_POWER_OFF, vcnl36832_get_ps_conf),
    sensorFsmCmd(STATE_PS_POWER_OFF, STATE_PS_DISABLE_DONE, vcnl36832_set_ps_power_off),
    /* ps rate state */
    sensorFsmCmd(STATE_PS_RATECHG, CHIP_PS_RATECHG_DONE, vcnl36832_ps_ratechg),
    /* init state */
    sensorFsmCmd(STATE_RESET, STATE_PS_CONF1_2_W, vcnl36832_get_ps_conf),
    sensorFsmCmd(STATE_PS_CONF1_2_W, STATE_PS_CONF3_MS_W, vcnl36832_set_ps_conf1_2),
    sensorFsmCmd(STATE_PS_CONF3_MS_W, STATE_PS_CANC_W, vcnl36832_set_ps_conf3_ms),
    sensorFsmCmd(STATE_PS_CANC_W, STATE_CORE, vcnl36832_set_ps_canc),
    sensorFsmCmd(STATE_CORE, STATE_INIT_DONE, vcnl36832_register_core),
};

static int vcnl36832Init(void)
{
    int ret = 0;
    vcnl36832DebugPoint = &mTask;

    mTask.hw = get_cust_alsps("vcnl36832");
    if (NULL == mTask.hw) {
        osLog(LOG_ERROR, "get_cust_acc_hw fail\n");
        return 0;
    }
    mTask.i2c_addr = mTask.hw->i2c_addr[0];
    mTask.ps_enable_time = 0;
    mTask.ps_sample_time = 0;
    mTask.ps_debounce_time = 0;
    mTask.ps_debounce_on = 0;
    mTask.psCali = 0;
    mTask.startCali = false;
    mTask.caliCount = 0;

    mTask.ps_threshold_high = mTask.hw->ps_threshold_high;
    mTask.ps_threshold_low = mTask.hw->ps_threshold_low;
/*for temp test, mask it when the system is really running*/
    mTask.ps_threshold_high = PS_CODE_NEAR_THD;
    mTask.ps_threshold_low = PS_CODE_FAR_THD;
    osLog(LOG_ERROR, "acc i2c_num: %d, i2c_addr: 0x%x\n", mTask.hw->i2c_num, mTask.i2c_addr);

    i2cMasterRequest(mTask.hw->i2c_num, I2C_SPEED);

    mTask.txBuf[0] = VCNL36832_REG_ID;

    ret = i2cMasterTxRxSync(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
        &mTask.deviceId, 2, NULL, NULL);
    if (ret < 0) {
        ret = -1;
        i2cMasterRelease(mTask.hw->i2c_num);
        goto err_out;
    }

#ifdef VCNL36832_DEVICE_ID_CHECK
    if (mTask.deviceId == 0x0358) {  //vcnl36832 device id is fixed
        goto success_out;
    } else {
        i2cMasterRelease(mTask.hw->i2c_num);
        osLog(LOG_ERROR, "vcnl36832: read id fail!!!\n");
        ret = -1;
        goto err_out;
    }
#endif
success_out:
    psSensorRegister();
    registerPsSeparateFsm(vcnl36832Fsm, ARRAY_SIZE(vcnl36832Fsm));
    registerPsInterruptMode(PS_POLLING_MODE); /*using polling mode*/
err_out:
    return ret;
}

#ifndef CFG_OVERLAY_INIT_SUPPORT
MODULE_DECLARE(vcnl36832, SENS_TYPE_ALS, vcnl36832Init);
#else
#include "mtk_overlay_init.h"
OVERLAY_DECLARE(vcnl36832, OVERLAY_ID_ALSPS_SECONDARY, vcnl36832Init);
#endif
