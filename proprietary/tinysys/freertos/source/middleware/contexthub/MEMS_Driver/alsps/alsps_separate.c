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
#include <alsps.h>
#include <alsps_separate.h>
#include <heap.h>

struct SeparateFsm {
    bool alsSeparateFsmRegistered;
    bool psSeparateFsmRegistered;
    bool integrateFsmRegistered;
    struct sensorFsm *mAlsSeparateFsm;
    struct sensorFsm *mPsSeparateFsm;
    struct sensorFsm *mIntegrateFsm;
    uint8_t mAlsSize;
    uint8_t mPsSize;
    uint8_t mIntegrateSize;
};
static struct SeparateFsm mSeparateFsm;

static void integrateSeparateFsm(void)
{
    uint8_t i = 0, als_reset = 0, als_reset_done = 0, ps_reset = 0, ps_reset_done = 0;
    struct sensorFsm *to = NULL, *from = NULL;
    uint32_t size = 0;

    size = (mSeparateFsm.mAlsSize + mSeparateFsm.mPsSize) * sizeof(struct sensorFsm);
    mSeparateFsm.mIntegrateFsm = (struct sensorFsm *)heapAlloc(size);
    if (!mSeparateFsm.mIntegrateFsm)
        return;

    to = mSeparateFsm.mIntegrateFsm;

    /* CHIP_ALS_RESET ==> CHIP_PS_RESET */
    for (i = 0; i < mSeparateFsm.mAlsSize; ++i) {
        if (mSeparateFsm.mAlsSeparateFsm[i].state == (void *)CHIP_ALS_RESET)
            als_reset = i;
        if (mSeparateFsm.mAlsSeparateFsm[i].next_state == (void *)CHIP_PS_RESET)
            als_reset_done = i;
    }
    size = (als_reset_done - als_reset + 1) * sizeof(struct sensorFsm);
    from = mSeparateFsm.mAlsSeparateFsm + als_reset;
    memcpy(to, from, size);
    to += (als_reset_done - als_reset + 1);

    /* CHIP_PS_RESET ==> CHIP_INIT_DONE */
    for (i = 0; i < mSeparateFsm.mPsSize; ++i) {
        if (mSeparateFsm.mPsSeparateFsm[i].state == (void *)CHIP_PS_RESET)
            ps_reset = i;
        if (mSeparateFsm.mPsSeparateFsm[i].next_state == (void *)CHIP_INIT_DONE)
            ps_reset_done = i;
    }
    size = (ps_reset_done - ps_reset + 1) * sizeof(struct sensorFsm);
    from = mSeparateFsm.mPsSeparateFsm + ps_reset;
    memcpy(to, from, size);
    to += (ps_reset_done - ps_reset + 1);

    /* 0 ==> CHIP_ALS_RESET */
    size = als_reset * sizeof(struct sensorFsm);
    from = mSeparateFsm.mAlsSeparateFsm;
    memcpy(to, from, size);
    to += als_reset;

    /* CHIP_PS_RESET ==> mAlsSize */
    size = (mSeparateFsm.mAlsSize - als_reset_done - 1) * sizeof(struct sensorFsm);
    from = mSeparateFsm.mAlsSeparateFsm + als_reset_done + 1;
    memcpy(to, from, size);
    to += (mSeparateFsm.mAlsSize - als_reset_done - 1);

    /* 0 ==> CHIP_PS_RESET */
    size = ps_reset * sizeof(struct sensorFsm);
    from = mSeparateFsm.mPsSeparateFsm;
    memcpy(to, from, size);
    to += ps_reset;

    /* CHIP_INIT_DONE ==> mPsSize */
    size = (mSeparateFsm.mPsSize - ps_reset_done - 1) * sizeof(struct sensorFsm);
    from = mSeparateFsm.mPsSeparateFsm + ps_reset_done + 1;
    memcpy(to, from, size);
    to += (mSeparateFsm.mPsSize - ps_reset_done - 1);

    mSeparateFsm.mIntegrateSize = mSeparateFsm.mAlsSize + mSeparateFsm.mPsSize;

    /* check size */
    if ((to - mSeparateFsm.mIntegrateFsm) !=
        (mSeparateFsm.mAlsSize + mSeparateFsm.mPsSize))
        configASSERT(0);
}

int registerPsSeparateFsm(struct sensorFsm *mSensorFsm, uint8_t size)
{
    if (mSeparateFsm.psSeparateFsmRegistered ||
            mSeparateFsm.integrateFsmRegistered || !mSensorFsm) {
        return -1;
    }

    mSeparateFsm.psSeparateFsmRegistered = true;
    mSeparateFsm.mPsSeparateFsm = mSensorFsm;
    mSeparateFsm.mPsSize = size;

    if (mSeparateFsm.alsSeparateFsmRegistered) {
        integrateSeparateFsm();
        registerAlsPsDriverFsm(mSeparateFsm.mIntegrateFsm, mSeparateFsm.mIntegrateSize);
        mSeparateFsm.integrateFsmRegistered = true;
    }
    return 0;
}

int registerAlsSeparateFsm(struct sensorFsm *mSensorFsm, uint8_t size)
{
    if (mSeparateFsm.alsSeparateFsmRegistered ||
            mSeparateFsm.integrateFsmRegistered || !mSensorFsm) {
        return -1;
    }

    mSeparateFsm.alsSeparateFsmRegistered = true;
    mSeparateFsm.mAlsSeparateFsm = mSensorFsm;
    mSeparateFsm.mAlsSize = size;

    if (mSeparateFsm.psSeparateFsmRegistered) {
        integrateSeparateFsm();
        registerAlsPsDriverFsm(mSeparateFsm.mIntegrateFsm, mSeparateFsm.mIntegrateSize);
        mSeparateFsm.integrateFsmRegistered = true;
    }
    return 0;
}

