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
#include <memsic_wrapper.h>
#include <vendor_fusion.h>
#include "mmc5603.h"
#include "magnetometer.h"
#include "cache_internal.h"

#include <MemsicConfig.h>
#include "./lib/memsic/MemsicOri.h"
#include "./lib/memsic/MemsicGyr.h"
#include "./lib/memsic/MemsicRov.h"
#include "./lib/memsic/MemsicConfig.h"
#include "./lib/memsic/MemsicNineAxisFusion.h"

static int has_Gyro=0;
//#define MEMSIC_DEBUG 1
#define I2C_SPEED                       400000
static int iRestart = 1;

enum MMC5603State {
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

SRAM_REGION_BSS static struct MMC5603Task {
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
} mTask;
static struct MMC5603Task *mmc5603DebugPoint;


SRAM_REGION_FUNCTION void mmc5603TimerCbkF(uint64_t time)
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


#ifdef VENDOR_EDIT
static void mmc5603_selftest(int32_t *testResult)
{
    int temp_result = -1;
    unsigned char buf[2]={0};
    int32_t prev_raw[3] = {0};
    int32_t curr_raw[3] = {0};

    buf[0] = MMC5603NJ_REG_CTRL1;
    buf[1] = 0x00;
    i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, buf, 2, NULL,NULL);
    vTaskDelay(1);

    buf[0]  = MMC5603NJ_REG_ODR;
    buf[1] = 0x00;
    i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, buf, 2, NULL,NULL);
    vTaskDelay(1);

    buf[0] = MMC5603NJ_REG_CTRL0;
    buf[1] = 0x00;
    i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, buf, 2, NULL,NULL);
    vTaskDelay(1);


    buf[0] = MMC5603NJ_REG_CTRL2;
    buf[1] = MMC5603NJ_CMD_CMM_DISEN;
    i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, buf, 2, NULL,NULL);
    vTaskDelay(1);

    buf[0] = MMC5603NJ_REG_CTRL0;
    buf[1] = MMC5603NJ_MANU_RESET;
    i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, buf, 2,NULL,NULL);
    vTaskDelay(1);

    buf[0] = MMC5603NJ_REG_CTRL0;
    buf[1] = MMC5603NJ_MANU_TM;
    i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, buf, 2,NULL,NULL);
    vTaskDelay(10);

    buf[0] = MMC5603_REG_DATA;
    i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, buf, 1,  mTask.rxBuf, 6, NULL,NULL);

    prev_raw[0] = (int)( (uint16_t)((mTask.rxBuf[0] << 8) + (uint16_t)(mTask.rxBuf[1])) );
    prev_raw[1] = (int)( (uint16_t)((mTask.rxBuf[2] << 8) + (uint16_t)(mTask.rxBuf[3])) );
    prev_raw[2] = (int)( (uint16_t)((mTask.rxBuf[4] << 8) + (uint16_t)(mTask.rxBuf[5])) );

    buf[0] = MMC5603NJ_REG_CTRL0;
    buf[1] = MMC5603NJ_MANU_SET;
    i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, buf, 2,NULL,NULL);
    vTaskDelay(1);
    //osLog(LOG_ERROR, "%s  mmc5603 reset\n",__func__);

    buf[0] = MMC5603NJ_REG_CTRL0;
    buf[1] = MMC5603NJ_MANU_TM;
    i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, buf, 2, NULL,NULL);
    vTaskDelay(10);

    buf[0] = MMC5603_REG_DATA;
    i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, buf, 1,  mTask.rxBuf, 6, NULL,NULL);

    curr_raw[0] = (int)( (uint16_t)((mTask.rxBuf[0] << 8) + (uint16_t)(mTask.rxBuf[1])) );
    curr_raw[1] = (int)( (uint16_t)((mTask.rxBuf[2] << 8) + (uint16_t)(mTask.rxBuf[3])) );
    curr_raw[2] = (int)( (uint16_t)((mTask.rxBuf[4] << 8) + (uint16_t)(mTask.rxBuf[5])) );
    //osLog(LOG_ERROR, "%s curr_raw: %d %d %d \n",__func__,curr_raw[0],curr_raw[1],curr_raw[2]);

    //osLog(LOG_ERROR, "%s diff: %d %d %d \n",__func__,abs(curr_raw[0]-prev_raw[0]),abs(curr_raw[1]-prev_raw[1]),abs(curr_raw[2]-prev_raw[2]));

    if ((abs(curr_raw[0]-prev_raw[0]) > 100) || (abs(curr_raw[1]-prev_raw[1]) > 100)||(abs(curr_raw[2]-prev_raw[2]) > 100))
    {
      temp_result=1;
      osLog(LOG_INFO, "%s : MEMSIC5603_selftest pass \n",__func__);

    }
    else
    {
        temp_result=-1;
       osLog(LOG_INFO, "%s : MEMSIC5603_selftest fail \n",__func__);
    }

     *testResult = (int32_t)temp_result;
}
#endif

static int mmc5603RegisterCore(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    struct sensorCoreInfo mInfo;
    memset(&mInfo, 0, sizeof(struct sensorCoreInfo));

    /* Register sensor Core */
    mInfo.sensType = SENS_TYPE_MAG;
    mInfo.getCalibration = magGetCalibration;
    mInfo.setCalibration = magSetCalibration;
    mInfo.getData = magGetData;
#ifdef VENDOR_EDIT
    // add for sensor self test
    mInfo.selfTest = mmc5603_selftest;
#endif//VENDOR_EDIT
    sensorCoreRegister(&mInfo);

    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int mmc5603Enable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "%s :  MMC5603 enable mag\n",__func__);
    iRestart = 1;

    mTask.txBuf[0] = MMC5603NJ_REG_CTRL1;
    mTask.txBuf[1] = MMC5603NJ_CMD_BW00;
    i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, NULL, NULL);
    //vTaskDelay(1);

    mTask.txBuf[2] = MMC5603NJ_REG_ODR;
    mTask.txBuf[3] = MMC5603NJ_ODR_50HZ;
    i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[2], 2, NULL, NULL);
    //vTaskDelay(1);

    mTask.txBuf[4] = MMC5603NJ_REG_CTRL0;
    mTask.txBuf[5] = MMC5603NJ_CMD_AUTO_SR_EN;
    i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[4], 2, NULL, NULL);
    //vTaskDelay(1);

    mTask.txBuf[6] = MMC5603NJ_REG_CTRL2;
    mTask.txBuf[7] = MMC5603NJ_CMD_CMM_EN;

    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[6], 2, i2cCallBack,
                       next_state);
}
static int mmc5603Disable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
//    osLog(LOG_INFO, "%s :  MMC5603 mmc5603Disable  mag\n",__func__);
    iRestart = 0;

    mTask.txBuf[0] = MMC5603NJ_REG_CTRL1;
    mTask.txBuf[1] = MMC5603NJ_CMD_BW00;
    i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, NULL,NULL);
    vTaskDelay(1);

    mTask.txBuf[2] = MMC5603NJ_REG_ODR;
    mTask.txBuf[3] = 0x00;
    i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[2], 2, NULL,NULL);
    vTaskDelay(1);


    mTask.txBuf[4] = MMC5603NJ_REG_CTRL0;
    mTask.txBuf[5] = 0x00;
    i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[4], 2, NULL,NULL);
    vTaskDelay(1);

    mTask.txBuf[6] = MMC5603NJ_REG_CTRL2;
    mTask.txBuf[7] = MMC5603NJ_CMD_CMM_DISEN;

    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[6], 2, i2cCallBack,
                       next_state);
}
static int mmc5603Rate(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    struct magCntlPacket cntlPacket;

    ret = rxControlInfo(&cntlPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "mmc5603Rate, rx inSize and elemSize error\n");
        return -1;
    }
    magTimeBufferReset(&mTask.magTimeBuf);    //reset time buffer

    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}


static int mmc5603GetStatus(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;

    ret = rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "%s mmc5603Sample, rx dataInfo error\n",__func__);
        return -1;
    }
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;

}
static int mmc5603Sample(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
 

    mTask.txBuf[0] = MMC5603_REG_DATA;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 6, i2cCallBack,
                         next_state);
}

static int mmc5603Convert(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    struct magData *data = mTask.magPacket.outBuf;
    float remap_data[AXES_NUM];
    int32_t idata[AXES_NUM];
    uint64_t timestamp = 0;
    uint8_t data_size = 0;

    //osLog(LOG_ERROR, "mmc3530Convert direction: %d\n", mTask.hw->direction);

#if 0
    osLog(LOG_ERROR, "%s mmc5603 raw reg data:[0]:%d, [1]:%d, [2]:%d, [3]:%d, [4]:%d, [5]:%d\n",
          __func__, mTask.rxBuf[0], mTask.rxBuf[1],mTask.rxBuf[2],mTask.rxBuf[3],mTask.rxBuf[4],mTask.rxBuf[5]);
#endif

    idata[AXIS_X] = (int)( (uint16_t)((mTask.rxBuf[0] << 8) + (uint16_t)(mTask.rxBuf[1])) );
    idata[AXIS_Y] = (int)( (uint16_t)((mTask.rxBuf[2] << 8) + (uint16_t)(mTask.rxBuf[3])) );
    idata[AXIS_Z] = (int)( (uint16_t)((mTask.rxBuf[4] << 8) + (uint16_t)(mTask.rxBuf[5])) );
    //osLog(LOG_ERROR, "%s mmc5603 raw_data: %d  %d %d\n " , __func__,idata[AXIS_X],idata[AXIS_Y], idata[AXIS_Z]);

    data[0].x = idata[AXIS_X];
    data[0].y = idata[AXIS_Y];
    data[0].z = idata[AXIS_Z];
    data[0].x -= 32768;
    data[0].y -= 32768;
    data[0].z -= 32768;

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
    /*osLog(LOG_ERROR, "mmc5603Convert raw data: %f, %f, %f, timestamp: %llu size: %u!\n", (double)remap_data[AXIS_X],
    (double)remap_data[AXIS_Y], (double)remap_data[AXIS_Z], timestamp, data_size); */

    txTransferDataInfo(&mTask.dataInfo, data_size, timestamp, data);

#if 0
    osLog(LOG_ERROR, "%s factoryData x:%d,y:%d,z:%d\n",
          __func__,mTask.factoryData.ix,mTask.factoryData.iy,mTask.factoryData.iz);
#endif
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int mmc5603SampleDone(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                          void *inBuf, uint8_t inSize, uint8_t elemInSize,
                          void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{

    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}
static struct sensorFsm mmc5603Fsm[] = {

    sensorFsmCmd(STATE_SAMPLE, STATE_CHIP_SAMPLE, mmc5603GetStatus),
    sensorFsmCmd(STATE_CHIP_SAMPLE, STATE_CONVERT, mmc5603Sample),
    sensorFsmCmd(STATE_CONVERT, STATE_CHIP_SAMPLE_OVER, mmc5603Convert),
    sensorFsmCmd(STATE_CHIP_SAMPLE_OVER, STATE_SAMPLE_DONE, mmc5603SampleDone),


    sensorFsmCmd(STATE_ENABLE, STATE_ENABLE_DONE, mmc5603Enable),
    sensorFsmCmd(STATE_DISABLE, STATE_DISABLE_DONE, mmc5603Disable),

    sensorFsmCmd(STATE_RATECHG, STATE_RATECHG_DONE, mmc5603Rate),


    sensorFsmCmd(STATE_FUSE_EN, STATE_INIT_DONE, mmc5603RegisterCore),
};

static int mmc5603CaliApiGetOffset(float offset[AXES_NUM])
{
    //osLog(LOG_ERROR, "%s\n",__func__);
    return 0;
}
static int mmc5603CaliApiSetOffset(float offset[AXES_NUM])
{
    //osLog(LOG_ERROR, "%s\n",__func__);
    return 0;
}
static int memsic5603CaliApiSetCaliParam(int32_t caliParameter[6])
{
#ifdef MEMSIC_DEBUG
    osLog(LOG_ERROR, "%s %ld, %ld,%ld,%ld  %ld\n\r", __func__,caliParameter[0],caliParameter[1],
          caliParameter[2],caliParameter[3],caliParameter[4]);
#endif

    float caliParameterInput[4]={0,0,0,50000};
    caliParameterInput[0]=(float)caliParameter[0]/1000;
    caliParameterInput[1]=(float)caliParameter[1]/1000;
    caliParameterInput[2]=(float)caliParameter[2]/1000;
    caliParameterInput[3]=(float)caliParameter[3]/1000;

    SetMagpara(caliParameterInput);

    return 0;
}
static int memsic5603CaliApiGetCaliParam(int32_t caliParameter[6])
{
    if (has_Gyro) {
        int Accuracy=0;
        static int LastAccuracy=0;
        float cal_para[6];

        Accuracy=GetAccuracy();
        if (LastAccuracy!=3 && Accuracy==3 ){
            GetCalibrationPara(cal_para);
            caliParameter[0]=(int32_t)(cal_para[0]*1000);
            caliParameter[1]=(int32_t)(cal_para[1]*1000);
            caliParameter[2]=(int32_t)(cal_para[2]*1000);
            caliParameter[3]=(int32_t)(cal_para[3]*1000);
            caliParameter[4]=(int32_t)(cal_para[4]*1000);
            caliParameter[5]=(int32_t)(cal_para[5]*1000);
            osLog(LOG_ERROR, "%s %ld, %ld,%ld,%ld,%ld\n\r",
                  __func__,caliParameter[0],caliParameter[1],caliParameter[2],caliParameter[3],caliParameter[4]);
        }
        LastAccuracy = Accuracy;
    } else {
        int Accuracy=0;
        static int LastAccuracy=0;
        float cal_para[7];

        Accuracy=GetMagAccuracy();
        if (LastAccuracy!=3 && Accuracy==3 ){
            GetCalPara_6(cal_para);
            caliParameter[0]=(int32_t)(cal_para[0]*1000);
            caliParameter[1]=(int32_t)(cal_para[1]*1000);
            caliParameter[2]=(int32_t)(cal_para[2]*1000);
            caliParameter[3]=(int32_t)(cal_para[3]*1000);
            caliParameter[4]=(int32_t)(cal_para[4]*1000);
            caliParameter[5]=(int32_t)(cal_para[5]*1000);
            osLog(LOG_ERROR, "%s %ld, %ld,%ld,%ld,%ld\n\r",
                  __func__,caliParameter[0],caliParameter[1],caliParameter[2],caliParameter[3],caliParameter[4]);
        }
            LastAccuracy = Accuracy;
    }
    return 0;
}
float acc_inv[3] = {0.0,0.0,9.8};
float gyro_inv[3] = {0.0,};
float mag_inv[3] = {0.0,};

static int mmc5603CaliApiSetGyroData(struct magCaliDataInPut *inputData)
{

#if 0
    osLog(LOG_ERROR, "%s x:%f, y:%f, z:%f, time:%lld\n\r",
          __func__,(double)inputData->x,(double)inputData->y,(double)inputData->z,inputData->timeStamp);
#endif

    gyro_inv[0] = inputData->x;
    gyro_inv[1] = inputData->y;
    gyro_inv[2] = inputData->z;

    return 1;

}
static uint64_t pre_timestamp=0;
static int mmc5603DoCaliAPI(struct magCaliDataInPut *inputData,
        struct magCaliDataOutPut *outputData)
{
    int err = 0;

    int64_t now_timestamp=0,delt_timestamp=0;

    float mag_offset[3]={0,};
    float calmag_para[6];

    float  ts=0.02f;
    float data_cali[3]={0,0,0};
    int8_t Accuracy = 0;
    float mag[3];
    //osLog(LOG_ERROR, "%s::enter\n",__func__);

    if (err < 0){
        osLog(LOG_ERROR, "mmc_set_mag_data fail\n\r");
        return err;
    }else{
        mag[0] = inputData->x;
        mag[1] = inputData->y;
        mag[2] = inputData->z;

       now_timestamp=inputData->timeStamp;
       delt_timestamp=now_timestamp-pre_timestamp;
       pre_timestamp=now_timestamp;
       ts=(float)delt_timestamp/(1000*1000*1000);//convert to seconds
       if(ts>0.02f)
         ts=0.02f;

#if 0
        osLog(LOG_ERROR, "%s: acc_inv:%f, %f, %f; mag:%f, %f, %f gyro_inv:%f %f %f ts: %f\n", __func__,
        (double)acc_inv[0], (double)acc_inv[1], (double)acc_inv[2],
        (double)mag[0], (double)mag[1], (double)mag[2],(double)gyro_inv[0],(double)gyro_inv[1],(double)gyro_inv[2],(double)ts);
#endif
        mag_inv[0] = mag[0]*100;
        mag_inv[1] = mag[1]*100;
        mag_inv[2] = mag[2]*100;

        if((myabs(acc_inv[0]) < 1e-6)&&(myabs(acc_inv[1]) < 1e-6)&&(myabs(acc_inv[2])< 1e-6))
        {
          acc_inv[0]=0.0f;
          acc_inv[1]=0.0f;
          acc_inv[2]=9.8f;
        }


    if (has_Gyro) {

        MainAlgoProcess(acc_inv, mag_inv, gyro_inv, ts, iRestart, 1);
        GetCalibratedMag(data_cali);
        NineAxisFusion(acc_inv,data_cali,gyro_inv,ts,iRestart);
        iRestart = 0;

        GetCalibrationPara(calmag_para);
        mag_offset[0]=(float)calmag_para[0];
        mag_offset[1]=(float)calmag_para[1];
        mag_offset[2]=(float)calmag_para[2];
        Accuracy = GetMagCalAccuracy();


        }else{
#if 1
        MainAlgorithmProcess(acc_inv,mag_inv);
        GetCalMag(data_cali);
        CalcMemsicGyro(data_cali, acc_inv, ts,iRestart);
        iRestart = 0;
        CalcMemsicRotVec(data_cali, acc_inv);
        Accuracy = GetMagAccuracy();
        GetOffset(mag_offset);

#endif
       }
      //osLog(LOG_ERROR, "%s  mmc3530 recv data_cali: %f, %f, %f, Accuracy: %d\n",__func__,
            //  (double)data_cali[0], (double)data_cali[1], (double)data_cali[2], Accuracy);

        outputData->x = data_cali[AXIS_X];
        outputData->y = data_cali[AXIS_Y];
        outputData->z = data_cali[AXIS_Z];

        outputData->x_bias = (float)mag_offset[AXIS_X];
        outputData->y_bias = (float)mag_offset[AXIS_Y];
        outputData->z_bias = (float)mag_offset[AXIS_Z];
        outputData->status = Accuracy;
#if MEMSIC_DEBUG
        osLog(LOG_ERROR, "%s  mmc5603 outputdata:%f, %f, %f\n",__func__,
               (double)outputData->x, (double)outputData->y, (double)outputData->z);
#endif
    }
    return 0;
}
static struct magCalibrationLibAPI mmc5603CaliAPI = {
    .caliApiGetOffset = mmc5603CaliApiGetOffset,
    .caliApiSetOffset = mmc5603CaliApiSetOffset,
    .caliApiGetCaliParam = memsic5603CaliApiGetCaliParam,
    .caliApiSetCaliParam = memsic5603CaliApiSetCaliParam,
    .caliApiSetGyroData = mmc5603CaliApiSetGyroData,
    .doCaliApi = mmc5603DoCaliAPI
};

#ifdef CFG_VENDOR_FUSION_SUPPORT
static int mmc5603FusionInitLib(int hwGyroSupport)
{
    has_Gyro=hwGyroSupport;
    //osLog(LOG_ERROR, "%s: hwgyrosupport:%d\n", __func__, hwGyroSupport);
    return MEMSIC_InitLib(hwGyroSupport);
}
static int mmc5603FusionSetGyro(struct InterfaceDataIn *inData)
{

    return 1;

}
static int mmc5603FusionSetAcc(struct InterfaceDataIn *inData)
{
    //osLog(LOG_ERROR, "%s: x:%f, y:%f, z:%f\n", __func__, (double)inData->vec[0], (double)inData->vec[1], (double)inData->vec[2]);
    acc_inv[0] = inData->vec[0];
    acc_inv[1] = inData->vec[1];
    acc_inv[2] = inData->vec[2];
    return MEMSIC_FusionSetAccData(inData->vec[0],
        inData->vec[1], inData->vec[2], inData->timeStamp);
}
static int mmc5603FusionSetMag(struct InterfaceDataIn *inData)
{

    //osLog(LOG_ERROR, "%s: x:%f, y:%f, z:%f\n", __func__, (double)inData->vec[0], (double)inData->vec[1], (double)inData->vec[2]);
    return MEMSIC_FusionSetMagData(inData->vec[0],
        inData->vec[1], inData->vec[2], inData->timeStamp);
}

static int mmc5603FusionGetGravity(struct InterfaceDataOut *outData)
{
    int ret = 0;
    float vec[3] = {0};
    int16_t accurancy = 0;

    ret = MEMSIC_GetGravity(vec, &accurancy);
    outData->vec[0] = vec[0];
    outData->vec[1] = vec[1];
    outData->vec[2] = vec[2];
    outData->status = accurancy;
    //osLog(LOG_ERROR, "%s: x:%f, y:%f, z:%f\n", __func__, (double)outData->vec[0], (double)outData->vec[1], (double)outData->vec[2]);
    return ret;
}

static int mmc5603FusionGetRotationVector(struct InterfaceDataOut *outData)
{

    int ret = 0;
    float vec[4] = {0};
    int16_t accurancy = 0;

    ret = MEMSIC_GetRotaionVector(vec, &accurancy);
    outData->vec[0] = vec[0];
    outData->vec[1] = vec[1];
    outData->vec[2] = vec[2];
    outData->vec[3] = vec[3];
    outData->status = accurancy;
    //osLog(LOG_ERROR, "%s: x:%f, y:%f, z:%f\n", __func__, (double)outData->vec[0], (double)outData->vec[1], (double)outData->vec[2]);
    return ret;
}

static int mmc5603FusionGetOrientation(struct InterfaceDataOut *outData)
{
    //int ret = 0;
    float vec[3] = {0};
    int16_t accurancy = 0;

    MEMSIC_GetOrientaion(vec, &accurancy);
    outData->vec[0] = vec[0];
    outData->vec[1] = vec[1];
    outData->vec[2] = vec[2];
    outData->status = accurancy;
   // osLog(LOG_ERROR, "%s: x:%f, y:%f, z:%f\n", __func__, (double)outData->vec[0], (double)outData->vec[1], (double)outData->vec[2]);
    return 1;
}

static int mmc5603FusionGetLinearaccel(struct InterfaceDataOut *outData)
{
    int ret = 0;
    float vec[3] = {0};
    int16_t accurancy = 0;

    ret = MEMSIC_GetLinearaccel(vec, &accurancy);
    outData->vec[0] = vec[0];
    outData->vec[1] = vec[1];
    outData->vec[2] = vec[2];
    outData->status = accurancy;
    //osLog(LOG_ERROR, "%s: x:%f, y:%f, z:%f\n", __func__, (double)outData->vec[0], (double)outData->vec[1], (double)outData->vec[2]);
    return ret;
}

static int mmc5603FusionGetGameRotationVector(struct InterfaceDataOut *outData)
{
    int ret = 0;
    float vec[4] = {0};
    int16_t accurancy = 0;

    ret = MEMSIC_GetGameRotaionVector(vec, &accurancy);
    outData->vec[0] = vec[0];
    outData->vec[1] = vec[1];
    outData->vec[2] = vec[2];
    outData->vec[3] = vec[3];
    outData->status = accurancy;
    //osLog(LOG_ERROR, "%s: x:%f, y:%f, z:%f\n", __func__, (double)outData->vec[0], (double)outData->vec[1], (double)outData->vec[2]);
    return ret;
}

static int mmc5603FusionGetGeoMagnetic(struct InterfaceDataOut *outData)
{
    int ret = 0;
    float vec[4] = {0};
    int16_t accurancy = 0;

    ret = MEMSIC_GetGeoMagnetic(vec, &accurancy);
    outData->vec[0] = vec[0];
    outData->vec[1] = vec[1];
    outData->vec[2] = vec[2];
    outData->vec[3] = vec[3];
    outData->status = accurancy;
    //osLog(LOG_ERROR, "%s: x:%f, y:%f, z:%f\n", __func__, (double)outData->vec[0], (double)outData->vec[1], (double)outData->vec[2]);
    return ret;
}

static int mmc5603FusionGetVirtualGyro(struct InterfaceDataOut *outData)
{
    int ret = 0;
    float vec[3] = {0};
    int16_t accurancy = 0;

    ret = MEMSIC_GetVirtualGyro(vec, &accurancy);
    outData->vec[0] = vec[0];
    outData->vec[1] = vec[1];
    outData->vec[2] = vec[2];
    outData->status = accurancy;
    //osLog(LOG_ERROR, "%s: x:%f, y:%f, z:%f\n", __func__, (double)outData->vec[0], (double)outData->vec[1], (double)outData->vec[2]);
    return ret;
}

static struct VendorFusionInterfact_t mmc5603Interface = {
    .name = "mmc5603",
    .initLib = mmc5603FusionInitLib,
    .setGyroData = mmc5603FusionSetGyro,
    .setAccData = mmc5603FusionSetAcc,
    .setMagData = mmc5603FusionSetMag,
    .getGravity = mmc5603FusionGetGravity,
    .getRotationVector = mmc5603FusionGetRotationVector,
    .getOrientation = mmc5603FusionGetOrientation,
    .getLinearaccel = mmc5603FusionGetLinearaccel,
    .getGameRotationVector = mmc5603FusionGetGameRotationVector,
    .getGeoMagnetic = mmc5603FusionGetGeoMagnetic,
    .getVirtualGyro = mmc5603FusionGetVirtualGyro
};
#endif

static int mmc5603Init(void)
{
    int ret = 0;

    mmc5603DebugPoint = &mTask;
    insertMagicNum(&mTask.magPacket);
    mTask.hw = get_cust_mag("mmc5603");
    if (NULL == mTask.hw) {
        osLog(LOG_ERROR, "mmc5603 get_cust_mag fail\n");
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
    mTask.txBuf[0] = MMC5603_REG_PRODUCT;
    ret = i2cMasterTxRxSync(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
        (uint8_t *)mTask.autoDetect, 1, NULL, NULL);
    if (ret < 0) {
        osLog(LOG_ERROR, "mmc5603 i2cMasterTxRxSync fail!!!\n");
        ret = -1;
        i2cMasterRelease(mTask.hw->i2c_num);
        goto err_out;
    }

    if ( mTask.autoDetect[0] == MMC5603_PRODUCT_ID_VALUE) {  // mmc5603 sensor id
        osLog(LOG_ERROR, "%s read id:0x%x suceess!!!\n", __func__, mTask.autoDetect[0]);
        osLog(LOG_INFO, "mmc5603: auto detect success\n");
#ifdef VENDOR_EDIT
        sensor_register_devinfo(SENS_TYPE_MAG, MAG_MMC5603);
#endif
        magSensorRegister();
        magRegisterInterruptMode(MAG_UNFIFO);
        registerMagDriverFsm(mmc5603Fsm, ARRAY_SIZE(mmc5603Fsm));
        registerMagCaliAPI(&mmc5603CaliAPI);
        registerMagTimerCbk(mmc5603TimerCbkF);
#ifdef CFG_VENDOR_FUSION_SUPPORT
        registerVendorInterface(&mmc5603Interface);
#endif
    } else {
        i2cMasterRelease(mTask.hw->i2c_num);
        osLog(LOG_ERROR, "mmc5603 read id fail!!!\n");
        ret = -1;
        goto err_out;
    }

err_out:
    return ret;
}
#ifndef CFG_OVERLAY_INIT_SUPPORT
MODULE_DECLARE(mmc5603, SENS_TYPE_MAG, mmc5603Init);
#else
#include "mtk_overlay_init.h"
OVERLAY_DECLARE(mmc5603, OVERLAY_ID_MAG, mmc5603Init);
#endif
