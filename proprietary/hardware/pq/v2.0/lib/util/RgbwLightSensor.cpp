/*
 * Copyright (C) 2010 The Android Open Source Project
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

#include <inttypes.h>
#include <android/sensor.h>
#include <android/log.h>
#include <fcntl.h>
#include <dlfcn.h>
#include "linux/hwmsensor.h"
#include <ASensorEventQueue.h>
#include <ASensorManager.h>


#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "PQRGBW"
#define RGBW_LOGD(fmt, ...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "%s: " fmt, __FUNCTION__, ##__VA_ARGS__)

typedef void (*PQSensorNotify)(void* user, int32_t r, int32_t g, int32_t b, int32_t w);
PQSensorNotify gPQSensorNotify;
void *gPQSensorUser;

typedef void (*PQSensorDied)(void* user, bool isDied);
PQSensorDied gPQSensorDied;
void *gPQSensorDiedUser;

#define UNUSED(expr) do { (void)(expr); } while (0)

namespace android {
using android::frameworks::sensorservice::V1_0::ISensorManager;
using android::frameworks::sensorservice::V1_0::Result;
using android::hardware::sensors::V1_0::SensorInfo;
using android::hardware::sensors::V1_0::SensorType;
using android::frameworks::sensorservice::V1_0::IEventQueueCallback;
using android::hardware::Return;
using android::hardware::sensors::V1_0::Event;
using android::frameworks::sensorservice::V1_0::IEventQueue;

static int64_t gRGBWTimeStamp;
static bool gRGBWLightInitFlag;
static float gRGBWValue[4];
static Mutex gLock;
static sp<ISensorManager> gSensorMgr;
static sp<IEventQueue>    gSensorEventQueue;
static int32_t gRGBWSensorHandle = 0;

class RGBWCallback : public IEventQueueCallback {
public:
    Return<void> onEvent(const Event &e) {
        Mutex::Autolock _l(gLock);
        sensors_event_t sensorEvent;

        android::hardware::sensors::V1_0::implementation::convertToSensorEvent(e, &sensorEvent);

        gRGBWTimeStamp = sensorEvent.timestamp;
        gRGBWValue[0] = sensorEvent.data[0];
        gRGBWValue[1] = sensorEvent.data[1];
        gRGBWValue[2] = sensorEvent.data[2];
        gRGBWValue[3] = sensorEvent.data[3];
        //RGBW_LOGD("timestamp[%d] data[0][%f] data[1][%f] data[2][%f] data[3][%f]", (int)gRGBWTimeStamp, gRGBWValue[0], gRGBWValue[1], gRGBWValue[2], gRGBWValue[3]);
        if (gPQSensorNotify != NULL) {
            gPQSensorNotify(gPQSensorUser,
                (int)gRGBWValue[0],
                (int)gRGBWValue[1],
                (int)gRGBWValue[2],
                (int)gRGBWValue[3]
            );
        }

        return android::hardware::Void();
    }
};

struct SensorDeathRecipient : public android::hardware::hidl_death_recipient
{
public:
    // hidl_death_recipient interface
    virtual void serviceDied(uint64_t cookie,
        const ::android::wp<::android::hidl::base::V1_0::IBase>& who){
        RGBW_LOGD("sensor service died");
        if (gPQSensorDied != NULL){
            gPQSensorDied(gPQSensorDiedUser, true);
        }

        gSensorMgr = NULL;
        gSensorEventQueue = NULL;
        UNUSED(cookie);
        UNUSED(who);
    }
};

static sp<RGBWCallback> gRGBWCallback;
static sp<SensorDeathRecipient> mpDeathRecipient;

int rgbwLightSensorEnable(int enable)
{
    Mutex::Autolock _l(gLock);

    Result ret;
    if (enable == 1) {
        ret = gSensorEventQueue->enableSensor(gRGBWSensorHandle, 200 * 1000 /* sample period */, 0 /* latency */);
        RGBW_LOGD("enable RGBW sensor ret[%d]", ret);
    } else {
        ret = gSensorEventQueue->disableSensor(gRGBWSensorHandle);
        RGBW_LOGD("disable RGBW sensor ret[%d]", ret);
    }

    if (ret == Result::OK)
        return 1;
    else
        return -1;
}

int rgbwLightSensorInit(void)
{
#ifdef MTK_CHAMELEON_DISPLAY_SUPPORT
    gSensorMgr = ISensorManager::tryGetService();

    if (gSensorMgr == NULL)
    {
        RGBW_LOGD("get SensorManager FAIL!\n");
        return -1;
    }
    RGBW_LOGD("get SensorManager SUCCESS!\n");

    if (mpDeathRecipient == NULL)
    {
        mpDeathRecipient = new SensorDeathRecipient();
        ::android::hardware::Return<bool> linked = gSensorMgr->linkToDeath(mpDeathRecipient, /*cookie*/ 0);
        if (!linked || !linked.isOk()) {
            RGBW_LOGD("Unable to link to sensor service death notifications");
            return -1;
        }
    }


    gSensorMgr->getDefaultSensor((::android::hardware::sensors::V1_0::SensorType)SENSOR_TYPE_RGBW,
        [&](const SensorInfo& sensor, Result ret) {
            ret == Result::OK ? gRGBWSensorHandle = sensor.sensorHandle : gRGBWSensorHandle = -1;
        });
#else
    gRGBWSensorHandle = -1;
#endif

    if (gRGBWSensorHandle == -1)
    {
        RGBW_LOGD("get DefaultSensor FAIL! %d", gRGBWSensorHandle);
        return -1;
    }
    gRGBWCallback = new RGBWCallback();
    gSensorMgr->createEventQueue(gRGBWCallback,
        [&](const sp<IEventQueue>& queue, Result ret) {
            ret == Result::OK ? gSensorEventQueue = queue : gSensorEventQueue = NULL;
        });

    if (gSensorEventQueue == NULL)
    {
        RGBW_LOGD("createEventQueue FAIL!");
        return -1;
    }

    //enable sensor
    if (gSensorEventQueue->enableSensor(gRGBWSensorHandle, 200 * 1000 , 0) != Result::OK)
    {
        RGBW_LOGD("enable Sensor FAIL!");
        return -1;
    }
    else
    {
        rgbwLightSensorEnable(1);
    }

    gRGBWLightInitFlag = true;
    return 1;
}

void rgbwLightSensorDeInit(void)
{
    if (gSensorMgr != NULL)
    {
        gSensorMgr = NULL;
    }
    if (gSensorEventQueue != NULL)
    {
        ::android::hardware::Return<Result> ret = gSensorEventQueue->disableSensor(gRGBWSensorHandle);
        if (!ret.isOk())
        {
            RGBW_LOGD("disable rgbw sensor fail");
        }
        gSensorEventQueue = NULL;
    }
    if (gRGBWCallback != NULL)
    {
        gRGBWCallback = NULL;
    }
    gRGBWLightInitFlag = false;
}

};

extern "C" {

void pqSetListener(PQSensorNotify pqSensorNotify, void *user) {
    gPQSensorNotify = pqSensorNotify;
    gPQSensorUser = user;
}

void pqSetDiedListener(PQSensorDied pqSensorDied, void *user) {
    gPQSensorDied = pqSensorDied;
    gPQSensorDiedUser = user;
}


int pqLightSensorInit(void) {
    return android::rgbwLightSensorInit();
}

void pqLightSensorDeInit(void) {
    android::rgbwLightSensorDeInit();
}

int pqLightSensorEnable(int enable) {
    return android::rgbwLightSensorEnable(enable);
}

}
