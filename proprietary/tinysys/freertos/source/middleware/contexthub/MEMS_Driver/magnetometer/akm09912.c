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
#include <plat/inc/rtc.h>

#include <magnetometer.h>
#include <contexthub_core.h>
#include <cust_mag.h>
#include <AkmApi.h>

#define I2C_SPEED                       400000
#define AKM09912_DATA_LEN       8

/* Device specific constant values */
#define AKM09912_REG_WIA1           0x00
#define AKM09912_REG_WIA2           0x01
#define AKM09912_REG_INFO1          0x02
#define AKM09912_REG_INFO2          0x03
#define AKM09912_REG_ST1                0x10
#define AKM09912_REG_HXL                0x11
#define AKM09912_REG_HXH                0x12
#define AKM09912_REG_HYL                0x13
#define AKM09912_REG_HYH                0x14
#define AKM09912_REG_HZL                0x15
#define AKM09912_REG_HZH                0x16
#define AKM09912_REG_TMPS           0x17
#define AKM09912_REG_ST2                0x18
#define AKM09912_REG_CNTL1          0x30
#define AKM09912_REG_CNTL2          0x31
#define AKM09912_REG_CNTL3          0x32
#define AKM09912_REG_ASAX           0x60
#define AKM09912_REG_ASAY           0x61
#define AKM09912_REG_ASAZ           0x62


/*! \name AKM09912 operation mode
 \anchor AKM09912_Mode
 Defines an operation mode of the AKM09912.*/
#define AKM09912_MODE_SNG_MEASURE   0x01
#define AKM09912_MODE_10HZ_MEASURE  0x02
#define AKM09912_MODE_20HZ_MEASURE  0x04
#define AKM09912_MODE_50HZ_MEASURE  0x06
#define AKM09912_MODE_100HZ_MEASURE 0x08
#define AKM09912_MODE_SELF_TEST     0x10
#define AKM09912_MODE_FUSE_ACCESS   0x1F
#define AKM09912_MODE_POWERDOWN     0x00
#define AKM09912_RESET_DATA         0x01

#define DATA_READY_BIT              0X01


enum AKM09912State {
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
    STATE_FUSE_EN = CHIP_RESET,
    STATE_FUSE_RD,
    STATE_CORE,
};

static struct AKM09912Task {
    /* txBuf for i2c operation, fill register and fill value */
    uint8_t txBuf[8];
    /* rxBuf for i2c operation, receive rawdata */
    uint8_t rxBuf[8];
    uint8_t fuse[3];
    uint8_t autoDetect;
    uint64_t hwSampleTime;
    struct transferDataInfo dataInfo;
    struct magDataPacket magPacket;
    /* data for factory */
    struct TripleAxisDataPoint factoryData;
    struct mag_hw *hw;
    struct sensorDriverConvert cvt;
    uint8_t i2c_addr;
    struct mag_dev_info_t mag_dev_info;
} mTask;
static struct AKM09912Task *akm09912DebugPoint;
void akm09912TimerCbkF(uint64_t time)
{
    mTask.hwSampleTime = time;
}
static void magGetCalibration(int32_t *cali, int32_t size)
{
}
static void magSetCalibration(int32_t *cali, int32_t size)
{
}
static void magGetData(void *sample)
{
    struct TripleAxisDataPoint *tripleSample = (struct TripleAxisDataPoint *)sample;
    tripleSample->ix = mTask.factoryData.ix;
    tripleSample->iy = mTask.factoryData.iy;
    tripleSample->iz = mTask.factoryData.iz;
}

static int akm09912FuseEn(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "AKM09912: state reset\n");
    mTask.txBuf[0] = AKM09912_REG_CNTL2;
    mTask.txBuf[1] = AKM09912_MODE_FUSE_ACCESS;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}
static int akm09912FuseRd(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "AKM09912: state id\n");

    mTask.txBuf[0] = AKM09912_REG_ASAX;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         (uint8_t *)&mTask.fuse, 3, i2cCallBack,
                         next_state);
}
static void magGetSensorInfo(struct mag_dev_info_t *data)
{
    memcpy(data, &mTask.mag_dev_info, sizeof(struct mag_dev_info_t));
}
static int akm09912RegisterCore(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    struct sensorCoreInfo mInfo;
    memset(&mInfo, 0x00, sizeof(struct sensorCoreInfo));
    /* Register sensor Core */
    mInfo.sensType = SENS_TYPE_MAG;
    mInfo.getCalibration = magGetCalibration;
    mInfo.setCalibration = magSetCalibration;
    mInfo.getData = magGetData;
    mInfo.getSensorInfo = magGetSensorInfo;
    sensorCoreRegister(&mInfo);

    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int akm09912Enable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "AKM09912: state enable mag\n");

    mTask.txBuf[0] = AKM09912_REG_CNTL2;
    mTask.txBuf[1] = AKM09912_MODE_100HZ_MEASURE;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}
static int akm09912Disable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "AKM09912: state disable mag\n");

    mTask.txBuf[0] = AKM09912_REG_CNTL2;
    mTask.txBuf[1] = AKM09912_MODE_POWERDOWN;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}
static int akm09912Rate(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    uint32_t sample_rate, water_mark;
    struct magCntlPacket cntlPacket;

    ret = rxControlInfo(&cntlPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "akm09912Rate, rx inSize and elemSize error\n");
        return -1;
    }
    sample_rate = cntlPacket.rate;
    water_mark = cntlPacket.waterMark;
    osLog(LOG_INFO, "akm09912Rate: %d, water_mark:%d\n", sample_rate, water_mark);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int akm09912Sample(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    ret = rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "akm09912Sample, rx dataInfo error\n");
        return -1;
    }
    mTask.txBuf[0] = AKM09912_REG_HXL;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 8, i2cCallBack,
                         next_state);
}
static int akm09912Convert(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    struct magData *data = mTask.magPacket.outBuf;

    float remap_data[AXES_NUM];
    int32_t idata[AXES_NUM];
    uint64_t timestamp = 0;

    idata[AXIS_X] = (int16_t)((mTask.rxBuf[AXIS_X*2+1] << 8) | (mTask.rxBuf[AXIS_X*2]));
    idata[AXIS_Y] = (int16_t)((mTask.rxBuf[AXIS_Y*2+1] << 8) | (mTask.rxBuf[AXIS_Y*2]));
    idata[AXIS_Z] = (int16_t)((mTask.rxBuf[AXIS_Z*2+1] << 8) | (mTask.rxBuf[AXIS_Z*2]));

    remap_data[mTask.cvt.map[AXIS_X]] = mTask.cvt.sign[AXIS_X] * idata[AXIS_X];
    remap_data[mTask.cvt.map[AXIS_Y]] = mTask.cvt.sign[AXIS_Y] * idata[AXIS_Y];
    remap_data[mTask.cvt.map[AXIS_Z]] = mTask.cvt.sign[AXIS_Z] * idata[AXIS_Z];

    /* osLog(LOG_ERROR, "msensor remap data: %d, %d, %d!\n", remap_data[AXIS_X], remap_data[AXIS_Y],
		remap_data[AXIS_Z]); */


	/* osLog(LOG_DEBUG, "msensor raw data: %d, %d, %d, timestamp: %lld!\n", remap_data[AXIS_X],
		remap_data[AXIS_Y], remap_data[AXIS_Z], mTask.hwSampleTime); */
    data[0].x = remap_data[AXIS_X];
    data[0].y = remap_data[AXIS_Y];
    data[0].z = remap_data[AXIS_Z];
    mTask.factoryData.ix = (int32_t)(remap_data[AXIS_X] * MAGNETOMETER_INCREASE_NUM_AP);
    mTask.factoryData.iy = (int32_t)(remap_data[AXIS_Y] * MAGNETOMETER_INCREASE_NUM_AP);
    mTask.factoryData.iz = (int32_t)(remap_data[AXIS_Z] * MAGNETOMETER_INCREASE_NUM_AP);
    //osLog(LOG_ERROR, "msensor output data: %d, %d, %d!\n", data[AXIS_X], data[AXIS_Y], data[AXIS_Z]);
    timestamp = addThenRetreiveAverageMagTimeStamp(mTask.hwSampleTime);
    txTransferDataInfo(&mTask.dataInfo, 1, timestamp, data);
	/*osLog(LOG_INFO, "[%08X %08X %08X] => [%5d %5d %5d], st2: %d\n",
		data[AXIS_X], data[AXIS_Y], data[AXIS_Z],
		data[AXIS_X], data[AXIS_Y], data[AXIS_Z], mTask.rxBuf[7]);*/

    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}
static struct sensorFsm akm09912Fsm[] = {
    sensorFsmCmd(STATE_SAMPLE, STATE_CONVERT, akm09912Sample),
    sensorFsmCmd(STATE_CONVERT, STATE_SAMPLE_DONE, akm09912Convert),

    sensorFsmCmd(STATE_ENABLE, STATE_ENABLE_DONE, akm09912Enable),
    sensorFsmCmd(STATE_DISABLE, STATE_DISABLE_DONE, akm09912Disable),

    sensorFsmCmd(STATE_RATECHG, STATE_RATECHG_DONE, akm09912Rate),

    sensorFsmCmd(STATE_FUSE_EN, STATE_FUSE_RD, akm09912FuseEn),
    sensorFsmCmd(STATE_FUSE_RD, STATE_DISABLE, akm09912FuseRd),
    sensorFsmCmd(STATE_DISABLE, STATE_CORE, akm09912Disable),
    sensorFsmCmd(STATE_CORE, STATE_INIT_DONE, akm09912RegisterCore),
};
static int akm09912CaliApiGetOffset(float offset[AXES_NUM])
{
    return 0;
}
static int akm09912CaliApiSetOffset(float offset[AXES_NUM])
{
    AKM_ReloadContext(offset);
    AKM_Open();
    return 0;
}
static int akm09912CaliApiSetGyroData(struct magCaliDataInPut *inputData)
{
    return AKM_SetGyroData((double)inputData->x, (double)inputData->y,
        (double)inputData->z, inputData->timeStamp);
}
static int akm09912DoCaliAPI(struct magCaliDataInPut *inputData,
        struct magCaliDataOutPut *outputData)
{
    int err = 0;
    int16_t tmpStatus = 0;
    double data_offset[AXES_NUM];
    double data_cali[AXES_NUM];

    err = AKM_SetMagData((int32_t)inputData->x, (int32_t)inputData->y,
        (int32_t)inputData->z, inputData->timeStamp);
    if (err < 0) {
        osLog(LOG_ERROR, "akm_set_mag_data fail\n\r");
        return err;
    } else {
        AKM_Calibrate(data_cali, data_offset, &tmpStatus);
        outputData->x = data_cali[AXIS_X];
        outputData->y = data_cali[AXIS_Y];
        outputData->z = data_cali[AXIS_Z];
        outputData->x_bias = data_offset[AXIS_X];
        outputData->y_bias = data_offset[AXIS_Y];
        outputData->z_bias = data_offset[AXIS_Z];
        outputData->status = tmpStatus;
    }
    return 0;
}
static struct magCalibrationLibAPI akm09912CaliAPI = {
    .caliApiGetOffset = akm09912CaliApiGetOffset,
    .caliApiSetOffset = akm09912CaliApiSetOffset,
    .caliApiSetGyroData = akm09912CaliApiSetGyroData,
    .doCaliApi = akm09912DoCaliAPI
};

static void i2cAutoDetect(void *cookie, size_t tx, size_t rx, int err)
{
    if (err == 0) {
        osLog(LOG_INFO, "akm09912: auto detect success\n");
        mTask.mag_dev_info.layout = 0;
        mTask.mag_dev_info.deviceid = mTask.autoDetect[1];
        strncpy(mTask.mag_dev_info.libname, "akl", sizeof(mTask.mag_dev_info.libname));
        magRegisterInterruptMode(MAG_UNFIFO);
        registerMagDriverFsm(akm09912Fsm, ARRAY_SIZE(akm09912Fsm));
        registerMagCaliAPI(&akm09912CaliAPI);
        registerMagTimerCbk(akm09912TimerCbkF);
    } else
        osLog(LOG_ERROR, "akm09912: auto detect error (%d)\n", err);
}
int akm09912Init(void)
{
    int ret = 0;

    akm09912DebugPoint = &mTask;
    insertMagicNum(&mTask.magPacket);
    mTask.hw = get_cust_mag("akm09912");
    if (NULL == mTask.hw) {
        osLog(LOG_ERROR, "akm09912 get_cust_mag_hw fail\n");
        return 0;
    }
    mTask.i2c_addr = mTask.hw->i2c_addr[0];
    osLog(LOG_ERROR, "mag i2c_num: %d, i2c_addr: 0x%x\n", mTask.hw->i2c_num, mTask.i2c_addr);

    if (0 != (ret = sensorDriverGetConvert(mTask.hw->direction, &mTask.cvt))) {
        osLog(LOG_ERROR, "invalid direction: %d\n", mTask.hw->direction);
    }
    osLog(LOG_ERROR, "mag map[0]:%d, map[1]:%d, map[2]:%d, sign[0]:%d, sign[1]:%d, sign[2]:%d\n\r",
        mTask.cvt.map[AXIS_X], mTask.cvt.map[AXIS_Y], mTask.cvt.map[AXIS_Z],
        mTask.cvt.sign[AXIS_X], mTask.cvt.sign[AXIS_Y], mTask.cvt.sign[AXIS_Z]);
    AKM_Open();
    i2cMasterRequest(mTask.hw->i2c_num, I2C_SPEED);
    mTask.txBuf[0] = AKM09912_REG_ASAX;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
        (uint8_t *)&mTask.autoDetect, 1, &i2cAutoDetect, NULL);
}
MODULE_DECLARE(akm09912, SENS_TYPE_MAG, akm09912Init);
