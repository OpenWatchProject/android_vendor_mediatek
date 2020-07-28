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

#include <vendor_fusion.h>
#include <contexthub_fw.h>
#include "cache_internal.h"

#ifndef CFG_MAG_CALIBRATION_IN_AP
#include <memsic_wrapper.h>
#endif

#ifdef CFG_VENDOR_FUSION_SUPPORT
#ifndef CFG_MEMSIC_TINY_FUSION_SUPPORT
#include <memsic_wrapper.h>
#else
#include <memsic_wrapper.h>
#include <MemsicCommon.h>
#include <platform.h>
#include <MemsicNineAxisFusion.h>
#endif
#endif

#include "mmc3530.h"


#define MAG_NAME                    "mmc3530"
#define I2C_SPEED  400000
#define RESET_INTV 150

enum MMC3530State {
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
    STATE_CHIP_RESET,
    STATE_CHIP_CTRL1,
    STATE_CHIP_CTRL2,
    STATE_CHIP_CTRL3,
    STATE_CHIP_CTRL4,
    STATE_CHIP_BITS,
    STATE_CHIP_TM,
    STATE_CHIP_READ_OTP,
    STATE_CHIP_SAMPLE,
    STATE_CHIP_SAMPLE_OVER,
};

SRAM_REGION_BSS static struct MMC3530Task {
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

    int iRestart;
    unsigned short otpMatrix[3] ;
    struct mag_dev_info_t mag_dev_info;
} mTask;

static struct MMC3530Task *mmc3530DebugPoint;

SRAM_REGION_FUNCTION void mmc3530TimerCbkF(uint64_t time)
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

static void magGetSensorInfo(struct sensorInfo_t *data)
{
    strncpy(data->name, MAG_NAME, sizeof(data->name));
    memcpy(&data->mag_dev_info, &mTask.mag_dev_info, sizeof(struct mag_dev_info_t));
}

static int mmc3530Refill(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = -1;
    mTask.txBuf[0] = MMC3530_REG_CTRL;
    mTask.txBuf[1] = MMC3530_CTRL_REFILL;
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                      i2cCallBack, next_state);
    return ret;
}
static int mmc3530ChipRest(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                           void *inBuf, uint8_t inSize, uint8_t elemInSize,
                           void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = -1;
    mTask.txBuf[0] = MMC3530_REG_CTRL;
    mTask.txBuf[1] = MMC3530_CTRL_RESET;
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                      i2cCallBack, next_state);
    return ret;
}
static int mmc3530ChipCtrl1(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                            void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = -1;
    mTask.txBuf[0] = MMC3530_REG_CTRL;
    mTask.txBuf[1] = 0;
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                      i2cCallBack, next_state);
    return ret;
}
static int mmc3530ChipCtrl2(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                            void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = -1;
    mTask.txBuf[0] = MMC3530_REG_CTRL;
    mTask.txBuf[1] = MMC3530_CTRL_REFILL;
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                      i2cCallBack, next_state);
    return ret;
}



static int mmc3530ChipCtrl4(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                            void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = -1;
    mTask.txBuf[0] = MMC3530_REG_CTRL;
    mTask.txBuf[1] = 0;
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                      i2cCallBack, next_state);
    return ret;
}
static int mmc3530ChipBits(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                           void *inBuf, uint8_t inSize, uint8_t elemInSize,
                           void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = -1;
    mTask.txBuf[0] = MMC3530_REG_BITS;
    mTask.txBuf[1] = MMC3530_BITS_SLOW_16;
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                      i2cCallBack, next_state);
    return ret;
}
static int mmc3530ChipTm(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = -1;
    mTask.txBuf[0] = MMC3530_REG_CTRL;
    mTask.txBuf[1] = MMC3530_CTRL_TM;
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                      i2cCallBack, next_state);
    vTaskDelay(MMC3530_DELAY_TM);
    return ret;
}

static int mmc3530ReadOtp(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                          void *inBuf, uint8_t inSize, uint8_t elemInSize,
                          void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = -1;
    mTask.txBuf[0] = MMC3530_REG_OTP;
    ret = i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                        mTask.rxBuf, 4, i2cCallBack, next_state);
    return ret;
}
static void mmc3530_convert_otp(uint8_t *data)
{
    signed short stemp = 0;
    unsigned short utemp = 0;
    mTask.otpMatrix[0] = 1000;
    mTask.otpMatrix[1] = 1000;
    mTask.otpMatrix[2] = 1350;

    mTask.otpMatrix[0] = 1000;
    stemp = (signed short)(((data[1] & 0x03) << 4) | (data[2] >> 4));
    if (stemp >= 32)
        stemp = 32 - stemp;
    mTask.otpMatrix[1] = (unsigned short)(stemp * 6 + 1000); //*0.006*1000

    stemp = (signed short)(data[3] & 0x3f);
    if (stemp >= 32)
        stemp = 32 - stemp;
    utemp = (unsigned short)(stemp * 6 + 1000);        //magnify 1000 times
    mTask.otpMatrix[2] = utemp + (utemp * 3) / 10 + (utemp * 30 % 100 + utemp * 5) / 100;
}

static int mmc3530RegisterCore(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                               void *inBuf, uint8_t inSize, uint8_t elemInSize,
                               void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    struct sensorCoreInfo mInfo;
    memset(&mInfo, 0, sizeof(struct sensorCoreInfo));

    mmc3530_convert_otp(mTask.rxBuf);

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

static int mmc3530Enable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.iRestart = 1;
    mTask.txBuf[0] = MMC3530_REG_CTRL;
    mTask.txBuf[1] = MMC3530_CTRL_TM;

    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}
static int mmc3530Disable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                          void *inBuf, uint8_t inSize, uint8_t elemInSize,
                          void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mTask.iRestart = 0;
    mTask.txBuf[0] = MMC3530_REG_CTRL;
    mTask.txBuf[1] = MMC3530_CTRL_TM;

    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}
static int mmc3530Rate(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                       void *inBuf, uint8_t inSize, uint8_t elemInSize,
                       void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    struct magCntlPacket cntlPacket;

    ret = rxControlInfo(&cntlPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "mmc3530Rate, rx inSize and elemSize error\n");
        return -1;
    }
    magTimeBufferReset(&mTask.magTimeBuf);    //reset time buffer
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}


static int mmc3530GetStatus(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                            void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;

    ret = rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "%s mmc3530Sample, rx dataInfo error\n", __func__);
        return -1;
    }
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;

}
static int mmc3530Sample(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    static int read_idx = 0;

    mTask.txBuf[0] = MMC3530_REG_DATA;
    i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1, mTask.rxBuf, 6, NULL, NULL);
    read_idx++;

    if (!(read_idx % RESET_INTV)) {
        mTask.txBuf[0] = MMC3530_REG_CTRL;
        mTask.txBuf[1] = MMC3530_CTRL_RESET;
        i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack, next_state);
        read_idx = 0;
    } else {
        mTask.txBuf[0] = MMC3530_REG_CTRL;
        mTask.txBuf[1] = MMC3530_CTRL_TM;
        i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack, next_state);
    }

    return 0;
}

static int mmc3530Convert(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                          void *inBuf, uint8_t inSize, uint8_t elemInSize,
                          void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    struct magData *data = mTask.magPacket.outBuf;
    float remap_data[AXES_NUM];
    int32_t idata[AXES_NUM];
    uint64_t timestamp = 0;
    uint8_t data_size = 0;

    idata[AXIS_X] = (int)((uint16_t)((mTask.rxBuf[AXIS_X * 2 + 1] << 8) + (uint16_t)(mTask.rxBuf[AXIS_X * 2])));
    idata[AXIS_Y] = (int)((uint16_t)((mTask.rxBuf[AXIS_Y * 2 + 1] << 8) + (uint16_t)(mTask.rxBuf[AXIS_Y * 2])));
    idata[AXIS_Z] = (int)((uint16_t)((mTask.rxBuf[AXIS_Z * 2 + 1] << 8) + (uint16_t)(mTask.rxBuf[AXIS_Z * 2])));

    data[0].x = idata[AXIS_X];
    data[0].y = idata[AXIS_Y] - idata[AXIS_Z] + 32768;
    data[0].z = idata[AXIS_Y] + idata[AXIS_Z] - 32768;

    data[0].x -= 32768;
    data[0].y -= 32768;
    data[0].z -= 32768;

    data[0].x = data[0].x * mTask.otpMatrix[0] / 1000;
    data[0].y = data[0].y * mTask.otpMatrix[1] / 1000;
    data[0].z = (-1) * data[0].z * mTask.otpMatrix[2] / 1000;

    remap_data[mTask.cvt.map[AXIS_X]] = (mTask.cvt.sign[AXIS_X] * data[0].x) / 1024;
    remap_data[mTask.cvt.map[AXIS_Y]] = (mTask.cvt.sign[AXIS_Y] * data[0].y) / 1024;
    remap_data[mTask.cvt.map[AXIS_Z]] = (mTask.cvt.sign[AXIS_Z] * data[0].z) / 1024;

    mTask.factoryData.ix = (int32_t)(remap_data[AXIS_X] * MAGNETOMETER_INCREASE_NUM_AP * MAGNETOMETER_INCREASE_NUM_AP);
    mTask.factoryData.iy = (int32_t)(remap_data[AXIS_Y] * MAGNETOMETER_INCREASE_NUM_AP * MAGNETOMETER_INCREASE_NUM_AP);
    mTask.factoryData.iz = (int32_t)(remap_data[AXIS_Z] * MAGNETOMETER_INCREASE_NUM_AP * MAGNETOMETER_INCREASE_NUM_AP);

    data_size = magTimeBufferSize(&mTask.magTimeBuf);
    for (uint8_t i = 0; i < data_size; i++) {
        data[i].x = remap_data[AXIS_X];
        data[i].y = remap_data[AXIS_Y];
        data[i].z = remap_data[AXIS_Z];
        magTimeBufferRead(&mTask.magTimeBuf, &mTask.hwSampleTime);
        timestamp = addThenRetreiveAverageMagTimeStamp(mTask.hwSampleTime);
    }

    txTransferDataInfo(&mTask.dataInfo, data_size, timestamp, data);
    /*osLog(LOG_ERROR, "mmc3530Convert raw data: %f, %f, %f, timestamp: %llu size: %u!\n", (double)remap_data[AXIS_X],
        (double)remap_data[AXIS_Y], (double)remap_data[AXIS_Z], timestamp, data_size); */

    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int mmc3530SampleDone(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}
static struct sensorFsm mmc3530Fsm[] = {

    sensorFsmCmd(STATE_SAMPLE, STATE_CHIP_SAMPLE, mmc3530GetStatus),
    sensorFsmCmd(STATE_CHIP_SAMPLE, STATE_CONVERT, mmc3530Sample),
    sensorFsmCmd(STATE_CONVERT, STATE_CHIP_SAMPLE_OVER, mmc3530Convert),
    sensorFsmCmd(STATE_CHIP_SAMPLE_OVER, STATE_SAMPLE_DONE, mmc3530SampleDone),


    sensorFsmCmd(STATE_ENABLE, STATE_ENABLE_DONE, mmc3530Enable),
    sensorFsmCmd(STATE_DISABLE, STATE_DISABLE_DONE, mmc3530Disable),

    sensorFsmCmd(STATE_RATECHG, STATE_RATECHG_DONE, mmc3530Rate),

    sensorFsmCmd(STATE_FUSE_EN, STATE_CHIP_RESET, mmc3530Refill),
    sensorFsmCmd(STATE_CHIP_RESET, STATE_CHIP_CTRL1, mmc3530ChipRest),
    sensorFsmCmd(STATE_CHIP_CTRL1, STATE_CHIP_CTRL2, mmc3530ChipCtrl1),
    sensorFsmCmd(STATE_CHIP_CTRL2, STATE_CHIP_CTRL4, mmc3530ChipCtrl2),
    sensorFsmCmd(STATE_CHIP_CTRL4, STATE_CHIP_BITS, mmc3530ChipCtrl4),
    sensorFsmCmd(STATE_CHIP_BITS, STATE_CHIP_TM, mmc3530ChipBits),
    sensorFsmCmd(STATE_CHIP_TM, STATE_CHIP_READ_OTP, mmc3530ChipTm),
    sensorFsmCmd(STATE_CHIP_READ_OTP, STATE_CORE, mmc3530ReadOtp),

    sensorFsmCmd(STATE_CORE, STATE_INIT_DONE, mmc3530RegisterCore),
};

#ifndef CFG_MAG_CALIBRATION_IN_AP

static int mmc3530CaliApiGetOffset(float offset[AXES_NUM])
{
    return 0;
}
static int mmc3530CaliApiSetOffset(float offset[AXES_NUM])
{
    return 0;
}

static int memsic3530CaliApiSetCaliParam(int32_t caliParameter[6])
{
    MEMEMSIC_SetMagPara(caliParameter);
    return 0;
}
static int memsic3530CaliApiGetCaliParam(int32_t caliParameter[6])
{
    MEMEMSIC_GetMagPara(caliParameter);
    return 0;
}
static int mmc3530CaliApiSetGyroData(struct magCaliDataInPut *inputData)
{
    return MEMSIC_CaliApiSetGyroData((struct magDataInPut *)inputData);

}

static int mmc3530DoCaliAPI(struct magCaliDataInPut *inputData,
                            struct magCaliDataOutPut *outputData)
{
    MEMSIC_DoCaliAPI((struct magDataInPut *)inputData, (struct magDataOutPut *)outputData,  mTask.iRestart);
    mTask.iRestart = 0;

    return 0;
}
static int mmc3530CaliInitLib(int hwGyroSupport)
{
    return MEMSIC_InitLib(hwGyroSupport);
}

static struct magCalibrationLibAPI mmc3530CaliAPI = {
    .initLib = mmc3530CaliInitLib,
    .caliApiGetOffset = mmc3530CaliApiGetOffset,
    .caliApiSetOffset = mmc3530CaliApiSetOffset,
    .caliApiGetCaliParam = memsic3530CaliApiGetCaliParam,
    .caliApiSetCaliParam = memsic3530CaliApiSetCaliParam,
    .caliApiSetGyroData = mmc3530CaliApiSetGyroData,
    .doCaliApi = mmc3530DoCaliAPI
};
#endif

#ifdef CFG_VENDOR_FUSION_SUPPORT
#ifndef CFG_MEMSIC_TINY_FUSION_SUPPORT
static int mmc3530FusionInitLib(int hwGyroSupport)
{
    return MEMSIC_InitLib(hwGyroSupport);
}

static int mmc3530FusionSetGyro(struct InterfaceDataIn *inData)
{
    return MEMSIC_FusionSetGyroData(inData);
}

static int mmc3530FusionSetAcc(struct InterfaceDataIn *inData)
{
    return MEMSIC_FusionSetAccData(inData);
}

static int mmc3530FusionSetMag(struct InterfaceDataIn *inData)
{
    return MEMSIC_FusionSetMagData(inData);
}

static int mmc3530FusionGetGravity(struct InterfaceDataOut *outData)
{
    return MEMSIC_GetGravity(outData);
}

static int mmc3530FusionGetRotationVector(struct InterfaceDataOut *outData)
{
    return MEMSIC_GetRotaionVector(outData);
}

static int mmc3530FusionGetOrientation(struct InterfaceDataOut *outData)
{
    return MEMSIC_GetOrientaion(outData);
}

static int mmc3530FusionGetLinearaccel(struct InterfaceDataOut *outData)
{
    return MEMSIC_GetLinearaccel(outData);
}

static int mmc3530FusionGetGameRotationVector(struct InterfaceDataOut *outData)
{
    return MEMSIC_GetGameRotaionVector(outData);
}

static int mmc3530FusionGetGeoMagnetic(struct InterfaceDataOut *outData)
{
    return MEMSIC_GetGeoMagnetic(outData);
}

static int mmc3530FusionGetVirtualGyro(struct InterfaceDataOut *outData)
{
    return MEMSIC_GetVirtualGyro(outData);
}

static struct VendorFusionInterfact_t mmc3530Interface = {
    .name = "mmc3530",
    .initLib = mmc3530FusionInitLib,
    .setGyroData = mmc3530FusionSetGyro,
    .setAccData = mmc3530FusionSetAcc,
    .setMagData = mmc3530FusionSetMag,
    .getGravity = mmc3530FusionGetGravity,
    .getRotationVector = mmc3530FusionGetRotationVector,
    .getOrientation = mmc3530FusionGetOrientation,
    .getLinearaccel = mmc3530FusionGetLinearaccel,
    .getGameRotationVector = mmc3530FusionGetGameRotationVector,
    .getGeoMagnetic = mmc3530FusionGetGeoMagnetic,
    .getVirtualGyro = mmc3530FusionGetVirtualGyro
};

#else //MEMSIC_TINY_FUSION

static int mmc3530FusionInitLib(int hwGyroSupport)
{
    osLog(LOG_ERROR, "%s: hwgyrosupport:%d\n", __func__, hwGyroSupport);
    return MEMSIC_InitLib(hwGyroSupport);
}
static int mmc3530FusionSetGyro(struct InterfaceDataIn *inData)
{
    return MEMSIC_FusionSetGyroData(inData->vec[0],
                                    inData->vec[1], inData->vec[2], inData->timeStamp);

}
static int mmc3530FusionSetAcc(struct InterfaceDataIn *inData)
{
    return MEMSIC_FusionSetAccData(inData->vec[0],
                                   inData->vec[1], inData->vec[2], inData->timeStamp);
}

static int mmc3530FusionGetGravity(struct InterfaceDataOut *outData)
{
    int ret = 0;
    float vec[3] = {0};
    int16_t accurancy = 0;

    ret = MEMSIC_GetGravity(vec, &accurancy);
    outData->vec[0] = vec[0];
    outData->vec[1] = vec[1];
    outData->vec[2] = vec[2];
    outData->status = accurancy;
    return ret;
}

static int mmc3530FusionGetLinearaccel(struct InterfaceDataOut *outData)
{
    int ret = 0;
    float vec[3] = {0};
    int16_t accurancy = 0;

    ret = MEMSIC_GetLinearaccel(vec, &accurancy);
    outData->vec[0] = vec[0];
    outData->vec[1] = vec[1];
    outData->vec[2] = vec[2];
    outData->status = accurancy;
    return ret;
}

static int mmc3530FusionGetGameRotationVector(struct InterfaceDataOut *outData)
{
    int ret = 0;
    float vec[4] = {0};
    int16_t accurancy = 3;

    ret = MEMSIC_GetGameRotaionVector(vec, &accurancy);
    outData->vec[0] = vec[0];
    outData->vec[1] = vec[1];
    outData->vec[2] = vec[2];
    outData->vec[3] = vec[3];
    outData->status = accurancy;
    return ret;
}

static struct VendorFusionInterfact_t memsicInterface = {
    .name = "memsic",
    .initLib = mmc3530FusionInitLib,
    .setGyroData = mmc3530FusionSetGyro,
    .setAccData = mmc3530FusionSetAcc,
    .getGravity = mmc3530FusionGetGravity,
    .getLinearaccel = mmc3530FusionGetLinearaccel,
    .getGameRotationVector = mmc3530FusionGetGameRotationVector,
};

#endif //CFG_MEMSIC_TINY_FUSION_SUPPORT
#endif // VENDOR_FUSION

static int mmc3530Init(void)
{
    int ret = 0;

    mmc3530DebugPoint = &mTask;
    insertMagicNum(&mTask.magPacket);
    mTask.hw = get_cust_mag("mmc3530");
    if (NULL == mTask.hw) {
        osLog(LOG_ERROR, "mmc3530 get_cust_mag fail\n");
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
    mTask.txBuf[0] = MMC3530_REG_PID;
    ret = i2cMasterTxRxSync(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                            (uint8_t *)mTask.autoDetect, 1, NULL, NULL);
    if (ret < 0) {
        osLog(LOG_ERROR, "mmc3530 i2cMasterTxRxSync fail!!!\n");
        ret = -1;
        i2cMasterRelease(mTask.hw->i2c_num);
        goto err_out;
    }

    if (mTask.autoDetect[0] == MMC3530_DEVICE_ID) {   // mmc3530 sensor id
        osLog(LOG_INFO, "mmc3530: auto detect sucess: %d\n", mTask.autoDetect[0]);
        strncpy(mTask.mag_dev_info.libname, "memsic", sizeof(mTask.mag_dev_info.libname));

        magSensorRegister();
        magRegisterInterruptMode(MAG_UNFIFO);
        registerMagDriverFsm(mmc3530Fsm, ARRAY_SIZE(mmc3530Fsm));
        registerMagTimerCbk(mmc3530TimerCbkF);

#ifndef CFG_MAG_CALIBRATION_IN_AP
        registerMagCaliAPI(&mmc3530CaliAPI);
#endif
#ifdef CFG_VENDOR_FUSION_SUPPORT
#ifndef CFG_MEMSIC_TINY_FUSION_SUPPORT
        registerVendorInterface(&mmc3530Interface);
#else
        registerVendorInterface(&memsicInterface);
#endif
#endif

    } else {
        i2cMasterRelease(mTask.hw->i2c_num);
        osLog(LOG_ERROR, "mmc3530 read id fail!!!\n");
        ret = -1;
        goto err_out;
    }

err_out:
    return ret;
}
#ifndef CFG_OVERLAY_INIT_SUPPORT
MODULE_DECLARE(mmc3530, SENS_TYPE_MAG, mmc3530Init);
#else
#include "mtk_overlay_init.h"
OVERLAY_DECLARE(mmc3530, OVERLAY_ID_MAG, mmc3530Init);
#endif
