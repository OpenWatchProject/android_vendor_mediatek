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
#include <stdbool.h>
#include <stdint.h>
#include <seos.h>
#include <util.h>
#include <sensors.h>

#include <barometer.h>
#include <contexthub_core.h>
#include <cust_baro.h>

#define BARO_NAME                    "bmp280"
#define I2C_SPEED                       400000

#define BOSCH_BMP280_ID                 0x58

#define BOSCH_BMP280_REG_RESET          0x60
#define BOSCH_BMP280_REG_DIG_T1         0x88
#define BOSCH_BMP280_REG_ID             0xd0
#define BOSCH_BMP280_REG_CTRL_MEAS      0xf4
#define BOSCH_BMP280_REG_CONFIG         0xf5
#define BOSCH_BMP280_REG_PRES_MSB       0xf7

enum BMP280State {
    STATE_SAMPLE = CHIP_SAMPLING,
    STATE_CONVERT = CHIP_CONVERT,
    STATE_SAMPLE_DONE = CHIP_SAMPLING_DONE,
    STATE_ENABLE = CHIP_ENABLE,
    STATE_ENABLE_DONE = CHIP_ENABLE_DONE,
    STATE_DISABLE = CHIP_DISABLE,
    STATE_DISABLE_DONE = CHIP_DISABLE_DONE,
    STATE_RATECHG = CHIP_RATECHG,
    STATE_RATECHG_DONE = CHIP_RATECHG_DONE,
    STATE_INIT_DONE = CHIP_INIT_DONE,
    STATE_IDLE = CHIP_IDLE,
    STATE_RESET = CHIP_RESET,
    STATE_VERIFY_ID,
    STATE_AWAITING_COMP_PARAMS,
    STATE_MEAS,
    STATE_CONFIG,
    STATE_CORE,
};
struct BMP280CompParams {
    uint16_t dig_T1;
    int16_t dig_T2, dig_T3;
    uint16_t dig_P1;
    int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
} __attribute__((packed));

SRAM_REGION_BSS static struct BMP280Task {
    /* i2c operation read some data which is only use in prative driver */
    struct BMP280CompParams comp;
    /* txBuf for i2c operation, fill register and fill value */
    uint8_t txBuf[8];
    /* rxBuf for i2c operation, receive rawdata */
    struct transferDataInfo dataInfo;
    struct baroDataPacket baroPacket;
    uint8_t rxBuf[8];
    uint8_t verifyId;
    struct baro_hw *hw;
    uint8_t i2c_addr;
    /* data for factory */
    struct SingleAxisDataPoint factoryData;
} mTask;
static struct BMP280Task *bmp280DebugPoint;
static int32_t compensateTemp(int32_t adc_T, int32_t *t_fine)
{
    int32_t var1 =
        (((adc_T >> 3) - ((int32_t)mTask.comp.dig_T1 << 1))
         * (int32_t)mTask.comp.dig_T2) >> 11;

    int32_t tmp = (adc_T >> 4) - (int32_t)mTask.comp.dig_T1;

    int32_t var2 = (((tmp * tmp) >> 12) * (int32_t)mTask.comp.dig_T3) >> 14;

    int32_t sum = var1 + var2;

    *t_fine = sum;

    return (sum * 5 + 128) >> 8;
}

static float compensateBaro(int32_t t_fine, int32_t adc_P)
{
    float f = t_fine - 128000, fSqr = f * f;
    float a = 1048576 - adc_P;
    float v1, v2, p, pSqr;

    v2 = fSqr * mTask.comp.dig_P6 + f * mTask.comp.dig_P5 * (float)(1ULL << 17) + mTask.comp.dig_P4 * (float)(1ULL << 35);
    v1 = fSqr * mTask.comp.dig_P1 * mTask.comp.dig_P3 * (1.0f / (1ULL << 41)) + f * mTask.comp.dig_P1 * mTask.comp.dig_P2
         * (1.0f / (1ULL << 21)) + mTask.comp.dig_P1 * (float)(1ULL << 14);

    p = (a * (float)(1ULL << 31) - v2) * 3125 / v1;
    pSqr = p * p;

    return pSqr * mTask.comp.dig_P9 * (1.0f / (1ULL << 59)) + p * (mTask.comp.dig_P8 * (1.0f / (1ULL << 19)) + 1) *
           (1.0f / (1ULL << 8)) + 16.0f * mTask.comp.dig_P7;
}

static void getTempAndBaro(const uint8_t *tmp, float *pressure_Pa,
                           float *temp_centigrade)
{
    int32_t pres_adc = ((int32_t)tmp[0] << 12) | ((int32_t)tmp[1] << 4) | (tmp[2] >> 4);
    int32_t temp_adc = ((int32_t)tmp[3] << 12) | ((int32_t)tmp[4] << 4) | (tmp[5] >> 4);

    int32_t T_fine;
    int32_t temp = compensateTemp(temp_adc, &T_fine);
    float pres = compensateBaro(T_fine, pres_adc);

    *temp_centigrade = (float)temp * 0.01f;
    *pressure_Pa = pres * (1.0f / 256.0f) * 0.01f;
}
static void baroGetCalibration(int32_t *cali, int32_t size)
{

}
static void baroSetCalibration(int32_t *cali, int32_t size)
{

}
static void baroGetData(void *sample)
{
    struct SingleAxisDataPoint *singleSample = (struct SingleAxisDataPoint *)sample;
    singleSample->idata = mTask.factoryData.idata;
}

static void bmp280GetSensorInfo(struct sensorInfo_t *data)
{
    strncpy(data->name, BARO_NAME, sizeof(data->name));
}

static int bmp280Reset(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "BMP280: state reset\n");
    mTask.txBuf[0] = BOSCH_BMP280_REG_RESET;
    mTask.txBuf[1] = 0xB6;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}
static int bmp280VerifyId(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "BMP280: state id\n");

    mTask.txBuf[0] = BOSCH_BMP280_REG_ID;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         (uint8_t *)&mTask.verifyId, 1, i2cCallBack,
                         next_state);
}
static int bmp280CompParams(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;

    if (mTask.verifyId != BOSCH_BMP280_ID) {
        osLog(LOG_INFO, "BMP280: not detected\n");
        return -1;
    }
    //osLog(LOG_INFO, "BMP280: state comparam\n");
    /* Get compensation parameters */
    mTask.txBuf[0] = BOSCH_BMP280_REG_DIG_T1;
    ret = i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, (uint8_t*)&mTask.txBuf[0], 1,
                        (uint8_t*)&mTask.comp, 8, i2cCallBack,
                        (void *)STATE_IDLE);
    mTask.txBuf[1] = BOSCH_BMP280_REG_DIG_T1 + 8;
    ret = i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, (uint8_t*)&mTask.txBuf[1], 1,
                        (uint8_t*)((uint8_t*)&mTask.comp + 8), 8, i2cCallBack,
                        (void *)STATE_IDLE);
    mTask.txBuf[2] = BOSCH_BMP280_REG_DIG_T1 + 16;
    ret = i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, (uint8_t*)&mTask.txBuf[2], 1,
                        (uint8_t*)((uint8_t*)&mTask.comp + 16), 8, i2cCallBack,
                        next_state);
    return ret;
}
static int bmp280Meas(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "BMP280: state measure\n");
    mTask.txBuf[0] = BOSCH_BMP280_REG_CTRL_MEAS;
    // temp: 2x oversampling, baro: 16x oversampling, power: sleep
    mTask.txBuf[1] = (2 << 5) | (5 << 2);
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}
static int bmp280Config(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "BMP280: state config\n");

    mTask.txBuf[0] = BOSCH_BMP280_REG_CONFIG;
    // standby time: 62.5ms, IIR filter coefficient: 4
    mTask.txBuf[1] = (1 << 5) | (2 << 2);
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}
static int bmp280RegisterCore(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    struct sensorCoreInfo mInfo;
    memset(&mInfo, 0x00, sizeof(struct sensorCoreInfo));
    /* Register sensor Core */
    mInfo.sensType = SENS_TYPE_BARO;
    mInfo.getCalibration = baroGetCalibration;
    mInfo.setCalibration = baroSetCalibration;
    mInfo.getData = baroGetData;
    mInfo.getSensorInfo = bmp280GetSensorInfo;
    sensorCoreRegister(&mInfo);

    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int bmp280EnableBaro(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "BMP280: state enable baro\n");

    mTask.txBuf[0] = BOSCH_BMP280_REG_CTRL_MEAS;
    mTask.txBuf[1] = (2 << 5) | (5 << 2) | 3;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}
static int bmp280DisableBaro(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "BMP280: state disable baro\n");

    mTask.txBuf[0] = BOSCH_BMP280_REG_CTRL_MEAS;
    mTask.txBuf[1] = (2 << 5) | (5 << 2) | 0;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}
static int bmp280Rate(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    struct baroCntlPacket cntlPacket;
    ret = rxControlInfo(&cntlPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "bmp280Rate, rx inSize and elemSize error\n");
        return -1;
    }
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int bmp280Sample(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    ret = rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "bmp280Sample, rx dataInfo error\n");
        return -1;
    }
    mTask.txBuf[0] = BOSCH_BMP280_REG_PRES_MSB;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 6, i2cCallBack,
                         next_state);

}
static int bmp280Convert(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    struct baroData *data = mTask.baroPacket.outBuf;
    float pressure_Pa, temp_centigrade;

    getTempAndBaro(mTask.rxBuf, &pressure_Pa, &temp_centigrade);
    data[0].fdata = pressure_Pa;
    data[0].sensType = SENS_TYPE_BARO;
    mTask.factoryData.idata = (uint32_t)(pressure_Pa * BAROMETER_INCREASE_NUM_AP);
    //osLog(LOG_INFO, "BMP280: state convert :%f\n", (double)pressure_Pa);
    txTransferDataInfo(&mTask.dataInfo, 1, data);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}
static struct sensorFsm bmp280Fsm[] = {
    sensorFsmCmd(STATE_SAMPLE, STATE_CONVERT, bmp280Sample),
    sensorFsmCmd(STATE_CONVERT, STATE_SAMPLE_DONE, bmp280Convert),

    sensorFsmCmd(STATE_ENABLE, STATE_ENABLE_DONE, bmp280EnableBaro),

    sensorFsmCmd(STATE_DISABLE, STATE_DISABLE_DONE, bmp280DisableBaro),

    sensorFsmCmd(STATE_RATECHG, STATE_RATECHG_DONE, bmp280Rate),

    sensorFsmCmd(STATE_RESET, STATE_VERIFY_ID, bmp280Reset),
    sensorFsmCmd(STATE_VERIFY_ID, STATE_AWAITING_COMP_PARAMS, bmp280VerifyId),
    sensorFsmCmd(STATE_AWAITING_COMP_PARAMS, STATE_MEAS, bmp280CompParams),
    sensorFsmCmd(STATE_MEAS, STATE_CONFIG, bmp280Meas),
    sensorFsmCmd(STATE_CONFIG, STATE_CORE, bmp280Config),
    sensorFsmCmd(STATE_CORE, STATE_INIT_DONE, bmp280RegisterCore),
};

static int bmp280Init(void)
{
    int ret = 0;
    bmp280DebugPoint = &mTask;

    mTask.hw = get_cust_baro("bmp280");
    if (NULL == mTask.hw) {
        osLog(LOG_ERROR, "get_cust_baro_hw fail\n\r");
        ret = -1;
        goto err_out;
    }

    mTask.i2c_addr = mTask.hw->i2c_addr[0];
    osLog(LOG_ERROR, "i2c_num: %d, i2c_addr: 0x%x\n\r", mTask.hw->i2c_num, mTask.i2c_addr);
    i2cMasterRequest(mTask.hw->i2c_num, I2C_SPEED);
    mTask.txBuf[0] = BOSCH_BMP280_REG_ID;

    for (uint8_t i = 0; i < 3; i++) {
        ret = i2cMasterTxRxSync(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
            (uint8_t *)&mTask.verifyId, 1, NULL, NULL);
        if (ret >= 0)
            break;
    }
    if (ret < 0) {
        ret = -1;
        sendSensorErrToAp(ERR_SENSOR_BARO, ERR_CASE_BARO_INIT, BARO_NAME);
        i2cMasterRelease(mTask.hw->i2c_num);
        goto err_out;
    }

    if (mTask.verifyId == 0x58) {
        osLog(LOG_INFO, "bmp280: auto detect success %x\n", mTask.verifyId);
        goto success_out;
    } else {
        ret = -1;
        sendSensorErrToAp(ERR_SENSOR_BARO, ERR_CASE_BARO_INIT, BARO_NAME);
        i2cMasterRelease(mTask.hw->i2c_num);
        goto err_out;
    }

success_out:
    baroSensorRegister();
    baroRegisterInterruptMode(BARO_UNINTERRUPTIBLE);
    registerBaroDriverFsm(bmp280Fsm, ARRAY_SIZE(bmp280Fsm));

err_out:
    return ret;
}
#ifndef CFG_OVERLAY_INIT_SUPPORT
MODULE_DECLARE(bmp280, SENS_TYPE_BARO, bmp280Init);
#else
#include "mtk_overlay_init.h"
OVERLAY_DECLARE(bmp280, OVERLAY_ID_BARO, bmp280Init);
#endif
