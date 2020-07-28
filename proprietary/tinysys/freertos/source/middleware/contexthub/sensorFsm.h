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
#ifndef _SENSORFSM_H_
#define _SENSORFSM_H_
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <stdbool.h>
#include <stdint.h>
#include <i2c.h>
#include <spi.h>

struct sensorFsm {
    void *state;
    void *next_state;
    int (*operation)(I2cCallbackF i2cCallBack, SpiCbkF spiCallBack, void *next_state,
                     void *inBuf, uint8_t inSize, uint8_t elemInSize,
                     void *outBuf, uint8_t *outSize, uint8_t *elemOutSize);
};
struct transferDataInfo {
    void *outBuf;
    uint8_t *outSize;
    uint8_t *elemOutSize;
    void *inBuf;
    uint8_t inSize;
    uint8_t elemInSize;
};
struct fsmCurrInfo {
    char *moduleName;
    struct sensorFsm *mSensorFsm;
    struct sensorFsm *mCurrFsm;
    struct sensorFsm *mNextFsm;
    uint8_t mSensorFsmSize;
    uint8_t mCurrFsmSize;
};

#define sensorFsmCmd(_state, _next_state, _operation) \
    {.state = (void *)_state, .next_state = (void *)_next_state, .operation = _operation}
const struct sensorFsm *sensorFsmFindCmd(const struct sensorFsm * mFsm,
    uint32_t sensorFsmSize, const void *state);
int sensorFsmRunState(struct transferDataInfo *dataInfo, struct fsmCurrInfo *fsmInfo,
    const void *state, I2cCallbackF i2cCallBack, SpiCbkF spiCallBack);
void sensorFsmSwitchState(struct transferDataInfo *dataInfo, struct fsmCurrInfo *fsmInfo,
    I2cCallbackF i2cCallBack, SpiCbkF spiCallBack);
void sensorFsmReleaseState(struct fsmCurrInfo *fsmInfo);
#define SUCCESS_EVT  0
#define ERROR_EVT   -1
/* sensor fsm driver generally need call i2c txrx or spi txrx, if some fsm don't call i2c or spi,
 * we should call sensorFsmEnqueueFakeI2cEvt or sensorFsmEnqueueFakeSpiEvt send evt to common driver
 */
static inline void sensorFsmEnqueueFakeI2cEvt(I2cCallbackF i2cCallBack, void *next_state, int error)
{
    (*i2cCallBack)(next_state, 0, 0, error);
}
static inline void sensorFsmEnqueueFakeSpiEvt(SpiCbkF spiCallBack, void *next_state, int error)
{
    (*spiCallBack)(next_state, error);
}
#endif
