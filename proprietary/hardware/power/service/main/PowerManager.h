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

#ifndef ANDROID_HARDWARE_POWER_V2_0_MANAGER_H
#define ANDROID_HARDWARE_POWER_V2_0_MANAGER_H

enum {
    MSG_MTK_POWER_HINT = 0,
    MSG_NOTIFY_APP_STATE,
    MSG_SCN_REG,
    MSG_SCN_CONFIG,
    MSG_SCN_UNREG,
    MSG_SCN_ENABLE,
    MSG_SCN_DISABLE,

    MSG_TIMER_HINT_BASE = 100,
    MSG_TIMER_SCN_BASE = 200,
};


// prototype
extern pthread_mutex_t g_mutex;
extern pthread_cond_t  g_cond;
extern bool powerd_done;
extern int powerd_core_pre_init(void);
extern void* mtkPowerManager(void *data);

#endif  // ANDROID_HARDWARE_POWER_V2_0_MANAGER_H

