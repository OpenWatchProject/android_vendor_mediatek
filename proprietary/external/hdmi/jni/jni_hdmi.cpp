/*
 * Copyright (C) 2012 The Android Open Source Project
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

#define LOG_TAG "JNI_HDMI"

#define MTK_LOG_ENABLE 1
#include <jni_hdmi.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <utils/Log.h>
#include <utils/threads.h>
#include <cutils/log.h>
#include <cutils/properties.h>

#include <vendor/mediatek/hardware/hdmi/1.0/IMtkHdmiService.h>

using android::hardware::hidl_array;
using vendor::mediatek::hardware::hdmi::V1_0::IMtkHdmiService;
using vendor::mediatek::hardware::hdmi::V1_0::Result;
using vendor::mediatek::hardware::hdmi::V1_0::EDID_t;

using namespace android;

#define EDIDNUM 4

#define UNUSED(expr) do { (void)(expr); } while (0)
#define JNI_HDMI_CLASS_NAME "com/mediatek/hdmi/HdmiNative"

Mutex mLock;

static jboolean enableHDCP(JNIEnv *env, jobject thiz, jboolean isEnable)
{
    sp<IMtkHdmiService> service = IMtkHdmiService::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_HDMI] failed to get HW service");
        return JNI_FALSE;
    }

    android::hardware::Return<Result> ret = service->enable_hdcp(isEnable);
    if (!ret.isOk() || ret != Result::SUCCEED) {
        ALOGE("[JNI_HDMI] IMtkHdmiService::enable_hdcp failed!");
        return JNI_FALSE;
    }

    UNUSED(env);
    UNUSED(thiz);

    return JNI_TRUE;
}

static jboolean enableHdmi(JNIEnv *env, jobject thiz, jboolean isEnable)
{
    sp<IMtkHdmiService> service = IMtkHdmiService::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_HDMI] failed to get HW service");
        return JNI_FALSE;
    }

    android::hardware::Return<Result> ret = service->enable_hdmi(isEnable);
    if (!ret.isOk() || ret != Result::SUCCEED) {
        ALOGE("[JNI_HDMI] IMtkHdmiService::enable_hdmi failed!");
        return JNI_FALSE;
    }

    UNUSED(env);
    UNUSED(thiz);

    return JNI_TRUE;
}


static jboolean setAutoMode(JNIEnv *env, jobject thiz, jboolean isEnable)
{
    sp<IMtkHdmiService> service = IMtkHdmiService::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_HDMI] failed to get HW service");
        return JNI_FALSE;
    }

    android::hardware::Return<Result> ret = service->set_auto_mode(isEnable);
    if (!ret.isOk() || ret != Result::SUCCEED) {
        ALOGE("[JNI_HDMI] IMtkHdmiService::enable_hdmi failed!");
        return JNI_FALSE;
    }

    UNUSED(env);
    UNUSED(thiz);

    return JNI_TRUE;
}

static jboolean enableHdmiHdr(JNIEnv *env, jobject thiz, jboolean isEnable)
{
    sp<IMtkHdmiService> service = IMtkHdmiService::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_HDMI] failed to get HW service");
        return JNI_FALSE;
    }

    android::hardware::Return<Result> ret = service->enable_hdmi_hdr(isEnable);
    if (!ret.isOk() || ret != Result::SUCCEED) {
        ALOGE("[JNI_HDMI] IMtkHdmiService::enable_hdmi_hdr failed!");
        return JNI_FALSE;
    }

    UNUSED(env);
    UNUSED(thiz);

    return JNI_TRUE;
}

static jboolean setColorFormat(JNIEnv *env, jobject thiz, jint colorFormat)
{
    sp<IMtkHdmiService> service = IMtkHdmiService::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_HDMI] failed to get HW service");
        return JNI_FALSE;
    }

    android::hardware::Return<Result> ret = service->set_color_format(colorFormat);
    if (!ret.isOk() || ret != Result::SUCCEED) {
        ALOGE("[JNI_HDMI] IMtkHdmiService::set_color_format failed!");
        return JNI_FALSE;
    }

    UNUSED(env);
    UNUSED(thiz);

    return JNI_TRUE;
}

static jboolean setColorDepth(JNIEnv *env, jobject thiz, jint colorDepth)
{
    sp<IMtkHdmiService> service = IMtkHdmiService::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_HDMI] failed to get HW service");
        return JNI_FALSE;
    }

    android::hardware::Return<Result> ret = service->set_color_depth(colorDepth);
    if (!ret.isOk() || ret != Result::SUCCEED) {
        ALOGE("[JNI_HDMI] IMtkHdmiService::enable_hdmi failed!");
        return JNI_FALSE;
    }

    UNUSED(env);
    UNUSED(thiz);

    return JNI_TRUE;
}


static jintArray getResolutionMask(JNIEnv *env, jobject thiz)
{
    int* hdmi_edid = new int[EDIDNUM];

    ALOGD("[JNI_HDMI] getResolutionMask()");

    sp<IMtkHdmiService> service = IMtkHdmiService::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_HDMI] failed to get HW service");
        return NULL;
    }

    android::hardware::Return<void> ret = service->get_resolution_mask(
        [&] (Result retval, EDID_t s_edid) {
        ALOGD("[JNI_HDMI] getResolutionMask() get edid");
        if (Result::SUCCEED == retval){
            hdmi_edid[0] = s_edid.edid[0];
            hdmi_edid[1] = s_edid.edid[1];
            hdmi_edid[2] = s_edid.edid[2];
            hdmi_edid[3] = s_edid.edid[3];
        }
    });
    if (!ret.isOk()) {
        ALOGE("[JNI_HDMI] IMtkHdmiService::get_resolution_mask failed!");
        return nullptr;
    }
    ALOGD("[JNI_HDMI] getResolutionMask() before return hdmi_edid = %x", hdmi_edid);

    jintArray j_hdmi_edid = env->NewIntArray(EDIDNUM);
    env->ReleaseIntArrayElements(j_hdmi_edid, hdmi_edid, 0);
    UNUSED(env);
    UNUSED(thiz);

    return j_hdmi_edid;
}

static jboolean setVideoResolution(JNIEnv *env, jobject thiz, jint resolution)
{
    sp<IMtkHdmiService> service = IMtkHdmiService::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_HDMI] failed to get HW service");
        return JNI_FALSE;
    }

    android::hardware::Return<Result> ret = service->set_video_resolution(resolution);
    if (!ret.isOk() || ret != Result::SUCCEED) {
        ALOGE("[JNI_HDMI] IMtkHdmiService::set_video_resolution failed!");
        return JNI_FALSE;
    }

    UNUSED(env);
    UNUSED(thiz);

    return JNI_TRUE;
}


/////////////////////////////////////////////////////////////////////////////////

//JNI register
////////////////////////////////////////////////////////////////
static const char *classPathName = JNI_HDMI_CLASS_NAME;

static JNINativeMethod g_methods[] = {

    {"nativeGetResolutionMask", "()[I", (void*)getResolutionMask},

    {"nativeSetVideoResolution", "(I)Z", (void*)setVideoResolution},

    {"nativeEnableHDCP", "(Z)Z", (void*)enableHDCP},
    {"nativeEnableHdmi", "(Z)Z", (void*)enableHdmi},
    {"nativeSetAutoMode", "(Z)Z", (void*)setAutoMode},
    {"nativeEnableHdmiHdr", "(Z)Z", (void*)enableHdmiHdr},
    {"nativeSetColorFormat", "(I)Z", (void*)setColorFormat},
    {"nativeSetColorDepth", "(I)Z", (void*)setColorDepth},
};

/*
 * Register several native methods for one class.
 */
static int registerNativeMethods(JNIEnv* env, const char* className,
    JNINativeMethod* gMethods, int numMethods)
{
    jclass clazz;

    clazz = env->FindClass(className);
    if (clazz == NULL) {
        ALOGE("Native registration unable to find class '%s'", className);
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        ALOGE("RegisterNatives failed for '%s'", className);
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

// ----------------------------------------------------------------------------

/*
 * This is called by the VM when the shared library is first loaded.
 */

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;
    jint result = -1;

    UNUSED(reserved);

    ALOGI("JNI_OnLoad");

    if (JNI_OK != vm->GetEnv((void **)&env, JNI_VERSION_1_4)) {
        ALOGE("ERROR: GetEnv failed");
        goto bail;
    }

    if (!registerNativeMethods(env, classPathName, g_methods, sizeof(g_methods) / sizeof(g_methods[0]))) {
        ALOGE("ERROR: registerNatives failed");
        goto bail;
    }

    result = JNI_VERSION_1_4;

bail:
    return result;
}
