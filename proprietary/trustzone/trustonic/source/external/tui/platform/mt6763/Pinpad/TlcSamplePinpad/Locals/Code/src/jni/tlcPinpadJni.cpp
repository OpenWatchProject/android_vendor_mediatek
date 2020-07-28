/*
 * Copyright (c) 2013-2016 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#include <cstdlib>
#include <string.h>
#include <stdint.h>
#include <jni.h>

#define LOG_TAG "tlcPinpadJni"
#include "log.h"
#include "tee_client_api.h"

#include "tlcPinpad.h"
#include "960x540/KeyLayout_quartHD.h"
#include "720x1280/KeyLayout_HDReady.h"
#include "1024x600/KeyLayout.h"
#include "1920x1080/KeyLayout_HD.h"
#include "2560x1440/KeyLayout_QHD.h"

#define MAX_FILE_SIZE 200000
#define JAVA_TLC_WRAPPER_CLASS "com/trustonic/samplepinpad/PinpadTLCWrapper"

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
jclass gPinpadTlcWrapper = NULL;

/**
 * @brief Retrieve content from the apk
 *
 * Get the content of an asset in the APK.  This function does not allocates a
 * buffer.  The buffer is allocated by the caller and provided as an input
 * argument, along with the buffer size.
 * Returns the size of the asset if successful. Returns -1 if the asset cannot
 * be loaded.
 * @param assetName
 * @param pContent
 * @param maxSize
 * @return
 */
ssize_t getAssetContent(const char *assetName, uint8_t* pContent, size_t maxSize)
{
    JNIEnv* env = NULL;

    if (gVm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }

    if (NULL == env) {
        return -1;
    }

    /* Get the method ID for method */
    jmethodID mid= env->GetStaticMethodID(gPinpadTlcWrapper, "getAssetContent", "(Ljava/lang/String;)[B");

    if (mid == NULL) {
        printf("%s:%i Method %s not found", __func__, __LINE__, "getAssetContent");
        return -1;
    }

    /* Create a new string for the argument */
    jstring argString = env->NewStringUTF(assetName);
    if (argString == NULL) {
        printf("%s:%i Can't create jstring instance for asset name", __func__, __LINE__);
        return -1;
    }

    /* Call method */
    jbyteArray jData = (jbyteArray) env->CallStaticObjectMethod(gPinpadTlcWrapper,
                                                          mid, argString);
    if (NULL == jData) {
        printf("%s:%i Can't get asset data\n", __func__, __LINE__);
        return -1;
    }

    jsize len = env->GetArrayLength(jData);

    if (NULL != pContent) {
        /*
         * copy the data for the byte array (JAVA) to the uint8_t array (native)
         */
        jbyte *cArray = env->GetByteArrayElements(jData, NULL);
        for (size_t i = 0; i < len && i< maxSize; i++) {
            pContent[i] = cArray[i];
        }
        env->ReleaseByteArrayElements(jData, cArray, JNI_ABORT);
    }

    return len;

}

EXTERN_C JNIEXPORT bool JNICALL
Java_com_trustonic_samplepinpad_PinpadTLCWrapper_tlcPinpadConnect(JNIEnv *env,
        jobject obj, jobject applicationContext) {

    LOG_D("tlcPinpadConnect: calling securePinpadConnect()");
    /*
     * Register the Application Context so that we can bind to the Tui Service
     */
    JavaVM* jvm = NULL;
    env->GetJavaVM(&jvm);
    TEEC_TT_RegisterPlatformContext(jvm, applicationContext);
    return securePinpadConnect();
}

EXTERN_C JNIEXPORT void JNICALL
Java_com_trustonic_samplepinpad_PinpadTLCWrapper_tlcPinpadDisconnect(JNIEnv *env,
        jobject obj) {

    LOG_D("tlcPinpadDisconnect: calling securePinpadDisconnect()");
    securePinpadDisconnect();
}

EXTERN_C JNIEXPORT bool JNICALL
Java_com_trustonic_samplepinpad_PinpadTLCWrapper_tlcPinpadGetResolution(JNIEnv *env,
        jobject obj) {

    bool ret;
    jfieldID fidScreenWidth = NULL,
    fidScreenHeight = NULL;
    uint32_t width = 0,
    height = 0;

    LOG_D("tlcPinpadGetResolution: calling securePinpadGetResolution()");
    ret = securePinpadGetResolution(&width,&height);
    if(ret) {
        if(gPinpadTlcWrapper == NULL) {
            LOG_E("tlcPinpadGetResolution: missing parameter gPinpadTlcWrapper");
            ret = false;
            goto exit;
        }

        gEnv = env;

        if(gEnv != NULL) {
            /* ------------------------------------------------------------- */
            /* Retrieve JNI information about the field width in the PinpadTlcWrapper class */
            fidScreenWidth = gEnv->GetStaticFieldID(gPinpadTlcWrapper, "screenWidth", "I");
            if(fidScreenWidth == NULL) {
                LOG_E("tlcPinpadGetResolution: cannot find field screenWidth");
                ret = false; /* field not found */
                goto exit;
            }
            gEnv->SetStaticIntField(gPinpadTlcWrapper, fidScreenWidth, width);

            /* ------------------------------------------------------------- */
            /* Retrieve JNI information about the field height in the PinpadTlcWrapper class */
            fidScreenHeight = gEnv->GetStaticFieldID(gPinpadTlcWrapper, "screenHeight", "I");
            if(fidScreenHeight == NULL) {
                LOG_E("tlcPinpadGetResolution: cannot find field screenHeight");
                ret = false; /* field not found */
                goto exit;
            }
            gEnv->SetStaticIntField(gPinpadTlcWrapper, fidScreenHeight, height);
            ret = true;
        }

exit:
        if(gEnv != NULL) {
            if(gEnv->ExceptionCheck()) {
                LOG_E("tlcPinpadGetResolution: java exception");
                gEnv->ExceptionClear();
            }
        } else {
            LOG_E("tlcPinpadGetResolution: exit gEnv is NULL");
        }
    }

    return ret;
}

EXTERN_C JNIEXPORT bool JNICALL
Java_com_trustonic_samplepinpad_PinpadTLCWrapper_tlcPinpadProvisionItem(JNIEnv *env,
        jobject obj,
        jint itemID, jstring path,
        jbyteArray item, jint itemLen) {

    bool ret = false;
    uint8_t* nItem = NULL;

    LOG_D("tlcPinpadProvisionItem: itemLen = %d", itemLen);
    //-------------------------------------------------------------------------
    // Provision images to Trusted Application - not secure!
    //  Here we transfer images and pin in plain text to Trusted Application.
    //  Secure provisioning must establish a secure channel between image/pin provider and Trusted Application.
    nItem = (uint8_t*)malloc(MAX_FILE_SIZE);
    if (!nItem) {
        LOG_E("tlcPinpadProvisionItem: could not allocate memory!");
        return false;
    }
    memset(nItem,0,MAX_FILE_SIZE);

    gEnv->GetByteArrayRegion(item, 0, itemLen, (jbyte*) nItem);

    const char* fileLocation = gEnv->GetStringUTFChars(path, 0);

    if(fileLocation != NULL) {
        ret = securePinpadProvisionItem(itemID, fileLocation, nItem, itemLen);
    } else {
        ret = false;
    }
    /* Release native string */
    gEnv->ReleaseStringUTFChars(path, fileLocation);
    /* Release memory of the local buffer for the item */
    free(nItem);

    return ret;
}

EXTERN_C JNIEXPORT jstring JNICALL
Java_com_trustonic_samplepinpad_PinpadTLCWrapper_tlcPinpadVerify(JNIEnv *env,
        jobject obj) {

    char* result;
    LOG_D("tlcPinpadVerify: calling securePinpadVerify()");
    result = (char*)securePinpadVerify();

    return gEnv->NewStringUTF(result);
}

EXTERN_C JNIEXPORT jbyteArray JNICALL
Java_com_trustonic_samplepinpad_PinpadTLCWrapper_tlcPinpadGetNativeLayout(JNIEnv *env,
        jobject obj) {

    jbyteArray layout = NULL;
    gEnv = env;

    if(gEnv == NULL){
        LOG_E("tlcPinpadGetNativeLayout: gEnv is null!");
        return layout;
    }

    /* Allocate an array of byte to contain the layout */
    layout = gEnv->NewByteArray(sizeof(pinpad_layout));
    if(layout == NULL) {
        LOG_E("tlcPinpadGetNativeLayout: NewByteArrey failed!");
        return NULL;
    }
    /* Copy the layout to the array */
    gEnv->SetByteArrayRegion(layout, 0, sizeof(pinpad_layout), (jbyte*)&pinpad_layout);
    return layout;
}

EXTERN_C JNIEXPORT jbyteArray JNICALL
Java_com_trustonic_samplepinpad_PinpadTLCWrapper_tlcPinpadGetNativeLayoutHDReady(JNIEnv *env,
        jobject obj) {

    jbyteArray layout = NULL;
    gEnv = env;

    if(gEnv == NULL){
        LOG_E("tlcPinpadGetNativeLayout: gEnv is null!");
        return layout;
    }

    /* Allocate an array of byte to contain the layout */
    layout = gEnv->NewByteArray(sizeof(pinpad_layout_hdready));
    if(layout == NULL) {
        LOG_E("tlcPinpadGetNativeLayout: NewByteArrey failed!");
        return NULL;
    }
    /* Copy the layout to the array */
    gEnv->SetByteArrayRegion(layout, 0, sizeof(pinpad_layout_hdready), (jbyte*)&pinpad_layout_hdready);
    return layout;
}

EXTERN_C JNIEXPORT jbyteArray JNICALL
Java_com_trustonic_samplepinpad_PinpadTLCWrapper_tlcPinpadGetNativeLayoutHD(JNIEnv *env,
        jobject obj) {

    jbyteArray layout = NULL;
    gEnv = env;

    if(gEnv == NULL){
        LOG_E("tlcPinpadGetNativeLayout: gEnv is null!");
        return layout;
    }

    /* Allocate an array of byte to contain the layout */
    layout = gEnv->NewByteArray(sizeof(pinpad_layout_hd));
    if(layout == NULL) {
        LOG_E("tlcPinpadGetNativeLayout: NewByteArrey failed!");
        return NULL;
    }
    /* Copy the layout to the array */
    gEnv->SetByteArrayRegion(layout, 0, sizeof(pinpad_layout_hd), (jbyte*)&pinpad_layout_hd);
    return layout;
}

EXTERN_C JNIEXPORT jbyteArray JNICALL
Java_com_trustonic_samplepinpad_PinpadTLCWrapper_tlcPinpadGetNativeLayoutQuartHD(JNIEnv *env,
        jobject obj) {

    jbyteArray layout = NULL;
    gEnv = env;

    if(gEnv == NULL){
        LOG_E("tlcPinpadGetNativeLayoutQuartHD: gEnv is null!");
        return layout;
    }

    /* Allocate an array of byte to contain the layout */
    layout = gEnv->NewByteArray(sizeof(pinpad_layout_quarthd));
    if(layout == NULL) {
        LOG_E("tlcPinpadGetNativeLayoutQuartHD: NewByteArrey failed!");
        return NULL;
    }
    /* Copy the layout to the array */
    gEnv->SetByteArrayRegion(layout, 0, sizeof(pinpad_layout_quarthd), (jbyte*)&pinpad_layout_quarthd);
    return layout;
}

EXTERN_C JNIEXPORT jbyteArray JNICALL
Java_com_trustonic_samplepinpad_PinpadTLCWrapper_tlcPinpadGetNativeLayoutQHD(JNIEnv *env,
        jobject obj) {

    jbyteArray layout = NULL;
    gEnv = env;

    if(gEnv == NULL){
        LOG_E("tlcPinpadGetNativeLayoutQHD: gEnv is null!");
        return layout;
    }

    /* Allocate an array of byte to contain the layout */
    layout = gEnv->NewByteArray(sizeof(pinpad_layout_qhd));
    if(layout == NULL) {
        LOG_E("tlcPinpadGetNativeLayoutQHD: NewByteArrey failed!");
        return NULL;
    }
    /* Copy the layout to the array */
    gEnv->SetByteArrayRegion(layout, 0, sizeof(pinpad_layout_qhd), (jbyte*)&pinpad_layout_qhd);
    return layout;
}

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    jclass pinpadTlcWrapper;

    LOG_D("JNI_OnLoad");

    if(vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        LOG_E("JNI_OnLoad GetEnv failed");
        return -1;
    }

    pinpadTlcWrapper = env->FindClass(
            JAVA_TLC_WRAPPER_CLASS);
    if(pinpadTlcWrapper == NULL) {
        LOG_E("FindClass on class PinpadTLCWrapper failed!");
        return -1;
    }

    /* Cache the PinpadTLCWrapper class in a global reference */
    /* Use the cached gPinpadTlcWrapper.
     * As we called the AttachCurrentThread to get the java environnement from
     * a native thread, the FindClass will always fail. This is a ClassLoader issue.
     * This call (AttachCurrentThread) changes the call stack, so when the FindClass
     * try to start the class search in the class loader associated with this method,
     * FindClass find the ClassLoader associated with the a wrong class, so FindClass fails.*/
    gPinpadTlcWrapper = (jclass) env->NewGlobalRef(pinpadTlcWrapper);

    /* Cache the javaVM to get back a JNIEnv reference from native code*/
    gVm = vm;

    return JNI_VERSION_1_6;
}
