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

#include <sensorFsm.h>

const struct sensorFsm *sensorFsmFindCmd(const struct sensorFsm *mFsm,
    uint32_t sensorFsmSize, const void *state)
{
    int i;
    const struct sensorFsm *cmd;

    for (i = 0; i < sensorFsmSize; i++) {
        cmd = &mFsm[i];
        if (cmd->state == state)
            return cmd;
    }
    return NULL;
}
int sensorFsmRunState(struct transferDataInfo *dataInfo, struct fsmCurrInfo *fsmInfo,
    const void *state, I2cCallbackF i2cCallBack, SpiCbkF spiCallBack)
{
    int err = 0;
    const struct sensorFsm *cmd;

    cmd = sensorFsmFindCmd(fsmInfo->mSensorFsm, fsmInfo->mSensorFsmSize, state);
    //osLog(LOG_ERROR, "[fsm]%s debug: curr state:%d, next state:%d\n",
        //fsmInfo->moduleName, fsmInfo->mNextFsm->state, fsmInfo->mNextFsm->next_state);
    if (cmd != NULL) {
        if (dataInfo != NULL)
            cmd->operation(i2cCallBack, spiCallBack, cmd->next_state, dataInfo->inBuf,
                           dataInfo->inSize, dataInfo->elemInSize, dataInfo->outBuf, dataInfo->outSize, dataInfo->elemOutSize);
        else
            cmd->operation(i2cCallBack, spiCallBack, cmd->next_state, NULL,
                0, 0, NULL, NULL, NULL);
        fsmInfo->mCurrFsm = (struct sensorFsm *)cmd;
        fsmInfo->mNextFsm = fsmInfo->mCurrFsm + 1;
        fsmInfo->mCurrFsmSize = fsmInfo->mSensorFsmSize -
            ((uint8_t *)fsmInfo->mCurrFsm - (uint8_t *)fsmInfo->mSensorFsm) / sizeof(struct sensorFsm);
        err = 0;
    } else {
        osLog(LOG_ERROR, "[fsm]%s error: don't support state:%p\n", fsmInfo->moduleName, state);
        err = -1;
    }
    return err;
}
void sensorFsmSwitchState(struct transferDataInfo *dataInfo, struct fsmCurrInfo *fsmInfo,
    I2cCallbackF i2cCallBack, SpiCbkF spiCallBack)
{
    if (fsmInfo->mNextFsm != NULL) {
        //osLog(LOG_ERROR, "[fsm]%s debug: curr state:%d, next state:%d\n",
            //fsmInfo->moduleName, fsmInfo->mNextFsm->state, fsmInfo->mNextFsm->next_state);
        fsmInfo->mNextFsm->operation(i2cCallBack, spiCallBack, fsmInfo->mNextFsm->next_state,
            NULL, 0, 0, NULL, NULL, NULL);
        fsmInfo->mCurrFsm = fsmInfo->mNextFsm;
        fsmInfo->mNextFsm = fsmInfo->mCurrFsm + 1;
        fsmInfo->mCurrFsmSize = fsmInfo->mSensorFsmSize -
            ((uint8_t *)fsmInfo->mCurrFsm - (uint8_t *)fsmInfo->mSensorFsm) / sizeof(struct sensorFsm);
        if (fsmInfo->mCurrFsmSize == 0) {
            osLog(LOG_ERROR, "[fsm]%s error: mCurrFsmSize = 0\n", fsmInfo->moduleName);
            fsmInfo->mNextFsm = NULL;
        }
    } else
        osLog(LOG_ERROR, "[fsm]%s error: mNextFsm = NULL\n", fsmInfo->moduleName);
}
void sensorFsmReleaseState(struct fsmCurrInfo *fsmInfo)
{
    fsmInfo->mNextFsm = NULL;
}
