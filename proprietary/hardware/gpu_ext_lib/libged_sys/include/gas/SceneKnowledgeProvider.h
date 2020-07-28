/*
 * Copyright (C) 2011-2014 MediaTek Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __MTK_SCENEKNOWLEDGEPROVIDER_H_
#define __MTK_SCENEKNOWLEDGEPROVIDER_H_

__BEGIN_DECLS

/* Provider enumeration */
enum SKP_Provider {
    SKP_HWC,
    SKP_DOMAIN_AUDIOTRACK,
    SKP_DOMAIN_WAKELOCK,
    SKP_DOMAIN_GPU_UTILIZATION,
    SKP_DOMAIN_EGL,

    NUM_SKP_PROVIDER
};

/* To compatible with the def of PerfServiceNative.h */
enum SceneCategory {
    SCENE_OTHERS,
    SCENE_GAME
};

enum SKP_Hints {
    SKP_HINT_HWC_OTHERS_SCENE,
    SKP_HINT_HWC_GAME_SCENE,
    SKP_HINT_AUDIOTRACK_DESTRUCT,
    SKP_HINT_AUDIOTRACK_CONSTRUCT,
    SKP_HINT_AUDIOTRACK_START,
    SKP_HINT_AUDIOTRACK_STOP,
    SKP_HINT_WAKELOCK_RELEASE,
    SKP_HINT_WAKELOCK_ACQUIRE,
    SKP_HINT_GPU_LOW_UTILIZATION,
    SKP_HINT_GPU_HIGH_UTILIZATION,
    SKP_HINT_EGL_CREATE_CONTEXT,
    SKP_HINT_EGL_DESTROY_CONTEXT,

    SKP_HINT_UNINITIALIZED,
    NUM_SKP_HINT
};

void SceneKnowledgeProvider_enablePolling(int on);
void SceneKnowledgeProvider_notifyChange(int provider, int hint, int pid = -1);

__END_DECLS

#endif
