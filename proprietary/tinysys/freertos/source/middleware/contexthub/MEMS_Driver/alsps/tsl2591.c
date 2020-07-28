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
#include "alsps.h"
#include "tsl2591.h"
#include "alsps_separate.h"
#include "cache_internal.h"
#define I2C_SPEED                       400000
#define ALS_NAME                        "tsl2591"


enum tsl2591State {
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

    STATE_ALS_POWER_DELAY,
    STATE_GET_ALS_VAL,
    STATE_GET_ALS_EN,
    STATE_SET_ALS_GAIN,
    STATE_SET_ALS_DIS,
    STATE_SET_ALS_EN,
    STATE_GET_ALS_LUX,
    STATE_ALS_PST,
    STATE_CORE
};

struct scale_factor {
    unsigned char  whole;
    unsigned char  fraction;
};

struct tsl2591 {
    uint32_t als_sat;
    uint16_t als_ch0;
    uint16_t als_ch1;
    uint8_t  als_time;
    uint8_t  als_gain;
    uint8_t  als_cpl;
    uint8_t  als_control;
    uint8_t  als_enable;
};

#define MAX_RXBUF 8
#define MAX_TXBUF 8

SRAM_REGION_BSS static struct tsl2591Task {
    /* txBuf for i2c operation, fill register and fill value */
    bool alsPowerOn;
    unsigned int    als_debounce_time;
    unsigned int    als_debounce_on;    /*indicates if the debounce is on*/
    portTickType    als_debounce_end;
    uint8_t txBuf[MAX_TXBUF];
    /* rxBuf for i2c operation, receive rawdata */
    uint8_t rxBuf[MAX_RXBUF];
    uint8_t deviceId;
    /* data for factory */
    struct alsps_hw *hw;
    uint8_t i2c_addr;
    struct transferDataInfo dataInfo;
    struct AlsPsData data;
    int32_t alsCali;
    uint32_t    als_threshold_high;
    uint32_t    als_threshold_low;
    /* add for tsl2591 */
    struct tsl2591 chip;
    int prev_lux;
} mTask;
static struct tsl2591Task *tsl2591DebugPoint;

static uint16_t const als_gains[] = {
    1,
    25,
    400,
    9200
};

static int tsl2591_read_status(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                            void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    if (rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize)) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "als rx error\n");
        return -1;
    }
    mTask.txBuf[0] = TSL2591_STATUS_REG;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack, next_state);
}

static int tsl2591_read_raw(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                            void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    if (mTask.rxBuf[0] & AVALID) {
        mTask.txBuf[0] = TSL2591_CH0L_REG;
        return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 4, i2cCallBack, next_state);
    } else {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "Invalid als data\n");
        return -1;
    }
}

static int tsl2591_get_enable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                            void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.chip.als_ch0 = (mTask.rxBuf[1] << 8) | mTask.rxBuf[0];
    mTask.chip.als_ch1 = (mTask.rxBuf[3] << 8) | mTask.rxBuf[2];
    // osLog(LOG_INFO, "ch0:%d, ch1:%d\n", mTask.chip.als_ch0, mTask.chip.als_ch1);
    mTask.txBuf[0] = TSL2591_ENABLE_REG;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                        mTask.rxBuf, 2, i2cCallBack, next_state);
}

static int tsl2591_set_gain(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                            void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    uint8_t again = 0, atime = 0;
    uint16_t gain;

    mTask.chip.als_enable = mTask.rxBuf[0];
    mTask.chip.als_control = mTask.rxBuf[1];
    // osLog(LOG_INFO, "control reg: 0x%x\n", mTask.rxBuf[1]);

    atime = mTask.chip.als_time >> ATIME_SHIFT;
    again = mTask.chip.als_gain >> AGAIN_SHIFT;
    mTask.chip.als_sat = (atime == 0) ? 36863:65535;
    mTask.chip.als_sat = mTask.chip.als_sat * 8 / 10;
    gain = als_gains[again];
    mTask.chip.als_cpl = (atime + 1) * 100 * als_gains[again];

    mTask.txBuf[0] = TSL2591_CONTROL_REG;
        mTask.chip.als_control &= ~AGAIN_MASK;
    if (gain == 400 && mTask.chip.als_ch0 > mTask.chip.als_sat) {
        mTask.chip.als_control |= AGAIN_MED;
        mTask.chip.als_gain =  AGAIN_MED;
    } else if (gain == 400 && mTask.chip.als_ch0 < GAIN_SWITCH_LEVEL) {
         mTask.chip.als_control |= AGAIN_MAX;
         mTask.chip.als_gain =  AGAIN_MAX;
    } else if ((gain == 9200 && mTask.chip.als_ch0 >= mTask.chip.als_sat) ||
                   (gain == 25 && mTask.chip.als_ch0 < GAIN_SWITCH_LEVEL)) {
        mTask.chip.als_control |= AGAIN_HIGH;
        mTask.chip.als_gain =  AGAIN_MAX;
    }

    mTask.txBuf[1] = mTask.chip.als_control;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                           i2cCallBack, next_state);
}

/* Disable then enable als , To take gain register effect immediately  */
static int tsl2591_set_als_dis(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                            void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.txBuf[0] = TSL2591_ENABLE_REG;
    mTask.chip.als_enable &= ~AEN;
    mTask.txBuf[1] = mTask.chip.als_enable;

    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                                 i2cCallBack, next_state);
}

static int tsl2591_set_als_en(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                            void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.txBuf[0] = TSL2591_ENABLE_REG;
    mTask.chip.als_enable &= ~AEN;
    mTask.chip.als_enable |= AEN;
    mTask.txBuf[1] = mTask.chip.als_enable;

    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                                         i2cCallBack, next_state);
}

static int getLuxFromAlsData(void) {
    int lux = 0, lux1 = 0, lux2 = 0;
    uint8_t again = 0;

    again = mTask.chip.als_gain >> AGAIN_SHIFT;

    if ((mTask.chip.als_ch0 <= MIN_ALS_VALUE) || (mTask.chip.als_ch0 >= mTask.chip.als_sat))
        lux = mTask.prev_lux;
    else {
        if (als_gains[again] == 25) {
            mTask.chip.als_ch0 = (mTask.chip.als_ch0 * 250) / 245;
            mTask.chip.als_ch1 = (mTask.chip.als_ch0 * 250) / 245;
        } else if (als_gains[again] == 9200)
            mTask.chip.als_ch1 = mTask.chip.als_ch1 * 9200 / 9900;

        lux1 = (mTask.chip.als_ch0 * 1000) - mTask.chip.als_ch1 * B_Coef;
        lux2 = (mTask.chip.als_ch0 * C_Coef) - mTask.chip.als_ch1 * D_Coef;

        if (lux2 > lux1) {
            lux1 = lux2;
        }
        if (lux1 < 0) {
            lux1 = 0;
        }
        lux = lux1;

        if (lux > 100 * mTask.chip.als_cpl) {
            lux /= mTask.chip.als_cpl;
            lux *= D_Factor;
        } else {
            lux *= D_Factor;
            lux /= mTask.chip.als_cpl;
        }

        lux = (lux + 512) >> 10;
        // osLog(LOG_INFO, "lux %d\n", lux);
    }
    mTask.prev_lux = lux;
    return lux;
}
static int tsl2591_get_als_lux(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                 void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                 void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.data.als_data = getLuxFromAlsData();
    mTask.data.sensType = SENS_TYPE_ALS;
    txTransferDataInfo(&mTask.dataInfo, 1, &mTask.data);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static void alsGetData(void *sample) {
    struct SingleAxisDataPoint *singleSample = (struct SingleAxisDataPoint *)sample;
    singleSample->idata = getLuxFromAlsData();
}

static void alsGetSensorInfo(struct sensorInfo_t *data) {
    strncpy(data->name, ALS_NAME, sizeof(data->name));
}

static int tsl2591_set_als_enable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                       void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                       void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.txBuf[0] = TSL2591_ENABLE_REG;
    mTask.txBuf[1] = (AEN | PON);
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int tsl2591_set_als_power_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                    void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    osLog(LOG_INFO, "tsl power on\n");
    mTask.txBuf[0] = TSL2591_ENABLE_REG;
    mTask.txBuf[1] = PON;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

struct pri_data {
    I2cCallbackF i2cCallBack;
    void *state;
} pdata;

struct pri_data pdata;

static void sensorDelayCallBck(uint32_t timerId, void *data) {
    struct pri_data *tdata = (struct pri_data *)data;
    sensorFsmEnqueueFakeI2cEvt(tdata->i2cCallBack, tdata->state, SUCCESS_EVT);
}

static int tsl2591_set_als_power_delay(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                    void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    pdata.i2cCallBack = i2cCallBack;
    pdata.state = next_state;
    timTimerSet(30000000, 0, 50, sensorDelayCallBck, (void *)&pdata, true);
    return 0;
}

static int tsl2591_set_als_power_off(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                     void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                     void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.als_debounce_on = 0;
    mTask.txBuf[0] = TSL2591_ENABLE_REG;
    mTask.txBuf[1] = 0;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int tsl2591_als_ratechg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                               void *inBuf, uint8_t inSize, uint8_t elemInSize,
                               void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int tsl2591_als_cali(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    int32_t alsCali[2];
    float   alsCali_mi;

    alsCali_mi = 0.345;
    alsCali[0] = alsCali_mi * ALS_INCREASE_NUM_AP;
    alsCali[1] = 0;

    alsPsSendCalibrationResult(SENS_TYPE_ALS, alsCali);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int tsl2591_als_cfg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    int ret = 0;
    struct alspsCaliCfgPacket caliCfgPacket;
    double  alsCali_mi;

    ret = rxCaliCfgInfo(&caliCfgPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);

    if (ret < 0) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "tsl2591_als_cfg, rx inSize and elemSize error\n");
        return -1;
    }
    alsCali_mi = (double)((float)caliCfgPacket.caliCfgData[0] / ALS_INCREASE_NUM_AP);
    osLog(LOG_INFO, "tsl2591_als_cfg: [%f]\n", alsCali_mi);
    mTask.alsCali = caliCfgPacket.caliCfgData[0];
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int tsl2591_set_als_control(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                    void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.txBuf[0] = TSL2591_CONTROL_REG;
    mTask.txBuf[1] = mTask.chip.als_time | mTask.chip.als_gain;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int tsl2591_set_als_persist(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                    void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.txBuf[0] = TSL2591_PERSIST_REG;
    mTask.txBuf[1] = 0;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int tsl2591_register_core(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                 void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                 void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
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

static struct sensorFsm tsl2591AlsFsm[] = {
    sensorFsmCmd(STATE_SAMPLE_ALS, STATE_GET_ALS_VAL, tsl2591_read_status),
    sensorFsmCmd(STATE_GET_ALS_VAL, STATE_GET_ALS_EN, tsl2591_read_raw),
    sensorFsmCmd(STATE_GET_ALS_EN, STATE_SET_ALS_GAIN, tsl2591_get_enable),
    sensorFsmCmd(STATE_SET_ALS_GAIN, STATE_SET_ALS_DIS, tsl2591_set_gain),
    sensorFsmCmd(STATE_SET_ALS_DIS, STATE_SET_ALS_EN, tsl2591_set_als_dis),
    sensorFsmCmd(STATE_SET_ALS_EN, STATE_GET_ALS_LUX, tsl2591_set_als_en),
    sensorFsmCmd(STATE_GET_ALS_LUX, STATE_SAMPLE_ALS_DONE, tsl2591_get_als_lux),
    /* als enable state */
    sensorFsmCmd(STATE_ALS_ENABLE, STATE_ALS_POWER_DELAY, tsl2591_set_als_power_on),
    sensorFsmCmd(STATE_ALS_POWER_DELAY, STATE_SET_ALS_EN, tsl2591_set_als_power_delay),
    sensorFsmCmd(STATE_SET_ALS_EN, STATE_ALS_ENABLE_DONE, tsl2591_set_als_enable),
    /* als disable state */
    sensorFsmCmd(STATE_ALS_DISABLE, STATE_ALS_DISABLE_DONE, tsl2591_set_als_power_off),
    /* als rate state */
    sensorFsmCmd(STATE_ALS_RATECHG, CHIP_ALS_RATECHG_DONE, tsl2591_als_ratechg),
    /* als cali state */
    sensorFsmCmd(STATE_ALS_CALI, CHIP_ALS_CALI_DONE, tsl2591_als_cali),
    /* als cfg state */
    sensorFsmCmd(STATE_ALS_CFG, CHIP_ALS_CFG_DONE, tsl2591_als_cfg),
    /* init state */
    sensorFsmCmd(STATE_ALS_RESET, STATE_ALS_PST, tsl2591_set_als_control),
    sensorFsmCmd(STATE_ALS_PST, STATE_CORE, tsl2591_set_als_persist),
    sensorFsmCmd(STATE_CORE, STATE_INIT_DONE, tsl2591_register_core),
};

static int tsl2591Init(void) {
    int ret = 0, retry;

    osLog(LOG_INFO, "%s: task starting\n", __func__);
    tsl2591DebugPoint = &mTask;

    mTask.hw = get_cust_alsps("tsl2591");
    if (NULL == mTask.hw) {
        osLog(LOG_ERROR, "get_cust_alsps fail\n");
        return 0;
    }
    mTask.i2c_addr = mTask.hw->i2c_addr[0];
    mTask.als_debounce_time = 200;
    mTask.als_debounce_on = 0;
    mTask.alsCali = 0;
    mTask.als_threshold_high = mTask.hw->als_threshold_high;
    mTask.als_threshold_low = mTask.hw->als_threshold_low;
    mTask.chip.als_time = ATIME_100MS;
    mTask.chip.als_gain = AGAIN_LOW;
    mTask.prev_lux = 0;
    osLog(LOG_INFO, "als i2c_num: %d, i2c_addr: 0x%x\n", mTask.hw->i2c_num, mTask.i2c_addr);

    i2cMasterRequest(mTask.hw->i2c_num, I2C_SPEED);

    mTask.txBuf[0] = TSL2591_ID_REG;

    for (retry = 0; retry < 3; retry++) {
        ret = i2cMasterTxRxSync(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
            &mTask.deviceId, 1, NULL, NULL);
        if (ret >= 0)
            break;
    }
    if (ret < 0) {
        osLog(LOG_ERROR, "tsl2591 i2cMasterTxRxSync fail!!!\n");
        ret = -1;
        i2cMasterRelease(mTask.hw->i2c_num);
        goto err_out;
    }

    osLog(LOG_INFO, "tsl2591: auto detect success:0x%x\n", mTask.deviceId);
    alsSensorRegister();
    registerAlsSeparateFsm(tsl2591AlsFsm, ARRAY_SIZE(tsl2591AlsFsm));
err_out:
    return ret;
}

#ifndef CFG_OVERLAY_INIT_SUPPORT
MODULE_DECLARE(tsl2591, SENS_TYPE_ALS, tsl2591Init);
#else
#include "mtk_overlay_init.h"
OVERLAY_DECLARE(tsl2591, OVERLAY_ID_ALSPS, tsl2591Init);
#endif
