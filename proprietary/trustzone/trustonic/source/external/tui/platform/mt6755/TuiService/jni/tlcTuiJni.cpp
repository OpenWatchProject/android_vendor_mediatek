/*
 * Copyright (c) 2013-2015 TRUSTONIC LIMITED
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
#include <string.h>
#include <stdint.h>
#include <jni.h>
#include <android/log.h>

#include "tui_ioctl.h"

#include "tlcTui.h"
#include "tlcTuiJni.h"

#define LOG_TAG "TlcTuiJni"
#include "log.h"

/* See for more help about JNI:
 * http://java.sun.com/docs/books/jni/html/jniTOC.html
 * http://java.sun.com/developer/onlineTraining/Programming/JDCBook/jni.html
 * http://developer.android.com/training/articles/perf-jni.html
 */

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

JNIEnv *gEnv = NULL;
JavaVM *gVm = NULL;
jclass gTuiTlcWrapperClass = NULL;

static uint32_t setByteArrayStaticField(const char * field, const void *ptr,
                                        uint32_t length, jclass cls) {
    jfieldID    fid = NULL; /* store the field ID */
    jbyteArray  bytearray = NULL;

    LOG_I("setByteArrayStaticField: {%s}, length=%u", field, length);
    /* Look for the static field in class */
    fid = gEnv->GetStaticFieldID(cls, field, "[B");
    if (fid == NULL) {
        LOG_E("setByteArrayStaticField: cannot find field [%s]", field);
        return TUI_JNI_ERROR; /* field not found */
    }
    bytearray = gEnv->NewByteArray(length); /* TODO : free */
    if (bytearray == NULL) {
        LOG_E("setByteArrayStaticField: not enough memory");
        return TUI_JNI_ERROR; /* out of memory */
    }
    gEnv->SetByteArrayRegion(bytearray, (jint)0, (jint)length, (jbyte*)ptr);
    gEnv->SetStaticObjectField(cls, fid, bytearray);

    return TUI_JNI_OK;
}

static uint32_t setByteArrayField(const char * field, const void *ptr,
                                  uint32_t length, jclass cls,
                                  jobject instance) {
    jfieldID    fid = NULL; /* store the field ID */
    jbyteArray  bytearray = NULL;

    LOG_I("setByteArrayField: {%s}, length=%u", field, length);
    /* Look for the static field in class */
    fid = gEnv->GetFieldID(cls, field, "[B");
    if (fid == NULL) {
        LOG_E("setByteArrayField: cannot find field [%s]", field);
        return TUI_JNI_ERROR; /* field not found */
    }
    bytearray = gEnv->NewByteArray(length); /* TODO : free */
    if (bytearray == NULL) {
        LOG_E("setByteArrayField: not enough memory");
        return TUI_JNI_ERROR; /* out of memory */
    }
    gEnv->SetByteArrayRegion(bytearray, (jint)0, (jint)length, (jbyte*)ptr);
    gEnv->SetObjectField(instance, fid, bytearray);
    gEnv->DeleteLocalRef(bytearray);

    return TUI_JNI_OK;
}

uint32_t tlcStartTuiSession(void) {

    uint32_t    nResult = TUI_JNI_ERROR;
    jmethodID   midStartTuiSession = NULL;

    if (gTuiTlcWrapperClass == NULL) {
        LOG_E("tlcStartTuiSession: Missing parameter gTuiTlcWrapperClass");
        nResult = TUI_JNI_ERROR;
        goto exit;
    }

    /* Attach the thread to the VM */
    if (gVm->AttachCurrentThread(&gEnv, 0) != JNI_OK) {
        LOG_E("tlcStartTuiSession: AttachCurrentThread failed");
        return 1;
    }

    if (gEnv != NULL) {
        /* ------------------------------------------------------------- */
        /* Get the method ID for startTuiSession */
        midStartTuiSession = gEnv->GetStaticMethodID(gTuiTlcWrapperClass,
                                                      "startTuiSession",
                                                      "()I");
        if (midStartTuiSession == NULL) {
            LOG_E("tlcStartTuiSession: Method startTuiSession not found");
            nResult = TUI_JNI_ERROR;
            goto exit;
        }
        /* Call startTuiSession */
        nResult = (uint32_t) gEnv->CallStaticIntMethod(gTuiTlcWrapperClass, midStartTuiSession);
        /* ------------------------------------------------------------- */
        //nResult = TUI_JNI_OK;
        if (nResult != TUI_JNI_OK) {
            LOG_E("tlcStartTuiSession: Method startTuiSession error: %u", nResult);
            goto exit;
        }
    } else {
        LOG_E("tlcStartTuiSession: gEnv is null!");
        nResult = TUI_JNI_ERROR;
        goto exit;
    }

    exit: if (gEnv != NULL) {
        if (gEnv->ExceptionCheck()) {
            LOG_E("tlcStartTuiSession: Java exception");
            gEnv->ExceptionClear();
        }
    } else {
        LOG_E("tlcStartTuiSession: exit gEnv is NULL");
    }

    if (gVm->DetachCurrentThread() != 0) {
        LOG_E("tlcStartTuiSession: DetachCurrentThread failed");
    }
    return nResult;
}

uint32_t tlcFinishTuiSession(void) {

    uint32_t    nResult = TUI_JNI_ERROR;
    jmethodID   midFinishTuiSession = NULL;

    if (gTuiTlcWrapperClass == NULL) {
        LOG_E("tlcFinishTuiSession: Missing parameter gTuiTlcWrapperClass");
        nResult = TUI_JNI_ERROR;
        goto exit;
    }

    /* Attach the thread to the VM */
    if (gVm->AttachCurrentThread(&gEnv, 0) != JNI_OK) {
        LOG_E("tlcFinishTuiSession: AttachCurrentThread failed");
        return 1;
    }

    if (gEnv != NULL) {
        /* ------------------------------------------------------------- */
        /* Get the method ID for finishTuiSession */
        midFinishTuiSession = gEnv->GetStaticMethodID(gTuiTlcWrapperClass,
                                                       "finishTuiSession",
                                                       "()V");
        if (midFinishTuiSession == NULL) {
            LOG_E("tlcFinishTuiSession: Method finishTuiSession not found");
            nResult = TUI_JNI_ERROR;
            goto exit;
        }
        /* Call finishTuiSession */
        gEnv->CallStaticVoidMethod(gTuiTlcWrapperClass, midFinishTuiSession);
        /* ------------------------------------------------------------- */
        nResult = TUI_JNI_OK;
    } else {
        LOG_E("tlcFinishTuiSession: gEnv is null!");
        nResult = TUI_JNI_ERROR;
        goto exit;
    }

    exit: if (gEnv != NULL) {
        if (gEnv->ExceptionCheck()) {
            LOG_E("tlcFinishTuiSession: Java exception");
            gEnv->ExceptionClear();
        }
    } else {
        LOG_E("tlcFinishTuiSession: exit gEnv is NULL");
    }

    if (gVm->DetachCurrentThread() != 0) {
        LOG_E("tlcFinishTuiSession: DetachCurrentThread failed");
    }

    return nResult;
}


EXTERN_C JNIEXPORT bool JNICALL
Java_com_trustonic_tuiservice_TuiTlcWrapper_startTlcTui(JNIEnv */*env*/,
        jobject /*obj*/) {

    bool ret = false;

    LOG_I("calling tlcLaunch()");
    ret = tlcLaunch();
    if(!ret) {
        LOG_E("tlcLaunch: failed to start TlcTui!");
    }

    return ret;
}

EXTERN_C JNIEXPORT bool JNICALL
Java_com_trustonic_tuiservice_TuiTlcWrapper_notifyEvent(JNIEnv */*env*/,
        jobject /*obj*/, jint eventType) {

    bool ret = false;

    LOG_I("calling tlcNotifyEvent()");
    ret = tlcNotifyEvent(eventType);
    if (!ret) {
        LOG_E("tlcNotifyEvent: failed to notify an event!");
    }

    return ret;
}

jint JNI_OnLoad(JavaVM *vm, void */*reserved*/) {
    JNIEnv  *env = NULL;
    jclass  TuiTlcWrapperClass = NULL;

    LOG_I("JNI_OnLoad");

    if (vm->GetEnv((void **)&env, JNI_VERSION_1_6) != JNI_OK) {
        LOG_E("JNI_OnLoad: GetEnv failed!");
        return -1;
    }

    TuiTlcWrapperClass = env->FindClass(
            "com/trustonic/tuiservice/TuiTlcWrapper");
    if (TuiTlcWrapperClass == NULL) {
        LOG_E("JNI_OnLoad: FindClass on class TuiTlcWrapperClass failed!");
        return -1;
    }

    LOG_I("JNI_OnLoad: TuiTlcWrapperClass = %p", TuiTlcWrapperClass);

    /* Cache the TlcTuiWrapper class in a global reference */
    /* Use the cached gTuiTlcWrapperClass.
     * As we called the AttachCurrentThread to get the java environnement from
     * a native thread, the FindClass will always fail. This is a ClassLoader issue.
     * This call (AttachCurrentThread) changes the call stack, so when the FindClass
     * try to start the class search in the class loader associated with this method,
     * FindClass find the ClassLoader associated with the a wrong class, so FindClass fails.*/
    gTuiTlcWrapperClass = (jclass)env->NewGlobalRef(TuiTlcWrapperClass);

    /* Cache the javaVM to get back a JNIEnv reference from native code*/
    gVm = vm;
    LOG_I("JNI_OnLoad: gVm = %p vm = %p", gVm, vm);

    return JNI_VERSION_1_6;
}

