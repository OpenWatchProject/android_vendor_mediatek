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

#include <seos.h>

#include <nanohub_math.h>
#include <algos/fusion.h>
#include <sensors.h>
#include <limits.h>
#include <slab.h>
#include <vendor_fusion.h>

#define ULONG_LONG_MAX 0XFFFFFFFFFFFFFFFFULL


#define MAX_NUM_COMMS_EVENT_SAMPLES 15 // at most 15 samples can fit in one comms_event
#define NUM_COMMS_EVENTS_IN_FIFO    2  // This controls how often the hub needs to wake up in batching
#define FIFO_DEPTH                  (NUM_COMMS_EVENTS_IN_FIFO * MAX_NUM_COMMS_EVENT_SAMPLES)  // needs to be greater than max raw sensor rate ratio
/*
 * FIFO_MARGIN: max raw sensor rate ratio is 8:1.
 * If 2 batchs of high rate data comes before 1 low rate data, there can be at max 15 samples left in the FIFO
 */
#define FIFO_MARGIN                 15
#define MAX_NUM_SAMPLES             (FIFO_MARGIN + FIFO_DEPTH) // actual input sample fifo depth
#define EVT_SENSOR_ACC_DATA_RDY     sensorGetMyEventType(SENS_TYPE_ACCEL)
#define EVT_SENSOR_GYR_DATA_RDY     sensorGetMyEventType(SENS_TYPE_GYRO)
#define EVT_SENSOR_MAG_DATA_RDY     sensorGetMyEventType(SENS_TYPE_MAG)

#define kGravityEarth 9.8f
#define MIN_ACC_RATE_HZ             SENSOR_HZ(200.0f)
#define MIN_GYRO_RATE_HZ            SENSOR_HZ(200.0f)
#define MAX_MAG_RATE_HZ             SENSOR_HZ(100.0f)

enum RawSensorType
{
    ACC,
    GYR,
    MAG,
    NUM_OF_RAW_SENSOR
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
    bool use_gyro_data;
    bool use_mag_data;
    uint8_t idx;
};

struct FusionTask {
    uint32_t tid;
    uint32_t accelHandle;
    uint32_t gyroHandle;
    uint32_t magHandle;

    struct FusionSensor sensors[NUM_OF_FUSION_SENSOR];
    struct FusionSensorSample samples[NUM_OF_RAW_SENSOR][MAX_NUM_SAMPLES];
    size_t sample_counts[NUM_OF_RAW_SENSOR];
    size_t drop_division[NUM_OF_FUSION_SENSOR];
    size_t drop_counter[NUM_OF_FUSION_SENSOR];
    uint64_t ResamplePeriodNs[NUM_OF_FUSION_SENSOR];
    uint64_t last_time[NUM_OF_RAW_SENSOR];
    struct TripleAxisDataPoint last_sample[NUM_OF_RAW_SENSOR];

    uint32_t flags;

    uint32_t raw_sensor_rate[NUM_OF_RAW_SENSOR];
    uint64_t raw_sensor_latency;

    uint8_t accel_client_cnt;
    uint8_t gyro_client_cnt;
    uint8_t mag_client_cnt;

    bool hwGyroSupported;
    struct VendorFusionInterfact_t *interface;
};

static uint32_t FusionRates[] = {
    SENSOR_HZ(12.5f),
    SENSOR_HZ(25.0f),
    SENSOR_HZ(50.0f),
    SENSOR_HZ(100.0f),
    SENSOR_HZ(200.0f),
    SENSOR_HZ(400.0f),
    0,
};

static const uint64_t rateTimerVals[] = //should match "supported rates in length" and be the timer length for that rate in nanosecs
{
    1000000000ULL / 12.5f,
    1000000000ULL / 25,
    1000000000ULL / 50,
    1000000000ULL / 100,
    1000000000ULL / 200,
    1000000000ULL / 400,
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
    { DEC_INFO_RATE("Virtual Gyroscope", FusionRates, SENS_TYPE_GYRO, NUM_AXIS_THREE, NANOHUB_INT_NONWAKEUP, 20) },
};

static struct SlabAllocator *mDataSlab;

static size_t sensorDivisionLookup(uint32_t wantedRate)
{
    size_t division = 1;
    if (wantedRate <= SENSOR_HZ(50.0f))
        division = 4;
    else if (wantedRate <= SENSOR_HZ(100.0f))
        division = 2;
    else
        division = 1;
    return division;
}

static void dataEvtFree(void *ptr)
{
    slabAllocatorFree(mDataSlab, ptr);
}

static void fillSamples(struct TripleAxisDataEvent *ev, enum RawSensorType index)
{
    size_t i = 0, num_samples;
    struct TripleAxisDataPoint *curr_sample, *next_sample;
    uint64_t curr_time, next_time;

    if (index == GYR && mTask.gyro_client_cnt == 0) {
        return;
    }
    if (index == MAG && mTask.mag_client_cnt == 0) {
        return;
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

        mTask.samples[index][i].x = curr_sample->x;
        mTask.samples[index][i].y = curr_sample->y;
        mTask.samples[index][i].z = curr_sample->z;
        mTask.samples[index][i].time = curr_time;
        //osLog(LOG_ERROR, "vendor fusion: index=%d, [%lld %f, %f, %f]\n", index, curr_time,
            //(double)mTask.samples[index][i].x, (double)mTask.samples[index][i].y, (double)mTask.samples[index][i].z);
        i++;
    }

    mTask.sample_counts[index] = i;
    mTask.last_sample[index] = *curr_sample;
    mTask.last_time[index] = curr_time;
}

static bool allocateDataEvt(struct FusionSensor *mSensor, uint64_t time)
{
    mSensor->ev = slabAllocatorAlloc(mDataSlab);
    if (mSensor->ev == NULL) {
        // slab allocation failed
        osLog(LOG_ERROR, "vendor fusion: slabAllocatorAlloc() Failed\n");
        return false;
    }

    // delta time for the first sample is sample count
    memset(&mSensor->ev->samples[0].firstSample, 0x00, sizeof(struct SensorFirstSample));
    mSensor->ev->referenceTime = time;
    mSensor->prev_time = time;

    return true;
}

static void addSample(struct FusionSensor *mSensor, uint64_t time, float x, float y, float z)
{
    struct TripleAxisDataPoint *sample;

    if (mSensor->ev == NULL) {
        if (!allocateDataEvt(mSensor, time))
            return;
    }

    if (mSensor->ev->samples[0].firstSample.numSamples >= MAX_NUM_COMMS_EVENT_SAMPLES) {
        osLog(LOG_ERROR, "vendor fusion: BAD_INDEX\n");
        return;
    }

    sample = &mSensor->ev->samples[mSensor->ev->samples[0].firstSample.numSamples++];

    if (mSensor->ev->samples[0].firstSample.numSamples > 1) {
        sample->deltaTime = time > mSensor->prev_time ? (time - mSensor->prev_time) : 0;
        mSensor->prev_time = time;
    }

    sample->x = x;
    sample->y = y;
    sample->z = z;

    if (mSensor->ev->samples[0].firstSample.numSamples == MAX_NUM_COMMS_EVENT_SAMPLES) {
        osEnqueueEvtOrFree(EVENT_TYPE_BIT_DISCARDABLE | sensorGetMyEventType(mSi[mSensor->idx].sensorType),
                           mSensor->ev, dataEvtFree);
        mSensor->ev = NULL;
    }
}
static void updateOutput(ssize_t last_accel_sample_index, uint64_t last_sensor_time)
{
    struct InterfaceDataOut dataOut;

    if (mTask.sensors[ORIENT].active) {
        if (mTask.interface && mTask.interface->getOrientation) {
            mTask.drop_counter[ORIENT]++;
            if (mTask.drop_counter[ORIENT] % mTask.drop_division[ORIENT] == 0) {
                mTask.interface->getOrientation(&dataOut);
                addSample(&mTask.sensors[ORIENT],
                    last_sensor_time,
                    dataOut.vec[0],
                    dataOut.vec[1],
                    dataOut.vec[2]);
                mTask.drop_counter[ORIENT] = 0;
            }
        }
    }
    if (mTask.sensors[GYRO].active) {
        if (mTask.interface && mTask.interface->getVirtualGyro) {
            mTask.drop_counter[GYRO]++;
            if (mTask.drop_counter[GYRO] % mTask.drop_division[GYRO] == 0) {
                mTask.interface->getVirtualGyro(&dataOut);
                addSample(&mTask.sensors[GYRO],
                        last_sensor_time,
                        dataOut.vec[0],
                        dataOut.vec[1],
                        dataOut.vec[2]);
            }
        }
    }
    if (mTask.sensors[GRAVITY].active) {
        if (mTask.interface && mTask.interface->getGravity) {
            mTask.drop_counter[GRAVITY]++;
            if (mTask.drop_counter[GRAVITY] % mTask.drop_division[GRAVITY] == 0) {
                mTask.interface->getGravity(&dataOut);
                addSample(&mTask.sensors[GRAVITY],
                    last_sensor_time,
                    dataOut.vec[0],
                    dataOut.vec[1],
                    dataOut.vec[2]);
                 mTask.drop_counter[GRAVITY] = 0;
            }
        }
    }
    if (mTask.sensors[LINEAR].active) {
        if (mTask.interface && mTask.interface->getLinearaccel) {
            mTask.drop_counter[LINEAR]++;
            if (mTask.drop_counter[LINEAR] % mTask.drop_division[LINEAR] == 0) {
                mTask.interface->getLinearaccel(&dataOut);
                addSample(&mTask.sensors[LINEAR],
                        last_sensor_time,
                        dataOut.vec[0],
                        dataOut.vec[1],
                        dataOut.vec[2]);
                mTask.drop_counter[LINEAR] = 0;
            }
        }
    }
    if (mTask.sensors[ROTAT].active) {
        if (mTask.interface && mTask.interface->getRotationVector) {
            mTask.drop_counter[ROTAT]++;
            if (mTask.drop_counter[ROTAT] % mTask.drop_division[ROTAT] == 0) {
                mTask.interface->getRotationVector(&dataOut);
                addSample(&mTask.sensors[ROTAT],
                        last_sensor_time,
                        dataOut.vec[0],
                        dataOut.vec[1],
                        dataOut.vec[2]);
                mTask.drop_counter[ROTAT] = 0;
            }
        }
    }
    if (mTask.sensors[GAME].active) {
        if (mTask.interface && mTask.interface->getGameRotationVector) {
            mTask.drop_counter[GAME]++;
            if (mTask.drop_counter[GAME] % mTask.drop_division[GAME] == 0) {
                mTask.interface->getGameRotationVector(&dataOut);
                addSample(&mTask.sensors[GAME],
                        last_sensor_time,
                        dataOut.vec[0],
                        dataOut.vec[1],
                        dataOut.vec[2]);
                mTask.drop_counter[GAME] = 0;
            }
        }
    }
    if (mTask.sensors[GEOMAG].active) {
        if (mTask.interface && mTask.interface->getGeoMagnetic) {
            mTask.drop_counter[GEOMAG]++;
            if (mTask.drop_counter[GEOMAG] % mTask.drop_division[GEOMAG] == 0) {
                mTask.interface->getGeoMagnetic(&dataOut);
                addSample(&mTask.sensors[GEOMAG],
                        last_sensor_time,
                        dataOut.vec[0],
                        dataOut.vec[1],
                        dataOut.vec[2]);
                mTask.drop_counter[GEOMAG] = 0;
            }
        }
    }
}
static void drainSamples()
{
    struct InterfaceDataIn dataIn;
    size_t i = 0;
    size_t j = 0;
    size_t k = 0;

    while (mTask.sample_counts[GYR] > 0) {
        dataIn.vec[0] = mTask.samples[GYR][i].x;
        dataIn.vec[1] = mTask.samples[GYR][i].y;
        dataIn.vec[2] = mTask.samples[GYR][i].z;
        dataIn.timeStamp = mTask.samples[GYR][i].time;
        if (mTask.interface && mTask.interface->setGyroData)
            mTask.interface->setGyroData(&dataIn);
        if (mTask.hwGyroSupported)
            updateOutput(i, mTask.samples[GYR][i].time);
        --mTask.sample_counts[GYR];
        i++;
    }
    while (mTask.sample_counts[ACC] > 0) {
        dataIn.vec[0] = mTask.samples[ACC][j].x;
        dataIn.vec[1] = mTask.samples[ACC][j].y;
        dataIn.vec[2] = mTask.samples[ACC][j].z;
        dataIn.timeStamp = mTask.samples[ACC][j].time;
        if (mTask.interface && mTask.interface->setAccData)
            mTask.interface->setAccData(&dataIn);
        if (!mTask.hwGyroSupported)
            updateOutput(j, mTask.samples[ACC][j].time);
        --mTask.sample_counts[ACC];
        j++;
    }
    while (mTask.sample_counts[MAG] > 0) {
        dataIn.vec[0] = mTask.samples[MAG][k].x;
        dataIn.vec[1] = mTask.samples[MAG][k].y;
        dataIn.vec[2] = mTask.samples[MAG][k].z;
        dataIn.timeStamp = mTask.samples[MAG][k].time;
        if (mTask.interface && mTask.interface->setMagData)
            mTask.interface->setMagData(&dataIn);
        --mTask.sample_counts[MAG];
        k++;
    }

    for (i = ORIENT; i < NUM_OF_FUSION_SENSOR; i++) {
        if (mTask.sensors[i].ev != NULL) {
            osEnqueueEvtOrFree(EVENT_TYPE_BIT_DISCARDABLE | sensorGetMyEventType(mSi[i].sensorType),
                               mTask.sensors[i].ev, dataEvtFree);
            mTask.sensors[i].ev = NULL;
        }
    }
}

static void fusionSetRateAcc(void)
{
    int i;
    if (mTask.accelHandle == 0) {
        mTask.sample_counts[ACC] = 0;
        mTask.last_time[ACC] = ULONG_LONG_MAX;
        for (i = 0; sensorFind(SENS_TYPE_ACCEL, i, &mTask.accelHandle) != NULL; i++) {
            if (sensorRequest(mTask.tid, mTask.accelHandle, mTask.raw_sensor_rate[ACC], mTask.raw_sensor_latency)) {
                osEventSubscribe(mTask.tid, EVT_SENSOR_ACC_DATA_RDY);
                break;
            }
        }
    } else {
        sensorRequestRateChange(mTask.tid, mTask.accelHandle, mTask.raw_sensor_rate[ACC], mTask.raw_sensor_latency);
    }
}

static void fusionSetRateGyr(void)
{
    int i;
    if (mTask.gyroHandle == 0) {
        mTask.sample_counts[GYR] = 0;
        mTask.last_time[GYR] = ULONG_LONG_MAX;
        for (i = 0; sensorFind(SENS_TYPE_GYRO, i, &mTask.gyroHandle) != NULL; i++) {
            if (sensorRequest(mTask.tid, mTask.gyroHandle, mTask.raw_sensor_rate[GYR], mTask.raw_sensor_latency)) {
                osEventSubscribe(mTask.tid, EVT_SENSOR_GYR_DATA_RDY);
                break;
            }
        }
    } else {
        sensorRequestRateChange(mTask.tid, mTask.gyroHandle, mTask.raw_sensor_rate[GYR], mTask.raw_sensor_latency);
    }
}
static void fusionSetRateMag(void)
{
    int i;
    if (mTask.magHandle == 0) {
        mTask.sample_counts[MAG] = 0;
        mTask.last_time[MAG] = ULONG_LONG_MAX;
        for (i = 0; sensorFind(SENS_TYPE_MAG, i, &mTask.magHandle) != NULL; i++) {
            if (sensorRequest(mTask.tid, mTask.magHandle, mTask.raw_sensor_rate[MAG], mTask.raw_sensor_latency)) {
                osEventSubscribe(mTask.tid, EVT_SENSOR_MAG_DATA_RDY);
                break;
            }
        }
    } else {
        sensorRequestRateChange(mTask.tid, mTask.magHandle, mTask.raw_sensor_rate[MAG], mTask.raw_sensor_latency);
    }
}

static bool fusionSetRate(uint32_t rate, uint64_t latency, void *cookie)
{
    struct FusionSensor *mSensor = &mTask.sensors[(int)cookie];
    int i, real_active = 0;
    uint32_t max_rate = 0;
    uint32_t acc_rate, gyr_rate, mag_rate;
    uint64_t min_resample_period = ULONG_LONG_MAX;
    struct InterfaceEnParam enParam;

    mSensor->rate = rate;
    mSensor->latency = latency;

    osLog(LOG_INFO, "fusionSetRate: %d, %u, %llu\n", (int)cookie, rate, latency);

    if (mSensor->rate > 0)
        mTask.drop_division[mSensor->idx] = sensorDivisionLookup(mSensor->rate);

    for (i = ORIENT; i < NUM_OF_FUSION_SENSOR; i++) {
        if (mTask.sensors[i].active) {
            max_rate = max_rate > mTask.sensors[i].rate ? max_rate : mTask.sensors[i].rate;
        }
    }

    if (mTask.accel_client_cnt > 0) {
        if (!mTask.hwGyroSupported)
            acc_rate = max_rate > MIN_ACC_RATE_HZ ? max_rate : MIN_ACC_RATE_HZ;
        else
            acc_rate = max_rate;
        mTask.raw_sensor_rate[ACC] = acc_rate;
        mTask.ResamplePeriodNs[ACC] = sensorTimerLookupCommon(FusionRates, rateTimerVals, acc_rate);
        min_resample_period = mTask.ResamplePeriodNs[ACC] < min_resample_period ?
            mTask.ResamplePeriodNs[ACC] : min_resample_period;
    }
    if (mTask.gyro_client_cnt > 0) {
        gyr_rate = max_rate > MIN_GYRO_RATE_HZ ? max_rate : MIN_GYRO_RATE_HZ;
        mTask.raw_sensor_rate[GYR] = gyr_rate;
        mTask.ResamplePeriodNs[GYR] = sensorTimerLookupCommon(FusionRates, rateTimerVals, gyr_rate);
        min_resample_period = mTask.ResamplePeriodNs[GYR] < min_resample_period ?
            mTask.ResamplePeriodNs[GYR] : min_resample_period;
    }
    if (mTask.mag_client_cnt > 0) {
        mag_rate = max_rate < MAX_MAG_RATE_HZ ? max_rate : MAX_MAG_RATE_HZ;
        mTask.raw_sensor_rate[MAG] = mag_rate;
        mTask.ResamplePeriodNs[MAG] = sensorTimerLookupCommon(FusionRates, rateTimerVals, mag_rate);
        min_resample_period = mTask.ResamplePeriodNs[MAG] < min_resample_period ?
            mTask.ResamplePeriodNs[MAG] : min_resample_period;
    }

    // This guarantees that local raw sensor FIFOs won't overflow.
    mTask.raw_sensor_latency = min_resample_period * (FIFO_DEPTH - 1);

    for (i = ORIENT; i < NUM_OF_FUSION_SENSOR; i++) {
        if (mTask.sensors[i].active) {
            mTask.raw_sensor_latency = mTask.sensors[i].latency < mTask.raw_sensor_latency ?
                mTask.sensors[i].latency : mTask.raw_sensor_latency;
        }
    }
    //osLog(LOG_INFO, "min_resample_period: %lld, raw_sensor_latency: %lld\n", min_resample_period, mTask.raw_sensor_latency);

    if (mTask.accel_client_cnt > 0)
        fusionSetRateAcc();
    if (mTask.gyro_client_cnt > 0)
        fusionSetRateGyr();
    if (mTask.mag_client_cnt > 0)
        fusionSetRateMag();

    for (i = ORIENT; i < NUM_OF_FUSION_SENSOR; i++) {
        real_active |= mTask.sensors[i].active;
    }
    memset(&enParam, 0, sizeof(struct InterfaceEnParam));
    enParam.en = real_active;
    enParam.latency = (mTask.raw_sensor_latency < min_resample_period) ? 0 : mTask.raw_sensor_latency;
    enParam.sensorType = (enum FusionSensorType)cookie;
    if (mTask.interface && mTask.interface->enableLib)
        mTask.interface->enableLib(&enParam);
    if (mSensor->rate > 0)
        sensorSignalInternalEvt(mSensor->handle, SENSOR_INTERNAL_EVT_RATE_CHG, rate, latency);

    return true;
}

static bool fusionPower(bool on, void *cookie)
{
    struct FusionSensor *mSensor = &mTask.sensors[(int)cookie];
    int idx, i = 0, real_active = 0;
    struct InterfaceEnParam enParam;

    osLog(LOG_INFO, "fusionPower:  %d\n", (int)cookie);

    mSensor->active = on;
    if (on == false) {
        mTask.accel_client_cnt--;
        if (mSensor->use_gyro_data)
            mTask.gyro_client_cnt--;
        if (mSensor->use_mag_data)
            mTask.mag_client_cnt--;

        // if client_cnt == 0 and Handle == 0, nothing need to be done.
        // if client_cnt > 0 and Handle == 0, something else is turning it on, all will be done.
        if (mTask.accel_client_cnt == 0 && mTask.accelHandle != 0) {
            sensorRelease(mTask.tid, mTask.accelHandle);
            mTask.accelHandle = 0;
            osEventUnsubscribe(mTask.tid, EVT_SENSOR_ACC_DATA_RDY);
        }

        if (mTask.gyro_client_cnt == 0 && mTask.gyroHandle != 0) {
            sensorRelease(mTask.tid, mTask.gyroHandle);
            mTask.gyroHandle = 0;
            osEventUnsubscribe(mTask.tid, EVT_SENSOR_GYR_DATA_RDY);
        }

        if (mTask.mag_client_cnt == 0 && mTask.magHandle != 0) {
            sensorRelease(mTask.tid, mTask.magHandle);
            mTask.magHandle = 0;
            osEventUnsubscribe(mTask.tid, EVT_SENSOR_MAG_DATA_RDY);
        }

        idx = mSensor->idx;
        (void) fusionSetRate(0, ULONG_LONG_MAX, (void *)idx);
    } else {
        mTask.accel_client_cnt++;
        if (mSensor->use_gyro_data)
            mTask.gyro_client_cnt++;
        if (mSensor->use_mag_data)
            mTask.mag_client_cnt++;
    }

    for (i = ORIENT; i < NUM_OF_FUSION_SENSOR; i++) {
        real_active |= mTask.sensors[i].active;
    }
    if (real_active == false && on == false) {
        memset(&enParam, 0, sizeof(struct InterfaceEnParam));
        enParam.en = on;
        enParam.sensorType = (enum FusionSensorType)cookie;
        if (mTask.interface && mTask.interface->enableLib)
            mTask.interface->enableLib(&enParam);
    }
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
    struct TripleAxisDataEvent *ev;
    int i;

    if (evtData == SENSOR_DATA_EVENT_FLUSH)
        return;

    switch (evtType) {
    case EVT_APP_START:
        osEventUnsubscribe(mTask.tid, EVT_APP_START);
        mTask.gyroHandle = 0;
        if (!sensorFind(SENS_TYPE_MAG, 0, &mTask.magHandle)) {
            for (i = ORIENT; i < NUM_OF_FUSION_SENSOR; i++)
                mTask.sensors[i].use_mag_data = false;
        }
        mTask.magHandle = 0;
        break;
    case EVT_SENSOR_ACC_DATA_RDY:
        ev = (struct TripleAxisDataEvent *)evtData;
        fillSamples(ev, ACC);
        drainSamples();
        break;
    case EVT_SENSOR_GYR_DATA_RDY:
        ev = (struct TripleAxisDataEvent *)evtData;
        fillSamples(ev, GYR);
        drainSamples();
        break;
    case EVT_SENSOR_MAG_DATA_RDY:
        ev = (struct TripleAxisDataEvent *)evtData;
        fillSamples(ev, MAG);
        drainSamples();
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
    size_t i, slabSize;
    uint32_t handle = 0;

    mTask.tid = tid;

    if (mTask.interface)
        osLog(LOG_INFO, "vendor fusion, tid=%ld, libname=%s\n",
            tid, mTask.interface->name);
    else
        osLog(LOG_INFO, "vendor fusion, tid=%ld, libname=NULL\n",
            tid);

    for (i = 0; i < NUM_OF_RAW_SENSOR; i++) {
         mTask.sample_counts[i] = 0;
    }

    if (sensorFind(SENS_TYPE_GYRO, 0, &handle) == NULL) {
        mTask.sensors[GYRO].handle =
            sensorRegister(&mSi[GYRO], &mSops, (void *)GYRO, true);
        mTask.sensors[GYRO].idx = GYRO;
        mTask.sensors[GYRO].use_gyro_data = false;
        mTask.sensors[GYRO].use_mag_data = true;
        mTask.hwGyroSupported = false;
        osLog(LOG_INFO, "vendor fusion hwGyroNotSupported\n");
    } else {
        mTask.hwGyroSupported = true;
        osLog(LOG_INFO, "vendor fusion hwGyroSupported\n");
    }

    if (mTask.interface && mTask.interface->initLib)
        mTask.interface->initLib(mTask.hwGyroSupported);

    for (i = ORIENT; i <= ROTAT; i++) {
        mTask.sensors[i].handle = sensorRegister(&mSi[i], &mSops, (void *)i, true);
        mTask.sensors[i].idx = i;
        if (mTask.hwGyroSupported)
            mTask.sensors[i].use_gyro_data = true;
        else
            mTask.sensors[i].use_gyro_data = false;
        mTask.sensors[i].use_mag_data = true;
    }

    //mTask.sensors[GEOMAG].use_gyro_data = false;
    //mTask.sensors[GAME].use_mag_data = false;
    //mTask.sensors[GRAVITY].use_mag_data = false;
#ifdef CFG_MEMSIC_TINY_FUSION_SUPPORT
    mTask.sensors[GAME].use_mag_data = false;
    mTask.sensors[GRAVITY].use_mag_data = false;
    mTask.sensors[LINEAR].use_mag_data = false;
#endif

    mTask.accel_client_cnt = 0;
    mTask.gyro_client_cnt = 0;
    mTask.mag_client_cnt = 0;

    slabSize = sizeof(struct TripleAxisDataEvent)
        + MAX_NUM_COMMS_EVENT_SAMPLES * sizeof(struct TripleAxisDataPoint);
    mDataSlab = slabAllocatorNew(slabSize, 4, 6 * (NUM_COMMS_EVENTS_IN_FIFO + 1));
    if (!mDataSlab) {
        osLog(LOG_ERROR, "vendor fusion: slabAllocatorNew() FAILED\n");
        return false;
    }

    osEventSubscribe(mTask.tid, EVT_APP_START);

    return true;
}

static void fusionEnd()
{
    slabAllocatorDestroy(mDataSlab);
}

void registerVendorInterface(struct VendorFusionInterfact_t *interface)
{
    mTask.interface = interface;
}

INTERNAL_APP_INIT(
        APP_ID_MAKE(APP_ID_VENDOR_MTK, MTK_APP_ID_WRAP(SENS_TYPE_ORIENTATION, 0, 0)),
        0,
        fusionStart,
        fusionEnd,
        fusionHandleEvent);
