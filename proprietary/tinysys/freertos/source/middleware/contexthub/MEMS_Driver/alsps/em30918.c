/*
 * Copyright (C) 2017 The Android Open Source Project
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
#include "em30918.h"
#include "alsps.h"

enum EM30918State {
    STATE_SAMPLE_ALS = CHIP_SAMPLING_ALS,
    STATE_SAMPLE_ALS_DONE = CHIP_SAMPLING_ALS_DONE,
    STATE_SAMPLE_PS = CHIP_SAMPLING_PS,
    STATE_SAMPLE_PS_ONCE = CHIP_SAMPLING_PS_ONCE,
    STATE_SAMPLE_PS_DONE = CHIP_SAMPLING_PS_DONE,
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
    STATE_PS_ENABLE = CHIP_PS_ENABLE,
    STATE_PS_ENABLE_DONE = CHIP_PS_ENABLE_DONE,
    STATE_PS_DISABLE = CHIP_PS_DISABLE,
    STATE_PS_DISABLE_DONE = CHIP_PS_DISABLE_DONE,
    STATE_PS_RATECHG = CHIP_PS_RATECHG,
    STATE_PS_RATECHG_DONE = CHIP_PS_RATECHG_DONE,
    STATE_PS_CALI = CHIP_PS_CALI,
    STATE_PS_CALI_DONE = CHIP_PS_CALI_DONE,
    STATE_PS_CFG = CHIP_PS_CFG,
    STATE_PS_CFG_DONE = CHIP_PS_CFG_DONE,
    STATE_INIT_DONE = CHIP_INIT_DONE,
    STATE_IDEL = CHIP_IDLE,
    STATE_RESET = CHIP_RESET,
    /* Alsps sample */
    STATE_GET_ALS_DATA,
    STATE_ALS_SET_DEBOUNCE,
    STATE_GET_PS_FLG,
    STATE_GET_PS_RAW_DATA,
    STATE_GET_PS_RAW_DATA_ONCE,
    STATE_DUMP_REG,
    STATE_PS_SET_DEBOUNCE,
    /* Power on & off */
    STATE_GET_ALSPS_STATE,
    STATE_ALS_POWER_ON,
    STATE_ALS_POWER_OFF,
    STATE_PS_POWER_ON,
    STATE_PS_POWER_OFF,
    STATE_PS_UNMASK_EINT,
    STATE_CLR_INT,
    STATE_EN_EINT,
    /* Init state */
    STATE_SET_SW_RST,
    STATE_SET_ALSPS_CTRL,
    STATE_SET_PS_THDH,
    STATE_SET_PS_THDL,
    STATE_SETUP_EINT,
    STATE_CORE
};

#define I2C_SPEED                       400000
#define MAX_I2C_PER_PACKET          8
#define MAX_RXBUF                   512
#define MAX_TXBUF (MAX_RXBUF / MAX_I2C_PER_PACKET)


static struct EM30918Task {
    /* txBuf for i2c operation, fill register and fill value */
    bool alsPowerOn;
    bool psPowerOn;
    unsigned int    als_debounce_time;
    unsigned int    als_debounce_on;    /*indicates if the debounce is on*/
    portTickType    als_debounce_end;
    unsigned int    ps_debounce_time;
    unsigned int    ps_debounce_on;     /*indicates if the debounce is on*/
    portTickType    ps_debounce_end;
    unsigned int    ps_suspend;
    uint8_t txBuf[MAX_TXBUF];
    /* rxBuf for i2c operation, receive rawdata */
    uint8_t rxBuf[MAX_RXBUF];
    uint8_t deviceId;
    /* data for factory */
    struct alsps_hw *hw;
    uint8_t i2c_addr;
    struct transferDataInfo dataInfo;
    struct AlsPsData data;
   uint32_t debug_trace;

    int32_t psCali[3];
    double alsCali;
    uint32_t    als_raw_data;
    uint32_t    prox_raw_data;
    uint32_t    ps_threshold_high;
    uint32_t    ps_threshold_low;
    uint32_t    als_threshold_high;
    uint32_t    als_threshold_low;
    uint8_t     ledctrl_val;
    uint8_t     state_val;
	uint8_t     als_ctrl_val;
	uint8_t     ps_ctrl_val;
	uint8_t		irs_running;
	bool		als_drup_data;

	uint16_t	last_ir_code;
	uint16_t	last_lux_val;
	uint16_t	ps_raw_data;
	uint32_t	psoff_data;
	uint32_t	ps_min;
	uint32_t	ps_max;
	uint8_t		flag_reg;
	uint32_t	ct_tracking_index;
	bool		ps_thd_update;
	uint16_t    psi_set;
	uint8_t  intr_flag_value ;
	uint8_t  intr_flag_state ;
} mTask;

struct alspsCaliCfgPacket pscaliCfgPacket;

static struct EM30918Task *EM30918DebugPoint;
static int getLuxFromAlsData(void);

static int check_timeout(portTickType *end_tick) {
    return xTaskGetTickCount() > *end_tick ? 1 : 0;
}

static void cal_end_time(unsigned int ms, portTickType *end_tick) {
    *end_tick = xTaskGetTickCount() +
                (portTickType)(ms / (1000 / portTICK_RATE_MS));
}

static int EM30918_read_als_data(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {

     osLog(LOG_INFO, "EM30918_read_als_data\n");
    if (rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize)) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "als rx error\n");
        return -1;
    }
	osLog(LOG_INFO, "EM30918_read_als_data\n");
	mTask.txBuf[0] = EM_DATA1_ALS_REG;
	#if 1
	    i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,
                         next_state);
	 mTask.txBuf[0] = EM_DATA2_ALS_REG;
	return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         &(mTask.rxBuf[1]), 1, i2cCallBack,
                         next_state);
	#else
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 2, i2cCallBack,
                         next_state);
	#endif
}

static int EM30918_get_als_value(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
	int als_data = 0;

	osLog(LOG_INFO, "EM30918_get_als_value\n");

	mTask.als_raw_data = ((mTask.rxBuf[1] << 8) | mTask.rxBuf[0]);
	als_data = getLuxFromAlsData();
	if (als_data < 0){
        mTask.data.sensType = SENS_TYPE_INVALID; /*invalid lux value when debounce is on*/
    } else {
		mTask.data.als_data = als_data;
		mTask.data.sensType = SENS_TYPE_ALS;
	}
    osLog(LOG_INFO,"EM30918: EM30918_get_als_value(als_raw_data:%d,als_data:%d,mTask.alsCali:%.5f)\n", mTask.als_raw_data,als_data,mTask.alsCali);

    txTransferDataInfo(&mTask.dataInfo, 1, &mTask.data);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);

#if 0
	//osLog(LOG_INFO,"EM30918: EM30918_get_als_value(als_raw_data:%d)\n", mTask.data.als_raw_data);
	mTask.data.als_data = (uint16_t)EM30918_als_data_correct_factor(mTask.data.als_raw_data);
    if (mTask.alsCali != 0)
		mTask.data.als_data *= mTask.alsCali;
	mTask.last_lux_val = mTask.data.als_data;
	osLog(LOG_WARN,"EM30918: EM30918_get_als_value(als_ctrl_val:%d,als_raw_data:%u,als_data:%u,mTask.alsCali:%.5f)\n", mTask.als_ctrl_val,mTask.data.als_raw_data,mTask.data.als_data,mTask.alsCali);
	txTransferDataInfo(&mTask.dataInfo, 1, &mTask.data);
	sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
#endif
    return 0;
}

static int getLuxFromAlsData(void) {
    int idx;
    int invalid = 0;
    int als_level_num, als_value_num;
    int value = 0;
    als_level_num = sizeof(mTask.hw->als_level) / sizeof(mTask.hw->als_level[0]);
    als_value_num = sizeof(mTask.hw->als_value) / sizeof(mTask.hw->als_value[0]);

    osLog(LOG_INFO, "getLuxFromAlsData alsdata :%d \n",mTask.als_raw_data );

    for (idx = 0; idx < als_level_num; idx++) {
        if (mTask.als_raw_data < mTask.hw->als_level[idx]) {
            break;
        }
    }

    if (idx >= als_value_num) {
        idx = als_value_num - 1;
    }

    if (1 == mTask.als_debounce_on) {
        if (check_timeout(&mTask.als_debounce_end)) {
            mTask.als_debounce_on = 0;
        }

        if (1 == mTask.als_debounce_on) {
            invalid = 1;
        }
    }

//value = mTask.data.als_raw_data;
	value = mTask.hw->als_value[idx];
	osLog(LOG_INFO, "TQQ getLuxFromAlsData alsdata :%d \n",value );

    if (!invalid) {
	#if 0
        int level_high = mTask.hw->als_level[idx];
        int level_low = (idx > 0) ? mTask.hw->als_level[idx - 1] : 0;
        int level_diff = level_high - level_low;
        int value_high = mTask.hw->als_value[idx];
        int value_low = (idx > 0) ? mTask.hw->als_value[idx - 1] : 0;
        int value_diff = value_high - value_low;
        int value = 0;

        if ((level_low >= level_high) || (value_low >= value_high))
            value = value_low;
        else
            value = (level_diff * value_low + (mTask.data.als_raw_data - level_low) * value_diff + ((
                         level_diff + 1) >> 1)) / level_diff;
/*bug 333213, liangjianfeng.wt, 2018/01/03, Temp Add for Lightsensor lux calu begin*/
		value = mTask.data.als_raw_data*6000/1000;
/*bug 333213, liangjianfeng.wt, 2018/01/03, Temp Add for Lightsensor lux calu end*/
#endif
        return value;
    }

    return -1;
}

static void alsGetData(void *sample) {
	 osLog(LOG_INFO, "alsGetData\n");
    struct SingleAxisDataPoint *singleSample = (struct SingleAxisDataPoint *)sample;
    singleSample->idata = getLuxFromAlsData();
}

static int EM30918_read_ps(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                          void *inBuf, uint8_t inSize, uint8_t elemInSize,
                          void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
                           osLog(LOG_INFO, "EM30918_read_ps\n");
    if (rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize)) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "EM30918 ps, rx dataInfo error\n");
        return -1;
    }
    mTask.txBuf[0] = EM_DATA1_PS_REG;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,next_state);
}
static int EM30918_read_ps_once(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                          void *inBuf, uint8_t inSize, uint8_t elemInSize,
                          void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
                           osLog(LOG_INFO, "EM30918_read_ps once \n");
    if (rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize)) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "EM30918 ps, rx dataInfo error\n");
        return -1;
    }
    mTask.txBuf[0] = EM_DATA1_PS_REG;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,next_state);
}


static int EM30918_get_ps_status(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {

    //int ps_flag = mTask.rxBuf[0] | (mTask.rxBuf[1]<<8);
    int ps_flag = mTask.rxBuf[0];
    osLog(LOG_INFO, "EM30918_get_ps_status ps data:%d  ps_threshold_low:%d   hight :%d\n",ps_flag,mTask.ps_threshold_low,mTask.ps_threshold_high);
    if (ps_flag <mTask.ps_threshold_low) { /* 1 is far */
        mTask.data.prox_state = PROX_STATE_FAR; /* far */
	 mTask.intr_flag_state =PROX_STATE_FAR;
        mTask.data.prox_data = 1; /* far state distance is 1cm */
	 mTask.intr_flag_value = 1;

    } else if (ps_flag >mTask.ps_threshold_high) {
        mTask.data.prox_state = PROX_STATE_NEAR; /* near */
	 mTask.intr_flag_state =PROX_STATE_NEAR;
        mTask.data.prox_data = 0; /* near state distance is 0cm */
	 mTask.intr_flag_value = 0;

    } else {
        mTask.data.prox_state = mTask.intr_flag_state;
        mTask.data.prox_data = mTask.intr_flag_value ;
	}
    mTask.txBuf[0] = EM_FLAG_REG;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 2, i2cCallBack,
                         next_state);
}
#if 1
static int EM30918_get_ps_status_once(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {

    //int ps_flag = mTask.rxBuf[0] | (mTask.rxBuf[1]<<8);
    int ps_flag = mTask.rxBuf[0];
    osLog(LOG_INFO, "EM30918_get_ps_status ps data:%d  ps_threshold_low:%d   hight :%d\n",ps_flag,mTask.ps_threshold_low,mTask.ps_threshold_high);
    if (ps_flag <mTask.ps_threshold_low) { /* 1 is far */
        mTask.data.prox_state = PROX_STATE_FAR; /* far */
	 mTask.intr_flag_state =PROX_STATE_FAR;
        mTask.data.prox_data = 1; /* far state distance is 1cm */
	 mTask.intr_flag_value = 1;

    } else if (ps_flag >mTask.ps_threshold_high) {
        mTask.data.prox_state = PROX_STATE_NEAR; /* near */
	 mTask.intr_flag_state =PROX_STATE_NEAR;
        mTask.data.prox_data = 0; /* near state distance is 0cm */
	 mTask.intr_flag_value = 0;

    } else {
        mTask.data.prox_state = mTask.intr_flag_state;
        mTask.data.prox_data = mTask.intr_flag_value ;
	}

	mTask.data.sensType = SENS_TYPE_PROX;
	txTransferDataInfo(&mTask.dataInfo, 1, &mTask.data);
	sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);

	mTask.txBuf[0] = EM_FLAG_REG;
	return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 2, i2cCallBack,
                         next_state);
}

#endif
#if 0
static int EM30918_get_ps_raw_data(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                  void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                  void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
                                    osLog(LOG_INFO, "EM30918_get_ps_raw_data\n");
    mTask.data.prox_raw_data = (mTask.rxBuf[1] << 8) | mTask.rxBuf[0];

    osLog(LOG_WARN,"EM30918: EM30918_get_ps_raw_data(prox_raw_data:%u,mTask.psCali[0]:%u)\n", mTask.data.prox_raw_data,mTask.psCali[0]);
/*
    if (mTask.data.prox_raw_data < mTask.psCali[0])
        mTask.data.prox_raw_data = 0;
    mTask.data.prox_raw_data -= mTask.psCali[0];
*/
//    osLog(LOG_WARN,"EM30918: EM30918_get_ps_raw_data(prox_raw_data:%u,prox_data:%u)\n", mTask.data.prox_raw_data,mTask.data.prox_data);

    mTask.txBuf[0] = EM_FLAG_REG;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                       mTask.rxBuf, 2, i2cCallBack,
                       next_state);
}
#endif
static int EM30918_clr_int(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                  void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                  void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
                                    osLog(LOG_INFO, "EM30918_clr_int\n");
    mTask.txBuf[0] = EM_FLAG_REG;
    mTask.txBuf[1] = mTask.rxBuf[0] & ~EM_FLG_PSINT_MASK;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}


static int EM30918_enable_eint(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                  void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                  void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
         mt_eint_unmask(mTask.hw->eint_num);
          osLog(LOG_INFO, "EM30918_enable_eint\n");
	 mTask.data.sensType = SENS_TYPE_PROX;

         txTransferDataInfo(&mTask.dataInfo, 1, &mTask.data);
         sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
         return 0;
}

static void psGetData(void *sample) {
    char txBuf[1];
    static char rxBuf[2];
    struct TripleAxisDataPoint *tripleSample = (struct TripleAxisDataPoint *)sample;
	  osLog(LOG_INFO, "psGetData mTask.psCali[0]:%d\n",mTask.psCali[0]);
    tripleSample->ix = mTask.prox_raw_data;
    tripleSample->iy = (mTask.data.prox_state == PROX_STATE_NEAR ? 0 : 1);

    void get_ps_data(void *cookie, size_t tx, size_t rx, int err) {
        char *rxBuf = cookie;
        if (err == 0) {
            //mTask.data.prox_raw_data = (rxBuf[1] << 8) | rxBuf[0];
            mTask.prox_raw_data =  rxBuf[0];
         /*   if (mTask.data.prox_raw_data < mTask.psCali[0])
                mTask.data.prox_raw_data = 0;
            mTask.data.prox_raw_data -= mTask.psCali[0];*/
        } else
            osLog(LOG_INFO, "EM30918: read ps data i2c error (%d)\n", err);
    }
    txBuf[0] = EM_DATA1_PS_REG;
    i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, txBuf, 1,
                  rxBuf, 1, get_ps_data, rxBuf);
}

static void EM30918_ps_set_threshold(uint32_t threshold_high, uint32_t threshold_low) {
    int ret;
    char txBuf[6];
  osLog(LOG_INFO, "EM30918_ps_set_threshold\n");
    /* when user don't do ps cali, AP will set 0 to here,
     * so filter it and still use alsps cust setting */
    if (threshold_high == 0 && threshold_low == 0)
        return;

	 //txBuf[0] = EM_THDL1_PS_REG;
	txBuf[1] = (uint8_t)(threshold_low & 0xFF);
	txBuf[2] = (uint8_t)(threshold_high & 0xFF);


       ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, &txBuf[0], 3,
                NULL, 0);

    if (ret) {
        osLog(LOG_INFO, "EM30918: set ps threshold i2c error (%d)\n", ret);
        return;
    }
}

#ifdef EM_TUNE0
static int EM30918_compare_sun_level(uint32_t word_data)
{
	uint8_t mode;
	int32_t lii;

	mode = (mTask.ps_ctrl_val) & 0x3F;
	if (mode == 0x30)
		lii = 100;
	else if (mode == 0x31)
		lii = 200;
	else if (mode == 0x32)
		lii = 400;
	else if (mode == 0x33)
		lii = 800;
	else
	{
		lii = 100;
		//osLog(LOG_INFO, "EM30918_get_sun_level: unsupported PS_IT(0x%x)\n", mode);
	}
	osLog(LOG_INFO, "EM30918_get_sun_level: word_data1=%d, lii=%d\n", word_data, lii);
	if (word_data > lii) {
		return -1;
	}
	return 0;
}

static void EM30918_ct_tracking(void)// ill
{
	osLog(LOG_WARN, "EM30918_ct_tracking\n");
	if (mTask.data.prox_state == PROX_STATE_NEAR)
	{
		if (((mTask.ps_raw_data - mTask.ps_min) > EM_H_PS) && (mTask.ps_min != 0xFFFF) && (mTask.ps_thd_update == false))
		{//judge if skin is close
			mTask.ps_threshold_high = mTask.ps_min + EM_H_HT;
			mTask.ps_threshold_low = mTask.ps_min + EM_H_LT;
			EM30918_ps_set_threshold(mTask.ps_threshold_high, mTask.ps_threshold_low);
			mTask.ps_thd_update = true;
			osLog(LOG_WARN, "EM30918_ct_tracking: set thd HT=%d, LT=%d, ps=%d, ps_min=%d\n", mTask.ps_threshold_high, mTask.ps_threshold_low, mTask.ps_raw_data, mTask.ps_min);
		}
	}
	else
	{
		osLog(LOG_INFO, "EM30918_ct_tracking mTask.data.prox_state far\n");
		if (mTask.ps_thd_update)
		{
			mTask.ps_thd_update = false;
			mTask.txBuf[0] = EM_DATA_PS_OFF_REG;
			i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1, mTask.rxBuf, 4, NULL, NULL);

			mTask.psoff_data = (mTask.rxBuf[0] << 8) | mTask.rxBuf[1];
			mTask.psoff_data += ((mTask.rxBuf[2] << 8) | mTask.rxBuf[3]);
			osLog(LOG_INFO, "EM30918_ct_tracking psoff_data:%ld\n",mTask.psoff_data);

			if (EM30918_compare_sun_level(mTask.psoff_data) == 0)
			{
				if (mTask.ps_min<0xFFFF)
				{
					mTask.ps_threshold_high = mTask.ps_min + pscaliCfgPacket.caliCfgData[0];//EM_HT_N_CT;
					mTask.ps_threshold_low = mTask.ps_min + pscaliCfgPacket.caliCfgData[1];//EM_LT_N_CT;
				}
				EM30918_ps_set_threshold(mTask.ps_threshold_high, mTask.ps_threshold_low);
				osLog(LOG_WARN, "EM30918_ct_tracking: light mode change thd HT=%d, LT=%d, ps=%d\n", mTask.ps_threshold_high, mTask.ps_threshold_low, mTask.ps_raw_data);
			}
		}
	}
}

static int EM30918_prx_tune_zero_func_fae(void)
{
	uint16_t ct_value;

	if (!mTask.psPowerOn)
	{
		return 0;
	}
	mTask.ps_raw_data = mTask.psCali[2];

	if((mTask.psi_set != 0) && (mTask.ps_thd_update==0))
	{
		if(mTask.data.prox_state != PROX_STATE_NEAR)
		{
			mTask.txBuf[0] = EM_DATA_PS_OFF_REG;
			i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1, mTask.rxBuf, 4, NULL, NULL);
			mTask.psoff_data = (mTask.rxBuf[0] << 8) | mTask.rxBuf[1];
			mTask.psoff_data += ((mTask.rxBuf[2] << 8) | mTask.rxBuf[3]);
			osLog(LOG_INFO, "EM30918_prx_tune_zero_func_fae 00 psoff_data:%ld\n",mTask.psoff_data);
			if (EM30918_compare_sun_level(mTask.psoff_data) == 0)
			{
				if(mTask.ps_raw_data > 0)
				{
					ct_value = mTask.ps_threshold_high - pscaliCfgPacket.caliCfgData[0];
					if((mTask.ps_raw_data < ct_value) && ((ct_value - mTask.ps_raw_data) > 2))
					{
						mTask.ps_min = mTask.ps_raw_data;
						mTask.ps_threshold_high = mTask.ps_min + pscaliCfgPacket.caliCfgData[0];
						mTask.ps_threshold_low = mTask.ps_min + pscaliCfgPacket.caliCfgData[1];
						EM30918_ps_set_threshold(mTask.ps_threshold_high, mTask.ps_threshold_low);
					}
				}
			}
		}
		return 0;
	}
	mTask.txBuf[0] = EM_DATA_PS_OFF_REG;
	i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1, mTask.rxBuf, 4, NULL, NULL);
	mTask.psoff_data = (mTask.rxBuf[0] << 8) | mTask.rxBuf[1];
	mTask.psoff_data += ((mTask.rxBuf[2] << 8) | mTask.rxBuf[3]);
	osLog(LOG_INFO, "EM30918_prx_tune_zero_func_fae 01 psoff_data:%ld\n",mTask.psoff_data);

	//Under the sun
	if (EM30918_compare_sun_level(mTask.psoff_data) != 0)
	{
		return 0;
	}

	//Find ps min & max
	if ((mTask.ps_raw_data != 0) && (mTask.ps_raw_data != 0xFFFF))
	{
		if (mTask.ps_raw_data > mTask.ps_max)
		{
			mTask.ps_max = mTask.ps_raw_data;
			osLog(LOG_INFO, "EM30918_prx_tune_zero_func_fae: update ps_max=%d, psi=%d\n", mTask.ps_max, mTask.ps_min);
		}
		if (mTask.ps_raw_data < mTask.ps_min)
		{
			mTask.ps_min = mTask.ps_raw_data;
			osLog(LOG_INFO, "EM30918_prx_tune_zero_func_fae: update ps_max=%d, ps_min=%d\n", mTask.ps_max, mTask.ps_min);
		}
	}

	if (mTask.ps_min != 0xFFFF)
	{
		//ps min to set thd
		if ((mTask.ps_max - mTask.ps_min) > EM_MAX_MIN_DIFF)
		{
			mTask.psi_set = mTask.ps_min;
			mTask.ps_threshold_high = mTask.ps_min + pscaliCfgPacket.caliCfgData[0];//EM_HT_N_CT;
			mTask.ps_threshold_low = mTask.ps_min + pscaliCfgPacket.caliCfgData[1];//EM_LT_N_CT;
			EM30918_ps_set_threshold(mTask.ps_threshold_high, mTask.ps_threshold_low);
			osLog(LOG_INFO, "EM30918_prx_tune_zero_func_fae: sun thd HT=%d, LT=%d, psi=%d\n", mTask.ps_threshold_high, mTask.ps_threshold_low, mTask.ps_min);
		}
	}
	return 0;
}
#endif
static void psGetCalibration(int32_t *cali, int32_t size) {
    cali = mTask.psCali;
    osLog(LOG_INFO, "EM30918:%s prox_raw_data:%ld,cali[1]:%ld,cali[2]:%ld \n", __func__, mTask.prox_raw_data, cali[1],cali[2]);
	if(mTask.psCali[2] == 0)
	{
		mTask.psCali[2] = mTask.prox_raw_data;
	//	EM30918_ps_set_threshold(pscaliCfgPacket.caliCfgData[0] + mTask.psCali[2],
        //                    pscaliCfgPacket.caliCfgData[1] + mTask.psCali[2]);

		EM30918_ps_set_threshold(mTask.ps_threshold_high, mTask.ps_threshold_low);

	}
//	(void)EM30918_prx_tune_zero_func_fae();
	//EM30918_ct_tracking();
}

static void psSetCalibration(int32_t *cali, int32_t size) {
    mTask.psCali[0] = cali[0];
    mTask.psCali[1] = cali[1];
    mTask.psCali[2] = cali[2];
    osLog(LOG_INFO, "EM30918:%s cali[0]:%ld,cali[1]:%ld,cali[2]:%ld \n", __func__, cali[0], cali[1],cali[2]);
    /*EM30918_ps_set_threshold(mTask.ps_threshold_high + mTask.psCali[0],
                            mTask.ps_threshold_low + mTask.psCali[0]);*/
}

static void psGetThreshold(uint32_t *threshold_high, uint32_t *threshold_low) {
    *threshold_high = mTask.ps_threshold_high;
    *threshold_low = mTask.ps_threshold_low;
    osLog(LOG_INFO, "%s ==>threshold_high:%ld threshold_low:%ld \n", __func__, *threshold_high, *threshold_low);
}

static void psSetThreshold(uint32_t threshold_high, uint32_t threshold_low) {
    osLog(LOG_INFO, "%s ==>threshold_high:%ld threshold_low:%ld \n", __func__, threshold_high, threshold_low);
    mTask.ps_threshold_high = threshold_high;
    mTask.ps_threshold_low = threshold_low;
    EM30918_ps_set_threshold(mTask.ps_threshold_high, mTask.ps_threshold_low);
}


static void psGetSensorInfo(struct sensorInfo_t *data)
{
    strncpy(data->name, "EM30918_p", sizeof(data->name));
}

static void alsGetSensorInfo(struct sensorInfo_t *data)
{
    strncpy(data->name, "EM30918_l", sizeof(data->name));
}

static int EM30918_mask_eint(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mt_eint_mask(mTask.hw->eint_num);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int EM30918_unmask_eint(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mt_eint_unmask(mTask.hw->eint_num);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int EM30918_set_als_debounce_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                      void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                      void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.als_debounce_on = 1;
    cal_end_time(mTask.als_debounce_time, &mTask.als_debounce_end);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int EM30918_set_ps_debounce_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                     void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                     void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
                                      osLog(LOG_INFO, "EM30918_set_ps_debounce_on\n");
    mTask.ps_debounce_on = 1;
    cal_end_time(mTask.ps_debounce_time, &mTask.ps_debounce_end);
    mTask.data.prox_state = PROX_STATE_INIT;
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int EM30918_set_als_power_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                   void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                   void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
                                    osLog(LOG_INFO, "EM30918_set_als_power_on\n");
	mTask.alsPowerOn = true;
	mTask.txBuf[0] = EM_STATE_REG;
	mTask.txBuf[1] = mTask.rxBuf[0] | EM_STATE_EN_ALS_MASK;
	mTask.state_val = mTask.txBuf[1];
      return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int EM30918_set_als_power_off(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                    void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
                                       osLog(LOG_INFO, "EM30918_set_als_power_off\n");
	mTask.alsPowerOn = false;
	mTask.als_drup_data = true;
	mTask.als_debounce_on = 0;
      mTask.txBuf[0] = EM_STATE_REG;
      mTask.txBuf[1] = mTask.rxBuf[0] & ~EM_STATE_EN_ALS_MASK;
	mTask.state_val = mTask.txBuf[1];
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int EM30918_als_ratechg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
                               osLog(LOG_INFO, "EM30918_als_ratechg\n");
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int EM30918_als_cali(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int32_t alsCali[2];
    double   alsCali_mi;

    alsCali_mi = (double)200/mTask.data.als_data;
    osLog(LOG_INFO, "EM30918_als_cali:[%.5f] als_data:%d\n", alsCali_mi,mTask.data.als_data);
    alsCali[0] = alsCali_mi * ALS_INCREASE_NUM_AP;
    alsCali[1] = 0;

    alsPsSendCalibrationResult(SENS_TYPE_ALS, alsCali);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int EM30918_als_cfg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    struct alspsCaliCfgPacket caliCfgPacket;
    double  alsCali_mi;
    osLog(LOG_INFO, "EM30918_als_cfg\n");
    ret = rxCaliCfgInfo(&caliCfgPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);

    if (ret < 0) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "EM30918_als_cfg, rx inSize and elemSize error\n");
        return -1;
    }
    alsCali_mi = (double)((float)caliCfgPacket.caliCfgData[0] / ALS_INCREASE_NUM_AP);
    osLog(LOG_INFO, "EM30918_als_cfg: [%.5f]\n", alsCali_mi);
    mTask.alsCali = alsCali_mi;//caliCfgPacket.caliCfgData[0];
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int EM30918_set_ps_power_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                  void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                  void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {

    osLog(LOG_INFO, "EM30918_set_ps_power_on caliCfgData[0]=%d,caliCfgData[0]=%d\n",pscaliCfgPacket.caliCfgData[0],pscaliCfgPacket.caliCfgData[1]);
/*	if(pscaliCfgPacket.caliCfgData[0] == 0 && pscaliCfgPacket.caliCfgData[1] == 0)
	{
		pscaliCfgPacket.caliCfgData[0] = EM_HT_N_CT;
		pscaliCfgPacket.caliCfgData[1] = EM_LT_N_CT;
	}*/

	EM30918_ps_set_threshold(mTask.ps_threshold_high, mTask.ps_threshold_low);
	mTask.ps_thd_update = 0;
	mTask.ct_tracking_index = 0;
	mTask.psPowerOn = true;
	mTask.intr_flag_value = 1;
	mTask.txBuf[0] = EM_STATE_REG;
	mTask.txBuf[1] = mTask.rxBuf[0] | EM_STATE_EN_PS_MASK;
	mTask.state_val = mTask.txBuf[1];

    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int EM30918_set_ps_power_off(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                   void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                   void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
                                   osLog(LOG_INFO, "EM30918_set_ps_power_off\n");
	mTask.psPowerOn = false;
	mTask.ps_debounce_on = 0;
	mTask.data.prox_state = PROX_STATE_FAR;
	mTask.txBuf[0] = EM_STATE_REG;
	mTask.txBuf[1] = mTask.rxBuf[0] & ~EM_STATE_EN_PS_MASK;
	mTask.state_val = mTask.txBuf[1];
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int EM30918_ps_ratechg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
                             osLog(LOG_INFO, "EM30918_ps_ratechg\n");
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int EM30918_ps_cali(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int32_t threshold[2];
    threshold[0] = 2; // high simulation data
    threshold[1] = 3; // low simulation data
     osLog(LOG_INFO, "EM30918_ps_cali\n");
    alsPsSendCalibrationResult(SENS_TYPE_PROX, threshold);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int EM30918_ps_cfg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
  osLog(LOG_INFO, "EM30918_ps_cfg\n");
    ret = rxCaliCfgInfo(&pscaliCfgPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);

    if (ret < 0) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "EM30918_ps_cfg, rx inSize and elemSize error\n");
        return -1;
    }

    mTask.txBuf[0] = EM_DATA1_PS_REG;
    i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,
                         next_state);
	//mTask.data.prox_raw_data = (mTask.rxBuf[0] << 8) | mTask.rxBuf[1];
	mTask.prox_raw_data = mTask.rxBuf[0];
    osLog(LOG_INFO, "EM30918_ps_cfg, [high, low]: [%d, %d] [rawdata:%d]\n",pscaliCfgPacket.caliCfgData[0], pscaliCfgPacket.caliCfgData[1],mTask.prox_raw_data );
  //  EM30918_ps_set_threshold(pscaliCfgPacket.caliCfgData[0], pscaliCfgPacket.caliCfgData[1]);
  EM30918_ps_set_threshold(mTask.als_threshold_high, mTask.als_threshold_low);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int EM30918_get_alsps_state(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                               void *inBuf, uint8_t inSize, uint8_t elemInSize,
                               void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
                                 osLog(LOG_INFO, "EM30918_get_alsps_state\n");
    mTask.txBuf[0] = EM_STATE_REG;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,
                         next_state);
}

static int EM30918_set_sw_reset(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    mTask.txBuf[0] = EM_SW_RESET_REG;
    mTask.txBuf[1] = 0X00;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                       i2cCallBack, next_state);
}

static int EM30918_set_alsps_ctrl(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
	int ret;
 osLog(LOG_INFO, "EM30918_set_alsps_ctrl\n");
    mTask.txBuf[0] = EM_STATE_REG;
    mTask.txBuf[1] = 0x00;
	ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, NULL, NULL);
	if(ret){
		sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
		osLog(LOG_INFO, "EM30918: EM30918_set_alsps_ctrl i2c error (%d)\n", ret);
		return -1;
	}

    mTask.txBuf[0] = EM_INT_CTRL;
    mTask.txBuf[1] = 0x00;
	ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, NULL, NULL);
	if(ret){
		sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
		osLog(LOG_INFO, "EM30918: EM30918_set_alsps_ctrl i2c error (%d)\n", ret);
		return -1;
	}
	    mTask.txBuf[0] = EM_THDL_PS_REG;
    mTask.txBuf[1] = 0x0f;
	ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, NULL, NULL);
	if(ret){
		sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
		osLog(LOG_INFO, "EM30918: EM30918_set_alsps_ctrl i2c error (%d)\n", ret);
		return -1;
	}

    mTask.txBuf[0] = EM_THDH_PS_REG;
    mTask.txBuf[1] = 0x19;
	ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, NULL, NULL);
	if(ret){
		sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
		osLog(LOG_INFO, "EM30918: EM30918_set_alsps_ctrl i2c error (%d)\n", ret);
		return -1;
	}
/*
	mTask.txBuf[0] = EM_OSLCAL_REG;
      mTask.txBuf[1] = 0x80;
	ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, NULL, NULL);
	if(ret){
		sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
		osLog(LOG_INFO, "EM30918: EM30918_set_alsps_ctrl i2c error (%d)\n", ret);
		return -1;
	}

	mTask.txBuf[0] = EM_LEDCTR_REG;
       mTask.txBuf[1] = 0x80;
	ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, NULL, NULL);
	if(ret){
		sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
		osLog(LOG_INFO, "EM30918: EM30918_set_alsps_ctrl i2c error (%d)\n", ret);
		return -1;
	}

	mTask.txBuf[0] = EM_PSCTR_REG;
       mTask.txBuf[1] = 0x3d;
	ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, NULL, NULL);
	if(ret){
		sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
		osLog(LOG_INFO, "EM30918: EM30918_set_alsps_ctrl i2c error (%d)\n", ret);
		return -1;
	}

	mTask.txBuf[0] = EM_PS_GAIN_CTRL;
       mTask.txBuf[1] = 0x57;
	ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, NULL, NULL);
	if(ret){
		sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
		osLog(LOG_INFO, "EM30918: EM30918_set_alsps_ctrl i2c error (%d)\n", ret);
		return -1;
	}

	mTask.txBuf[0] = EM_ALS_CTRL_REG;
       mTask.txBuf[1] = 0x67; //d1
	ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, NULL, NULL);
	if(ret){
		sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
		osLog(LOG_INFO, "EM30918: EM30918_set_alsps_ctrl i2c error (%d)\n", ret);
		return -1;
	}
	mTask.txBuf[0] = EM_PS_OFFSET_REG;
       mTask.txBuf[1] = 0x00;
	ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, NULL, NULL);
	if(ret){
		sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
		osLog(LOG_INFO, "EM30918: EM30918_set_alsps_ctrl i2c error (%d)\n", ret);
		return -1;
	}
	mTask.txBuf[0] = EM_ALS_OFFSET_REG;
       mTask.txBuf[1] = 0x00;
	ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, NULL, NULL);
	if(ret){
		sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
		osLog(LOG_INFO, "EM30918: EM30918_set_alsps_ctrl i2c error (%d)\n", ret);
		return -1;
	}*/
	sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
	return 0;

}

static int EM30918_set_ps_thdl(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
                                 osLog(LOG_INFO, "EM30918_set_ps_thdl\n");
    if (mTask.hw->polling_mode_ps == 0) {
       /* mTask.txBuf[0] = EM_THDL1_PS_REG;
	 mTask.txBuf[1] = (uint8_t)((mTask.ps_threshold_low + mTask.psCali[0]) & 0x00FF);
        mTask.txBuf[2] = (uint8_t)(((mTask.ps_threshold_low + mTask.psCali[0]) & 0xFF00) >> 8);


        return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3,
                           i2cCallBack, next_state);*/
                           mTask.txBuf[0] = EM_THDL_PS_REG;
	 mTask.txBuf[1] = (uint8_t)((mTask.ps_threshold_low + mTask.psCali[0]) & 0x00FF);



        return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                           i2cCallBack, next_state);

    } else {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
        return 0;
    }
}

static int EM30918_set_ps_thdh(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
                                  osLog(LOG_INFO, "EM30918_set_ps_thdh\n");
    if (mTask.hw->polling_mode_ps == 0) {
        /*mTask.txBuf[0] = EM_THDH1_PS_REG;
	 mTask.txBuf[1] = (uint8_t)((mTask.ps_threshold_high + mTask.psCali[0]) & 0x00FF);
        mTask.txBuf[2] = (uint8_t)(((mTask.ps_threshold_high + mTask.psCali[0]) & 0xFF00) >> 8);

        return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3,
                           i2cCallBack, next_state);*/


                            mTask.txBuf[0] = EM_THDH_PS_REG;
	 mTask.txBuf[1] = (uint8_t)((mTask.ps_threshold_high + mTask.psCali[0]) & 0x00FF);


        return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
                           i2cCallBack, next_state);
    } else {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
        return 0;
    }
}

static void EM30918_eint_handler(int arg) {
	  osLog(LOG_INFO, "EM30918_eint_handler\n");
    alsPsInterruptOccur();
}

static int EM30918_setup_eint(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
                              osLog(LOG_INFO, "EM30918_setup_eint\n");
    mt_eint_dis_hw_debounce(mTask.hw->eint_num);
    mt_eint_registration(mTask.hw->eint_num, LEVEL_SENSITIVE, LOW_LEVEL_TRIGGER, EM30918_eint_handler, EINT_INT_UNMASK,
                         EINT_INT_AUTO_UNMASK_OFF);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}



static int EM30918_register_core(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
                                osLog(LOG_INFO, "EM30918_register_core\n");
    struct sensorCoreInfo mInfo;
    memset(&mInfo,0x00,sizeof(struct sensorCoreInfo));
    /* Register sensor Core */
    mInfo.sensType = SENS_TYPE_ALS;
    mInfo.getData = alsGetData;
    mInfo.getSensorInfo = alsGetSensorInfo;
    sensorCoreRegister(&mInfo);
//    memset(&mInfo,0x00,sizeof(struct sensorCoreInfo));
    mInfo.sensType = SENS_TYPE_PROX,
    mInfo.gain = 1;
    mInfo.sensitivity = 1;
    sensorDriverGetConvert(0, &mInfo.cvt);
    mInfo.getCalibration = psGetCalibration;
    mInfo.setCalibration = psSetCalibration;
    mInfo.getThreshold = psGetThreshold;
    mInfo.setThreshold = psSetThreshold;
    mInfo.getData = psGetData;


  // mInfo.setDebugTrace = EM30918_SetDebugTrace;
    mInfo.getSensorInfo = psGetSensorInfo;
    sensorCoreRegister(&mInfo);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static struct sensorFsm EM30918Fsm[] = {
    /* sample als */
    sensorFsmCmd(STATE_SAMPLE_ALS, STATE_GET_ALS_DATA, EM30918_read_als_data),
    sensorFsmCmd(STATE_GET_ALS_DATA, STATE_SAMPLE_ALS_DONE, EM30918_get_als_value),

    /* sample ps */
#if 1
    sensorFsmCmd(STATE_SAMPLE_PS, STATE_GET_PS_FLG, EM30918_read_ps),
    sensorFsmCmd(STATE_GET_PS_FLG, STATE_CLR_INT, EM30918_get_ps_status),
    sensorFsmCmd(STATE_CLR_INT, STATE_EN_EINT, EM30918_clr_int),
    sensorFsmCmd(STATE_EN_EINT, STATE_SAMPLE_PS_DONE, EM30918_enable_eint),
#endif
#if 1
sensorFsmCmd(STATE_SAMPLE_PS_ONCE, STATE_GET_PS_RAW_DATA_ONCE, EM30918_read_ps_once),
sensorFsmCmd(STATE_GET_PS_RAW_DATA_ONCE, STATE_SAMPLE_PS_DONE, EM30918_get_ps_status_once),

#endif
    /* als enable state */
    sensorFsmCmd(STATE_ALS_ENABLE, STATE_ALS_POWER_ON, EM30918_get_alsps_state),
    sensorFsmCmd(STATE_ALS_POWER_ON, STATE_ALS_SET_DEBOUNCE, EM30918_set_als_power_on),
    sensorFsmCmd(STATE_ALS_SET_DEBOUNCE, STATE_ALS_ENABLE_DONE, EM30918_set_als_debounce_on),

    /* als disable state */
    sensorFsmCmd(STATE_ALS_DISABLE, STATE_ALS_POWER_OFF, EM30918_get_alsps_state),
    sensorFsmCmd(STATE_ALS_POWER_OFF, STATE_ALS_DISABLE_DONE, EM30918_set_als_power_off),

    /* als rate change */
    sensorFsmCmd(STATE_ALS_RATECHG, STATE_ALS_RATECHG_DONE, EM30918_als_ratechg),


    /* als cali state */
    sensorFsmCmd(STATE_ALS_CALI, CHIP_ALS_CALI_DONE, EM30918_als_cali),
    /* als cfg state */
    sensorFsmCmd(STATE_ALS_CFG, CHIP_ALS_CFG_DONE, EM30918_als_cfg),

    /* ps enable state */
    sensorFsmCmd(STATE_PS_ENABLE, STATE_PS_POWER_ON, EM30918_get_alsps_state),
    sensorFsmCmd(STATE_PS_POWER_ON, STATE_PS_SET_DEBOUNCE, EM30918_set_ps_power_on),
    sensorFsmCmd(STATE_PS_SET_DEBOUNCE, STATE_PS_UNMASK_EINT, EM30918_set_ps_debounce_on),
    sensorFsmCmd(STATE_PS_UNMASK_EINT, STATE_PS_ENABLE_DONE, EM30918_unmask_eint),
    /* ps disable state */
    sensorFsmCmd(STATE_PS_DISABLE, STATE_GET_ALSPS_STATE, EM30918_mask_eint),
    sensorFsmCmd(STATE_GET_ALSPS_STATE, STATE_PS_POWER_OFF, EM30918_get_alsps_state),
    sensorFsmCmd(STATE_PS_POWER_OFF, STATE_PS_DISABLE_DONE, EM30918_set_ps_power_off),

    /* ps rate change */
    sensorFsmCmd(STATE_PS_RATECHG, STATE_PS_RATECHG_DONE, EM30918_ps_ratechg),

    /* ps cali state */
    sensorFsmCmd(STATE_PS_CALI, CHIP_PS_CALI_DONE, EM30918_ps_cali),
    /* ps cfg state */
    sensorFsmCmd(STATE_PS_CFG, CHIP_PS_CFG_DONE, EM30918_ps_cfg),

    /* init state */
    sensorFsmCmd(STATE_RESET, STATE_SET_ALSPS_CTRL, EM30918_set_sw_reset),
    sensorFsmCmd(STATE_SET_ALSPS_CTRL, STATE_SET_PS_THDH, EM30918_set_alsps_ctrl),
    sensorFsmCmd(STATE_SET_PS_THDH, STATE_SET_PS_THDL, EM30918_set_ps_thdh),
    sensorFsmCmd(STATE_SET_PS_THDL, STATE_SETUP_EINT, EM30918_set_ps_thdl),
    sensorFsmCmd(STATE_SETUP_EINT, STATE_CORE, EM30918_setup_eint),
    sensorFsmCmd(STATE_CORE, STATE_INIT_DONE, EM30918_register_core),

};

static int EM30918Init(void) {
    int ret = 0;
    osLog(LOG_INFO, "%s: task starting\n", __func__);
    EM30918DebugPoint = &mTask;

    mTask.hw = get_cust_alsps("EM30918");
    if (NULL == mTask.hw) {
        osLog(LOG_ERROR, "get_cust_acc_hw fail\n");
        return -1;
    }
    mTask.i2c_addr = mTask.hw->i2c_addr[0];
    mTask.als_debounce_time = 200;
    mTask.als_debounce_on = 0;
    mTask.ps_debounce_time = 10;
    mTask.ps_debounce_on = 0;
    mTask.psCali[0] = 0;
    mTask.psCali[1] = 0;
    mTask.psCali[2] = 0;
    mTask.als_threshold_high = mTask.hw->als_threshold_high;
    mTask.als_threshold_low = mTask.hw->als_threshold_low;
    mTask.ps_threshold_high = mTask.hw->ps_threshold_high;
    mTask.ps_threshold_low = mTask.hw->ps_threshold_low;

	mTask.state_val = 0;
	mTask.ps_min = 0xFFFF;
	mTask.ps_max = 0;
//mTask.psi_set = 0;

    i2cMasterRequest(mTask.hw->i2c_num, I2C_SPEED);

    mTask.txBuf[0] = EM_PDT_ID_REG;
    ret = i2cMasterTxRxSync(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
        &mTask.deviceId, 1, NULL, NULL);
    if (ret < 0) {
        osLog(LOG_ERROR, "EM30918 i2cMasterTxRxSync fail!!!\n");
        ret = -1;
        i2cMasterRelease(mTask.hw->i2c_num);
        goto err_out;
    }

    if (mTask.deviceId == 0x31) {
        osLog(LOG_INFO, "em30918 auto detect success! deviceid = %x\n", mTask.deviceId);
    } else {
        ret = -1;
        i2cMasterRelease(mTask.hw->i2c_num);
		osLog(LOG_INFO, "em30918 auto detect fail! deviceid =  %x\n", mTask.deviceId);
        goto err_out;
    }

  //  mTask.ledctrl_val = 0x20;

    alsSensorRegister();
    psSensorRegister();
    registerAlsPsDriverFsm(EM30918Fsm, ARRAY_SIZE(EM30918Fsm));
err_out:
    return ret;
}

#ifndef CFG_OVERLAY_INIT_SUPPORT
MODULE_DECLARE(EM30918, SENS_TYPE_ALS, EM30918Init);
#else
#include "mtk_overlay_init.h"
OVERLAY_DECLARE(EM30918, OVERLAY_ID_ALSPS, EM30918Init);
#endif
