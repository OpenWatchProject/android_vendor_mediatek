/*
 * Copyright (C) 2008 The Android Open Source Project
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

#ifndef ActivityManager_H
#define ActivityManager_H

#include <utils/String16.h>

namespace android {

// send broadcast to DrmDialogReceiver to show refer dialog
extern bool sendBroadcastMessage(String16 data, String16 filePath);
extern bool startDrmDialogService(String16 drm_cid, String16 rightsIssuer, String16 drmMethod, int rightsStatus,
                                  String16 filePath);

}

#endif
