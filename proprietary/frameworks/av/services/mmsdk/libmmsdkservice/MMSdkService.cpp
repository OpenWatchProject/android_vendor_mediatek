/*
**
** Copyright (C) 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#define LOG_TAG "mmsdk/MMSdkService"

#define MTK_LOG_ENABLE 1
#include <stdio.h>
#include <sys/types.h>

#include <dlfcn.h>
#include <binder/AppOpsManager.h>
#include <binder/IPCThreadState.h>
#include <cutils/atomic.h>
#include <cutils/properties.h>
#include <utils/Errors.h>
#include <utils/Log.h>
#include <utils/String16.h>
//#include <dlfcn.h>
//
#include <vendor/mediatek/hardware/camera/callbackclient/1.0/IMtkCallbackClient.h>
//
// Feature Manager -- bridge to EffectFactory
#include <mmsdk/IFeatureManager.h>
//
#include <mmsdk/MMSdkService.h>

using namespace android;
using namespace NSMMSdk;
using vendor::mediatek::hardware::camera::callbackclient::V1_0::IMtkCallbackClient;
using namespace hardware::camera::common::V1_0;

namespace android {


// ----------------------------------------------------------------------------
// Logging support -- this is for debugging only
// Use "adb shell dumpsys media.mmsdk -v 1" to change it.
volatile int32_t gLogLevel = 1;

#include <cutils/log.h>
#define MY_LOGV(fmt, arg...)       ALOGV(fmt"\r\n", ##arg)
#define MY_LOGD(fmt, arg...)       ALOGD(fmt, ##arg)
#define MY_LOGI(fmt, arg...)       ALOGI(fmt"\r\n", ##arg)
#define MY_LOGW(fmt, arg...)       ALOGW(fmt"\r\n", ##arg)
#define MY_LOGE(fmt, arg...)       ALOGE(fmt" (%s){#%d:%s}", ##arg, __FUNCTION__, __LINE__, __FILE__)

#define LOG1(...) ALOGD_IF(gLogLevel >= 1, __VA_ARGS__);
#define LOG2(...) ALOGD_IF(gLogLevel >= 2, __VA_ARGS__);

static void setLogLevel(int level) {
    android_atomic_write(level, &gLogLevel);
}

// ----------------------------------------------------------------------------

/******************************************************************************
 *
 ******************************************************************************/
static int getCallingPid() {
    return IPCThreadState::self()->getCallingPid();
}

/******************************************************************************
 *
 ******************************************************************************/
static int getCallingUid() {
    return IPCThreadState::self()->getCallingUid();
}

/******************************************************************************
 *
 ******************************************************************************/
MMSdkService::MMSdkService()
    : mpFeatureManager(0)
{
    mpFeatureManager = new NSCam::FeatureManager();

    ALOGI("MMSdkService started (pid=%d)", getpid());
}

/******************************************************************************
 *
 ******************************************************************************/
void MMSdkService::onFirstRef()
{
    LOG1("MMSdkService::onFirstRef");
    BnMMSdkService::onFirstRef();
}

/******************************************************************************
 *
 ******************************************************************************/
MMSdkService::~MMSdkService()
{
    LOG1("MMSdkService::~MMSdkService()");
    mpFeatureManager = NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t MMSdkService::connectFeatureManager(sp<NSCam::IFeatureManager> & featureManager)
{
    LOG1("MMSdkService::connectFeatureManager E");
    featureManager = mpFeatureManager;
    LOG1("MMSdkService::connectFeatureManager X");
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t MMSdkService::existCallbackClient()
{
    /* return 1 if CallbackClient is existence */
    LOG1("MMSdkService::existCallbackClient E");
    int enable = 0;
#if 1
    sp<IMtkCallbackClient> pCallbackClient = IMtkCallbackClient::tryGetService("internal/0");
    if (pCallbackClient != NULL)
    {
        enable = pCallbackClient->existCallbackClient() == Status::OK ? 1 : 0;
    }
    else
    {
      MY_LOGD("ERROR : pCallbackClient is NULL");
    }
//TODO, get by calling HIDL to camera hal
#else
    //
    char const szModulePath[] = "libfeatureio.featurefactory.so";
    char const szEntrySymbol[] = "existCallbackClient";
    typedef uint8_t (*PFN_T)();
    PFN_T pfnEntry = NULL;
    void *gLib = NULL;
    //
    gLib = ::dlopen(szModulePath, RTLD_NOW);
    if  ( ! gLib )
    {
        char const *err_str = ::dlerror();
        ALOGE("dlopen: %s error=%s", szModulePath, (err_str ? err_str : "unknown"));
        goto lbExit;
    }
    //
    pfnEntry = (PFN_T)::dlsym(gLib, szEntrySymbol);
    if  ( ! pfnEntry )
    {
        char const *err_str = ::dlerror();
        ALOGE("dlsym: %s error=%s", szEntrySymbol, (err_str ? err_str : "unknown"));
        goto lbExit;
    }
    //
    enable = pfnEntry();

lbExit:

    if( gLib != NULL)
    {
        ::dlclose(gLib);
        gLib = NULL;
    }
#endif
    LOG1("MMSdkService::existCallbackClient: enable %d", enable);
/*
    {
        char prop[PROPERTY_VALUE_MAX];
        int en;
        ::property_get("camera.callback.enable", prop, "1");
        en = ::atoi(prop);
        // force to disable postProc
        if (en == 0)
        {
            enable = 0;
            LOG1("MMSdkService::existCallbackClient: force to disable! enable %d", enable);
        }
    }
*/
    LOG1("MMSdkService::existCallbackClient X");

    return (enable > 0) ? 1 : 0;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t MMSdkService::dump(
    int fd,
    const Vector<String16>& args
)
{
    (void)fd;
    (void)args;
    return NO_ERROR;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t MMSdkService::onTransact(
    uint32_t code,
    const Parcel& data,
    Parcel* reply,
    uint32_t flags)
{
    return BnMMSdkService::onTransact(code, data, reply, flags);
}

/******************************************************************************
 *
 ******************************************************************************/
/*virtual*/void MMSdkService::binderDied
(
    const wp<IBinder> &who
)
{
    (void)who;
    /**
      * While tempting to promote the wp<IBinder> into a sp,
      * it's actually not supported by the binder driver
      */
    ALOGD("java clients' binder died");
}


}; // namespace android
