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

#define LOG_TAG "PowerHalWrapper"
#include <log/log.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

#include "jni.h"

#undef NELEM
#define NELEM(x) (sizeof(x)/sizeof(*(x)))

namespace android
{
static int inited = false;

#define LIB_FULL_NAME "libpowerhalwrap.so"
#define PERF_LOCK_LIB_FULL_NAME  "libmtkperf_client.so"

static int (*phwMtkPowerHint)(uint32_t, int32_t) = NULL;
static int (*phwMtkCusPowerHint)(uint32_t, int32_t) = NULL;
static int (*phwQuerySysInfo)(uint32_t, int32_t) = NULL;
static int (*phwNotifyAppState)(const char *, const char *, int32_t, int32_t, int32_t) = NULL;
static int (*phwScnReg)(void) = NULL;
static int (*phwScnConfig)(int32_t, int32_t, int32_t, int32_t, int32_t, int32_t) = NULL;
static int (*phwScnUnreg)(int32_t) = NULL;
static int (*phwScnEnable)(int32_t, int32_t) = NULL;
static int (*phwScnDisable)(int32_t) = NULL;
static int (*phwScnUltraCfg)(int32_t, int32_t, int32_t, int32_t, int32_t, int32_t) = NULL;
static int (*phwSetSysInfo)(int32_t, const char *) = NULL;
static int (*phwperfLockAcq)(int32_t, int32_t, int[], int32_t) = NULL;
static int (*phwperfLockRel)(int32_t) = NULL;

typedef int (*phw_mtk_power_hint)(uint32_t, int32_t);
typedef int (*phw_mtk_cus_power_hint)(uint32_t, int32_t);
typedef int (*phw_query_sys_info)(uint32_t, int32_t);
typedef int (*phw_notify_app_state)(const char *, const char *, int32_t, int32_t, int32_t);
typedef int (*phw_scn_reg)(void);
typedef int (*phw_scn_config)(int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
typedef int (*phw_scn_unreg)(int32_t);
typedef int (*phw_scn_enable)(int32_t, int32_t);
typedef int (*phw_scn_disable)(int32_t);
typedef int (*phw_scn_ultracfg)(int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
typedef int (*phw_set_sysinfo)(int32_t, const char *);
typedef int (*phw_perf_lock_acq)(int32_t, int32_t, int[], int32_t);
typedef int (*phw_perf_lock_rel)(int32_t);

static void load_perf_api(void)
{
    void *handle, *func;

    handle = dlopen(PERF_LOCK_LIB_FULL_NAME, RTLD_NOW);
    if (handle == NULL) {
        ALOGE("dlopen error: %s\n", dlerror());
        return;
    }

    func = dlsym(handle, "perf_lock_rel");
    phwperfLockRel = reinterpret_cast<phw_perf_lock_rel>(func);

    if (phwperfLockRel == NULL) {
        ALOGE("phwperfLockRel error: %s\n", dlerror());
        dlclose(handle);
        return;
    }

    func = dlsym(handle, "perf_lock_acq");
    phwperfLockAcq = reinterpret_cast<phw_perf_lock_acq>(func);

    if (phwperfLockAcq == NULL) {
        ALOGE("phwperfLockAcq error: %s\n", dlerror());
        dlclose(handle);
        return;
    }

}


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

    func = dlsym(handle, "PowerHal_Wrap_mtkPowerHint");
    phwMtkPowerHint = reinterpret_cast<phw_mtk_power_hint>(func);

    if (phwMtkPowerHint == NULL) {
        ALOGE("phwMtkPowerHint error: %s", dlerror());
        phwMtkPowerHint = NULL;
        dlclose(handle);
        return;
    }

    func = dlsym(handle, "PowerHal_Wrap_mtkCusPowerHint");
    phwMtkCusPowerHint = reinterpret_cast<phw_mtk_cus_power_hint>(func);

    if (phwMtkCusPowerHint == NULL) {
        ALOGE("phwMtkCusPowerHint error: %s", dlerror());
        phwMtkCusPowerHint = NULL;
        dlclose(handle);
        return;
    }

    func = dlsym(handle, "PowerHal_Wrap_querySysInfo");
    phwQuerySysInfo = reinterpret_cast<phw_query_sys_info>(func);

    if (phwQuerySysInfo == NULL) {
        ALOGE("phwQuerySysInfo error: %s", dlerror());
        phwQuerySysInfo = NULL;
        dlclose(handle);
        return;
    }

    func = dlsym(handle, "PowerHal_Wrap_notifyAppState");
    phwNotifyAppState = reinterpret_cast<phw_notify_app_state>(func);

    if (phwNotifyAppState == NULL) {
        ALOGE("phwNotifyAppState error: %s", dlerror());
        phwNotifyAppState = NULL;
        dlclose(handle);
        return;
    }

    func = dlsym(handle, "PowerHal_Wrap_scnReg");
    phwScnReg = reinterpret_cast<phw_scn_reg>(func);

    if (phwScnReg == NULL) {
        ALOGE("phwScnReg error: %s", dlerror());
        phwScnReg = NULL;
        dlclose(handle);
        return;
    }

    func = dlsym(handle, "PowerHal_Wrap_scnConfig");
    phwScnConfig = reinterpret_cast<phw_scn_config>(func);

    if (phwScnConfig == NULL) {
        ALOGE("phwScnConfig error: %s", dlerror());
        phwScnConfig = NULL;
        dlclose(handle);
        return;
    }

    func = dlsym(handle, "PowerHal_Wrap_scnUnreg");
    phwScnUnreg = reinterpret_cast<phw_scn_unreg>(func);

    if (phwScnUnreg == NULL) {
        ALOGE("phwScnUnreg error: %s", dlerror());
        phwScnUnreg = NULL;
        dlclose(handle);
        return;
    }

    func = dlsym(handle, "PowerHal_Wrap_scnEnable");
    phwScnEnable = reinterpret_cast<phw_scn_enable>(func);

    if (phwScnEnable == NULL) {
        ALOGE("phwScnEnable error: %s", dlerror());
        phwScnEnable = NULL;
        dlclose(handle);
        return;
    }

    func = dlsym(handle, "PowerHal_Wrap_scnDisable");
    phwScnDisable = reinterpret_cast<phw_scn_disable>(func);

    if (phwScnDisable == NULL) {
        ALOGE("phwScnDisable error: %s", dlerror());
        phwScnDisable = NULL;
        dlclose(handle);
        return;
    }

    func = dlsym(handle, "PowerHal_Wrap_scnUltraCfg");
    phwScnUltraCfg = reinterpret_cast<phw_scn_ultracfg>(func);

    if (phwScnUltraCfg == NULL) {
        ALOGE("phwScnUltraCfg error: %s", dlerror());
        phwScnUltraCfg = NULL;
        dlclose(handle);
        return;
    }

    func = dlsym(handle, "PowerHal_Wrap_setSysInfo");
    phwSetSysInfo = reinterpret_cast<phw_set_sysinfo>(func);

    if (phwSetSysInfo == NULL) {
        ALOGE("phwSetSysInfo error: %s", dlerror());
        phwSetSysInfo = NULL;
        dlclose(handle);
        return;
    }

    load_perf_api();
}

static int
com_mediatek_powerhalwrapper_PowerHalWrapper_nativeMtkPowerHint(JNIEnv* /* env */,
                                          jobject /* thiz */, jint hint, jint data)
{
    if (!inited)
        init();

    //ALOGI("MtkPowerHint");
    if (phwMtkPowerHint)
        return phwMtkPowerHint(hint, data);

    return -1;
}

static int
com_mediatek_powerhalwrapper_PowerHalWrapper_nativeMtkCusPowerHint(JNIEnv* /* env */,
                                             jobject /* thiz */, jint hint, jint data)
{
    if (!inited)
        init();

    //ALOGI("MtkCusPowerHint");
    if (phwMtkCusPowerHint)
        return phwMtkCusPowerHint(hint, data);

    return -1;
}

static int
com_mediatek_powerhalwrapper_PowerHalWrapper_nativeQuerySysInfo(JNIEnv* /* env */,
                                          jobject /* thiz */, jint cmd, jint param)
{
    if (!inited)
        init();

    //ALOGI("QuerySysInfo");
    if (phwQuerySysInfo)
        return phwQuerySysInfo(cmd, param);

    return -1;
}

static int
com_mediatek_powerhalwrapper_PowerHalWrapper_nativeNotifyAppState(JNIEnv *env,
                   jobject /* thiz */, jstring packname, jstring actname, jint pid, jint status, jint uid)
{
    int ret = -1;

    if (!inited)
        init();

    //ALOGI("NotifyAppState");
    if (phwNotifyAppState) {
        const char *nativePack = (packname) ? env->GetStringUTFChars(packname, 0) : NULL;
        const char *nativeAct = (actname) ? env->GetStringUTFChars(actname, 0) : NULL;

        ret = phwNotifyAppState(nativePack, nativeAct, pid, status, uid);

        if (nativePack) {
            env->ReleaseStringUTFChars(packname, nativePack);
        }
        if (nativeAct) {
            env->ReleaseStringUTFChars(actname, nativeAct);
        }
    }

    return ret;
}

static int
com_mediatek_powerhalwrapper_PowerHalWrapper_nativeScnReg(JNIEnv* /* env */, jobject /* thiz */)
{
    if (!inited)
        init();

    ALOGI("ScnReg");
    if (phwScnReg)
        return phwScnReg();

    return -1;
}

static int
com_mediatek_powerhalwrapper_PowerHalWrapper_nativeScnConfig(JNIEnv* /* env */, jobject /* thiz */,
                         jint hdl, jint cmd, jint param_1, jint param_2, jint param_3, jint param_4)
{
    if (!inited)
        init();

    ALOGI("ScnConfig");
    if (phwScnConfig)
        return phwScnConfig(hdl, cmd, param_1, param_2, param_3, param_4);

    return -1;
}

static int
com_mediatek_powerhalwrapper_PowerHalWrapper_nativeScnUnreg(JNIEnv* /* env */,
                                                  jobject /* thiz */, jint hdl)
{
    if (!inited)
        init();

    ALOGI("ScnUnreg");
    if (phwScnUnreg)
        return phwScnUnreg(hdl);

    return -1;
}

static int
com_mediatek_powerhalwrapper_PowerHalWrapper_nativeScnEnable(JNIEnv* /* env */,
                                     jobject /* thiz */, jint hdl, jint timeout)
{
    if (!inited)
        init();

    ALOGI("ScnEnable");
    if (phwScnEnable)
        return phwScnEnable(hdl, timeout);

    return -1;
}

static int
com_mediatek_powerhalwrapper_PowerHalWrapper_nativeScnDisable(JNIEnv* /* env */,
                                                    jobject /* thiz */, jint hdl)
{
    if (!inited)
        init();

    ALOGI("ScnDisable");
    if (phwScnDisable)
        return phwScnDisable(hdl);

    return -1;
}

static int
com_mediatek_powerhalwrapper_PowerHalWrapper_nativeScnUltraCfg(JNIEnv* /* env */, jobject /* thiz */,
                         jint hdl, jint ultracmd, jint param_1, jint param_2, jint param_3, jint param_4)
{
    if (!inited)
        init();

    ALOGI("ScnUltraCfg");
    if (phwScnUltraCfg)
        return phwScnUltraCfg(hdl, ultracmd, param_1, param_2, param_3, param_4);

    return -1;
}

static int
com_mediatek_powerhalwrapper_PowerHalWrapper_nativeSetSysInfo(JNIEnv *env,
                   jobject /* thiz */, jstring data, jint type)
{
    int ret = -1;

    if (!inited)
        init();

    ALOGI("SetSysInfo");
    if (phwSetSysInfo) {
        const char *nativedata = (data) ? env->GetStringUTFChars(data, 0) : NULL;

        ret = phwSetSysInfo(type, nativedata);
        if (nativedata) {
            env->ReleaseStringUTFChars(data, nativedata);
        }
    }

    return ret;
}

static int
com_mediatek_powerhalwrapper_PowerHalWrapper_nativePerfLockAcq(JNIEnv* env, jobject /* thiz */,
                         jint hdl, jint duration, jintArray list)
{
    jint listlen = env->GetArrayLength(list);
    jint buf[listlen];
    int handle = 0;
    env->GetIntArrayRegion(list, 0, listlen, buf);

    if (!inited)
        init();

    ALOGI("phwperfLockAcq");
    if (phwperfLockAcq)
        handle = phwperfLockAcq(hdl, duration, buf, listlen);

    return handle;
}

static int
com_mediatek_powerhalwrapper_PowerHalWrapper_nativePerfLockRel(JNIEnv* /* env */, jobject /* thiz */,
                         jint hdl)
{
    if (!inited)
        init();

    ALOGI("perfLockRel");
    if (phwperfLockRel)
        return phwperfLockRel(hdl);

    return -1;
}


static JNINativeMethod sMethods[] = {
    {"nativeMtkPowerHint",   "(II)I",
            (int *)com_mediatek_powerhalwrapper_PowerHalWrapper_nativeMtkPowerHint},
    {"nativeMtkCusPowerHint",   "(II)I",
            (int *)com_mediatek_powerhalwrapper_PowerHalWrapper_nativeMtkCusPowerHint},
    {"nativeQuerySysInfo",   "(II)I",
            (int *)com_mediatek_powerhalwrapper_PowerHalWrapper_nativeQuerySysInfo},
    {"nativeNotifyAppState",   "(Ljava/lang/String;Ljava/lang/String;III)I",
            (int *)com_mediatek_powerhalwrapper_PowerHalWrapper_nativeNotifyAppState},
    {"nativeScnReg",   "()I",
            (int *)com_mediatek_powerhalwrapper_PowerHalWrapper_nativeScnReg},
    {"nativeScnConfig",   "(IIIIII)I",
            (int *)com_mediatek_powerhalwrapper_PowerHalWrapper_nativeScnConfig},
    {"nativeScnUnreg",   "(I)I",
            (int *)com_mediatek_powerhalwrapper_PowerHalWrapper_nativeScnUnreg},
    {"nativeScnEnable",   "(II)I",
            (int *)com_mediatek_powerhalwrapper_PowerHalWrapper_nativeScnEnable},
    {"nativeScnDisable",   "(I)I",
            (int *)com_mediatek_powerhalwrapper_PowerHalWrapper_nativeScnDisable},
    {"nativeScnUltraCfg",   "(IIIIII)I",
            (int *)com_mediatek_powerhalwrapper_PowerHalWrapper_nativeScnUltraCfg},
    {"nativeSetSysInfo",    "(Ljava/lang/String;I)I",
            (int *)com_mediatek_powerhalwrapper_PowerHalWrapper_nativeSetSysInfo},
    {"nativePerfLockAcq",   "(II[I)I",
            (int *)com_mediatek_powerhalwrapper_PowerHalWrapper_nativePerfLockAcq},
    {"nativePerfLockRel",   "(I)I",
            (int *)com_mediatek_powerhalwrapper_PowerHalWrapper_nativePerfLockRel},
};

int register_com_mediatek_powerhalwrapper_PowerHalWrapper(JNIEnv* env)
{
    jclass clazz = env->FindClass("com/mediatek/powerhalwrapper/PowerHalWrapper");

    if (env->RegisterNatives(clazz, sMethods, NELEM(sMethods)) < 0) {
        ALOGE("RegisterNatives error");
        return JNI_ERR;
    }

    return JNI_OK;
}

extern int register_com_mediatek_amsAal_AalUtils(JNIEnv*);

extern "C" jint JNI_OnLoad(JavaVM* vm, void* /* reserved */)
{
    JNIEnv* env = NULL;
    jint result = -1;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        return result;
    }

    register_com_mediatek_powerhalwrapper_PowerHalWrapper(env);
    register_com_mediatek_amsAal_AalUtils(env);

    return JNI_VERSION_1_4;
}

}
