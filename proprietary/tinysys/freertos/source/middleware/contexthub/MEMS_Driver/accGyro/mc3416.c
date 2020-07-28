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
#include <math.h>

#include <accGyro.h>
#include <contexthub_core.h>
#include <cust_accGyro.h>
#include <mt_gpt.h>
#include <API_sensor_calibration.h>

#include "mc3416.h"

#define abs(x) 				( (x)>=0 ? (x) : -(x) )
#define I2C_SPEED      		400000
#define MAX_I2C_PER_PACKET  8
#define MAX_RXBUF 			512
#define MAX_TXBUF 			(MAX_RXBUF / MAX_I2C_PER_PACKET)
#define ACC_NAME     "mc3416_acc"



//#define MC34XX_DUMP_REGISTER
#if 0
#define MC_ALOGF()                 do{printf("[mcube@f@%04d]-@%s(): entry!\n", __LINE__, __FUNCTION__);}while(0)
#define MC_ALOGD(fmt, args...)     do{printf("[mcube@d@%04d]-@%s(): " fmt "\n", __LINE__, __FUNCTION__, ##args);}while(0)
#define MC_ALOGE(fmt, args...)     do{printf("[mcube@e@%04d]-@%s(): " fmt "\n", __LINE__, __FUNCTION__, ##args);}while(0)
// key log, keep with final code.
#define MC_ALOGK(fmt, args...)     do{printf("[mcube@k@%04d]-@%s(): " fmt "\n", __LINE__, __FUNCTION__, ##args);}while(0)
#else
#define MC_ALOGF()
#define MC_ALOGD(fmt, args...)
#define MC_ALOGE(fmt, args...)     do{printf("[mcube@e@%04d]-@%s(): " fmt "\n", __LINE__, __FUNCTION__, ##args);}while(0)
// key log, keep with final code.
#define MC_ALOGK(fmt, args...)     do{printf("[mcube@k@%04d]-@%s(): " fmt "\n", __LINE__, __FUNCTION__, ##args);}while(0)
#endif



enum MC34XXState {
    STATE_SAMPLE = CHIP_SAMPLING,
    STATE_CONVERT = CHIP_CONVERT,
    STATE_SAMPLE_DONE = CHIP_SAMPLING_DONE,
    STATE_ACC_ENABLE = CHIP_ACC_ENABLE,
    STATE_ACC_ENABLE_DONE = CHIP_ACC_ENABLE_DONE,
    STATE_ACC_DISABLE = CHIP_ACC_DISABLE,
    STATE_ACC_DISABLE_DONE = CHIP_ACC_DISABLE_DONE,
    STATE_ACC_RATECHG = CHIP_ACC_RATECHG,
    STATE_ACC_RATECHG_DONE = CHIP_ACC_RATECHG_DONE,
    STATE_ACC_CALI = CHIP_ACC_CALI,
    STATE_ACC_CALI_DONE = CHIP_ACC_CALI_DONE,
    STATE_ACC_CFG = CHIP_ACC_CFG,
    STATE_ACC_CFG_DONE = CHIP_ACC_CFG_DONE,
    STATE_INIT_DONE = CHIP_INIT_DONE,
    STATE_IDLE = CHIP_IDLE,
    STATE_SW_RESET = CHIP_RESET,
    MC34XX_DEVICE_ID,
    MC34XX_STANDBY_MODE,
    MC34XX_RESOLUTION,
    MC34XX_SAMPLERATE,
    MC34XX_LPF_RANGE,
    MC34XX_GAIN,
    MC34XX_WAKE_MODE,
};
/*
enum SensorIndex {
    ACC = 0,
    ANYMO,
    NUM_OF_SENSOR,
};
*/

static struct MC34XXTask {
    bool accPowerOn;
    /* txBuf for i2c operation, fill register and fill value */
    uint8_t txBuf[MAX_TXBUF];
    /* rxBuf for i2c operation, receive rawdata */
    uint8_t rxBuf[MAX_RXBUF];
    uint8_t HWId;
    uint8_t waterMark;
    uint8_t value;
    uint32_t accRate;
    uint16_t accRateDiv;

    uint16_t sensitivity;
    uint8_t resolution;
    uint8_t pCode;

    uint64_t sampleTime;
    struct transferDataInfo dataInfo;
    struct accGyroDataPacket accGyroPacket;
    /* data for factory */
    struct TripleAxisDataPoint accFactoryData;

    int16_t cross;
    int32_t accSwCali[AXES_NUM];
    struct accGyro_hw *hw;
    struct sensorDriverConvert cvt;
    uint8_t i2c_addr;
   int32_t debug_trace;

    bool startCali;
    float staticCali[AXES_NUM];
    int32_t accuracy;
} mTask;
static struct MC34XXTask *mc34xxDebugPoint;


static int mc34xxDeviceId(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
	mTask.HWId = mTask.HWId & 0xF0;
	osLog(LOG_INFO, "mc34xx:get pcode=%02X,HWId=%02X\n",mTask.pCode,mTask.HWId);

    mTask.txBuf[0] = MC34XX_REG_PRODUCT_CODE_L ;
    ret = i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         &mTask.rxBuf,1, i2cCallBack, next_state);
    return ret;
}

static int mc34xxSetResolution(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    int ret = 0;
    //osLog(LOG_INFO, "0x3b=%02X,mc34xx: set resolution\n",mTask.rxBuf[0]);

    mTask.pCode = mTask.rxBuf[0] & 0xF1;
	switch (mTask.pCode)
    {
    case MC34XX_PCODE_3433:
    case MC34XX_PCODE_3436:
         mTask.resolution = MC34XX_RESOLUTION_LOW;
         break;
    case MC34XX_PCODE_3413:
    case MC34XX_PCODE_3416:
         mTask.resolution = MC34XX_RESOLUTION_HIGH;
         break;
    default:
         osLog(LOG_INFO,"ERR: no resolution assigned!\n");
         break;
    }
    mTask.txBuf[0] = MC34XX_REG_LPF_RANGE_RES ;
    ret = i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         &mTask.rxBuf, 1, i2cCallBack, next_state);
	return ret;
}

static int mc34xxStandByMode(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    osLog(LOG_INFO, "0x20=%02X,set standby mode\n",mTask.rxBuf[0]);

    mTask.txBuf[0] = MC34XX_REG_MODE_FEATURE;
    mTask.txBuf[1] = 0xC3;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}

static int mc34xxWakeMode(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    osLog(LOG_INFO, "set standby mode \n");

    mTask.txBuf[0] = MC34XX_REG_MODE_FEATURE;
    mTask.txBuf[1] = 0xC1;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}

static int mc34xxSetSampleRate(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    //osLog(LOG_INFO, "set sample rate \n");
    if(0xA0 == mTask.HWId){//mensa
        mTask.txBuf[0] = MC34XX_REG_SAMPLE_RATE;
        mTask.txBuf[1] = 0x15;//OSR=64,CLK=640,RATE=5,ODR=1024Hz
    }else{//merak
        mTask.txBuf[0] = MC34XX_REG_SAMPLE_RATE;
        mTask.txBuf[1] = 0x09;//OSR=32,CLK=640,RATE=5,ODR=128Hz
    }
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}

static int mc34xxLPFRange(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    //osLog(LOG_INFO, "set lpf range \n");

    mTask.txBuf[0] = MC34XX_REG_LPF_RANGE_RES;
    if(0xA0 == mTask.HWId){//mensa
		if (MC34XX_RESOLUTION_LOW ==  mTask.resolution)
			mTask.txBuf[1] = 0x09 | 0x20;//128Hz@1024Hz ODR,8bit,(+2g~-2g)
		else
			mTask.txBuf[1] = 0x09 | 0x20;//128Hz@1024Hz ODR,16bit,(+8g~-8g)
    }else{//merak
		if (MC34XX_RESOLUTION_LOW ==  mTask.resolution)
			mTask.txBuf[1] = 0x02;//128Hz@1024Hz ODR,8bit,(+2g~-2g)
		else
			mTask.txBuf[1] = 0x25;//128Hz@1024Hz ODR,14bit,(+8g~-8g)

    }
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}

static int mc34xxSetGain(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    //osLog(LOG_INFO, "set gain \n");
	if(0xA0 == mTask.HWId){//mensa
		if (MC34XX_RESOLUTION_LOW ==  mTask.resolution)
			mTask.sensitivity = (1 << 8)/(1 << 2);//8bit,(+2g~-2g)-->64
		else
			mTask.sensitivity = (1 << 16)/(1 << 4);//16bit,(+8g~-8g)-->4096
	}else{
		if (MC34XX_RESOLUTION_LOW ==  mTask.resolution)
			mTask.sensitivity = (1 << 8)/(1 << 2);//8bit,(+2g~-2g)-->64
		else
			mTask.sensitivity = (1 << 14)/(1 << 4);//14bit,(+8g~-8g)-->1024
	}
	mTask.txBuf[0] = MC34XX_REG_LPF_RANGE_RES ;
	return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         &mTask.rxBuf, 1, i2cCallBack, next_state);
}

static int mc34xxSample(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    //osLog(LOG_INFO, "mc34xxSample\n");

    ret = rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "mc34xxSample, rx dataInfo error\n");
        return -1;
    }

    mTask.sampleTime = rtcGetTime();
	/*get x,y,z raw data*/
    mTask.txBuf[0] = MC34XX_REG_XOUT_EX_L;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 6, i2cCallBack,
                         next_state);
}

static int mc34xxParseRawData(uint8_t *databuf, struct accGyroData *data)
{
    int32_t raw_data[AXES_NUM];
    int32_t remap_data[AXES_NUM];
    int32_t caliResult[AXES_NUM] = {0};
    float temp_data[AXES_NUM] = {0};
    float calibrated_data_output[AXES_NUM] = {0};
    int32_t delta_time = 0;
    int16_t status = 0;

    raw_data[AXIS_X] = (int16_t)(databuf[0] | (databuf[1] << 8));
    raw_data[AXIS_Y] = (int16_t)(databuf[2] | (databuf[3] << 8));
    raw_data[AXIS_Z] = (int16_t)(databuf[4] | (databuf[5] << 8));

    /*add calibrated data*/
    raw_data[AXIS_X] = raw_data[AXIS_X] + mTask.accSwCali[AXIS_X];
    raw_data[AXIS_Y] = raw_data[AXIS_Y] + mTask.accSwCali[AXIS_Y];
    raw_data[AXIS_Z] = raw_data[AXIS_Z] + mTask.accSwCali[AXIS_Z];

    /*remap coordinate*/
    remap_data[mTask.cvt.map[AXIS_X]] = mTask.cvt.sign[AXIS_X] * raw_data[AXIS_X];
    remap_data[mTask.cvt.map[AXIS_Y]] = mTask.cvt.sign[AXIS_Y] * raw_data[AXIS_Y];
    remap_data[mTask.cvt.map[AXIS_Z]] = mTask.cvt.sign[AXIS_Z] * raw_data[AXIS_Z];

    temp_data[AXIS_X] = (float)remap_data[AXIS_X] * GRAVITY_EARTH_SCALAR / mTask.sensitivity;
    temp_data[AXIS_Y] = (float)remap_data[AXIS_Y] * GRAVITY_EARTH_SCALAR / mTask.sensitivity;
    temp_data[AXIS_Z] = (float)remap_data[AXIS_Z] * GRAVITY_EARTH_SCALAR / mTask.sensitivity;

    if (UNLIKELY(mTask.startCali)) {
        status = Acc_run_factory_calibration_timeout(delta_time, temp_data, calibrated_data_output, (int *)&mTask.accuracy, rtcGetTime());
        if (status != 0) {
	    mTask.startCali = false;
            if (status > 0) {
                osLog(LOG_INFO, "ACC cali detect shake\n");
                caliResult[AXIS_X] = (int32_t)(mTask.staticCali[AXIS_X] * 1000);
                caliResult[AXIS_Y] = (int32_t)(mTask.staticCali[AXIS_Y] * 1000);
                caliResult[AXIS_Z] = (int32_t)(mTask.staticCali[AXIS_Z] * 1000);
                accGyroSendCalibrationResult(SENS_TYPE_ACCEL, (int32_t *)&caliResult[0], (uint8_t)status);
            } else
                osLog(LOG_INFO, "ACC cali time out\n");
         }
         else if (mTask.accuracy == 3) {
             mTask.startCali = false;
             mTask.staticCali[AXIS_X] = calibrated_data_output[AXIS_X] - temp_data[AXIS_X];
             mTask.staticCali[AXIS_Y] = calibrated_data_output[AXIS_Y] - temp_data[AXIS_Y];
             mTask.staticCali[AXIS_Z] = calibrated_data_output[AXIS_Z] - temp_data[AXIS_Z];
             caliResult[AXIS_X] = (int32_t)(mTask.staticCali[AXIS_X] * 1000);
             caliResult[AXIS_Y] = (int32_t)(mTask.staticCali[AXIS_Y] * 1000);
             caliResult[AXIS_Z] = (int32_t)(mTask.staticCali[AXIS_Z] * 1000);
             accGyroSendCalibrationResult(SENS_TYPE_ACCEL, (int32_t *)&caliResult[0], (uint8_t)status);
             osLog(LOG_INFO, "ACC cali done:caliResult[0]:%d, caliResult[1]:%d, caliResult[2]:%d, offset[0]:%f, offset[1]:%f, offset[2]:%f\n",
				    caliResult[AXIS_X], caliResult[AXIS_Y], caliResult[AXIS_Z],
				    (double)mTask.staticCali[AXIS_X],
				    (double)mTask.staticCali[AXIS_Y],
				    (double)mTask.staticCali[AXIS_Z]);
          }
    }

    /* Out put the degree/second(mo/s) */
    data->sensType = SENS_TYPE_ACCEL;

    data->x = temp_data[AXIS_X] + mTask.staticCali[AXIS_X];
    data->y = temp_data[AXIS_Y] + mTask.staticCali[AXIS_Y];
    data->z = temp_data[AXIS_Z] + mTask.staticCali[AXIS_Z];


    mTask.accFactoryData.ix = (int32_t)(data->x * ACCELEROMETER_INCREASE_NUM_AP);
    mTask.accFactoryData.iy = (int32_t)(data->y * ACCELEROMETER_INCREASE_NUM_AP);
    mTask.accFactoryData.iz = (int32_t)(data->z * ACCELEROMETER_INCREASE_NUM_AP);
   // osLog(LOG_INFO, "acc data convert after  x:%f, y:%f, z:%f\n", (double)data->x, (double)data->y, (double)data->z);

    return 0;
}

static int mc34xxConvert(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    struct accGyroData *data = mTask.accGyroPacket.outBuf;
    uint8_t accEventSize = 0, gyroEventSize = 0;
    uint8_t *databuf;

    databuf = &mTask.rxBuf[0];
    accEventSize++;
    mc34xxParseRawData(databuf, &data[0]);

    /*if startcali true , can't send to runtime cali in parseRawData to accGyro*/
    if (mTask.startCali) {
        accEventSize = 0;
    }

    txTransferDataInfo(&mTask.dataInfo, accEventSize, gyroEventSize, mTask.sampleTime, data,0);

    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}


static int mc34xxAccPowerOn(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "set mc34xx wake mode \n");

    mTask.accPowerOn = true;
    mTask.txBuf[0] = MC34XX_REG_MODE_FEATURE;
    mTask.txBuf[1] = 0xC1;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);

}

static int mc34xxAccPowerOff(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    struct accGyroCntlPacket cntlPacket;

	//osLog(LOG_INFO, "set mc34xx standby mode \n");

    ret = rxControlInfo(&cntlPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_INFO, "mc34xxDisable, rx water_mark err\n");
        return -1;
    }

    mTask.accPowerOn = false;
    mTask.accRate = 0;
    mTask.txBuf[0] = MC34XX_REG_MODE_FEATURE;
    mTask.txBuf[1] = 0xC3;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}

static int mc34xxRate(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    uint32_t sample_rate, water_mark;
    struct accGyroCntlPacket cntlPacket;

    ret = rxControlInfo(&cntlPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "mc34xxRate, rx inSize and elemSize error\n");
        return -1;
    }
    sample_rate = cntlPacket.rate;
    water_mark = cntlPacket.waterMark;

    if (sample_rate > 0 && sample_rate < 6000) {
        mTask.accRate = 5000;
        mTask.accRateDiv = 224;
    } else if (sample_rate > 9000 && sample_rate < 11000) {
        mTask.accRate = 10000;
        mTask.accRateDiv = 111;
    } else if (sample_rate > 16000 && sample_rate < 17000) {
        mTask.accRate = 15000;
        mTask.accRateDiv = 74;
    } else if (sample_rate > 40000 && sample_rate < 60000) {
        mTask.accRate = 51100;
        mTask.accRateDiv = 21;
    } else if (sample_rate > 90000 && sample_rate < 110000) {
        mTask.accRate = 102300;
        mTask.accRateDiv = 10;
    } else if (sample_rate > 190000 && sample_rate < 210000) {
        mTask.accRate = 225000;
        mTask.accRateDiv = 4;
    } else {
        mTask.accRate = sample_rate;
        mTask.accRateDiv = 1125000 / sample_rate - 1;
    }

    //TODO: write rate to sensor

    mTask.waterMark =1;// water_mark;
    osLog(LOG_INFO, "mc34xxRate: %d, minAccGyroDelay:%d, water_mark:%d\n", mTask.accRate, water_mark);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return ret;
}

static int mc34xxAccCali(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                            void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    float bias[AXES_NUM] = {0};
    mTask.startCali = true;

    //osLog(LOG_INFO, "mc34xxAccCali\n");
    Acc_init_calibration(bias);

    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int mc34xxAccCfgCali(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                            void *inBuf, uint8_t inSize, uint8_t elemInSize,
                            void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    struct accGyroCaliCfgPacket caliCfgPacket;

    ret = rxCaliCfgInfo(&caliCfgPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);

    if (ret < 0) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "mc34xxAccCfgCali, rx inSize and elemSize error\n");
        return -1;
    }
    osLog(LOG_INFO, "mc34xxAccCfgCali: cfgData[0]:%d, cfgData[1]:%d, cfgData[2]:%d\n",
        caliCfgPacket.caliCfgData[0], caliCfgPacket.caliCfgData[1], caliCfgPacket.caliCfgData[2]);

    mTask.staticCali[0] = (float)caliCfgPacket.caliCfgData[0] / 1000;
    mTask.staticCali[1] = (float)caliCfgPacket.caliCfgData[1] / 1000;
    mTask.staticCali[2] = (float)caliCfgPacket.caliCfgData[2] / 1000;

    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static void accGetCalibration(int32_t *cali, int32_t size)
{
    cali[AXIS_X] = mTask.accSwCali[AXIS_X];
    cali[AXIS_Y] = mTask.accSwCali[AXIS_Y];
    cali[AXIS_Z] = mTask.accSwCali[AXIS_Z];
    osLog(LOG_INFO, "accGetCalibration cali x:%d, y:%d, z:%d\n", cali[AXIS_X], cali[AXIS_Y], cali[AXIS_Z]);
}
static void accSetCalibration(int32_t *cali, int32_t size)
{
    mTask.accSwCali[AXIS_X] = cali[AXIS_X];
    mTask.accSwCali[AXIS_Y] = cali[AXIS_Y];
    mTask.accSwCali[AXIS_Z] = cali[AXIS_Z];
    osLog(LOG_INFO, "accSetCalibration cali x:%d, y:%d, z:%d\n", mTask.accSwCali[AXIS_X],
        mTask.accSwCali[AXIS_Y], mTask.accSwCali[AXIS_Z]);
}
static void accGetData(void *sample)
{
    struct TripleAxisDataPoint *tripleSample = (struct TripleAxisDataPoint *)sample;
    tripleSample->ix = mTask.accFactoryData.ix;
    tripleSample->iy = mTask.accFactoryData.iy;
    tripleSample->iz = mTask.accFactoryData.iz;
}

static void mc34xxSetDebugTrace(int32_t trace) {
    mTask.debug_trace = trace;
    //MC_ALOGD("%s ==> trace:%d\n", __func__, mTask.debug_trace);
}

static void mc34xxGetSensorInfo(struct sensorInfo_t *data)
{
    strncpy(data->name, "ACC_NAME", sizeof(data->name));
}



static int mc34xxRegisterCore(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    struct sensorCoreInfo mInfo;

    if ((mTask.HWId == 0xA0)||(mTask.HWId == 0xC0)||(mTask.HWId == 0x40) ||(mTask.HWId == 0x20)) {
        osLog(LOG_ERROR, "match mc3xx,mc34xxRegisterCore HWId 0x%x\n", mTask.pCode);
    }else{
        osLog(LOG_ERROR, "don't match mc3xx,mc34xxRegisterCore HWId 0x%x\n", mTask.pCode);
        return -1;
    }

    /* Register sensor Core */
    mInfo.sensType = SENS_TYPE_ACCEL;
    mInfo.gain = GRAVITY_EARTH_1000;
    mInfo.sensitivity = mTask.sensitivity;
    mInfo.cvt = mTask.cvt;
    mInfo.getCalibration = accGetCalibration;
    mInfo.setCalibration = accSetCalibration;
    mInfo.getData = accGetData;
    mInfo.setDebugTrace = mc34xxSetDebugTrace;
    mInfo.getSensorInfo = mc34xxGetSensorInfo;

    sensorCoreRegister(&mInfo);

    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static struct sensorFsm mc34xxFsm[] = {

	/* init state */
    sensorFsmCmd(STATE_SW_RESET, MC34XX_DEVICE_ID, mc34xxDeviceId),
    sensorFsmCmd(MC34XX_DEVICE_ID,MC34XX_STANDBY_MODE, mc34xxStandByMode),
    sensorFsmCmd(MC34XX_STANDBY_MODE, MC34XX_RESOLUTION, mc34xxSetResolution),
    sensorFsmCmd(MC34XX_RESOLUTION, MC34XX_SAMPLERATE, mc34xxSetSampleRate),
    sensorFsmCmd(MC34XX_SAMPLERATE, MC34XX_LPF_RANGE, mc34xxLPFRange),
    sensorFsmCmd(MC34XX_LPF_RANGE, MC34XX_GAIN, mc34xxSetGain),
    sensorFsmCmd(MC34XX_GAIN, MC34XX_WAKE_MODE, mc34xxWakeMode),
    sensorFsmCmd(MC34XX_WAKE_MODE, STATE_INIT_DONE, mc34xxRegisterCore),
    /* acc report data */
    sensorFsmCmd(STATE_SAMPLE, STATE_CONVERT, mc34xxSample),
    sensorFsmCmd(STATE_CONVERT, STATE_SAMPLE_DONE, mc34xxConvert),
    /* acc enable state */
    sensorFsmCmd(STATE_ACC_ENABLE, STATE_ACC_ENABLE_DONE, mc34xxAccPowerOn),
    sensorFsmCmd(STATE_ACC_DISABLE, STATE_ACC_DISABLE_DONE, mc34xxAccPowerOff),
    /* acc rate state */
    sensorFsmCmd(STATE_ACC_RATECHG,  STATE_ACC_RATECHG_DONE, mc34xxRate),
    /* acc calibration */
    sensorFsmCmd(STATE_ACC_CALI, STATE_ACC_CALI_DONE, mc34xxAccCali),
    sensorFsmCmd(STATE_ACC_CFG, STATE_ACC_CFG_DONE, mc34xxAccCfgCali),
    /* For Anymotion */
	//sensorFsmCmd(STATE_ANYMO_ENABLE, STATE_ANYMO_ENABLE_DONE, mc34xxAmdPowerOn),
    //sensorFsmCmd(STATE_ANYMO_DISABLE, STATE_ANYMO_DISABLE_DONE, mc34xxAmdPowerOff),
};
void mc34xxTimerCbkF(void)
{
    //osLog(LOG_INFO, "mc34xxTimerCbkF\n");
}
static void i2cAutoDetect(void *cookie, size_t tx, size_t rx, int err)
{
    if (err == 0) {
        osLog(LOG_INFO, "mc34xx: auto detect success:0x%x\n", mTask.HWId);
        accSensorRegister();
        registerAccGyroInterruptMode(ACC_GYRO_FIFO_UNINTERRUPTIBLE);
        registerAccGyroDriverFsm(mc34xxFsm, ARRAY_SIZE(mc34xxFsm));
        registerAccGyroTimerCbk(mc34xxTimerCbkF);
    } else{
		osLog(LOG_ERROR, "mc34xx: auto detect error (%d)\n", err);
	}
}
int mc34xxInit(void)
{
    int ret = 0;

    osLog(LOG_INFO, "mc34xx: mc34xxInit\n");

    mc34xxDebugPoint = &mTask;
    insertMagicNum(&mTask.accGyroPacket);
    mTask.hw = get_cust_accGyro("mc34xx");
    if (NULL == mTask.hw) {
        osLog(LOG_ERROR, "get_cust_acc_hw fail\n");
        return 0;
    }

    mTask.i2c_addr = mTask.hw->i2c_addr[0];
    osLog(LOG_ERROR, "acc i2c_num: %d, i2c_addr: 0x%x\n", mTask.hw->i2c_num, mTask.i2c_addr);

    if (0 != (ret = sensorDriverGetConvert(mTask.hw->direction, &mTask.cvt))) {
        osLog(LOG_ERROR, "invalid direction: %d\n", mTask.hw->direction);
    }
    osLog(LOG_ERROR, "acc map[0]:%d, map[1]:%d, map[2]:%d, sign[0]:%d, sign[1]:%d, sign[2]:%d\n",
        mTask.cvt.map[AXIS_X], mTask.cvt.map[AXIS_Y], mTask.cvt.map[AXIS_Z],
        mTask.cvt.sign[AXIS_X], mTask.cvt.sign[AXIS_Y], mTask.cvt.sign[AXIS_Z]);
	/*
	mTask.latch_time_id = alloc_latch_time();
    enable_latch_time(mTask.latch_time_id, mTask.hw->eint_num);
	for (i = ACC; i < NUM_OF_SENSOR; i++) {
        initSensorStruct(&mTask.sensors[i], i);
    }
	*/
    i2cMasterRequest(mTask.hw->i2c_num, I2C_SPEED);
	mdelay(10);
    mTask.txBuf[0] = MC34XX_REG_CHIPID ;
    ret = i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         &mTask.HWId, 1, i2cAutoDetect, NULL);
    if(ret!= 0){//auto dectect i2c address
        mTask.i2c_addr = 0x6C;
        ret = i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         &mTask.HWId, 1, i2cAutoDetect, NULL);
    }
    return ret;

}


#ifndef CFG_OVERLAY_INIT_SUPPORT
MODULE_DECLARE(mc34xx, SENS_TYPE_ACCEL, mc34xxInit);
#else
#include "mtk_overlay_init.h"
OVERLAY_DECLARE(mc34xx, OVERLAY_WORK_00, mc34xxInit);
#endif
