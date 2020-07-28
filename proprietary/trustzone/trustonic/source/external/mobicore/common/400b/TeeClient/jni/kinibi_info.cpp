/*
 * Copyright (c) 2016-2018 TRUSTONIC LIMITED
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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <jni.h>

#include "mcVersionHelper.h"
#include "mcVersionInfo.h"
#include "mcSuid.h"

#include "dynamic_log.h"
#include "tBaseInfo.h"

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

static void formatSuid(unsigned char *suid, char *data) {
    char *s = data;
    for(size_t i = 0; i < sizeof(mcSuid_t); i++) {
        char hex[3];
        sprintf(hex, "%02x ", suid[i]);
        s = strncat(s, hex, 3);
    }
}

EXTERN_C JNIEXPORT jint JNICALL
Java_com_trustonic_teeclient_kinibichecker_KinibiCheckerJni_getInfo(
        JNIEnv *env,
        jobject obj,
        jobject applicationContext,
        jint cmpVersionMajor,
        jint cmpVersionMinor) {
    jint ret = 0;
    char data[sizeof(mcVersionInfo_t)];
    mcSuid_t suid;
    mcVersionInfo_t versionInfo;

    memset(&versionInfo, 0, sizeof(versionInfo));
    memset(&suid, 0, sizeof(suid));

    LOG_D("Cmp interface: %d.%d; %s; %s", cmpVersionMajor,
            cmpVersionMinor, __FILE__, __func__);

    JavaVM* jvm = nullptr;
    env->GetJavaVM(&jvm);
    TEEC_TT_RegisterPlatformContext(jvm, applicationContext);

    if(MC_MAKE_VERSION(cmpVersionMajor, cmpVersionMinor) <= MC_MAKE_VERSION(2, 0)) {
        ret = getTlCmInfo_2_0(&suid, &versionInfo);
    } else {
        ret = getTlCmInfo(&suid, &versionInfo);
    }

    if(ret != 0)
    {
        if(ret != MC_DRV_OK)
        {
            LOG_E("Failed: %d; %s; %s", ret, __FILE__, __func__);
            return ret;
        }
    }

    jclass clazz = env->GetObjectClass(obj);
    jfieldID field;
    jstring jString;

    // Suid.
    field = env->GetFieldID(clazz, "mSuid", "Ljava/lang/String;");
    {
        char tmp[3*sizeof(mcSuid_t)+1];
        tmp[0] = '\0';
        formatSuid((unsigned char*)&suid, tmp);
        jString = env->NewStringUTF(tmp);
        env->SetObjectField(obj, field, jString);
        LOG_D("mSuid %s; %s; %s", tmp, __FILE__, __func__);
    }

    // ProductId.
    field = env->GetFieldID(clazz, "mProductId", "Ljava/lang/String;");
    memset(data, 0, sizeof(data));
    memcpy(data, &versionInfo.productId, MC_PRODUCT_ID_LEN);
    jString = env->NewStringUTF(data);
    env->SetObjectField(obj, field, jString);
    LOG_D("mProductId %s; %s; %s", data, __FILE__, __func__);

    // Mci.
    field = env->GetFieldID(clazz, "mMci", "Ljava/lang/String;");
    memset(data, 0, sizeof(data));
    sprintf(data, "%d.%d", MC_GET_MAJOR_VERSION(versionInfo.versionMci),
            MC_GET_MINOR_VERSION(versionInfo.versionMci));
    jString = env->NewStringUTF(data);
    env->SetObjectField(obj, field, jString);
    LOG_D("mMci %s; %s; %s", data, __FILE__, __func__);

    // So.
    field = env->GetFieldID(clazz, "mSo", "Ljava/lang/String;");
    memset(data, 0, sizeof(data));
    sprintf(data, "%d.%d", MC_GET_MAJOR_VERSION(versionInfo.versionSo),
            MC_GET_MINOR_VERSION(versionInfo.versionSo));
    jString = env->NewStringUTF(data);
    env->SetObjectField(obj, field, jString);
    LOG_D("mSo %s; %s; %s", data, __FILE__, __func__);

    // Mclf.
    field = env->GetFieldID(clazz, "mMclf", "Ljava/lang/String;");
    memset(data, 0, sizeof(data));
    sprintf(data, "%d.%d", MC_GET_MAJOR_VERSION(versionInfo.versionMclf),
            MC_GET_MINOR_VERSION(versionInfo.versionMclf));
    jString = env->NewStringUTF(data);
    env->SetObjectField(obj, field, jString);
    LOG_D("mMclf %s; %s; %s", data, __FILE__, __func__);

    // Container.
    field = env->GetFieldID(clazz, "mContainer", "Ljava/lang/String;");
    memset(data, 0, sizeof(data));
    sprintf(data, "%d.%d", MC_GET_MAJOR_VERSION(versionInfo.versionContainer),
            MC_GET_MINOR_VERSION(versionInfo.versionContainer));
    jString = env->NewStringUTF(data);
    env->SetObjectField(obj, field, jString);
    LOG_D("mContainer %s; %s; %s", data, __FILE__, __func__);

    // McConfig.
    field = env->GetFieldID(clazz, "mMcConfig", "Ljava/lang/String;");
    memset(data, 0, sizeof(data));
    sprintf(data, "%d.%d", MC_GET_MAJOR_VERSION(versionInfo.versionMcConfig),
            MC_GET_MINOR_VERSION(versionInfo.versionMcConfig));
    jString = env->NewStringUTF(data);
    env->SetObjectField(obj, field, jString);
    LOG_D("mMcConfig %s; %s; %s", data, __FILE__, __func__);

    // TlApi.
    field = env->GetFieldID(clazz, "mTlApi", "Ljava/lang/String;");
    memset(data, 0, sizeof(data));
    sprintf(data, "%d.%d", MC_GET_MAJOR_VERSION(versionInfo.versionTlApi),
            MC_GET_MINOR_VERSION(versionInfo.versionTlApi));
    jString = env->NewStringUTF(data);
    env->SetObjectField(obj, field, jString);
    LOG_D("mTlApi %s; %s; %s", data, __FILE__, __func__);

    // DrApi.
    field = env->GetFieldID(clazz, "mDrApi", "Ljava/lang/String;");
    memset(data, 0, sizeof(data));
    sprintf(data, "%d.%d", MC_GET_MAJOR_VERSION(versionInfo.versionDrApi),
            MC_GET_MINOR_VERSION(versionInfo.versionDrApi));
    jString = env->NewStringUTF(data);
    env->SetObjectField(obj, field, jString);
    LOG_D("mDrApi %s; %s; %s", data, __FILE__, __func__);

    // Cmp.
    field = env->GetFieldID(clazz, "mCmp", "Ljava/lang/String;");
    memset(data, 0, sizeof(data));
    if( MC_GET_MAJOR_VERSION(versionInfo.versionCmp)==0 && MC_GET_MINOR_VERSION(versionInfo.versionCmp)==0)
    {
        sprintf(data, "not found");
    }
    else
    {
        sprintf(data, "%d.%d", MC_GET_MAJOR_VERSION(versionInfo.versionCmp),
                MC_GET_MINOR_VERSION(versionInfo.versionCmp));
    }
    jString = env->NewStringUTF(data);
    env->SetObjectField(obj, field, jString);
    LOG_D("mCmp %s; %s; %s", data, __FILE__, __func__);

    return ret;
}
