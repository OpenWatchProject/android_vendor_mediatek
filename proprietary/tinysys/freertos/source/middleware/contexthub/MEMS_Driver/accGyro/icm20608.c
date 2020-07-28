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

#include <accGyro.h>
#include <contexthub_core.h>
#include <cust_accGyro.h>
#include <mt_gpt.h>
///MEIZU_BSP{@
#include "accGyro.h"
#include "eint.h"
///@}

#define I2C_SPEED                       400000
#define AXIS_X              0
#define AXIS_Y              1
#define AXIS_Z              2
#define AXES_NUM            3

/* WHOAMI */
#define MPU6500_WHOAMI 0x70
#define MPU6881_WHOAMI 0x78
#define ICM20608G_WHOAMI 0xAF
#define ICM20608D_WHOAMI 0xAE
#define ICM20609_WHOAMI 0xA6
#define ICM20600_WHOAMI 0x11
#define ICM20602_WHOAMI 0x12
#define ICM20689_WHOAMI 0x98
#define ICM20690_WHOAMI 0x20
#define ICM20622_WHOAMI 0x25


#define DATA_READY_BIT  0x01
#define FIFO_OVERFLOW_BIT  0x10
#define ICM20608_REG_INT_MODE          0x38



#define ICM20600_REG_LP_CONFIG          0x1e
#define ICM20600_PWR_MGMT_1             0x6b
#define ICM20600_PWR_MGMT_2             0x6c
#define ICM20600_REG_FIFO_R_W           0x74
#define ICM20600_REG_FIFO_COUNTH        0x72
#define ICM20600_REG_FIFO_COUNTL        0x73
#define ICM20600_REG_FIFO_EN_2          0x23
#define ICM20600_REG_FIFO_RST           0x6a
#define ICM20600_REG_USER_CTRL          0x6a

#define REG_FIFO_EN (0x23)
#define REG_INT_PIN_CFG (0x37)
#define REG_INT_ENABLE  (0x38)
#define REG_INT_STATUS  (0x3a)


#define BIT_GYRO_LP_EN                  0x80
//#define BIT_ACC_LP_EN                   0x20
//#define BIT_G_AVGCFG                    0x00
//#define BIT_ACC_I2C_MST                 0x40

#define BIT_LP_EN                       0x20
#define BIT_CLK_PLL                     0x01
#define BIT_TEMP_DIS                    (1<<3)

#define BIT_PWR_ACCEL_STBY              0x38
#define BIT_PWR_GYRO_STBY               0x07

#define ICM20600_TIMEBASE_PLL           0x28

/* gyro */
#define ICM20600_REG_SAMRT_DIV          0x19
#define ICM20600_REG_GYRO_CONFIG_1      0x1a
#define ICM20600_REG_GYRO_CONFIG_2      0x1b

#define ICM20600_REG_WATERMARK_H        0x60
#define ICM20600_REG_WATERMARK_L        0x61

#define SHIFT_GYRO_FS_SEL                  3
#define GYRO_FS_SEL_250                     (0x00 << SHIFT_GYRO_FS_SEL)
#define GYRO_FS_SEL_500                     (0x01 << SHIFT_GYRO_FS_SEL)
#define GYRO_FS_SEL_1000                    (0x02 << SHIFT_GYRO_FS_SEL)
#define GYRO_FS_SEL_2000                    (0x03 << SHIFT_GYRO_FS_SEL)

#define ICM20600_FS_250_LSB             131.0f
#define ICM20600_FS_500_LSB             65.5f
#define ICM20600_FS_1000_LSB            32.8f
#define ICM20600_FS_2000_LSB            16.4f

#define GYRO_AVGCFG_1X                     0
#define GYRO_AVGCFG_2X                     1
#define GYRO_AVGCFG_4X                     2
#define GYRO_AVGCFG_8X                     3
#define GYRO_AVGCFG_16X                    4
#define GYRO_AVGCFG_32X                    5
#define GYRO_AVGCFG_64X                    6
#define GYRO_AVGCFG_128X                   7
#define GYRO_DLPFCFG_1                  (0x01)
#define GYRO_DLPFCFG_1_90hz             (0x02)
#define GYRO_DLPFCFG_4                  (0x04)
#define GYRO_DLPFCFG                    (0x07)
#define GYRO_FCHOICE                    (0x00)
/* gyro */

/* acc */
//#define ICM20600_REG_SAMRT_DIV1         0x10
//#define ICM20600_REG_SAMRT_DIV2         0x11
#define ICM20600_REG_ACC_CONFIG         0x1c
#define ICM20600_REG_ACC_CONFIG_2       0x1d


#define ICM20600_RANGE_2G               (0x00 << 3)
#define ICM20600_RANGE_4G               (0x01 << 3)
#define ICM20600_RANGE_8G               (0x02 << 3)
#define ICM20600_RANGE_16G              (0x03 << 3)

#define ACCEL_AVGCFG_1_4X               (0x00 << 4)
#define ACCEL_AVGCFG_8X                 (0x01 << 4)
#define ACCEL_AVGCFG_16X                (0x02 << 4)
#define ACCEL_AVGCFG_32X                (0x03 << 4)
#define ACCEL_FCHOICE                   1 << 3
#define ACCEL_DLPFCFG_2                 2
#define ACCEL_DLPFCFG_4                 4
#define ACCEL_DLPFCFG                   7
#define ACCEL_SAMPLE_FILTER             0x10// 0x10==8, 0x20==16

/* acc */

#define ACCEL_AVG_SAMPLE                (0x3)


#define ICM20600_REG_DATAX0             0x2d
#define ICM20600_REG_DATAY0             0x2f
#define ICM20600_REG_DATAZ0             0x31


#define ICM20600_SLEEP                  0x40    //enable low power sleep mode
#define ICM20600_DEV_RESET              0x80
#define ICM20600_ACCEL_CYCLE            0x20


/* ICM20600 Register Map  (Please refer to ICM20600 Specifications) */
#define ICM20600_REG_DEVID              0x75
#define remapFifoDelay(delay_ns, pll)     ((delay_ns) * (1270ULL)/(1270ULL + pll))

//#define FIFO_ISR
//#define DATA_NOT_FROM_FIFO
#undef DATA_NOT_FROM_FIFO
//#define DEBUG
#ifdef DATA_NOT_FROM_FIFO
#define ICM20608_REG_ACC_X_OUT_H  0x3b
#define ICM20608_REG_GYRO_X_OUT_H  0x43
#endif

enum ICM20600State {
    STATE_SAMPLE = CHIP_SAMPLING,
    STATE_FIFO = CHIP_FIFO,
    STATE_CONVERT = CHIP_CONVERT,
    STATE_SAMPLE_DONE = CHIP_SAMPLING_DONE,
    STATE_ACC_ENABLE = CHIP_ACC_ENABLE,
    STATE_ACC_ENABLE_DONE = CHIP_ACC_ENABLE_DONE,
    STATE_ACC_DISABLE = CHIP_ACC_DISABLE,
    STATE_ACC_DISABLE_DONE = CHIP_ACC_DISABLE_DONE,
    STATE_ACC_RATECHG = CHIP_ACC_RATECHG,
    STATE_ACC_RATECHG_DONE = CHIP_ACC_RATECHG_DONE,
    STATE_GYRO_ENABLE = CHIP_GYRO_ENABLE,
    STATE_GYRO_ENABLE_DONE = CHIP_GYRO_ENABLE_DONE,
    STATE_GYRO_DISABLE = CHIP_GYRO_DISABLE,
    STATE_GYRO_DISABLE_DONE = CHIP_GYRO_DISABLE_DONE,
    STATE_GYRO_RATECHG = CHIP_GYRO_RATECHG,
    STATE_GYRO_RATECHG_DONE = CHIP_GYRO_RATECHG_DONE,
    STATE_INIT_DONE = CHIP_INIT_DONE,
    STATE_IDLE = CHIP_IDLE,
    STATE_RESET_R = CHIP_RESET,
    STATE_RESET_W,
    STATE_WAIT_RESET_DONE,
    STATE_DEVID,
    STATE_SELCLK_R,
    STATE_SELCLK_W,
    STATE_POWER_R,
    STATE_DEPOWER_W,
    STATE_ENPOWER_W,
    STATE_DUTY_CYCLE,
    STATE_TURN_R,
    STATE_TURN_R_AGAIN,
    STATE_ACC_TURNON_W,
    STATE_ACC_TURNOFF_W,
    STATE_GYRO_TURNON_W,
    STATE_GYRO_TURNOFF_W,
    STATE_ACC_CONFIG1_R,
    STATE_ACC_CONFIG1_W,
    STATE_ACC_CONFIG2_R,
    STATE_ACC_CONFIG2_W,
    STATE_GYRO_CONFIG1_R,
    STATE_GYRO_CONFIG1_W,
    STATE_GYRO_CONFIG2_R,
    STATE_GYRO_CONFIG2_W,
    STATE_DATA_RESO,
    STATE_CALC_RESO,
    STATE_LP_R,
    STATE_ENLP_W,
    STATE_DELP_W,
    STATE_FIFO_RST,
    STATE_FIFOREG2_R,
    STATE_FIFOREG2_R_1,
    STATE_FIFOREG2_R_1_2rd_times,
    STATE_ACC_ENFIFO_W,
    STATE_ACC_DEFIFO_W,
    STATE_GYRO_ENFIFO_W,
    STATE_GYRO_DEFIFO_W,
    STATE_REG_USER_CTRL_R,
    STATE_REG_USER_CTRL_R_AGAIN,
    STATE_REG_USER_CTRL_R_3RD_TIMES,
    STATE_ENFIFO_W,
    STATE_DEFIFO_W,
    STATE_TIMEBASE,
    STATE_CORE,
    STATE_ACC_RATE,
    STATE_GYRO_RATE,
    STATE_INT_MODE,
    STATE_INIT_POWER_ON,
    STATE_INT_PIN_CFG,
    STATE_UNMASK_IRQ,
};

struct scale_factor {
    unsigned char  whole;
    unsigned char  fraction;
};

struct acc_data_resolution {
    struct scale_factor scalefactor;
    int                 sensitivity;
};
struct gyro_data_resolution {
    float   sensitivity;
};

static int32_t caliBias[3]={0,};

static struct acc_data_resolution icm20600g_data_resolution[] = {
    {{ 0,  6}, 16384},  /*+/-2g  in 16-bit resolution:  0.06 mg/LSB*/
    {{ 0, 12}, 8192},   /*+/-4g  in 16-bit resolution:  0.12 mg/LSB*/
    {{ 0, 24}, 4096},   /*+/-8g  in 16-bit resolution:  0.24 mg/LSB*/
    {{ 0, 5}, 2048},    /*+/-16g in 16-bit resolution:  0.49 mg/LSB*/
};
static struct gyro_data_resolution icm20600gy_data_resolution[] = {
    {ICM20600_FS_250_LSB},
    {ICM20600_FS_500_LSB},
    {ICM20600_FS_1000_LSB},
    {ICM20600_FS_2000_LSB},
};
#define MAX_I2C_PER_PACKET  6//8
#define ONE_SAMPLE_IN_FIFO_LEN  12// 14 // ACC:6 Temperature:2 GYRO:6
#define ICM20608_DATA_LEN   6
#define MAX_RXBUF 1024
#define MAX_TXBUF (MAX_RXBUF / MAX_I2C_PER_PACKET)
static struct ICM20600Task {
    /* txBuf for i2c operation, fill register and fill value */
    bool accPowerOn;
    bool gyroPowerOn;
    bool accFifoConfiged;
    bool gyroFifoConfiged;
    bool accFirstData;
    bool gyroFirstData;
    uint8_t txBuf[MAX_TXBUF];
    uint8_t prvTxBuf[8];
    /* rxBuf for i2c operation, receive rawdata */
    uint8_t rxBuf[MAX_RXBUF];
    uint8_t deviceId;
    uint8_t reset_wait_cnt;
    int8_t timeBase;
    uint8_t fifoLenHigh;
    uint8_t fifoLenLow;
    uint16_t accWaterMark;
    uint16_t gyroWaterMark;
    uint16_t fifoLen;
    uint32_t minAccGyroDelay;
    uint32_t maxAccGyroRate;
    uint32_t accRate;
    uint32_t gyroRate;
    uint16_t accRateDiv;
    uint16_t gyroRateDiv;
    uint64_t hwSampleTime;
    uint64_t swSampleTime;
    uint64_t lastSampleTime;
    uint64_t sampleTime;
    I2cCallbackF i2cCallBack;
    struct transferDataInfo dataInfo;
    struct accGyroDataPacket accGyroPacket;
    /* data for factory */
    struct TripleAxisDataPoint accFactoryData;
    struct TripleAxisDataPoint gyroFactoryData;

    int32_t accSwCali[AXES_NUM];
    int32_t gyroSwCali[AXES_NUM];
    struct acc_data_resolution *accReso;
    struct gyro_data_resolution *gyroReso;
    struct accGyro_hw *hw;
    struct sensorDriverConvert cvt;
    uint8_t i2c_addr;
} mTask;
static struct ICM20600Task *icm20600DebugPoint;
static unsigned char cali_reset_done=0;

#if 0
static void icm20645SelectBank(I2cCallbackF i2cCallBack, void *next_state, uint8_t bank)
{
    int ret = 0;
    //osLog(LOG_INFO, "icm20645SelectBank: 0x%x\n", bank);
    if (bank == BANK_SEL_0) {
        mTask.prvTxBuf[0] = REG_BANK_SEL;
        mTask.prvTxBuf[1] = bank;
        ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.prvTxBuf, 2, i2cCallBack,
                           next_state);
        if (ret != 0)
            osLog(LOG_INFO, "REG_BANK_SEL err\n");
    } else if (bank == BANK_SEL_1) {
        mTask.prvTxBuf[2] = REG_BANK_SEL;
        mTask.prvTxBuf[3] = bank;
        ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.prvTxBuf[2], 2, i2cCallBack,
                           next_state);
        if (ret != 0)
            osLog(LOG_INFO, "REG_BANK_SEL err\n");
    } else if (bank == BANK_SEL_2) {
        mTask.prvTxBuf[4] = REG_BANK_SEL;
        mTask.prvTxBuf[5] = bank;
        ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.prvTxBuf[4], 2, i2cCallBack,
                           next_state);
        if (ret != 0)
            osLog(LOG_INFO, "REG_BANK_SEL err\n");
    }
}
#endif
void icm20600TimerCbkF(void)
{
    mTask.hwSampleTime = rtcGetTime();
}

static int icm20600ResetRead(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    osLog(LOG_INFO, "icm20600ResetRead\n");
    //icm20600SelectBank(i2cCallBack, (void *)STATE_IDLE, BANK_SEL_0);

    mTask.txBuf[0] = ICM20600_PWR_MGMT_1;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,
                         next_state);
}

static int icm20600ResetWrite(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    osLog(LOG_INFO, "read 0x%x before icm20600ResetWrite\n", mTask.rxBuf[0]);

    // patrick: should add some delay at somewhere after reset to check reset is done
    // off to idle delay 100ms, idle to ready delay ACC 10/20ms, GYRO 80ms, add?
    mTask.reset_wait_cnt = 0;
    mTask.txBuf[0] = ICM20600_PWR_MGMT_1;
    mTask.txBuf[1] = mTask.rxBuf[0] | ICM20600_DEV_RESET;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}

static int icm20600WaitResetDone(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
#if 0
    unsigned char state=STATE_RESET_R;
    osLog(LOG_INFO, "icm20600WaitResetDone, reset_wait_cnt:%d \n", mTask.reset_wait_cnt);

    mdelay(1);

    /* mTask.reset_wait_cnt > 0 means, this function has been called,
     * and PWR_MGMT_1 has been read, so we can check the RESET bit */
    if (mTask.reset_wait_cnt > 0 && !(mTask.rxBuf[0] & ICM20600_DEV_RESET)) {
        /* Reset is done, goto next state */
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
        return 0;
    }

    /* The max reset time is 120ms */
    if (mTask.reset_wait_cnt > 120) {
        /* Too much time, reset the device again */
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, &state, SUCCESS_EVT);
    }

    mTask.reset_wait_cnt++;
#endif
    mdelay(150);//The max reset time is 120ms; wait reset complete
    mTask.txBuf[0] = ICM20600_PWR_MGMT_1;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,
                         next_state);
}


static int icm20600DeviceId(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    /* WE MUST SLEEP 10MS TO WAIT DEVICE RESET DONE */
    mdelay(50);
    osLog(LOG_INFO, "icm20600DeviceId\n");

    mTask.txBuf[0] = ICM20600_REG_DEVID;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         &mTask.deviceId, 1, i2cCallBack,
                         next_state);
}

static int icm20600SelClkRead(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    osLog(LOG_INFO, "icm20600SelClkRead\n");

    if ( mTask.deviceId != ICM20608D_WHOAMI ) {
        osLog(LOG_INFO, "icm20608g_CheckDeviceID ERR 0x%x\n\r", mTask.deviceId);
        return 0;
    }
    mTask.txBuf[0] = ICM20600_PWR_MGMT_1;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,
                         next_state);
}

static int icm20600SelClkWrite(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    osLog(LOG_INFO, "icm20600SelClkWrite\n");

    mTask.txBuf[0] = ICM20600_PWR_MGMT_1;
    mTask.txBuf[1] = mTask.rxBuf[0] | (BIT_CLK_PLL | BIT_TEMP_DIS);// 0x08==disable inter Temperature sensor
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}
static int icm20600PowerRead(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20600PowerRead PWR_MGMT_1\n");
    mTask.txBuf[0] = ICM20600_PWR_MGMT_1;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,
                         next_state);
}
static int icm20600PowerEnableWrite(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20600PowerEnableWrite: reg:0x6b<--value:0x%x\n", mTask.rxBuf[0]);
    mTask.txBuf[0] = ICM20600_PWR_MGMT_1;
    mTask.txBuf[1] = mTask.rxBuf[0] & (~ICM20600_SLEEP);
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}
static int icm20600PowerDisableWrite(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20600PowerDisableWrite: 0x%x\n", mTask.rxBuf[0]);
    if (mTask.accPowerOn || mTask.gyroPowerOn) {
        osLog(LOG_INFO, "icm20600PowerDisableWrite should not disable, acc:%d, gyro:%d\n",
            mTask.accPowerOn, mTask.gyroPowerOn);

    #ifdef FIFO_ISR
    mt_eint_unmask(mTask.hw->eint_num);
    #endif

        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    } else {
        mTask.txBuf[0] = ICM20600_PWR_MGMT_1;
        mTask.txBuf[1] = mTask.rxBuf[0] | ICM20600_SLEEP;
        return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                           next_state);
    }
    return 0;
}

static int icm20600SetIntMode(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                     void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                     void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    osLog(LOG_INFO, "icm20600SetIntMode: set irq data ready mode\n");

    mTask.txBuf[0] = ICM20608_REG_INT_MODE;
    mTask.txBuf[1] = DATA_READY_BIT | FIFO_OVERFLOW_BIT;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}
static int icm20600IntPinCfg(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                     void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                     void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    osLog(LOG_INFO, "%s: int pin cfg\n",__func__);

    // bit7 irq pin level is active 1:low, 0:high
    // bit5 irq pin level is held untile irq status is cleared
    // bit4 irq status is cleared only by reading INT_STATUS register
    mTask.txBuf[0] = REG_INT_PIN_CFG;
    mTask.txBuf[1] = 0xa0;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}
#if 0
static int icm20600gyDutyCycle(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    osLog(LOG_INFO, "icm20600gyDutyCycle\n");//gyro dutycycle sample
    mTask.txBuf[0] = ICM20600_REG_LP_CONFIG;
    mTask.txBuf[1] = BIT_GYRO_LP_EN | (GYRO_AVGCFG_2X<<4) | 0x08; // 0x08==62.5HZ
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}
#endif
static int icm20600TurnRead(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20600TurnRead PWR_MGMT_2\n");
    mTask.txBuf[0] = ICM20600_PWR_MGMT_2;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,
                         next_state);
}
static int icm20600gTurnOnWrite(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret=0;
    //osLog(LOG_INFO, "icm20600gTurnOnWrite: reg:0x6c<--value:0x%x\n", mTask.rxBuf[0]);
    mTask.txBuf[0] = ICM20600_PWR_MGMT_2;
    mTask.txBuf[1] = mTask.rxBuf[0] & ~BIT_PWR_ACCEL_STBY;
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
    return ret;
}
static int icm20600gTurnOffWrite(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20600gTurnOffWrite: 0x%x\n", mTask.rxBuf[0]);
    mTask.txBuf[0] = ICM20600_PWR_MGMT_2;
    mTask.txBuf[1] = mTask.rxBuf[0] | BIT_PWR_ACCEL_STBY;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}

static int icm20600gyTurnOnWrite(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret=0;
    //osLog(LOG_INFO, "icm20600gyTurnOnWrite: reg:0x6c<--value:0x%x\n", mTask.rxBuf[0]);
    mTask.txBuf[0] = ICM20600_PWR_MGMT_2;
    mTask.txBuf[1] = mTask.rxBuf[0] & ~BIT_PWR_GYRO_STBY;
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
    return ret;
}

static int icm20600gyTurnOffWrite(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20600gyTurnOffWrite: 0x%x\n", mTask.rxBuf[0]);
    mTask.txBuf[0] = ICM20600_PWR_MGMT_2;
    mTask.txBuf[1] = mTask.rxBuf[0] | BIT_PWR_GYRO_STBY;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}

static int icm20600AccConfig1Read(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    osLog(LOG_INFO, "icm20600AccConfig1Read\n");
    mTask.txBuf[0] = ICM20600_REG_ACC_CONFIG;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,
                         next_state);
}

static int icm20600AccConfig1Write(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    osLog(LOG_INFO, "icm20600AccConfig1Write\n");
    //icm20600SelectBank(i2cCallBack, (void *)STATE_IDLE, BANK_SEL_2);
    mTask.txBuf[0] = ICM20600_REG_ACC_CONFIG;
    mTask.txBuf[1] = (mTask.rxBuf[0] & (~ICM20600_RANGE_16G)) | ICM20600_RANGE_4G;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}

static int icm20600AccConfig2Read(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    osLog(LOG_INFO, "icm20600AccConfig2Read\n");
    mTask.txBuf[0] = ICM20600_REG_ACC_CONFIG_2;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,
                         next_state);
}

static int icm20600AccConfig2Write(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;

    osLog(LOG_INFO, "icm20600AccConfig2Write, reg:0x1d,value:0x%x\n",mTask.txBuf[0]);
    mTask.txBuf[0] = ICM20600_REG_ACC_CONFIG_2;
    mTask.txBuf[1] =  (mTask.rxBuf[0] & (~ACCEL_DLPFCFG)) | ACCEL_DLPFCFG_4; // patrick FCHOICE should be 0
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
    return ret;
}

static int icm20600GyroConfig1Read(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    osLog(LOG_INFO, "icm20600GyroConfig1Read\n");
    mTask.txBuf[0] = ICM20600_REG_GYRO_CONFIG_1;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,
                         next_state);
}

static int icm20600GyroConfig1Write(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;

    osLog(LOG_INFO, "icm20600GyroConfig\n");
    mTask.txBuf[0] = ICM20600_REG_GYRO_CONFIG_1;
    mTask.txBuf[1] = (mTask.rxBuf[0] & (~GYRO_DLPFCFG)) | (0x04);//GYRO_DLPFCFG_1_90hz;//
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
    return ret;
}

static int icm20600GyroConfig2Read(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    osLog(LOG_INFO, "icm20600GyroConfig1Read\n");
    mTask.txBuf[0] = ICM20600_REG_GYRO_CONFIG_2;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,
                         next_state);
}


static int icm20600GyroConfig2Write(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;

    osLog(LOG_INFO, "icm20600GyroConfig1\n");
    mTask.txBuf[0] = ICM20600_REG_GYRO_CONFIG_2;
    mTask.txBuf[1] = (mTask.rxBuf[0]&(~0x03)) | GYRO_FS_SEL_2000; //2000dps
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
    return ret;
}

static int icm20600DataReso(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;

    osLog(LOG_INFO, "icm20600DataReso\n");
    mTask.txBuf[0] = ICM20600_REG_ACC_CONFIG;
    ret = i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         &mTask.rxBuf[0], 1, i2cCallBack,
                         (void *)STATE_IDLE);
    mTask.txBuf[1] = ICM20600_REG_GYRO_CONFIG_2;
    ret = i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[1], 1,
                         &mTask.rxBuf[1], 1, i2cCallBack,
                         next_state);
    return ret;
}

static int icm20600CalcReso(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    uint8_t reso = 0;
    //icm20600SelectBank(i2cCallBack, (void *)STATE_IDLE, BANK_SEL_0);
    osLog(LOG_INFO, "icm20600CalcReso:acc(0x%x), gyro(0x%x)\n", mTask.rxBuf[0], mTask.rxBuf[1]);
    reso = 0x00;
    reso = (mTask.rxBuf[0] & ICM20600_RANGE_16G) >> 3; // patrick: should be 3
    if (reso < sizeof(icm20600g_data_resolution) / sizeof(icm20600g_data_resolution[0])) {
        mTask.accReso = &icm20600g_data_resolution[reso];
        osLog(LOG_INFO, "acc reso: %d, sensitivity: %d\n", reso, mTask.accReso->sensitivity);
    }
    reso = (mTask.rxBuf[1] & GYRO_FS_SEL_2000) >> 3;
    #if 0
    if (reso < sizeof(icm20600gy_data_resolution) / sizeof(icm20600gy_data_resolution[0])) {
        mTask.gyroReso = &icm20600gy_data_resolution[reso];
        osLog(LOG_INFO, "gyro reso: %d, sensitivity: %f\n", reso, (double)mTask.gyroReso->sensitivity);
    }
    #else
    mTask.gyroReso = &icm20600gy_data_resolution[3];
    osLog(LOG_INFO, "gyro reso: %d, sensitivity: %f\n", reso, (double)mTask.gyroReso->sensitivity);
    #endif
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int icm20600ReadTimeBase(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //icm20600SelectBank(i2cCallBack, (void *)STATE_IDLE, BANK_SEL_1);
#if 0
    mTask.txBuf[0] = ICM20600_TIMEBASE_PLL;
    i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         &mTask.timeBase, 1, i2cCallBack,
                         (void *)STATE_IDLE);
    //icm20600SelectBank(i2cCallBack, next_state, BANK_SEL_0);

#endif
    mTask.timeBase = 11;  //icm20600 have no pll register, so Directly specify
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

#if 0
static int icm20600LpRead(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    osLog(LOG_INFO, "icm20600LpRead\n");
    return icm20600PowerRead(i2cCallBack, spiCallBack, next_state,
        inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
}

static int icm20600EnableLpWrite(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    osLog(LOG_INFO, "icm20600EnableLpWrite:0x%x\n", mTask.rxBuf[0]);
    if ((mTask.gyroPowerOn == true && mTask.accPowerOn == true) ||
            (mTask.gyroPowerOn == true && mTask.accPowerOn == false)) {
        mTask.txBuf[0] = ICM20600_PWR_MGMT_1;
        mTask.txBuf[1] = mTask.rxBuf[0] & ~BIT_LP_EN;
    } else if (mTask.gyroPowerOn == false && mTask.accPowerOn == true) {
        mTask.txBuf[0] = ICM20600_PWR_MGMT_1;
        mTask.txBuf[1] = mTask.rxBuf[0] | BIT_LP_EN;
    } else if (mTask.gyroPowerOn == false && mTask.accPowerOn == false) {
        mTask.txBuf[0] = ICM20600_PWR_MGMT_1;
        mTask.txBuf[1] = mTask.rxBuf[0] | BIT_LP_EN;
    }
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}

static int icm20600DisableLpWrite(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    osLog(LOG_INFO, "icm20600DisableLpWrite:0x%x\n", mTask.rxBuf[0]);
    mTask.txBuf[0] = ICM20600_PWR_MGMT_1;
    mTask.txBuf[1] = mTask.rxBuf[0] & ~BIT_LP_EN;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}
#endif
static int icm20600gEnable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20600gEnable\n");
    unsigned char buf[2]={0,},ret=0;

    #ifdef FIFO_ISR
    mt_eint_mask(mTask.hw->eint_num);
    #endif

if( mTask.gyroPowerOn==false ) {
    buf[0] = ICM20600_PWR_MGMT_1;
    //0x59;bit7--Dev Reset 0,bit6--chip sleep 1,bit5--acc cycle 0,bit4--gyro sleep 0,
    // bit3--temp sensor disable 1,bit2~0--clk 1;
    buf[1] = 0x49 & (~ICM20600_SLEEP);

    //osLog(LOG_INFO, " PWR_MGMT_1 reg:0x6b<--value:0x%x\n", buf[1]);
    //mTask.txBuf[1] = mTask.rxBuf[0] & (~ICM20600_SLEEP);// ~0x08==enable inter Temperature sensor
    ret |= i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, buf, 2, NULL,
                    NULL);
}


    if( mTask.gyroPowerOn==false ) {
        buf[1] = 0x3f & ~BIT_PWR_ACCEL_STBY;
    } else {
        buf[1] = 0x38 & ~BIT_PWR_ACCEL_STBY;
    }
    buf[0] = ICM20600_PWR_MGMT_2;

    //osLog(LOG_INFO, "PWR_MGMT_2 reg:0x6c<--value:0x%x\n", buf[1]);
    ret |= i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, buf, 2, i2cCallBack,
                    next_state);


    mTask.accPowerOn = true;

    return 0;
}
static int icm20600gDisable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    struct accGyroCntlPacket cntlPacket;

    #ifdef FIFO_ISR
    mt_eint_mask(mTask.hw->eint_num);
    #endif

    ret = rxControlInfo(&cntlPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_INFO, "icm20600gDisable, rx water_mark err\n");
        return -1;
    }
    osLog(LOG_INFO, "icm20600gDisable: water_mark:%d\n", cntlPacket.waterMark);
    mTask.accPowerOn = false;
    mTask.accRate = 0;
    if (mTask.gyroPowerOn == true)
        mTask.maxAccGyroRate = (mTask.accRate > mTask.gyroRate ? mTask.accRate : mTask.gyroRate);
    else
        mTask.maxAccGyroRate = mTask.accRate;
    mTask.minAccGyroDelay = (mTask.maxAccGyroRate == 0) ? 0 : 1000000000000ULL / mTask.maxAccGyroRate;

    return icm20600TurnRead(i2cCallBack, spiCallBack, next_state,
        inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
}
static int icm20600gRate(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    uint32_t sample_rate, water_mark;
    struct accGyroCntlPacket cntlPacket;


    #ifdef FIFO_ISR
    mt_eint_mask(mTask.hw->eint_num);
    #endif

    ret = rxControlInfo(&cntlPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "icm20600gRate, rx inSize and elemSize error\n");
        return -1;
    }
    sample_rate = cntlPacket.rate;
    water_mark = cntlPacket.waterMark;
    osLog(LOG_INFO, "icm20600gRate, desire acc rate:%d, desire acc water_mark:%d\n",sample_rate, water_mark);

    if (sample_rate > 0 && sample_rate < 6000) {
        mTask.accRate = SENSOR_HZ(5);
        mTask.accRateDiv = 199;
    } else if (sample_rate > 9000 && sample_rate < 11000) {
        mTask.accRate = SENSOR_HZ(10);
        mTask.accRateDiv = 99;
    } else if (sample_rate > 16000 && sample_rate < 17000) {
        mTask.accRate = SENSOR_HZ(16.666);
        mTask.accRateDiv = 59;
    } else if (sample_rate > 40000 && sample_rate < 60000) {
        mTask.accRate = SENSOR_HZ(50);
        mTask.accRateDiv = 19;
    } else if (sample_rate > 90000 && sample_rate < 110000) {
        mTask.accRate = SENSOR_HZ(100);
        mTask.accRateDiv = 9;
    } else if (sample_rate > 190000 && sample_rate < 210000) {
        mTask.accRate = SENSOR_HZ(200);
        mTask.accRateDiv = 4;
    } else {
        mTask.accRate = sample_rate;
        if (sample_rate <= 4000)
            mTask.accRateDiv = 255;
        else if (sample_rate >= 1000000)
            mTask.accRateDiv = 0;
        else
            mTask.accRateDiv = 1024000 / sample_rate - 1;
    }

    if (mTask.gyroPowerOn == true) {
        mTask.maxAccGyroRate = (mTask.accRate > mTask.gyroRate ? mTask.accRate : mTask.gyroRate);
    } else {
        mTask.maxAccGyroRate = mTask.accRate;
    }

    mTask.accWaterMark = water_mark;
    mTask.minAccGyroDelay = 1024000000000ULL / mTask.maxAccGyroRate;
    osLog(LOG_INFO, "icm20600gRate: %d, minAccGyroDelay:%d, water_mark:%d\n", mTask.accRate, mTask.minAccGyroDelay, mTask.accWaterMark);

    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);

    return ret;
}
static int icm20600gSetRate(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;

    //osLog(LOG_INFO, "icm20600gSetRate\n");

    if (mTask.accRate < mTask.gyroRate && mTask.gyroPowerOn == true && mTask.accPowerOn == true) {
        /* set gyro register */
        mTask.txBuf[0] = ICM20600_REG_SAMRT_DIV;
        mTask.txBuf[1] = mTask.gyroRateDiv;
        ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                           (void *)STATE_IDLE);
        if (ret != 0)
            osLog(LOG_INFO, "ICM20600_REG_SAMRT_DIV err\n");

    } else if (mTask.accPowerOn == true) {
        /* set acc register */
        mTask.txBuf[0] = ICM20600_REG_SAMRT_DIV;
        mTask.txBuf[1] = mTask.accRateDiv;
        ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                           (void *)STATE_IDLE);
    }

    if (mTask.gyroWaterMark < mTask.accWaterMark && mTask.accPowerOn == true && mTask.gyroPowerOn == true) {
        /* set acc watermark to register */
        mTask.txBuf[0] = ICM20600_REG_WATERMARK_H;
        mTask.txBuf[1] = (mTask.gyroWaterMark >> 8) & 0x03; // max watermark is 1024, 10bit, high bit is watermark[9:8]
        mTask.txBuf[2] = mTask.gyroWaterMark & 0x00ff;
        ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3, i2cCallBack,
                           (void *)STATE_IDLE);
        if (ret != 0)
            osLog(LOG_INFO, "ICM20600_REG_SAMRT_DIV err\n");
    } else if (mTask.accPowerOn == true) {
        /* set gyro rate to gyro register */
        mTask.txBuf[0] = ICM20600_REG_WATERMARK_H;
        mTask.txBuf[1] = (mTask.accWaterMark >> 8) & 0x03; // max watermark is 1024, 10bit, high bit is watermark[9:8]
        mTask.txBuf[2] = mTask.accWaterMark & 0x00ff;
        ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3, i2cCallBack,
                           (void *)STATE_IDLE);
        if (ret != 0)
            osLog(LOG_INFO, "ICM20600_REG_SAMRT_DIV err\n");
    }

    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);

    return true;
}

static int icm20600RegUserCTRLRead(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20600RegUserCTRLRead\n");
    mTask.txBuf[0] = ICM20600_REG_USER_CTRL;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,
                         next_state);
}


static int icm20600FifoReset(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    //osLog(LOG_INFO, "icm20600FifoReset \n");

    mTask.txBuf[0] = ICM20600_REG_USER_CTRL;
    mTask.txBuf[1] = 0x04;
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[0], 2, i2cCallBack,
                       (void *)STATE_IDLE);

    mTask.txBuf[0] = 0x68;
    mTask.txBuf[1] = 0x1f;
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[0], 2, i2cCallBack,
                       (void *)STATE_IDLE);
    mTask.txBuf[0] = 0x68;
    mTask.txBuf[1] = 0x1f-1;
    ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[0], 2, i2cCallBack,
                       next_state);

    if (ret != 0)
        osLog(LOG_INFO, "ICM20600_REG_USER_CTRL err\n");
    return ret;
}
static int icm20600FifoReg2Read(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20600FifoReg2Read\n");

    mTask.txBuf[0] = ICM20600_REG_FIFO_EN_2;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack,
                         next_state);
}

static int icm20600gFifoEnable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20600gFifoEnable: 0x%x\n", mTask.rxBuf[0]);
#if 1
    mTask.accFifoConfiged = true;
    mTask.accFirstData = true;
    registerAccGyroFifoInfo(remapFifoDelay(mTask.minAccGyroDelay, mTask.timeBase),
        remapFifoDelay(mTask.minAccGyroDelay, mTask.timeBase));
    mTask.txBuf[0] = ICM20600_REG_FIFO_EN_2;
    mTask.txBuf[1] = mTask.rxBuf[0] | 0x08;

#else
    mTask.accFifoConfiged = false;
    mTask.txBuf[0] = ICM20600_REG_FIFO_EN_2;
    mTask.txBuf[1] = mTask.rxBuf[0] & (~(1<<3));//disable acc fifo en
#endif

    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}
static int icm20600gFifoDisable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20600gFifoDisable: 0x%x\n", mTask.rxBuf[0]);
#if 0
    mTask.accFifoConfiged = true;
    registerAccGyroFifoInfo(remapFifoDelay(mTask.minAccGyroDelay, mTask.timeBase),
        remapFifoDelay(mTask.minAccGyroDelay, mTask.timeBase));
    mTask.txBuf[0] = ICM20600_REG_FIFO_EN_2;
    mTask.txBuf[1] = mTask.rxBuf[0] | 0x08;

#else
    mTask.accFifoConfiged = false;
    registerAccGyroFifoInfo(remapFifoDelay(mTask.minAccGyroDelay, mTask.timeBase),
    remapFifoDelay(mTask.minAccGyroDelay, mTask.timeBase));
    mTask.txBuf[0] = ICM20600_REG_FIFO_EN_2;
    mTask.txBuf[1] = mTask.rxBuf[0] & (~(1<<3));//disable acc fifo en
#endif
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}

#if 0
static int icm20600gFifoDisable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    osLog(LOG_INFO, "icm20600gFifoDisable: 0x%x\n", mTask.rxBuf[0]);
    mTask.accFifoConfiged = false;
    registerAccGyroFifoInfo(remapFifoDelay(mTask.minAccGyroDelay, mTask.timeBase),
        remapFifoDelay(mTask.minAccGyroDelay, mTask.timeBase));
    mTask.txBuf[0] = ICM20600_REG_FIFO_EN_2;
    mTask.txBuf[1] = mTask.rxBuf[0] & ~(0x08);

    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}
static int icm20600gyFifoDisable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    osLog(LOG_INFO, "icm20600gyFifoDisable: 0x%x\n", mTask.rxBuf[0]);
    mTask.gyroFifoConfiged = false;
    registerAccGyroFifoInfo(remapFifoDelay(mTask.minAccGyroDelay, mTask.timeBase),
        remapFifoDelay(mTask.minAccGyroDelay, mTask.timeBase));
    mTask.txBuf[0] = ICM20600_REG_FIFO_EN_2;
    mTask.txBuf[1] = mTask.rxBuf[0] & (~0x10);
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);icm20600gyFifoEnable
}

static int icm20600FifoDisable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    osLog(LOG_INFO, "icm20600FifoDisable\n");
    mTask.txBuf[0] = ICM20600_REG_USER_CTRL;
    mTask.txBuf[1] = mTask.rxBuf[0] & ~(1 << 6);
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}
#endif
static int icm20600gyFifoEnable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20600gyFifoEnable: 0x%x\n", mTask.rxBuf[0]);
#if 1
    mTask.gyroFifoConfiged = true;
    mTask.gyroFirstData = true;
    registerAccGyroFifoInfo(remapFifoDelay(mTask.minAccGyroDelay, mTask.timeBase),
                        remapFifoDelay(mTask.minAccGyroDelay, mTask.timeBase));
    mTask.txBuf[0] = ICM20600_REG_FIFO_EN_2;
    mTask.txBuf[1] = mTask.rxBuf[0] | (0x70);//0x70==gyro fifo en;0xf0==temp+gyro fifo en
#else
    mTask.gyroFifoConfiged = false;
    mTask.txBuf[0] = ICM20600_REG_FIFO_EN_2;
    mTask.txBuf[1] = mTask.rxBuf[0] | (0x00); //acc gyro temp fifo en all disable
#endif

    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}
static int icm20600gyFifoDisable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20600gyFifoDisable: 0x%x\n", mTask.rxBuf[0]);
#if 0
    mTask.gyroFifoConfiged = true;
    registerAccGyroFifoInfo(remapFifoDelay(mTask.minAccGyroDelay, mTask.timeBase),
                        remapFifoDelay(mTask.minAccGyroDelay, mTask.timeBase));
    mTask.txBuf[0] = ICM20600_REG_FIFO_EN_2;
    mTask.txBuf[1] = mTask.rxBuf[0] | (0xf0);//0x70==gyro fifo en;0xf0==temp+gyro fifo en
#else
    mTask.gyroFifoConfiged = false;
    registerAccGyroFifoInfo(remapFifoDelay(mTask.minAccGyroDelay, mTask.timeBase),
        remapFifoDelay(mTask.minAccGyroDelay, mTask.timeBase));
    mTask.txBuf[0] = ICM20600_REG_FIFO_EN_2;
    mTask.txBuf[1] = mTask.rxBuf[0] & (~0xf0); //gyro temp fifo en all disable
#endif

    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}

static int icm20600FifoEnable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20600FifoEnable\n");
    mTask.txBuf[0] = ICM20600_REG_USER_CTRL;
    mTask.txBuf[1] = 0x40;  // fifo en switch

    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}
static int icm20600FifoDisable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20600FifoDisable\n");
    mTask.txBuf[0] = ICM20600_REG_USER_CTRL;
    mTask.txBuf[1] = 0x00;  // disable all fifo

    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                       next_state);
}

static int icm20600RateChangeDone(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    //osLog(LOG_INFO, "icm20600RateChangeDone\n");

    #ifdef FIFO_ISR
    mt_eint_unmask(mTask.hw->eint_num);
    #endif
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}


static int icm20600gyEnable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    //osLog(LOG_INFO, "icm20600gyEnable\n");
    unsigned char buf[2]={0,},ret=0;


    #ifdef FIFO_ISR
    mt_eint_mask(mTask.hw->eint_num);
    #endif

if( mTask.accPowerOn==false ) {
    buf[0] = ICM20600_PWR_MGMT_1;
    buf[1] = 0x49 & (~ICM20600_SLEEP);
    //osLog(LOG_INFO, " PWR_MGMT_1 reg:0x6b<--value:0x%x\n", buf[1]);
    //mTask.txBuf[1] = mTask.rxBuf[0] & (~ICM20600_SLEEP);// ~0x08==enable inter Temperature sensor
    ret |= i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, buf, 2, NULL, NULL);
}

    if( mTask.accPowerOn==false ) {
        buf[1] = 0x3f & ~BIT_PWR_GYRO_STBY;
    } else {
        buf[1] = 0x07 & ~BIT_PWR_GYRO_STBY;
    }

    //osLog(LOG_INFO, "PWR_MGMT_2 reg:0x6c<--value:0x%x\n", buf[1]);
    buf[0] = ICM20600_PWR_MGMT_2;
    //mTask.txBuf[1] = mTask.rxBuf[0] & ~BIT_PWR_GYRO_STBY;
    ret |= i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, buf, 2, i2cCallBack,
                    next_state);

    mTask.gyroPowerOn = true;

    //meizudebug
    return 0;

    return icm20600PowerRead(i2cCallBack, spiCallBack, next_state,
        inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
}
static int icm20600gyDisable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    struct accGyroCntlPacket cntlPacket;

    ret = rxControlInfo(&cntlPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_INFO, "icm20600gyDisable, rx water_mark err\n");
        return -1;
    }
    osLog(LOG_INFO, "icm20600gyDisable: water_mark:%d\n", cntlPacket.waterMark);
    mTask.gyroPowerOn = false;
    mTask.gyroRate = 0;
    if (mTask.accPowerOn == true)
        mTask.maxAccGyroRate = (mTask.accRate > mTask.gyroRate ? mTask.accRate : mTask.gyroRate);
    else
        mTask.maxAccGyroRate = mTask.gyroRate;
    mTask.minAccGyroDelay = (mTask.maxAccGyroRate == 0) ? 0 : 1000000000000ULL / mTask.maxAccGyroRate;

    return icm20600TurnRead(i2cCallBack, spiCallBack, next_state,
        inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
}
static int icm20600gyRate(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    uint32_t sample_rate, water_mark;
    struct accGyroCntlPacket cntlPacket;

    #ifdef FIFO_ISR
    mt_eint_mask(mTask.hw->eint_num);
    #endif

    ret = rxControlInfo(&cntlPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "icm20600gyRate, rx inSize and elemSize error\n");
        return -1;
    }
    sample_rate = cntlPacket.rate;
    water_mark = cntlPacket.waterMark;
    osLog(LOG_INFO, "icm20600gyRate, desire gyro rate:%d, desire gyro water_mark:%d\n",sample_rate, water_mark);

    if (sample_rate > 0 && sample_rate < 6000) {
        mTask.gyroRate = SENSOR_HZ(5);
        mTask.gyroRateDiv = 199;
    } else if (sample_rate > 9000 && sample_rate < 11000) {
        mTask.gyroRate = SENSOR_HZ(10);
        mTask.gyroRateDiv = 99;
    } else if (sample_rate > 16000 && sample_rate < 17000) {
        mTask.gyroRate = SENSOR_HZ(16.666);
        mTask.gyroRateDiv = 59;
    } else if (sample_rate > 40000 && sample_rate < 60000) {
        mTask.gyroRate = SENSOR_HZ(50);
        mTask.gyroRateDiv = 19;
    } else if (sample_rate > 90000 && sample_rate < 110000) {
        mTask.gyroRate = SENSOR_HZ(100);
        mTask.gyroRateDiv = 9;
    } else if (sample_rate > 190000 && sample_rate < 210000) {
        mTask.gyroRate = SENSOR_HZ(200);
        mTask.gyroRateDiv = 4;
    } else {
        mTask.gyroRate = sample_rate;
        if (sample_rate <= 4000)
            mTask.gyroRateDiv = 255;
        else if (sample_rate >= 1000000)
            mTask.gyroRateDiv = 0;
        else
            mTask.gyroRateDiv = 1024000 / sample_rate - 1;
    }
    if (mTask.accPowerOn == true)
        mTask.maxAccGyroRate = (mTask.accRate > mTask.gyroRate ? mTask.accRate : mTask.gyroRate);
    else
        mTask.maxAccGyroRate = mTask.gyroRate;

    mTask.gyroWaterMark = water_mark;
    mTask.minAccGyroDelay = 1024000000000ULL / mTask.maxAccGyroRate;
    osLog(LOG_INFO, "icm20600gyRate: %d, minAccGyroDelay: %d, water_mark:%d\n", mTask.gyroRate, mTask.minAccGyroDelay, mTask.gyroWaterMark);

    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);

    return ret;
}
static int icm20600gySetRate(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
    /* hardware issue, fifo odr relay on gyro rate, if acc rate > gyro rate, we should
     * set acc rate to gyro rate register, because set acc rate register fifo odr is also gyro rate,
     * then acc odr will < the odr user needed.
     */
    if (mTask.accRate > mTask.gyroRate && mTask.accPowerOn == true && mTask.gyroPowerOn == true) {
        /* set acc rate to gyro register */
        mTask.txBuf[0] = ICM20600_REG_SAMRT_DIV;
        mTask.txBuf[1] = mTask.accRateDiv;
        ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                           (void *)STATE_IDLE);
        if (ret != 0)
            osLog(LOG_INFO, "ICM20600_REG_SAMRT_DIV err\n");
    } else if (mTask.gyroPowerOn == true) {
        /* set gyro rate to gyro register */
        mTask.txBuf[0] = ICM20600_REG_SAMRT_DIV;
        mTask.txBuf[1] = mTask.gyroRateDiv;
        ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2, i2cCallBack,
                           (void *)STATE_IDLE);
        if (ret != 0)
            osLog(LOG_INFO, "ICM20600_REG_SAMRT_DIV err\n");
    }

    if (mTask.accWaterMark < mTask.gyroWaterMark && mTask.accPowerOn == true && mTask.gyroPowerOn == true) {
        /* set acc watermark to register */
        mTask.txBuf[0] = ICM20600_REG_WATERMARK_H;
        mTask.txBuf[1] = (mTask.accWaterMark >> 8) & 0x03; // max watermark is 1024, 10bit, high bit is watermark[9:8]
        mTask.txBuf[2] = mTask.accWaterMark & 0x00ff;
        ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3, i2cCallBack,
                           (void *)STATE_IDLE);
        if (ret != 0)
            osLog(LOG_INFO, "ICM20600_REG_SAMRT_DIV err\n");
    } else if (mTask.gyroPowerOn == true) {
        /* set gyro rate to gyro register */
        mTask.txBuf[0] = ICM20600_REG_WATERMARK_H;
        mTask.txBuf[1] = (mTask.gyroWaterMark >> 8) & 0x03; // max watermark is 1024, 10bit, high bit is watermark[9:8]
        mTask.txBuf[2] = mTask.gyroWaterMark & 0x00ff;
        ret = i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 3, i2cCallBack,
                           (void *)STATE_IDLE);
        if (ret != 0)
            osLog(LOG_INFO, "ICM20600_REG_SAMRT_DIV err\n");
    }

#if 0
    registerAccGyroFifoInfo((mTask.accRate == 0) ? 0 : 1000000000000ULL / mTask.accRate,
                                 (mTask.gyroRate == 0) ? 0 : 1000000000000ULL / mTask.gyroRate);
#endif
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);

    return true;
}

static void accGetCalibration(int32_t *cali, int32_t size)
{
    static int time=0;
#if 0
    int retry=0;
    while( cali_reset_done && (retry<10) ) {//<=300ms,kernel 500ms timeout
        retry++;
        vTaskDelay(30);
    };
#endif

    cali[AXIS_X]  = caliBias[AXIS_X];
    cali[AXIS_Y]  = caliBias[AXIS_Y];
    cali[AXIS_Z]  = caliBias[AXIS_Z];

    time++;
    osLog(LOG_INFO, "accGetCalibration time:%d, cali x:%d, y:%d, z:%d\n", time, cali[AXIS_X], cali[AXIS_Y], cali[AXIS_Z]);
}
static void accSetCalibration(int32_t *cali, int32_t size)
{
    static int time=0;
#if 0
    struct SimpleQueue* sq;
    struct HostIntfDataBuffer *data;
#endif
    mTask.accSwCali[AXIS_X] = cali[AXIS_X];
    mTask.accSwCali[AXIS_Y] = cali[AXIS_Y];
    mTask.accSwCali[AXIS_Z] = cali[AXIS_Z];
#if 0
    // bool SensorQueueEnqueue(struct SimpleQueue* sq, const void *data, int length, bool possiblyDiscardable)
    data->triple[0].x = 0;
    data->triple[0].y = 0;
    data->triple[0].z = 0;
    SensorQueueEnqueue(sq, (void *)data, 1, 1);
#endif
    time++;
    if( !mTask.accSwCali[AXIS_X] && !mTask.accSwCali[AXIS_Y] && !mTask.accSwCali[AXIS_Z] ) {
        cali_reset_done = 1;
    } else {
        cali_reset_done = 0;
    }
    osLog(LOG_INFO, "accSetCalibration time:%d, cali x:%d, y:%d, z:%d\n", time, mTask.accSwCali[AXIS_X],
        mTask.accSwCali[AXIS_Y], mTask.accSwCali[AXIS_Z]);
}
static void accGetData(void *sample)
{
    struct TripleAxisDataPoint *tripleSample = (struct TripleAxisDataPoint *)sample;
    tripleSample->ix = mTask.accFactoryData.ix;
    tripleSample->iy = mTask.accFactoryData.iy;
    tripleSample->iz = mTask.accFactoryData.iz;

    osLog(LOG_INFO,"%s x:%d,y:%d,z:%d\n",__func__,tripleSample->ix,tripleSample->iy,tripleSample->iz);
}
static void gyroGetCalibration(int32_t *cali, int32_t size)
{
    cali[AXIS_X] = mTask.gyroSwCali[AXIS_X];
    cali[AXIS_Y] = mTask.gyroSwCali[AXIS_Y];
    cali[AXIS_Z] = mTask.gyroSwCali[AXIS_Z];
    osLog(LOG_INFO, "gyroGetCalibration cali x:%d, y:%d, z:%d\n", cali[AXIS_X], cali[AXIS_Y], cali[AXIS_Z]);
}
static void gyroSetCalibration(int32_t *cali, int32_t size)
{
    mTask.gyroSwCali[AXIS_X] = cali[AXIS_X];
    mTask.gyroSwCali[AXIS_Y] = cali[AXIS_Y];
    mTask.gyroSwCali[AXIS_Z] = cali[AXIS_Z];
    osLog(LOG_INFO, "gyroSetCalibration cali x:%d, y:%d, z:%d\n", mTask.gyroSwCali[AXIS_X],
        mTask.gyroSwCali[AXIS_Y], mTask.gyroSwCali[AXIS_Z]);
}
static void gyroGetData(void *sample)
{
    struct TripleAxisDataPoint *tripleSample = (struct TripleAxisDataPoint *)sample;
    tripleSample->ix = mTask.gyroFactoryData.ix;
    tripleSample->iy = mTask.gyroFactoryData.iy;
    tripleSample->iz = mTask.gyroFactoryData.iz;

    osLog(LOG_INFO,"%s x:%d,y:%d,z:%d\n",__func__,tripleSample->ix,tripleSample->iy,tripleSample->iz);
}

#ifdef FIFO_ISR
static void icm20608_isr(int arg)
{
    osLog(LOG_INFO, "%s, %lld\n", __func__, rtcGetTime());
    accGyroInterruptOccur();
}
#endif

static int icm20600RegisterCore(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    struct sensorCoreInfo mInfo;
    /* Register sensor Core */
    mInfo.sensType = SENS_TYPE_ACCEL;
    mInfo.gain = GRAVITY_EARTH_1000;
    mInfo.sensitivity = mTask.accReso->sensitivity;
    mInfo.cvt = mTask.cvt;
    mInfo.getCalibration = accGetCalibration;
    mInfo.setCalibration = accSetCalibration;
    mInfo.getData = accGetData;
    sensorCoreRegister(&mInfo);

    mInfo.sensType = SENS_TYPE_GYRO;
    mInfo.gain = GYROSCOPE_INCREASE_NUM_AP;
    mInfo.sensitivity = mTask.gyroReso->sensitivity;
    mInfo.cvt = mTask.cvt;
    mInfo.getCalibration = gyroGetCalibration;
    mInfo.setCalibration = gyroSetCalibration;
    mInfo.getData = gyroGetData;
    sensorCoreRegister(&mInfo);

#ifdef FIFO_ISR
    mt_eint_dis_hw_debounce(mTask.hw->eint_num);
    mt_eint_registration(mTask.hw->eint_num, LEVEL_SENSITIVE, LOW_LEVEL_TRIGGER, icm20608_isr, EINT_INT_UNMASK,
                            EINT_INT_AUTO_UNMASK_OFF);
#endif
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}
#ifdef DEBUG
static unsigned char buf[10]={0,};
#endif
static int icm20600Sample(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;

    ret = rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "icm20600Sample, rx dataInfo error\n");
        return -1;
    }
#ifdef DEBUG
    osLog(LOG_INFO, "%s acc on: %d, gyro on:%d\n",__func__, mTask.accPowerOn, mTask.gyroPowerOn);
    mTask.txBuf[0] = ICM20600_PWR_MGMT_1;
    ret |= i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[0], 1,
                        &mTask.rxBuf[0], 1, i2cCallBack,
                        (void *)STATE_IDLE);
    mTask.txBuf[0] = ICM20600_PWR_MGMT_2;
    ret |= i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[0], 1,
                        &mTask.rxBuf[1], 1, i2cCallBack,
                        (void *)STATE_IDLE);

    mTask.txBuf[0] = REG_FIFO_EN;
    ret |= i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[0], 1,
                        &mTask.rxBuf[2], 1, i2cCallBack,
                        (void *)STATE_IDLE);
    mTask.txBuf[0] = REG_INT_PIN_CFG;
    ret |= i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[0], 1,
                        &mTask.rxBuf[3], 1, i2cCallBack,
                        (void *)STATE_IDLE);
    mTask.txBuf[0] = REG_INT_ENABLE;
    ret |= i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[0], 1,
                        &mTask.rxBuf[4], 1, i2cCallBack,
                        (void *)STATE_IDLE);
    mTask.txBuf[0] = REG_INT_STATUS;
    ret |= i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[0], 1,
                        &mTask.rxBuf[5], 1, i2cCallBack,
                        (void *)STATE_IDLE);

    mTask.txBuf[0] = ICM20600_REG_USER_CTRL;
    ret |= i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[0], 1,
                        &mTask.rxBuf[6], 1, i2cCallBack,
                        (void *)STATE_IDLE);




    mTask.txBuf[0] = ICM20600_REG_SAMRT_DIV;
    ret |= i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[0], 1,
                        &buf[0], 1, i2cCallBack,
                        (void *)STATE_IDLE);
    mTask.txBuf[0] = ICM20600_REG_GYRO_CONFIG_1;
    ret |= i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[0], 1,
                        &buf[1], 1, i2cCallBack,
                        (void *)STATE_IDLE);
    mTask.txBuf[0] = ICM20600_REG_GYRO_CONFIG_2;
    ret |= i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[0], 1,
                        &buf[2], 1, i2cCallBack,
                        (void *)STATE_IDLE);
    mTask.txBuf[0] = ICM20600_REG_WATERMARK_H;
    ret |= i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[0], 1,
                        &buf[3], 2, i2cCallBack,
                        (void *)STATE_IDLE);
#endif
    mTask.txBuf[0] = ICM20600_REG_FIFO_COUNTH;
    ret |= i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[0], 1,
                         &mTask.fifoLenHigh, 1, i2cCallBack,
                         (void *)STATE_IDLE);
    if (ret != 0)
        osLog(LOG_INFO, "ICM20600_REG_FIFO_COUNTH err\n");
    mTask.txBuf[1] = ICM20600_REG_FIFO_COUNTL;
    ret |= i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[1], 1,
                         &mTask.fifoLenLow, 1, i2cCallBack,
                         next_state);
    if (ret != 0)
        osLog(LOG_INFO, "ICM20600_REG_FIFO_COUNTL err\n");
    return 0;
}

#ifdef DATA_NOT_FROM_FIFO
static unsigned char buffer[20]={0,};
#ifdef DEBUG
static unsigned char value[5]={0,};
#endif
#endif
static unsigned char fifo_reset=0;
static void i2cIsrCallBack(void *cookie, size_t tx, size_t rx, int err)
{
    if (err != 0) {
        osLog(LOG_ERROR, "icm20600: i2cIsrCallback err\n");
        sensorFsmEnqueueFakeI2cEvt(mTask.i2cCallBack, cookie, ERROR_EVT);
    } else {
        mTask.swSampleTime = rtcGetTime();
        sensorFsmEnqueueFakeI2cEvt(mTask.i2cCallBack, cookie, SUCCESS_EVT);
    }
}
static int icm20600ReadFifo(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    int ret = 0;
#ifndef DATA_NOT_FROM_FIFO
    static uint16_t lastFifoLen = 0;
#endif
    static uint64_t lastSampleTime = 0;
    static uint64_t times=0;
    //uint16_t fifoCount = 0;

    mTask.sampleTime = rtcGetTime();
    lastSampleTime = mTask.sampleTime;
    times++;
#ifdef DEBUG
    osLog(LOG_INFO, "%s run times:%lld, PWR_MGMT_1:0x%x,PWR_MGMT_2:0x%x,FIFO_EN:0x%x, ACC_GYR_FIFO_EN:0x%x,\n"
                        " INT_PIN_CFG:0x%x,INT_ENABLE:0x%x,INT_STATUS:0x%x,\n"
                        " timestamp:%lld, SAMRT_DIV:0x%x,GYR_CFG1:0x%x,GYR_CFG2:0x%x,WATERMARK H:0x%x,L:0x%x\n",__func__,
                        times, mTask.rxBuf[0],mTask.rxBuf[1],
                        mTask.rxBuf[6], mTask.rxBuf[2], mTask.rxBuf[3], mTask.rxBuf[4], mTask.rxBuf[5],
                        lastSampleTime, buf[0], buf[1], buf[2], buf[3], buf[4]);
#endif
#ifdef DATA_NOT_FROM_FIFO
    if( mTask.accPowerOn ) {
        mTask.txBuf[0] = ICM20608_REG_ACC_X_OUT_H;
        ret |= i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[0], 1,
                             buffer, 6, i2cCallBack,
                             (void *)STATE_IDLE);
    }
    if( mTask.gyroPowerOn ) {
        mTask.txBuf[0] = ICM20608_REG_GYRO_X_OUT_H;
        ret |= i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[0], 1,
                             buffer+6, 6, i2cCallBack,
                             (void *)STATE_IDLE);
    }

#ifdef DEBUG
    mTask.txBuf[0] = 0x23;
    ret |= i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[0], 1,
                         &value[0], 1, i2cCallBack,
                         (void *)STATE_IDLE);
    mTask.txBuf[0] = 0x6a;
    ret |= i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[0], 1,
                         &value[1], 1, i2cCallBack,
                         (void *)STATE_IDLE);
    mTask.txBuf[0] = 0x6b;
    ret |= i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[0], 1,
                         &value[2], 1, i2cCallBack,
                         (void *)STATE_IDLE);
    mTask.txBuf[0] = 0x6c;
    ret |= i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[0], 1,
                         &value[3], 1, i2cCallBack,
                         next_state);
#endif
#else

    mTask.fifoLen = (mTask.fifoLenHigh << 8) | mTask.fifoLenLow;
    mTask.fifoLen = (mTask.fifoLen / ICM20608_DATA_LEN) * ICM20608_DATA_LEN;

    //osLog(LOG_INFO, "icm20600ReadFifo count: %d\n", mTask.fifoLen);
    /* if (mTask.gyroFifoConfiged && mTask.accFifoConfiged) {
        fifoCount = mTask.fifoLen / ICM20608_DATA_LEN;
        fifoCount = (fifoCount / 2) * 2;
        mTask.fifoLen = fifoCount * ICM20608_DATA_LEN;
    } */
    if (mTask.fifoLen <= 0) {
        lastSampleTime = mTask.sampleTime;
        lastFifoLen = mTask.fifoLen;
        osLog(LOG_INFO, "icm20600ReadFifo fifo len is '%d'\n", mTask.fifoLen);

        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
        return 0;
    }
    mTask.txBuf[0] = ICM20600_REG_FIFO_R_W;
    ret = i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         NULL, 0, i2cCallBack,
                         (void *)STATE_IDLE);
    if (ret != 0) {
        fifo_reset = 1;
        osLog(LOG_INFO, "icm20600ReadFifo err lastFifoLen3:%d, fifolen:%d, lastSampleTime:%lld, sampleTime:%lld\n",
            lastFifoLen, mTask.fifoLen, lastSampleTime, mTask.sampleTime);
    }
    mTask.i2cCallBack = i2cCallBack;
    ret = i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, NULL, 0,
                         mTask.rxBuf, mTask.fifoLen, i2cIsrCallBack,
                         next_state);
    if (ret != 0) {
        fifo_reset = 1;
        osLog(LOG_INFO, "icm20600ReadFifo err lastFifoLen3:%d, fifolen:%d, lastSampleTime:%lld, sampleTime:%lld\n",
            lastFifoLen, mTask.fifoLen, lastSampleTime, mTask.sampleTime);
    }

    lastSampleTime = mTask.sampleTime;
    lastFifoLen = mTask.fifoLen;
#endif

//meizudebug
    return 0;//ret;
}
static int icm20600gConvert(uint8_t *databuf, struct accGyroData *data)
{
    int32_t raw_data[AXES_NUM],i;
    int32_t remap_data[AXES_NUM];
    static float filter[3][5];
    float sumx = 0.0, sumy = 0.0, sumz = 0.0;
    static int index = 0;
    static int times = 0;

    raw_data[AXIS_X] = (int16_t)((databuf[AXIS_X * 2] << 8) | databuf[AXIS_X * 2 + 1]);
    raw_data[AXIS_Y] = (int16_t)((databuf[AXIS_Y * 2] << 8) | databuf[AXIS_Y * 2 + 1]);
    raw_data[AXIS_Z] = (int16_t)((databuf[AXIS_Z * 2] << 8) | databuf[AXIS_Z * 2 + 1]);
/*    osLog(LOG_INFO, "%s acc rawdata x:%d, y:%d, z:%d\n", __func__, raw_data[AXIS_X],
    raw_data[AXIS_Y], raw_data[AXIS_Z]);  */

    if( cali_reset_done ) {
        cali_reset_done = 0;
        caliBias[AXIS_X] = -raw_data[AXIS_X];
        caliBias[AXIS_Y] = -raw_data[AXIS_Y];
#if 0// 2G
        caliBias[AXIS_Z] = -((raw_data[AXIS_Z]>0)?(raw_data[AXIS_Z]-16384):(raw_data[AXIS_Z]+16384));
#else
        caliBias[AXIS_Z] = -((raw_data[AXIS_Z]>0)?(raw_data[AXIS_Z]-8196):(raw_data[AXIS_Z]+8196));
#endif
        osLog(LOG_INFO, "%s bias accSwCali x:%d, y:%d, z:%d\n", __func__, caliBias[AXIS_X],
              caliBias[AXIS_Y], caliBias[AXIS_Z]);
    } else {
        raw_data[AXIS_X] = raw_data[AXIS_X] + mTask.accSwCali[AXIS_X];
        raw_data[AXIS_Y] = raw_data[AXIS_Y] + mTask.accSwCali[AXIS_Y];
        raw_data[AXIS_Z] = raw_data[AXIS_Z] + mTask.accSwCali[AXIS_Z];
    }


    /*remap coordinate*/
    remap_data[mTask.cvt.map[AXIS_X]] = mTask.cvt.sign[AXIS_X] * raw_data[AXIS_X];
    remap_data[mTask.cvt.map[AXIS_Y]] = mTask.cvt.sign[AXIS_Y] * raw_data[AXIS_Y];
    remap_data[mTask.cvt.map[AXIS_Z]] = mTask.cvt.sign[AXIS_Z] * raw_data[AXIS_Z];

    /* Out put the degree/second(mo/s) */
    data->sensType = SENS_TYPE_ACCEL;
    data->x = (float)remap_data[AXIS_X] * GRAVITY_EARTH_SCALAR / mTask.accReso->sensitivity;
    data->y = (float)remap_data[AXIS_Y] * GRAVITY_EARTH_SCALAR / mTask.accReso->sensitivity;
    data->z = (float)remap_data[AXIS_Z] * GRAVITY_EARTH_SCALAR / mTask.accReso->sensitivity;
    mTask.accFactoryData.ix = (int32_t)(data->x * ACCELEROMETER_INCREASE_NUM_AP);
    mTask.accFactoryData.iy = (int32_t)(data->y * ACCELEROMETER_INCREASE_NUM_AP);
    mTask.accFactoryData.iz = (int32_t)(data->z * ACCELEROMETER_INCREASE_NUM_AP);

    filter[0][index] = data->x;
    filter[1][index] = data->y;
    filter[2][index] = data->z;
    for( i=0;i<=index;i++ ) {
        sumx += filter[0][i];
        sumy += filter[1][i];
        sumz += filter[2][i];
    }
    data->x = sumx/i;
    data->y = sumy/i;
    data->z = sumz/i;

    index++;
    if( index>=5 )
        index = 0;
    mTask.accFactoryData.ix = data->x;
    mTask.accFactoryData.iy = data->y;
    mTask.accFactoryData.iz = data->z;

    times++;
    if (times >= 10) {
        times = 0;
#ifdef DEBUG
        osLog(LOG_INFO, "%s acc rawdata x:%f, y:%f, z:%f\n", __func__,
            (double)data->x, (double)data->y, (double)data->z);
#endif
    }

    return 0;
}
static int icm20600gyConvert(uint8_t *databuf, struct accGyroData *data)
{
    int32_t raw_data[AXES_NUM];
    int32_t remap_data[AXES_NUM];
    static int times = 0;

    raw_data[AXIS_X] = (int16_t)((databuf[AXIS_X * 2] << 8) | databuf[AXIS_X * 2 + 1]);
    raw_data[AXIS_Y] = (int16_t)((databuf[AXIS_Y * 2] << 8) | databuf[AXIS_Y * 2 + 1]);
    raw_data[AXIS_Z] = (int16_t)((databuf[AXIS_Z * 2] << 8) | databuf[AXIS_Z * 2 + 1]);
    /*osLog(LOG_INFO, "gyro rawdata x:%d, y:%d, z:%d\n", raw_data[AXIS_X],
    raw_data[AXIS_Y], raw_data[AXIS_Z]);*/
    raw_data[AXIS_X] = raw_data[AXIS_X] + mTask.gyroSwCali[AXIS_X];
    raw_data[AXIS_Y] = raw_data[AXIS_Y] + mTask.gyroSwCali[AXIS_Y];
    raw_data[AXIS_Z] = raw_data[AXIS_Z] + mTask.gyroSwCali[AXIS_Z];

    /*remap coordinate*/
    remap_data[mTask.cvt.map[AXIS_X]] = mTask.cvt.sign[AXIS_X] * raw_data[AXIS_X];
    remap_data[mTask.cvt.map[AXIS_Y]] = mTask.cvt.sign[AXIS_Y] * raw_data[AXIS_Y];
    remap_data[mTask.cvt.map[AXIS_Z]] = mTask.cvt.sign[AXIS_Z] * raw_data[AXIS_Z];

    /* Out put the degree/second(mo/s) */
    data->sensType = SENS_TYPE_GYRO;
    data->x = (float)remap_data[AXIS_X] * DEGREE_TO_RADIRAN_SCALAR / mTask.gyroReso->sensitivity;
    data->y = (float)remap_data[AXIS_Y] * DEGREE_TO_RADIRAN_SCALAR / mTask.gyroReso->sensitivity;
    data->z = (float)remap_data[AXIS_Z] * DEGREE_TO_RADIRAN_SCALAR / mTask.gyroReso->sensitivity;
    mTask.gyroFactoryData.ix =
        (int32_t)((float)remap_data[AXIS_X] * GYROSCOPE_INCREASE_NUM_AP / mTask.gyroReso->sensitivity);
    mTask.gyroFactoryData.iy =
        (int32_t)((float)remap_data[AXIS_Y] * GYROSCOPE_INCREASE_NUM_AP / mTask.gyroReso->sensitivity);
    mTask.gyroFactoryData.iz =
        (int32_t)((float)remap_data[AXIS_Z] * GYROSCOPE_INCREASE_NUM_AP / mTask.gyroReso->sensitivity);


    times++;
    if (times >= 10) {
        times = 0;
#ifdef DEBUG
        osLog(LOG_INFO, "icm20600gyConvert gyro rawdata x:%f, y:%f, z:%f\n",
            (double)data->x, (double)data->y, (double)data->z);
#endif
    }

    return 0;
}

static int icm20600Convert(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                         void *inBuf, uint8_t inSize, uint8_t elemInSize,
                         void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    struct accGyroData *data = mTask.accGyroPacket.outBuf;
    uint8_t accEventSize = 0;
    uint8_t gyroEventSize = 0;
    uint64_t swSampleTime = 0, realSampleTime = 0;
    uint32_t delay;
    uint8_t targetSize;
    uint8_t lastAccIndex = -1, lastGyroIndex = -1;
    uint16_t fifoCount = 0;

///MEIZU_BSP{@
#ifdef DATA_NOT_FROM_FIFO
    uint8_t index=0;
#endif
///@}

#ifndef DATA_NOT_FROM_FIFO
    uint8_t N, i;
//    uint16_t index=0;
#endif
    uint8_t *databuf;

#ifdef DATA_NOT_FROM_FIFO
#ifdef DEBUG
    osLog(LOG_INFO, "%s dumpreg [23]:0x%.2x, [6a]:0x%.2x, [6b]:0x%.2x, [6c]:0x%.2x,\n",
          __func__,value[0],value[1],value[2],value[3]);
#endif

    if( mTask.accPowerOn ) {
        osLog(LOG_INFO, "%s ACCEL OUT REG, raw data, x:0x%.4x, y:0x%.4x, z:0x%.4x\n",__func__,
              (buffer[0]<<8)|buffer[1], (buffer[2]<<8)|buffer[3], (buffer[4]<<8)|buffer[5]);
        databuf = &buffer[0];
        icm20600gConvert(databuf, &data[0]);
        accEventSize++;
///MEIZU_BSP{@
        data[0].sensType = SENS_TYPE_ACCEL;
        index++;
///@}
    }
    if( mTask.gyroPowerOn ) {
        osLog(LOG_INFO, "%s GYRO OUT REG, raw data, x:0x%.4x, y:0x%.4x, z:0x%.4x\n",__func__,
              (buffer[6]<<8)|buffer[7], buffer[8]<<8|buffer[9], buffer[10]<<8|buffer[11]);
        databuf = &buffer[6];
#if 0
        icm20600gyConvert(databuf, &data[1]);
#else///MEIZU_BSP{@
        icm20600gyConvert(databuf, &data[index]);
        data[index].sensType = SENS_TYPE_GYRO;
#endif///@}
        gyroEventSize++;
    }
    txTransferDataInfo(&mTask.dataInfo, accEventSize, gyroEventSize, mTask.sampleTime, data);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);

#else
    /* If both sensor are enabled, but only one data is ready, drop it.
     * Otherwise, mTask.lastSampleTime will be updated,
     * and gyro add dummy data mechanism will be failed.
     */
    if (mTask.gyroFifoConfiged && mTask.accFifoConfiged) {
        fifoCount = mTask.fifoLen / ICM20608_DATA_LEN;
        fifoCount = (fifoCount / 2) * 2;
        mTask.fifoLen = fifoCount * ICM20608_DATA_LEN;
    }

    if ( (mTask.fifoLen <= 0) || (fifo_reset==1) ) {
        fifo_reset = 0;
        #ifdef FIFO_ISR
            mt_eint_unmask(mTask.hw->eint_num);
        #endif
        txTransferDataInfo(&mTask.dataInfo, 0, 0, mTask.sampleTime, data);
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
        osLog(LOG_INFO, "icm20600Convert mTask.fifoLen:%u, invalid, trans 0,reset fifo\n", mTask.fifoLen);
        return 0;
    }

    N = mTask.fifoLen / ICM20608_DATA_LEN;
    N = (N > MAX_RECV_PACKET) ? MAX_RECV_PACKET : N;
    for (i = 0; i < N; ++i) {
        databuf = &mTask.rxBuf[i * ICM20608_DATA_LEN];
        if (mTask.accFifoConfiged == true && mTask.gyroFifoConfiged == true) {
            if (i % 2 == 0) {
                accEventSize++;
                icm20600gConvert(databuf, &data[i]);
                lastAccIndex = i;
            } else {
                gyroEventSize++;
                icm20600gyConvert(databuf, &data[i]);
                lastGyroIndex = i;
            }
        } else if (mTask.accFifoConfiged == false && mTask.gyroFifoConfiged == true) {
            gyroEventSize++;
            icm20600gyConvert(databuf, &data[i]);
            lastGyroIndex = i;;
        } else if (mTask.accFifoConfiged == true && mTask.gyroFifoConfiged == false) {
            accEventSize++;
            icm20600gConvert(databuf, &data[i]);
            lastAccIndex = i;
        } else
            osLog(LOG_INFO, "icm20608Convert accFifoConfiged: %d, gyroFifoConfiged:%d\n",
            mTask.accFifoConfiged, mTask.gyroFifoConfiged);
    }
    if ((accEventSize + gyroEventSize) != N)
        osLog(LOG_INFO, "icm20608Convert wrong num accEventSize:%d, gyroEventSize:%d,total:%d\n",
            accEventSize, gyroEventSize, N);
    swSampleTime = mTask.swSampleTime;
    realSampleTime = calcFakeInterruptTime(swSampleTime, mTask.hwSampleTime, mTask.lastSampleTime,
        mTask.accRate, mTask.accFifoConfiged, accEventSize,
        mTask.gyroRate, mTask.gyroFifoConfiged, gyroEventSize);
    //osLog(LOG_INFO, "icm20600Convert, swSampleTime=%lld, hwSampleTime=%lld, realSampleTime=%lld, lastSampleTime=%lld, now=%lld\n",
        //swSampleTime, mTask.hwSampleTime, realSampleTime, mTask.lastSampleTime, rtcGetTime());
    //osLog(LOG_INFO, "icm20600Convert, accEventSize=%d, gyroEventSize=%d\n",
        //accEventSize, gyroEventSize);
    filterDataFifoUninterruptible(realSampleTime, mTask.lastSampleTime,
        mTask.accRate, mTask.accFifoConfiged, &accEventSize,
        mTask.gyroRate, mTask.gyroFifoConfiged, &gyroEventSize);

    /* add dummy data to prevent timestamp gap too large */
    delay = remapFifoDelay(mTask.minAccGyroDelay, mTask.timeBase);
    targetSize = (realSampleTime - mTask.lastSampleTime) / delay;
    if (mTask.accFifoConfiged) {
        if (mTask.accFirstData == true)
            mTask.accFirstData = false;
        else {
            for (; i < MAX_RECV_PACKET && accEventSize < targetSize; i++) {
                /* osLog(LOG_INFO, "%s, add acc dummy data, %d -> %d, %d", __func__, accEventSize, targetSize, lastAccIndex); */
                data[i] = data[lastAccIndex];
                accEventSize++;
            }
        }
    }
    if (mTask.gyroFifoConfiged) {
        if (mTask.gyroFirstData == true)
            mTask.gyroFirstData = false;
        else {
            for (; i < MAX_RECV_PACKET && gyroEventSize < targetSize; i++) {
                /* osLog(LOG_INFO, "%s, add gyro dummy data, %d -> %d, %d", __func__, gyroEventSize, targetSize, lastGyroIndex); */
                data[i] = data[lastGyroIndex];
                gyroEventSize++;
            }
        }
    }
    if (i == MAX_RECV_PACKET)
        osLog(LOG_INFO, "SW buffer is full.");
    //osLog(LOG_INFO, "icm20600Convert, resize accEventSize=%d, gyroEventSize=%d\n",
        //accEventSize, gyroEventSize);
    mTask.hwSampleTime = realSampleTime;
    mTask.lastSampleTime = realSampleTime;
    txTransferDataInfo(&mTask.dataInfo, accEventSize, gyroEventSize, realSampleTime, data);

#ifdef FIFO_ISR
    mt_eint_unmask(mTask.hw->eint_num);
#endif
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
#endif
    return 0;
}

static struct sensorFsm icm20600Fsm[] = {
    /* sample */
    sensorFsmCmd(STATE_SAMPLE, STATE_FIFO, icm20600Sample),
    sensorFsmCmd(STATE_FIFO, STATE_CONVERT, icm20600ReadFifo),
    sensorFsmCmd(STATE_CONVERT, STATE_SAMPLE_DONE, icm20600Convert),

    /* acc enable state */
    sensorFsmCmd(STATE_ACC_ENABLE, STATE_ACC_ENABLE_DONE, icm20600gEnable),
    // code below is useless now
    sensorFsmCmd(STATE_FIFOREG2_R_1, STATE_ACC_ENFIFO_W, icm20600FifoReg2Read),
    sensorFsmCmd(STATE_ACC_ENFIFO_W, STATE_ACC_ENABLE_DONE, icm20600gFifoEnable),
    sensorFsmCmd(STATE_ENPOWER_W, STATE_TURN_R, icm20600PowerEnableWrite),
    sensorFsmCmd(STATE_TURN_R, STATE_ACC_TURNON_W, icm20600TurnRead),
    sensorFsmCmd(STATE_ACC_TURNON_W, STATE_ACC_ENABLE_DONE, icm20600gTurnOnWrite),

    /* acc disable state */
    sensorFsmCmd(STATE_ACC_DISABLE, STATE_ACC_TURNOFF_W, icm20600gDisable),
    sensorFsmCmd(STATE_ACC_TURNOFF_W, STATE_REG_USER_CTRL_R, icm20600gTurnOffWrite),
    sensorFsmCmd(STATE_REG_USER_CTRL_R, STATE_DEFIFO_W, icm20600RegUserCTRLRead),
    sensorFsmCmd(STATE_DEFIFO_W, STATE_GYRO_RATE, icm20600FifoDisable),
    sensorFsmCmd(STATE_GYRO_RATE, STATE_FIFOREG2_R_1, icm20600gySetRate),
    sensorFsmCmd(STATE_FIFOREG2_R_1, STATE_ACC_DEFIFO_W, icm20600FifoReg2Read),
    sensorFsmCmd(STATE_ACC_DEFIFO_W, STATE_REG_USER_CTRL_R_AGAIN, icm20600gFifoDisable),
    sensorFsmCmd(STATE_REG_USER_CTRL_R_AGAIN, STATE_FIFO_RST, icm20600RegUserCTRLRead),
    sensorFsmCmd(STATE_FIFO_RST, STATE_REG_USER_CTRL_R_3RD_TIMES, icm20600FifoReset),
    sensorFsmCmd(STATE_REG_USER_CTRL_R_3RD_TIMES, STATE_ENFIFO_W, icm20600RegUserCTRLRead),
    sensorFsmCmd(STATE_ENFIFO_W, STATE_POWER_R, icm20600FifoEnable),
    sensorFsmCmd(STATE_POWER_R, STATE_DEPOWER_W, icm20600PowerRead),
    sensorFsmCmd(STATE_DEPOWER_W, STATE_ACC_DISABLE_DONE, icm20600PowerDisableWrite),

    /* acc rate state */
    sensorFsmCmd(STATE_ACC_RATECHG, STATE_REG_USER_CTRL_R, icm20600gRate),
    sensorFsmCmd(STATE_REG_USER_CTRL_R, STATE_DEFIFO_W, icm20600RegUserCTRLRead),
    sensorFsmCmd(STATE_DEFIFO_W, STATE_ACC_RATE, icm20600FifoDisable),
    sensorFsmCmd(STATE_ACC_RATE, STATE_REG_USER_CTRL_R_AGAIN, icm20600gSetRate),
    sensorFsmCmd(STATE_REG_USER_CTRL_R_AGAIN, STATE_FIFO_RST, icm20600RegUserCTRLRead),
    sensorFsmCmd(STATE_FIFO_RST, STATE_FIFOREG2_R, icm20600FifoReset),
    sensorFsmCmd(STATE_FIFOREG2_R, STATE_ACC_ENFIFO_W, icm20600FifoReg2Read),
    sensorFsmCmd(STATE_ACC_ENFIFO_W, STATE_REG_USER_CTRL_R_3RD_TIMES, icm20600gFifoEnable),
    sensorFsmCmd(STATE_REG_USER_CTRL_R_3RD_TIMES, STATE_ENFIFO_W, icm20600RegUserCTRLRead),
    sensorFsmCmd(STATE_ENFIFO_W, STATE_UNMASK_IRQ, icm20600FifoEnable),
    sensorFsmCmd(STATE_UNMASK_IRQ, STATE_ACC_RATECHG_DONE, icm20600RateChangeDone),



    /* gyro enable state */
    sensorFsmCmd(STATE_GYRO_ENABLE, STATE_GYRO_ENABLE_DONE, icm20600gyEnable),
    // code below is useless now
    sensorFsmCmd(STATE_FIFOREG2_R, STATE_GYRO_ENFIFO_W, icm20600FifoReg2Read),
    sensorFsmCmd(STATE_GYRO_ENFIFO_W, STATE_GYRO_ENABLE_DONE, icm20600gyFifoEnable),
    sensorFsmCmd(STATE_ENPOWER_W, STATE_TURN_R, icm20600PowerEnableWrite),
    sensorFsmCmd(STATE_TURN_R, STATE_GYRO_TURNON_W, icm20600TurnRead),
    sensorFsmCmd(STATE_GYRO_TURNON_W, STATE_GYRO_ENABLE_DONE, icm20600gyTurnOnWrite),

    /* gyro disable state */
    sensorFsmCmd(STATE_GYRO_DISABLE, STATE_GYRO_TURNOFF_W, icm20600gyDisable),
    sensorFsmCmd(STATE_GYRO_TURNOFF_W, STATE_REG_USER_CTRL_R, icm20600gyTurnOffWrite),
    sensorFsmCmd(STATE_REG_USER_CTRL_R, STATE_DEFIFO_W, icm20600RegUserCTRLRead),
    sensorFsmCmd(STATE_DEFIFO_W, STATE_ACC_RATE, icm20600FifoDisable),
    sensorFsmCmd(STATE_ACC_RATE, STATE_FIFOREG2_R, icm20600gSetRate),
    sensorFsmCmd(STATE_FIFOREG2_R, STATE_ACC_DEFIFO_W, icm20600FifoReg2Read),
    sensorFsmCmd(STATE_ACC_DEFIFO_W, STATE_REG_USER_CTRL_R_AGAIN, icm20600gyFifoDisable),
    sensorFsmCmd(STATE_REG_USER_CTRL_R_AGAIN, STATE_FIFO_RST, icm20600RegUserCTRLRead),
    sensorFsmCmd(STATE_FIFO_RST, STATE_REG_USER_CTRL_R_3RD_TIMES, icm20600FifoReset),
    sensorFsmCmd(STATE_REG_USER_CTRL_R_3RD_TIMES, STATE_ENFIFO_W, icm20600RegUserCTRLRead),
    sensorFsmCmd(STATE_ENFIFO_W, STATE_POWER_R, icm20600FifoEnable),
    sensorFsmCmd(STATE_POWER_R, STATE_DEPOWER_W, icm20600PowerRead),
    sensorFsmCmd(STATE_DEPOWER_W, STATE_GYRO_DISABLE_DONE, icm20600PowerDisableWrite),

    /* gyro rate state */
    sensorFsmCmd(STATE_GYRO_RATECHG, STATE_REG_USER_CTRL_R, icm20600gyRate),
    sensorFsmCmd(STATE_REG_USER_CTRL_R, STATE_DEFIFO_W, icm20600RegUserCTRLRead),
    sensorFsmCmd(STATE_DEFIFO_W, STATE_GYRO_RATE, icm20600FifoDisable),
    sensorFsmCmd(STATE_GYRO_RATE, STATE_REG_USER_CTRL_R_AGAIN, icm20600gySetRate),
    sensorFsmCmd(STATE_REG_USER_CTRL_R_AGAIN, STATE_FIFO_RST, icm20600RegUserCTRLRead),
    sensorFsmCmd(STATE_FIFO_RST, STATE_FIFOREG2_R, icm20600FifoReset),
    sensorFsmCmd(STATE_FIFOREG2_R, STATE_GYRO_ENFIFO_W, icm20600FifoReg2Read),
    sensorFsmCmd(STATE_GYRO_ENFIFO_W, STATE_REG_USER_CTRL_R_3RD_TIMES, icm20600gyFifoEnable),
    sensorFsmCmd(STATE_REG_USER_CTRL_R_3RD_TIMES, STATE_ENFIFO_W, icm20600RegUserCTRLRead),
    sensorFsmCmd(STATE_ENFIFO_W, STATE_UNMASK_IRQ, icm20600FifoEnable),
    sensorFsmCmd(STATE_UNMASK_IRQ, STATE_GYRO_RATECHG_DONE, icm20600RateChangeDone),

    /* init state */
    sensorFsmCmd(STATE_RESET_R, STATE_RESET_W, icm20600ResetRead),
    sensorFsmCmd(STATE_RESET_W, STATE_WAIT_RESET_DONE, icm20600ResetWrite),
    sensorFsmCmd(STATE_WAIT_RESET_DONE, STATE_DEVID, icm20600WaitResetDone),
    sensorFsmCmd(STATE_DEVID, STATE_INIT_POWER_ON, icm20600DeviceId),
    sensorFsmCmd(STATE_INIT_POWER_ON, STATE_SELCLK_R, icm20600PowerEnableWrite),
    sensorFsmCmd(STATE_SELCLK_R, STATE_SELCLK_W, icm20600SelClkRead),
    sensorFsmCmd(STATE_SELCLK_W, STATE_TURN_R, icm20600SelClkWrite),
    //sensorFsmCmd(STATE_POWER_R, STATE_DEPOWER_W, icm20600PowerRead),
    //sensorFsmCmd(STATE_DEPOWER_W, STATE_ENPOWER_W, icm20600PowerDisableWrite),
    //sensorFsmCmd(STATE_ENPOWER_W, STATE_DUTY_CYCLE, icm20600PowerEnableWrite), /* Unnecessary, default is normal mode */
    sensorFsmCmd(STATE_TURN_R, STATE_ACC_TURNOFF_W, icm20600TurnRead),
    sensorFsmCmd(STATE_ACC_TURNOFF_W, STATE_TURN_R_AGAIN, icm20600gTurnOffWrite),
    sensorFsmCmd(STATE_TURN_R_AGAIN, STATE_GYRO_TURNOFF_W, icm20600TurnRead),
    sensorFsmCmd(STATE_GYRO_TURNOFF_W, STATE_ACC_CONFIG1_R, icm20600gyTurnOffWrite),
    sensorFsmCmd(STATE_ACC_CONFIG1_R, STATE_ACC_CONFIG1_W, icm20600AccConfig1Read),
    sensorFsmCmd(STATE_ACC_CONFIG1_W, STATE_ACC_CONFIG2_R, icm20600AccConfig1Write),
    sensorFsmCmd(STATE_ACC_CONFIG2_R, STATE_ACC_CONFIG2_W, icm20600AccConfig2Read),
    sensorFsmCmd(STATE_ACC_CONFIG2_W, STATE_GYRO_CONFIG1_R, icm20600AccConfig2Write),
    sensorFsmCmd(STATE_GYRO_CONFIG1_R, STATE_GYRO_CONFIG1_W, icm20600GyroConfig1Read),
    sensorFsmCmd(STATE_GYRO_CONFIG1_W, STATE_GYRO_CONFIG2_R, icm20600GyroConfig1Write),
    sensorFsmCmd(STATE_GYRO_CONFIG2_R, STATE_GYRO_CONFIG2_W, icm20600GyroConfig2Read),
    sensorFsmCmd(STATE_GYRO_CONFIG2_W, STATE_REG_USER_CTRL_R, icm20600GyroConfig2Write),
    sensorFsmCmd(STATE_REG_USER_CTRL_R, STATE_ENFIFO_W, icm20600RegUserCTRLRead),
    sensorFsmCmd(STATE_ENFIFO_W, STATE_REG_USER_CTRL_R_AGAIN, icm20600FifoEnable),
    sensorFsmCmd(STATE_REG_USER_CTRL_R_AGAIN, STATE_FIFO_RST, icm20600RegUserCTRLRead),
    sensorFsmCmd(STATE_FIFO_RST, STATE_DATA_RESO, icm20600FifoReset),
#if 0
    sensorFsmCmd(STATE_FIFOREG2_R, STATE_GYRO_ENFIFO_W, icm20600FifoReg2Read),
    sensorFsmCmd(STATE_GYRO_ENFIFO_W, STATE_FIFOREG2_R_1, icm20600gyFifoEnable),
    sensorFsmCmd(STATE_FIFOREG2_R_1, STATE_ACC_ENFIFO_W, icm20600FifoReg2Read),
    sensorFsmCmd(STATE_ACC_ENFIFO_W, STATE_DATA_RESO, icm20600gFifoEnable),
#endif
    sensorFsmCmd(STATE_DATA_RESO, STATE_CALC_RESO, icm20600DataReso),
    sensorFsmCmd(STATE_CALC_RESO, STATE_TIMEBASE, icm20600CalcReso),
    sensorFsmCmd(STATE_TIMEBASE, STATE_INT_MODE, icm20600ReadTimeBase),  //quset

    sensorFsmCmd(STATE_INT_MODE, STATE_INT_PIN_CFG, icm20600SetIntMode),
    sensorFsmCmd(STATE_INT_PIN_CFG, STATE_POWER_R, icm20600IntPinCfg),

    sensorFsmCmd(STATE_POWER_R, STATE_DEPOWER_W, icm20600PowerRead),
    sensorFsmCmd(STATE_DEPOWER_W, STATE_CORE, icm20600PowerDisableWrite),
    sensorFsmCmd(STATE_CORE, STATE_INIT_DONE, icm20600RegisterCore),
};
static void i2cAutoDetect(void *cookie, size_t tx, size_t rx, int err)
{
    if (err == 0) {
        if( mTask.deviceId==ICM20608D_WHOAMI )
            osLog(LOG_INFO, "icm20600: auto detect success:0x%x\n", mTask.deviceId);
        //registerAccGyroInterruptMode(ACC_GYRO_FIFO_UNINTERRUPTIBLE);

        registerAccGyroInterruptMode(ACC_GYRO_FIFO_UNINTERRUPTIBLE);
        registerAccGyroDriverFsm(icm20600Fsm, ARRAY_SIZE(icm20600Fsm));
        registerAccGyroTimerCbk(icm20600TimerCbkF);
    } else
        osLog(LOG_ERROR, "icm20600: auto detect error (%d)\n", err);
}

int icm20600Init(void)
{
    int ret = 0;

    cali_reset_done = 0;
    icm20600DebugPoint = &mTask;
    insertMagicNum(&mTask.accGyroPacket);
    mTask.hw = get_cust_accGyro("icm20608");
    if (NULL == mTask.hw) {
        osLog(LOG_ERROR, "get_cust_acc_hw fail\n");
        return 0;
    }

    mTask.accPowerOn = false;
    mTask.gyroPowerOn = false;

    mTask.i2c_addr = mTask.hw->i2c_addr[0];
    osLog(LOG_ERROR, "acc i2c_num: %d, i2c_addr: 0x%x\n", mTask.hw->i2c_num, mTask.i2c_addr);

    mTask.gyroWaterMark = mTask.accWaterMark = 1024;
    osLog(LOG_INFO, "icm20600 set default watermark to %d \n", mTask.accWaterMark);

    if (0 != (ret = sensorDriverGetConvert(mTask.hw->direction, &mTask.cvt))) {
        osLog(LOG_ERROR, "invalid direction: %d\n", mTask.hw->direction);
    }
    osLog(LOG_ERROR, "acc map[0]:%d, map[1]:%d, map[2]:%d, sign[0]:%d, sign[1]:%d, sign[2]:%d\n\r",
        mTask.cvt.map[AXIS_X], mTask.cvt.map[AXIS_Y], mTask.cvt.map[AXIS_Z],
        mTask.cvt.sign[AXIS_X], mTask.cvt.sign[AXIS_Y], mTask.cvt.sign[AXIS_Z]);
    i2cMasterRequest(mTask.hw->i2c_num, I2C_SPEED);

    mTask.txBuf[0] = ICM20600_REG_DEVID;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         &mTask.deviceId, 1, i2cAutoDetect, NULL);
}
MODULE_DECLARE(icm20600, SENS_TYPE_ACCEL, icm20600Init);
