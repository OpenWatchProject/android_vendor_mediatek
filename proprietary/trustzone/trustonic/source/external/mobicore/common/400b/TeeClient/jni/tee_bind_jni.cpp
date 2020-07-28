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
#include "tee_bind_jni.h"

#include "dynamic_log.h"

struct JavaProcess::Impl {
    std::string package_name;
    std::string starter_class;
    std::string starter_class_path;
    bool        is_binded = false;
    JavaVM*     jvm = nullptr;
    bool        thread_attached = false;

    enum class BindResult {
        OK,
        OVERLOAD,
        FAILURE
    };

    Impl(
            const std::string& pn,
            const std::string& sc):
        package_name(pn), starter_class(sc) {
        starter_class_path = package_name + "/." + starter_class;
    }

    int setup(JNIEnv** env, jclass* clazz) {
        LOG_I("Bind service %s", package_name.c_str());
        if (!jvm) {
            LOG_E("No JVM");
            return -1;
        }

        switch (jvm->GetEnv((void**)env, JNI_VERSION_1_6)) {
            case JNI_OK:
                break;
            case JNI_EDETACHED:
                // Thread not attached, attach it
                if (jvm->AttachCurrentThread(env, NULL) != 0) {
                    LOG_E("Failed to attach");
                    return -1;
                }
                thread_attached = true;
                break;
            default:
                LOG_E("GetEnv failed");
                return -1;
        }

        // Get TeeClient java class
        *clazz = (*env)->FindClass("com/trustonic/teeclient/TeeBind");
        if (*clazz == NULL) {
            LOG_E("Cannot find TeeClient java class");
            teardown(env, clazz);
            return -1;
        }

        return 0;
    }

    void teardown(JNIEnv** env, jclass* clazz) {
        if (clazz != NULL) {
            (*env)->DeleteLocalRef(*clazz);
        }
        if (thread_attached) {
            jvm->DetachCurrentThread();
            thread_attached = false;
        }
    }

    int registerContext(JavaVM* jvm_in, jobject context) {
        int rc = -1;
        jvm = jvm_in;
        JNIEnv* env = nullptr;
        jclass clazz;
        if (setup(&env, &clazz)) {
            // Log done below
            return rc;
        }

        jmethodID registerContext = env->GetStaticMethodID(clazz, "registerContext",
                                    "(Landroid/content/Context;)V");
        if (registerContext == NULL) {
            LOG_E("Failed to get static method 'registerContext' from TeeClient");
        } else {
            env->CallStaticVoidMethod(clazz, registerContext, context);
            LOG_D("Context registered successfully, java initialization complete");
            rc = 0;
        }

        teardown(&env, &clazz);
        return rc;
    }

    void broadcastIntent(const std::string& action) {
        JNIEnv* env = nullptr;
        jclass clazz;
        if (setup(&env, &clazz)) {
            // Log done below
            return;
        }

        jmethodID broadcastIntent = env->GetStaticMethodID(clazz, "broadcastIntent",
                                    "(Ljava/lang/String;)V");
        if (broadcastIntent == NULL) {
            LOG_E("Failed to get static method 'broadcastIntent' from TeeClient");
        } else {
            jstring jAction = env->NewStringUTF(action.c_str());
            env->CallStaticVoidMethod(clazz, broadcastIntent, jAction);
            env->DeleteLocalRef(jAction);
        }

        teardown(&env, &clazz);
    }
};

JavaProcess::JavaProcess(
        const std::string& package_name,
        const std::string& starter_class):
    // Missing std::make_unique
    pimpl_(new Impl(package_name, starter_class)) {
}

JavaProcess::~JavaProcess() {
    if (pimpl_->is_binded) {
        LOG_I("Unbind service %s", pimpl_->package_name.c_str());
        unbind();
    }
}

bool JavaProcess::bind(
        JavaVM* jvm,
        jobject javaContext,
        bool send_restart) {
    if (pimpl_->registerContext(jvm, javaContext)) {
        errno = ENXIO;
        return false;
    }

    JNIEnv* env = nullptr;
    jclass clazz;
    if (pimpl_->setup(&env, &clazz)) {
        // Log done in setup
        return false;
    }

    Impl::BindResult rc = Impl::BindResult::FAILURE;
    jmethodID bind = env->GetStaticMethodID(clazz, "bind", "(Ljava/lang/String;)I");
    if (bind == NULL) {
        LOG_E("Failed to get static method 'bind' from TeeClient");
    } else {
        jstring jStarterClass = env->NewStringUTF(pimpl_->starter_class_path.c_str());
        jint ret = env->CallStaticIntMethod(clazz, bind, jStarterClass);
        env->DeleteLocalRef(jStarterClass);

        rc = static_cast<Impl::BindResult>(ret);
        if (rc == Impl::BindResult::FAILURE) {
            errno = ECONNREFUSED;
        } else {
            pimpl_->is_binded = true;
        }
    }

    pimpl_->teardown(&env, &clazz);
    if ((rc == Impl::BindResult::FAILURE) && send_restart) {
        // Tui Service binding has been disabled on SSMG GS7 and GS8.
        // Instead, the Client Application should do send a "restart"
        // intent
        pimpl_->broadcastIntent(pimpl_->package_name + ".action.restart");
    }

    return rc == Impl::BindResult::OK;
}

void JavaProcess::unbind() {
    JNIEnv* env = nullptr;
    jclass clazz;
    if (pimpl_->setup(&env, &clazz)) {
        // Log done in setup
        return;
    }
    jmethodID unbind = env->GetStaticMethodID(clazz, "unbind",
                       "(Ljava/lang/String;)I");
    if (unbind == NULL) {
        LOG_E("Cannot get method ID for TeeClient's \"unbind\" java method");
    } else {
        jstring jStarterClass = env->NewStringUTF(pimpl_->starter_class_path.c_str());
        jint ret = env->CallStaticIntMethod(clazz, unbind, jStarterClass);
        env->DeleteLocalRef(jStarterClass);

        Impl::BindResult rc = static_cast<Impl::BindResult>(ret);
        if (rc == Impl::BindResult::OK) {
            pimpl_->is_binded = false;
        }
    }
    pimpl_->teardown(&env, &clazz);
}
