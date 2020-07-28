/* MediaTek Inc. (C) 2015. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <stdbool.h>
#include <stdint.h>
#include <seos.h>
#include <util.h>
#include <plat/inc/rtc.h>
#include <sensors.h>

#include <sar.h>
#include <contexthub_core.h>
#include <cust_sar.h>
#include <sx9310.h>
#include <timer.h>
#include "cache_internal.h"

#define SAR_NAME                        "sx9310"
#define I2C_SPEED                       400000

enum SX9310State {
    STATE_CHECK_INT = CHIP_SAR_SAMPLING,
    STATE_CONVERT = CHIP_SAR_CONVERT,
    STATE_SAMPLE_DONE = CHIP_SAR_SAMPLING_DONE,
    STATE_ENABLE = CHIP_SAR_ENABLE,
    STATE_ENABLE_DONE = CHIP_SAR_ENABLE_DONE,
    STATE_DISABLE = CHIP_SAR_DISABLE,
    STATE_DISABLE_DONE = CHIP_SAR_DISABLE_DONE,
    STATE_RATECHG = CHIP_SAR_RATECHG,
    STATE_RATECHG_DONE = CHIP_SAR_RATECHG_DONE,
    STATE_CALI = CHIP_SAR_CALI,
    STATE_CALI_DONE = CHIP_SAR_CALI_DONE,
    STATE_INIT_DONE = CHIP_INIT_DONE,
    STATE_IDLE = CHIP_IDLE,
    STATE_RESET = CHIP_RESET,         //15

    STATE_CORE,//16
    STATE_DEALY_1,//17
    STATE_REG_INIT_1,//18
    STATE_REG_INIT_2,//19
    STATE_REG_INIT_3,//20
    STATE_REG_INIT_4,//21
    STATE_DEALY_4,//22
    STATE_SAMPLE_0,
    STATE_SAMPLE_1,
};

SRAM_REGION_BSS static struct SX9310Task {
    /* txBuf for i2c operation, fill register and fill value */
    uint8_t txBuf[8];
    uint8_t rxBuf[8];
    uint8_t sarIntStatus;

    uint8_t i2c_addr;
    uint8_t chipId;
    struct sar_hw *hw;

    uint32_t timerHandle;
    /* rxBuf for i2c operation, receive rawdata */
    struct transferDataInfo dataInfo;
    struct SarData_t data;

    /* data for factory */
    struct TripleAxisDataPoint factoryData;
} mTask;
//static struct SX9310Task *sx9310DebugPoint;

static int sx9310CheckHwState(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
    void *inBuf, uint8_t inSize, uint8_t elemInSize,
    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    //osLog(LOG_ERROR, "%s, time:%lld\n", __func__, rtcGetTime());

    if (rxTransferDataInfo(&mTask.dataInfo, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize)) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "sar rx error\n");
        return -1;
    }
    mTask.txBuf[0] = SX9310_STAT0_REG;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
                         mTask.rxBuf, 1, i2cCallBack, next_state);
}

static int sx9310SampleCh0(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
    void *inBuf, uint8_t inSize, uint8_t elemInSize,
    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    //osLog(LOG_ERROR, "%s, time:%lld\n", __func__, rtcGetTime());

    mTask.sarIntStatus = mTask.rxBuf[0];
    osLog(LOG_ERROR, "sar int status: %x\n", mTask.rxBuf[0]);

    // TODO: this is sample code, you can read any channel data that you want to use

    mTask.txBuf[0] = SX9310_CPSRD;
    mTask.txBuf[1] = 0x00 + 0;//channel 0

    i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
            NULL, 0, NULL, NULL);

    mTask.txBuf[0] = SX9310_USEMSB;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
            mTask.rxBuf, 8, NULL, NULL);
}

static int sx9310SampleCh1(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
    void *inBuf, uint8_t inSize, uint8_t elemInSize,
    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    //osLog(LOG_ERROR, "%s, time:%lld\n", __func__, rtcGetTime());
    // TODO: this is sample code, you can read any channel data that you want to use
    //osLog(LOG_ERROR, "sar ch0 useful data: %d avg: %d diff: %d offset: %d\n",
    //    (int16_t)((mTask.rxBuf[0] << 8) | mTask.rxBuf[1]),
    //    (int16_t)((mTask.rxBuf[2] << 8) | mTask.rxBuf[3]),
    //    (int16_t)((mTask.rxBuf[4] << 8) | mTask.rxBuf[5]),
    //    (int16_t)((mTask.rxBuf[6] << 8) | mTask.rxBuf[7]));
    mTask.factoryData.ix = (int32_t)((mTask.rxBuf[0] << 8) | mTask.rxBuf[1]);
    mTask.factoryData.iy = (int32_t)((mTask.rxBuf[2] << 8) | mTask.rxBuf[3]);
    mTask.factoryData.iz = (int32_t)((mTask.rxBuf[4] << 8) | mTask.rxBuf[5]);

    mTask.txBuf[0] = SX9310_CPSRD;
    mTask.txBuf[1] = 0x00 + 1;

    i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
            NULL, 0, NULL, NULL);

    mTask.txBuf[0] = SX9310_USEMSB;
    return i2cMasterTxRx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
            mTask.rxBuf, 8, NULL, NULL);
}

static int sx9310Convert(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
    void *inBuf, uint8_t inSize, uint8_t elemInSize,
    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    //osLog(LOG_ERROR, "%s, time:%lld\n", __func__, rtcGetTime());

    //osLog(LOG_ERROR, "sar ch1 useful data: %d avg: %d diff: %d offset: %d\n",
    //    (int16_t)((mTask.rxBuf[0] << 8) | mTask.rxBuf[1]),
    //    (int16_t)((mTask.rxBuf[2] << 8) | mTask.rxBuf[3]),
    //    (int16_t)((mTask.rxBuf[4] << 8) | mTask.rxBuf[5]),
    //    (int16_t)((mTask.rxBuf[6] << 8) | mTask.rxBuf[7]));

    if (mTask.sarIntStatus)
        mTask.data.sarState = SAR_STATE_NEAR;
    else
        mTask.data.sarState = SAR_STATE_FAR;

    mTask.data.sarData[0] = mTask.sarIntStatus;
    //mTask.data.sarData[1] = 1;
    //mTask.data.sarData[2] = 2;
    mTask.data.sensType = SENS_TYPE_SAR;

    txTransferDataInfo(&mTask.dataInfo, 1, &mTask.data);
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}


static int sx9310Enable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
    void *inBuf, uint8_t inSize, uint8_t elemInSize,
    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    //osLog(LOG_ERROR, "%s, time:%lld\n", __func__, rtcGetTime());

    mTask.txBuf[0] = SX9310_CPS_CTRL0_REG;
    mTask.txBuf[1] = 0x20 | 0x07;
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
        i2cCallBack, next_state);
}

static int sx9310Disable(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
    void *inBuf, uint8_t inSize, uint8_t elemInSize,
    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    //osLog(LOG_ERROR, "%s, time:%lld\n", __func__, rtcGetTime());

    mTask.txBuf[0] = SX9310_CPS_CTRL0_REG;
    mTask.txBuf[1] = 0x20 & (~0x07);
    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
        i2cCallBack, next_state);
}

static int sx9310Rate(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
    void *inBuf, uint8_t inSize, uint8_t elemInSize,
    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    //osLog(LOG_ERROR, "%s, time:%lld\n", __func__, rtcGetTime());

    int ret = 0;
    struct SarCntlPacket cntlPacket = {0};

    ret = rxControlInfo(&cntlPacket, inBuf, inSize, elemInSize, outBuf, outSize, elemOutSize);
    if (ret < 0) {
        sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, ERROR_EVT);
        osLog(LOG_ERROR, "sx9310Rate, rx inSize and elemSize error\n");
        return -1;
    }

    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int sx9310Cali(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
    void *inBuf, uint8_t inSize, uint8_t elemInSize,
    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    //osLog(LOG_ERROR, "%s, time:%lld\n", __func__, rtcGetTime());
    //void sarSendCalibrationResult(uint8_t sensorType,
            //int32_t bias[3], int8_t status); //status:0 sucess;-1 fail

    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static int sx9310Reset(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
    void *inBuf, uint8_t inSize, uint8_t elemInSize,
    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    //osLog(LOG_ERROR, "%s, time:%lld\n", __func__, rtcGetTime());

    mTask.txBuf[0] = SX9310_SOFTRESET_REG;
    mTask.txBuf[1] = SX9310_SOFTRESET;

    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
        i2cCallBack, next_state);
}
static int sx9310Init_1(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
    void *inBuf, uint8_t inSize, uint8_t elemInSize,
    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    //osLog(LOG_ERROR, "%s, time:%lld\n", __func__, rtcGetTime());

    mTask.txBuf[0] = SX9310_CPS_CTRL1_REG;
    mTask.txBuf[1] = 0x00;
    mTask.txBuf[2] = 0x04;
    mTask.txBuf[3] = 0x0A;
    mTask.txBuf[4] = 0x0D;
    mTask.txBuf[5] = 0xC1;
    mTask.txBuf[6] = 0x20;
    mTask.txBuf[7] = 0x4C;

    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 8,
        i2cCallBack, next_state);
}

static int sx9310Init_2(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
    void *inBuf, uint8_t inSize, uint8_t elemInSize,
    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    //osLog(LOG_ERROR, "%s, time:%lld\n", __func__, rtcGetTime());

    mTask.txBuf[0] = SX9310_CPS_CTRL8_REG;
    mTask.txBuf[1] = 0x7E;
    mTask.txBuf[2] = 0x7D;
    mTask.txBuf[3] = 0x00;
    mTask.txBuf[4] = 0x00;
    mTask.txBuf[5] = 0x00;
    mTask.txBuf[6] = 0x00;
    mTask.txBuf[7] = 0x00;

    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 8,
        i2cCallBack, next_state);
}

static int sx9310Init_3(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
    void *inBuf, uint8_t inSize, uint8_t elemInSize,
    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    //osLog(LOG_ERROR, "%s, time:%lld\n", __func__, rtcGetTime());

    mTask.txBuf[0] = SX9310_CPS_CTRL15_REG;
    mTask.txBuf[1] = 0x00;
    mTask.txBuf[2] = 0x00;
    mTask.txBuf[3] = 0x04;
    mTask.txBuf[4] = 0x00;
    mTask.txBuf[5] = 0x00;
    mTask.txBuf[6] = 0x00;
    mTask.txBuf[7] = 0x80;

    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 8,
        i2cCallBack, next_state);
}

static int sx9310Init_4(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
    void *inBuf, uint8_t inSize, uint8_t elemInSize,
    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    //osLog(LOG_ERROR, "%s, time:%lld\n", __func__, rtcGetTime());

    mTask.txBuf[0] = SX9310_SAR_CTRL2_REG;
    mTask.txBuf[1] = 0x0C;

    i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 2,
            NULL, NULL);

    mTask.txBuf[2] = SX9310_CPS_CTRL0_REG;
    mTask.txBuf[3] = 0x20;

    return i2cMasterTx(mTask.hw->i2c_num, mTask.i2c_addr, &mTask.txBuf[2], 2,
        i2cCallBack, next_state);
}

static int sx9310Delay(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
    void *inBuf, uint8_t inSize, uint8_t elemInSize,
    void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    uint64_t timerDelay = 1000000000ull / 100;
   // osLog(LOG_ERROR, "%s, time:%lld\n", __func__, rtcGetTime());

    if (mTask.timerHandle)
        timTimerCancel(mTask.timerHandle);
    mTask.timerHandle = timTimerSet(timerDelay, 0, 50, delayCallback, next_state, true);
    if (!mTask.timerHandle)
        configASSERT(0);

    return 0;
}

static void sarGetSensorInfo(struct sensorInfo_t *data)
{
    strncpy(data->name, SAR_NAME, sizeof(data->name));
}

static void sarGetData(void *sample) {
    struct TripleAxisDataPoint *tripleSample = (struct TripleAxisDataPoint *)sample;
    tripleSample->ix = mTask.factoryData.ix;
    tripleSample->iy = mTask.factoryData.iy;
    tripleSample->iz = mTask.factoryData.iz;
}

static int sx9310RegisterCore(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                void *outBuf, uint8_t *outSize, uint8_t *elemOutSize) {
    //osLog(LOG_ERROR, "%s, time:%lld\n", __func__, rtcGetTime());

    struct sensorCoreInfo mInfo;
    memset(&mInfo, 0x00, sizeof(struct sensorCoreInfo));
    /* Register sensor Core */
    mInfo.sensType = SENS_TYPE_SAR;
    mInfo.getData = sarGetData;
    mInfo.getSensorInfo = sarGetSensorInfo;
    sensorCoreRegister(&mInfo);

    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static struct sensorFsm sx9310Fsm[] = {
    sensorFsmCmd(STATE_CHECK_INT, STATE_SAMPLE_0, sx9310CheckHwState),
    sensorFsmCmd(STATE_SAMPLE_0, STATE_SAMPLE_1, sx9310SampleCh0),
    sensorFsmCmd(STATE_SAMPLE_1, STATE_CONVERT, sx9310SampleCh1),
    sensorFsmCmd(STATE_CONVERT, STATE_SAMPLE_DONE, sx9310Convert),

    sensorFsmCmd(STATE_ENABLE, STATE_ENABLE_DONE, sx9310Enable),
    sensorFsmCmd(STATE_DISABLE, STATE_DISABLE_DONE, sx9310Disable),

    sensorFsmCmd(STATE_RATECHG, STATE_RATECHG_DONE, sx9310Rate),

    sensorFsmCmd(STATE_CALI, STATE_CALI_DONE, sx9310Cali),

    sensorFsmCmd(STATE_RESET, STATE_DEALY_1, sx9310Reset),
    sensorFsmCmd(STATE_DEALY_1, STATE_REG_INIT_1, sx9310Delay),
    sensorFsmCmd(STATE_REG_INIT_1, STATE_REG_INIT_2, sx9310Init_1),
    sensorFsmCmd(STATE_REG_INIT_2, STATE_REG_INIT_3, sx9310Init_2),
    sensorFsmCmd(STATE_REG_INIT_3, STATE_REG_INIT_4, sx9310Init_3),
    sensorFsmCmd(STATE_REG_INIT_4, STATE_DEALY_4, sx9310Init_4),
    sensorFsmCmd(STATE_DEALY_4, STATE_CORE, sx9310Delay),
    sensorFsmCmd(STATE_CORE, STATE_INIT_DONE, sx9310RegisterCore),
};


static int sx9310Init(void)
{
    int ret = 0;

    mTask.hw = get_cust_sar("sx9310");
    if (NULL == mTask.hw) {
        osLog(LOG_ERROR, "sx9310 get_cust_sar fail\n");
        ret = -1;
        goto err_out;
    }
    mTask.i2c_addr = mTask.hw->i2c_addr[0];
    osLog(LOG_ERROR, "sar i2c_num: %d, i2c_addr: 0x%x\n", mTask.hw->i2c_num, mTask.i2c_addr);

    i2cMasterRequest(mTask.hw->i2c_num, I2C_SPEED);
    mTask.txBuf[0] = SX9310_WHOAMI_REG;


    for (uint8_t i = 0; i < 3; i++) {
        ret = i2cMasterTxRxSync(mTask.hw->i2c_num, mTask.i2c_addr, mTask.txBuf, 1,
            &mTask.chipId, 1, NULL, NULL);

        if (ret >= 0 && mTask.chipId == SX9310_WHOAMI_VALUE) {
            osLog(LOG_INFO, "sx9310 auto detect success %x\n", mTask.chipId);
            goto success_out;
        } else
            ret = -1;
    }

    if (ret < 0) {
        ret = -1;
        osLog(LOG_INFO, "sx9310 id fail: %x\n", mTask.chipId);
        i2cMasterRelease(mTask.hw->i2c_num);
        goto err_out;
    }

success_out:
    sarSensorRegister();
    registerSarDriverFsm(sx9310Fsm, ARRAY_SIZE(sx9310Fsm));
err_out:
    return ret;
}

#ifndef CFG_OVERLAY_INIT_SUPPORT
MODULE_DECLARE(sx9310, SENS_TYPE_SAR, sx9310Init);
#else
#include "mtk_overlay_init.h"
OVERLAY_DECLARE(sx9310, OVERLAY_ID_SAR, sx9310Init);
#endif
