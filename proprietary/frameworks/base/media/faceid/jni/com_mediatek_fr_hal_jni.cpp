/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2018. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "FR_CLIENT_JNI"
#include <utils/Log.h>

#include <jni.h>
#include <nativehelper/JNIHelp.h>
#include <utils/Mutex.h>
#include <android_runtime/AndroidRuntime.h>

#include <vendor/mediatek/hardware/camera/frhandler/1.0/IFRHandler.h>
#include <vendor/mediatek/hardware/camera/frhandler/1.0/IFRClientCallback.h>


using namespace android;
using namespace vendor::mediatek::hardware::camera::frhandler::V1_0;
using ::android::hardware::hidl_string;

// --------------------------------------PROCESS static VARIABLES----------------------------
static constexpr uint64_t kCookie = 0;
static sp<IFRHandler> gHidlService;
static jclass sJavaFaceIdManager = NULL;
// --------------------------------------PROCESS static VARIABLES END----------------------
class Utility {
public:
    static hidl_string getStringValue(JNIEnv* env, jstring string);
};

hidl_string Utility::getStringValue(JNIEnv* env, jstring string) {
    hidl_string hstr;

    if (NULL != string && string != env->NewStringUTF("")) {
        char* bytes = const_cast< char* > (env->GetStringUTFChars(string, NULL));

        const int length = strlen(bytes) + 1;
        char *data = new char[length];
        strncpy(data, bytes, length);
        hstr = hidl_string(data);

        env->ReleaseStringUTFChars(string, bytes);
        delete [] data;
        data = NULL;
    }
    return hstr;
}

//----------------------------------------HalClientHelper START-------------------------------------
/** HIDL Client Helper **/
class HalClientHelper : virtual public RefBase
{
public:
    HalClientHelper();
    virtual ~HalClientHelper() {}

    const sp<::android::hardware::hidl_death_recipient>&
        getHidlDeathRecipient() const { return mClientCallback.get(); }

    const sp<IFRClientCallback>& getFRClientCallback() const
    { return mClientCallback; }

private:
    uint64_t mHidlDeviceId;

    struct ClientCallback :
        virtual public IFRClientCallback,
        virtual public hardware::hidl_death_recipient
    {
        ClientCallback(const wp<HalClientHelper>& interface) : mInterface(interface) {}

        // Implementation of hidl_death_recipient interface
        void serviceDied(
                uint64_t cookie,
                const wp<hidl::base::V1_0::IBase>& who) override;

        // Implementation of vendor::mediatek::hardware::camera::frhandler::V1_0::IFRClientCallback
        virtual hardware::Return<void> onFRClientCallback(
                int32_t fr_cmd_id, int32_t errCode, const FRCbParam& param) override;

        wp<HalClientHelper> mInterface;
    };

    sp<ClientCallback> mClientCallback;
};

HalClientHelper::HalClientHelper()
    : mHidlDeviceId(0)
{
    // register the recipient for a notification if the hwbinder object goes away
    mClientCallback = new HalClientHelper::ClientCallback(this);
}

void HalClientHelper::ClientCallback::serviceDied(
        uint64_t cookie, const wp<hidl::base::V1_0::IBase>& who)
{
    (void) who;
    ALOGD("FRHandler has died");
    if (cookie != kCookie) {
        // TODO: cookie check...
    }

    // TODO: do something here if necessary when the HAL server has gone

    // release HIDL interface
    auto parent = mInterface.promote();
    if (parent.get())
        parent->mClientCallback.clear();
}

hardware::Return<void> HalClientHelper::ClientCallback::onFRClientCallback(
                int32_t fr_cmd_id, int32_t errCode, const FRCbParam& frCbParam)
{
    ALOGD("FRHandler callback, cmd:[%d], errCode: %d\n", fr_cmd_id, errCode);
    JavaVM* vm = AndroidRuntime::getJavaVM();
    int status = 0;
    if (vm != NULL) {
        JNIEnv* env = NULL;
        jint attachRes = 0;
        status = vm->GetEnv((void**)&env, JNI_VERSION_1_6);
        if (status == JNI_EDETACHED) {
            JavaVMAttachArgs args = {
                .version = JNI_VERSION_1_6, .name = "HwBinderCallback", .group = nullptr};
            attachRes = vm->AttachCurrentThread(&env, &args);
        } else if (status == JNI_OK) {
            attachRes = vm->AttachCurrentThread(&env, NULL);
        } else {
            ALOGE("Attach callback thread failed.");
            return hardware::Void();
        }
        if (attachRes != JNI_OK) {
            ALOGE("Attach callback thread failed.");
            return hardware::Void();
        }
        jmethodID onInfoMid = NULL;
        if (sJavaFaceIdManager != NULL) {
            onInfoMid = env->GetStaticMethodID(sJavaFaceIdManager, "onInfo", "(II)V");
        }
        if (onInfoMid != NULL) {
            env->CallStaticVoidMethod(sJavaFaceIdManager, onInfoMid, fr_cmd_id, errCode);
        }
        vm->DetachCurrentThread();
    }
    return hardware::Void();
}

static HalClientHelper gClientCallBack;
// ----------------------------------------HalClientHelper END----------------------------------

static Mutex sLock;

static jint com_mediatek_fr_init(JNIEnv* env, jobject thiz, jint uid, jobject callback) {
    Mutex::Autolock l(sLock);
    int err = 0;

    if (gHidlService == nullptr) {
        gHidlService= IFRHandler::getService("internal/0");
        if (gHidlService == nullptr) {
            ALOGE("failed to get FRHandler HIDL i/f");
            err = -1;
            return err;
        } else {
             ALOGE("get FRHandler HIDL i/f done");

             hardware::Return<bool> linked =
                 gHidlService->linkToDeath(
                         gClientCallBack.getHidlDeathRecipient(), /*cookie*/ kCookie);
             if (!linked.isOk()) {
                 ALOGE("%s: Transaction error in linking to FRHandler death: %s",
                         __FUNCTION__, linked.description().c_str());
                 err = -1;
                 return err;
             } else if (!linked) {
                 ALOGE("%s: Unable to link to FRHandler death notifications",
                         __FUNCTION__);
                 err = -1;
                 return err;
             }
        }
    }

    // to avoid previous Monkey's failture, call uninit first
    err = gHidlService->uninit();
    if (err != 0) {
        ALOGE("unit error before calling init.... result:%d", err);
    }

    err = gHidlService->init(uid, gClientCallBack.getFRClientCallback());
    if (err != 0) {
        ALOGE("init failed with %d\n", err);
    }
    return err;
}

static jint com_mediatek_fr_deinit(JNIEnv* env, jobject thiz) {
    Mutex::Autolock l(sLock);
    int errCode = gHidlService->uninit();
    if (errCode != 0) {
        ALOGE("unit failed result:%d", errCode);
    }
    ALOGE("deinit <--");
    return errCode;
}

static jint com_mediatek_fr_registerCallback(JNIEnv* env, jobject thiz, jint uid, jobject callback) {
    Mutex::Autolock l(sLock);
    return 0;
}

static jint com_mediatek_fr_setPowerMode(JNIEnv* env, jobject thiz, jint mode) {
    Mutex::Autolock l(sLock);
    int result = gHidlService->setPowerMode(mode);
    if (result != 0) {
        ALOGE("setPowerMode failed: %d\n", result);
    }
    return result;
}

static jint com_mediatek_fr_detectAndSaveFeature(JNIEnv* env, jobject thiz, jstring username) {
    Mutex::Autolock l(sLock);
    int result = gHidlService->startSaveFeature(Utility::getStringValue(env, username));
    if (result != 0) {
        ALOGE("detectAndSaveFeature, failed:%d\n", result);
    }
    return result;
}

static jint com_mediatek_fr_startCompareFeature(JNIEnv* env, jobject thiz, jstring username) {
    Mutex::Autolock l(sLock);
    int result = gHidlService->startCompareFeature(Utility::getStringValue(env, username));
    if (result != 0) {
        ALOGE("StartCompareFeature, failed:%d\n", result);
    }
    return result;
}



static jint com_mediatek_fr_stopCompareFeature(JNIEnv* env, jobject thiz) {
    Mutex::Autolock l(sLock);
    return gHidlService->stopCompareFeature();
}

static jint com_mediatek_fr_deleteFeature(JNIEnv* env, jobject thiz, jstring username) {
    Mutex::Autolock l(sLock);
    int result = gHidlService->deleteFeature(Utility::getStringValue(env, username));
    if (result != 0) {
        ALOGE("deleteFeature, failed:%d\n", result);
    }
    return result;
}

static const JNINativeMethod nativeMethods[] = {
    { "initNative", "(ILjava/lang/Object;)I",
                                       (void*) com_mediatek_fr_init },
    { "registerCallbackNative", "(ILjava/lang/Object;)I",
                                       (void*) com_mediatek_fr_registerCallback },
    { "setPowerModeNative", "(I)I",
                                       (void*) com_mediatek_fr_setPowerMode },
    { "detectAndSaveFeatureNative", "(Ljava/lang/String;)I",
                                       (void*) com_mediatek_fr_detectAndSaveFeature },
    { "startCompareFeatureNative", "(Ljava/lang/String;)I",
                                       (void*) com_mediatek_fr_startCompareFeature },
    { "stopCompareFeatureNative", "()I",
                                       (void*) com_mediatek_fr_stopCompareFeature },
    { "deleteFeatureNative", "(Ljava/lang/String;)I",
                                       (void*) com_mediatek_fr_deleteFeature },
    { "releaseNative", "()I",
                                       (void*) com_mediatek_fr_deinit }
};

static int registerNativeMethods(JNIEnv* env) {
    int result = -1;

    /* look up the class */
    jclass clazz = env->FindClass("com/mediatek/faceid/FaceIdManager");

    if (NULL != clazz) {
        if (env->RegisterNatives(clazz, nativeMethods, sizeof(nativeMethods)
                / sizeof(nativeMethods[0])) == JNI_OK) {
            result = JNI_OK;
        } else {
            result = JNI_ERR;
        }

        //sJavaFaceIdManager is binded to the caller process.
        sJavaFaceIdManager = (jclass) env->NewGlobalRef(clazz);
    }
    return result;
}

jint JNI_OnLoad(JavaVM* vm, void* /* reserved */) {
    JNIEnv* env = NULL;
    jint result = -1;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) == JNI_OK) {
        if (NULL != env && registerNativeMethods(env) == 0) {
            result = JNI_VERSION_1_4;
        }
    }
    return result;
}
