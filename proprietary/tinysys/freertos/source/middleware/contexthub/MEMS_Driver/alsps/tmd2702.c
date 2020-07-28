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
#include "tmd2702.h"
#include "alsps_separate.h"
#include "alsps.h"

#define I2C_SPEED       400000
#define PS_NAME         "tmd2702"

enum tmd2702State {
    STATE_SAMPLE_PS        = CHIP_SAMPLING_PS,
    STATE_SAMPLE_PS_DONE   = CHIP_SAMPLING_PS_DONE,
    STATE_PS_ENABLE        = CHIP_PS_ENABLE,
    STATE_PS_ENABLE_DONE   = CHIP_PS_ENABLE_DONE,
    STATE_PS_DISABLE       = CHIP_PS_DISABLE,
    STATE_PS_DISABLE_DONE  = CHIP_PS_DISABLE_DONE,
    STATE_PS_RATECHG       = CHIP_PS_RATECHG,
    STATE_PS_RATECHG_DONE  = CHIP_PS_RATECHG_DONE,
    STATE_PS_CALI          = CHIP_PS_CALI,
    STATE_PS_CALI_DONE     = CHIP_PS_CALI_DONE,
    STATE_PS_CFG           = CHIP_PS_CFG,
    STATE_PS_CFG_DONE      = CHIP_PS_CFG_DONE,
    STATE_INIT_DONE        = CHIP_INIT_DONE,
    STATE_IDLE             = CHIP_IDLE,
    STATE_RESET            = CHIP_PS_RESET,

    STATE_GET_PS_VALUE,
    STATE_PS_SET_DEBOUNCE,
    STATE_SET_PS_LED,
    STATE_SET_PS_PULSES,
    STATE_SET_PS_MEAS_RATE,
    STATE_PS_POWER_ON,
    STATE_PS_POWER_OFF,
    STATE_CORE
};


#define MAX_RXBUF 8
#define MAX_TXBUF 8


SRAM_REGION_BSS static struct tmd2702Task {
    /* txBuf for i2c operation, fill register and fill value */
    uint8_t                     txBuf[MAX_TXBUF];
    /* rxBuf for i2c operation, receive rawdata */
    uint8_t                     rxBuf[MAX_RXBUF];
    uint8_t                     deviceId;
    /* data for factory */
    struct alsps_hw             *hw;
    uint8_t                     i2c_addr;
    struct transferDataInfo     dataInfo;
    struct AlsPsData            data;
    uint32_t                    prox_raw_data;
    int32_t                     psCali;
    uint32_t                    ps_threshold_high;
    uint32_t                    ps_threshold_low;
    uint8_t                     int_status;
} mTask;
static struct tmd2702Task *tmd2702DebugPoint;

static int tmd2702_read_ps_raw_data(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                          void *inBuf, uint8_t inSize, uint8_t elemInSize,
                          void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    if (rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize)) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "tmd2702 ps, rx dataInfo error\n");
        return -1;
    }
    mTask.txBuf[0] = TMD2702_REG_PDATA;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 2, i2cCallBack,
                         next_state);
}

static void tmd2702_ps_set_threshold(uint32_t threshold_high, uint32_t threshold_low);
static int tmd2702_get_ps_status(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {

    static bool ps_flag = 0;
    uint32_t psRawData = 0;
    psRawData = ((mTask.rxBuf[1] << 8) | mTask.rxBuf[0]);
    if (psRawData >= mTask.ps_threshold_high) {
        ps_flag = 1;
    } else if (psRawData <= mTask.ps_threshold_low) {
        ps_flag = 0;
    }
    if (ps_flag == 0) { /* 0 is far */
        mTask.data.prox_state = PROX_STATE_FAR; /* far */
        mTask.data.prox_data = 1; /* far state distance is 1cm */
    } else {
        mTask.data.prox_state = PROX_STATE_NEAR; /* near */
        mTask.data.prox_data = 0; /* near state distance is 0cm */
    }

    mTask.prox_raw_data = psRawData;
    if (mTask.prox_raw_data < mTask.psCali)
        mTask.prox_raw_data = 0;
    mTask.prox_raw_data -= mTask.psCali;
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
            mTask.prox_raw_data = ((rxBuf[1] << 8) |(rxBuf[0]));
            if (mTask.prox_raw_data < mTask.psCali)
                mTask.prox_raw_data = 0;
            mTask.prox_raw_data -= mTask.psCali;
        } else
            osLog(LOG_INFO, "tmd2702: read ps data i2c error (%d)\n", err);
    }
    txBuf[0] = TMD2702_REG_PDATA;
    i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, txBuf, 1,
                  rxBuf, 2, get_ps_data, rxBuf);
}

static void tmd2702_ps_set_threshold(uint32_t threshold_high, uint32_t threshold_low) {

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

static void psGetCalibration(int32_t *cali, int32_t size) {
    cali[0] = mTask.psCali;
}

static void psSetCalibration(int32_t *cali, int32_t size) {
    mTask.psCali = cali[0];
    tmd2702_ps_set_threshold(mTask.ps_threshold_high + mTask.psCali,
                            mTask.ps_threshold_low + mTask.psCali);
}

static void psGetThreshold(uint32_t *threshold_high, uint32_t *threshold_low) {
    *threshold_high = mTask.ps_threshold_high;
    *threshold_low = mTask.ps_threshold_low;
}

static void psSetThreshold(uint32_t threshold_high, uint32_t threshold_low) {
    mTask.ps_threshold_high = threshold_high;
    mTask.ps_threshold_low = threshold_low;
    tmd2702_ps_set_threshold(mTask.ps_threshold_high, mTask.ps_threshold_low);
}

static int tmd2702_set_ps_power_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                  void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                  void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.txBuf[0] = TMD2702_REG_ENABLE;
    mTask.txBuf[1] = mTask.rxBuf[0] | TMD2702_PEN;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int tmd2702_set_ps_power_off(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                   void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                   void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.data.prox_state = PROX_STATE_FAR; /* default value */
    mTask.txBuf[0] = TMD2702_REG_ENABLE;
    mTask.txBuf[1] = mTask.rxBuf[0] & ~TMD2702_PEN;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int tmd2702_ps_ratechg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int tmd2702_ps_cali(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    /*Not work now!*/
    int32_t threshold[2];
    threshold[0] = 2; // high simulation data
    threshold[1] = 3; // low simulation data
    alsPsSendCalibrationResult(SENS_TYPE_PROX, threshold);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}
static int tmd2702_ps_cfg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
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
    tmd2702_ps_set_threshold(caliCfgPacket.caliCfgData[0], caliCfgPacket.caliCfgData[1]);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int tmd2702_get_ps_conf(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.txBuf[0] = TMD2702_REG_ENABLE;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,
                         next_state);
}

static int tmd2702_set_main_ctrl(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.txBuf[0] = TMD2702_REG_ENABLE;
    mTask.txBuf[1] = TMD2702_PON;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static void tmd2702_ps_reduce_current(void) {
    //set the CFG1 register to reduce the ps IR drive current
    int ret = 0;
    mTask.txBuf[0] = TMD2702_REG_CFG1;
    mTask.txBuf[1] = PLDRIVE_REDUCE;
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr,  mTask.txBuf, 2,
                NULL, 0);
    if (ret) {
        osLog(LOG_ERROR, "tmd2702: set ps reduce_current i2c error (%d)\n", ret);
        return;
    }
    return;
}

static int tmd2702_set_ps_led(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {

    tmd2702_ps_reduce_current();
    mTask.txBuf[0] = TMD2702_REG_PGCFG1;
    mTask.txBuf[1] = PGAIN_4X | PLDRIVE_MA(10);
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int tmd2702_set_ps_pulses(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.txBuf[0] = TMD2702_REG_PGCFG0;
    mTask.txBuf[1] = PPLEN_32US | PPULSES(16);
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int tmd2702_set_ps_meas_rate(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                   void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                   void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.txBuf[0] = TMD2702_REG_PTIME;
    mTask.txBuf[1] = PTIME_US(INTEGRATION_CYCLE);
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int tmd2702_register_core(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    struct sensorCoreInfo mInfo;
    int ret = 0;

    /* Register sensor Core */
    memset(&mInfo, 0x00, sizeof(struct sensorCoreInfo));
    mInfo.sensType = SENS_TYPE_PROX,
    mInfo.gain = 1;
    mInfo.sensitivity = 1;
    ret = sensorDriverGetConvert(0, &mInfo.cvt);
    if (ret)
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

static struct sensorFsm tmd2702Fsm[] = {
    /* sample ps */
    sensorFsmCmd(STATE_SAMPLE_PS, STATE_GET_PS_VALUE, tmd2702_read_ps_raw_data),
    sensorFsmCmd(STATE_GET_PS_VALUE, STATE_SAMPLE_PS_DONE, tmd2702_get_ps_status),
    /* ps enable state */
    sensorFsmCmd(STATE_PS_ENABLE, STATE_PS_POWER_ON, tmd2702_get_ps_conf),
    sensorFsmCmd(STATE_PS_POWER_ON, STATE_PS_ENABLE_DONE, tmd2702_set_ps_power_on),
    /* ps disable state */
    sensorFsmCmd(STATE_PS_DISABLE, STATE_PS_POWER_OFF, tmd2702_get_ps_conf),
    sensorFsmCmd(STATE_PS_POWER_OFF, STATE_PS_DISABLE_DONE, tmd2702_set_ps_power_off),
    /* ps rate change */
    sensorFsmCmd(STATE_PS_RATECHG, STATE_PS_RATECHG_DONE, tmd2702_ps_ratechg),
    /*ps cali*/
    sensorFsmCmd(STATE_PS_CALI, STATE_PS_CALI_DONE, tmd2702_ps_cali),
    /*ps cali*/
    sensorFsmCmd(STATE_PS_CFG, STATE_PS_CFG_DONE, tmd2702_ps_cfg),
    /* init state */
    sensorFsmCmd(STATE_RESET, STATE_SET_PS_LED, tmd2702_set_main_ctrl),
    sensorFsmCmd(STATE_SET_PS_LED, STATE_SET_PS_PULSES, tmd2702_set_ps_led),
    sensorFsmCmd(STATE_SET_PS_PULSES, STATE_SET_PS_MEAS_RATE, tmd2702_set_ps_pulses),
    sensorFsmCmd(STATE_SET_PS_MEAS_RATE, STATE_CORE, tmd2702_set_ps_meas_rate),
    sensorFsmCmd(STATE_CORE, STATE_INIT_DONE, tmd2702_register_core),
};


static int tmd2702Init(void) {
    int ret = 0;
    tmd2702DebugPoint = &mTask;

    mTask.hw = get_cust_alsps("tmd2702");
    if (NULL == mTask.hw) {
        ret = -1;
        goto err_out;
    }
    mTask.i2c_addr = mTask.hw->i2c_addr[0];
    mTask.psCali = 0;
    mTask.ps_threshold_high = mTask.hw->ps_threshold_high;
    mTask.ps_threshold_low = mTask.hw->ps_threshold_low;
    osLog(LOG_ERROR, "alsps i2c_num: %d, i2c_addr: 0x%x\n", mTask.hw->i2c_num, mTask.i2c_addr);

    i2cMasterRequest(mTask.hw->i2c_num, I2C_SPEED);

    mTask.txBuf[0] = TMD2702_REG_ID;
    ret = i2cMasterTxRxSync(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                    &mTask.deviceId, 1, NULL, NULL);
    if (ret < 0) {
        ret = -1;
        i2cMasterRelease(mTask.hw->i2c_num);
        goto err_out;
    }

    if (mTask.deviceId == TMD2702_ID) {
        goto success_out;
    } else {
        i2cMasterRelease(mTask.hw->i2c_num);
        ret = -1;
        goto err_out;
    }
success_out:
    osLog(LOG_INFO, "tmd2702: auto detect success:0x%x\n", mTask.deviceId);

    psSensorRegister();
    registerPsSeparateFsm(tmd2702Fsm, ARRAY_SIZE(tmd2702Fsm));
    registerPsInterruptMode(PS_POLLING_MODE); /*using polling mode*/

err_out:
    return ret;
}
#ifndef CFG_OVERLAY_INIT_SUPPORT
MODULE_DECLARE(tmd2702, SENS_TYPE_ALS, tmd2702Init);
#else
#include "mtk_overlay_init.h"
OVERLAY_DECLARE(tmd2702, OVERLAY_ID_ALSPS_SECONDARY, tmd2702Init);
#endif


