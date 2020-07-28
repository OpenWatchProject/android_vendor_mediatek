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

#define LOG_TAG "advcam/AdvCamService"

#define MTK_LOG_ENABLE 1
//#define ADVCAM_DEV_TEST
#include <stdio.h>
#include <sys/types.h>

#include <dlfcn.h>
#include <binder/AppOpsManager.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <cutils/atomic.h>
#include <cutils/properties.h>
#include <utils/Errors.h>
#include <utils/Log.h>
#include <utils/String16.h>

#include <camera/camera2/CaptureRequest.h>

#include <vendor/mediatek/hardware/camera/advcam/1.0/IAdvCamControl.h>
#include <android/hardware/camera/device/3.2/types.h>
#include <android/hardware/camera/common/1.0/types.h>

using namespace android;
using vendor::mediatek::hardware::camera::advcam::V1_0::IAdvCamControl;
//
#include <advcam/AdvCamService.h>

using namespace android;
using namespace NSAdvCam;

namespace android {


// ----------------------------------------------------------------------------
// Logging support -- this is for debugging only
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
AdvCamService::AdvCamService()
{
    ALOGI("AdvCamService started (pid=%d)", getpid());
}

/******************************************************************************
 *
 ******************************************************************************/
void AdvCamService::onFirstRef()
{
    LOG1("AdvCamService::onFirstRef");
    BnAdvCamService::onFirstRef();
}

/******************************************************************************
 *
 ******************************************************************************/
AdvCamService::~AdvCamService()
{
    LOG1("AdvCamService::~AdvCamService()");
}


/******************************************************************************
 *
 ******************************************************************************/
status_t AdvCamService::setConfigureParam( uint32_t openId, CaptureRequest& request)
{
    LOG1("AdvCamService::setConfigureParam E");
    sp<IAdvCamControl> advCamControl = IAdvCamControl::tryGetService("internal/0");
    const camera_metadata_t* meta = request.mMetadata.getAndLock();

    if (meta != NULL && advCamControl != NULL){
        android::hardware::camera::device::V3_2::CameraMetadata hidlMeta;
        hidlMeta.setToExternal(reinterpret_cast<uint8_t*>(const_cast<camera_metadata_t*>(meta)),
                     get_camera_metadata_size(meta));
        advCamControl->setConfigureParam(openId, hidlMeta);
    }else{
         ALOGE("ERROR : Config Metadata NULL(%d), advCamControl NULL(%d)", (meta == NULL), (advCamControl == NULL));
#ifdef ADVCAM_DEV_TEST
        android::hardware::camera::device::V3_2::CameraMetadata hidlMeta;
        if(advCamControl != NULL)
            advCamControl->setConfigureParam(openId, hidlMeta);
#endif
    }

    if(meta != NULL)
        request.mMetadata.unlock(meta);


    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t AdvCamService::dump(
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
status_t AdvCamService::onTransact(
    uint32_t code,
    const Parcel& data,
    Parcel* reply,
    uint32_t flags)
{
    return BnAdvCamService::onTransact(code, data, reply, flags);
}

/******************************************************************************
 *
 ******************************************************************************/
/*virtual*/void AdvCamService::binderDied
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
