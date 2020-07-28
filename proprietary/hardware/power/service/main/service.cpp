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

#define LOG_TAG "mtkpower@1.0-service"

#include <android/hardware/power/1.3/IPower.h>
#include <vendor/mediatek/hardware/power/2.1/IPower.h>
#include <vendor/mediatek/hardware/mtkpower/1.0/IMtkPerf.h>
#include <vendor/mediatek/hardware/mtkpower/1.0/IMtkPower.h>
#include <hidl/LegacySupport.h>
#include <unistd.h>
#include <pthread.h>
#include "PowerManager.h"

using android::hardware::power::V1_3::IPower;
using vendor::mediatek::hardware::mtkpower::V1_0::IMtkPerf;
using vendor::mediatek::hardware::mtkpower::V1_0::IMtkPower;
//using android::hardware::defaultPassthroughServiceImplementation;
using ::android::hardware::registerPassthroughServiceImplementation;
using ::android::hardware::configureRpcThreadpool;
using ::android::hardware::joinRpcThreadpool;
using ::android::OK;
using ::android::status_t;

pthread_mutex_t g_mutex;
pthread_cond_t  g_cond;
bool powerd_done = false;

#define register(service) do { \
    status_t err = registerPassthroughServiceImplementation<service>(); \
    if (err != OK) { \
        ALOGE("Err %d while registering " #service, err); \
    } \
} while(false)

#if 0
void* mtkPowerHandler(void *data)
{
    ALOGI("mtkPowerHandler - data:%p", data);

    while(1) {
        pthread_mutex_lock(&g_mutex);
        pthread_cond_wait(&g_cond, &g_mutex);
        ALOGI("mtkPowerHandler - TODO");
        pthread_mutex_unlock(&g_mutex);
    }
}
#endif

void* mtkPowerService(void *data)
{
    //int ret;
    ALOGV("mtkPowerService - data:%p", data);

    //nice(-10);

    // set name
    //pthread_setname_np(*(pthread_t*)data, "mtkPowerService");
    pthread_mutex_lock(&g_mutex);
    while (powerd_done == false) {
       pthread_cond_wait(&g_cond, &g_mutex);
    }
    pthread_mutex_unlock(&g_mutex);

    configureRpcThreadpool(1, true /* will call join */);

#ifdef MTK_POWER_HAL_V_2_1_SUPPORT
    ALOGI("mtkPowerService register legacy IPower");
    register(vendor::mediatek::hardware::power::V2_1::IPower);
#endif

    ALOGI("mtkPowerService register service");
    register(IPower);
    ALOGI("mtkPowerService register IMtkPower");
    register(IMtkPower);
    ALOGI("mtkPowerService register IMtkPerf");
    register(IMtkPerf);

    ALOGI("mtkPowerService joinRpcThreadpool");
    joinRpcThreadpool();

    return NULL;
}

int main() {
    pthread_t handlerThread, serviceThread;
    pthread_attr_t attr;
    //struct sched_param param;

    /* init */
    pthread_mutex_init(&g_mutex, NULL);
    pthread_cond_init(&g_cond, NULL);

    /* handler */
    pthread_attr_init(&attr);
    pthread_create(&handlerThread, &attr, mtkPowerManager, NULL);
    pthread_setname_np(handlerThread, "mtkPowerManager");

    /* service */
    pthread_attr_init(&attr);
    pthread_create(&serviceThread, &attr, mtkPowerService, (void*)&serviceThread);
    pthread_setname_np(serviceThread, "mtkPowerService");

    pthread_join(handlerThread, NULL);
    pthread_join(serviceThread, NULL);
    return 0;
    //return defaultPassthroughServiceImplementation<IPower>();
}

