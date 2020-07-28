/*
 * Copyright (C) 2011 The Android Open Source Project
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

#define LOG_TAG "PowerHal"
#include <log/log.h>
#include <dlfcn.h>

#include <stdio.h>

//#include <fcntl.h>      /* open */
//#include <unistd.h>     /* exit */
//#include <sys/ioctl.h>  /* ioctl */

#include "jni.h"

#undef NELEM
#define NELEM(x) (sizeof(x)/sizeof(*(x)))

//namespace android
//{
static int inited = false;

typedef int (*power_hal_wrap_scn_reg)();
typedef int (*power_hal_wrap_scn_config)(int, int, int, int, int, int);
typedef int (*power_hal_wrap_scn_unreg)(int);
typedef int (*power_hal_wrap_scn_enable)(int, int);
typedef int (*power_hal_wrap_scn_disable)(int);

static int (*powerHalWrapScnReg)() = NULL;
static int (*powerHalWrapScnConfig)(int, int, int, int, int, int) = NULL;
static int (*powerHalWrapScnUnreg)(int) = NULL;
static int (*powerHalWrapScnEnable)(int, int) = NULL;
static int (*powerHalWrapScnDisable)(int) = NULL;

#define LIB_FULL_NAME "libpowerhalwrap.so"

static void init()
{
    void *handle, *func;

    // only enter once
    inited = true;

    handle = dlopen(LIB_FULL_NAME, RTLD_NOW);
    if (handle == NULL) {
        ALOGE("Can't load library: %s", dlerror());
        return;
    }

    func = dlsym(handle, "PowerHal_Wrap_scnReg");
    powerHalWrapScnReg = reinterpret_cast<power_hal_wrap_scn_reg>(func);
    if (powerHalWrapScnReg == NULL) {
        ALOGE("PowerHal_Wrap_scnReg init fail!");
        return;
    }

    func = dlsym(handle, "PowerHal_Wrap_scnConfig");
    powerHalWrapScnConfig = reinterpret_cast<power_hal_wrap_scn_config>(func);
    if (powerHalWrapScnConfig == NULL) {
        ALOGE("PowerHal_Wrap_scnConfig init fail!");
        return;
    }

    func = dlsym(handle, "PowerHal_Wrap_scnUnreg");
    powerHalWrapScnUnreg = reinterpret_cast<power_hal_wrap_scn_unreg>(func);
    if (powerHalWrapScnUnreg == NULL) {
        ALOGE("PowerHal_Wrap_scnUnreg init fail!");
        return;
    }

    func = dlsym(handle, "PowerHal_Wrap_scnEnable");
    powerHalWrapScnEnable = reinterpret_cast<power_hal_wrap_scn_enable>(func);
    if (powerHalWrapScnEnable == NULL) {
        ALOGE("PowerHal_Wrap_scnEnable init fail!");
        return;
    }

    func = dlsym(handle, "PowerHal_Wrap_scnDisable");
    powerHalWrapScnDisable = reinterpret_cast<power_hal_wrap_scn_disable>(func);
    if (powerHalWrapScnDisable == NULL) {
        ALOGE("PowerHal_Wrap_scnDisable init fail!");
        return;
    }
}

static int
com_mediatek_powerhalservice_PowerHalMgrServiceImpl_scnReg(JNIEnv *env, jobject thiz)
{
    int hdl = 0;

    if (!inited)
        init();

    ALOGE("PowerHal_Wrap_scnReg");
    hdl = powerHalWrapScnReg();
    return hdl;
}

static void
com_mediatek_powerhalservice_PowerHalMgrServiceImpl_scnConfig(JNIEnv *env, jobject thiz,
                                                              jint handle, jint cmd, jint param_1,
                                                              jint param_2, jint param_3, jint param_4)
{
    if (!inited)
        init();

    ALOGE("PowerHal_Wrap_scnConfig");
    powerHalWrapScnConfig(handle, cmd, param_1, param_2, param_3, param_4);
}

static void
com_mediatek_powerhalservice_PowerHalMgrServiceImpl_scnUnreg(JNIEnv *env, jobject thiz,
                                                             jint handle)
{
    if (!inited)
        init();

    ALOGE("PowerHal_Wrap_scnUnreg");
    powerHalWrapScnUnreg(handle);
}

static void
com_mediatek_powerhalservice_PowerHalMgrServiceImpl_scnEnable(JNIEnv *env, jobject thiz,
                                                              jint handle, jint timeout)
{
    if (!inited)
        init();

    ALOGE("PowerHal_Wrap_scnEnable");
    powerHalWrapScnEnable(handle, timeout);
}

static void
com_mediatek_powerhalservice_PowerHalMgrServiceImpl_scnDisable(JNIEnv *env, jobject thiz,
                                                               jint handle)
{
    if (!inited)
        init();

    ALOGE("PowerHal_Wrap_scnDisable");
    powerHalWrapScnDisable(handle);
}

static JNINativeMethod sMethods[] = {
    {"nativePowerHalScnReg",   "()I",
        (int *)com_mediatek_powerhalservice_PowerHalMgrServiceImpl_scnReg},
    {"nativePowerHalScnConfig",   "(IIIIII)V",
        (void *)com_mediatek_powerhalservice_PowerHalMgrServiceImpl_scnConfig},
    {"nativePowerHalScnUnreg",   "(I)V",
        (void *)com_mediatek_powerhalservice_PowerHalMgrServiceImpl_scnUnreg},
    {"nativePowerHalScnEnable",   "(II)V",
        (void *)com_mediatek_powerhalservice_PowerHalMgrServiceImpl_scnEnable},
    {"nativePowerHalScnDisable",   "(I)V",
        (void *)com_mediatek_powerhalservice_PowerHalMgrServiceImpl_scnDisable},
};

int register_com_mediatek_powerhalservice_PowerHalMgrServiceImpl(JNIEnv* env)
{
    jclass clazz = env->FindClass("com/mediatek/powerhalservice/PowerHalMgrServiceImpl");
    ALOGE("PowerHal RegisterNatives");

    if (env->RegisterNatives(clazz, sMethods, NELEM(sMethods)) < 0) {
        ALOGE("RegisterNatives error");
        return JNI_ERR;
    }

    return JNI_OK;
}

extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;
    jint result = -1;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        return result;
    }

    register_com_mediatek_powerhalservice_PowerHalMgrServiceImpl(env);

    return JNI_VERSION_1_4;
}


//}

