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

#define LOG_TAG "JNI_PQ"

#define MTK_LOG_ENABLE 1
#include <jni_pq.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <utils/Log.h>
#include <utils/threads.h>
#include <cutils/log.h>
#include <cutils/properties.h>

#include "cust_color.h"
#include "cust_tdshp.h"
#include "cust_gamma.h"
#include <PQServiceCommon.h>
#include <vendor/mediatek/hardware/pq/2.3/IPictureQuality.h>

using android::hardware::hidl_array;
using vendor::mediatek::hardware::pq::V2_3::IPictureQuality;
using vendor::mediatek::hardware::pq::V2_0::PQFeatureID;
using vendor::mediatek::hardware::pq::V2_0::Result;

#ifdef MTK_SYSTEM_AAL
#include "AALClient.h"
#endif

#define AAL_FUNCTION_DEFAULT                 "2"     // default is ESS only

#ifdef MTK_MIRAVISION_IMAGE_DC_SUPPORT
#include <PQDCHistogram.h>
#endif

using namespace android;

#define MIN_COLOR_WIN_SIZE (0x0)
#define MAX_COLOR_WIN_SIZE (0xFFFF)

#define UNUSED(expr) do { (void)(expr); } while (0)
#define JNI_PQ_CLASS_NAME "com/mediatek/pq/PictureQuality"

Mutex mLock;

static int CAPABILITY_MASK_COLOR       = 0x00000001;
static int CAPABILITY_MASK_SHARPNESS   = 0x00000002;
static int CAPABILITY_MASK_GAMMA       = 0x00000004;
static int CAPABILITY_MASK_DC          = 0x00000008;
#ifdef MTK_OD_SUPPORT
static int CAPABILITY_MASK_OD          = 0x00000010;
#endif
/////////////////////////////////////////////////////////////////////////////////
static int gAalSupport = -1;
static int gPQSupport = -1;

bool AalIsSupport(void)
{
    char value[PROP_VALUE_MAX];

    if (gAalSupport < 0) {
        if (__system_property_get("ro.vendor.mtk_aal_support", value) > 0) {
            gAalSupport = (int)strtoul(value, NULL, 0);
        }
    }

    return (gAalSupport > 0) ? true : false;
}

bool PQIsSupport(void)
{
    char value[PROP_VALUE_MAX];

    if (gPQSupport < 0) {
        if (__system_property_get("ro.vendor.mtk_pq_support", value) > 0) {
            gPQSupport = (int)strtoul(value, NULL, 0);
        }
    }

    return (gPQSupport > 0) ? true : false;
}


static jint getCapability(JNIEnv *env, jobject thiz)
{
    Mutex::Autolock autoLock(mLock);

    UNUSED(env);
    UNUSED(thiz);

    int cap = (CAPABILITY_MASK_COLOR |
               CAPABILITY_MASK_SHARPNESS |
               CAPABILITY_MASK_GAMMA |
               CAPABILITY_MASK_DC);

#ifdef MTK_OD_SUPPORT
    cap |= CAPABILITY_MASK_OD;
#endif

    return cap;
}

static void setCameraPreviewMode(JNIEnv* env, jobject thiz, jint step)
{
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] setCameraPreviewMode(): MTK_PQ_SUPPORT disabled");
        return;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return;
    }

    android::hardware::Return<Result> ret = service->setDISPScenario(SCENARIO_ISP_PREVIEW, step);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setDISPScenario failed!");
    }

    UNUSED(env);
    UNUSED(thiz);

    return;
}

static void setGalleryNormalMode(JNIEnv* env, jobject thiz, jint step)
{
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] setGalleryNormalMode(): MTK_PQ_SUPPORT disabled");
        return;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return;
    }

    android::hardware::Return<Result> ret = service->setDISPScenario(SCENARIO_PICTURE, step);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setDISPScenario failed!");
    }

    UNUSED(env);
    UNUSED(thiz);

    return;
}

static void setVideoPlaybackMode(JNIEnv* env, jobject thiz, jint step)
{
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] setVideoPlaybackMode(): MTK_PQ_SUPPORT disabled");
        return ;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return;
    }

    android::hardware::Return<Result> ret = service->setDISPScenario(SCENARIO_VIDEO, step);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setDISPScenario failed!");
    }

    UNUSED(env);
    UNUSED(thiz);

    return;
}

#ifdef MTK_MIRAVISION_IMAGE_DC_SUPPORT
static void Hist_set(JNIEnv* env, jobject obj, jint index, jint value)
{
    jclass clazz = env->FindClass(JNI_PQ_CLASS_NAME "$Hist");
    jmethodID setMethod = env->GetMethodID(clazz, "set", "(II)V");
    env->CallVoidMethod(obj, setMethod, index, value);

    env->DeleteLocalRef(clazz);
}
#endif

static void getDynamicContrastHistogram(JNIEnv* env, jclass clz, jbyteArray srcBuffer, jint srcWidth, jint srcHeight, jobject hist)
{
#ifdef MTK_MIRAVISION_IMAGE_DC_SUPPORT
    Mutex::Autolock autoLock(mLock);
    CPQDCHistogram *pDCHist = new CPQDCHistogram;
    DynCInput   input;
    DynCOutput  output;
    int i;

    input.pSrcFB = (unsigned char*)env->GetByteArrayElements(srcBuffer, 0);
    input.iWidth = srcWidth;
    input.iHeight = srcHeight;

    pDCHist->Main(input, &output);
    for (i = 0; i < DCHIST_INFO_NUM; i++)
    {
        Hist_set(env, hist, i, output.Info[i]);
    }

    env->ReleaseByteArrayElements(srcBuffer, (jbyte*)input.pSrcFB, 0);
    delete pDCHist;
#else
    ALOGE("[JNI_PQ] getDynamicContrastHistogram(), not supported!");

    UNUSED(env);
    UNUSED(srcBuffer);
    UNUSED(srcWidth);
    UNUSED(srcHeight);
    UNUSED(hist);
#endif
    UNUSED(clz);
}

static void Range_set(JNIEnv* env, jobject obj, jint min, jint max, jint defaultValue)
{
    jclass clazz = env->FindClass(JNI_PQ_CLASS_NAME "$Range");
    jmethodID setMethod = env->GetMethodID(clazz, "set", "(III)V");
    env->CallVoidMethod(obj, setMethod, min, max, defaultValue);
}

static jboolean enableColor(JNIEnv *env, jobject thiz, int isEnable);
static jboolean enableSharpness(JNIEnv *env, jobject thiz, int isEnable);
static jboolean enableDynamicContrast(JNIEnv *env, jobject thiz, int isEnable);
static jboolean enableGamma(JNIEnv *env, jobject thiz, int isEnable);
static jboolean enableOD(JNIEnv *env, jobject thiz, int isEnable);

/////////////////////////////////////////////////////////////////////////////////
static jboolean enablePQ(JNIEnv *env, jobject thiz, int isEnable)
{
    if (isEnable)
    {
        enableColor(env, thiz, 1);
        enableSharpness(env, thiz, 1);
        enableDynamicContrast(env, thiz, 1);
        enableOD(env, thiz, 1);
    }
    else
    {
        enableColor(env, thiz, 0);
        enableSharpness(env, thiz, 0);
        enableDynamicContrast(env, thiz, 0);
        enableOD(env, thiz, 0);
    }

    UNUSED(env);
    UNUSED(thiz);

    return JNI_TRUE;
}

/////////////////////////////////////////////////////////////////////////////////
static jboolean enableColor(JNIEnv *env, jobject thiz, int isEnable)
{
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] enableColor(): MTK_PQ_SUPPORT disabled");
        return JNI_FALSE;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return JNI_FALSE;
    }

    android::hardware::Return<Result> ret = service->setFeatureSwitch(PQFeatureID::DISPLAY_COLOR, isEnable);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setFeatureSwitch failed!");
        return JNI_FALSE;
    }

    UNUSED(env);
    UNUSED(thiz);

    return JNI_TRUE;
}

/////////////////////////////////////////////////////////////////////////////////
static jboolean enableSharpness(JNIEnv *env, jobject thiz, int isEnable)
{
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] enableSharpness(): MTK_PQ_SUPPORT disabled");
        return JNI_FALSE;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return JNI_FALSE;
    }

    android::hardware::Return<Result> ret = service->setFeatureSwitch(PQFeatureID::SHARPNESS, isEnable);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setFeatureSwitch failed!");
        return JNI_FALSE;
    }

    UNUSED(env);
    UNUSED(thiz);

    return JNI_TRUE;
}

/////////////////////////////////////////////////////////////////////////////////
static jboolean enableDynamicContrast(JNIEnv *env, jobject thiz, int isEnable)
{
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] enableDynamicContrast(): MTK_PQ_SUPPORT disabled");
        return JNI_FALSE;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return JNI_FALSE;
    }

    android::hardware::Return<Result> ret = service->setFeatureSwitch(PQFeatureID::DYNAMIC_CONTRAST, isEnable);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setFeatureSwitch failed!");
        return JNI_FALSE;
    }

    UNUSED(env);
    UNUSED(thiz);

    return JNI_TRUE;
}

static jboolean enableColorEffect(JNIEnv *env, jobject thiz, int isEnable)
{
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] enableColorEffect(): MTK_PQ_SUPPORT disabled");
        return JNI_FALSE;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return JNI_FALSE;
    }

    android::hardware::Return<Result> ret = service->setFeatureSwitch(PQFeatureID::CONTENT_COLOR_VIDEO, isEnable);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setFeatureSwitch failed!");
        return JNI_FALSE;
    }

    UNUSED(env);
    UNUSED(thiz);

    return JNI_TRUE;
}

static jboolean enableDynamicSharpness(JNIEnv *env, jobject thiz, int isEnable)
{
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] enableDynamicSharpness(): MTK_PQ_SUPPORT disabled");
        return JNI_FALSE;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return JNI_FALSE;
    }

    android::hardware::Return<Result> ret = service->setFeatureSwitch(PQFeatureID::DYNAMIC_SHARPNESS, isEnable);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setFeatureSwitch failed!");
        return JNI_FALSE;
    }

    UNUSED(env);
    UNUSED(thiz);

    return JNI_TRUE;
}

static jboolean enableUltraResolution(JNIEnv *env, jobject thiz, int isEnable)
{
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] enableUltraResolution(): MTK_PQ_SUPPORT disabled");
        return JNI_FALSE;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return JNI_FALSE;
    }

    android::hardware::Return<Result> ret = service->setFeatureSwitch(PQFeatureID::ULTRA_RESOLUTION, isEnable);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setFeatureSwitch failed!");
        return JNI_FALSE;
    }

    UNUSED(env);
    UNUSED(thiz);

    return JNI_TRUE;
}

static jboolean enableISOAdaptiveSharpness(JNIEnv *env, jobject thiz, int isEnable)
{
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] enableISOAdaptiveSharpness(): MTK_PQ_SUPPORT disabled");
        return JNI_FALSE;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return JNI_FALSE;
    }

    android::hardware::Return<Result> ret = service->setFeatureSwitch(PQFeatureID::ISO_ADAPTIVE_SHARPNESS, isEnable);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setFeatureSwitch failed!");
        return JNI_FALSE;
    }

    UNUSED(env);
    UNUSED(thiz);

    return JNI_TRUE;
}

static jboolean enableContentColor(JNIEnv *env, jobject thiz, int isEnable)
{
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] enableContentColor(): MTK_PQ_SUPPORT disabled");
        return JNI_FALSE;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return JNI_FALSE;
    }

    android::hardware::Return<Result> ret = service->setFeatureSwitch(PQFeatureID::CONTENT_COLOR, isEnable);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setFeatureSwitch failed!");
        return JNI_FALSE;
    }

    UNUSED(env);
    UNUSED(thiz);

    return JNI_TRUE;
}

/////////////////////////////////////////////////////////////////////////////////
static jboolean enableGamma(JNIEnv *env, jobject thiz, int isEnable)
{
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] enableGamma(): MTK_PQ_SUPPORT disabled");
        return JNI_FALSE;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return JNI_FALSE;
    }

    android::hardware::Return<Result> ret = service->setFeatureSwitch(PQFeatureID::DISPLAY_GAMMA, isEnable);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setFeatureSwitch failed!");
        return JNI_FALSE;
    }

    UNUSED(env);
    UNUSED(thiz);

    return JNI_TRUE;
}

/////////////////////////////////////////////////////////////////////////////////
static jint getPictureMode(JNIEnv *env, jobject thiz)
{
    Mutex::Autolock autoLock(mLock);
    char value[PROPERTY_VALUE_MAX];
    int mode = -1;

    property_get(PQ_PIC_MODE_PROPERTY_STR, value, PQ_PIC_MODE_DEFAULT);
    mode = atoi(value);
    ALOGD("[JNI_PQ] getPictureMode(), property get [%d]", mode);

    UNUSED(env);
    UNUSED(thiz);

    return mode;
}

/////////////////////////////////////////////////////////////////////////////////

static jboolean setPictureMode(JNIEnv *env, jobject thiz, int mode, int step)
{
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] setPictureMode(): MTK_PQ_SUPPORT disabled");
        return JNI_FALSE;
    }

    Mutex::Autolock autoLock(mLock);
    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return JNI_FALSE;
    }

    android::hardware::Return<Result> ret = service->setPQMode(mode, step);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setPQMode failed!");
        return JNI_FALSE;
    }

    UNUSED(env);
    UNUSED(thiz);

    return JNI_TRUE;
}

/////////////////////////////////////////////////////////////////////////////////

static jboolean setColorRegion(JNIEnv *env, jobject thiz, int isEnable, int startX, int startY, int endX, int endY)
{
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] setColorRegion(): MTK_PQ_SUPPORT disabled");
        return JNI_FALSE;
    }

    DISP_PQ_WIN_PARAM win_param;
    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return JNI_FALSE;
    }

    ALOGD("[JNI_PQ] setColorRegion(), en[%d], sX[%d], sY[%d], eX[%d], eY[%d]", isEnable, startX, startY, endX, endY);

    if (isEnable)
    {
        win_param.split_en = 1;
        win_param.start_x = startX;
        win_param.start_y = startY;
        win_param.end_x = endX;
        win_param.end_y = endY;
    }
    else
    {
        win_param.split_en = 0;
        win_param.start_x = MIN_COLOR_WIN_SIZE;
        win_param.start_y = MIN_COLOR_WIN_SIZE;
        win_param.end_x = MAX_COLOR_WIN_SIZE;
        win_param.end_y = MAX_COLOR_WIN_SIZE;
    }
    android::hardware::Return<Result> ret = service->setColorRegion(win_param.split_en,win_param.start_x,win_param.start_y,win_param.end_x,win_param.end_y);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setColorRegion failed!");
        return JNI_FALSE;
    }

    UNUSED(env);
    UNUSED(thiz);

    return JNI_TRUE;
}

/////////////////////////////////////////////////////////////////////////////////
static void getContrastIndexRange(JNIEnv* env, jclass clz, jobject range)
{
    Mutex::Autolock autoLock(mLock);

    Range_set(env, range, 0, PQ_CONTRAST_INDEX_RANGE_NUM - 1, atoi(PQ_CONTRAST_INDEX_DEFAULT));

    UNUSED(clz);
}

static jint getContrastIndex(JNIEnv *env, jobject thiz)
{
    Mutex::Autolock autoLock(mLock);
    char value[PROPERTY_VALUE_MAX];
    int index = -1;

    property_get(PQ_CONTRAST_PROPERTY_STR, value, PQ_CONTRAST_INDEX_DEFAULT);
    index = atoi(value);
    ALOGD("[JNI_PQ] getContrastIndex(), property get [%d]", index);

    UNUSED(env);
    UNUSED(thiz);

    return index;
}

static void setContrastIndex(JNIEnv *env, jobject thiz, int index, int step)
{
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] setContrastIndex(): MTK_PQ_SUPPORT disabled");
        return;
    }

    Mutex::Autolock autoLock(mLock);
    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return;
    }

    android::hardware::Return<Result> ret_hidl = service->setPQIndex(index, SCENARIO_PICTURE, TDSHP_FLAG_NORMAL, SET_PQ_CONTRAST, step);
    if (!ret_hidl.isOk() || ret_hidl != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setPQIndex failed!");
        return;
    }

    UNUSED(env);
    UNUSED(thiz);

}

/////////////////////////////////////////////////////////////////////////////////
static void getSaturationIndexRange(JNIEnv* env, jclass clz, jobject range)
{
    Mutex::Autolock autoLock(mLock);

    Range_set(env, range, 0, PQ_GSAT_INDEX_RANGE_NUM - 1, atoi(PQ_GSAT_INDEX_DEFAULT));

    UNUSED(clz);
}

static jint getSaturationIndex(JNIEnv *env, jobject thiz)
{
    Mutex::Autolock autoLock(mLock);
    char value[PROPERTY_VALUE_MAX];
    int index = -1;

    property_get(PQ_GSAT_PROPERTY_STR, value, PQ_GSAT_INDEX_DEFAULT);
    index = atoi(value);
    ALOGD("[JNI_PQ] getSaturationIndex(), property get [%d]", index);

    UNUSED(env);
    UNUSED(thiz);

    return index;
}

static void setSaturationIndex(JNIEnv *env, jobject thiz, int index, int step)
{
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] setSaturationIndex(): MTK_PQ_SUPPORT disabled");
        return;
    }

    Mutex::Autolock autoLock(mLock);
    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return;
    }

    ALOGD("[JNI_PQ] setSaturationIndex...index[%d]", index);

    android::hardware::Return<Result> ret_hidl = service->setPQIndex(index, SCENARIO_PICTURE, TDSHP_FLAG_NORMAL, SET_PQ_SAT_GAIN, step);
    if (!ret_hidl.isOk() || ret_hidl != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setPQIndex failed!");
        return;
    }

    UNUSED(env);
    UNUSED(thiz);
}

/////////////////////////////////////////////////////////////////////////////////
static void getPicBrightnessIndexRange(JNIEnv* env, jclass clz, jobject range)
{
    Mutex::Autolock autoLock(mLock);

    Range_set(env, range, 0, PQ_PIC_BRIGHT_INDEX_RANGE_NUM - 1, atoi(PQ_PIC_BRIGHT_INDEX_DEFAULT));

    UNUSED(clz);
}

static jint getPicBrightnessIndex(JNIEnv *env, jobject thiz)
{
    Mutex::Autolock autoLock(mLock);
    char value[PROPERTY_VALUE_MAX];
    int index = -1;

    property_get(PQ_PIC_BRIGHT_PROPERTY_STR, value, PQ_PIC_BRIGHT_INDEX_DEFAULT);
    index = atoi(value);
    ALOGD("[JNI_PQ] getPicBrightnessIndex(), property get [%d]", index);

    UNUSED(env);
    UNUSED(thiz);

    return index;
}

static void setPicBrightnessIndex(JNIEnv *env, jobject thiz, int index, int step)
{
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] setPicBrightnessIndex(): MTK_PQ_SUPPORT disabled");
        return;
    }

    Mutex::Autolock autoLock(mLock);
    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return;
    }

    ALOGD("[JNI_PQ] setPicBrightnessIndex...index[%d]", index);

    android::hardware::Return<Result> ret_hidl = service->setPQIndex(index, SCENARIO_PICTURE, TDSHP_FLAG_NORMAL, SET_PQ_BRIGHTNESS, step);
    if (!ret_hidl.isOk() || ret_hidl != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setPQIndex failed!");
        return;
    }

    UNUSED(env);
    UNUSED(thiz);

}

/////////////////////////////////////////////////////////////////////////////////
static void getSharpnessIndexRange(JNIEnv* env, jclass clz, jobject range)
{
    Mutex::Autolock autoLock(mLock);

    Range_set(env, range, 0, PQ_TDSHP_INDEX_RANGE_NUM - 1, atoi(PQ_TDSHP_INDEX_DEFAULT));

    UNUSED(clz);
}

static jint getSharpnessIndex(JNIEnv *env, jobject thiz)
{
    Mutex::Autolock autoLock(mLock);
    char value[PROPERTY_VALUE_MAX];
    int index = -1;

    property_get(PQ_TDSHP_PROPERTY_STR, value, PQ_TDSHP_STANDARD_DEFAULT);
    index = atoi(value);
    ALOGD("[JNI_PQ] getSharpnessIndex(), property get [%d]", index);

    UNUSED(env);
    UNUSED(thiz);

    return index;
}

static void setSharpnessIndex(JNIEnv *env, jobject thiz , int index)
{
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] setSharpnessIndex(): MTK_PQ_SUPPORT disabled");
        return;
    }

    Mutex::Autolock autoLock(mLock);
    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return;
    }

    ALOGD("[JNI_PQ] setSharpnessIndex...index[%d]", index);

    android::hardware::Return<Result> ret_hidl = service->setPQIndex(index, SCENARIO_PICTURE, TDSHP_FLAG_NORMAL, SET_PQ_SHP_GAIN, PQ_DEFAULT_TRANSITION_OFF_STEP);
    if (!ret_hidl.isOk() || ret_hidl != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setPQIndex failed!");
        return;
    }

    UNUSED(env);
    UNUSED(thiz);
}

/////////////////////////////////////////////////////////////////////////////////
static void getDynamicContrastIndexRange(JNIEnv* env, jclass clz, jobject range)
{
    Mutex::Autolock autoLock(mLock);

    Range_set(env, range, 0, PQ_ADL_INDEX_RANGE_NUM, atoi(PQ_ADL_INDEX_DEFAULT));

    UNUSED(clz);
}

static jint getDynamicContrastIndex(JNIEnv *env, jobject thiz)
{
    Mutex::Autolock autoLock(mLock);
    char value[PROPERTY_VALUE_MAX];
    int index = -1;

    property_get(PQ_ADL_PROPERTY_STR, value, PQ_ADL_INDEX_DEFAULT);
    index = atoi(value);
    ALOGD("[JNI_PQ] getDynamicContrastIndex(), property get [%d]", index);

    UNUSED(env);
    UNUSED(thiz);

    return index;
}

static void setDynamicContrastIndex(JNIEnv *env, jobject thiz, int index)
{
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] setDynamicContrastIndex(): MTK_PQ_SUPPORT disabled");
        return;
    }

    Mutex::Autolock autoLock(mLock);
    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return;
    }

    android::hardware::Return<Result> ret = service->setFeatureSwitch(PQFeatureID::DYNAMIC_CONTRAST, index);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setFeatureSwitch failed!");
    }

    UNUSED(env);
    UNUSED(thiz);
}

static void getColorEffectIndexRange(JNIEnv* env, jclass clz, jobject range)
{
    Mutex::Autolock autoLock(mLock);

    Range_set(env, range, 0, PQ_MDP_COLOR_EN_INDEX_RANGE_NUM, atoi(PQ_MDP_COLOR_EN_DEFAULT));

    UNUSED(clz);
}

static jint getColorEffectIndex(JNIEnv *env, jobject thiz)
{
    Mutex::Autolock autoLock(mLock);
    char value[PROPERTY_VALUE_MAX];
    int index = -1;

    property_get(PQ_MDP_COLOR_EN_STR, value, PQ_MDP_COLOR_EN_DEFAULT);
    index = atoi(value);
    ALOGD("[JNI_PQ] getColorEffectIndex(), property get [%d]", index);

    UNUSED(env);
    UNUSED(thiz);

    return index;
}

static void setColorEffectIndex(JNIEnv *env, jobject thiz, int index)
{
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] setColorEffectIndex(): MTK_PQ_SUPPORT disabled");
        return;
    }

    Mutex::Autolock autoLock(mLock);
    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return;
    }

    android::hardware::Return<Result> ret = service->setFeatureSwitch(PQFeatureID::CONTENT_COLOR_VIDEO, index);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setFeatureSwitch failed!");
    }

    UNUSED(env);
    UNUSED(thiz);
}
static void getGammaIndexRange(JNIEnv* env, jclass clz, jobject range)
{
    Mutex::Autolock autoLock(mLock);

    Range_set(env, range, 0, GAMMA_INDEX_MAX - 1, GAMMA_INDEX_DEFAULT);

    UNUSED(clz);
}


static void setGammaIndex(JNIEnv* env, jclass clz, jint index, jint step)
{
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] setGammaIndex(): MTK_PQ_SUPPORT disabled");
        return;
    }

    Mutex::Autolock autoLock(mLock);
    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return;
    }

    android::hardware::Return<Result> ret = service->setGammaIndex(index, step);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setGammaIndex failed!");
    }

    UNUSED(env);
    UNUSED(clz);
}

// OD
static jboolean enableOD(JNIEnv *env, jobject thiz, int isEnable)
{
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] enableOD(): MTK_PQ_SUPPORT disabled");
        return JNI_FALSE;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return JNI_FALSE;
    }

    android::hardware::Return<Result> ret = service->setFeatureSwitch(PQFeatureID::DISPLAY_OVER_DRIVE, isEnable);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setFeatureSwitch failed!");
        return JNI_FALSE;
    }

    UNUSED(env);
    UNUSED(thiz);

    return JNI_TRUE;
}

static jboolean nativeEnableVideoHDR(JNIEnv* env, jclass clz, jboolean isEnable)
{
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] nativeEnableVideoHDR(): MTK_PQ_SUPPORT disabled");
        return JNI_FALSE;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return JNI_FALSE;
    }

    android::hardware::Return<Result> ret = service->setFeatureSwitch(PQFeatureID::VIDEO_HDR, isEnable);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setFeatureSwitch failed!");
        return JNI_FALSE;
    }

    UNUSED(env);
    UNUSED(clz);

    return JNI_TRUE;
}


static jboolean nativeIsVideoHDREnabled(JNIEnv* env, jclass clz)
{
    bool isEnabled = false;

    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] nativeIsVideoHDREnabled(): MTK_PQ_SUPPORT disabled");
        return JNI_FALSE;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return JNI_FALSE;
    }

    android::hardware::Return<void> ret = service->getFeatureSwitch(PQFeatureID::VIDEO_HDR,
        [&] (Result retval, uint32_t value) {
        if (retval == Result::OK) {
            isEnabled = value;
        } else {
            ALOGE("[JNI_PQ] nativeIsVideoHDREnabled() failed!");
        }
    });
    if (!ret.isOk()) {
        ALOGE("[JNI_PQ] IPictureQuality::getFeatureSwitch failed!");
        return JNI_FALSE;
    }

    UNUSED(env);
    UNUSED(clz);

    return (isEnabled ? JNI_TRUE : JNI_FALSE);
}

static jboolean nativeEnableMdpDRE(JNIEnv *env, jobject thiz, jint enable)
{
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] nativeEnableMdpDRE(): MTK_PQ_SUPPORT disabled");
        return JNI_FALSE;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return JNI_FALSE;
    }

    android::hardware::Return<Result> ret = service->setFeatureSwitch(
        (vendor::mediatek::hardware::pq::V2_0::PQFeatureID)PQFeatureID_DRE, enable);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setFeatureSwitch failed!");
        return JNI_FALSE;
    }

    UNUSED(env);
    UNUSED(thiz);

    return JNI_TRUE;
}

static jboolean nativeEnableMdpCCORR(JNIEnv *env, jobject thiz, jint enable)
{
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] nativeEnableMdpCCORR(): MTK_PQ_SUPPORT disabled");
        return JNI_FALSE;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return JNI_FALSE;
    }

    android::hardware::Return<Result> ret = service->setFeatureSwitch(
        (vendor::mediatek::hardware::pq::V2_0::PQFeatureID)PQFeatureID_CCORR, enable);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setFeatureSwitch failed!");
        return JNI_FALSE;
    }

    UNUSED(env);
    UNUSED(thiz);

    return JNI_TRUE;
}

static jboolean nativeSetBlueLightStrength(JNIEnv* env, jclass clz, jint strength, jint step)
{
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] nativeSetBlueLightStrength(): MTK_PQ_SUPPORT disabled");
        return JNI_FALSE;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return JNI_FALSE;
    }
    android::hardware::Return<Result> ret = service->setBlueLightStrength(strength, step);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setBlueLightStrength failed!");
        return JNI_FALSE;
    }

    UNUSED(env);
    UNUSED(clz);

    return JNI_TRUE;
}


static jint nativeGetBlueLightStrength(JNIEnv* env, jclass clz)
{
    int32_t getStrength = 0;

    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] nativeGetBlueLightStrength(): MTK_PQ_SUPPORT disabled");
        return JNI_FALSE;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return 0;
    }

    android::hardware::Return<void> ret = service->getBlueLightStrength(
        [&] (Result retval, int32_t strength) {
        if (retval == Result::OK) {
            getStrength = strength;
        } else {
            ALOGE("[JNI_PQ] nativeGetBlueLightStrength() failed!");
        }
    });
    if (!ret.isOk()) {
        ALOGE("[JNI_PQ] IPictureQuality::getBlueLightStrength failed!");
        return 0;
    }

    UNUSED(env);
    UNUSED(clz);

    return getStrength;
}


static jboolean nativeEnableBlueLight(JNIEnv* env, jclass clz, jboolean isEnable, jint step)
{
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] nativeEnableBlueLight(): MTK_PQ_SUPPORT disabled");
        return JNI_FALSE;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return JNI_FALSE;
    }

    android::hardware::Return<Result> ret = service->enableBlueLight(isEnable ? true : false, step);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::enableBlueLight failed!");
        return JNI_FALSE;
    }

    UNUSED(env);
    UNUSED(clz);

    return JNI_TRUE;
}


static jboolean nativeIsBlueLightEnabled(JNIEnv* env, jclass clz)
{
    bool getEnabled = false;

    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] nativeIsBlueLightEnabled(): MTK_PQ_SUPPORT disabled");
        return JNI_FALSE;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return JNI_FALSE;
    }

    android::hardware::Return<void> ret = service->getBlueLightEnabled(
        [&] (Result retval, bool isEnabled) {
        if (retval == Result::OK) {
            getEnabled = isEnabled;
        } else {
            ALOGE("[JNI_PQ] nativeIsBlueLightEnabled() failed!");
        }
    });
    if (!ret.isOk()) {
        ALOGE("[JNI_PQ] IPictureQuality::getBlueLightEnabled failed!");
        return JNI_FALSE;
    }

    UNUSED(env);
    UNUSED(clz);

    return (getEnabled ? JNI_TRUE : JNI_FALSE);
}

static jboolean nativeSetChameleonStrength(JNIEnv* env, jclass clz, jint strength, jint step)
{
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] nativeSetChameleonStrength(): MTK_PQ_SUPPORT disabled");
        return JNI_FALSE;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();
    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return JNI_FALSE;
    }

    android::hardware::Return<Result> ret = service->setChameleonStrength(strength, step);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setChameleonStrength failed!");
        return JNI_FALSE;
    }

    return JNI_TRUE;

    UNUSED(env);
    UNUSED(clz);
}

static jint nativeGetChameleonStrength(JNIEnv* env, jclass clz)
{
    int32_t getStrength = 0;

    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] nativeGetChameleonStrength(): MTK_PQ_SUPPORT disabled");
        return JNI_FALSE;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();
    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return 0;
    }

    android::hardware::Return<void> ret = service->getChameleonStrength(
        [&] (Result retval, int32_t strength) {
        if (retval == Result::OK) {
            getStrength = strength;
        } else {
            ALOGE("[JNI_PQ] nativeGetChameleonStrength() failed!");
        }
    });
    if (!ret.isOk()) {
        ALOGE("[JNI_PQ] IPictureQuality::getChameleonStrength failed!");
        return 0;
    }

    return getStrength;

    UNUSED(env);
    UNUSED(clz);
}


static jboolean nativeEnableChameleon(JNIEnv* env, jclass clz, jboolean isEnable, jint step)
{
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] nativeEnableChameleon(): MTK_PQ_SUPPORT disabled");
        return JNI_FALSE;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();
    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return JNI_FALSE;
    }

    android::hardware::Return<Result> ret = service->enableChameleon(isEnable ? true : false, step);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::enableChameleon failed!");
        return JNI_FALSE;
    }

    return JNI_TRUE;

    UNUSED(env);
    UNUSED(clz);
}


static jboolean nativeIsChameleonEnabled(JNIEnv* env, jclass clz)
{
    bool getEnabled = false;

    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] nativeIsChameleonEnabled(): MTK_PQ_SUPPORT disabled");
        return JNI_FALSE;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();
    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return JNI_FALSE;
    }

    android::hardware::Return<void> ret = service->getChameleonEnabled(
        [&] (Result retval, bool isEnabled) {
        if (retval == Result::OK) {
            getEnabled = isEnabled;
        } else {
            ALOGE("[JNI_PQ] nativeIsChameleonEnabled() failed!");
        }
    });
    if (!ret.isOk()) {
        ALOGE("[JNI_PQ] IPictureQuality::getChameleonEnabled failed!");
        return JNI_FALSE;
    }

    return (getEnabled ? JNI_TRUE : JNI_FALSE);

    UNUSED(env);
    UNUSED(clz);
}

static jint nativeGetDefaultOffTransitionStep(JNIEnv* env, jclass clz)
{
    UNUSED(env);
    UNUSED(clz);

    return PQ_DEFAULT_TRANSITION_OFF_STEP;
}

static jint nativeGetDefaultOnTransitionStep(JNIEnv* env, jclass clz)
{
    UNUSED(env);
    UNUSED(clz);

    return PQ_DEFAULT_TRANSITION_ON_STEP;
}

static jboolean nativeSetGlobalPQSwitch(JNIEnv* env, jclass clz, jint globalPQSwitch)
{
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] nativeSetGlobalPQSwitch(): MTK_PQ_SUPPORT disabled");
        return JNI_FALSE;
    }

    UNUSED(env);
    UNUSED(clz);

#if defined(MTK_GLOBAL_PQ_SUPPORT)

    Mutex::Autolock autoLock(mLock);
    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return JNI_FALSE;
    }

    android::hardware::Return<Result> ret = service->setGlobalPQSwitch(globalPQSwitch);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setGlobalPQSwitch failed!");
        return JNI_FALSE;
    }

    return JNI_TRUE;

#else

    UNUSED(globalPQSwitch);
    ALOGE("[JNI_PQ] nativeSetGlobalPQSwitch() not supported.");
    return JNI_FALSE;

#endif
}


static jint nativeGetGlobalPQSwitch(JNIEnv* env, jclass clz)
{
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] nativeGetGlobalPQSwitch(): MTK_PQ_SUPPORT disabled");
        return JNI_FALSE;
    }

    UNUSED(env);
    UNUSED(clz);

#if defined(MTK_GLOBAL_PQ_SUPPORT)

    Mutex::Autolock autoLock(mLock);
    int32_t globalPQSwitch;
    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return JNI_FALSE;
    }

    android::hardware::Return<void> ret = service->getGlobalPQSwitch(
        [&] (Result retval, int32_t switch_value) {
        if (retval == Result::OK) {
            globalPQSwitch = switch_value;
        } else {
            ALOGE("[JNI_PQ] nativeGetGlobalPQSwitch() failed!");
        }
    });
    if (!ret.isOk()) {
        ALOGE("[JNI_PQ] IPictureQuality::getGlobalPQSwitch failed!");
        return JNI_FALSE;
    }

    ALOGD("[JNI_PQ] globalPQSwitch = %d\n", globalPQSwitch);

    return globalPQSwitch;

#else

    ALOGE("[JNI_PQ] nativeGetGlobalPQSwitch() not supported.");
    return 0;

#endif
}

static jboolean nativeSetGlobalPQStrength(JNIEnv* env, jclass clz, jint globalPQStrength)
{
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] nativeSetGlobalPQStrength(): MTK_PQ_SUPPORT disabled");
        return JNI_FALSE;
    }

    UNUSED(env);
    UNUSED(clz);

#if defined(MTK_GLOBAL_PQ_SUPPORT)

    Mutex::Autolock autoLock(mLock);
    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return JNI_FALSE;
    }

    android::hardware::Return<Result> ret = service->setGlobalPQStrength(globalPQStrength);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setGlobalPQStrength failed!");
        return JNI_FALSE;
    }

    return JNI_TRUE;

#else

    UNUSED(globalPQStrength);
    ALOGE("[JNI_PQ] nativeSetGlobalPQStrength() not supported.");
    return JNI_FALSE;

#endif
}


static jint nativeGetGlobalPQStrength(JNIEnv* env, jclass clz)
{
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] nativeGetGlobalPQStrength(): MTK_PQ_SUPPORT disabled");
        return JNI_FALSE;
    }

    UNUSED(env);
    UNUSED(clz);

#if defined(MTK_GLOBAL_PQ_SUPPORT)

    Mutex::Autolock autoLock(mLock);
    uint32_t globalPQStrength;
    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return JNI_FALSE;
    }

    android::hardware::Return<void> ret = service->getGlobalPQStrength(
        [&] (Result retval, int32_t strength) {
        if (retval == Result::OK) {
            globalPQStrength = strength;
        } else {
            ALOGE("[JNI_PQ] nativeGetGlobalPQStrength() failed!");
        }
    });
    if (!ret.isOk()) {
        ALOGE("[JNI_PQ] IPictureQuality::getGlobalPQStrength failed!");
        return JNI_FALSE;
    }

    ALOGD("[JNI_PQ] globalPQStrength = %d\n", globalPQStrength);

    return globalPQStrength;

#else

    ALOGE("[JNI_PQ] nativeGetGlobalPQStrength() not supported.");
    return 0;

#endif
}


static jint nativeGetGlobalPQStrengthRange(JNIEnv* env, jclass clz)
{
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] nativeGetGlobalPQStrengthRange(): MTK_PQ_SUPPORT disabled");
        return JNI_FALSE;
    }

    UNUSED(env);
    UNUSED(clz);

#if defined(MTK_GLOBAL_PQ_SUPPORT)

    Mutex::Autolock autoLock(mLock);
    uint32_t globalPQStrengthRange;
    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return JNI_FALSE;
    }

    android::hardware::Return<void> ret = service->getGlobalPQStrengthRange(
        [&] (Result retval, int32_t strength_range) {
        if (retval == Result::OK) {
            globalPQStrengthRange = strength_range;
        } else {
            ALOGE("[JNI_PQ] nativeGetGlobalPQStrength() failed!");
        }
    });
    if (!ret.isOk()) {
        ALOGE("[JNI_PQ] IPictureQuality::getGlobalPQStrength failed!");
        return JNI_FALSE;
    }

    ALOGD("[JNI_PQ] globalPQStrengthRange = %d\n", globalPQStrengthRange);

    return globalPQStrengthRange;

#else

    ALOGE("[JNI_PQ] nativeGetGlobalPQStrengthRange() not supported.");
    return 0;

#endif
}

static jint nativeGetAALFunction(JNIEnv* env, jclass clz)
{
    UNUSED(env);
    UNUSED(clz);

    int mode = -1;

    if (AalIsSupport() == false) {
        ALOGE("nativeGetAALFunction(): MTK_AAL_SUPPORT disabled");
        return mode;
    }

    char value[PROPERTY_VALUE_MAX];
    Mutex::Autolock autoLock(mLock);

#ifdef MTK_SYSTEM_AAL
    if (__system_property_get("persist.vendor.sys.aal.function", value) > 0) {
        mode = atoi(value);
    }
#else
    if (__system_property_get("persist.vendor.sys.mtkaal.function", value) > 0) {
        mode = atoi(value);
    }
#endif

    return mode;
}

static void nativeSetAALFunction(JNIEnv* env, jclass clz, jint func)
{
    UNUSED(env);
    UNUSED(clz);

    if (AalIsSupport() == false) {
        ALOGE("nativeSetAALFunction(): MTK_AAL_SUPPORT disabled");
        return;
    }

#ifdef MTK_SYSTEM_AAL
    AALClient::getInstance().setFunction(func);
#else
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] nativeSetAALFunction(): MTK_PQ_SUPPORT disabled");
        return;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return;
    }

    android::hardware::Return<Result> ret = service->setFunction(func, false);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setFunction failed!");
    }
#endif
}

static void nativeSetAALFunctionProperty(JNIEnv* env, jclass clz, jint func)
{
    UNUSED(env);
    UNUSED(clz);

    if (AalIsSupport() == false) {
        ALOGE("nativeSetAALFunctionProperty(): MTK_AAL_SUPPORT disabled");
        return;
    }

#ifdef MTK_SYSTEM_AAL
    AALClient::getInstance().setFunctionProperty(func);
#else
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] nativeSetAALFunctionProperty(): MTK_PQ_SUPPORT disabled");
        return;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return;
    }

    android::hardware::Return<Result> ret = service->setFunction(func, true);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setFunction failed!");
    }
#endif
}

static void nativeSetSmartBacklightStrength(JNIEnv* env, jclass clz, jint value)
{
    UNUSED(env);
    UNUSED(clz);

    if (AalIsSupport() == false) {
        ALOGE("nativeSetSmartBacklightStrength(): MTK_AAL_SUPPORT disabled");
        return;
    }

#ifdef MTK_SYSTEM_AAL
    AALClient::getInstance().setSmartBacklightStrength(value);
#else
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] nativeSetSmartBacklightStrength(): MTK_PQ_SUPPORT disabled");
        return;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return;
    }

    android::hardware::Return<Result> ret = service->setSmartBacklightStrength(value);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setSmartBacklightStrength failed!");
    }
#endif
}

static void nativeSetReadabilityLevel(JNIEnv* env, jclass clz, jint value)
{
    UNUSED(env);
    UNUSED(clz);

    if (AalIsSupport() == false) {
        ALOGE("nativeSetReadabilityLevel(): MTK_AAL_SUPPORT disabled");
        return;
    }

#ifdef MTK_SYSTEM_AAL
    AALClient::getInstance().setReadabilityLevel(value);
#else
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] nativeSetReadabilityLevel(): MTK_PQ_SUPPORT disabled");
        return;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return;
    }

    android::hardware::Return<Result> ret = service->setReadabilityLevel(value);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setReadabilityLevel failed!");
    }
#endif
}

static void nativeSetLowBLReadabilityLevel(JNIEnv* env, jclass clz, jint value)
{
    UNUSED(env);
    UNUSED(clz);

    if (AalIsSupport() == false) {
        ALOGE("nativeSetLowBLReadabilityLevel(): MTK_AAL_SUPPORT disabled");
        return;
    }

#ifdef MTK_SYSTEM_AAL
    AALClient::getInstance().setLowBLReadabilityLevel(value);
#else
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] nativeSetLowBLReadabilityLevel(): MTK_PQ_SUPPORT disabled");
        return;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return;
    }

    android::hardware::Return<Result> ret = service->setLowBLReadabilityLevel(value);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setLowBLReadabilityLevel failed!");
    }
#endif
}

static jboolean nativeSetESSLEDMinStep(JNIEnv* env, jclass clz, jint value)
{
    UNUSED(env);
    UNUSED(clz);

    if (AalIsSupport() == false) {
        ALOGE("nativeSetESSLEDMinStep(): MTK_AAL_SUPPORT disabled");
        return JNI_FALSE;
    }

#ifdef MTK_SYSTEM_AAL
    AALClient::getInstance().setESSLEDMinStep(value);
#else
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] nativeSetESSLEDMinStep(): MTK_PQ_SUPPORT disabled");
        return JNI_FALSE;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return JNI_FALSE;
    }

    android::hardware::Return<Result> ret = service->setESSLEDMinStep(value);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setESSLEDMinStep failed!");
        return JNI_FALSE;
    }
#endif
    ALOGD("[JNI_PQ] Set ESSLEDMinStep = %d\n", value);

    return JNI_TRUE;
}


static jint nativeGetESSLEDMinStep(JNIEnv* env, jclass clz)
{
    UNUSED(env);
    UNUSED(clz);

    if (AalIsSupport() == false) {
        ALOGE("[JNI_PQ] nativeGetESSLEDMinStep(): MTK_AAL_SUPPORT disabled");
        return 0;
    }

    uint32_t value = 0;

#ifdef MTK_SYSTEM_AAL
    AALClient::getInstance().getESSLEDMinStep(&value);
#else
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] nativeGetESSLEDMinStep(): MTK_PQ_SUPPORT disabled");
        return 0;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return 0;
    }

    android::hardware::Return<void> ret = service->getESSLEDMinStep(
        [&] (Result retval, uint32_t getValue) {
        if (retval == Result::OK) {
            value = getValue;
        } else {
            ALOGE("[JNI_PQ] nativeGetESSLEDMinStep() failed!");
        }
    });
    if (!ret.isOk()) {
        ALOGE("[JNI_PQ] IPictureQuality::getESSLEDMinStep failed!");
        return 0;
    }
#endif
    ALOGD("[JNI_PQ] Get ESSLEDMinStep = %d\n", value);

    return value;
}

static jboolean nativeSetESSOLEDMinStep(JNIEnv* env, jclass clz, jint value)
{
    UNUSED(env);
    UNUSED(clz);

    if (AalIsSupport() == false) {
        ALOGE("nativeSetESSOLEDMinStep(): MTK_AAL_SUPPORT disabled");
        return JNI_FALSE;
    }

#ifdef MTK_SYSTEM_AAL
    AALClient::getInstance().setESSOLEDMinStep(value);
#else
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] nativeSetESSOLEDMinStep(): MTK_PQ_SUPPORT disabled");
        return JNI_FALSE;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return JNI_FALSE;
    }

    android::hardware::Return<Result> ret = service->setESSOLEDMinStep(value);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setESSOLEDMinStep failed!");
        return JNI_FALSE;
    }
#endif
    ALOGD("[JNI_PQ] Set ESSOLEDMinStep = %d\n", value);

    return JNI_TRUE;
}


static jint nativeGetESSOLEDMinStep(JNIEnv* env, jclass clz)
{
    UNUSED(env);
    UNUSED(clz);

    if (AalIsSupport() == false) {
        ALOGE("[JNI_PQ] nativeGetESSOLEDMinStep(): MTK_AAL_SUPPORT disabled");
        return JNI_FALSE;
    }

    uint32_t value = 0;

#ifdef MTK_SYSTEM_AAL
    AALClient::getInstance().getESSOLEDMinStep(&value);
#else
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] nativeGetESSOLEDMinStep(): MTK_PQ_SUPPORT disabled");
        return JNI_FALSE;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return 0;
    }

    android::hardware::Return<void> ret = service->getESSOLEDMinStep(
        [&] (Result retval, uint32_t getValue) {
        if (retval == Result::OK) {
            value = getValue;
        } else {
            ALOGE("[JNI_PQ] nativeGetESSOLEDMinStep() failed!");
        }
    });
    if (!ret.isOk()) {
        ALOGE("[JNI_PQ] IPictureQuality::getESSOLEDMinStep failed!");
        return 0;
    }
#endif
    ALOGD("[JNI_PQ] Get ESSOLEDMinStep = %d\n", value);

    return value;
}

static jboolean nativeSetExternalPanelNits(JNIEnv* env, jclass clz, jint externalPanelNits)
{
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] nativeSetExternalPanelNits(): MTK_PQ_SUPPORT disabled");
        return JNI_FALSE;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return JNI_FALSE;
    }

    android::hardware::Return<Result> ret = service->setExternalPanelNits(externalPanelNits);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setExternalPanelNits failed!");
        return JNI_FALSE;
    }

    UNUSED(env);
    UNUSED(clz);

    return JNI_TRUE;
}


static jint nativeGetExternalPanelNits(JNIEnv* env, jclass clz)
{
    uint32_t getExternalPanelNits = 0;

    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] nativeGetExternalPanelNits(): MTK_PQ_SUPPORT disabled");
        return JNI_FALSE;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return 0;
    }

    android::hardware::Return<void> ret = service->getExternalPanelNits(
        [&] (Result retval, uint32_t externalPanelNits) {
        if (retval == Result::OK) {
            getExternalPanelNits = externalPanelNits;
        } else {
            ALOGE("[JNI_PQ] nativeGetExternalPanelNits() failed!");
        }
    });
    if (!ret.isOk()) {
        ALOGE("[JNI_PQ] IPictureQuality::getExternalPanelNits failed!");
        return JNI_FALSE;
    }

    UNUSED(env);
    UNUSED(clz);

    return getExternalPanelNits;
}


static jboolean nativeSetRGBGain(JNIEnv *env, jobject thiz, jint r_gain, jint g_gain, jint b_gain, jint step)
{
    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] nativeSetRGBGain(): MTK_PQ_SUPPORT disabled");
        return JNI_FALSE;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return JNI_FALSE;
    }

    android::hardware::Return<Result> ret = service->setRGBGain(r_gain, g_gain, b_gain, step);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setRGBGain failed!");
        return JNI_FALSE;
    }

    UNUSED(env);
    UNUSED(thiz);

    return JNI_TRUE;
}

static jintArray nativeGetRGBGain(JNIEnv *env, jobject thiz)
{
    jintArray arr;
    arr = env->NewIntArray(3);
    if (arr == NULL)
    {
        ALOGE("[JNI_PQ] nativeGetRGBGain error, out of memory");
        return NULL;
    }
    jint rgb_arr[3];
    for (int i = 0; i < 3; i++) {
         rgb_arr[i] = 1024;
     }

    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] nativeGetRGBGain(): MTK_PQ_SUPPORT disabled");
        return arr;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return arr;
    }

    android::hardware::Return<void> ret;
    ret = service->getTuningField(MOD_RGB_GAIN, 0x0,
        [&] (Result retval, int32_t value) {
        if (retval == Result::OK) {
            rgb_arr[0] = value;
        }
    });
    if (!ret.isOk()) {
        ALOGE("[JNI_PQ] IPictureQuality::getTuningField(MOD_RGB_GAIN, 0x0) failed!");
        return arr;
    }
    ret = service->getTuningField(MOD_RGB_GAIN, 0x1,
        [&] (Result retval, int32_t value) {
        if (retval == Result::OK) {
            rgb_arr[1] = value;
        }
    });
    if (!ret.isOk()) {
        ALOGE("[JNI_PQ] IPictureQuality::getTuningField(MOD_RGB_GAIN, 0x1) failed!");
        return arr;
    }
    ret = service->getTuningField(MOD_RGB_GAIN, 0x2,
        [&] (Result retval, int32_t value) {
        if (retval == Result::OK) {
            rgb_arr[2] = value;
        }
    });
    if (!ret.isOk()) {
        ALOGE("[JNI_PQ] IPictureQuality::getTuningField(MOD_RGB_GAIN, 0x2) failed!");
        return arr;
    }

    env->SetIntArrayRegion(arr, 0, 3, rgb_arr);
    UNUSED(thiz);

    return arr;
}

static jboolean nativeSetCcorrMatrix(JNIEnv* env, jclass clz, jintArray matrix, jint step)
{
    UNUSED(clz);

    if (PQIsSupport() == false) {
        ALOGE("[JNI_PQ] nativeSetCcorrMatrix(): MTK_PQ_SUPPORT disabled");
        return JNI_FALSE;
    }

    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[JNI_PQ] failed to get HW service");
        return JNI_FALSE;
    }

    jsize arr_len;
    arr_len = env->GetArrayLength(matrix);
    if (arr_len != 9) {
        ALOGE("[JNI_PQ] nativeSetCcorrMatrix() input matrix size[%d] error!", arr_len);
        return JNI_FALSE;
    }

    int c_array[arr_len];
    env->GetIntArrayRegion(matrix, 0, arr_len, c_array);

    hidl_array<int32_t, 3, 3> send_matrix;
    for (int32_t i = 0; i < 3; i++)
    {
        for (int32_t j = 0; j < 3; j++)
        {
            send_matrix[i][j] = c_array[i*3 + j];
        }
    }
    android::hardware::Return<Result> ret = service->setColorMatrix3x3(send_matrix, step);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[JNI_PQ] IPictureQuality::setCcorrMatrix failed!");
        return JNI_FALSE;
    }

    return JNI_TRUE;
}


/////////////////////////////////////////////////////////////////////////////////

//JNI register
////////////////////////////////////////////////////////////////
static const char *classPathName = JNI_PQ_CLASS_NAME;

static JNINativeMethod g_methods[] = {

    // query features
    {"nativeGetCapability", "()I", (void*)getCapability},

    // Camera PQ switch
    {"nativeSetCameraPreviewMode", "(I)V", (void*)setCameraPreviewMode},
    {"nativeSetGalleryNormalMode", "(I)V", (void*)setGalleryNormalMode},
    {"nativeSetVideoPlaybackMode", "(I)V", (void*)setVideoPlaybackMode},
    // Image DC
    {"nativeGetDynamicContrastHistogram", "([BIIL" JNI_PQ_CLASS_NAME "$Hist;)V", (void*)getDynamicContrastHistogram},

    // MiraVision setting
    {"nativeEnablePQ", "(I)Z", (void*)enablePQ},
    {"nativeEnableColor", "(I)Z", (void*)enableColor},
    {"nativeEnableContentColor", "(I)Z", (void*)enableContentColor},
    {"nativeEnableSharpness", "(I)Z", (void*)enableSharpness},
    {"nativeEnableDynamicContrast", "(I)Z", (void*)enableDynamicContrast},
    {"nativeEnableDynamicSharpness", "(I)Z", (void*)enableDynamicSharpness},
    {"nativeEnableColorEffect", "(I)Z", (void*)enableColorEffect},
    {"nativeEnableGamma", "(I)Z", (void*)enableGamma},
    {"nativeEnableUltraResolution", "(I)Z", (void*)enableUltraResolution},
    {"nativeEnableISOAdaptiveSharpness", "(I)Z", (void*)enableISOAdaptiveSharpness},
    {"nativeGetPictureMode", "()I", (void*)getPictureMode},
    {"nativeSetPictureMode", "(II)Z", (void*)setPictureMode},
    {"nativeSetColorRegion", "(IIIII)Z", (void*)setColorRegion},
    {"nativeGetContrastIndexRange", "(L" JNI_PQ_CLASS_NAME "$Range;)V", (void*)getContrastIndexRange},
    {"nativeGetContrastIndex", "()I", (void*)getContrastIndex},
    {"nativeSetContrastIndex", "(II)V", (void*)setContrastIndex},
    {"nativeGetSaturationIndexRange", "(L" JNI_PQ_CLASS_NAME "$Range;)V", (void*)getSaturationIndexRange},
    {"nativeGetSaturationIndex", "()I", (void*)getSaturationIndex},
    {"nativeSetSaturationIndex", "(II)V", (void*)setSaturationIndex},
    {"nativeGetPicBrightnessIndexRange", "(L" JNI_PQ_CLASS_NAME "$Range;)V", (void*)getPicBrightnessIndexRange},
    {"nativeGetPicBrightnessIndex", "()I", (void*)getPicBrightnessIndex},
    {"nativeSetPicBrightnessIndex", "(II)V", (void*)setPicBrightnessIndex},
    {"nativeGetSharpnessIndexRange", "(L" JNI_PQ_CLASS_NAME "$Range;)V", (void*)getSharpnessIndexRange},
    {"nativeGetSharpnessIndex", "()I", (void*)getSharpnessIndex},
    {"nativeSetSharpnessIndex", "(I)V", (void*)setSharpnessIndex},
    {"nativeGetDynamicContrastIndexRange", "(L" JNI_PQ_CLASS_NAME "$Range;)V", (void*)getDynamicContrastIndexRange},
    {"nativeGetDynamicContrastIndex", "()I", (void*)getDynamicContrastIndex},
    {"nativeSetDynamicContrastIndex", "(I)V", (void*)setDynamicContrastIndex},
    {"nativeGetColorEffectIndexRange", "(L" JNI_PQ_CLASS_NAME "$Range;)V", (void*)getColorEffectIndexRange},
    {"nativeGetColorEffectIndex", "()I", (void*)getColorEffectIndex},
    {"nativeSetColorEffectIndex", "(I)V", (void*)setColorEffectIndex},
    {"nativeGetGammaIndexRange", "(L" JNI_PQ_CLASS_NAME "$Range;)V", (void*)getGammaIndexRange},
    {"nativeSetGammaIndex", "(II)V", (void*)setGammaIndex},
    {"nativeEnableOD", "(I)Z", (void*)enableOD},
    {"nativeSetBlueLightStrength", "(II)Z", (void*)nativeSetBlueLightStrength},
    {"nativeGetBlueLightStrength", "()I", (void*)nativeGetBlueLightStrength},
    {"nativeEnableBlueLight", "(ZI)Z", (void*)nativeEnableBlueLight},
    {"nativeIsBlueLightEnabled", "()Z", (void*)nativeIsBlueLightEnabled},
    {"nativeSetChameleonStrength", "(II)Z", (void*)nativeSetChameleonStrength},
    {"nativeGetChameleonStrength", "()I", (void*)nativeGetChameleonStrength},
    {"nativeEnableChameleon", "(ZI)Z", (void*)nativeEnableChameleon},
    {"nativeIsChameleonEnabled", "()Z", (void*)nativeIsChameleonEnabled},
    {"nativeGetDefaultOffTransitionStep", "()I", (void*)nativeGetDefaultOffTransitionStep},
    {"nativeGetDefaultOnTransitionStep", "()I", (void*)nativeGetDefaultOnTransitionStep},
    {"nativeSetGlobalPQSwitch", "(I)Z", (void*)nativeSetGlobalPQSwitch},
    {"nativeGetGlobalPQSwitch", "()I", (void*)nativeGetGlobalPQSwitch},
    {"nativeSetGlobalPQStrength", "(I)Z", (void*)nativeSetGlobalPQStrength},
    {"nativeGetGlobalPQStrength", "()I", (void*)nativeGetGlobalPQStrength},
    {"nativeGetGlobalPQStrengthRange", "()I", (void*)nativeGetGlobalPQStrengthRange},
    {"nativeEnableVideoHDR", "(Z)Z", (void*)nativeEnableVideoHDR},
    {"nativeIsVideoHDREnabled", "()Z", (void*)nativeIsVideoHDREnabled},
    {"nativeEnableMdpDRE", "(I)Z", (void*)nativeEnableMdpDRE},
    {"nativeEnableMdpCCORR", "(I)Z", (void*)nativeEnableMdpCCORR},
    {"nativeGetAALFunction", "()I", (void*)nativeGetAALFunction},
    {"nativeSetAALFunction", "(I)V", (void*)nativeSetAALFunction},
    {"nativeSetAALFunctionProperty", "(I)V", (void*)nativeSetAALFunctionProperty},
    {"nativeSetSmartBacklightStrength", "(I)V", (void*)nativeSetSmartBacklightStrength},
    {"nativeSetReadabilityLevel", "(I)V", (void*)nativeSetReadabilityLevel},
    {"nativeSetLowBLReadabilityLevel", "(I)V", (void*)nativeSetLowBLReadabilityLevel},
    {"nativeSetESSLEDMinStep", "(I)Z", (void*)nativeSetESSLEDMinStep},
    {"nativeGetESSLEDMinStep", "()I", (void*)nativeGetESSLEDMinStep},
    {"nativeSetESSOLEDMinStep", "(I)Z", (void*)nativeSetESSOLEDMinStep},
    {"nativeGetESSOLEDMinStep", "()I", (void*)nativeGetESSOLEDMinStep},
    {"nativeSetExternalPanelNits", "(I)Z", (void*)nativeSetExternalPanelNits},
    {"nativeGetExternalPanelNits", "()I", (void*)nativeGetExternalPanelNits},
    {"nativeSetRGBGain", "(IIII)Z", (void*)nativeSetRGBGain},
    {"nativeGetRGBGain", "()[I", (void*)nativeGetRGBGain},
    {"nativeSetCcorrMatrix", "([II)Z", (void*)nativeSetCcorrMatrix},
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
