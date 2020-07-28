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
#include <timer.h>
#include <heap.h>
#include <plat/inc/rtc.h>
//#include <plat/inc/syscfg.h>
#include <hostIntf.h>
#include <nanohubPacket.h>
#include <floatRt.h>
#include "mpe_cm4_API.h"

#include <seos.h>

#include <nanohub_math.h>
#include <sensors.h>
#include <limits.h>
#include <slab.h>
#include <wakelock.h>

#define ULONG_LONG_MAX 0XFFFFFFFFFFFFFFFFULL
#define MAX_NUM_COMMS_EVENT_SAMPLES 6
#define MAX_NUM_RAW_DATA_SAMPLES  50000000000ull
#define SENS_TYPE_GYRO_BIAS        (SENS_TYPE_FIRST_USER + 0)
#define SENS_TYPE_MAG_BIAS        (SENS_TYPE_FIRST_USER + 1)
#define SENS_TYPE_ACC_BIAS        (SENS_TYPE_FIRST_USER + 2)
#define EVT_SENSOR_ACC_DATA_RDY         sensorGetMyEventType(SENS_TYPE_ACCEL)
#define EVT_SENSOR_GYR_DATA_RDY         sensorGetMyEventType(SENS_TYPE_GYRO)
#define EVT_SENSOR_MAG_DATA_RDY         sensorGetMyEventType(SENS_TYPE_MAG)
#define EVT_SENSOR_GYR_BIAS_RDY         sensorGetMyEventType(SENS_TYPE_GYRO_BIAS)
#define EVT_SENSOR_MAG_BIAS_RDY         sensorGetMyEventType(SENS_TYPE_MAG_BIAS)

#define DEFAULT_ACC_RATE_HZ     SENSOR_HZ(200.0f)
#define DEFAULT_GYRO_RATE_HZ    SENSOR_HZ(200.0f)
#define DEFAULT_MAG_RATE_HZ     SENSOR_HZ(50.0f)
#define DEFAULT_BARO_RATE_HZ    SENSOR_HZ(1.0f)
#define DEFAULT_BARO_LATENCY    1000000000ull   // 480 ms

#define ONE_EVENT_DELAY_PER_FIFO_LOOP 4424778

mtk_uint32 Algo_reg_union = 0;
mtk_uint32 Algo_reg_group = 0;

bool isfirst_acc = 1;
bool isfirst_mag = 1;

MPE_SENSOR_COMMON acceleration;
MPE_SENSOR_COMMON gyro;
MPE_SENSOR_COMMON mag;
mtk_uint64 last_gyro_timestamp = 0;
mtk_uint8 type_agm_cnt = 0;
mtk_uint8 type_am_cnt = 0;
mtk_uint8 type_ag_cnt = 0;
bool mag_acc = false;
bool gyro_acc = false;
mtk_uint32 fusion_sensor_max_rate = 0;
mtk_uint32 dr_exit_cnt = 0;
mtk_uint32 acc_unflush_cnt = 0;

#define PDR_LOG
#ifdef PDR_LOG
#define PLOG(fmt, args...)    osLog(LOG_DEBUG, "[FUSION]: "fmt, ##args)
#else
#define PLOG(fmt, args...)
#endif

enum
{
    FUSION_FLAG_ENABLED             = 0x01,
    FUSION_FLAG_INITIALIZED         = 0x08,
    FUSION_FLAG_GAME_ENABLED        = 0x10,
    FUSION_FLAG_GAME_INITIALIZED    = 0x20
};

enum RawSensorType
{
    ACC,
    GYR,
    MAG,
    BARO,
    NUM_OF_RAW_SENSOR
};

enum FusionSensorType
{
    ORIENT,
    GRAVITY,
    GEOMAG,
    LINEAR,
    GAME,
    ROTAT,
    NUM_OF_FUSION_SENSOR
};

struct FusionSensorSample {
    uint64_t time;
    float x, y, z;
};

struct FusionSensor {
    uint32_t handle;
    struct TripleAxisDataEvent *ev;
    uint64_t prev_time;
    uint64_t latency;
    uint32_t rate;
    bool active;
    bool use_gyr_data;
    bool use_mag_data;
    uint8_t idx;
    wakelock_t wakeLock;
};

struct FusionTask {
    uint32_t tid;
    uint32_t accelHandle;
    uint32_t gyroHandle;
    uint32_t magHandle;

    struct FusionSensor sensors[NUM_OF_FUSION_SENSOR];
    struct FusionSensorSample samples_gyr[MAX_NUM_COMMS_EVENT_SAMPLES];
    size_t sample_counts[NUM_OF_RAW_SENSOR];
    uint64_t ResamplePeriodNs[NUM_OF_RAW_SENSOR];
    uint64_t last_time[NUM_OF_RAW_SENSOR];
    struct TripleAxisDataPoint last_sample[NUM_OF_RAW_SENSOR];

    uint32_t flags;

    uint32_t raw_sensor_rate[NUM_OF_RAW_SENSOR];
    uint64_t raw_sensor_latency;

    uint8_t acc_cnt;
    uint8_t gyr_cnt;
    uint8_t mag_cnt;
};

static uint32_t FusionRates[] = {
    SENSOR_HZ(50.0f),
    SENSOR_HZ(100.0f),
    SENSOR_HZ(200.0f),
    0,
};

static const uint64_t rateTimerVals[] = //should match "supported rates in length" and be the timer length for that rate in nanosecs
{
    1000000000ULL / 50,
    1000000000ULL / 100,
    1000000000ULL / 200,
};

static struct FusionTask mTask;

#define DEC_INFO_RATE(name, rates, type, axis, inter, samples) \
    .sensorName = name, \
    .supportedRates = rates, \
    .sensorType = type, \
    .numAxis = axis, \
    .interrupt = inter, \
    .minSamples = samples

static const struct SensorInfo mSi[NUM_OF_FUSION_SENSOR] =
{
    { DEC_INFO_RATE("Orientation", FusionRates, SENS_TYPE_ORIENTATION, NUM_AXIS_THREE, NANOHUB_INT_NONWAKEUP, 20) },
    { DEC_INFO_RATE("Gravity", FusionRates, SENS_TYPE_GRAVITY, NUM_AXIS_THREE, NANOHUB_INT_NONWAKEUP, 20) },
    { DEC_INFO_RATE("Geomagnetic Rotation Vector", FusionRates, SENS_TYPE_GEO_MAG_ROT_VEC, NUM_AXIS_THREE, NANOHUB_INT_NONWAKEUP, 20) },
    { DEC_INFO_RATE("Linear Acceleration", FusionRates, SENS_TYPE_LINEAR_ACCEL, NUM_AXIS_THREE, NANOHUB_INT_NONWAKEUP, 20) },
    { DEC_INFO_RATE("Game Rotation Vector", FusionRates, SENS_TYPE_GAME_ROT_VECTOR, NUM_AXIS_THREE, NANOHUB_INT_NONWAKEUP, 300) },
    { DEC_INFO_RATE("Rotation Vector", FusionRates, SENS_TYPE_ROTATION_VECTOR, NUM_AXIS_THREE, NANOHUB_INT_NONWAKEUP, 20) },
};

static struct SlabAllocator *mDataSlab;

static uint32_t sensorRateLookup(uint32_t inputRate)
{
    if (inputRate == 0)
        return 0;
    else if (inputRate <= SENSOR_HZ(50.0f))
        return SENSOR_HZ(50.0f);
    else if (inputRate <= SENSOR_HZ(100.0f))
        return SENSOR_HZ(100.0f);
    else
        return SENSOR_HZ(200.0f);
}

static int fusionOperation(int power, int fusion_type, int fusion_group)
{
    int err = 0;
    mtk_uint8 *group_cnt;
    PLOG("fusionOperation power = %d, type = %d\n\r", power, fusion_type);
    if (fusion_group == REG_AGM_ALGO) {
        group_cnt = &type_agm_cnt;
    } else if (fusion_group == REG_AG_ALGO) {
        group_cnt = &type_ag_cnt;
    } else if (fusion_group == REG_AM_ALGO) {
        group_cnt = &type_am_cnt;
    } else {
        return -1;
    }

    if (power == 0) {
        Algo_reg_union &= (~fusion_type);
        if((*group_cnt) > 0) {
            if((*group_cnt) == 1) {
                Algo_reg_group &= (~fusion_group);
            }
            (*group_cnt)--;
        }
    } else if (power == 1) {
        Algo_reg_union |= fusion_type;
        Algo_reg_group |= fusion_group;
        (*group_cnt)++;
        mpe_re_initialize();
    } else {
        PLOG("Enable fusion_sensor_operation error!\n\r");
    }
    return err;
}

static void dataEvtFree(void *ptr)
{
    slabAllocatorFree(mDataSlab, ptr);
}

static void addSample(struct FusionSensor *mSensor, uint64_t time, float x, float y, float z)
{
    struct TripleAxisDataPoint *sample;
    uint32_t deltaTime = 0;

    if (mSensor->ev == NULL) {
        mSensor->ev = slabAllocatorAlloc(mDataSlab);
        if (mSensor->ev == NULL) {
            // slaballocation failed
            osLog(LOG_ERROR, "FUSION: Slab Allocation Failed\n");
            return;
        }
        mSensor->ev->samples[0].firstSample.numSamples = 0;
        mSensor->ev->samples[0].firstSample.biasCurrent = 0;
        mSensor->ev->samples[0].firstSample.biasPresent = 0;
        mSensor->ev->referenceTime = time;
        mSensor->prev_time = time;
    }

    if (mSensor->ev->samples[0].firstSample.numSamples >= MAX_NUM_COMMS_EVENT_SAMPLES) {
        osLog(LOG_ERROR, "BAD_INDEX\n");
        return;
    }

    sample = &mSensor->ev->samples[mSensor->ev->samples[0].firstSample.numSamples++];

    if (mSensor->ev->samples[0].firstSample.numSamples > 1) {
        deltaTime = time - mSensor->prev_time;
        deltaTime = deltaTime < 0 ? 0 : deltaTime;
        sample->deltaTime = deltaTime;
        mSensor->prev_time = time;
    }
    sample->x = x;
    sample->y = y;
    sample->z = z;
    //mSensor->ev->samples[0].firstSample.biasCurrent = mag_acc;
    //mSensor->ev->samples[0].firstSample.biasPresent = 1;
    if (mSensor->ev->samples[0].firstSample.numSamples == MAX_NUM_COMMS_EVENT_SAMPLES) {
        osEnqueueEvt(EVENT_TYPE_BIT_DISCARDABLE | sensorGetMyEventType(mSi[mSensor->idx].sensorType),
                mSensor->ev, dataEvtFree);
        mSensor->ev = NULL;
    }
}

static void updateOutput(uint64_t last_sensor_time)
{
    MPE_SENSOR_RESULT result;
    if (mTask.sensors[ORIENT].active) {
        //Call mpe main function to extract mpe data
        memset(&result, 0, sizeof(MPE_SENSOR_RESULT));
        mpe_get_fusion_output(&result, ORIENTATION_ALGO);
        addSample(&mTask.sensors[ORIENT],
                last_sensor_time,
                result.x,
                result.y,
                result.z);
    }
    if (mTask.sensors[GRAVITY].active) {
        //Call mpe main function to extract mpe data
        memset(&result, 0, sizeof(MPE_SENSOR_RESULT));
        mpe_get_fusion_output(&result, GRAVITY_ALGO);
        addSample(&mTask.sensors[GRAVITY],
                last_sensor_time,
                result.x,
                result.y,
                result.z);
    }
    if (mTask.sensors[GEOMAG].active) {
        //Call mpe main function to extract mpe data
        memset(&result, 0, sizeof(MPE_SENSOR_RESULT));
        mpe_get_fusion_output(&result, GEOMAG_ROT_VEC_ALGO);
        addSample(&mTask.sensors[GEOMAG],
                last_sensor_time,
                result.x,
                result.y,
                result.z);
    }
    if (mTask.sensors[LINEAR].active) {
        //Call mpe main function to extract mpe data
        memset(&result, 0, sizeof(MPE_SENSOR_RESULT));
        mpe_get_fusion_output(&result, LINEAR_ACC_ALGO);
        addSample(&mTask.sensors[LINEAR],
                last_sensor_time,
                result.x,
                result.y,
                result.z);
    }
    if (mTask.sensors[GAME].active) {
        //Call mpe main function to extract mpe data
        memset(&result, 0, sizeof(MPE_SENSOR_RESULT));
        mpe_get_fusion_output(&result, GAME_ROT_VEC);
        addSample(&mTask.sensors[GAME],
                last_sensor_time,
                result.x,
                result.y,
                result.z);
    }
    if (mTask.sensors[ROTAT].active) {
        //Call mpe main function to extract mpe data
        memset(&result, 0, sizeof(MPE_SENSOR_RESULT));
        mpe_get_fusion_output(&result, ROT_VEC_ALGO);
        addSample(&mTask.sensors[ROTAT],
                last_sensor_time,
                result.x,
                result.y,
                result.z);
    }
}

static void fillSamples(struct TripleAxisDataEvent *ev, enum RawSensorType index)
{
    size_t i = 0;
    size_t num_samples = 0;
    struct TripleAxisDataPoint *curr_sample, *next_sample;
    uint64_t curr_time, next_time;
    MPE_SENSOR_COMMON local_gyro;

    if (index == ACC && mTask.acc_cnt == 0) {
        PLOG("acc no user, bypass raw data\n\r");
        return;
    }
    if (index == GYR && mTask.gyr_cnt == 0) {
        PLOG("gyr no user, bypass raw data\n\r");
        return;
    }
    if (index == MAG && mTask.mag_cnt == 0) {
        PLOG("mag no user, bypass raw data\n\r");
        return;
    }
    if (index == ACC && (acc_unflush_cnt % 20) == 1) {
        PLOG("No gyr data, acc unflush count= %u\n\r", acc_unflush_cnt);
    }

    // check if this sensor was used before
    if (mTask.last_time[index] == ULONG_LONG_MAX) {
        curr_sample = ev->samples;
        next_sample = curr_sample + 1;
        num_samples = ev->samples[0].firstSample.numSamples;
        curr_time = ev->referenceTime;
    } else {
        curr_sample = &mTask.last_sample[index];
        next_sample = ev->samples;
        num_samples = ev->samples[0].firstSample.numSamples + 1;
        curr_time = mTask.last_time[index];
    }

    while (num_samples > 1) {
        if (next_sample == ev->samples)
            next_time = ev->referenceTime;
        else
            next_time = curr_time + next_sample->deltaTime;

        num_samples--;
        curr_sample = next_sample;
        next_sample++;
        curr_time = next_time;
        if (index == GYR) {
            if (i < MAX_NUM_COMMS_EVENT_SAMPLES) {
                mTask.samples_gyr[i].x = curr_sample->x;
                mTask.samples_gyr[i].y = curr_sample->y;
                mTask.samples_gyr[i].z = curr_sample->z;
                mTask.samples_gyr[i].time = curr_time;
                i++;
            } else {
                PLOG("gyr data exceeded batch size %d\n\r", i);
            }
        } else if (index == ACC) {
            isfirst_acc = 0;
            acceleration.x = curr_sample->x;
            acceleration.y = curr_sample->y;
            acceleration.z = curr_sample->z;
            acceleration.timestamp = curr_time;
            acceleration.accuracy = 3;
            acc_unflush_cnt++;

            if (Algo_reg_group == (REG_AM_ALGO)) {
                local_gyro.x = 3000;
                local_gyro.y = 6000;
                local_gyro.z = 9000;
                local_gyro.accuracy = 3;
                local_gyro.timestamp = acceleration.timestamp;
                mpe_set_sensor_input(&local_gyro, 1, (U1)SENSOR_GYRO);
                mpe_set_sensor_input(&acceleration, 1, (U1)SENSOR_ACC);
                mpe_set_sensor_input(&mag, 1, (U1)SENSOR_MAG);
                mpe_run_algo(Algo_reg_union, REG_AM_ALGO);
                updateOutput(acceleration.timestamp);
                acc_unflush_cnt = 0;
            }
        } else if (index == MAG) {
            isfirst_mag = 0;
            mag.x = curr_sample->x;
            mag.y = curr_sample->y;
            mag.z = curr_sample->z;
            mag.timestamp = curr_time;
            if (!mag_acc) {
                mag.accuracy = 0;
            } else {
                mag.accuracy = 3;
            }
        }
    }
    if (index == GYR) {
        mTask.sample_counts[index] = i;
    }
    /*if the algo is A+M only, after we finish calcu with all acc data, we need flush the fusion data*/
    if (Algo_reg_group == (REG_AM_ALGO)) {
        for (i = ORIENT; i < NUM_OF_FUSION_SENSOR; i++) {
            if (mTask.sensors[i].ev != NULL) {
                osEnqueueEvt(EVENT_TYPE_BIT_DISCARDABLE | sensorGetMyEventType(mSi[i].sensorType),
                        mTask.sensors[i].ev, dataEvtFree);
                mTask.sensors[i].ev = NULL;
            }
        }
    }

    mTask.last_sample[index] = *curr_sample;
    mTask.last_time[index] = curr_time;
}

static void drainSamples()
{
    size_t i = 0;
    MPE_SENSOR_COMMON local_gyro;
    if (isfirst_acc) {
        PLOG("acc raw data not ready\n\r");
        return;
    }
    if (isfirst_mag && Algo_reg_group != REG_AG_ALGO) {
        PLOG("mag raw data not ready\n\r");
        return;
    }
    if (mTask.sample_counts[GYR] == MAX_NUM_COMMS_EVENT_SAMPLES) {
        PLOG("gyr raw data cnt abnormal, gyr: %d\n\r", mTask.sample_counts[GYR]);
        return;
    }

    acc_unflush_cnt = 0;

    for (i = 0; i < mTask.sample_counts[GYR]; i++) {
        gyro.x = mTask.samples_gyr[i].x;
        gyro.y = mTask.samples_gyr[i].y;
        gyro.z = mTask.samples_gyr[i].z;
        gyro.accuracy = 3;
        gyro.timestamp = mTask.samples_gyr[i].time;

        if (last_gyro_timestamp != 0) {
            GyroSensor_set_timebase_correction_parameter((mtk_uint32)(gyro.timestamp - last_gyro_timestamp), 0);
        }
        last_gyro_timestamp = gyro.timestamp;

        if ((Algo_reg_group == (REG_AGM_ALGO))  //type 1: AGM
            || (Algo_reg_group == (REG_AGM_ALGO + REG_AG_ALGO)) //type 3: AGM +AG
            || (Algo_reg_group == (REG_AGM_ALGO + REG_AM_ALGO)) //type 5: AGM +AM
            || (Algo_reg_group == (REG_AG_ALGO + REG_AM_ALGO)) //type 6:AG + AM
            || (Algo_reg_group == (REG_AGM_ALGO + REG_AG_ALGO + REG_AM_ALGO))) {//type 7: AGM +AG + AM
                mpe_set_sensor_input(&acceleration, 1, (U1)SENSOR_ACC);
                mpe_set_sensor_input(&gyro, 1, (U1)SENSOR_GYRO);
                mpe_set_sensor_input(&mag, 1, (U1)SENSOR_MAG);
                mpe_run_algo(Algo_reg_union, Algo_reg_group);
        } else if (Algo_reg_group == REG_AG_ALGO) { //type 2:AG
            mpe_set_sensor_input(&acceleration, 1, (U1)SENSOR_ACC);
            mpe_set_sensor_input(&gyro, 1, (U1)SENSOR_GYRO);
            mpe_set_sensor_input(&mag, 1, (U1)SENSOR_MAG);
            mpe_run_algo(Algo_reg_union, REG_AG_ALGO);
        } else if (Algo_reg_group == (REG_AM_ALGO)) { //type 4:AM
            mpe_set_sensor_input(&acceleration, 1, (U1)SENSOR_ACC);
            mpe_set_sensor_input(&mag, 1, (U1)SENSOR_MAG);
            //local gyro
            local_gyro.x = 3000;
            local_gyro.y = 6000;
            local_gyro.z = 9000;
            local_gyro.accuracy = 3;
            local_gyro.timestamp = acceleration.timestamp;
            mpe_set_sensor_input(&local_gyro, 1, (U1)SENSOR_GYRO);
            mpe_run_algo(Algo_reg_union, REG_AM_ALGO);
        }
        updateOutput(gyro.timestamp);
    }

    mTask.sample_counts[GYR] = 0;
    memset(mTask.samples_gyr, 0 ,MAX_NUM_COMMS_EVENT_SAMPLES*sizeof(struct FusionSensorSample));

    for (i = ORIENT; i < NUM_OF_FUSION_SENSOR; i++) {
        if (mTask.sensors[i].ev != NULL) {
            osEnqueueEvt(EVENT_TYPE_BIT_DISCARDABLE | sensorGetMyEventType(mSi[i].sensorType),
                    mTask.sensors[i].ev, dataEvtFree);
            mTask.sensors[i].ev = NULL;
        }
    }
}

static void fusionSetRateAcc(void)
{
    int i = 0;
    if  (mTask.accelHandle == 0) {
        mTask.last_time[ACC] = ULONG_LONG_MAX;
        if (sensorFind(SENS_TYPE_ACCEL, i, &mTask.accelHandle) != NULL) {
            if (sensorRequest(mTask.tid, mTask.accelHandle, fusion_sensor_max_rate, 0)) {
                osEventSubscribe(mTask.tid, EVT_SENSOR_ACC_DATA_RDY);
            }
        }
    } else {
        sensorRequestRateChange(mTask.tid, mTask.accelHandle, fusion_sensor_max_rate, 0);
    }
}

static void fusionSetRateGyr(void)
{
    int i = 0;
    if (mTask.gyroHandle == 0) {
        mTask.sample_counts[GYR] = 0;
        mTask.last_time[GYR] = ULONG_LONG_MAX;
        if (sensorFind(SENS_TYPE_GYRO, i, &mTask.gyroHandle) != NULL) {
            if (sensorRequest(mTask.tid, mTask.gyroHandle, fusion_sensor_max_rate, 0)) {
                osEventSubscribe(mTask.tid, EVT_SENSOR_GYR_DATA_RDY);
            }
        }
    } else {
        sensorRequestRateChange(mTask.tid, mTask.gyroHandle, fusion_sensor_max_rate, 0);
    }
}

static void fusionSetRateMag(void)
{
    int i = 0;
    if (mTask.magHandle == 0) {
        mTask.last_time[MAG] = ULONG_LONG_MAX;
        if (sensorFind(SENS_TYPE_MAG, i, &mTask.magHandle) != NULL) {
            if (sensorRequest(mTask.tid, mTask.magHandle, DEFAULT_MAG_RATE_HZ, 0)) {
                osEventSubscribe(mTask.tid, EVT_SENSOR_MAG_DATA_RDY);
            }
        }
    } else {
        sensorRequestRateChange(mTask.tid, mTask.magHandle, DEFAULT_MAG_RATE_HZ, 0);
    }
}

static bool fusionSetRate(uint32_t rate, uint64_t latency, void *cookie)
{
    struct FusionSensor *mSensor = &mTask.sensors[(int)cookie];
    int i;
    uint32_t max_rate = 0;
    uint32_t gyr_rate, mag_rate;
    uint32_t total_sample = 0;

    PLOG("fusionSetRate [%s] %u %llu\n\r", mSi[(int)cookie].sensorName, rate, latency);

    mSensor->rate = rate;
    mSensor->latency = latency;

    for (i = ORIENT; i < NUM_OF_FUSION_SENSOR; i++) {
        if (mTask.sensors[i].active) {
            max_rate = max_rate > mTask.sensors[i].rate ? max_rate : mTask.sensors[i].rate;
            PLOG("set %s rate = %u\n\r", mSi[i].sensorName, mTask.sensors[i].rate);
        }
    }

    fusion_sensor_max_rate = sensorRateLookup(max_rate);
    PLOG("set fusion max_rate = %u\n\r", fusion_sensor_max_rate);

    if (mTask.acc_cnt > 0) {
        mTask.ResamplePeriodNs[ACC] = sensorTimerLookupCommon(FusionRates, rateTimerVals, max_rate);
        mTask.raw_sensor_rate[ACC] = max_rate;
        total_sample += mTask.raw_sensor_rate[ACC];
    }

    if (mTask.gyr_cnt > 0) {
        mTask.ResamplePeriodNs[GYR] = sensorTimerLookupCommon(FusionRates, rateTimerVals, max_rate);
        gyr_rate = max_rate > DEFAULT_GYRO_RATE_HZ ? max_rate : DEFAULT_GYRO_RATE_HZ;
        mTask.raw_sensor_rate[GYR] = gyr_rate;
        total_sample += mTask.raw_sensor_rate[GYR];
    }

    if (mTask.mag_cnt > 0) {
        mag_rate = max_rate < DEFAULT_MAG_RATE_HZ ? max_rate : DEFAULT_MAG_RATE_HZ;
        mTask.ResamplePeriodNs[MAG] = sensorTimerLookupCommon(FusionRates, rateTimerVals, mag_rate);
        mTask.raw_sensor_rate[MAG] = mag_rate;
        total_sample += mTask.raw_sensor_rate[MAG];
    }

    mTask.raw_sensor_latency = total_sample ? ((uint32_t)(MAX_NUM_RAW_DATA_SAMPLES / 12.5f) / (total_sample / SENSOR_HZ(12.5))) : 0;
    for (i = ORIENT; i < NUM_OF_FUSION_SENSOR; i++) {
        if (mTask.sensors[i].active) {
            mTask.raw_sensor_latency = mTask.sensors[i].latency < mTask.raw_sensor_latency ?
                mTask.sensors[i].latency : mTask.raw_sensor_latency;
        }
    }

    if (mTask.acc_cnt > 0)
        fusionSetRateAcc();
    if (mTask.gyr_cnt > 0)
        fusionSetRateGyr();
    if (mTask.mag_cnt > 0)
        fusionSetRateMag();
    if (mSensor->rate > 0) {
        sensorSignalInternalEvt(mSensor->handle, SENSOR_INTERNAL_EVT_RATE_CHG, rate, latency);
    }
    return true;
}

static bool fusionPower(bool on, void *cookie)
{
    struct FusionSensor *mSensor = &mTask.sensors[(int)cookie];
    int idx;
    int power;
    PLOG("fusionPower [%s] %d\n\r", mSi[(int)cookie].sensorName, on);

    mSensor->active = on;
    if (on == false) {
        power = 0;
        wake_unlock(&mSensor->wakeLock);
    } else {
        power = 1;
        wake_lock(&mSensor->wakeLock);
    }
    switch(mSensor->idx) {
        case ORIENT:
            fusionOperation(power, REG_ORIENTATION_ALGO, REG_AGM_ALGO);
            break;
        case GRAVITY:
            fusionOperation(power, REG_GRAVITY_ALGO, REG_AGM_ALGO);
            break;
        case GEOMAG:
            fusionOperation(power, REG_GEOMAG_ROT_VEC_ALGO, REG_AM_ALGO);
            break;
        case LINEAR:
            fusionOperation(power, REG_LINEAR_ACC_ALGO, REG_AGM_ALGO);
            break;
        case GAME:
            fusionOperation(power, REG_GAME_ROT_VEC, REG_AG_ALGO);
            break;
        case ROTAT:
            fusionOperation(power, REG_ROT_VEC_ALGO, REG_AGM_ALGO);
            break;
    }

    if (on == false) {
        mTask.acc_cnt--;
        if (mSensor->idx != GEOMAG)
            mTask.gyr_cnt--;
        if (mSensor->idx != GAME)
            mTask.mag_cnt--;

        // if cnt == 0 and Handle == 0, nothing need to be done.
        // if cnt > 0 and Handle == 0, something else is turning it on, all will be done.
        if (mTask.acc_cnt == 0 && mTask.accelHandle != 0) {
            sensorRelease(mTask.tid, mTask.accelHandle);
            mTask.accelHandle = 0;
            isfirst_acc = 1;
            acc_unflush_cnt = 0;
            osEventUnsubscribe(mTask.tid, EVT_SENSOR_ACC_DATA_RDY);
            memset(&acceleration, 0, sizeof(MPE_SENSOR_COMMON));
        }

        if (mTask.gyr_cnt == 0 && mTask.gyroHandle != 0) {
            sensorRelease(mTask.tid, mTask.gyroHandle);
            mTask.gyroHandle = 0;
            osEventUnsubscribe(mTask.tid, EVT_SENSOR_GYR_DATA_RDY);
            last_gyro_timestamp = 0;
            memset(&gyro, 0, sizeof(MPE_SENSOR_COMMON));
        }

        if (mTask.mag_cnt == 0 && mTask.magHandle != 0) {
            sensorRelease(mTask.tid, mTask.magHandle);
            mTask.magHandle = 0;
            isfirst_mag = 1;
            osEventUnsubscribe(mTask.tid, EVT_SENSOR_MAG_DATA_RDY);
            memset(&mag, 0, sizeof(MPE_SENSOR_COMMON));
        }
        idx = mSensor->idx;
        (void) fusionSetRate(0, ULONG_LONG_MAX, (void *)idx);
    } else {
        mTask.acc_cnt++;
        if (mSensor->idx != GEOMAG) {
            mTask.gyr_cnt++;
        }
        if (mSensor->idx != GAME) {
            mTask.mag_cnt++;
        }
    }
    PLOG("fusionPower acc=%d gyr=%d mag=%d\n\r",mTask.acc_cnt, mTask.gyr_cnt, mTask.mag_cnt);
    sensorSignalInternalEvt(mSensor->handle, SENSOR_INTERNAL_EVT_POWER_STATE_CHG, on, 0);

    return true;
}

static bool fusionFirmwareUpload(void *cookie)
{
    struct FusionSensor *mSensor = &mTask.sensors[(int)cookie];

    sensorSignalInternalEvt(mSensor->handle, SENSOR_INTERNAL_EVT_FW_STATE_CHG, 1, 0);
    return true;
}

static bool fusionFlush(void *cookie)
{
    struct FusionSensor *mSensor = &mTask.sensors[(int)cookie];
    uint32_t evtType = sensorGetMyEventType(mSi[mSensor->idx].sensorType);

    osEnqueueEvt(evtType, SENSOR_DATA_EVENT_FLUSH, NULL);
    return true;
}

static void fusionHandleEvent(uint32_t evtType, const void* evtData)
{
    int i = 0;
    uint32_t handle = 0;
    struct TripleAxisDataEvent *ev;

    if (evtData == SENSOR_DATA_EVENT_FLUSH)
        return;
    switch (evtType) {
    case EVT_APP_START:
        osEventUnsubscribe(mTask.tid, EVT_APP_START);
        if (sensorFind(SENS_TYPE_GYRO, i, &handle) != NULL) {
            osEventSubscribe(mTask.tid, EVT_SENSOR_GYR_BIAS_RDY);
        }
        if (sensorFind(SENS_TYPE_MAG, i, &handle) != NULL) {
            osEventSubscribe(mTask.tid, EVT_SENSOR_MAG_BIAS_RDY);
        }
        break;
    case EVT_SENSOR_ACC_DATA_RDY:
        ev = (struct TripleAxisDataEvent *)evtData;
        fillSamples(ev, ACC);
        break;
    case EVT_SENSOR_GYR_DATA_RDY:
        ev = (struct TripleAxisDataEvent *)evtData;
        fillSamples(ev, GYR);
        drainSamples();
        break;
    case EVT_SENSOR_MAG_DATA_RDY:
        ev = (struct TripleAxisDataEvent *)evtData;
        fillSamples(ev, MAG);
        break;
    case EVT_SENSOR_MAG_BIAS_RDY:
        ev = (struct TripleAxisDataEvent *)evtData;
        for (i = 0; i < ev->samples[0].firstSample.numSamples; i++) {
            if (ev->samples[0].firstSample.biasPresent && ev->samples[0].firstSample.biasSample == i)
                mag_acc = ev->samples[0].firstSample.biasCurrent;
            //osLog(LOG_INFO, "mag status: %d, offset: [%f, %f, %f]\n", mag_acc ? 3 : 0,
                //(double)ev->samples[i].x, (double)ev->samples[i].y, (double)ev->samples[i].z);
        }
        break;
    case EVT_SENSOR_GYR_BIAS_RDY:
        ev = (struct TripleAxisDataEvent *)evtData;
        for (i = 0; i < ev->samples[0].firstSample.numSamples; i++) {
            if (ev->samples[0].firstSample.biasPresent && ev->samples[0].firstSample.biasSample == i)
                gyro_acc = ev->samples[0].firstSample.biasCurrent;
            //osLog(LOG_INFO, "gyro status: %d, offset: [%f, %f, %f]\n", gyro_acc ? 3 : 0,
                //(double)ev->samples[i].x, (double)ev->samples[i].y, (double)ev->samples[i].z);
        }
        break;
    }
}

static const struct SensorOps mSops =
{
    .sensorPower = fusionPower,
    .sensorFirmwareUpload = fusionFirmwareUpload,
    .sensorSetRate = fusionSetRate,
    .sensorFlush = fusionFlush,
};

static bool fusionStart(uint32_t tid)
{
    osLog(LOG_INFO, "        FUSION:  %ld\n", tid);
    size_t i, slabSize;

    mTask.tid = tid;
    mTask.flags = 0;

    for (i = 0; i < NUM_OF_RAW_SENSOR; i++) {
         mTask.sample_counts[i] = 0;
    }

    for (i = ORIENT; i < NUM_OF_FUSION_SENSOR; i++) {
        mTask.sensors[i].handle = sensorRegister(&mSi[i], &mSops, (void *)i, true);
        mTask.sensors[i].idx = i;
        mTask.sensors[i].use_gyr_data = true;
        mTask.sensors[i].use_mag_data = true;
        wake_lock_init(&mTask.sensors[i].wakeLock, "fusion");
    }

    mTask.sensors[GEOMAG].use_gyr_data = false;
    mTask.sensors[GAME].use_mag_data = false;

    memset(&acceleration, 0, sizeof(MPE_SENSOR_COMMON));
    memset(&mag, 0, sizeof(MPE_SENSOR_COMMON));
    memset(&gyro, 0, sizeof(MPE_SENSOR_COMMON));

    slabSize = sizeof(struct TripleAxisDataEvent)
        + MAX_NUM_COMMS_EVENT_SAMPLES * sizeof(struct TripleAxisDataPoint);
    mDataSlab = slabAllocatorNew(slabSize, 4, 10); // 10 slots for now..
    if (!mDataSlab) {
        osLog(LOG_ERROR, "FUSION SLAB ALLOCATION FAILED\n");
        return false;
    }
    GyroSensor_set_timebase_correction_parameter((mtk_uint32)ONE_EVENT_DELAY_PER_FIFO_LOOP, 0);
    osEventSubscribe(mTask.tid, EVT_APP_START);
    return true;
}

static void fusionEnd()
{
    slabAllocatorDestroy(mDataSlab);
}

INTERNAL_APP_INIT(
        APP_ID_MAKE(APP_ID_VENDOR_MTK, MTK_APP_ID_WRAP(SENS_TYPE_ROTATION_VECTOR, 0, 0)),
        0,
        fusionStart,
        fusionEnd,
        fusionHandleEvent);

