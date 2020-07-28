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
#include <seos.h>
#include <util.h>
#include <sensors.h>
#include <plat/inc/rtc.h>
#include <contexthub_core.h>
#include <mt_gpt.h>
#include <timer.h>
//#include "eint.h"
#include "vcnl36832.h"
#include "alsps_separate.h"
#include "alsps.h"

#define I2C_SPEED   400000
#define PS_NAME     "fakeAls"

enum fakeAlsState {
    STATE_INIT_DONE        = CHIP_PS_RESET,
    STATE_RESET            = CHIP_ALS_RESET,

};

static int fakeAls_register_core(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                                 void *inBuf, uint8_t inSize, uint8_t elemInSize,
                                 void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    sensorFsmEnqueueFakeI2cEvt(i2cCallBack, next_state, SUCCESS_EVT);
    return 0;
}

static struct sensorFsm fakeAlsFsm[] = {
    /* init state */
    sensorFsmCmd(STATE_RESET, STATE_INIT_DONE, fakeAls_register_core),
};

static int fakeAlsInit(void)
{
    int ret = 0;

    osLog(LOG_INFO, "fakeAls: auto detect success!\n");

    psSensorRegister();
    registerAlsSeparateFsm(fakeAlsFsm, ARRAY_SIZE(fakeAlsFsm));
    return ret;
}

#ifndef CFG_OVERLAY_INIT_SUPPORT
MODULE_DECLARE(fakeAls, SENS_TYPE_ALS, fakeAlsInit);
#else
#include "mtk_overlay_init.h"
OVERLAY_DECLARE(fakeAls, OVERLAY_ID_ALSPS, fakeAlsInit);
#endif

