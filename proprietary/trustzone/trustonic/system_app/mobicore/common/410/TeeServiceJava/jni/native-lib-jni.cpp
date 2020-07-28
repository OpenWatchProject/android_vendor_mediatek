/*
 * Copyright (c) 2013-2018 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <mutex>
#include <string>
#include <thread>

#include <jni.h>

#define LOG_TAG "TeeServiceNative"
#include <utils/Log.h>

#include "vendor/trustonic/tee/1.0/ITee.h"
#include "vendor/trustonic/tee/1.0/ITeeCallback.h"
#include "TeeCallback.h"
#include "teeserviceserver.h"

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

#define TUI_JNI_OK              0
#define TUI_JNI_ERROR           1
#define TUI_JNI_ERROR_IN_JAVA   2

using namespace vendor::trustonic::tee::V1_0;

static JNIEnv* gEnv;
static JavaVM* gVm;
static std::mutex serverThreadMutex;
static std::thread serverThread;

static void serverThreadFunction(jobject javaServiceObject) {
    // Attach the thread to the VM
    if (gVm->AttachCurrentThread(&gEnv, 0) != JNI_OK) {
        ALOGE("%s: AttachCurrentThread failed", __func__);
        return;
    }

    ALOGD("calling teeServerService()");
    int ret = teeserviceserver(gEnv, javaServiceObject);
    if(!ret) {
        ALOGE("%s: failed to start teeServerService!", __func__);
    }
}

jint JNI_OnLoad(JavaVM *vm, void */*reserved*/) {
    ALOGD("JNI_OnLoad");
    JNIEnv* env = nullptr;
    if (vm->GetEnv((void **)&env, JNI_VERSION_1_6) != JNI_OK) {
        ALOGE("JNI_OnLoad: GetEnv failed!");
        return -1;
    }

    /* Cache the javaVM to get back a JNIEnv reference from native code*/
    gVm = vm;
    ALOGE("JNI_OnLoad: gVm = %p vm = %p", gVm, vm);

    return JNI_VERSION_1_6;
}

EXTERN_C JNIEXPORT bool JNICALL
Java_com_trustonic_teeservice_TeeService_startTeeServiceServer(
        JNIEnv* env,
        jclass /*clazz*/,
        jobject thiz) {
    std::lock_guard<std::mutex> lock(serverThreadMutex);
    if (serverThread.joinable()) {
        ALOGW("Server thread is already running.");
        return false;
    }

    jobject teeService = env->NewGlobalRef(thiz);
    serverThread = std::thread(serverThreadFunction, teeService);
    return true;
}

EXTERN_C JNIEXPORT bool JNICALL
Java_com_trustonic_teeservice_TeeService_registerTeeCallback(
        JNIEnv* /*env*/,
        jobject /*obj*/) {
    
    ::android::sp<ITeeCallback> cb = new TeeCallback();
    ::android::sp<ITee> service_ = ITee::getService();
    service_->registerTeeCallback(cb);
    return true;
}
