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

#ifndef __MTK_GPUAPPSPECTATORSERVICE_H_
#define __MTK_GPUAPPSPECTATORSERVICE_H_

#include <binder/BinderService.h>
#include <cutils/compiler.h>
#include <utils/AndroidThreads.h>
#include <utils/Condition.h>
#include <utils/Errors.h>
#include <utils/Looper.h>
#include <utils/RefBase.h>
#include <utils/threads.h>

#include <ged/ged.h>
#include <ged/ged_type.h>

#include "SceneKnowledgeProvider.h"
#include "IGpuAppSpectatorService.h"

namespace android {
//-----------------------------------------------------------------------------

/*
TODO: Binder Death Receipt
*/
class GpuAppSpectatorService :
    public BinderService<GpuAppSpectatorService>,
    public BnGpuAppSpectatorService,
    protected Thread
{
public:
    enum {
        NOTIFY_TARGET_PERFSERV,
        NOTIFY_TARGET_GED,

        NUM_NOTIFY_TARGET,
    };

    class SceneKnowledgeReceiver {

    public:
        SceneKnowledgeReceiver() :
            mClassifier(GAS_DEFAULT_CLASSIFIER),
            mPerfServNotifying(true),
            mGedNotifying(true),
            mFakeClassificationResult(-1),
            mPollingStatus(true)
        {
            resetStates();
        }

        int updateState(uint32_t provider, uint32_t state);

        int classify();

        void dump(int fd, const Vector<String16>& args);

        void resetStates() {
            for (int i = 0 ; i < NUM_SKP_PROVIDER ; i++) {
                stats[i] = SKP_HINT_UNINITIALIZED;
            }
            mAudioTrackCounter = 0;
        }

        void checkForeground(const String16& fg_app);

        void changeClassifier(uint32_t index);

        String16 mSessionHolder;

    private:

        static constexpr uint32_t GAS_DEFAULT_CLASSIFIER = 5;

        friend GpuAppSpectatorService;

        int     mAudioTrackCounter;

        uint32_t mClassifier;

        int stats[NUM_SKP_PROVIDER];

        mutable Mutex mAccessLock;

        bool mPerfServNotifying;

        bool mGedNotifying;

        int mFakeClassificationResult;

        bool mPollingStatus;

    };

    SceneKnowledgeReceiver mSceneKnowledgeReceiver;

    static char const* getServiceName() ANDROID_API {
        return "GpuAppSpectatorService";
    }

    GpuAppSpectatorService() ANDROID_API;

    void initialize();

    virtual status_t    onTransact(uint32_t code,
                                   const Parcel& data,
                                   Parcel* reply,
                                   uint32_t flags = 0);

    virtual void enableService(uint32_t enabled);

    virtual void sendAppInfo(uint32_t source, uint32_t hint, int pid);

    virtual void changeClassifier(uint32_t index);

    virtual void setGpuUtilizationThreshold(uint32_t threshold);

    // interval in ms
    virtual void setSamplingInterval(uint32_t interval);

    virtual void enableNotifying(uint32_t target, uint32_t value);

    virtual void notifyForegroundChanged(const String16 &app_name);

    virtual void fakeClassificationResult(int result);

    virtual void enablePolling(uint32_t enable);

    virtual void chooseGlpmVersion(float version);

    int checkGpuUtilization();

private:
    int     mEnableDumpsys;

    mutable Mutex mPollingLock;

    virtual ~GpuAppSpectatorService();

    virtual void onFirstRef();

    virtual bool threadLoop();

    virtual status_t dump(int fd, const Vector<String16>& args);

    Condition mCondition;

    uint32_t mGpuUtilizationThreshold;

    uint32_t mSamplingInterval;

    GED_HANDLE mGedHandle;

    uint32_t mDebugFlags;

    mutable Mutex mAccessLock;

    float mGlpmVersion;
};

//-----------------------------------------------------------------------------
}

#endif
