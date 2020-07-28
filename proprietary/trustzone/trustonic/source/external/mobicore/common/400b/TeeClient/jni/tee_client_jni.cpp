/*
 * Copyright (c) 2015-2017 TRUSTONIC LIMITED
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

#include <limits.h>
#include <dlfcn.h>
#include <jni.h>

#include <MobiCoreDriverApi.h>

#include "dynamic_log.h"
#include "common.h"

using namespace trustonic;
static Common& common = Common::getInstance();
JavaVM* gJavaVm = nullptr;

jint JNI_OnLoad(JavaVM* vm, void* /*reserved*/) {
    LOG_D("JNI_OnLoad succeed");
    gJavaVm = vm;
    return JNI_VERSION_1_6;
}

void JNI_OnUnload(JavaVM* /*vm*/, void* /*reserved*/) {
    LOG_D("JNI_OnUnLoad");
}

extern "C" JNIEXPORT jint JNICALL
Java_com_trustonic_teeclient_TeeClientJni_jniMcOpenDevice(JNIEnv* /*env*/, jobject /*obj*/, jint deviceId, jobject applicationContext) {
    common.TEEC_TT_RegisterPlatformContext(gJavaVm, applicationContext);
    jint res = static_cast<jint>(common.mcOpenDevice(deviceId));
    LOG_D("mcOpenDevice deviceId(%d) returned 0x%x", static_cast<uint32_t>(deviceId), res);
    return res;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_trustonic_teeclient_TeeClientJni_jniMcCloseDevice(JNIEnv* /*env*/, jobject /*obj*/, jint deviceId) {
    jint res = static_cast<jint>(common.mcCloseDevice(deviceId));
    LOG_D("mcCloseDevice deviceId(%d) returned 0x%x", static_cast<uint32_t>(deviceId), res);
    return res;
}
