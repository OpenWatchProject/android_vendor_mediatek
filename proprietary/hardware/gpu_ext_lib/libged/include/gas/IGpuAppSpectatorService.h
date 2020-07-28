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

#ifndef __MTK_IGPUAPPSPECTATORSERVICE_H_
#define __MTK_IGPUAPPSPECTATORSERVICE_H_

#include <utils/Errors.h>
#include <utils/RefBase.h>

#include <binder/IInterface.h>
#include <binder/Parcel.h>

namespace android {
//-----------------------------------------------------------------------------

class IGpuAppSpectatorService: public IInterface
{
public:
    static constexpr int ENABLE_SERVICE                 = IBinder::FIRST_CALL_TRANSACTION;
    static constexpr int SEND_APP_INFO                  = IBinder::FIRST_CALL_TRANSACTION + 1;
    static constexpr int CHANGE_CLASSIFIER              = IBinder::FIRST_CALL_TRANSACTION + 2;
    static constexpr int SET_GPU_UTILIZATION_THRESHOLD  = IBinder::FIRST_CALL_TRANSACTION + 3;
    static constexpr int SET_SAMPLING_INTERVAL          = IBinder::FIRST_CALL_TRANSACTION + 4;
    static constexpr int ENABLE_NOTIFYING               = IBinder::FIRST_CALL_TRANSACTION + 5;
    static constexpr int NOTIFY_FOREGROUND_CHANGED      = IBinder::FIRST_CALL_TRANSACTION + 6;
    static constexpr int FAKE_CLASSIFICATION_RESULT     = IBinder::FIRST_CALL_TRANSACTION + 7;
    static constexpr int CHOOSE_GLPM_VERSION            = IBinder::FIRST_CALL_TRANSACTION + 8;
    static constexpr int ENABLE_POLLING                 = IBinder::FIRST_CALL_TRANSACTION + 9;

    DECLARE_META_INTERFACE(GpuAppSpectatorService);

    virtual void enableService(uint32_t enabled) = 0;

    virtual void sendAppInfo(uint32_t source, uint32_t hint, int pid) = 0;

    virtual void changeClassifier(uint32_t index) = 0;

    virtual void setGpuUtilizationThreshold(uint32_t threshold) = 0;

    virtual void setSamplingInterval(uint32_t interval) = 0;

    virtual void enableNotifying(uint32_t target, uint32_t value) = 0;

    virtual void notifyForegroundChanged(const String16& app_name) = 0;

    virtual void fakeClassificationResult(int result) = 0;

    virtual void enablePolling(uint32_t enabled) = 0;

    virtual void chooseGlpmVersion(float version) = 0;
};

//-----------------------------------------------------------------------------

class BnGpuAppSpectatorService : public BnInterface<IGpuAppSpectatorService>
{
public:
    virtual status_t    onTransact(uint32_t code,
                                   const Parcel& data,
                                   Parcel* reply,
                                   uint32_t flags = 0);
};

//-----------------------------------------------------------------------------
}; // namespace android

#endif
