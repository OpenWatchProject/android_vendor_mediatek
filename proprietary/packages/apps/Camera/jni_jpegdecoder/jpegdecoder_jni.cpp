/*
 * Copyright (C) 2012 The Android Open Source Project
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

#define LOG_TAG "JpegDecoder_JNI"

#include <string.h>
#include <android/log.h>
#include <jni.h>
#include <android/bitmap.h>
#include <gui/Surface.h>
#include <ui/Region.h>
#include <utils/RefBase.h>
#include <cstdio>
#include <stdio.h>
#include "JNIHelp.h"
#include "android_runtime/AndroidRuntime.h"
#include <android_runtime/android_graphics_SurfaceTexture.h>
#include <android_runtime/android_view_Surface.h>
#include "mhal/MediaHal.h"
#include "mhal/MediaTypes.h"
#include <linux/ion_drv.h>
#include <ion/ion.h>
#include "libion_mtk/include/ion.h"
#include "ui/gralloc_extra.h"
#include <sys/mman.h>
#include "DebugHelper.h"

#define ALIGN16(x) ((x + 15)&(~(16-1)))
#define ALIGN32(x) ((x + 31)&(~(32-1)))
#define ALIGN128(x) ((x + 127)&(~(128-1)))
#define ALIGN512(x) ((x + 511)&(~(512-1)))

using namespace android;

class DecoderContext : public virtual RefBase{
public:
    ANativeWindow *nativeWindow;
    jbyteArray outDataArray;
    int32_t width;
    int32_t height;
    int32_t format;
    int32_t ionDri_handle;

    DecoderContext() {
        nativeWindow = NULL;
        outDataArray = NULL;
        width = 0;
        height = 0;
        format = 0;
        ionDri_handle = -1;
    }

    ~DecoderContext() {
        ALOGE("DecoderContext destoryed");
        nativeWindow = NULL;
        outDataArray = NULL;
        width = 0;
        height = 0;
        format = 0;
        ionDri_handle = -1;
    }
};

struct IonInfo{
    int32_t ion_fd;
    int size;
    void *virtualAddr;
    ion_user_handle_t ion_allocHandle;
};

jfieldID gNativeContext;
//   static DebugHelper* m_pDebugHelper = new DebugHelper();
   static DebugHelper* m_pDebugHelper = NULL;

static DecoderContext * getNativeContext(JNIEnv *env, jobject thiz) {
    jlong context = env->GetLongField(thiz, gNativeContext);
    return reinterpret_cast<DecoderContext*>(context);
}

static void setNativeContext(JNIEnv *env, jobject thiz, sp<DecoderContext> decoderContext) {
    DecoderContext* const p = getNativeContext(env, thiz);
    if (decoderContext != 0) {
        decoderContext->incStrong((void*)setNativeContext);
    }
    if (p) {
        p->decStrong((void*)setNativeContext);
    }
    jlong context = reinterpret_cast<jlong>(decoderContext.get());
    env->SetLongField(thiz, gNativeContext, context);
}

static sp<DecoderContext> newNativeContext(ANativeWindow *nativeWindow, jbyteArray outDataArray,
        int32_t width, int32_t height, int32_t format, int32_t ionDri_handle) {
    DecoderContext *context = new DecoderContext();
    sp<DecoderContext> decoderContext(context);
    decoderContext->nativeWindow = nativeWindow;
    decoderContext->outDataArray = outDataArray;
    decoderContext->width = width;
    decoderContext->height = height;
    decoderContext->format = format;
    decoderContext->ionDri_handle = ionDri_handle;
    return decoderContext;
}

static void setUpWindow(ANativeWindow *w, int32_t width, int32_t height, int32_t format) {
    native_window_api_connect(w, NATIVE_WINDOW_API_CPU);
    native_window_set_buffers_dimensions(w, width, height);
    native_window_set_buffers_format(w, format);
    native_window_set_usage(w, GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_HW_TEXTURE);
    native_window_set_scaling_mode(w, NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
    native_window_set_buffer_count(w, 3);
}

static IonInfo* allocateIon(int size, int32_t ionDri_handle) {
    ion_user_handle_t ion_allocHandle;
    int32_t ion_fd = -1;
    int alloc_ret = ion_alloc_mm(ionDri_handle, size, 1, 0, &ion_allocHandle);
    if (alloc_ret) {
        ALOGE("src Ion allocate failed, alloc_ret:%d, return", alloc_ret);
    }
    int share_ret = ion_share(ionDri_handle, ion_allocHandle, &ion_fd);
    if (share_ret) {
        ALOGE("src Ion share failed, share_ret:%d, return", share_ret);
    }
//    ALOGD("allocate ION successed, ion_fd=%d,", ion_fd);
    void *virtualAddr = ion_mmap(ionDri_handle, 0, size,
            PROT_READ | PROT_WRITE, MAP_SHARED, ion_fd, 0);
    IonInfo *info = (IonInfo *)malloc(sizeof(IonInfo));
    info->ion_fd = ion_fd;
    info->size = size;
    info->virtualAddr = virtualAddr;
    info->ion_allocHandle = ion_allocHandle;
    return info;
}

static void destroyIon(IonInfo* info, int32_t ionDri_Handle) {
    ion_munmap(ionDri_Handle, info->virtualAddr, info->size);
    ion_share_close(ionDri_Handle, info->ion_fd);
    ion_free(ionDri_Handle, info->ion_allocHandle);
    free(info);
}

static void decodeJpegToYV12(uint8_t * jpegData, uint32_t srcSize, uint8_t * dstData,
        uint32_t dstWidth, uint32_t dstHeight, int32_t srcFD, int32_t dstFD) {
//    ALOGD("[%s] + ", __func__);
    MHAL_JPEG_DEC_START_IN inParams;
    memset(&inParams, 0, sizeof(inParams));
    MHAL_JPEG_DEC_INFO_OUT outInfo;
    MHAL_JPEG_DEC_SRC_IN srcInfo;

    MINT32 err = MHAL_NO_ERROR;
    srcInfo.jpgDecHandle = NULL;

    inParams.srcFD = srcFD;
    inParams.dstFormat = JPEG_OUT_FORMAT_YV12;
    inParams.srcBuffer = jpegData;
    inParams.srcBufSize = ALIGN512(srcSize) + 512;
    inParams.srcLength = srcSize;
    inParams.dstWidth = dstWidth;
    inParams.dstHeight = dstHeight;
    inParams.dstFD = dstFD;
    inParams.dstVirAddr = dstData;
    inParams.dstPhysAddr = NULL;
    inParams.doDithering = 0;
    inParams.doRangeDecode = 0;
    inParams.doPostProcessing = 0;
    inParams.postProcessingParam = NULL;
    inParams.PreferQualityOverSpeed = 0;

    srcInfo.srcBuffer = jpegData;
    srcInfo.srcLength = srcSize;

    if (err != mHalJpeg(MHAL_IOCTL_JPEG_DEC_PARSER, (void *)&srcInfo, sizeof(srcInfo),
            NULL, 0, NULL)) {
//        ALOGD("[onDecode]parser file error");
        return;
    }

    outInfo.jpgDecHandle = srcInfo.jpgDecHandle;
//    ALOGD("outInfo.jpgDecHandle --> %p", outInfo.jpgDecHandle);

    if (err != mHalJpeg(MHAL_IOCTL_JPEG_DEC_GET_INFO, NULL, 0,
            (void *) &outInfo, sizeof(outInfo), NULL)) {
//        ALOGD("[onDecode]get info error");
        return;
    }

//    ALOGD("outInfo.srcWidth --> %d", outInfo.srcWidth);
//    ALOGD("outInfo.srcHeight -- > %d", outInfo.srcHeight);

    //2 step8: set inParams
    inParams.jpgDecHandle = srcInfo.jpgDecHandle;
    inParams.dstWidth = ALIGN16(inParams.dstWidth);
    inParams.dstHeight = ALIGN16(inParams.dstHeight);

//    ALOGD("inParams.dstFormat --> %d", inParams.dstFormat);
//    ALOGD("inParams.dstWidth -- > %d", inParams.dstWidth);
//    ALOGD("inParams.dstHeight --> %d", inParams.dstHeight);
//    ALOGD("inParams.srcLength --> %d", inParams.srcLength);
    //2 step9: start decode
    if (err != mHalJpeg(MHAL_IOCTL_JPEG_DEC_START, (void *) &inParams,
            sizeof(inParams), NULL, 0, NULL)) {
//        ALOGD("JPEG HW not support this image");
        return;
    }
//    ALOGD("[%s] - ", __func__);
}

static void com_mediatek_jpegdecoder_classInit(JNIEnv *env, jclass clazz) {
    gNativeContext = env->GetFieldID(clazz, "mNativeContext", "J");
}

static void com_mediatek_jpegdecoder_setup_surfacetexture(JNIEnv *env, jobject thiz,
                                                          jobject jsurfacetexture) {
//    ALOGD("[%s] +", __func__);
    sp<ANativeWindow> sp_nativeWindow = NULL;
    sp<IGraphicBufferProducer> producer;
    int32_t width = 0;
    int32_t height = 0;
    int32_t format = HAL_PIXEL_FORMAT_YV12;
    if (jsurfacetexture != NULL) {
        producer = SurfaceTexture_getProducer(env, jsurfacetexture);
        if (producer == NULL) {
            ALOGE("Graphic buffer producer is null");
            return;
        }

        sp_nativeWindow = new Surface(producer, true);
        sp_nativeWindow->incStrong((void*)com_mediatek_jpegdecoder_setup_surfacetexture);
        if (sp_nativeWindow == NULL) {
            ALOGE("native window is null");
            return;
        }

        ANativeWindow *nativeWindow = sp_nativeWindow.get();
        int err = NO_ERROR;
        err = nativeWindow->query(nativeWindow, NATIVE_WINDOW_WIDTH, &width);
        if (NO_ERROR != err) {
//            ALOGD("[%s]: Error while querying surface width %s %d", __func__,
//                    strerror(-err), err);
            return;
        }

        err = nativeWindow->query(nativeWindow, NATIVE_WINDOW_HEIGHT, &height);
        if (NO_ERROR != err) {
//            ALOGD("[%s]: Error while querying surface height %s %d", __func__,
//                    strerror(-err), err);
            return;
        }

        err = nativeWindow->query(nativeWindow, NATIVE_WINDOW_FORMAT, &format);
        if (NO_ERROR != err) {
//            ALOGD("[%s]: Error while querying surface format %s %d", __func__,
//                    strerror(-err), err);
            return;
        }
        setUpWindow(nativeWindow, ALIGN32(width), ALIGN16(height), HAL_PIXEL_FORMAT_YV12);
        int32_t ionDri_handle = mt_ion_open("jpeg_decoder");
        sp<DecoderContext> decoderContext = newNativeContext(nativeWindow, NULL,
                ALIGN32(width), ALIGN16(height), HAL_PIXEL_FORMAT_YV12, ionDri_handle);
//        ALOGI("[%s]: Surface width:%d, height:%d, format:%d, nativeWindow:%p, ionDri_handle:%d",
//                __func__, ALIGN32(width), ALIGN16(height), format, nativeWindow, ionDri_handle);
        setNativeContext(env, thiz, decoderContext);
    }
//    ALOGD("[%s] - ", __func__);
}

static void com_mediatek_jpegdecoder_setup_1(JNIEnv *env, jobject thiz, jint width,
        jint height, jint format, jbyteArray outDataArray) {
//    ALOGD("[%s]", __func__);
    int32_t ionDri_handle = mt_ion_open("jpeg_decoder");
    sp<DecoderContext> decoderContext = newNativeContext(NULL, outDataArray, width,
            height, format, ionDri_handle);
//    ALOGD("[%s], width:%d, height:%d, format:%d, ionDri_handle:%d", __func__,
//            width, height, format, ionDri_handle);
    setNativeContext(env, thiz, decoderContext);
//    ALOGD("[%s] - ", __func__);
}

static void com_mediatek_jpegdecoder_decode(JNIEnv *env, jobject thiz, jbyteArray jpegDataArray) {
//    ALOGD("[%s] +", __func__);
    DecoderContext *decoderContext = getNativeContext(env, thiz);
    ANativeWindowBuffer *nativeWindowBuffer = NULL;
    sp<GraphicBuffer> graphicbuffer = NULL;
    void *ptr = NULL;
    int err = NO_ERROR;
    if (decoderContext == NULL) {
//        ALOGD("[%s], Get context is null", __func__);
        return;
    }
    ANativeWindow *nativeWindow = decoderContext->nativeWindow;
    jbyteArray outDataArray = decoderContext->outDataArray;
    int width = decoderContext->width;
    int height = decoderContext->height;
    int32_t ionDri_handle = decoderContext->ionDri_handle;
//    ALOGD("[%s], dequeue buffer, nativeWindow:%p", __func__, nativeWindow);
    if (nativeWindow != NULL) {
        err = nativeWindow->dequeueBuffer_DEPRECATED(nativeWindow,
                &nativeWindowBuffer);
        if (NO_ERROR != err) {
            ALOGE("dequeue failed (after): %s", strerror(-err));
            return;
        }

        // graphicbuffer = new GraphicBuffer(nativeWindowBuffer, /*keepOwnership*/false);
        graphicbuffer = GraphicBuffer::from(nativeWindowBuffer);
        int32_t dstFD = -1;
        err = gralloc_extra_query(graphicbuffer->handle, GRALLOC_EXTRA_GET_ION_FD, &dstFD);
//        ALOGD("query graphic buffer ion fd = %d,", dstFD);

        if (GRALLOC_EXTRA_OK != err) {
            ALOGE("get Ion FD failed:%d, return", err);
            return;
        }

        err = graphicbuffer->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, &ptr);
        if (NO_ERROR != err) {
            ALOGE("lock buffer failed (after): %s", strerror(-err));
            graphicbuffer->unlock();
            return;
        }
        //decode jpeg data.
        jbyte* jpegData = (jbyte*) env->GetByteArrayElements(jpegDataArray,
                NULL);
        int size = env->GetArrayLength(jpegDataArray);

        IonInfo* srcIoninfo = allocateIon(ALIGN512(size) + 512, ionDri_handle);
        memcpy(srcIoninfo->virtualAddr, (uint8_t *) jpegData, size);

        if (NULL != m_pDebugHelper) {
           m_pDebugHelper->dumpBufferToFile("/sdcard/effect/jpgtoyv12_in.jpg", (uint8_t*)jpegData,
                   size);
        }

        decodeJpegToYV12((uint8_t *)srcIoninfo->virtualAddr, (uint32_t) size,
                (uint8_t *) ptr, width, height, srcIoninfo->ion_fd, dstFD);

        if (NULL != m_pDebugHelper) {
           m_pDebugHelper->dumpBufferToFile("/sdcard/effect/jpgtoyv12_out.yuv", (uint8_t*)ptr,
                  width * height * 3 /2);
        }

        graphicbuffer->unlock();
//        ALOGD("[%s], queue buffer, nativeWindow:%p, nativeWindowBuffer:%p, ptr:%p",
//                __func__, nativeWindow, nativeWindowBuffer, ptr);
        nativeWindow->queueBuffer_DEPRECATED(nativeWindow,
                nativeWindowBuffer);
        destroyIon(srcIoninfo, ionDri_handle);
        env->ReleaseByteArrayElements(jpegDataArray, jpegData, JNI_ABORT);
    } else if (outDataArray != NULL) {
        jbyte* jpegData = (jbyte*) env->GetByteArrayElements(jpegDataArray,
                NULL);
        int size = env->GetArrayLength(jpegDataArray);
        jbyte* outData = (jbyte*) env->GetByteArrayElements(outDataArray,
                        NULL);
        decodeJpegToYV12((uint8_t *) jpegData, (uint32_t) size, (uint8_t *) outData,
                width, height, -1, -1);
        env->ReleaseByteArrayElements(jpegDataArray, jpegData, JNI_ABORT);
        env->ReleaseByteArrayElements(outDataArray, outData, JNI_ABORT);
    }
//    ALOGD("[%s] - ", __func__);
}

static void com_mediatek_jpegdecoder_release(JNIEnv *env, jobject thiz) {
//    ALOGI("[%s] +", __func__);
    DecoderContext *decoderContext = getNativeContext(env, thiz);
    if (decoderContext != NULL) {
        ANativeWindow *nativeWindow = decoderContext->nativeWindow;
        native_window_api_disconnect(nativeWindow, NATIVE_WINDOW_API_CPU);
        nativeWindow->decStrong((void*)com_mediatek_jpegdecoder_setup_surfacetexture);
        ion_close(decoderContext->ionDri_handle);
        setNativeContext(env, thiz, NULL);
    }
//    ALOGI("[%s] - ", __func__);
}

const char *classPathName = "com/mediatek/camera/util/jpegcodec/HwJpegDecodeImpl";

JNINativeMethod methods[] = {
    {"native_classInit", "()V", (void*)com_mediatek_jpegdecoder_classInit},
    {"native_setup", "(Landroid/graphics/SurfaceTexture;)V",
                                             (void*)com_mediatek_jpegdecoder_setup_surfacetexture},
    {"native_setup", "(III[B)V", (void*)com_mediatek_jpegdecoder_setup_1},
    {"native_decode", "([B)V", (void*)com_mediatek_jpegdecoder_decode},
    {"native_release", "()V", (void*)com_mediatek_jpegdecoder_release},
};

/*
 * Register several native methods for one class.
 */
static int registerNativeMethods(JNIEnv* env, const char* className,
    JNINativeMethod* gMethods, int numMethods)
{
    jclass clazz;
    clazz = env->FindClass(className);
    if (clazz == NULL) {
//        ALOGE("Native registration unable to find class '%s'", className);
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
//        ALOGE("RegisterNatives failed for '%s'", className);
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

/*
 * Register native methods for all classes we know about.
 *
 * returns JNI_TRUE on success.
 */
static int registerNatives(JNIEnv* env)
{
  if (!registerNativeMethods(env, classPathName,
                 methods, sizeof(methods) / sizeof(methods[0]))) {
    return JNI_FALSE;
  }

  return JNI_TRUE;
}


// ----------------------------------------------------------------------------

/*
 * This is called by the VM when the shared library is first loaded.
 */

typedef union {
    JNIEnv* env;
    void* venv;
} UnionJNIEnvToVoid;

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    UnionJNIEnvToVoid uenv;
    uenv.venv = NULL;
    jint result = -1;
    JNIEnv* env = NULL;

    ALOGI("JNI_OnLoad");

    if (vm->GetEnv(&uenv.venv, JNI_VERSION_1_4) != JNI_OK) {
//        ALOGE("ERROR: GetEnv failed");
        goto bail;
    }
    env = uenv.env;

    if (registerNatives(env) != JNI_TRUE) {
//        ALOGE("ERROR: registerNatives failed");
        goto bail;
    }

    result = JNI_VERSION_1_4;

bail:
    return result;
}

