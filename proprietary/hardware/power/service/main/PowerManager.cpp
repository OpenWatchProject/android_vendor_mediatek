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

#define LOG_TAG "mtkpower@1.0-impl"

#include <unistd.h>
#include <log/log.h>

#include <hardware/hardware.h>
#include <hardware/power.h>
#include <pthread.h>

#include "powerd_int.h"
#include "PowerManager.h"

static void wakeupService(void)
{
    pthread_cond_signal(&g_cond);
}

void* mtkPowerManager(void *data)
{
    pthread_mutex_lock(&g_mutex);
    if (!powerd_core_pre_init())
        powerd_done = true;
    pthread_mutex_unlock(&g_mutex);
    ALOGI("mtkPowerManager - data:%p", data);

    //nice(-10);

    powerd_main(0, NULL, wakeupService);

    return NULL;
}

