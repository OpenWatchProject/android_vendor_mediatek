/*
* Copyright (C) 2017 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
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

#ifndef MTK_HARDWARE_PRIMARY_DEVICES_HAL_CLIENT_IMPL_H
#define MTK_HARDWARE_PRIMARY_DEVICES_HAL_CLIENT_IMPL_H

#include <media/AudioParameter.h>
#include <utils/Errors.h>
#include <utils/Mutex.h>
#include "MTKPrimaryDevicesHalClientInterface.h"
#include "MTKDeviceHalInterface.h"
#include "MTKDevicesFactoryHalInterface.h"

namespace android {

class MTKPrimaryDevicesHalClientImpl: public MTKPrimaryDevicesHalClient {
public:
    status_t setParameters(const String8 &keyValuePairs);

    String8  getParameters(const String8 &keys);

    virtual ~MTKPrimaryDevicesHalClientImpl();

    MTKPrimaryDevicesHalClientImpl();
private:
    Mutex   mLock;
    sp<MTKDeviceHalInterface> mPrimaryDev;
    sp<MTKDevicesFactoryHalInterface> mDevicesFactoryHal;
};

} // namespace android

#endif // MTK_HARDWARE_PRIMARY_DEVICES_HAL_CLIENT_IMPL_H