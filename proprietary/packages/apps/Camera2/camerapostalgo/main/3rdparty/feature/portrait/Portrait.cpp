/*
 *   Copyright Statement:
 *
 *     This software/firmware and related documentation ("MediaTek Software") are
 *     protected under relevant copyright laws. The information contained herein is
 *     confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *     the prior written permission of MediaTek inc. and/or its licensors, any
 *     reproduction, modification, use or disclosure of MediaTek Software, and
 *     information contained herein, in whole or in part, shall be strictly
 *     prohibited.
 *
 *     MediaTek Inc. (C) 2019. All rights reserved.
 *
 *     BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *    THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *     RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *     ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *     WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *     NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *     RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *     TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *     RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *     OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *     SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *     RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *     STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *     ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *     RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *     MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *     CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     The following software/firmware and/or related documentation ("MediaTek
 *     Software") have been modified by MediaTek Inc. All revisions are subject to
 *     any receiver's applicable license agreements with MediaTek Inc.
 */
#include <pthread.h>
#include <android/looper.h>
#include <cutils/properties.h>
#include <utils/std/Format.h>
#include <utils/Mutex.h>
#include <utils/Condition.h>
#include <string>
#include <system/graphics-base-v1.0.h>
#include <vendor/mediatek/hardware/mms/1.2/IMms.h>
#include "plugin/PipelinePluginType.h"
#include "mtk/mtk_platform_metadata_tag.h"
#include "mtk/mtk_feature_type.h"
#include "vndk/hardware_buffer.h"
#include "aw_portrait_bokeh.h"
#include "aw_portrait_bokeh_opengl.h"
#include "aw_portrait_common.h"
#include "aw_portrait_mask.h"
#include "LogUtils.h"
#include "GLContext.h"
#include "GLUtils.h"
#include "BufferUtils.h"
#include "TaskThread.h"
#include "ProcessingThread.h"
#include "PicDetect.h"
#define LOG_TAG "Portrait"
#define ONE_THREAD_MODE

using ::vendor::mediatek::hardware::mms::V1_2::IMms;
using ::vendor::mediatek::hardware::mms::V1_0::HwCopybitParam;
using NSCam::NSPipelinePlugin::Interceptor;
using NSCam::NSPipelinePlugin::PipelinePlugin;
using NSCam::NSPipelinePlugin::PluginRegister;
using NSCam::NSPipelinePlugin::eFeatureIndexMtk;
using NSCam::NSPipelinePlugin::Yuv;
using NSCam::NSPipelinePlugin::YuvPlugin;
using android::OK;
using android::sp;
using android::hardware::hidl_handle;
using namespace android;
using namespace std;
using namespace NSCam;
using namespace NSCam::Utils::Format;
using namespace NSCam::NSPipelinePlugin;

#define DUMP_BUFFER_CAPTURE "debug.portrait.dumpcapture.enabled"
#define DUMP_BUFFER_PREVIEW "debug.portrait.dumppreview.enabled"
#define INTERVAL_FRAME 3
static int gDumpBufferCaptureEnabled = ::property_get_int32(DUMP_BUFFER_CAPTURE, 0);
static int gDumpBufferPreviewEnabled = ::property_get_int32(DUMP_BUFFER_PREVIEW, 0);

class Portrait : public YuvPlugin::IProvider
{
public:
    typedef YuvPlugin::Property Property;
    typedef YuvPlugin::Selection Selection;
    typedef YuvPlugin::Request::Ptr RequestPtr;
    typedef YuvPlugin::RequestCallback::Ptr RequestCallbackPtr;
    typedef YuvPlugin::ConfigParam ConfigParam;

    Portrait();
    ~Portrait();
    const Property& property();
    void set(MINT32 openID1, MINT32 openID2);
    void init();
    void uninit();
    void doInit();
    void doUninit();
    void config(const ConfigParam &param);
    MERROR negotiate(Selection& sel);
    MERROR process(RequestPtr pRequest, RequestCallbackPtr pCallback);
    void doCapture(RequestPtr pRequest, RequestCallbackPtr pCallback, MSize inSize, MSize outSize,
        IImageBuffer *in, IImageBuffer *out);
    void doPreview(RequestPtr pRequest, RequestCallbackPtr pCallback, MSize inSize, MSize outSize,
        IImageBuffer *in, IImageBuffer *out);
    void abort(std::vector<RequestPtr> &pRequests);
    unsigned char* getBuffer(IImageBuffer *in, float multiple);
    static int getJpegRotation(RequestPtr pRequest);
    std::atomic<bool> waiting_request = true;
    long frame_count = 0;
    AWPortraitBokehOpenGL *mPortraits = nullptr;

private:
    enum TaskType {
        INIT,
        PROCESS,
        UNINIT,
        EXIT,
        UNKNOWN,
    };

    class PortraitTask : public Task {
    private:
        TaskType type = UNKNOWN;
        Portrait* portrait = nullptr;
        RequestPtr request;
        RequestCallbackPtr callback;

    public:
        PortraitTask() {}
        PortraitTask(TaskType t, Portrait* f) : type(t), portrait(f) {}
        PortraitTask(TaskType t, Portrait* f, RequestPtr r) : type(t), portrait(f), request(r) {}

        void run() {
            switch (type) {
                case INIT:
                    portrait->doInit();
                    break;
                case UNINIT:
                    portrait->doUninit();
                    break;
                case PROCESS:
                    portrait->doProcess(request, callback);
                    break;
                case EXIT:
                    portrait->mTaskThread->exit();
                    break;
                case UNKNOWN:
                    break;
            }
        }
    };

    static const int LOOPER_ID_MESSAGEPIPE = 1;
    static const int DETECT_BUFFER_RATE = 3;
    bool mHasInitGLContext = false;
    bool mHasInitMask = false;
    int mPortraitType = 0;
    int mDofLevel = 4;
    aw_face_orientation mFaceOrientation = FACE_LEFT;
    GLContext *mGLContext = nullptr;
    AwPortraitMask *mMasks = nullptr;
    IImageBuffer* mRGBAWorkingPreviewBuffer = nullptr;
    IImageBuffer* mYV12WorkingCaptureBuffer = nullptr;
    IImageBuffer* mRGBAInPreviewBuffer = nullptr;
    IImageBuffer* mRGBAInCaptureBuffer = nullptr;
    sp<ProcessingThread> mProcessingThread =  new ProcessingThread();
    sp<PicDetect> mRequest = new PicDetect();

#ifdef ONE_THREAD_MODE
    TaskThread<PortraitTask>* mTaskThread;
#endif

    MERROR doProcess(RequestPtr pRequest, RequestCallbackPtr pCallback);
    MERROR doProcessPreview(RequestPtr pRequest);
    MERROR doProcessCapture(RequestPtr pRequest);
};

class PicDetectCallback: public RequestCallback {
            private:
                Portrait *mImpl;
            public:
                PicDetectCallback(Portrait *impl) {
                    mImpl = impl;
                }
                virtual ~PicDetectCallback() {
                }
                virtual void onCompleted(sp<Request> current_request) {
                    MY_LOGW("onCompleted mImpl = %p", mImpl);
                    if (current_request != nullptr) {
                        PicDetect * picDetect = static_cast<PicDetect*>(current_request.get());
                        if (picDetect->maskResultForPreview != nullptr) {
                            mImpl-> mPortraits->setMask(picDetect->maskResultForPreview);
                        }
                    }
                    mImpl->waiting_request = true;
                }
};
sp<PicDetectCallback> picDetectCallback = nullptr;

Portrait::Portrait()
{
    FUNCTION_IN;
#ifdef ONE_THREAD_MODE
    mTaskThread = new TaskThread<PortraitTask>();
#endif
    FUNCTION_OUT;
}

Portrait::~Portrait()
{
    FUNCTION_IN;
#ifdef ONE_THREAD_MODE
    mTaskThread->postTaskAndWaitDone(PortraitTask(EXIT, this));
    mTaskThread->join();
    delete mTaskThread;
#endif
    FUNCTION_OUT;
}

const Portrait::Property& Portrait::property()
{
    FUNCTION_IN;
    static Property prop;
    static bool inited;

    if (!inited) {
        prop.mName = "360AI portrait";
        prop.mFeatures = MTK_FEATURE_PORTRAIT;
        prop.mInPlace = MFALSE;
        inited = true;
    }
    FUNCTION_OUT;
    return prop;
}
void Portrait::set(MINT32 openID1, MINT32 openID2)
{
    FUNCTION_IN;
    MY_LOGD("set openID1:%d openID2:%d", openID1, openID2);
    FUNCTION_OUT;
}

void Portrait::init()
{
    FUNCTION_IN;
#ifdef ONE_THREAD_MODE
    mTaskThread->postTaskAndWaitDone(PortraitTask(INIT, this));
#else
    doInit();
#endif
    FUNCTION_OUT;
}

void Portrait::doInit()
{
    FUNCTION_IN;
    if (mPortraitType == 0 && mProcessingThread != nullptr) {
        mProcessingThread -> run("PicDetectTread", PRIORITY_DEFAULT);
    }
    frame_count = 0;
    FUNCTION_OUT;
}

void Portrait::uninit()
{
    FUNCTION_IN;
#ifdef ONE_THREAD_MODE
    mTaskThread->postTaskAndWaitDone(PortraitTask(UNINIT, this));
#else
    doUninit();
#endif
    FUNCTION_OUT;
}

void Portrait::doUninit()
{
    FUNCTION_IN;
    if (mPortraitType == 0) {
        MY_LOGD("uninit AWPortrait +");
        mPortraits->uninit();
        MY_LOGD("uninit AWPortrait -");
        delete mPortraits;
        if (mGLContext != NULL) {
            mGLContext->unInitialize();
            mGLContext = NULL;
        }
    } else if (mPortraitType == 1) {
        mMasks->release();
    }
    FUNCTION_OUT;
}

void Portrait::config(const ConfigParam &param)
{
    FUNCTION_IN;
    FUNCTION_OUT;
}

template <class T> bool tryGetMetadata( IMetadata const *pMetadata, MUINT32 tag, T& rVal ) {
    if(pMetadata == nullptr) return false;
    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if(!entry.isEmpty()) {
        rVal = entry.itemAt(0,Type2Type<T>());
        return true;
    }
    return false;
}

MERROR Portrait::negotiate(Selection& sel)
{
    FUNCTION_IN;
    sel.mIBufferFull.setRequired(MTRUE)
                    .addAcceptedFormat(eImgFmt_YV12)
                    .addAcceptedSize(eImgSize_Full);
    sel.mOBufferFull.setRequired(MTRUE)
                    .addAcceptedFormat(eImgFmt_YV12)
                    .addAcceptedSize(eImgSize_Full);

    sel.mIMetadataDynamic.setRequired(MTRUE);
    sel.mIMetadataApp.setRequired(MTRUE);
    sel.mIMetadataHal.setRequired(MFALSE);
    sel.mOMetadataApp.setRequired(MTRUE);
    sel.mOMetadataHal.setRequired(MFALSE);
    FUNCTION_OUT;
    return OK;
}

MERROR Portrait::process(RequestPtr pRequest, RequestCallbackPtr pCallback)
{
    FUNCTION_IN;
#ifdef ONE_THREAD_MODE
    mTaskThread->postTaskAndWaitDone(PortraitTask(PROCESS, this, pRequest));
#else
    doProcess(pRequest, pCallback);
#endif
    if (pCallback != nullptr) {
        MY_LOGD("callback request");
        pCallback->onCompleted(pRequest, OK);
    }
    FUNCTION_OUT;
    return OK;
}

MERROR Portrait::doProcess(RequestPtr pRequest, RequestCallbackPtr pCallback)
{
    FUNCTION_IN;
    if(pRequest->mIBufferFull == NULL)
        MY_LOGD("pRequest->mIBufferFull==NULL");
    if(pRequest->mOBufferFull == NULL)
        MY_LOGD("pRequest->mOBufferFull==NULL");
    IImageBuffer *in = NULL, *out = NULL;
    MSize inSize, outSize;
    in = pRequest->mIBufferFull->acquire();
    out = pRequest->mOBufferFull->acquire();
    inSize = in->getImgSize();
    outSize = out->getImgSize();

    int faceOrientation = 1;
    //init metadata
    if(pRequest->mIBufferFull != NULL && pRequest->mOBufferFull != NULL)
    {
        IMetadata* pImetadata = pRequest->mIMetadataApp->acquire();
        if (pImetadata != nullptr) {
            MY_LOGE("doProcess Imetadata in meda data = %d", pImetadata->count());
            if (pImetadata->count() > 0) {
                tryGetMetadata<MINT32>(pImetadata, MTK_POSTALGO_PORTRAIT_DOF_LEVEL, mDofLevel);
                MY_LOGE("doProcess Imetadata in mDofLevel = %d", mDofLevel);
                tryGetMetadata<MINT32>(pImetadata, MTK_POSTALGO_PORTRAIT_TYPE, mPortraitType);
                MY_LOGE("doProcess Imetadata in mPortraitType = %d", mPortraitType);
                tryGetMetadata<MINT32>(pImetadata, MTK_POSTALGO_PORTRAIT_FACEORIENTATION, faceOrientation);
                MY_LOGE("doProcess Imetadata in faceOrientation = %d", faceOrientation);
                switch (faceOrientation) {
                    case 0:
                    mFaceOrientation = FACE_UP;
                    break;
                    case 1:
                    mFaceOrientation = FACE_LEFT;
                    break;
                    case 2:
                    mFaceOrientation = FACE_DOWN;
                    break;
                    case 3:
                    mFaceOrientation = FACE_RIGHT;
                    break;
                }
            }
        }
    }

    //do preview or do capture
    if (mPortraitType == 0) {
        if (mHasInitGLContext == false) {
            mGLContext = new GLContext();
            mGLContext->initialize(true);
            MY_LOGD("new AWPortrait");
            mPortraits = new AWPortraitBokehOpenGL();
            MY_LOGD("init AWPortrait +");
            mPortraits->init(true, 128, 128);
            mPortraits->setLogable(true);
            MY_LOGD("init AWPortrait -");
            mGLContext->enable();
            mHasInitGLContext = true;
        }
        doPreview(pRequest, pCallback, inSize, outSize, in, out);
    } else if (mPortraitType == 1) {
        if (mHasInitMask == false) {
            mMasks = new AwPortraitMask();
            string modelDir="/sdcard/modeldir";
            char *dir = modelDir.data();
            mMasks->init(dir, mMasks->getLowPrecision(), mMasks->getHighPrecision());
            mMasks->setLogable(true);
            mHasInitMask = true;
        }
        doCapture(pRequest, pCallback, inSize, outSize, in, out);
    }
    FUNCTION_OUT;
    return OK;
}

void Portrait::doCapture(RequestPtr pRequest, RequestCallbackPtr pCallback, MSize inSize,
    MSize outSize, IImageBuffer *in, IImageBuffer *out) {
    FUNCTION_IN;
    MY_LOGD("portrait capture start");

    if(pRequest->mIBufferFull != NULL && pRequest->mOBufferFull != NULL) {
        MY_LOGD("inSize.w = %d, inSize.h = %d, outSize.w = %d, outSize.h = %d",
            inSize.w, inSize.h, outSize.w, outSize.h);
        unsigned char* maskResultForCapture = (unsigned char *)malloc((inSize.w/4) * (inSize.h/4));
        int buffer_size = inSize.w*inSize.h*1.5;
        unsigned char* inBuffer = (unsigned char *)malloc(buffer_size);
        if (inBuffer != nullptr) {
            memcpy((void *)(inBuffer), (void *)(in->getBufVA(0)), buffer_size);
        }
        mMasks->detectPic(inBuffer, inSize.w, inSize.h, PIX_FMT_YUVI420, mFaceOrientation,
            maskResultForCapture, inSize.w / 4, inSize.h / 4);
        MY_LOGD("detectPic done");

        unsigned char* captureResult = (unsigned char *)malloc(inSize.w * inSize.h * 3 / 2);
        aw_portrait_bokeh(inBuffer, inSize.w, inSize.h, PIX_FMT_YUVI420, maskResultForCapture,
            inSize.w / 4, inSize.h / 4, mDofLevel, captureResult);
        MY_LOGD("capture done");
        if (captureResult != nullptr) {
            mYV12WorkingCaptureBuffer = BufferUtils::acquireWorkingBuffer(inSize, eImgFmt_YV12);
            memcpy((void *)mYV12WorkingCaptureBuffer->getBufVA(0), captureResult, inSize.w * inSize.h * 3 / 2);
            MY_LOGD("copy to working buffer done");
        }
        MY_LOGD("getJpegRotation(pRequest) = %d", getJpegRotation(pRequest));
        if (mYV12WorkingCaptureBuffer != nullptr) {
            BufferUtils::mdpResizeAndConvert(mYV12WorkingCaptureBuffer, out, getJpegRotation(pRequest));
            MY_LOGD("copy to out done");
        }

        if (gDumpBufferCaptureEnabled) {
            char bufferName[128];
            sprintf(bufferName, "capture_in_portrait");
            BufferUtils::dumpBuffer(in, bufferName);

            if (inBuffer != nullptr) {
                sprintf(bufferName, "capture_in_buffer_portrait");
                BufferUtils::dumpBufferX(inBuffer, bufferName, (inSize.w) * (inSize.h) * 1.5);
            }

            if (maskResultForCapture != nullptr) {
                sprintf(bufferName, "capture_mask_in_portrait");
                BufferUtils::dumpBufferX(maskResultForCapture, bufferName, (inSize.w/4) * (inSize.h/4) * 1.5);
            }

            if (mYV12WorkingCaptureBuffer != nullptr) {
                sprintf(bufferName, "capture_working_portrait");
                BufferUtils::dumpBuffer(mYV12WorkingCaptureBuffer, bufferName);
            }

            sprintf(bufferName, "capture_out_portrait");
            BufferUtils::dumpBuffer(out, bufferName);
        }

        free(maskResultForCapture);
        free(inBuffer);
        free(captureResult);
    }
    FUNCTION_OUT;
}

void Portrait::doPreview(RequestPtr pRequest, RequestCallbackPtr pCallback, MSize inSize,
    MSize outSize, IImageBuffer *in, IImageBuffer *out) {
    FUNCTION_IN;
    FPS;
    MY_LOGD("portrait preview start");
    static float identityMatrix[16] =
        {1.0f, 0.0f, 0.0f, 0.0f,
         0.0f, 1.0f, 0.0f, 0.0f,
         0.0f, 0.0f, 1.0f, 0.0f,
         0.0f, 0.0f, 0.0f, 1.0f};
    static const float quality = 1.0f;
    if(pRequest->mIBufferFull != NULL && pRequest->mOBufferFull != NULL) {
        GLUTILS_GET_EGLIMAGE_TEXTURE_PARAMS paramsIn;
        memset(&paramsIn, 0, sizeof(GLUTILS_GET_EGLIMAGE_TEXTURE_PARAMS));
        paramsIn.eglDisplay = mGLContext->_display;
        paramsIn.isRenderTarget = false;
        MY_LOGD("getEGLImageTexture for input start");
        paramsIn.graphicBuffer = BufferUtils::getAHWBuffer(in);
        GLUtils::getEGLImageTexture(&paramsIn);
        MY_LOGD("getEGLImageTexture for input done");

        GLUTILS_GET_EGLIMAGE_TEXTURE_PARAMS paramsOut;
        memset(&paramsOut, 0, sizeof(GLUTILS_GET_EGLIMAGE_TEXTURE_PARAMS));
        paramsOut.eglDisplay = mGLContext->_display;
        paramsOut.isRenderTarget = true;
        if (out->getImgFormat() != eImgFmt_RGBA8888) {
            MY_LOGD("out->getImgFormat() is not eImgFmt_RGBA8888");
            if (mRGBAWorkingPreviewBuffer == nullptr) {
                mRGBAWorkingPreviewBuffer = BufferUtils::acquireWorkingBuffer(inSize, eImgFmt_RGBA8888);
            }
            paramsOut.graphicBuffer = BufferUtils::getAHWBuffer(mRGBAWorkingPreviewBuffer);
        } else {
            MY_LOGD("out->getImgFormat() is eImgFmt_RGBA8888");
            paramsOut.graphicBuffer = BufferUtils::getAHWBuffer(out);
        }
        GLUtils::getEGLImageTexture(&paramsOut);
        MY_LOGD("getEGLImageTexture for output done");

        MY_LOGD("inSize.w = %d, inSize.h = %d, outSize.w = %d, outSize.h = %d, paramsIn.textureID = %d",
            inSize.w, inSize.h, outSize.w, outSize.h, paramsIn.textureID);
        if (picDetectCallback == nullptr) {
            picDetectCallback = new PicDetectCallback(this);
        }
        if (frame_count%INTERVAL_FRAME == 0) {
            mRequest -> in = in;
            mRequest -> imageWidth = inSize.w;
            mRequest -> imageHeight = inSize.h;
            mRequest -> orientation = mFaceOrientation;
            mProcessingThread -> sendRequest(mRequest, picDetectCallback);
            waiting_request = false;
        }
        frame_count++;
        mPortraits->setBlurLevel(mDofLevel);
        MY_LOGD("setBlurLevel is %d", mDofLevel);
        mPortraits->drawFrame(paramsIn.textureID, identityMatrix, inSize.w, inSize.h,
            outSize.w, outSize.h, paramsOut.fbo, quality);
        glFlush();
        mGLContext->createSyncKHR();
        int acquireFenceFd = mGLContext->createFenceFd();
        MY_LOGD("acquireFenceFd = %d", acquireFenceFd);
        out->acquireFenceFd = acquireFenceFd;

        MY_LOGD("drawFrame done");
        if (out->getImgFormat() != eImgFmt_RGBA8888 && mRGBAWorkingPreviewBuffer != nullptr) {
            BufferUtils::mdpResizeAndConvert(mRGBAWorkingPreviewBuffer, out);
            MY_LOGD("mdpResizeAndConvert done");
        }

        //dump preview buffer
        if (gDumpBufferPreviewEnabled) {
            char bufferName[128];
            sprintf(bufferName, "preview_in_portrait");
            BufferUtils::dumpBuffer(in, bufferName);

            if (mRGBAWorkingPreviewBuffer != nullptr) {
                sprintf(bufferName, "preview_working_portrait");
                BufferUtils::dumpBuffer(mRGBAWorkingPreviewBuffer, bufferName);
            }

            sprintf(bufferName, "preview_out_portrait");
            BufferUtils::dumpBuffer(out, bufferName);
        }
        GLUtils::releaseEGLImageTexture(&paramsIn);
        GLUtils::releaseEGLImageTexture(&paramsOut);
    }
    FUNCTION_OUT;
}

int Portrait::getJpegRotation(RequestPtr pRequest) {
    MINT32 jpegRotation = 0;
    IMetadata* pImetadata = pRequest->mIMetadataApp->acquire();
    if (pImetadata != nullptr && pImetadata->count() > 0) {
        tryGetMetadata<MINT32>(pImetadata, MTK_POSTALGO_JPEG_ORIENTATION, jpegRotation);
    }
    return jpegRotation;
}

void Portrait::abort(std::vector<RequestPtr> &pRequests)
{
    FUNCTION_IN;
    FUNCTION_OUT;
}

REGISTER_PLUGIN_PROVIDER(Yuv, Portrait);
