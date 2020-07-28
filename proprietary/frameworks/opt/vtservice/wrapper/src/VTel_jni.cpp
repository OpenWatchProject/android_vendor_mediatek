/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
#include <utils/RefBase.h>
#include <utils/String8.h>
#include <utils/threads.h>
#include "jni.h"
//#include "JNIHelp.h"
#include "android_runtime/AndroidRuntime.h"
#include <android_runtime/android_view_Surface.h>
#include <Surface.h>
#include <SurfaceComposerClient.h>
#include "VTDef.h"
#include "VTClient.h"

using namespace android;
using namespace VTService;

struct field {
    const char *class_name;
    const char *field_name;
    const char *field_type;
    jfieldID   *jfield;
};

struct fields_t {
    jfieldID    context;
    jfieldID    surface;
    jfieldID    surfaceControl;
    jmethodID   post_event;
};

struct fields_sensor_info{
    jfieldID mId;
    jfieldID mMaxWidth;
    jfieldID mMaxHeight;
    jfieldID mDegree;
    jfieldID mFacing;
    jfieldID mHal;
};

JavaVM*             g_vm = NULL;
jclass              g_ImsProvider = NULL;
jclass              g_VTSourceSensorInfo = NULL;
static fields_t     g_Ims_class_fields;
static sp<VTClient> g_client = NULL;
static fields_sensor_info g_VTS_SensorInfo_class_fields;

void notify(
    int id,
    int msgType,
    int arg1,
    int arg2,
    int arg3,
    const char* obj1,
    const char* obj2,
    const sp<IGraphicBufferProducer>& obj3) {

    VT_LOGD("[JNI] notify Entering (thread id %x)", (unsigned int) pthread_self());
    VT_LOGD("[JNI] notify id : %d, msg : %d, arg1 : %d, arg2 : %d, arg3 : %d", id, msgType, arg1, arg2, arg3);

    if (NULL == g_vm) {
        VT_LOGE("[JNI] notify g_vm = 0");
        return;
    }

    if (msgType == VT_SRV_ERROR_SERVER_DIED) {
        g_client = NULL;
        VT_LOGE("[JNI] notify set g_client = 0");
    }

    JNIEnv *env = AndroidRuntime::getJNIEnv();

    const char* tmpstr1 = (const char *) obj1;
    jstring rtstr1 = env->NewStringUTF(tmpstr1);
    const char* tmpstr2 = (const char *) obj2;
    jstring rtstr2 = env->NewStringUTF(tmpstr2);
    jobject rtobj3= android_view_Surface_createFromIGraphicBufferProducer(env, obj3);

    if (g_Ims_class_fields.post_event != NULL) {
            env->CallStaticVoidMethod(
                    g_ImsProvider,
                    g_Ims_class_fields.post_event,
                    msgType,
                    id,
                    arg1,
                    arg2,
                    arg3,
                    rtstr1,
                    rtstr2,
                    rtobj3);
    } else {
        VT_LOGI("[JNI] notify g_Ims_class_fields = 0");
    }

    // Need to delete local reference,
    // otherwise would resulting local reference table overflow(max=512).
    VT_LOGD("[JNI] notify delete local reference string(rtstr1/2/3)");
    env->DeleteLocalRef(rtstr1);
    env->DeleteLocalRef(rtstr2);

    VT_LOGD("[JNI] notify delete local reference surface(rtobj3)");
    env->DeleteLocalRef(rtobj3);

    // re-get client again
    //while (g_client == NULL) {
        //VT_LOGE("[JNI] notify (g_client == 0)");
        //g_client = VTClient::getVTClient();
    //}

    VT_LOGD("[JNI] notify Leave");
}

// Get all the required offsets in java class
void native_init(JNIEnv *env, int mode) {
    VT_LOGI("[JNI] Entering native_init");

    if (mode == 4 && NULL == g_ImsProvider) {
        // local reference to VTelProvider class
        g_ImsProvider = env->FindClass("com/mediatek/ims/internal/ImsVTProvider");

        // global reference to VTelProvider class
        g_ImsProvider = (jclass)env->NewGlobalRef(g_ImsProvider);

        g_Ims_class_fields.post_event
                = env->GetStaticMethodID(
                        g_ImsProvider,
                        "postEventFromNative",
                        "(IIIIILjava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;)V");

        if (NULL == g_Ims_class_fields.post_event) {
            VT_LOGE("[JNI] native_init Entering (post_event == NULL)");
        }

        // global reference to VTSource Resolution class
        g_VTSourceSensorInfo = env->FindClass("com/mediatek/ims/internal/VTSource$Resolution");
        g_VTSourceSensorInfo = (jclass)env->NewGlobalRef(g_VTSourceSensorInfo);
        g_VTS_SensorInfo_class_fields.mId = env->GetFieldID(g_VTSourceSensorInfo, "mId", "I");
        g_VTS_SensorInfo_class_fields.mMaxWidth = env->GetFieldID(g_VTSourceSensorInfo, "mMaxWidth", "I");
        g_VTS_SensorInfo_class_fields.mMaxHeight = env->GetFieldID(g_VTSourceSensorInfo, "mMaxHeight", "I");
        g_VTS_SensorInfo_class_fields.mDegree = env->GetFieldID(g_VTSourceSensorInfo, "mDegree", "I");
        g_VTS_SensorInfo_class_fields.mFacing = env->GetFieldID(g_VTSourceSensorInfo, "mFacing", "I");
        g_VTS_SensorInfo_class_fields.mHal = env->GetFieldID(g_VTSourceSensorInfo, "mHal", "I");

    } else {
        VT_LOGI("[JNI] native_init Entering (Already init Provider)");
    }

    //Connect vtservice after notify method ready
    while (g_client == NULL) {
        VT_LOGE("[JNI] native_init (g_client == 0)");
        g_client = VTClient::getVTClient();
    }

    VT_LOGD("[JNI] native_init Leave");
}

void native_deinit(JNIEnv *env, int mode) {
    RFX_UNUSED(env);
    RFX_UNUSED(mode);

    /* never clear once connect to Java */
    /* it is for all calls, not for one call only */
    /*
    if (mode == 4) {
        env->DeleteGlobalRef(g_ImsProvider);
        g_ImsProvider = NULL;
        g_Ims_class_fields.post_event = NULL;
    }
    */
}

#ifdef __cplusplus
extern "C" {
#endif

int JNI_OnLoad(JavaVM* vm, void* reserved) {
    RFX_UNUSED(reserved);

    JNIEnv* env = NULL;
    g_vm = vm;
    VT_LOGD("Entering JNI_OnLoad\n");
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_4) != JNI_OK) {
        ALOGE("GetEnv failed!");
        return -1;
    }

    return JNI_VERSION_1_4;
}

// ============================================================================
jint Java_com_mediatek_ims_internal_ImsVTProvider_nInitRefVTP(JNIEnv *env, jobject thiz) {
    VT_LOGI("[JNI] Entering InitRefVTP");

    RFX_UNUSED(thiz);

    // initialize the related global varialbes
    native_init(env, 4);

    VT_LOGI("[JNI] Leaving InitRefVTP");
    return VT_SRV_RET_OK;
}

jint Java_com_mediatek_ims_internal_ImsVTProvider_nInitialization(JNIEnv *env, jobject thiz, jint id, jint sim_id) {
    VT_LOGI("[JNI] Entering Initialization");

    RFX_UNUSED(env);
    RFX_UNUSED(thiz);

    while (g_client == NULL) {
        VT_LOGE("[JNI] Initialization (g_client == 0)");
        g_client = VTClient::getVTClient();
    }

    int ret = g_client->initialization(VT_SRV_CALL_4G, id, sim_id);
    if (ret != VT_SRV_RET_OK) {
        VT_LOGE("[JNI] Initialization (failed)");
        return ret;
    }

    VT_LOGI("[JNI] Leaving Initialization");
    return VT_SRV_RET_OK;
}

jint Java_com_mediatek_ims_internal_ImsVTProvider_nFinalization(JNIEnv *env, jobject thiz, jint id) {
    VT_LOGI("[JNI] Entering Finalization");

    RFX_UNUSED(thiz);

    while (g_client == NULL) {
        VT_LOGE("[JNI] Finalization (g_client == 0)");
        g_client = VTClient::getVTClient();
    }

    int ret = g_client->finalization(id);
    if (ret != VT_SRV_RET_OK) {
        VT_LOGE("[JNI] Finalization (failed)");
        return ret;
    }

    // initialize the related global varialbes
    native_deinit(env, 4);

    VT_LOGI("[JNI] Leaving Finalization");
    return VT_SRV_RET_OK;
}

jint Java_com_mediatek_ims_internal_ImsVTProvider_nSetCamera(JNIEnv *env, jobject thiz, jint id, jint cam) {
    VT_LOGI("[JNI] Entering SetCamera");

    RFX_UNUSED(env);
    RFX_UNUSED(thiz);

    while (g_client == NULL) {
        VT_LOGE("[JNI] SetCamera (g_client == 0)");
        g_client = VTClient::getVTClient();
    }

    int ret = g_client->setCamera(id, cam);
    if (ret != VT_SRV_RET_OK) {
        VT_LOGE("[JNI] SetCamera (failed)");
        return ret;
    }

    VT_LOGI("[JNI] Leaving SetCamera");
    return VT_SRV_RET_OK;
}

jint Java_com_mediatek_ims_internal_ImsVTProvider_nSetPreviewSurface(JNIEnv *env, jobject thiz, jint id, jobject surface) {
    VT_LOGI("[JNI] Entering SetPreviewSurface");

    RFX_UNUSED(thiz);

    while (g_client == NULL) {
        VT_LOGE("[JNI] SetPreviewSurface (g_client == 0)");
        g_client = VTClient::getVTClient();
    }

    sp<Surface> sf = NULL;
    if (surface != NULL) {
        sf = android_view_Surface_getSurface(env, surface);
    }

    VT_LOGE("[JNI] SetPreviewSurface (surface = %p)", sf.get());
    int ret = g_client->setPreviewSurface(id, sf);
    if (ret != VT_SRV_RET_OK) {
        VT_LOGE("[JNI] SetPreviewSurface (failed)");
        return ret;
    }

    VT_LOGI("[JNI] Leaving SetPreviewSurface");
    return VT_SRV_RET_OK;
}

jint Java_com_mediatek_ims_internal_ImsVTProvider_nSetDisplaySurface(JNIEnv *env, jobject thiz, jint id, jobject surface) {
    VT_LOGI("[JNI] Entering SetDisplaySurface");

    RFX_UNUSED(thiz);

    while (g_client == NULL) {
        VT_LOGE("[JNI] SetDisplaySurface (g_client == 0)");
        g_client = VTClient::getVTClient();
    }

    sp<Surface> sf = NULL;
    if (surface != NULL) {
        sf = android_view_Surface_getSurface(env, surface);
    }

    VT_LOGE("[JNI] SetDisplaySurface (surface = %p)", sf.get());
    int ret = g_client->setDisplaySurface(id, sf);
    if (ret != VT_SRV_RET_OK) {
        VT_LOGE("[JNI] SetDisplaySurface (failed)");
        return ret;
    }

    VT_LOGI("[JNI] Leaving SetDisplaySurface");
    return VT_SRV_RET_OK;
}

jint Java_com_mediatek_ims_internal_ImsVTProvider_nSetCameraParameters(JNIEnv *env, jobject thiz, jint id, jobjectArray sens_info) {
    VT_LOGI("[JNI] Entering SetCameraParameters");

    RFX_UNUSED(thiz);

    while (g_client == NULL) {
        VT_LOGE("[JNI] SetCameraParameters (g_client == 0)");
        g_client = VTClient::getVTClient();
    }

    if (sens_info == NULL) {
        VT_LOGE("[JNI] SetCameraParameters (sens_info = NULL)");

        int ret = g_client->setCameraParameters(id, 0, NULL);
        if (ret != VT_SRV_RET_OK) {
            VT_LOGE("[JNI] SetCameraParameters (failed)");
            return ret;
        }
        return VT_SRV_RET_OK;
    }

    jsize length = env->GetArrayLength(sens_info);

    if(length > 0) {
        sensor_info_vilte_t *sensors = new sensor_info_vilte_t[length];
        for (jsize i = 0; i < length; i++) {

            jobject sen_info = env->GetObjectArrayElement(sens_info, i);
            if (!sen_info) {
                VT_LOGE("[JNI] SetCameraParameters (sens_res[%d]=NULL)", i);
                break;
            }

            sensors[i].index = env->GetIntField(sen_info, g_VTS_SensorInfo_class_fields.mId);
            sensors[i].max_width = env->GetIntField(sen_info, g_VTS_SensorInfo_class_fields.mMaxWidth);
            sensors[i].max_height = env->GetIntField(sen_info, g_VTS_SensorInfo_class_fields.mMaxHeight);
            sensors[i].degree = env->GetIntField(sen_info, g_VTS_SensorInfo_class_fields.mDegree);
            sensors[i].facing = env->GetIntField(sen_info, g_VTS_SensorInfo_class_fields.mFacing);
            sensors[i].hal = env->GetIntField(sen_info, g_VTS_SensorInfo_class_fields.mHal);

            env->DeleteLocalRef(sen_info);
        }

        int ret = g_client->setCameraParameters(id, length, sensors);
        if (ret != VT_SRV_RET_OK) {
            delete [] sensors;
            VT_LOGE("[JNI] SetCameraParameters (failed)");
            return ret;
        }
        delete [] sensors;

    } else {
        VT_LOGE("[JNI] SetCameraParameters (sens_res number = 0)");

        int ret = g_client->setCameraParameters(id, 0, NULL);
        if (ret != VT_SRV_RET_OK) {
            VT_LOGE("[JNI] SetCameraParameters (failed)");
            return ret;
        }
    }

    VT_LOGI("[JNI] Leaving SetCameraParameters");
    return VT_SRV_RET_OK;
}

jint Java_com_mediatek_ims_internal_ImsVTProvider_nSetCameraParametersOnly(JNIEnv *env, jobject thiz, jobjectArray sens_info) {
    VT_LOGI("[JNI] Entering SetCameraParametersOnly");

    RFX_UNUSED(thiz);

    while (g_client == NULL) {
        VT_LOGE("[JNI] SetCameraParametersOnly (g_client == 0)");
        g_client = VTClient::getVTClient();
    }

    if (sens_info == NULL) {
        VT_LOGE("[JNI] SetCameraParametersOnly (sens_info = NULL)");

        int ret = g_client->setCameraParametersOnly(0, NULL);
        if (ret != VT_SRV_RET_OK) {
            VT_LOGE("[JNI] SetCameraParametersOnly (failed)");
            return ret;
        }
        return VT_SRV_RET_OK;
    }

    jsize length = env->GetArrayLength(sens_info);

    if(length > 0) {
        sensor_info_vilte_t *sensors = new sensor_info_vilte_t[length];
        for (jsize i = 0; i < length; i++) {

            jobject sen_info = env->GetObjectArrayElement(sens_info, i);
            if (!sen_info) {
                VT_LOGE("[JNI] SetCameraParametersOnly (sens_res[%d]=NULL)", i);
                break;
            }

            sensors[i].index = env->GetIntField(sen_info, g_VTS_SensorInfo_class_fields.mId);
            sensors[i].max_width = env->GetIntField(sen_info, g_VTS_SensorInfo_class_fields.mMaxWidth);
            sensors[i].max_height = env->GetIntField(sen_info, g_VTS_SensorInfo_class_fields.mMaxHeight);
            sensors[i].degree = env->GetIntField(sen_info, g_VTS_SensorInfo_class_fields.mDegree);
            sensors[i].facing = env->GetIntField(sen_info, g_VTS_SensorInfo_class_fields.mFacing);
            sensors[i].hal = env->GetIntField(sen_info, g_VTS_SensorInfo_class_fields.mHal);

            env->DeleteLocalRef(sen_info);
        }

        int ret = g_client->setCameraParametersOnly(length, sensors);
        if (ret != VT_SRV_RET_OK) {
            delete [] sensors;
            VT_LOGE("[JNI] SetCameraParametersOnly (failed)");
            return ret;
        }
        delete [] sensors;

    } else {
        VT_LOGE("[JNI] SetCameraParametersOnly (sens_res number = 0)");

        int ret = g_client->setCameraParametersOnly(0, NULL);
        if (ret != VT_SRV_RET_OK) {
            VT_LOGE("[JNI] SetCameraParametersOnly (failed)");
            return ret;
        }
    }

    VT_LOGI("[JNI] Leaving setCameraParametersOnly");
    return VT_SRV_RET_OK;
}

jint Java_com_mediatek_ims_internal_ImsVTProvider_nSetCameraParametersWithSim(JNIEnv *env, jobject thiz, jint id, jint major_sim_id, jobjectArray sens_info) {
    VT_LOGI("[JNI] Entering SetCameraParametersWithSim");

    RFX_UNUSED(thiz);

    while (g_client == NULL) {
        VT_LOGE("[JNI] SetCameraParametersWithSim (g_client == 0)");
        g_client = VTClient::getVTClient();
    }

    if (sens_info == NULL) {
        VT_LOGE("[JNI] SetCameraParametersWithSim (sens_info = NULL)");

        int ret = g_client->setCameraParametersWithSim(id, major_sim_id, 0, NULL);
        if (ret != VT_SRV_RET_OK) {
            VT_LOGE("[JNI] SetCameraParametersWithSim (failed)");
            return ret;
        }
        return VT_SRV_RET_OK;
    }

    jsize length = env->GetArrayLength(sens_info);

    if(length > 0) {
        sensor_info_vilte_t *sensors = new sensor_info_vilte_t[length];
        for (jsize i = 0; i < length; i++) {

            jobject sen_info = env->GetObjectArrayElement(sens_info, i);
            if (!sen_info) {
                VT_LOGE("[JNI] SetCameraParametersWithSim (sens_res[%d]=NULL)", i);
                break;
            }

            sensors[i].index = env->GetIntField(sen_info, g_VTS_SensorInfo_class_fields.mId);
            sensors[i].max_width = env->GetIntField(sen_info, g_VTS_SensorInfo_class_fields.mMaxWidth);
            sensors[i].max_height = env->GetIntField(sen_info, g_VTS_SensorInfo_class_fields.mMaxHeight);
            sensors[i].degree = env->GetIntField(sen_info, g_VTS_SensorInfo_class_fields.mDegree);
            sensors[i].facing = env->GetIntField(sen_info, g_VTS_SensorInfo_class_fields.mFacing);
            sensors[i].hal = env->GetIntField(sen_info, g_VTS_SensorInfo_class_fields.mHal);

            env->DeleteLocalRef(sen_info);
        }

        int ret = g_client->setCameraParametersWithSim(id, major_sim_id, length, sensors);
        if (ret != VT_SRV_RET_OK) {
            delete [] sensors;
            VT_LOGE("[JNI] SetCameraParametersWithSim (failed)");
            return ret;
        }
        delete [] sensors;

    } else {
        VT_LOGE("[JNI] SetCameraParametersWithSim (sens_res number = 0)");

        int ret = g_client->setCameraParametersWithSim(id, major_sim_id, 0, NULL);
        if (ret != VT_SRV_RET_OK) {
            VT_LOGE("[JNI] SetCameraParametersWithSim (failed)");
            return ret;
        }
    }

    VT_LOGI("[JNI] Leaving SetCameraParametersWithSim");
    return VT_SRV_RET_OK;
}

jint Java_com_mediatek_ims_internal_ImsVTProvider_nSetDeviceOrientation(JNIEnv *env, jobject thiz, jint id, jint rotation) {
    VT_LOGI("[JNI] Entering SetDeviceOrientation");

    RFX_UNUSED(env);
    RFX_UNUSED(thiz);

    while (g_client == NULL) {
        VT_LOGE("[JNI] SetDeviceOrientation (g_client == 0)");
        g_client = VTClient::getVTClient();
    }

    int ret = g_client->setDeviceOrientation(id, rotation);
    if (ret != VT_SRV_RET_OK) {
        VT_LOGE("[JNI] SetDeviceOrientation (failed)");
        return ret;
    }

    VT_LOGI("[JNI] Leaving SetDeviceOrientation");
    return VT_SRV_RET_OK;
}

jint Java_com_mediatek_ims_internal_ImsVTProvider_nSetUIMode(JNIEnv *env, jobject thiz, jint id, jint mode) {
    VT_LOGI("[JNI] Entering SetUIMode");

    RFX_UNUSED(env);
    RFX_UNUSED(thiz);

    while (g_client == NULL) {
        VT_LOGE("[JNI] SetUIMode (g_client == 0)");
        g_client = VTClient::getVTClient();
    }

    int ret = g_client->setUIMode(id, (VT_SRV_UI_MODE) mode);
    if (ret != VT_SRV_RET_OK) {
        VT_LOGE("[JNI] SetUIMode (failed)");
        return ret;
    }

    VT_LOGI("[JNI] Leaving SetUIMode");
    return VT_SRV_RET_OK;
}

jint Java_com_mediatek_ims_internal_ImsVTProvider_nRequestPeerConfig(JNIEnv *env, jobject thiz, jint id, jstring config) {
    VT_LOGI("[JNI] Entering RequestPeerConfig");

    RFX_UNUSED(thiz);

    while (g_client == NULL) {
        VT_LOGE("[JNI] RequestPeerConfig (g_client == 0)");
        g_client = VTClient::getVTClient();
    }

    const jchar* str = env->GetStringCritical(config, 0);
    String8 params8;
    if (config) {
        params8 = String8((const char16_t*)str, env->GetStringLength(config));
        env->ReleaseStringCritical(config, str);
    }

    int ret = g_client->requestSessionModify(id, params8);
    if (ret != VT_SRV_RET_OK) {
        VT_LOGE("[JNI] RequestPeerConfig (failed)");
        return ret;
    }

    VT_LOGI("[JNI] Leaving RequestPeerConfig");
    return VT_SRV_RET_OK;
}

jint Java_com_mediatek_ims_internal_ImsVTProvider_nResponseLocalConfig(JNIEnv *env, jobject thiz, jint id, jstring config) {
    VT_LOGI("[JNI] Entering ResponseLocalConfig");

    RFX_UNUSED(thiz);

    while (g_client == NULL) {
        VT_LOGE("[JNI] ResponseLocalConfig (g_client == 0)");
        g_client = VTClient::getVTClient();
    }

    const jchar* str = env->GetStringCritical(config, 0);
    String8 params8;
    if (config) {
        params8 = String8((const char16_t*)str, env->GetStringLength(config));
        env->ReleaseStringCritical(config, str);
    }

    int ret = g_client->responseSessionModify(id, params8);
    if (ret != VT_SRV_RET_OK) {
        VT_LOGE("[JNI] ResponseLocalConfig (failed)");
        return ret;
    }

    VT_LOGI("[JNI] Leaving ResponseLocalConfig");
    return VT_SRV_RET_OK;
}

jint Java_com_mediatek_ims_internal_ImsVTProvider_nSnapshot(JNIEnv *env, jobject thiz, jint id, jint type, jstring uri) {
    VT_LOGI("[JNI] Entering Snapshot(");

    RFX_UNUSED(thiz);

    while (g_client == NULL) {
        VT_LOGE("[JNI] Snapshot( (g_client == 0)");
        g_client = VTClient::getVTClient();
    }

    const jchar* str = env->GetStringCritical(uri, 0);
    String8 params8;
    if (uri) {
        params8 = String8((const char16_t*)str, env->GetStringLength(uri));
        env->ReleaseStringCritical(uri, str);
    }

    int ret = g_client->snapshot(id, (VT_SRV_SNAPSHOT_TYPE)type, params8);
    if (ret != VT_SRV_RET_OK) {
        VT_LOGE("[JNI] Snapshot( (failed)");
        return ret;
    }

    VT_LOGI("[JNI] Leaving Snapshot");
    return VT_SRV_RET_OK;
}

jint Java_com_mediatek_ims_internal_ImsVTProvider_nStartRecording(JNIEnv *env, jobject thiz, jint id, jint type, jstring url, jlong maxSize) {
    VT_LOGI("[JNI] Entering StartRecording");

    RFX_UNUSED(thiz);

    while (g_client == NULL) {
        VT_LOGE("[JNI] StartRecording (g_client == 0)");
        g_client = VTClient::getVTClient();
    }

    const jchar* str = env->GetStringCritical(url, 0);
    String8 params8;
    if (url) {
        params8 = String8((const char16_t*)str, env->GetStringLength(url));
        env->ReleaseStringCritical(url, str);
    }

    int ret = g_client->startRecording(id, (VT_SRV_RECORD_TYPE) type, params8, maxSize);
    if (ret != VT_SRV_RET_OK) {
        VT_LOGE("[JNI] StartRecording (failed)");
        return ret;
    }

    VT_LOGI("[JNI] Leaving StartRecording");
    return VT_SRV_RET_OK;
}

jint Java_com_mediatek_ims_internal_ImsVTProvider_nStopRecording(JNIEnv *env, jobject thiz, jint id) {
    VT_LOGI("[JNI] Entering StopRecording");

    RFX_UNUSED(env);
    RFX_UNUSED(thiz);

    while (g_client == NULL) {
        VT_LOGE("[JNI] StopRecording (g_client == 0)");
        g_client = VTClient::getVTClient();
    }

    int ret = g_client->stopRecording(id);
    if (ret != VT_SRV_RET_OK) {
        VT_LOGE("[JNI] StopRecording (failed)");
        return ret;
    }

    VT_LOGI("[JNI] Leaving StopRecording");
    return VT_SRV_RET_OK;
}

jint Java_com_mediatek_ims_internal_ImsVTProvider_nSwitchFeature(JNIEnv *env, jobject thiz, jint id, jint feature, jint isOn) {
    VT_LOGI("[JNI] Entering SwitchFeature");

    RFX_UNUSED(env);
    RFX_UNUSED(thiz);

    while (g_client == NULL) {
        VT_LOGE("[JNI] SwitchFeature (g_client == 0)");
        g_client = VTClient::getVTClient();
    }

    int ret = g_client->switchFeature(id, feature, isOn);
    if (ret != VT_SRV_RET_OK) {
        VT_LOGE("[JNI] SwitchFeature (failed)");
        return ret;
    }

    VT_LOGI("[JNI] Leaving SwitchFeature");
    return VT_SRV_RET_OK;
}

jint Java_com_mediatek_ims_internal_ImsVTProvider_nUpdateNetworkTable(JNIEnv *env, jobject thiz, jboolean is_add, jint network_id, jstring if_name) {
    VT_LOGI("[JNI] Entering UpdateNetworkTable");

    RFX_UNUSED(thiz);

    while (g_client == NULL) {
        VT_LOGE("[JNI] UpdateNetworkTable (g_client == 0)");
        g_client = VTClient::getVTClient();
    }

    String8 params8 = String8("");
    if (if_name != NULL) {
        const jchar* str = env->GetStringCritical(if_name, 0);
        params8 = String8((const char16_t*)str, env->GetStringLength(if_name));
        env->ReleaseStringCritical(if_name, str);
    }

    int ret = g_client->updateNetworkTable(is_add, network_id, params8);
    if (ret != VT_SRV_RET_OK) {
        VT_LOGE("[JNI] UpdateNetworkTable (failed)");
        return ret;
    }

    VT_LOGI("[JNI] Leaving UpdateNetworkTable");
    return VT_SRV_RET_OK;

}

#ifdef __cplusplus
};

#endif
