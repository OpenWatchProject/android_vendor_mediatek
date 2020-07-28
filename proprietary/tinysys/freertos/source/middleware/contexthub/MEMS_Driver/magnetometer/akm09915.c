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

#include "cache_internal.h"
#include <vendor_fusion.h>

#ifndef CFG_MAG_CALIBRATION_IN_AP
#include <akm_wrapper.h>
#endif

#ifdef CFG_VENDOR_FUSION_SUPPORT
#include <akm_wrapper.h>
#endif

#define MAG_NAME                    "akm09915"
#define I2C_SPEED                       400000
#define AKM09915_DATA_LEN       8

/* Device specific constant values */
#define AKM09915_REG_WIA1           0x00
#define AKM09915_REG_WIA2           0x01
#define AKM09915_REG_INFO1          0x02
#define AKM09915_REG_INFO2          0x03
#define AKM09915_REG_ST1                0x10
#define AKM09915_REG_HXL                0x11
#define AKM09915_REG_HXH                0x12
#define AKM09915_REG_HYL                0x13
#define AKM09915_REG_HYH                0x14
#define AKM09915_REG_HZL                0x15
#define AKM09915_REG_HZH                0x16
#define AKM09915_REG_TMPS           0x17
#define AKM09915_REG_ST2                0x18
#define AKM09915_REG_CNTL1          0x30
#define AKM09915_REG_CNTL2          0x31
#define AKM09915_REG_CNTL3          0x32
#define AKM09915_REG_ASAX           0x60
#define AKM09915_REG_ASAY           0x61
#define AKM09915_REG_ASAZ           0x62


/*! \name AKM09915 operation mode
 \anchor AKM09915_Mode
 Defines an operation mode of the AKM09915.*/
#define AKM09915_MODE_SNG_MEASURE   0x01
#define AKM09915_MODE_10HZ_MEASURE  0x02
#define AKM09915_MODE_20HZ_MEASURE  0x04
#define AKM09915_MODE_50HZ_MEASURE  0x06
#define AKM09915_MODE_100HZ_MEASURE 0x08
#define AKM09915_MODE_SELF_TEST     0x10
#define AKM09915_MODE_FUSE_ACCESS   0x1F
#define AKM09915_MODE_POWERDOWN     0x00
#define AKM09915_RESET_DATA         0x01

#define DATA_READY_BIT              0X01

enum AKM09915State {
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

SRAM_REGION_BSS static struct AKM09915Task {
    /* txBuf for i2c operation, fill register and fill value */
    uint8_t txBuf[8];
    /* rxBuf for i2c operation, receive rawdata */
    uint8_t rxBuf[8];
    uint8_t fuse[3];
    uint8_t autoDetect[2];
    uint64_t hwSampleTime;
    struct magTimeStampBuffer magTimeBuf; /* please define in sram region */
    struct transferDataInfo dataInfo;
    struct magDataPacket magPacket;
    /* data for factory */
    struct TripleAxisDataPoint factoryData;
    struct mag_hw *hw;
    struct sensorDriverConvert cvt;
    uint8_t i2c_addr;
    struct mag_dev_info_t mag_dev_info;
} mTask;
static struct AKM09915Task *akm09915DebugPoint;

SRAM_REGION_FUNCTION void akm09915TimerCbkF(uint64_t time)
{
    magTimeBufferWrite(&mTask.magTimeBuf, time);
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

static int akm09915FuseEn(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                          void *inBuf, uint8_t inSize, uint8_t elemInSize,
                          void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "AKM09915: state reset\n");
    mTask.txBuf[0] = AKM09915_REG_CNTL2;
    mTask.txBuf[1] = AKM09915_MODE_FUSE_ACCESS;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}
static int akm09915FuseRd(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                          void *inBuf, uint8_t inSize, uint8_t elemInSize,
                          void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "AKM09915: state id\n");

    mTask.txBuf[0] = AKM09915_REG_ASAX;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         (uint8_t *)&mTask.fuse, 3, i2cCallBack,
                         next_state);
}
static void magGetSensorInfo(struct sensorInfo_t *data)
{
    strncpy(data->name, MAG_NAME, sizeof(data->name));
    memcpy(&data->mag_dev_info, &mTask.mag_dev_info, sizeof(struct mag_dev_info_t));
}

static int akm09915RegisterCore(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
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

static int akm09915Enable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                          void *inBuf, uint8_t inSize, uint8_t elemInSize,
                          void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "AKM09915: state enable mag\n");
#ifndef CFG_MAG_CALIBRATION_IN_AP
    AKM_EnableCalibrate();
#endif

    mTask.txBuf[0] = AKM09915_REG_CNTL2;
    mTask.txBuf[1] = AKM09915_MODE_50HZ_MEASURE;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}
static int akm09915Disable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                           void *inBuf, uint8_t inSize, uint8_t elemInSize,
                           void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "AKM09915: state disable mag\n");

    mTask.txBuf[0] = AKM09915_REG_CNTL2;
    mTask.txBuf[1] = AKM09915_MODE_POWERDOWN;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}
static int akm09915Rate(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                        void *inBuf, uint8_t inSize, uint8_t elemInSize,
                        void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    uint32_t  sample_rate, water_mark;
    struct magCntlPacket cntlPacket;

    ret = rxControlInfo(&cntlPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "akm09915Rate, rx inSize and elemSize error\n");
        return -1;
    }
    sample_rate = cntlPacket.rate;
    water_mark = cntlPacket.waterMark;
    magTimeBufferReset(&mTask.magTimeBuf);    //reset time buffer
    osLog(LOG_INFO, "akm09915Rate: %u, water_mark:%u\n", sample_rate, water_mark);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int akm09915Sample(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                          void *inBuf, uint8_t inSize, uint8_t elemInSize,
                          void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    ret = rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "akm09915Sample, rx dataInfo error\n");
        return -1;
    }
    mTask.txBuf[0] = AKM09915_REG_HXL;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 8, i2cCallBack,
                         next_state);
}
static int akm09915Convert(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                           void *inBuf, uint8_t inSize, uint8_t elemInSize,
                           void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    struct magData *data = mTask.magPacket.outBuf;
    uint8_t data_size = 0;
    float remap_data[AXES_NUM];
    int32_t idata[AXES_NUM];
    uint64_t timestamp = 0;

    idata[AXIS_X] = (int16_t)((mTask.rxBuf[AXIS_X * 2 + 1] << 8) | (mTask.rxBuf[AXIS_X * 2]));
    idata[AXIS_Y] = (int16_t)((mTask.rxBuf[AXIS_Y * 2 + 1] << 8) | (mTask.rxBuf[AXIS_Y * 2]));
    idata[AXIS_Z] = (int16_t)((mTask.rxBuf[AXIS_Z * 2 + 1] << 8) | (mTask.rxBuf[AXIS_Z * 2]));

    remap_data[mTask.cvt.map[AXIS_X]] = mTask.cvt.sign[AXIS_X] * idata[AXIS_X];
    remap_data[mTask.cvt.map[AXIS_Y]] = mTask.cvt.sign[AXIS_Y] * idata[AXIS_Y];
    remap_data[mTask.cvt.map[AXIS_Z]] = mTask.cvt.sign[AXIS_Z] * idata[AXIS_Z];

    mTask.factoryData.ix = (int32_t)(remap_data[AXIS_X] * MAGNETOMETER_INCREASE_NUM_AP);
    mTask.factoryData.iy = (int32_t)(remap_data[AXIS_Y] * MAGNETOMETER_INCREASE_NUM_AP);
    mTask.factoryData.iz = (int32_t)(remap_data[AXIS_Z] * MAGNETOMETER_INCREASE_NUM_AP);

    data_size = magTimeBufferSize(&mTask.magTimeBuf);
    for (uint8_t i = 0; i < data_size; i++) {
        data[i].x = remap_data[AXIS_X];
        data[i].y = remap_data[AXIS_Y];
        data[i].z = remap_data[AXIS_Z];
        magTimeBufferRead(&mTask.magTimeBuf, &mTask.hwSampleTime);
        timestamp = addThenRetreiveAverageMagTimeStamp(mTask.hwSampleTime);
    }

    txTransferDataInfo(&mTask.dataInfo, data_size, timestamp, data);
    /*osLog(LOG_ERROR, "akm09915Convert raw data: %f, %f, %f, timestamp: %llu size: %u!\n", (double)remap_data[AXIS_X],
    (double)remap_data[AXIS_Y], (double)remap_data[AXIS_Z], timestamp, data_size); */

    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}
static struct sensorFsm akm09915Fsm[] = {
    sensorFsmCmd(STATE_SAMPLE, STATE_CONVERT, akm09915Sample),
    sensorFsmCmd(STATE_CONVERT, STATE_SAMPLE_DONE, akm09915Convert),

    sensorFsmCmd(STATE_ENABLE, STATE_ENABLE_DONE, akm09915Enable),
    sensorFsmCmd(STATE_DISABLE, STATE_DISABLE_DONE, akm09915Disable),

    sensorFsmCmd(STATE_RATECHG, STATE_RATECHG_DONE, akm09915Rate),

    sensorFsmCmd(STATE_FUSE_EN, STATE_FUSE_RD, akm09915FuseEn),
    sensorFsmCmd(STATE_FUSE_RD, STATE_DISABLE, akm09915FuseRd),
    sensorFsmCmd(STATE_DISABLE, STATE_CORE, akm09915Disable),
    sensorFsmCmd(STATE_CORE, STATE_INIT_DONE, akm09915RegisterCore),
};

#ifndef CFG_MAG_CALIBRATION_IN_AP
static int akm09915CaliInitLib(int hwGyroSupported)
{
    AKM_SetGyroFlagForCalibrate(hwGyroSupported);
    return 0;
}

static int akm09915CaliApiGetOffset(float offset[AXES_NUM])
{
    return 0;
}
static int akm09915CaliApiSetOffset(float offset[AXES_NUM])
{
    AKM_ReloadContext(offset);
    return 0;
}
static int akm09915CaliApiSetGyroData(struct magCaliDataInPut *inputData)
{
    return AKM_SetGyroData((double)inputData->x, (double)inputData->y,
                           (double)inputData->z, inputData->timeStamp);
}
static int akm09915DoCaliAPI(struct magCaliDataInPut *inputData,
                             struct magCaliDataOutPut *outputData)
{
    int err = 0;
    int16_t tmpStatus = 0;
    double data_offset[AXES_NUM] = {0};
    double data_cali[AXES_NUM] = {0};

    err = AKM_SetMagData((int32_t)inputData->x, (int32_t)inputData->y,
        (int32_t)inputData->z, inputData->timeStamp);
    if (err < 0)
        return err;
    err = AKM_GetMagData(data_cali, data_offset, &tmpStatus);
    if (err < 0)
        return err;
    outputData->x = data_cali[AXIS_X];
    outputData->y = data_cali[AXIS_Y];
    outputData->z = data_cali[AXIS_Z];
    outputData->x_bias = data_offset[AXIS_X];
    outputData->y_bias = data_offset[AXIS_Y];
    outputData->z_bias = data_offset[AXIS_Z];
    outputData->status = tmpStatus;

    return 0;
}
static int akm09915CaliApiGetCaliParam(int32_t caliParameter[6])
{
    //osLog(LOG_INFO, "akm09915CaliApiGetCaliParam :%d, %d, %d, %d, %d, %d\n",
    //caliParameter[0], caliParameter[1], caliParameter[2],
    //caliParameter[3], caliParameter[4], caliParameter[5]);
    AKM_SaveCaliInfoToFile(caliParameter);
    return 0;
}
static int akm09915CaliApiSetCaliParam(int32_t caliParameter[6])
{
    AKM_ReloadCaliInfoFromFile(caliParameter);
    //osLog(LOG_INFO, "SetCaliParam :%d, %d, %d, %d, %d, %d\n",
    //caliParameter[0], caliParameter[1], caliParameter[2],
    //caliParameter[3], caliParameter[4], caliParameter[5]);
    AKM_Open();
    return 0;
}

static struct magCalibrationLibAPI akm09915CaliAPI = {
    .initLib = akm09915CaliInitLib,
    .caliApiGetOffset = akm09915CaliApiGetOffset,
    .caliApiSetOffset = akm09915CaliApiSetOffset,
    .caliApiGetCaliParam = akm09915CaliApiGetCaliParam,
    .caliApiSetCaliParam = akm09915CaliApiSetCaliParam,
    .caliApiSetGyroData = akm09915CaliApiSetGyroData,
    .doCaliApi = akm09915DoCaliAPI
};
#endif

#ifdef CFG_VENDOR_FUSION_SUPPORT
static int akm09915FusionInitLib(int hwGyroSupport)
{
    AKM_SetGyroFlagForFusion(hwGyroSupport);
    return 0;
}
static int akm09915FusionEnableLib(struct InterfaceEnParam *pEnParam)
{
    if (pEnParam->en) {
        AKM_EnableFusion();
        AKM_FusionSetLatency(pEnParam->latency);
    } else
        AKM_DisableFusion();

    return 0;
}

static int akm09915FusionSetGyro(struct InterfaceDataIn *inData)
{
    return AKM_FusionSetGyroData(inData->vec[0],
                                 inData->vec[1], inData->vec[2], inData->timeStamp);
}
static int akm09915FusionSetAcc(struct InterfaceDataIn *inData)
{
    return AKM_FusionSetAccData(inData->vec[0],
                                inData->vec[1], inData->vec[2], inData->timeStamp);
}
static int akm09915FusionSetMag(struct InterfaceDataIn *inData)
{
    return AKM_FusionSetMagData(inData->vec[0],
                                inData->vec[1], inData->vec[2], inData->timeStamp);
}
static int akm09915FusionGetGravity(struct InterfaceDataOut *outData)
{
    int ret = 0;
    double vec[6] = {0};
    int16_t accurancy = 0;

    ret = AKM_GetGravity(vec, &accurancy);
    outData->vec[0] = vec[0];
    outData->vec[1] = vec[1];
    outData->vec[2] = vec[2];
    outData->status = accurancy;
    return ret;
}
static int akm09915FusionGetRotationVector(struct InterfaceDataOut *outData)
{
    int ret = 0;
    double vec[6] = {0};
    int16_t accurancy = 0;

    ret = AKM_GetRotationVector(vec, &accurancy);
    outData->vec[0] = vec[0];
    outData->vec[1] = vec[1];
    outData->vec[2] = vec[2];
    outData->vec[3] = vec[3];
    outData->status = accurancy;
    return ret;
}
static int akm09915FusionGetOrientation(struct InterfaceDataOut *outData)
{
    int ret = 0;
    double vec[6] = {0};
    int16_t accurancy = 0;

    ret = AKM_GetOri(vec, &accurancy);
    outData->vec[0] = vec[0];
    outData->vec[1] = vec[1];
    outData->vec[2] = vec[2];
    outData->status = accurancy;
    return ret;
}
static int akm09915FusionGetLinearaccel(struct InterfaceDataOut *outData)
{
    int ret = 0;
    double vec[6] = {0};
    int16_t accurancy = 0;

    ret = AKM_GetLinearAccelerometer(vec, &accurancy);
    outData->vec[0] = vec[0];
    outData->vec[1] = vec[1];
    outData->vec[2] = vec[2];
    outData->status = accurancy;
    return ret;
}
static int akm09915FusionGetGameRotationVector(struct InterfaceDataOut *outData)
{
    int ret = 0;
    double vec[6] = {0};
    int16_t accurancy = 0;

    ret = AKM_GetRotationVector(vec, &accurancy);
    outData->vec[0] = vec[0];
    outData->vec[1] = vec[1];
    outData->vec[2] = vec[2];
    outData->vec[3] = vec[3];
    outData->status = accurancy;
    return ret;
}
static int akm09915FusionGetGeoMagnetic(struct InterfaceDataOut *outData)
{
    int ret = 0;
    double vec[6] = {0};
    int16_t accurancy = 0;

    ret = AKM_GetRotationVector(vec, &accurancy);
    outData->vec[0] = vec[0];
    outData->vec[1] = vec[1];
    outData->vec[2] = vec[2];
    outData->vec[3] = vec[3];
    outData->status = accurancy;
    return ret;
}
static int akm09915FusionGetVirtualGyro(struct InterfaceDataOut *outData)
{
    int ret = 0;
    double vec[6] = {0};
    int16_t accurancy = 0;

    ret = AKM_GetGyroscope(vec, &accurancy);
    outData->vec[0] = vec[0];
    outData->vec[1] = vec[1];
    outData->vec[2] = vec[2];
    outData->status = accurancy;
    return ret;
}

static struct VendorFusionInterfact_t akm09915Interface = {
    .name = "akm09915",
    .initLib = akm09915FusionInitLib,
    .enableLib = akm09915FusionEnableLib,
    .setGyroData = akm09915FusionSetGyro,
    .setAccData = akm09915FusionSetAcc,
    .setMagData = akm09915FusionSetMag,
    .getGravity = akm09915FusionGetGravity,
    .getRotationVector = akm09915FusionGetRotationVector,
    .getOrientation = akm09915FusionGetOrientation,
    .getLinearaccel = akm09915FusionGetLinearaccel,
    .getGameRotationVector = akm09915FusionGetGameRotationVector,
    .getGeoMagnetic = akm09915FusionGetGeoMagnetic,
    .getVirtualGyro = akm09915FusionGetVirtualGyro
};
#endif // VENDOR_FUSION

static int akm09915Init(void)
{
    int ret = 0;

    akm09915DebugPoint = &mTask;
    insertMagicNum(&mTask.magPacket);
    mTask.hw = get_cust_mag("akm09915");
    if (NULL == mTask.hw) {
        osLog(LOG_ERROR, "akm09915 get_cust_mag fail\n");
        ret = -1;
        goto err_out;
    }
    mTask.i2c_addr = mTask.hw->i2c_addr[0];
    osLog(LOG_ERROR, "mag i2c_num: %d, i2c_addr: 0x%x\n", mTask.hw->i2c_num, mTask.i2c_addr);

    if (0 != (ret = sensorDriverGetConvert(mTask.hw->direction, &mTask.cvt))) {
        osLog(LOG_ERROR, "invalid direction: %d\n", mTask.hw->direction);
    }
    osLog(LOG_ERROR, "mag map[0]:%d, map[1]:%d, map[2]:%d, sign[0]:%d, sign[1]:%d, sign[2]:%d\n\r",
          mTask.cvt.map[AXIS_X], mTask.cvt.map[AXIS_Y], mTask.cvt.map[AXIS_Z],
          mTask.cvt.sign[AXIS_X], mTask.cvt.sign[AXIS_Y], mTask.cvt.sign[AXIS_Z]);
    i2cMasterRequest(mTask.hw->i2c_num, I2C_SPEED);
    mTask.txBuf[0] = AKM09915_REG_WIA1;

    for (uint8_t i = 0; i < 3; i++) {
        ret = i2cMasterTxRxSync(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                                (uint8_t *)mTask.autoDetect, 2, NULL, NULL);
        if (ret >= 0)
            break;
    }
    if (ret < 0) {
        ret = -1;
        i2cMasterRelease(mTask.hw->i2c_num);
        sendSensorErrToAp(ERR_SENSOR_MAG, ERR_CASE_MAG_INIT, MAG_NAME);
        goto err_out;
    }
    if ((mTask.autoDetect[0] == 0x48) && (mTask.autoDetect[1] == 0x10)) {  // ak09915 sensor id
        osLog(LOG_INFO, "akm09915: auto detect success\n");
        goto success_out;
    } else if ((mTask.autoDetect[0] == 0x48) && (mTask.autoDetect[1] == 0x09)) {  // ak09916C sensor id
        osLog(LOG_INFO, "akm09916c: auto detect success\n");
        goto success_out;
    } else {
        ret = -1;
        i2cMasterRelease(mTask.hw->i2c_num);
        sendSensorErrToAp(ERR_SENSOR_MAG, ERR_CASE_MAG_INIT, MAG_NAME);
        goto err_out;
    }
success_out:
    mTask.mag_dev_info.layout = 0x00;
    mTask.mag_dev_info.deviceid = mTask.autoDetect[1];
    strncpy(mTask.mag_dev_info.libname, "akm", sizeof(mTask.mag_dev_info.libname));

    magSensorRegister();
    magRegisterInterruptMode(MAG_UNFIFO);
    registerMagDriverFsm(akm09915Fsm, ARRAY_SIZE(akm09915Fsm));
    registerMagTimerCbk(akm09915TimerCbkF);
#ifndef CFG_MAG_CALIBRATION_IN_AP
    registerMagCaliAPI(&akm09915CaliAPI);
    AKM_SetDevice(mTask.autoDetect);
    AKM_Open();
#endif

#ifdef CFG_VENDOR_FUSION_SUPPORT
    registerVendorInterface(&akm09915Interface);
#endif

err_out:
    return ret;
}
#ifndef CFG_OVERLAY_INIT_SUPPORT
MODULE_DECLARE(akm09915, SENS_TYPE_MAG, akm09915Init);
#else
#include "mtk_overlay_init.h"
OVERLAY_DECLARE(akm09915, OVERLAY_ID_MAG, akm09915Init);
#endif
