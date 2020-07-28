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
#include "bh1726.h"
#include "alsps.h"
#include "alsps_separate.h"
#include "cache_internal.h"
#define I2C_SPEED       400000
#define ALS_NAME        "bh1726"

enum sampleAlsState {
    STATE_SAMPLE_ALS = CHIP_SAMPLING_ALS,
    STATE_SAMPLE_ALS_DONE = CHIP_SAMPLING_ALS_DONE,
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
    STATE_INIT_DONE = CHIP_PS_RESET,
    STATE_IDEL = CHIP_IDLE,
    STATE_ALS_RESET = CHIP_ALS_RESET,
    STATE_CHECK_DATA_VALID,
    STATE_GET_ALS_VAL,
    STATE_ALS_SET_DEBOUNCE,
    STATE_ALS_CONF_R,
    STATE_ALS_THDH_W,
    STATE_ALS_THDL_W,
    STATE_ALS_POWER_ON,
    STATE_ALS_POWER_OFF,
    STATE_ALS_SET_MEASURE_TIME,
    STATE_ALS_SET_GAIN,
    STATE_CORE,
};

struct scale_factor {
    unsigned char  whole;
    unsigned char  fraction;
};

#define RATIO_LEVEL_NUM 5
enum data_ch{
    DATA0 = 0,
    DATA1,
    CH_NUM
};

/*this struct's values depend on HW & environment, need debug to update*/
struct calculate_coeff{
    float ratio_level[RATIO_LEVEL_NUM];
    float data_coeff[RATIO_LEVEL_NUM][CH_NUM];
};
static const float final_coeff = 102.6 / 134.96;
static const struct calculate_coeff calcu_coeff = {
    .ratio_level = {0.157, 0.261, 1.121, 4.910, 0},
    .data_coeff = {{0.159, 0.941}, {0.403, -0.615}, {0.265, -0.084}, {0.221, -0.045}, {0, 0}}
};

#define MAX_RXBUF 8
#define MAX_TXBUF 8
#define DEBOUNCE_TIME 100000000 /*100ms*/

SRAM_REGION_BSS static struct bh1726Task {
    /* txBuf for i2c operation, fill register and fill value */
    uint8_t txBuf[MAX_TXBUF];
    /* rxBuf for i2c operation, receive rawdata */
    uint8_t rxBuf[MAX_RXBUF];
    uint8_t deviceId;
    /* data for factory */
    struct alsps_hw *hw;
    uint8_t i2c_addr;
    uint32_t rawData[CH_NUM];
    struct transferDataInfo dataInfo;
    struct AlsPsData data;
    int32_t alsCali;
    uint64_t enableTime;
    uint64_t sampleTime;
    uint64_t debounceTime;
    uint8_t debounce_on;
} mTask;
static struct bh1726Task *bh1726DebugPoint;

static int bh1726_check_status(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                            void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    if (rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize)) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "als rx error\n");
        return -1;
    }
    mTask.txBuf[0] = BH1726_REG_CTRL;
    //osLog(LOG_INFO, "check status\n");/*lzc*/
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,
                         next_state);
}


static int bh1726_read_als(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                            void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.sampleTime = rtcGetTime();
    mTask.debounceTime = mTask.sampleTime - mTask.enableTime;
    if(mTask.debounceTime > DEBOUNCE_TIME)
        mTask.debounce_on = 0;
    if (mTask.rxBuf[0] & BH1726_DATA_VALID) {
        mTask.txBuf[0] = BH1726_REG_DATA0_L;
        return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
               mTask.rxBuf, 4, i2cCallBack, next_state);
    } else {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
        //osLog(LOG_ERROR, "Invaild als data!\n");
        return -1;
    }
}

float getLuxFromData()
{
    float rawData0 = mTask.rawData[DATA0];
    float rawData1 = mTask.rawData[DATA1];
    float ratio, data0_lux, data1_lux, lux = 0.0f, lux_temp = 0.0f;
    if (rawData0 == 0.0f)
        return lux;
    ratio = rawData1/rawData0;
    if (ratio < calcu_coeff.ratio_level[0]){
        data0_lux = calcu_coeff.data_coeff[0][DATA0] * rawData0;
        data1_lux = calcu_coeff.data_coeff[0][DATA1] * rawData1;
    }
    else if (ratio < calcu_coeff.ratio_level[1]){
        data0_lux = calcu_coeff.data_coeff[1][DATA0] * rawData0;
        data1_lux = calcu_coeff.data_coeff[1][DATA1] * rawData1;
    }
    else if (ratio < calcu_coeff.ratio_level[2]){
        data0_lux = calcu_coeff.data_coeff[2][DATA0] * rawData0;
        data1_lux = calcu_coeff.data_coeff[2][DATA1] * rawData1;
    }
    else if (ratio < calcu_coeff.ratio_level[3]){
        data0_lux = calcu_coeff.data_coeff[3][DATA0] * rawData0;
        data1_lux = calcu_coeff.data_coeff[3][DATA1] * rawData1;
    }
    else {
        lux= 0;
        return lux;
    }

    lux_temp = data0_lux + data1_lux;
    lux = lux_temp * final_coeff;
    return lux;
}

static int bh1726_get_als_value(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                 void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                 void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    uint8_t count = 1;
    mTask.rawData[DATA0] = ((mTask.rxBuf[1] << 8) | mTask.rxBuf[0]);
    mTask.rawData[DATA1] = ((mTask.rxBuf[3] << 8) | mTask.rxBuf[2]);
    mTask.data.als_data = (int)getLuxFromData();  //aal mapping value
    mTask.data.sensType = SENS_TYPE_ALS;
    if(mTask.debounce_on)
        count = 0;
    txTransferDataInfo(&mTask.dataInfo, count, &mTask.data);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static void alsGetData(void *sample)
{
    struct SingleAxisDataPoint *singleSample = (struct SingleAxisDataPoint *)sample;
    singleSample->idata = getLuxFromData();
}

static void alsGetSensorInfo(struct sensorInfo_t *data)
{
    strncpy(data->name, ALS_NAME, sizeof(data->name));
}

static int bh1726_set_als_power_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                    void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.debounce_on = 1;
    mTask.enableTime = rtcGetTime();
    mTask.txBuf[0] = BH1726_REG_CTRL;
    mTask.txBuf[1] = BH1726_POWER_ON | BH1726_ADC_ENABLE;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int bh1726_set_als_power_off(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                     void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                     void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.enableTime = 0;
    mTask.sampleTime = 0;
    mTask.debounce_on = 0;
    mTask.txBuf[0] = BH1726_REG_CTRL;
    mTask.txBuf[1] = BH1726_POWER_OFF;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int bh1726_als_ratechg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                               void *inBuf, uint8_t inSize, uint8_t elemInSize,
                               void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int bh1726_als_cali(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
/*temp set the cali value = 0, if enable the feature, please rewrite here*/
    int32_t alsCali[2] = {0, 0};
    alsPsSendCalibrationResult(SENS_TYPE_ALS, alsCali);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int bh1726_als_cfg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
/*Align als_cali not need work now*/
#if 0
    int ret = 0;
    struct alspsCaliCfgPacket caliCfgPacket;
    double  alsCali_mi;

    ret = rxCaliCfgInfo(&caliCfgPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);

    if (ret < 0) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "sampleAls_als_cfg, rx inSize and elemSize error\n");
        return -1;
    }
    alsCali_mi = (double)((float)caliCfgPacket.caliCfgData[0] / ALS_INCREASE_NUM_AP);
    osLog(LOG_INFO, "sampleAls_als_cfg: [%f]\n", alsCali_mi);
    mTask.alsCali = caliCfgPacket.caliCfgData[0];
#endif
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int bh1726_als_sw_reset(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.txBuf[0] = BH1726_SW_RESET;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                       i2cCallBack, next_state);
}

static int bh1726_set_als_measure_time(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.txBuf[0] = BH1726_REG_TIMING;
    mTask.txBuf[1] = 0xE3;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int bh1726_set_als_gain(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.txBuf[0] = BH1726_REG_GAIN;
    mTask.txBuf[1] = BH1726_DATA_GAIN_1x;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int bh1726_register_core(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                 void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                 void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    struct sensorCoreInfo mInfo;
    memset(&mInfo, 0x00, sizeof(struct sensorCoreInfo));
    /* Register sensor Core */
    mInfo.sensType = SENS_TYPE_ALS;
    mInfo.getData = alsGetData;
    mInfo.getSensorInfo = alsGetSensorInfo;
    sensorCoreRegister(&mInfo);

    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static struct sensorFsm bh1726Fsm[] = {
    sensorFsmCmd(STATE_SAMPLE_ALS, STATE_CHECK_DATA_VALID, bh1726_check_status),
    sensorFsmCmd(STATE_CHECK_DATA_VALID, STATE_GET_ALS_VAL, bh1726_read_als),
    sensorFsmCmd(STATE_GET_ALS_VAL, STATE_SAMPLE_ALS_DONE, bh1726_get_als_value),
    /* als enable state */
    sensorFsmCmd(STATE_ALS_ENABLE, STATE_ALS_ENABLE_DONE, bh1726_set_als_power_on),
    /* als disable state */
    sensorFsmCmd(STATE_ALS_DISABLE, STATE_ALS_DISABLE_DONE, bh1726_set_als_power_off),
    /* als rate state */
    sensorFsmCmd(STATE_ALS_RATECHG, CHIP_ALS_RATECHG_DONE, bh1726_als_ratechg),
    /* als cali state */
    sensorFsmCmd(STATE_ALS_CALI, CHIP_ALS_CALI_DONE, bh1726_als_cali),
    /* als cfg state */
    sensorFsmCmd(STATE_ALS_CFG, CHIP_ALS_CFG_DONE, bh1726_als_cfg),
    /* init state */
    sensorFsmCmd(STATE_ALS_RESET, STATE_ALS_SET_MEASURE_TIME, bh1726_als_sw_reset),
    sensorFsmCmd(STATE_ALS_SET_MEASURE_TIME, STATE_ALS_SET_GAIN, bh1726_set_als_measure_time),
    sensorFsmCmd(STATE_ALS_SET_GAIN, STATE_CORE, bh1726_set_als_gain),
    sensorFsmCmd(STATE_CORE, STATE_INIT_DONE, bh1726_register_core),
};

static int bh1726Init(void)
{
    int ret = 0;
    bh1726DebugPoint = &mTask;

    mTask.hw = get_cust_alsps("bh1726");
    if (NULL == mTask.hw) {
        osLog(LOG_ERROR, "get_cust_acc_hw fail\n");
        return 0;
    }
    mTask.i2c_addr = mTask.hw->i2c_addr[0];
    mTask.alsCali = 0;
    osLog(LOG_ERROR, "als i2c_num: %d, i2c_addr: 0x%x\n", mTask.hw->i2c_num, mTask.i2c_addr);

    i2cMasterRequest(mTask.hw->i2c_num, I2C_SPEED);

    mTask.txBuf[0] = BH1726_REG_ID;

    ret = i2cMasterTxRxSync(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
        &mTask.deviceId, 1, NULL, NULL);
    if (ret < 0) {
        osLog(LOG_ERROR, "bh1726 i2cMasterTxRxSync fail!!!\n");
        ret = -1;
        i2cMasterRelease(mTask.hw->i2c_num);
        goto err_out;
    }
    osLog(LOG_INFO, "bh1726: auto detect success:0x%x\n", mTask.deviceId);
    alsSensorRegister();
    registerAlsSeparateFsm(bh1726Fsm, ARRAY_SIZE(bh1726Fsm));
err_out:
    return ret;
}

#ifndef CFG_OVERLAY_INIT_SUPPORT
MODULE_DECLARE(bh1726, SENS_TYPE_ALS, bh1726Init);
#else
#include "mtk_overlay_init.h"
OVERLAY_DECLARE(bh1726, OVERLAY_ID_ALSPS, bh1726Init);
#endif

