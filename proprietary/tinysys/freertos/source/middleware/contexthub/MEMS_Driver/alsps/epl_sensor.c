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
//#include <stdbool.h>
//#include <stdint.h>
#include <seos.h>
#include <util.h>
#include <sensors.h>
#include <plat/inc/rtc.h>
#include <contexthub_core.h>
#include <mt_gpt.h>
#include <timer.h>
#include "eint.h"
#include "epl_sensor.h"
#include "alsps.h"
#include "cust_alsps.h"

#define DRIVER_VERSION          "1.0.1"

#define PS_DYN_K            1
#define PS_FIRST_REPORT     1
#define ALS_DYN_INTT        1
#define ALS_LSRC            0
#define REG_MONITOR         0
#define ALS_DYN_INTT_NO_DELAY 1
#define PS_KEEP_LOW_PATCH   1

#define I2C_SPEED           400000

enum eplState {
    STATE_SAMPLE_ALS = CHIP_SAMPLING_ALS,
    STATE_SAMPLE_ALS_DONE = CHIP_SAMPLING_ALS_DONE,
    STATE_SAMPLE_PS = CHIP_SAMPLING_PS,
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

    //enalbe
    STATE_ALS_POWER_ON,
    STATE_ALS_SET_DEBOUNCE,
    STATE_ALS_MODE,
#if ALS_DYN_INTT
    STATE_SET_ALS_DYN,
#endif
    STATE_ALS_CHIP_ON,

    STATE_PS_POWER_ON,
    STATE_PS_SET_DEBOUNCE,
    STATE_PS_MODE,
    STATE_PS_CHIP_ON,
#if PS_FIRST_REPORT
    STATE_PS_EN_SET_MAX_THD,
    STATE_PS_EN_CMP_RST,
    STATE_PS_EN_CMP_RUN,
#endif

    //disable
    STATE_ALS_POWER_OFF,
    STATE_ALS_CHIP_OFF,
    STATE_PS_POWER_OFF,
    STATE_PS_CHIP_OFF,
    //read als
    STATE_GET_ALS_DATA,
    STATE_REPORT_ALS,
    //read ps
    STATE_GET_PS_DATA,
#if PS_KEEP_LOW_PATCH
    STATE_GET_PS_PWR_ON,
#endif
    STATE_SET_PS_CMP_RUN,
    STATE_REPORT_PS,
#if PS_DYN_K
    STATE_PS_DO_CIAL,
#endif
#if REG_MONITOR
    //read reg
    STATE_GET_REG_DATA,
    STATE_CHECK_REG,
#endif
    //init
    STATE_SET_INIT_GLB,
    STATE_GET_RENVO,
    STATE_PS_CMP_RESET,
    STATE_PS_CMP_RUN,
    STATE_ALS_CMP_RESET,
    STATE_ALS_CMP_RUN,
    STATE_POWER_OFF,
    STATE_POWER_ON,
    STATE_CHIP_UNKEY,
    STATE_CHIP_REFRESH0,
    STATE_CHIP_REFRESH1,
    STATE_CHIP_KEY,
    STATE_CHIP_DOC,
    STATE_OFFSET,
    STATE_SET_PS_THD,
    STATE_SET_ALS_THD,
    STATE_SET_ALS_CONF,
    STATE_SET_PS_CONF,
    STATE_SET_LED_CONF,
    STATE_SETUP_EINT,
    STATE_CORE
};
#define MAX_RXBUF 			8
#define MAX_TXBUF 			8

/**********************************************************************************************/

static epl_optical_sensor epl_sensor;

#define LUX_PER_COUNT			275 //ALS lux per count, 400/1000=0.4

typedef enum
{
    CMC_BIT_RAW   			= 0x0,
    CMC_BIT_PRE_COUNT     	= 0x1,
    CMC_BIT_DYN_INT			= 0x2,
} CMC_ALS_REPORT_TYPE;

/******************************************************************************
 *  PS_DYN_K
 ******************************************************************************/
#if PS_DYN_K
uint16_t dynk_min_ps_raw_data = 0xffff;
uint16_t dynk_max_ir_data;
uint32_t dynk_thd_low = 0;
uint32_t dynk_thd_high = 0;
uint16_t dynk_low_offset;
uint16_t dynk_high_offset;
#endif  /*PS_DYN_K*/
/******************************************************************************
 *  PS_FIRST_REPORT
 ******************************************************************************/
/******************************************************************************
 *  ALS_DYN_INTT
 ******************************************************************************/
#if ALS_DYN_INTT
//Dynamic INTT
int dynamic_intt_idx;
int dynamic_intt_init_idx = 0;	//initial dynamic_intt_idx
int c_gain;
int dynamic_intt_lux = 1;
uint16_t dynamic_intt_high_thr;
uint16_t dynamic_intt_low_thr;
uint32_t dynamic_intt_max_lux = 30000;
uint32_t dynamic_intt_min_lux = 0;
uint32_t dynamic_intt_min_unit = 1000;
static int als_dynamic_intt_intt[] = {EPL_ALS_INTT_1024, EPL_ALS_INTT_1024};
static int als_dynamic_intt_value[] = {1024, 1024}; //match als_dynamic_intt_intt table
static int als_dynamic_intt_gain[] = {EPL_GAIN_MID, EPL_GAIN_LOW};
static int als_dynamic_intt_high_thr[] = {30000, 65535};
static int als_dynamic_intt_low_thr[] = {0, 200};
static int als_dynamic_intt_intt_num =  sizeof(als_dynamic_intt_value)/sizeof(int);
#if ALS_DYN_INTT_NO_DELAY
uint16_t dynamic_intt_last_raw = 1;
#endif
#endif /*ALS_DYN_INTT*/

/******************************************************************************
 *  ALS Light source
 ******************************************************************************/
#if ALS_LSRC
int offset_gain;
int scale_gain;
uint32_t lsrc_als_offset = 0;
uint16_t lsrc_raw = 0;
uint16_t lsrc_lux = 0;
uint16_t als_lux = 0;
uint32_t lsrc_ratio = 0;
#if ALS_DYN_INTT
uint16_t als_dyn_intt_ch0;
#endif
#endif

/******************************************************************************
 *  REG_MONITOR
 ******************************************************************************/
#if REG_MONITOR
#define EPL_REG_TIMER_DELAY             200000000ULL
#endif /*REG_MONITOR*/

/**********************************************************************************************/
static struct eplTask {
    /* txBuf for i2c operation, fill register and fill value */
    bool alsPowerOn;
    bool psPowerOn;
    int polling_mode_ps;                               /*!< 1: polling mode ; 0:interrupt mode*/
    int polling_mode_als;                              /*!< 1: polling mode ; 0:interrupt mode(dont support)*/
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
	uint32_t als_raw_data;
    uint32_t prox_raw_data;
    int32_t psCali;
    uint32_t    ps_threshold_high;
    uint32_t    ps_threshold_low;
    uint32_t    als_threshold_high;
    uint32_t    als_threshold_low;
#if REG_MONITOR
    uint32_t regmonitorTimerHandle;
#endif
} mTask;


static struct eplTask *eplDebugPoint;
/*
static int check_timeout(portTickType *end_tick)
{
    return xTaskGetTickCount() > *end_tick ? 1 : 0;
}
*/
static void cal_end_time(unsigned int ms, portTickType *end_tick)
{
    *end_tick = xTaskGetTickCount() +
                (portTickType)(ms / (1000 / portTICK_RATE_MS));
}
/********************************************************************************************************/

static int epl_sensor_I2C_Write(I2cCallbackF i2cCallBack, void *next_state, uint8_t reg, uint8_t value)
{
    int ret = -1;

    mTask.txBuf[0] = reg;
    mTask.txBuf[1] = value;
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack, next_state);
    osLog(LOG_INFO,"[%s]: ret=%d \r\n", __func__, ret);
    return ret;
}

static int epl_sensor_I2C_Write_Block(I2cCallbackF i2cCallBack, void *next_state, uint8_t reg, uint8_t value[], uint8_t len)
{
    int ret = -1;
    int i = 0;

    mTask.txBuf[0] = reg;
    for(i=0; i<len; i++)
    {
        mTask.txBuf[i+1] = value[i];
    }
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, (len+1), i2cCallBack, next_state);
    osLog(LOG_INFO,"[%s]: ret=%d \r\n", __func__, ret);

    return ret;
}

static int epl_sensor_I2C_Read(I2cCallbackF i2cCallBack, void *next_state, uint8_t reg, uint8_t len)
{
    int ret = -1;
    mTask.txBuf[0] = reg;
    ret = i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1, mTask.rxBuf, len, i2cCallBack, next_state);
    osLog(LOG_INFO,"[%s]: ret=%d \r\n", __func__, ret);
    return ret;
}
//sensing time
static int als_sensing_time(int intt, int osr, int cycle)
{
    long sensing_us_time;
    int sensing_ms_time;
    int als_intt, als_osr, als_cycle;

    als_intt = als_intt_value[intt>>2];
    als_osr = osr_value[osr>>3];
    als_cycle = cycle_value[cycle];

    osLog(LOG_INFO,"ALS: INTT=%d, OSR=%d, Cycle=%d \r\n", als_intt, als_osr, als_cycle);

    sensing_us_time = (als_intt + als_osr*2*2) * 2 * als_cycle;
    sensing_ms_time = sensing_us_time / 1000;


    osLog(LOG_INFO,"[als_sensing_time]: sensing=%d ms \r\n", sensing_ms_time);

    return (sensing_ms_time + 5);
}

static int ps_sensing_time(int intt, int osr, int cycle)
{
    long sensing_us_time;
    int sensing_ms_time;
    int ps_intt, ps_osr, ps_cycle;

    ps_intt = ps_intt_value[intt>>2];
    ps_osr = osr_value[osr>>3];
    ps_cycle = cycle_value[cycle];

    osLog(LOG_INFO,"PS: INTT=%d, OSR=%d, Cycle=%d \r\n", ps_intt, ps_osr, ps_cycle);


    sensing_us_time = (ps_intt*3 + ps_osr*2*3) * ps_cycle;
    sensing_ms_time = sensing_us_time / 1000;

    osLog(LOG_INFO, "[ps_sensing_time]: sensing=%d ms\r\n", sensing_ms_time);

    return (sensing_ms_time + 5);
}

static int set_psensor_intr_threshold(I2cCallbackF i2cCallBack, void *next_state, uint16_t pilt, uint16_t piht)
{
    int ret = -1;
    uint8_t thd[4];

    thd[0] = (uint8_t) (pilt & 0x00ff);
    thd[1] = (uint8_t) (pilt >> 8);
    thd[2] = (uint8_t) (piht & 0x00ff);
    thd[3] = (uint8_t) (piht >> 8);

    ret = epl_sensor_I2C_Write_Block(i2cCallBack, next_state, DEVREG_PS_ILTL, thd, 4);

    osLog(LOG_INFO,"[%s]: low_thd = %d, high_thd = %d \r\n", __func__, pilt, piht);
    return ret;
}

static int set_lsensor_intr_threshold(I2cCallbackF i2cCallBack, void *next_state, uint16_t ailt, uint16_t aiht)
{
    int ret = -1;
    uint8_t thd[4];

    thd[0] = (uint8_t) (ailt & 0x00ff);
    thd[1] = (uint8_t) (ailt >> 8);
    thd[2] = (uint8_t) (aiht & 0x00ff);
    thd[3] = (uint8_t) (aiht >> 8);

    ret = epl_sensor_I2C_Write_Block(i2cCallBack, next_state, DEVREG_ALS_ILTL, thd, 4);

    osLog(LOG_INFO,"[%s]: low_thd = %d, high_thd = %d \r\n", __func__, ailt, aiht);
    return ret;
}

static void set_als_ps_intr_type(bool ps_polling, bool als_polling)
{
    //set als / ps interrupt control mode and trigger type
	switch((ps_polling << 1) | als_polling)
	{
		case 0: // ps and als interrupt
			epl_sensor.interrupt_control = 	EPL_INT_CTRL_ALS_OR_PS;
			epl_sensor.als.interrupt_type = EPL_INTTY_ACTIVE;
			epl_sensor.ps.interrupt_type = EPL_INTTY_ACTIVE;
		break;

		case 1: //ps interrupt and als polling
			epl_sensor.interrupt_control = 	EPL_INT_CTRL_PS;
			epl_sensor.als.interrupt_type = EPL_INTTY_DISABLE;
			epl_sensor.ps.interrupt_type = EPL_INTTY_ACTIVE;
		break;

		case 2: // ps polling and als interrupt
			epl_sensor.interrupt_control = 	EPL_INT_CTRL_ALS;
			epl_sensor.als.interrupt_type = EPL_INTTY_ACTIVE;
			epl_sensor.ps.interrupt_type = EPL_INTTY_DISABLE;
		break;

		case 3: //ps and als polling
			epl_sensor.interrupt_control = 	EPL_INT_CTRL_ALS_OR_PS;
			epl_sensor.als.interrupt_type = EPL_INTTY_DISABLE;
			epl_sensor.ps.interrupt_type = EPL_INTTY_DISABLE;
		break;
	}
}

#if REG_MONITOR
static int epl_sensor_reset_reg(void)
{
    int ret = -1;
    uint8_t buf[3];

    osLog(LOG_INFO, "[epl_sensor_reset_reg] \n" );
    ret = epl_sensor_I2C_Write(NULL, NULL, DEVREG_RESET, EPL_POWER_ON | EPL_RESETN_RUN);
    ret = epl_sensor_I2C_Write(NULL, NULL, DEVREG_PS_STATUS, (EPL_CMP_RESET | EPL_UN_LOCK));
    ret = epl_sensor_I2C_Write(NULL, NULL, DEVREG_PS_STATUS, (EPL_CMP_RUN | EPL_UN_LOCK));
    ret = epl_sensor_I2C_Write(NULL, NULL, DEVREG_ALS_STATUS, (EPL_CMP_RESET | EPL_UN_LOCK));
    ret = epl_sensor_I2C_Write(NULL, NULL, DEVREG_ALS_STATUS, (EPL_CMP_RUN | EPL_UN_LOCK));
    ret = epl_sensor_I2C_Write(NULL, NULL, DEVREG_RESET, EPL_POWER_OFF | EPL_RESETN_RESET);

    /*chip refrash*/
    ret = epl_sensor_I2C_Write(NULL, NULL, 0xfd, 0x8e);
    if(epl_sensor.revno == 0xa188)
    {
        ret = epl_sensor_I2C_Write(NULL, NULL, 0xfe, 0xa2);
        ret = epl_sensor_I2C_Write(NULL, NULL, 0xfe, 0x82);
    }
    else if(epl_sensor.revno == 0x0288)
    {
        ret = epl_sensor_I2C_Write(NULL, NULL, 0xfb, 0x21);
        ret = epl_sensor_I2C_Write(NULL, NULL, 0xfb, 0x20);
    }
    else
    {
        ret = epl_sensor_I2C_Write(NULL, NULL, 0xfe, 0x22);
        ret = epl_sensor_I2C_Write(NULL, NULL, 0xfe, 0x02);
    }
    ret = epl_sensor_I2C_Write(NULL, NULL, 0xfd, 0x00);
    if(epl_sensor.revno == 0x0288)
        ret = epl_sensor_I2C_Write(NULL, NULL, 0xfc, EPL_INTERNAL | EPL_NORMAL| EPL_GFIN_ENABLE | EPL_VOS_ENABLE | EPL_DOC_ON);
    else
        ret = epl_sensor_I2C_Write(NULL, NULL, 0xfc, EPL_A_D | EPL_NORMAL| EPL_GFIN_ENABLE | EPL_VOS_ENABLE | EPL_DOC_ON);

    ret = set_psensor_intr_threshold(NULL, NULL, epl_sensor.ps.low_threshold, epl_sensor.ps.high_threshold);
    ret = set_lsensor_intr_threshold(NULL, NULL, epl_sensor.als.low_threshold, epl_sensor.als.high_threshold);

    ret = epl_sensor_I2C_Write(NULL, NULL, DEVREG_ALS_STATUS, (epl_sensor.als.compare_reset | epl_sensor.als.lock));
    ret = epl_sensor_I2C_Write(NULL, NULL, DEVREG_PS_STATUS, (epl_sensor.ps.compare_reset | epl_sensor.ps.lock));

    ret = epl_sensor_I2C_Write(NULL, NULL, DEVREG_PS_OFSL, (uint8_t)(epl_sensor.ps.cancelation& 0xff));
    ret = epl_sensor_I2C_Write(NULL, NULL, DEVREG_PS_OFSH, (uint8_t)((epl_sensor.ps.cancelation & 0xff00) >> 8));

    //set als / ps interrupt control mode and trigger type
    set_als_ps_intr_type(epl_sensor.ps.polling_mode, epl_sensor.als.polling_mode);

    //write ALS config
    buf[0] = epl_sensor.als.als_intb_nonlos | epl_sensor.als.integration_time | epl_sensor.als.gain; //REG0x01
    buf[1] = epl_sensor.als.als_rs | epl_sensor.als.osr | epl_sensor.als.cycle; //REG0x02
    ret = epl_sensor_I2C_Write_Block(NULL, NULL, DEVREG_ALS_CONFIG, buf, 2);

    //write PS config
    buf[0] = epl_sensor.ps.ps_intb_nonlos | epl_sensor.ps.integration_time | epl_sensor.ps.gain; //REG0x03
    buf[1] = epl_sensor.ps.ps_rs | epl_sensor.ps.osr | epl_sensor.ps.cycle; //REG0x04
    ret = epl_sensor_I2C_Write_Block(NULL, NULL, DEVREG_PS_CONFIG, buf, 2);

    //write LED config
    buf[0] = epl_sensor.ps.ps_std | epl_sensor.ps.ir_on_control | epl_sensor.ps.ir_mode | epl_sensor.ps.ir_drive; //REG0x05
    buf[1] = epl_sensor.interrupt_control | epl_sensor.ps.persist |epl_sensor.ps.interrupt_type; //REG0x06
    buf[2] = epl_sensor.als.als_std | epl_sensor.als.interrupt_channel_select | epl_sensor.als.persist | epl_sensor.als.interrupt_type; //REG0x07
    ret = epl_sensor_I2C_Write_Block(NULL, NULL, DEVREG_LED_CONFIG, buf, 3);

    //set mode and wait
    ret = epl_sensor_I2C_Write(NULL, NULL, DEVREG_ENABLE, (epl_sensor.wait | epl_sensor.mode));
    osLog(LOG_INFO, "[epl_sensor_reset_reg]: ret = %d \n", ret);
    return ret;
}

static int epl_sensor_check_reg(uint8_t reg0x05, uint8_t reg0x06)
{
    osLog(LOG_INFO, "[epl_sensor_check_reg]: REG0x05=0x%x, REG0x06=0x%x \r\n", reg0x05, reg0x06);

    if(reg0x05!= (epl_sensor.ps.ir_on_control | epl_sensor.ps.ir_mode | epl_sensor.ps.ir_drive)
        || reg0x06 != (epl_sensor.interrupt_control | epl_sensor.ps.persist |epl_sensor.ps.interrupt_type))
    {
        osLog(LOG_INFO, "[epl_sensor_check_reg]: REG different..... REG0x05=0x%x, REG0x06 = 0x%x \r\n", reg0x05, reg0x06);
        osLog(LOG_INFO, "[epl_sensor_check_reg]: reset all setting \r\n");
#if PS_DYN_K
        epl_sensor.ps.low_threshold = dynk_thd_low;
        epl_sensor.ps.high_threshold = dynk_thd_high;
#endif
        //run epl_sensor_reset_reg
        epl_sensor_reset_reg();
        osLog(LOG_INFO, "[epl_sensor_check_reg]: PS CMP RESET/RUN \r\n");
        return -1;
    }
    return 0;
}

static void regTimerCallback(uint32_t timerId, void *cookie)
{
    alsPsInterruptOccur(); //read_reg, STATE_SAMPLE_PS
}

static void reg_check_Timer_start(bool on)
{
    if (on && !mTask.regmonitorTimerHandle)
    {
        mTask.regmonitorTimerHandle = timTimerSet(EPL_REG_TIMER_DELAY, 0, 50, regTimerCallback, NULL, false);
    }
    else if (!on && mTask.regmonitorTimerHandle)
    {
        timTimerCancel(mTask.regmonitorTimerHandle);
        mTask.regmonitorTimerHandle = 0;
    }
}
#endif /*REG_MONITOR*/

#if ALS_LSRC
static uint16_t lsrc_raw_convert_to_adc(uint32_t ch0, uint32_t ch1)
{
    uint32_t als_offset=0;
    uint16_t nor_raw=0;

    if(ch1 > 0)
    {
        lsrc_ratio = ch0*1000 / ch1;
    	als_offset = (ch0 * ch0) / (ch0+ch1) * offset_gain / 1000;
        osLog(LOG_INFO, "[lsrc_raw_convert_to_adc]: als_offset=%lu \r\n", als_offset);
    	if(als_offset < ( (1000-scale_gain)*ch1 / 1000) )
    		nor_raw = ch1 - als_offset;
    	else
    		nor_raw = ch1 * scale_gain / 1000;
    }
    else
    {
        nor_raw = ch1;
    }
    lsrc_raw = nor_raw;
    lsrc_als_offset = als_offset;

    osLog(LOG_INFO, "[lsrc_raw_convert_to_adc]: ch0=%lu, ch1=%lu, nor_raw=%u \r\n", ch0, ch1, nor_raw);

    return nor_raw;
}
#endif

#if ALS_DYN_INTT
static long raw_convert_to_lux(uint16_t raw_data)
{
    long lux = 0;
    long dyn_intt_raw = 0;
    int gain_value = 0;
#if ALS_LSRC
    uint16_t als_lsrc_raw = 0;
#endif
    if(epl_sensor.revno != 0x8188)  //3638
    {
#if ALS_LSRC
        als_dyn_intt_ch0 = epl_sensor.als.data.channels[0];
#endif
        dyn_intt_raw = raw_data;
    }
    else
    {
        if(epl_sensor.als.gain == EPL_GAIN_MID)
        {
            gain_value = 8;
        }
        else if (epl_sensor.als.gain == EPL_GAIN_LOW)
        {
            gain_value = 1;
        }
#if ALS_LSRC
        als_dyn_intt_ch0 = (epl_sensor.als.data.channels[0] * 10) / (10 * gain_value * als_dynamic_intt_value[dynamic_intt_idx] / als_dynamic_intt_value[1]); //float calculate
#endif
        dyn_intt_raw = (raw_data * 10) / (10 * gain_value * als_dynamic_intt_value[dynamic_intt_idx] / als_dynamic_intt_value[1]); //float calculate
    }

    osLog(LOG_INFO, "[raw_convert_to_lux]: dyn_intt_raw=%ld \r\n", dyn_intt_raw);
    if(dyn_intt_raw > 0xffff)
        epl_sensor.als.dyn_intt_raw = 0xffff;
    else
        epl_sensor.als.dyn_intt_raw = dyn_intt_raw;

#if ALS_LSRC
    als_lsrc_raw = lsrc_raw_convert_to_adc(als_dyn_intt_ch0, dyn_intt_raw);
    lux = c_gain * als_lsrc_raw;

    lsrc_lux = lux / 1000;
    als_lux = c_gain * dyn_intt_raw / 1000;

    osLog(LOG_INFO, "[raw_convert_to_lux]:raw_data=%d, als_lsrc_raw=%d\r\n", raw_data, als_lsrc_raw);
#else
    lux = c_gain * epl_sensor.als.dyn_intt_raw;
    osLog(LOG_INFO, "[raw_convert_to_lux]:raw_data=%d, epl_sensor.als.dyn_intt_raw=%d, \r\n", raw_data, epl_sensor.als.dyn_intt_raw);
#endif /*ALS_LSRC*/
    osLog(LOG_INFO, "[raw_convert_to_lux]:lux=%ld \r\n", lux);

    if(lux >= (dynamic_intt_max_lux*dynamic_intt_min_unit)){
        osLog(LOG_INFO, "[raw_convert_to_lux]:raw_convert_to_lux: change max lux\r\n");
        lux = dynamic_intt_max_lux * dynamic_intt_min_unit;
    }
    else if(lux <= (dynamic_intt_min_lux*dynamic_intt_min_unit)){
        osLog(LOG_INFO, "[raw_convert_to_lux]:raw_convert_to_lux: change min lux\r\n");
        lux = dynamic_intt_min_lux * dynamic_intt_min_unit;
    }

    return lux;
}
#endif

#if 0
static int epl_sensor_fast_update_mode(void)
{
    int ret = -1;
    uint8_t buf[2];
    int als_fast_time = als_sensing_time(epl_sensor.als.integration_time, epl_sensor.als.osr, EPL_CYCLE_1);

    osLog(LOG_INFO, "[epl_sensor_fast_update_mode] \r\n");
    ret = epl_sensor_I2C_Write(NULL, NULL, DEVREG_RESET, (EPL_POWER_OFF | EPL_RESETN_RESET) );

    buf[0] = epl_sensor.als.als_intb_nonlos | epl_sensor.als.integration_time | epl_sensor.als.gain;
#if ALS_DYN_INTT
    if(epl_sensor.revno != 0x8188)
    {
        if(dynamic_intt_idx == 0)
        {
            buf[1] = epl_sensor.als.als_rs | epl_sensor.als.osr | EPL_CYCLE_1;
        }
        else
        {
            buf[1] = EPL_RS_0 | epl_sensor.als.osr | EPL_CYCLE_1;
        }
    }
#endif
    else
    {
        buf[1] = epl_sensor.als.als_rs | epl_sensor.als.osr | EPL_CYCLE_1;
    }

    ret = epl_sensor_I2C_Write_Block(NULL, NULL, DEVREG_ALS_CONFIG, buf, 2);
    ret = epl_sensor_I2C_Write(NULL, NULL, DEVREG_ENABLE, (epl_sensor.wait | EPL_MODE_ALS));
    ret = epl_sensor_I2C_Write(NULL, NULL, DEVREG_RESET, (EPL_POWER_ON | EPL_RESETN_RUN) );

    osLog(LOG_INFO, "[%s]: %d \n", __func__, xTaskGetTickCount());
    //vTaskDelay(als_fast_time);
    osLog(LOG_INFO, "[%s]: %d, delay(%d ms) \n", __func__, xTaskGetTickCount(), als_fast_time);

    //revover setting
    buf[0] = epl_sensor.als.als_intb_nonlos | epl_sensor.als.integration_time | epl_sensor.als.gain;
#if ALS_DYN_INTT
    if(epl_sensor.revno != 0x8188)  //3638
    {
        if(dynamic_intt_idx == 0)
        {
            buf[1] = (epl_sensor.als.als_rs | epl_sensor.als.osr | epl_sensor.als.cycle);
        }
        else
        {
            buf[1] = (EPL_RS_0 | epl_sensor.als.osr | epl_sensor.als.cycle);
        }
    }
    else
#endif
    {
        buf[1] = (epl_sensor.als.als_rs | epl_sensor.als.osr | epl_sensor.als.cycle);
    }

    ret = epl_sensor_I2C_Write_Block(NULL, NULL, DEVREG_ALS_CONFIG, buf, 2);
    ret = epl_sensor_I2C_Write(NULL, NULL, DEVREG_RESET, (EPL_POWER_OFF | EPL_RESETN_RESET) );
    return ret;

}
#endif

static int epl_sensor_update_mode(I2cCallbackF i2cCallBack, void *next_state)
{


    int ret = -1;
    int als_time = als_sensing_time(epl_sensor.als.integration_time, epl_sensor.als.osr, epl_sensor.als.cycle);
    int ps_time = ps_sensing_time(epl_sensor.ps.integration_time, epl_sensor.ps.osr, epl_sensor.ps.cycle);
    osLog(LOG_INFO, "time: als =%d, ps=%d\n", als_time, ps_time);
    osLog(LOG_INFO, "mode selection =0x%x\n", (mTask.psPowerOn | (mTask.alsPowerOn << 1)));

    //**** mode selection ****
    switch((mTask.alsPowerOn << 1) | mTask.psPowerOn)
    {
        case 0: //disable all
            epl_sensor.mode = EPL_MODE_IDLE;
            break;

        case 1: //als = 0, ps = 1
            epl_sensor.mode = EPL_MODE_PS;
            break;
        case 2: //als = 1, ps = 0
            epl_sensor.mode = EPL_MODE_ALS;
            break;

        case 3: //als = 1, ps = 1
            epl_sensor.mode = EPL_MODE_ALS_PS;
            break;
    }
    ret = epl_sensor_I2C_Write(i2cCallBack, next_state, DEVREG_ENABLE, (epl_sensor.wait | epl_sensor.mode));
    return ret;

}

static uint16_t epl_sensor_get_als_value(I2cCallbackF i2cCallBack, uint16_t als_ch0, uint16_t als_ch1)
{
#if ALS_DYN_INTT
	long now_lux=0, lux_tmp=0;
    bool change_flag = false;
#endif
    switch(epl_sensor.als.report_type)
    {
        case CMC_BIT_RAW:
#if ALS_LSRC
            lsrc_lux = lsrc_raw_convert_to_adc(epl_sensor.als.data.channels[0], epl_sensor.als.data.channels[1]);
            return lsrc_lux;
#else
            return als_ch1;
#endif
        break;
        case CMC_BIT_PRE_COUNT:
#if ALS_LSRC
            lsrc_lux = (lsrc_raw_convert_to_adc(epl_sensor.als.data.channels[0], epl_sensor.als.data.channels[1]) * epl_sensor.als.factory.lux_per_count)/1000;
            als_lux = (als_ch1 * epl_sensor.als.factory.lux_per_count)/1000;
            return lsrc_lux;
#else
            return (als_ch1 * epl_sensor.als.factory.lux_per_count)/1000;
#endif
        break;
#if ALS_DYN_INTT
		case CMC_BIT_DYN_INT:

            osLog(LOG_INFO, "[epl_sensor_get_als_value]: als_dynamic_intt_value=%d, dynamic_intt_gain=%d \r\n", als_dynamic_intt_value[dynamic_intt_idx], als_dynamic_intt_gain[dynamic_intt_idx]);
            osLog(LOG_INFO, "[epl_sensor_get_als_value]: als=%d \r\n", als_ch1);
#if ALS_DYN_INTT_NO_DELAY
            osLog(LOG_INFO, "[epl_sensor_get_als_value]: dynamic_intt_last_raw=%u \r\n", dynamic_intt_last_raw);
            if( (dynamic_intt_last_raw != als_ch1) || ((dynamic_intt_idx==(als_dynamic_intt_intt_num - 1)) && (als_ch1==65535)) || (dynamic_intt_idx==0&&(als_ch1==0)) )
            {
                dynamic_intt_last_raw = als_ch1;
#endif
            if( als_ch1 > dynamic_intt_high_thr || ((dynamic_intt_idx == 0) && epl_sensor.als.saturation == EPL_SATURATION) )
        	{
          		if(dynamic_intt_idx == (als_dynamic_intt_intt_num - 1)){
                    //als = dynamic_intt_high_thr;
          		    lux_tmp = raw_convert_to_lux(als_ch1);
        	      	osLog(LOG_INFO, ">>>>>>>>>>>>>>>>>>>>>>>> INTT_MAX_LUX\r\n");
          		}
                else{
                    change_flag = true;
        			als_ch1  = dynamic_intt_high_thr;
              		lux_tmp = raw_convert_to_lux(als_ch1);
                    dynamic_intt_idx++;
                    if(dynamic_intt_idx >= (als_dynamic_intt_intt_num - 1))
                        dynamic_intt_idx = (als_dynamic_intt_intt_num - 1);
                    osLog(LOG_INFO, ">>>>>>>>>>>>>>>>>>>>>>>>change INTT high: %d, raw: %d \r\n", dynamic_intt_idx, als_ch1);
                }
            }
            else if(als_ch1 < dynamic_intt_low_thr)
            {
                if(dynamic_intt_idx == 0){
                    //als = dynamic_intt_low_thr;
                    lux_tmp = raw_convert_to_lux(als_ch1);
                    osLog(LOG_INFO, ">>>>>>>>>>>>>>>>>>>>>>>> INTT_MIN_LUX\r\n");
                }
                else{
                    change_flag = true;
        			als_ch1  = dynamic_intt_low_thr;
                	lux_tmp = raw_convert_to_lux(als_ch1);
                    dynamic_intt_idx--;
                    if(dynamic_intt_idx <= 0)
                        dynamic_intt_idx = 0;
                    osLog(LOG_INFO, ">>>>>>>>>>>>>>>>>>>>>>>>change INTT low: %d, raw: %d \r\n", dynamic_intt_idx, als_ch1);
                }
            }
            else
            {
            	lux_tmp = raw_convert_to_lux(als_ch1);
            }

            now_lux = lux_tmp;
            dynamic_intt_lux = now_lux/dynamic_intt_min_unit;

            if(change_flag == true)
            {
                uint8_t buf[2];
#if !ALS_DYN_INTT_NO_DELAY
                int als_time = als_sensing_time(epl_sensor.als.integration_time, epl_sensor.als.osr, epl_sensor.als.cycle);
#endif
                osLog(LOG_INFO, "[epl_sensor_get_als_value]: ALS_DYN_INTT:Chang Setting \r\n");
                epl_sensor.als.integration_time = als_dynamic_intt_intt[dynamic_intt_idx];
                epl_sensor.als.gain = als_dynamic_intt_gain[dynamic_intt_idx];
                dynamic_intt_high_thr = als_dynamic_intt_high_thr[dynamic_intt_idx];
                dynamic_intt_low_thr = als_dynamic_intt_low_thr[dynamic_intt_idx];

                //epl_sensor_fast_update_mode();
                epl_sensor_I2C_Write(NULL, NULL, DEVREG_RESET, (EPL_POWER_OFF | EPL_RESETN_RESET) );
                buf[0] = epl_sensor.als.als_intb_nonlos | epl_sensor.als.integration_time | epl_sensor.als.gain;
                if(dynamic_intt_idx == 0)
                {
                    buf[1] = (epl_sensor.als.als_rs | epl_sensor.als.osr | epl_sensor.als.cycle);
                }
                else
                {
                    buf[1] = (EPL_RS_0 | epl_sensor.als.osr | epl_sensor.als.cycle);
                }
                epl_sensor_I2C_Write_Block(NULL, NULL, DEVREG_ALS_CONFIG, buf, 2);
#if !ALS_DYN_INTT_NO_DELAY
                epl_sensor_I2C_Write(NULL, NULL, DEVREG_ENABLE, (epl_sensor.wait | EPL_MODE_ALS)); //REG0x00
                epl_sensor_I2C_Write(NULL, NULL, DEVREG_RESET, (EPL_POWER_ON | EPL_RESETN_RUN) );
                osLog(LOG_INFO, "[%s]: %d \n", __func__, xTaskGetTickCount());
                vTaskDelay(als_time);
                osLog(LOG_INFO, "[%s]: %d, delay(%d ms) \n", __func__, xTaskGetTickCount(), als_time);
                epl_sensor_I2C_Write(NULL, NULL, DEVREG_RESET, (EPL_POWER_OFF | EPL_RESETN_RESET) );
                epl_sensor_I2C_Write(NULL, NULL, DEVREG_ENABLE, (epl_sensor.wait | epl_sensor.mode)); //REG0x00
#endif
                epl_sensor_I2C_Write(NULL, NULL, DEVREG_RESET, (EPL_POWER_ON | EPL_RESETN_RUN) );
		    }
#if ALS_DYN_INTT_NO_DELAY
			}
#endif
                if(dynamic_intt_lux < 3)
                    return 0;
                else
                   return dynamic_intt_lux - 2;
		break;
#endif
    }

    return 0;
}

static int epl_sensor_enable_als(I2cCallbackF i2cCallBack, void *next_state, bool enable)
{
    int ret = -1;
    osLog(LOG_INFO, "[epl_sensor_enable_als]: als enable=%d ", enable);

    if(enable)
    {
        mTask.alsPowerOn = true;
#if ALS_DYN_INTT
        if(epl_sensor.als.report_type == CMC_BIT_DYN_INT)
        {
            dynamic_intt_idx = dynamic_intt_init_idx;
            epl_sensor.als.integration_time = als_dynamic_intt_intt[dynamic_intt_idx];
            epl_sensor.als.gain = als_dynamic_intt_gain[dynamic_intt_idx];
            dynamic_intt_high_thr = als_dynamic_intt_high_thr[dynamic_intt_idx];
            dynamic_intt_low_thr = als_dynamic_intt_low_thr[dynamic_intt_idx];
        }
#endif

    }
    else
    {
        mTask.alsPowerOn = false;
    }

    //if(mTask.alsPowerOn == true)
    //    ret = epl_sensor_fast_update_mode();
    ret = epl_sensor_update_mode(i2cCallBack, next_state);
    return ret;
}

static int epl_sensor_enable_ps(I2cCallbackF i2cCallBack, void *next_state, bool enable)
{
    int ret = -1;
    osLog(LOG_INFO, "[epl_sensor_enable_ps]: ps enable=%d", enable);

    if(enable)
    {
        mTask.psPowerOn = true;
#if REG_MONITOR
       epl_sensor_reset_reg();
#endif
#if PS_DYN_K
        dynk_min_ps_raw_data = 0xffff;
#endif
    }
    else
    {
        mTask.psPowerOn = false;
    }

    //if(mTask.alsPowerOn == true)
    //    ret = epl_sensor_fast_update_mode();
    ret = epl_sensor_update_mode(i2cCallBack, next_state);

#if REG_MONITOR
    reg_check_Timer_start(enable);
#endif
    return ret;
}

static void report_ps(I2cCallbackF i2cCallBack, uint8_t ps_state)
{
    if( ps_state == 1 )
    { /* 1 is far */
        mTask.data.prox_state = PROX_STATE_FAR; /* far */
        mTask.data.prox_data = 1; /* far state distance is 1cm */
    }
    else
    {
        mTask.data.prox_state = PROX_STATE_NEAR; /* near */
        mTask.data.prox_data = 0; /* near state distance is 0cm */
    }
    mTask.prox_raw_data = epl_sensor.ps.data.data;
    osLog(LOG_INFO, "[%s]: =====> prox_state=%d, ps_raw_data:%lx\n", __func__, mTask.data.prox_state, mTask.prox_raw_data);
    mt_eint_unmask(mTask.hw->eint_num); /* MT6797 use 11 */

	mTask.data.sensType = SENS_TYPE_PROX;
    txTransferDataInfo(&mTask.dataInfo, 1, &mTask.data);
}

static void report_als(I2cCallbackF i2cCallBack, uint16_t ch0, uint16_t ch1)
{
    mTask.als_raw_data = ch1;
    mTask.data.als_data = epl_sensor_get_als_value(i2cCallBack, ch0, ch1);

	mTask.data.sensType = SENS_TYPE_ALS;
    txTransferDataInfo(&mTask.dataInfo, 1, &mTask.data);
}

static int epl_sensor_set_als_power_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = -1;
    osLog(LOG_INFO, "[%s]", __func__);

    ret = epl_sensor_enable_als(i2cCallBack, next_state, true);

    return ret;
}

static int epl_sensor_set_als_power_off(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = -1;
    osLog(LOG_INFO, "[%s]", __func__);

    ret = epl_sensor_enable_als(i2cCallBack, next_state, false);

    return ret;
}

static int epl_sensor_set_ps_power_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = -1;
	osLog(LOG_INFO, "[%s]", __func__);

    ret = epl_sensor_enable_ps(i2cCallBack, next_state, true);
    return ret;
}

static int epl_sensor_set_ps_power_off(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = -1;
	osLog(LOG_INFO, "[%s]", __func__);

    ret = epl_sensor_enable_ps(i2cCallBack, next_state, false);
    return ret;
}

static int epl_sensor_detect_pwr(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = -1;
	osLog(LOG_INFO, "[%s]", __func__);
    if(epl_sensor.mode == EPL_MODE_IDLE)
        ret = epl_sensor_I2C_Write(i2cCallBack, next_state, DEVREG_RESET, (EPL_POWER_OFF | EPL_RESETN_RESET));
    else
        ret = epl_sensor_I2C_Write(i2cCallBack, next_state, DEVREG_RESET, (EPL_POWER_ON | EPL_RESETN_RUN));
    return ret;
}

static int epl_sensor_run_next_state(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}


static int epl_sensor_set_als_debounce_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                      void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                      void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{

    mTask.als_debounce_on = 1;
    cal_end_time(mTask.als_debounce_time, &mTask.als_debounce_end);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int epl_sensor_set_ps_debounce_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                      void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                      void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{

    mTask.ps_debounce_on = 1;
    cal_end_time(mTask.ps_debounce_time, &mTask.ps_debounce_end);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

//read als
static int epl_sensor_read_als_data(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = -1;
    if (rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize))
    {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "epl_sensor_read_als_data, rx dataInfo error\n");
        return -1;
    }
    ret = epl_sensor_I2C_Read(i2cCallBack, next_state, DEVREG_ALS_STATUS, 5);
    return ret;
}

static int epl_sensor_read_als(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    epl_sensor.als.saturation = (mTask.rxBuf[0] & 0x20);
    epl_sensor.als.compare_high = (mTask.rxBuf[0] & 0x10);
    epl_sensor.als.compare_low = (mTask.rxBuf[0] & 0x08);
    epl_sensor.als.interrupt_flag = (mTask.rxBuf[0] & 0x04);
    epl_sensor.als.compare_reset = (mTask.rxBuf[0] & 0x02);
    epl_sensor.als.lock = (mTask.rxBuf[0] & 0x01);
    epl_sensor.als.data.channels[0] = ((mTask.rxBuf[2]<<8) | mTask.rxBuf[1]);
    epl_sensor.als.data.channels[1] = ((mTask.rxBuf[4]<<8) | mTask.rxBuf[3]);

	osLog(LOG_INFO, "als: ~~~~ ALS ~~~~~ \n");
	osLog(LOG_INFO, "als: buf = 0x%x\n", mTask.rxBuf[0]);
	osLog(LOG_INFO, "als: sat = 0x%x\n", epl_sensor.als.saturation);
	osLog(LOG_INFO, "als: cmp h = 0x%x, l = %d\n", epl_sensor.als.compare_high, epl_sensor.als.compare_low);
	osLog(LOG_INFO, "als: int_flag = 0x%x\n",epl_sensor.als.interrupt_flag);
	osLog(LOG_INFO, "als: cmp_rstn = 0x%x, lock = 0x%x\n", epl_sensor.als.compare_reset, epl_sensor.als.lock);
    osLog(LOG_INFO, "read als channel 0 = %d\n", epl_sensor.als.data.channels[0]);
    osLog(LOG_INFO, "read als channel 1 = %d\n", epl_sensor.als.data.channels[1]);

    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int epl_sensor_report_als(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                           void *inBuf, uint8_t inSize, uint8_t elemInSize,
                           void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    report_als(i2cCallBack, epl_sensor.als.data.channels[0], epl_sensor.als.data.channels[1]);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

#if PS_DYN_K
static int epl_sensor_do_ps_auto_k(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                    void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = -1;
    osLog(LOG_INFO, "[epl_sensor_do_ps_auto_k]:dynk_min_ps_raw_data = %d", dynk_min_ps_raw_data);
    if( (dynk_min_ps_raw_data > epl_sensor.ps.data.data)
#if PS_FIRST_REPORT
        && (epl_sensor.ps.data.data < mTask.hw->ps_max_ct)
#endif
        && (epl_sensor.ps.saturation == 0) && (epl_sensor.ps.data.ir_data < dynk_max_ir_data) )
    {
        dynk_min_ps_raw_data = epl_sensor.ps.data.data;

        dynk_thd_low = dynk_min_ps_raw_data + dynk_low_offset;
	    dynk_thd_high = dynk_min_ps_raw_data + dynk_high_offset;

	    if(dynk_thd_low>65534)
            dynk_thd_low = 65534;
        if(dynk_thd_high>65535)
            dynk_thd_high = 65535;

	    ret = set_psensor_intr_threshold(i2cCallBack, next_state, (uint16_t)dynk_thd_low, (uint16_t)dynk_thd_high);
	    osLog(LOG_INFO, "[epl_sensor_do_ps_auto_k]:dyn ps raw = %d, min = %d, ir_data = %d\n", epl_sensor.ps.data.data, dynk_min_ps_raw_data, epl_sensor.ps.data.ir_data);
	    osLog(LOG_INFO, "[epl_sensor_do_ps_auto_k]:dyn k thre_l = %d, thre_h = %d\n", (uint16_t)dynk_thd_low, (uint16_t)dynk_thd_high);
    }
    else
    {
        ret = 0;
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    }

    return ret;
}
#endif

#if PS_FIRST_REPORT
static int epl_sensor_en_set_max_thd(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = -1;
    ret = set_psensor_intr_threshold(i2cCallBack, next_state, mTask.hw->ps_max_ct, mTask.hw->ps_max_ct+1);
    return ret;
}
#endif

#if REG_MONITOR
//read reg
static int epl_sensor_read_reg_data(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = -1;
    if (rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize))
    {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "epl_sensor_read_als_data, rx dataInfo error\n");
        return -1;
    }
    ret = epl_sensor_I2C_Read(i2cCallBack, next_state, DEVREG_LED_CONFIG, 2);
    return ret;
}
static int epl_sensor_monitor_reg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    if(epl_sensor_check_reg(mTask.rxBuf[0], mTask.rxBuf[1]) == -1)
    {
        next_state = (void*)STATE_IDEL;  //if happend, dont report
    }
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}
#endif
//read ps
static int epl_sensor_read_ps_data(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = -1;
#if !REG_MONITOR
    if (rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize))
    {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "epl_sensor_read_als_data, rx dataInfo error\n");
        return -1;
    }
#endif
    ret = epl_sensor_I2C_Read(i2cCallBack, next_state, DEVREG_PS_STATUS, 5);
    return ret;
}

static int epl_sensor_read_ps(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    epl_sensor.ps.saturation = (mTask.rxBuf[0] & 0x20);
    epl_sensor.ps.compare_high = (mTask.rxBuf[0] & 0x10);
    epl_sensor.ps.compare_low = (mTask.rxBuf[0] & 0x08);
    epl_sensor.ps.interrupt_flag = (mTask.rxBuf[0] & 0x04);
    epl_sensor.ps.compare_reset = (mTask.rxBuf[0] & 0x02);
    epl_sensor.ps.lock = (mTask.rxBuf[0] & 0x01);
    epl_sensor.ps.data.ir_data = ((mTask.rxBuf[2]<<8) | mTask.rxBuf[1]);
    epl_sensor.ps.data.data = ((mTask.rxBuf[4]<<8) | mTask.rxBuf[3]);

    osLog(LOG_INFO, "ps: ~~~~ PS ~~~~~ \n");
	osLog(LOG_INFO, "ps: buf = 0x%x\n", mTask.rxBuf[0]);
	if(epl_sensor.revno != 0x8188)
	    osLog(LOG_INFO, "ps: sat_1 = 0x%x\n", epl_sensor.ps.saturation_1);
	osLog(LOG_INFO, "ps: sat = 0x%x\n", epl_sensor.ps.saturation);
	osLog(LOG_INFO, "ps: cmp h = 0x%x, l = 0x%x\n", epl_sensor.ps.compare_high, epl_sensor.ps.compare_low);
	osLog(LOG_INFO, "ps: int_flag = 0x%x\n",epl_sensor.ps.interrupt_flag);
	osLog(LOG_INFO, "ps: cmp_rstn = 0x%x, lock = 0x%x\n", epl_sensor.ps.compare_reset, epl_sensor.ps.lock);
	osLog(LOG_INFO, "[%s]: data = %d\n", __func__, epl_sensor.ps.data.data);
	osLog(LOG_INFO, "[%s]: ir data = %d\n", __func__, epl_sensor.ps.data.ir_data);

    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int epl_sensor_ps_cmp_unlock(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                           void *inBuf, uint8_t inSize, uint8_t elemInSize,
                           void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{

    if(epl_sensor.ps.interrupt_flag == EPL_INT_TRIGGER)
    {
        int ret = -1;
        //interrupt mode report
        ret = epl_sensor_I2C_Write(i2cCallBack, next_state, DEVREG_PS_STATUS, (EPL_CMP_RUN | EPL_UN_LOCK));
        return ret;
    }
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int epl_sensor_report_ps(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                           void *inBuf, uint8_t inSize, uint8_t elemInSize,
                           void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    report_ps(i2cCallBack, (epl_sensor.ps.compare_low >> 3));
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}
//write_global_variable
static int epl_sensor_pwr_on(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = -1;
    ret = epl_sensor_I2C_Write(i2cCallBack, next_state, DEVREG_RESET, (EPL_POWER_ON | EPL_RESETN_RUN));
    return ret;
}

static int epl_sensor_pwr_off(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = -1;
    ret = epl_sensor_I2C_Write(i2cCallBack, next_state, DEVREG_RESET, (EPL_POWER_OFF | EPL_RESETN_RESET));
    return ret;

}

static int epl_sensor_ps_cmp_rst(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = -1;
    ret = epl_sensor_I2C_Write(i2cCallBack, next_state, DEVREG_PS_STATUS, (EPL_CMP_RESET | EPL_UN_LOCK));
    return ret;
}

static int epl_sensor_ps_cmp_run(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = -1;
    ret = epl_sensor_I2C_Write(i2cCallBack, next_state, DEVREG_PS_STATUS, (EPL_CMP_RUN | EPL_UN_LOCK));
    return ret;
}

static int epl_sensor_als_cmp_rst(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = -1;
    ret = epl_sensor_I2C_Write(i2cCallBack, next_state, DEVREG_ALS_STATUS, (EPL_CMP_RESET | EPL_UN_LOCK));
    return ret;
}
static int epl_sensor_als_cmp_run(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = -1;
    ret = epl_sensor_I2C_Write(i2cCallBack, next_state, DEVREG_ALS_STATUS, (EPL_CMP_RUN | EPL_UN_LOCK));
    return ret;
}

static int epl_sensor_chip_unkey(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = -1;
    ret = epl_sensor_I2C_Write(i2cCallBack, next_state, 0xfd, 0x8e);
    return ret;
}

static int epl_sensor_chip_key(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = -1;
    ret = epl_sensor_I2C_Write(i2cCallBack, next_state, 0xfd, 0x00);
    return ret;
}

static int epl_sensor_chip_refresh0(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = -1;
    if(epl_sensor.revno == 0xa188)
    {
        ret = epl_sensor_I2C_Write(i2cCallBack, next_state, 0xfe, 0xa2);
    }
    else if(epl_sensor.revno == 0x0288)
    {
        ret = epl_sensor_I2C_Write(i2cCallBack, next_state, 0xfb, 0x21);
    }
    else
    {
        ret = epl_sensor_I2C_Write(i2cCallBack, next_state, 0xfe, 0x22);
    }
    return ret;
}

static int epl_sensor_chip_refresh1(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = -1;
    if(epl_sensor.revno == 0xa188)
    {
        ret = epl_sensor_I2C_Write(i2cCallBack, next_state, 0xfe, 0x82);
    }
    else if(epl_sensor.revno == 0x0288)
    {
        ret = epl_sensor_I2C_Write(i2cCallBack, next_state, 0xfb, 0x20);
    }
    else
    {
        ret = epl_sensor_I2C_Write(i2cCallBack, next_state, 0xfe, 0x02);
    }
    return ret;
}

static int epl_sensor_chip_doc(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = -1;
    if(epl_sensor.revno == 0x0288)
    {
        ret = epl_sensor_I2C_Write(i2cCallBack, next_state, 0xfc, (EPL_INTERNAL | EPL_NORMAL| EPL_GFIN_ENABLE | EPL_VOS_ENABLE | EPL_DOC_ON));
    }
    else
    {
        ret = epl_sensor_I2C_Write(i2cCallBack, next_state, 0xfc, (EPL_A_D | EPL_NORMAL| EPL_GFIN_ENABLE | EPL_VOS_ENABLE | EPL_DOC_ON));
    }
    return ret;
}

static int epl_sensor_ps_offset(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = -1;
    uint8_t buf[2];

    buf[0] = (uint8_t)(epl_sensor.ps.cancelation& 0xff);
    buf[1] = (epl_sensor.ps.cancelation & 0xff00) >> 8;

    ret = epl_sensor_I2C_Write_Block(i2cCallBack, next_state, DEVREG_PS_OFSL, buf, 2);

    return ret;
}

static int epl_sensor_ps_thd(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = -1;
    ret = set_psensor_intr_threshold(i2cCallBack, next_state, epl_sensor.ps.low_threshold, epl_sensor.ps.high_threshold);

    return ret;
}

static int epl_sensor_als_thd(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = -1;
    ret = set_lsensor_intr_threshold(i2cCallBack, next_state, epl_sensor.als.low_threshold, epl_sensor.als.high_threshold);

    return ret;
}

static int write_als_conf(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = -1;
    uint8_t buf[2];

    set_als_ps_intr_type(epl_sensor.ps.polling_mode, epl_sensor.als.polling_mode);

    buf[0] = epl_sensor.als.als_intb_nonlos | epl_sensor.als.integration_time | epl_sensor.als.gain; //REG0x01
    buf[1] = epl_sensor.als.als_rs | epl_sensor.als.osr | epl_sensor.als.cycle; //REG0x02
    ret = epl_sensor_I2C_Write_Block(i2cCallBack, next_state, DEVREG_ALS_CONFIG, buf, 2);

    return ret;
}

static int write_ps_conf(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = -1;
    uint8_t buf[2];

    buf[0] = epl_sensor.ps.ps_intb_nonlos | epl_sensor.ps.integration_time | epl_sensor.ps.gain; //REG0x03
    buf[1] = epl_sensor.ps.ps_rs | epl_sensor.ps.osr | epl_sensor.ps.cycle; //REG0x04
    ret = epl_sensor_I2C_Write_Block(i2cCallBack, next_state, DEVREG_PS_CONFIG, buf, 2);

    return ret;
}

static int write_led_conf(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = -1;
    uint8_t buf[3];

    buf[0] = epl_sensor.ps.ps_std | epl_sensor.ps.ir_on_control | epl_sensor.ps.ir_mode | epl_sensor.ps.ir_drive; //REG0x05
    buf[1] = epl_sensor.interrupt_control | epl_sensor.ps.persist |epl_sensor.ps.interrupt_type; //REG0x06
    buf[2] = epl_sensor.als.als_std | epl_sensor.als.interrupt_channel_select | epl_sensor.als.persist | epl_sensor.als.interrupt_type; //REG0x07
    ret = epl_sensor_I2C_Write_Block(i2cCallBack, next_state, DEVREG_LED_CONFIG, buf, 3);

    return ret;
}
#if 0
static int write_mode(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = -1;
    ret = epl_sensor_I2C_Write(i2cCallBack, next_state, DEVREG_ENABLE, (epl_sensor.wait | epl_sensor.mode)); //REG0x00
    return ret;
}
#endif
static int epl_sensor_read_renvo(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = -1;

    ret = epl_sensor_I2C_Read(i2cCallBack, next_state, DEVREG_REV_ID, 2);
    return ret;
}
//initial_global_variable
static int initial_global_variable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
#if ALS_DYN_INTT
    int idx=0, gain_value=0, intt_value=0, total_value=0;
#endif
    //epl_sensor_read_renvo
    epl_sensor.revno = (uint16_t) mTask.rxBuf[0] | (mTask.rxBuf[1]<<8);

    //general setting
    epl_sensor.power = EPL_POWER_ON;
    epl_sensor.reset = EPL_RESETN_RUN;
    epl_sensor.mode = EPL_MODE_IDLE;
    epl_sensor.osc_sel = EPL_OSC_SEL_1MHZ;

    //als setting
    epl_sensor.als.polling_mode = mTask.polling_mode_als;
    epl_sensor.als.osr = EPL_PSALS_OSR_11;
    epl_sensor.als.als_rs = EPL_RS_0;
    epl_sensor.als.als_intb_nonlos = EPL_T_DIS;
    epl_sensor.als.interrupt_channel_select = EPL_ALS_INT_CHSEL_1;
    epl_sensor.als.als_std = EPL_ALS_PRE;
    epl_sensor.als.high_threshold = mTask.als_threshold_high;
    epl_sensor.als.low_threshold = mTask.als_threshold_low;
#if ALS_DYN_INTT
    if(epl_sensor.als.report_type == CMC_BIT_DYN_INT)
    {
        dynamic_intt_idx = dynamic_intt_init_idx;
        epl_sensor.als.integration_time = als_dynamic_intt_intt[dynamic_intt_idx];
        epl_sensor.als.gain = als_dynamic_intt_gain[dynamic_intt_idx];
        dynamic_intt_high_thr = als_dynamic_intt_high_thr[dynamic_intt_idx];
        dynamic_intt_low_thr = als_dynamic_intt_low_thr[dynamic_intt_idx];

        c_gain = epl_sensor.als.factory.lux_per_count; // 400/1000=0.4 /*Lux per count*/

        if(epl_sensor.revno != 0x8188)  //3638
        {
            if(als_dynamic_intt_gain[0] == EPL_GAIN_MID)
            {
                gain_value = 8;
            }
            else if (als_dynamic_intt_gain[0] == EPL_GAIN_LOW)
            {
                gain_value = 1;
            }

            intt_value = als_dynamic_intt_value[0] / als_dynamic_intt_value[1];
            total_value = gain_value * intt_value;

            for(idx = 0; idx < rs_num;  idx++)
        	{
        	    if(total_value < als_rs_value[idx])
        	    {
        	        break;
        	    }
        	}
        	osLog(LOG_INFO, "[initial_global_variable]: idx=%d, als_rs_value=%d, total_value=%d\r\n", idx, als_rs_value[idx-1], total_value);
            epl_sensor.als.als_rs = ((idx-1)<<5);
            als_dynamic_intt_high_thr[0] = als_dynamic_intt_high_thr[0]/total_value;
            als_dynamic_intt_low_thr[0] = als_dynamic_intt_low_thr[0]/total_value;
            osLog(LOG_INFO, "[initial_global_variable]: als_dynamic_intt_thr[0]=%d,%d\r\n", als_dynamic_intt_low_thr[0], als_dynamic_intt_high_thr[0]);
        }
    }
#endif

#if ALS_LSRC
#endif

    //ps setting
    epl_sensor.ps.polling_mode = mTask.polling_mode_ps;
    epl_sensor.ps.ps_rs = EPL_RS_0;
    epl_sensor.ps.osr = EPL_PSALS_OSR_11;
    epl_sensor.ps.ps_intb_nonlos = EPL_T_DIS;

    epl_sensor.ps.ps_std = EPL_PS_PRE;
    epl_sensor.ps.ir_on_control = EPL_IR_ON_CTRL_ON;
    epl_sensor.ps.ir_mode = EPL_IR_MODE_CURRENT;

    epl_sensor.ps.high_threshold = mTask.ps_threshold_high;
    epl_sensor.ps.low_threshold = mTask.ps_threshold_low;
#if PS_DYN_K
    dynk_max_ir_data = 50000; // ps max ch0
#endif /*PS_DYN_K*/

    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}


static int epl_sensor_als_ratechg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                              void *inBuf, uint8_t inSize, uint8_t elemInSize,
                              void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int epl_sensor_ps_ratechg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

/**********************************************************************************************************************************/
static void psGetCalibration(int32_t *cali, int32_t size)
{
    cali[0] = mTask.psCali;
}

static void psSetCalibration(int32_t *cali, int32_t size)
{
    mTask.psCali = cali[0];
    //set_psensor_intr_threshold(NULL, NULL, epl_sensor.ps.low_threshold + mTask.psCali, epl_sensor.ps.high_threshold + mTask.psCali); //test
}

static void psGetThreshold(uint32_t *threshold_high, uint32_t *threshold_low)
{
    *threshold_high = epl_sensor.ps.high_threshold;
    *threshold_low = epl_sensor.ps.low_threshold;
    osLog(LOG_ERROR, "%s ==>threshold_high:%ld threshold_low:%ld \n", __func__, *threshold_high, *threshold_low);
}

static void psSetThreshold(uint32_t threshold_high, uint32_t threshold_low)
{
    osLog(LOG_ERROR, "%s ==>threshold_high:%ld threshold_low:%ld \n", __func__, threshold_high, threshold_low);
    epl_sensor.ps.high_threshold = threshold_high;
    epl_sensor.ps.low_threshold = threshold_low;
    //set_psensor_intr_threshold(NULL, NULL, epl_sensor.ps.low_threshold, epl_sensor.ps.high_threshold); //test
}

static void psGetData(void *sample)
{
    char txBuf[1];
    static char rxBuf[5];
    struct TripleAxisDataPoint *tripleSample = (struct TripleAxisDataPoint *)sample;
    tripleSample->ix = mTask.prox_raw_data;
    tripleSample->iy = (mTask.data.prox_state == PROX_STATE_NEAR ? 0 : 1);

    osLog(LOG_INFO, "%s ==> ps_raw_data:%lx\n", __func__, mTask.prox_raw_data);
    void get_ps_data(void *cookie, size_t tx, size_t rx, int err)
    {
        char *rxBuf = cookie;
        if (err == 0)
        {
            epl_sensor.ps.data.data = ((rxBuf[1]<<8) | rxBuf[0]);
            osLog(LOG_INFO, "%s ==> epl_sensor.ps.data.data:%d\n", __func__, epl_sensor.ps.data.data);

            mTask.prox_raw_data = epl_sensor.ps.data.data;
            if (mTask.prox_raw_data < mTask.psCali)
                mTask.prox_raw_data = 0;
            mTask.prox_raw_data -= mTask.psCali;
        } else
            osLog(LOG_INFO, "[%s]: read ps data i2c error (%d)\n", __func__, err);
    }
    txBuf[0] = DEVREG_PS_RDATAL;
    i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, txBuf, 1, rxBuf, 3, get_ps_data, rxBuf);
}

static void alsGetData(void *sample)
{
    struct SingleAxisDataPoint *singleSample = (struct SingleAxisDataPoint *)sample;
    singleSample->idata = epl_sensor_get_als_value(NULL, epl_sensor.als.data.channels[0], epl_sensor.als.data.channels[1]);
}

static void epl_sensor_eint_handler(int arg)
{
    alsPsInterruptOccur();
}

static int epl_sensor_setup_eint(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                             void *inBuf, uint8_t inSize, uint8_t elemInSize,
                             void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    mt_eint_dis_hw_debounce(mTask.hw->eint_num);
    mt_eint_registration(mTask.hw->eint_num, LEVEL_SENSITIVE, LOW_LEVEL_TRIGGER, epl_sensor_eint_handler, EINT_INT_UNMASK, EINT_INT_AUTO_UNMASK_OFF);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static void psGetSensorInfo(struct sensorInfo_t *data)
{
    strncpy(data->name, "MN66233_P", sizeof(data->name));
}

static void alsGetSensorInfo(struct sensorInfo_t *data)
{
    strncpy(data->name, "MN66233_L", sizeof(data->name));
}

static int epl_sensor_register_core(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    struct sensorCoreInfo mInfo;
    memset(&mInfo, 0x00, sizeof(struct sensorCoreInfo));
    /* Register sensor Core */
    mInfo.sensType = SENS_TYPE_ALS;
    ///mInfo.gain = GRAVITY_EARTH_1000;
    ///mInfo.cvt = mTask.cvt;
    ///mInfo.getCalibration = accGetCalibration;
    ///mInfo.setCalibration = accSetCalibration;
    mInfo.getData = alsGetData;
    mInfo.getSensorInfo = alsGetSensorInfo;
    sensorCoreRegister(&mInfo);

    memset(&mInfo,0x00,sizeof(struct sensorCoreInfo));
    mInfo.sensType = SENS_TYPE_PROX,
    mInfo.gain = 1;
    mInfo.sensitivity = 1;
    sensorDriverGetConvert(0, &mInfo.cvt);
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

/********************************************************************************************************/

static struct sensorFsm eplFsm[] =
{
    /* sample als */
    sensorFsmCmd(STATE_SAMPLE_ALS, STATE_GET_ALS_DATA, epl_sensor_read_als_data),
    sensorFsmCmd(STATE_GET_ALS_DATA, STATE_REPORT_ALS, epl_sensor_read_als),
    sensorFsmCmd(STATE_REPORT_ALS, STATE_SAMPLE_ALS_DONE, epl_sensor_report_als),
    /* sample ps */
#if REG_MONITOR
    sensorFsmCmd(STATE_SAMPLE_PS, STATE_GET_REG_DATA, epl_sensor_read_reg_data),
    sensorFsmCmd(STATE_GET_REG_DATA, STATE_CHECK_REG, epl_sensor_monitor_reg),
    sensorFsmCmd(STATE_CHECK_REG, STATE_GET_PS_DATA, epl_sensor_read_ps_data),
#else
    sensorFsmCmd(STATE_SAMPLE_PS, STATE_GET_PS_DATA, epl_sensor_read_ps_data),
#endif
#if PS_KEEP_LOW_PATCH
    sensorFsmCmd(STATE_GET_PS_DATA, STATE_GET_PS_PWR_ON, epl_sensor_read_ps),
    sensorFsmCmd(STATE_GET_PS_PWR_ON, STATE_SET_PS_CMP_RUN, epl_sensor_pwr_on),
#else
    sensorFsmCmd(STATE_GET_PS_DATA, STATE_SET_PS_CMP_RUN, epl_sensor_read_ps),
#endif
#if PS_DYN_K
    sensorFsmCmd(STATE_SET_PS_CMP_RUN, STATE_PS_DO_CIAL, epl_sensor_ps_cmp_unlock),
    sensorFsmCmd(STATE_PS_DO_CIAL, STATE_REPORT_PS, epl_sensor_do_ps_auto_k),
#else
    sensorFsmCmd(STATE_SET_PS_CMP_RUN, STATE_REPORT_PS, epl_sensor_ps_cmp_unlock),
#endif
    sensorFsmCmd(STATE_REPORT_PS, STATE_SAMPLE_PS_DONE, epl_sensor_report_ps),

    /* als cali state */
    sensorFsmCmd(STATE_ALS_CALI, STATE_ALS_CALI_DONE, epl_sensor_run_next_state),
    /* als cfg state */
    sensorFsmCmd(STATE_ALS_CFG, STATE_ALS_CFG_DONE, epl_sensor_run_next_state),
     /* ps cali state */
    sensorFsmCmd(STATE_PS_CALI, STATE_PS_CALI_DONE, epl_sensor_run_next_state),
    /* ps cfg state */
    sensorFsmCmd(STATE_PS_CFG, STATE_PS_CFG_DONE, epl_sensor_run_next_state),

    /* als enable state */
    sensorFsmCmd(STATE_ALS_ENABLE, STATE_ALS_MODE, epl_sensor_pwr_off),
    sensorFsmCmd(STATE_ALS_MODE, STATE_ALS_CHIP_ON, epl_sensor_set_als_power_on),
    sensorFsmCmd(STATE_ALS_CHIP_ON, STATE_ALS_SET_DEBOUNCE, epl_sensor_pwr_on),
    sensorFsmCmd(STATE_ALS_SET_DEBOUNCE, STATE_ALS_ENABLE_DONE, epl_sensor_set_als_debounce_on),
    /* als disable state */
    sensorFsmCmd(STATE_ALS_DISABLE, STATE_ALS_POWER_OFF, epl_sensor_pwr_off),
    sensorFsmCmd(STATE_ALS_POWER_OFF, STATE_ALS_CHIP_OFF, epl_sensor_set_als_power_off),
    sensorFsmCmd(STATE_ALS_CHIP_OFF, STATE_ALS_DISABLE_DONE, epl_sensor_detect_pwr),

    /* ps enable state */
    sensorFsmCmd(STATE_PS_ENABLE, STATE_PS_MODE, epl_sensor_pwr_off),
    sensorFsmCmd(STATE_PS_MODE, STATE_PS_CHIP_ON, epl_sensor_set_ps_power_on),
#if PS_FIRST_REPORT
    sensorFsmCmd(STATE_PS_CHIP_ON, STATE_PS_EN_SET_MAX_THD, epl_sensor_pwr_on),
    sensorFsmCmd(STATE_PS_EN_SET_MAX_THD, STATE_PS_EN_CMP_RST, epl_sensor_en_set_max_thd),
    sensorFsmCmd(STATE_PS_EN_CMP_RST, STATE_PS_EN_CMP_RUN, epl_sensor_ps_cmp_rst),
    sensorFsmCmd(STATE_PS_EN_CMP_RUN, STATE_PS_SET_DEBOUNCE, epl_sensor_ps_cmp_run),
#else
    sensorFsmCmd(STATE_PS_CHIP_ON, STATE_PS_SET_DEBOUNCE, epl_sensor_pwr_on),
#endif
    sensorFsmCmd(STATE_PS_SET_DEBOUNCE, STATE_PS_ENABLE_DONE, epl_sensor_set_ps_debounce_on),
    /* ps disable state */
    sensorFsmCmd(STATE_PS_DISABLE, STATE_PS_POWER_OFF, epl_sensor_pwr_off),
    sensorFsmCmd(STATE_PS_POWER_OFF, STATE_PS_CHIP_OFF, epl_sensor_set_ps_power_off),
    sensorFsmCmd(STATE_PS_CHIP_OFF, STATE_PS_DISABLE_DONE, epl_sensor_detect_pwr),
    /* als rate change */
    sensorFsmCmd(STATE_ALS_RATECHG, STATE_ALS_RATECHG_DONE, epl_sensor_als_ratechg),
    /* ps rate change */
    sensorFsmCmd(STATE_PS_RATECHG, STATE_PS_RATECHG_DONE, epl_sensor_ps_ratechg),

    /* init state */
    sensorFsmCmd(STATE_RESET, STATE_GET_RENVO, epl_sensor_pwr_off),
    sensorFsmCmd(STATE_GET_RENVO, STATE_SET_INIT_GLB, epl_sensor_read_renvo),
    sensorFsmCmd(STATE_SET_INIT_GLB, STATE_POWER_ON, initial_global_variable),
    sensorFsmCmd(STATE_POWER_ON, STATE_PS_CMP_RESET, epl_sensor_pwr_on),
    sensorFsmCmd(STATE_PS_CMP_RESET, STATE_PS_CMP_RUN, epl_sensor_ps_cmp_rst),
    sensorFsmCmd(STATE_PS_CMP_RUN, STATE_ALS_CMP_RESET, epl_sensor_ps_cmp_run),
    sensorFsmCmd(STATE_ALS_CMP_RESET, STATE_ALS_CMP_RUN, epl_sensor_als_cmp_rst),
    sensorFsmCmd(STATE_ALS_CMP_RUN, STATE_POWER_OFF, epl_sensor_als_cmp_run),
    sensorFsmCmd(STATE_POWER_OFF, STATE_CHIP_UNKEY, epl_sensor_pwr_off),
    sensorFsmCmd(STATE_CHIP_UNKEY, STATE_CHIP_REFRESH0, epl_sensor_chip_unkey),
    sensorFsmCmd(STATE_CHIP_REFRESH0, STATE_CHIP_REFRESH1, epl_sensor_chip_refresh0),
    sensorFsmCmd(STATE_CHIP_REFRESH1, STATE_CHIP_KEY, epl_sensor_chip_refresh1),
    sensorFsmCmd(STATE_CHIP_KEY, STATE_CHIP_DOC, epl_sensor_chip_key),
    sensorFsmCmd(STATE_CHIP_DOC, STATE_OFFSET, epl_sensor_chip_doc),
    sensorFsmCmd(STATE_OFFSET, STATE_SET_PS_THD, epl_sensor_ps_offset),
    sensorFsmCmd(STATE_SET_PS_THD, STATE_SET_ALS_THD, epl_sensor_ps_thd),
    sensorFsmCmd(STATE_SET_ALS_THD, STATE_SET_ALS_CONF, epl_sensor_als_thd),
    sensorFsmCmd(STATE_SET_ALS_CONF, STATE_SET_PS_CONF, write_als_conf),
    sensorFsmCmd(STATE_SET_PS_CONF, STATE_SET_LED_CONF, write_ps_conf),
    sensorFsmCmd(STATE_SET_LED_CONF, STATE_SETUP_EINT, write_led_conf),
    sensorFsmCmd(STATE_SETUP_EINT, STATE_CORE, epl_sensor_setup_eint),
    sensorFsmCmd(STATE_CORE, STATE_INIT_DONE, epl_sensor_register_core),
};

static void epl_sensor_parse_dt(void)
{
    /*general setting*/
    epl_sensor.wait = (mTask.hw->wait_time<<4);
    /*als setting*/
    epl_sensor.als.integration_time = (mTask.hw->als_intt<<2);
    epl_sensor.als.gain = mTask.hw->als_gain;
    epl_sensor.als.cycle = mTask.hw->als_filter;
    epl_sensor.als.persist = (mTask.hw->als_persist<<2);
    epl_sensor.als.report_type = mTask.hw->als_report_type;
    epl_sensor.als.factory.lux_per_count = mTask.hw->als_lpc;
#if ALS_DYN_INTT
    als_dynamic_intt_intt[0] = (mTask.hw->als_dyn_intt_0 << 2);
    als_dynamic_intt_value[0] = als_intt_value[mTask.hw->als_dyn_intt_0];
    als_dynamic_intt_gain[0] = (mTask.hw->als_dyn_gain_0);
    als_dynamic_intt_intt[1] = (mTask.hw->als_dyn_intt_1 << 2);
    als_dynamic_intt_value[1] = als_intt_value[mTask.hw->als_dyn_intt_1];
    als_dynamic_intt_gain[1] = (mTask.hw->als_dyn_gain_1);
#endif
#if ALS_LSRC
    offset_gain = mTask.hw->als_offset_gain;
    scale_gain = mTask.hw->als_scale_gain;
#endif
    /*ps setting*/
    epl_sensor.ps.integration_time = (mTask.hw->ps_intt<<2);
    epl_sensor.ps.gain = mTask.hw->ps_gain;
    epl_sensor.ps.cycle = mTask.hw->ps_filter;
    epl_sensor.ps.persist = (mTask.hw->ps_persist<<2);
    epl_sensor.ps.ir_drive = mTask.hw->ps_ir_drive;
#if PS_DYN_K
    dynk_low_offset = mTask.hw->ps_dynk_low;
    dynk_high_offset =  mTask.hw->ps_dynk_high;
#endif

}
static int eplInit(void)
{
    int ret = 0;
    osLog(LOG_INFO, "%s: task starting\n", __func__);
    eplDebugPoint = &mTask;

    mTask.hw = get_cust_alsps("epl_sensor");
    if (NULL == mTask.hw)
    {
        osLog(LOG_ERROR, "get_cust_acc_hw fail\n");
        return 0;
    }
    mTask.i2c_addr = mTask.hw->i2c_addr[0];
    mTask.als_debounce_time = 200;
    mTask.als_debounce_on = 0;
    mTask.ps_debounce_time = 200;
    mTask.ps_debounce_on = 0;
    mTask.psCali = 0;
    mTask.als_threshold_high = mTask.hw->als_threshold_high;
    mTask.als_threshold_low = mTask.hw->als_threshold_low;
    mTask.ps_threshold_high = mTask.hw->ps_threshold_high;
    mTask.ps_threshold_low = mTask.hw->ps_threshold_low;
    mTask.polling_mode_ps = mTask.hw->polling_mode_ps;
    mTask.polling_mode_als = mTask.hw->polling_mode_als;
    osLog(LOG_INFO, "alsps i2c_num: %d, i2c_addr: 0x%x\n", mTask.hw->i2c_num, mTask.i2c_addr);
    osLog(LOG_INFO, "alsps polling_mode_ps=%d, polling_mode_als=%d\n", mTask.hw->polling_mode_ps, mTask.hw->polling_mode_als);
    epl_sensor_parse_dt();
    i2cMasterRequest(mTask.hw->i2c_num, I2C_SPEED);

    mTask.txBuf[0] = DEVREG_REV_ID;
    ret = i2cMasterTxRxSync(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1, &mTask.deviceId, 1, NULL, NULL);
    if (ret < 0)
    {
        osLog(LOG_ERROR, "epl_sensor: i2cMasterTxRxSync fail!!!\n");
        ret = -1;
        i2cMasterRelease(mTask.hw->i2c_num);
        goto err_out;
    }

    osLog(LOG_INFO, "epl_sensor: auto detect success:0x%x, ret=%d\n", mTask.deviceId, ret);

    alsSensorRegister();
    psSensorRegister();
    registerAlsPsDriverFsm(eplFsm, ARRAY_SIZE(eplFsm));

err_out:
    return ret;
}

#ifndef CFG_OVERLAY_INIT_SUPPORT
MODULE_DECLARE(epl_sensor, SENS_TYPE_ALS, eplInit);
#else
#include "mtk_overlay_init.h"
OVERLAY_DECLARE(epl_sensor, OVERLAY_WORK_02, eplInit);
#endif


