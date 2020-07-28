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

#define BARO_NAME                    "spl06"
#define I2C_SPEED                       400000

/* common definition */
#define SPL_GET_BITSLICE(regvar, bitname)\
        ((regvar & bitname##__MSK) >> bitname##__POS)

#define SPL_SET_BITSLICE(regvar, bitname, val)\
        ((regvar & ~bitname##__MSK) | ((val<<bitname##__POS)&bitname##__MSK))

/* chip id */
#define SPL06_CHIP_ID                       0x10

/* power mode */
#define SPL06_SLEEP_MODE                    0x00
#define SPL06_NORMAL_MODE                   0x07

#define SPL06_TMP_SOURCE_EXT                0x80

/* calibration data */
#define SPL06_CALIBRATION_DATA_START            (0x10)
#define SPL06_CALIBRATION_DATA_LENGTH           (18)

/* register address */
#define SPL06_CHIP_ID_REG                   (0x0D)  /*Chip ID Register */
#define SPL06_RESET_REG                     (0x0C)  /*Softreset Register */
#define SPL06_INT_STATUS_REG                (0x0A)  /*Status Register */
#define SPL06_FIFO_STATUS_REG               (0x0B)  /*Status Register */
#define SPL06_PRS_CFG_REG                   (0x06)  /*Pressure Config Register */
#define SPL06_TMP_CFG_REG                   (0x07)  /*Temperature Config Register */
#define SPL06_CTRL_MEAS_REG                 (0x08)  /*Ctrl Measure Register */
#define SPL06_CONFIG_REG                    (0x09)  /*Configuration Register */

/* data */
#define SPL06_PRESSURE_MSB_REG              (0x00)  /*Pressure MSB Register */
#define SPL06_PRESSURE_LSB_REG              (0x01)  /*Pressure LSB Register */
#define SPL06_PRESSURE_XLSB_REG             (0x02)  /*Pressure XLSB Register */
#define SPL06_TEMPERATURE_MSB_REG           (0x03)  /*Temperature MSB Reg */
#define SPL06_TEMPERATURE_LSB_REG           (0x04)  /*Temperature LSB Reg */
#define SPL06_TEMPERATURE_XLSB_REG          (0x05)  /*Temperature XLSB Reg */

/* power mode bit definition */
#define SPL06_CTRL_MEAS_REG_POWER_MODE__POS     (0)
#define SPL06_CTRL_MEAS_REG_POWER_MODE__MSK     (0x07)
#define SPL06_CTRL_MEAS_REG_POWER_MODE__LEN     (3)
#define SPL06_CTRL_MEAS_REG_POWER_MODE__REG     (SPL06_CTRL_MEAS_REG)

#define SPL06_PRESSURE_SHIFT                    0x04
#define SPL06_TEMPERATURE_SHIFT                 0x08


enum SPL06State {
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
    STATE_POWER_ON,
    STATE_POWER_OFF,
    STATE_VERIFY_ID,
    STATE_AWAITING_COMP_PARAMS,
    STATE_MEAS,
    STATE_CONFIG,
    STATE_CORE,
};
struct spl06_calibration_data {
    int16_t c0;
    int16_t c1;
    int32_t c00;
    int32_t c10;
    int16_t c01;
    int16_t c11;
    int16_t c20;
    int16_t c21;
    int16_t c30;
} __attribute__((packed));

SRAM_REGION_BSS static struct SPL06Task {
    /* i2c operation read some data which is only use in prative driver */
    struct spl06_calibration_data spl06_cali;
    /* txBuf for i2c operation, fill register and fill value */
    uint8_t txBuf[8];
    /* rxBuf for i2c operation, receive rawdata */
    struct transferDataInfo dataInfo;
    struct baroDataPacket baroPacket;
    uint8_t rxBuf[20];
    uint8_t verifyId;
    struct baro_hw *hw;
    uint8_t i2c_addr;
    /* data for factory */
    struct SingleAxisDataPoint factoryData;
    uint32_t i32kP;
    uint32_t i32kT;
} mTask;
static struct SPL06Task *spl06DebugPoint;


static int spl_set_powermode(I2cCallbackF i2cCallBack, void *next_state,
                         uint8_t power_mode)
{

    mTask.txBuf[0] = SPL06_CTRL_MEAS_REG_POWER_MODE__REG;
    mTask.txBuf[1] = SPL_SET_BITSLICE(mTask.rxBuf[0],
                      SPL06_CTRL_MEAS_REG_POWER_MODE, power_mode);;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}
static int spl_power_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    return spl_set_powermode(i2cCallBack, next_state, SPL06_NORMAL_MODE);
}
static int spl_power_off(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
     return spl_set_powermode(i2cCallBack, next_state, SPL06_SLEEP_MODE);
}
static int spl_read_config(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
     mTask.txBuf[0] = SPL06_CTRL_MEAS_REG_POWER_MODE__REG;
     return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,
                         next_state);
}
static int spl_sample(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    ret = rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "spl Sample, rx dataInfo error\n");
        return -1;
    }
    mTask.txBuf[0] = SPL06_PRESSURE_MSB_REG;
    ret = i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                        mTask.rxBuf, 6, i2cCallBack,
                        next_state);
    return ret;

}
static void getTempAndBaro(const uint8_t *tmp, float *pressure_Pa,
                           float *temp_centigrade)
{
    int32_t upressure = ((int32_t)tmp[0] << 16) | ((int32_t)tmp[1] << 8) | tmp[2];
    int32_t utemp = ((int32_t)tmp[3] << 16) | ((int32_t)tmp[4] << 8) | tmp[5];
    float fTsc, fPsc;
    float qua2, qua3;
    float fpressure;

    utemp = (utemp&0x800000) ? (0xFF000000|utemp) : utemp;
    upressure = (upressure&0x800000) ? (0xFF000000|upressure) : upressure;
    fTsc = utemp / (float)mTask.i32kT;
    fPsc = upressure / (float)mTask.i32kP;
    qua2 = mTask.spl06_cali.c10 + fPsc * (mTask.spl06_cali.c20 + fPsc* mTask.spl06_cali.c30);
    qua3 = fTsc * fPsc * (mTask.spl06_cali.c11 + fPsc * mTask.spl06_cali.c21);

    fpressure = mTask.spl06_cali.c00 + fPsc * qua2 + fTsc * mTask.spl06_cali.c01 + qua3;
    *pressure_Pa = fpressure / 100;
    fTsc = utemp / (float)mTask.i32kT;
    *temp_centigrade = mTask.spl06_cali.c0 * 0.5 + mTask.spl06_cali.c1 * fTsc;
}

static int spl_convert(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    struct baroData *data = mTask.baroPacket.outBuf;
    float pressure_Pa, temp_centigrade;

    getTempAndBaro(mTask.rxBuf, &pressure_Pa, &temp_centigrade);
    data[0].fdata = pressure_Pa;
    data[0].sensType = SENS_TYPE_BARO;
    mTask.factoryData.idata = (uint32_t)(pressure_Pa * BAROMETER_INCREASE_NUM_AP);
    txTransferDataInfo(&mTask.dataInfo, 1, data);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
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

static void spl_getSensorInfo(struct sensorInfo_t *data)
{
    strncpy(data->name, BARO_NAME, sizeof(data->name));
}

static int spl_compParams(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;

    if (mTask.verifyId != SPL06_CHIP_ID) {
        osLog(LOG_INFO, "spl06: not detected\n");
        return -1;
    }
    /* Get compensation parameters */
    mTask.txBuf[0] = SPL06_CALIBRATION_DATA_START;
    ret = i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, (uint8_t*)&mTask.txBuf[0], 1,
                        (uint8_t*)&mTask.rxBuf[0], 6, i2cCallBack,
                        (void *)STATE_IDLE);
    mTask.txBuf[1] = SPL06_CALIBRATION_DATA_START + 6;
    ret = i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, (uint8_t*)&mTask.txBuf[1], 1,
                        (uint8_t*)&mTask.rxBuf[6], 6, i2cCallBack,
                        (void *)STATE_IDLE);
    mTask.txBuf[2] = SPL06_CALIBRATION_DATA_START + 12;
    ret = i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, (uint8_t*)&mTask.txBuf[2], 1,
                        (uint8_t*)&mTask.rxBuf[12], 6, i2cCallBack,
                        (void *)STATE_IDLE);
    mTask.txBuf[3] = SPL06_CTRL_MEAS_REG_POWER_MODE__REG;
    ret = i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, (uint8_t*)&mTask.txBuf[3], 1,
                        (uint8_t*)&mTask.rxBuf[18], 1, i2cCallBack,
                        next_state);
    return ret;
}
static void calculate_compParams(const uint8_t *tmp)
{
    mTask.spl06_cali.c0 = ((uint32_t)tmp[0]<<4) | (tmp[1]>>4);
    mTask.spl06_cali.c0 = (mTask.spl06_cali.c0&0x0800) ?
               (0xF000|mTask.spl06_cali.c0) :
               mTask.spl06_cali.c0;
    mTask.spl06_cali.c1 = ((uint32_t)(tmp[1]&0x0F)<<8) | tmp[2];
    mTask.spl06_cali.c1 = (mTask.spl06_cali.c1&0x0800) ?
                (0xF000|mTask.spl06_cali.c1) :
                mTask.spl06_cali.c1;
    mTask.spl06_cali.c00 = ((uint32_t)tmp[3]<<12) |
                ((uint32_t)tmp[4]<<4) |
                ((uint32_t)tmp[5]>>4);
    mTask.spl06_cali.c00 = (mTask.spl06_cali.c00&0x080000) ?
                (0xFFF00000|mTask.spl06_cali.c00) :
                mTask.spl06_cali.c00;
    mTask.spl06_cali.c10 = ((uint32_t)(tmp[5]&0x0F)<<16) |
                ((uint32_t)tmp[6]<<8) | tmp[7];
    mTask.spl06_cali.c10 = (mTask.spl06_cali.c10&0x080000) ?
                (0xFFF00000|mTask.spl06_cali.c10) :
                mTask.spl06_cali.c10;
    mTask.spl06_cali.c01 = ((uint32_t)tmp[8]<<8) | tmp[9];
    mTask.spl06_cali.c11 = ((uint32_t)tmp[10]<<8) | tmp[11];
    mTask.spl06_cali.c20 = ((uint32_t)tmp[12]<<8) | tmp[13];
    mTask.spl06_cali.c21 = ((uint32_t)tmp[14]<<8) | tmp[15];
    mTask.spl06_cali.c30 = ((uint32_t)tmp[16]<<8) | tmp[17];

}

static int spl_config(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;

    calculate_compParams(mTask.rxBuf);
    mTask.txBuf[0] = SPL06_CTRL_MEAS_REG_POWER_MODE__REG;
    mTask.txBuf[1] = SPL06_SLEEP_MODE;
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, (void *)STATE_IDLE);
    mTask.txBuf[2] = SPL06_PRS_CFG_REG;
    mTask.txBuf[3] = 0x14;
    mTask.i32kP = 253952;
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, (uint8_t*)&mTask.txBuf[2], 2,
                       i2cCallBack, (void *)STATE_IDLE);
    mTask.txBuf[4] = SPL06_TMP_CFG_REG;
    mTask.txBuf[5] = 0x80;
    mTask.i32kT = 524288;
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, (uint8_t*)&mTask.txBuf[4], 2,
                       i2cCallBack, (void *)STATE_IDLE);
    mTask.txBuf[6] = SPL06_CONFIG_REG;
    mTask.txBuf[7] = 0x4;
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, (uint8_t*)&mTask.txBuf[6], 2,
                       i2cCallBack, next_state);
    return ret;
}
static int spl_registerCore(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
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
    mInfo.getSensorInfo = spl_getSensorInfo;
    sensorCoreRegister(&mInfo);

    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int spl_rate(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    struct baroCntlPacket cntlPacket;
    ret = rxControlInfo(&cntlPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "spl06 Rate, rx inSize and elemSize error\n");
        return -1;
    }
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static struct sensorFsm spl06Fsm[] = {
    /* sample state */
    sensorFsmCmd(STATE_SAMPLE, STATE_CONVERT, spl_sample),
    sensorFsmCmd(STATE_CONVERT, STATE_SAMPLE_DONE, spl_convert),
    /* enable state */
    sensorFsmCmd(STATE_ENABLE, STATE_POWER_ON, spl_read_config),
    sensorFsmCmd(STATE_POWER_ON, STATE_ENABLE_DONE, spl_power_on),
    /* disable state*/
    sensorFsmCmd(STATE_DISABLE, STATE_POWER_OFF, spl_read_config),
    sensorFsmCmd(STATE_POWER_OFF, STATE_DISABLE_DONE, spl_power_off),

    sensorFsmCmd(STATE_RATECHG, STATE_RATECHG_DONE, spl_rate),

    sensorFsmCmd(STATE_RESET, STATE_CONFIG, spl_compParams),
    sensorFsmCmd(STATE_CONFIG, STATE_CORE, spl_config),
    sensorFsmCmd(STATE_CORE, STATE_INIT_DONE, spl_registerCore),
};

static int spl06Init(void)
{
    int ret = 0;
    spl06DebugPoint = &mTask;

    mTask.hw = get_cust_baro("spl06");
    if (NULL == mTask.hw) {
        osLog(LOG_ERROR, "get_cust_baro_hw fail\n\r");
        ret = -1;
        goto err_out;
    }

    mTask.i2c_addr = mTask.hw->i2c_addr[0];
    osLog(LOG_ERROR, "i2c_num: %d, i2c_addr: 0x%x\n\r", mTask.hw->i2c_num, mTask.i2c_addr);
    i2cMasterRequest(mTask.hw->i2c_num, I2C_SPEED);
    mTask.txBuf[0] = SPL06_CHIP_ID_REG;

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

    if (mTask.verifyId == 0x10) {
        osLog(LOG_INFO, "spl06 auto detect success %x\n", mTask.verifyId);
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
    registerBaroDriverFsm(spl06Fsm, ARRAY_SIZE(spl06Fsm));
err_out:
    return ret;
}
#ifndef CFG_OVERLAY_INIT_SUPPORT
MODULE_DECLARE(spl06, SENS_TYPE_BARO, spl06Init);
#else
#include "mtk_overlay_init.h"
OVERLAY_DECLARE(spl06, OVERLAY_ID_BARO, spl06Init);
#endif
