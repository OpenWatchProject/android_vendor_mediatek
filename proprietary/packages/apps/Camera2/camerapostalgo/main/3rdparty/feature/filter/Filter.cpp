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
#include <stdio.h>
#include <cutils/properties.h>
#include <utils/std/Format.h>
#include <system/graphics-base-v1.0.h>
#include "plugin/PipelinePluginType.h"
#include "mtk/mtk_platform_metadata_tag.h"
#include "mtk/mtk_feature_type.h"
#include "vndk/hardware_buffer.h"
#include "aw_filter_api.h"
#include "LogUtils.h"
#include "GLContext.h"
#include "GLUtils.h"
#include "BufferUtils.h"
#include "TaskThread.h"

#define LOG_TAG "Filter"

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

using namespace NSCam::Utils::Format;
using namespace NSCam::NSPipelinePlugin;

#define ONE_THREAD_MODE
#define DUMP_BUFFER_CAPTURE "debug.filter.dumpcapture.enabled"
#define DUMP_BUFFER_PREVIEW "debug.filter.dumppreview.enabled"
#define DUMP_BUFFER_MATRIX "debug.filter.dumpmatrix.enabled"
static int gDumpBufferCaptureEnabled = ::property_get_int32(DUMP_BUFFER_CAPTURE, 0);
static int gDumpBufferPreviewEnabled = ::property_get_int32(DUMP_BUFFER_PREVIEW, 0);
static int gDumpBufferMatrixEnabled = ::property_get_int32(DUMP_BUFFER_MATRIX, 0);

class Filter : public YuvPlugin::IProvider
{
public:
    typedef YuvPlugin::Property Property;
    typedef YuvPlugin::Selection Selection;
    typedef YuvPlugin::Request::Ptr RequestPtr;
    typedef YuvPlugin::RequestCallback::Ptr RequestCallbackPtr;
    typedef YuvPlugin::ConfigParam ConfigParam;
    enum FilterType {
        PREVIEW,
        CAPTURE,
        MATRIX,
    };

    Filter(FilterType type);
    ~Filter();
    const Property& property();
    void set(MINT32 openID1, MINT32 openID2);
    void init();
    void uninit();
    void config(const ConfigParam &param);
    MERROR negotiate(Selection& sel);
    MERROR process(RequestPtr pRequest, RequestCallbackPtr pCallback);
    void abort(std::vector<RequestPtr> &pRequests);

private:
    enum TaskType {
        INIT,
        PROCESS,
        UNINIT,
        EXIT,
        UNKNOWN,
    };

    class FilterTask : public Task {
    private:
        TaskType type = UNKNOWN;
        Filter* filter = nullptr;
        RequestPtr request;
        RequestCallbackPtr callback;

    public:
        FilterTask() {}
        FilterTask(TaskType t, Filter* f) : type(t), filter(f) {}
        FilterTask(TaskType t, Filter* f, RequestPtr r) : type(t), filter(f), request(r) {}

        void run() {
            switch (type) {
                case INIT:
                    filter->doInit();
                    break;
                case UNINIT:
                    filter->doUninit();
                    break;
                case PROCESS:
                    filter->doProcess(request);
                    break;
                case EXIT:
                    filter->mTaskThread->exit();
                    break;
                case UNKNOWN:
                    break;
            }
        }
    };

    FilterType mFilterType;
    Property mProperty;
    bool mPropertyInited = false;
    int mFilterHandle = -1;
    GLContext *mGLContext = nullptr;
    IImageBuffer* mRGBAInBuffer = nullptr;
    IImageBuffer* mRGBAOutBuffer = nullptr;
    bool mHasInit = false;

#ifdef ONE_THREAD_MODE
    TaskThread<FilterTask>* mTaskThread = nullptr;
#endif

    void doInit();
    void doUninit();
    MERROR doProcess(RequestPtr pRequest);
    MERROR doProcessPreview(RequestPtr pRequest);
    MERROR doProcessCapture(RequestPtr pRequest);
    MERROR doProcessMatrix(RequestPtr pRequest);
    void drawFilterFrame(IImageBuffer* in, IImageBuffer* out, aw_filters_type filterType);
    static aw_filters_type getFilterType(RequestPtr pRequest);
    static int getJpegRotation(RequestPtr pRequest);
};

class FilterPreview: public Filter {
    public:
        FilterPreview():Filter(PREVIEW) {
            FUNCTION_IN;
            FUNCTION_OUT;
        }
        ~FilterPreview() {
            FUNCTION_IN;
            FUNCTION_OUT;
        }
};

class FilterCapture: public Filter {
    public:
        FilterCapture():Filter(CAPTURE) {
            FUNCTION_IN;
            FUNCTION_OUT;
        }
        ~FilterCapture() {
            FUNCTION_IN;
            FUNCTION_OUT;
        }
};

class FilterMatrix: public Filter {
    public:
        FilterMatrix():Filter(MATRIX) {
            FUNCTION_IN;
            FUNCTION_OUT;
        }
        ~FilterMatrix() {
            FUNCTION_IN;
            FUNCTION_OUT;
        }
};

REGISTER_PLUGIN_PROVIDER(Yuv, FilterCapture);
REGISTER_PLUGIN_PROVIDER(Yuv, FilterPreview);
REGISTER_PLUGIN_PROVIDER(Yuv, FilterMatrix);

Filter::Filter(FilterType type)
{
    FUNCTION_IN;
    mFilterType = type;
#ifdef ONE_THREAD_MODE
    mTaskThread = new TaskThread<FilterTask>();
#endif
    FUNCTION_OUT;
}

Filter::~Filter()
{
    FUNCTION_IN;
#ifdef ONE_THREAD_MODE
    mTaskThread->postTaskAndWaitDone(FilterTask(EXIT, this));
    mTaskThread->join();
    delete mTaskThread;
#endif
    FUNCTION_OUT;
}

const Filter::Property& Filter::property()
{
    FUNCTION_IN;

    if (!mPropertyInited) {
        switch (mFilterType) {
            case PREVIEW:
                mProperty.mName = "360AI filter preview";
                mProperty.mFeatures = MTK_FEATURE_FILTER_PREVIEW;
                break;
            case CAPTURE:
                mProperty.mName = "360AI filter capture";
                mProperty.mFeatures = MTK_FEATURE_FILTER_CAPTURE;
                break;
            case MATRIX:
                mProperty.mName = "360AI filter matrix";
                mProperty.mFeatures = MTK_FEATURE_FILTER_MATRIX;
                break;
        }
        mProperty.mInPlace = MFALSE;
        mPropertyInited = true;
    }
    MY_LOGD("mProperty.mFeatures = %lu, mFilterType = %d", (unsigned long)mProperty.mFeatures, mFilterType);
    FUNCTION_OUT;
    return mProperty;
}
void Filter::set(MINT32 openID1, MINT32 openID2)
{
    FUNCTION_IN;
    MY_LOGD("set openID1:%d openID2:%d", openID1, openID2);
    FUNCTION_OUT;
}

void Filter::init()
{
    FUNCTION_IN;
#ifdef ONE_THREAD_MODE
    mTaskThread->postTaskAndWaitDone(FilterTask(INIT, this));
#else
    doInit();
#endif
    FUNCTION_OUT;
}

void Filter::doInit()
{
    FUNCTION_IN;
    FUNCTION_OUT;
}

void Filter::uninit()
{
    FUNCTION_IN;
#ifdef ONE_THREAD_MODE
    mTaskThread->postTaskAndWaitDone(FilterTask(UNINIT, this));
#else
    doUninit();
#endif
    FUNCTION_OUT;
}

void Filter::doUninit()
{
    FUNCTION_IN;
    if (mFilterType != CAPTURE) {
        MY_LOGD("aw_filter_api_uninit() handle %d +", mFilterHandle);
        aw_filter_api_uninit(mFilterHandle);
        MY_LOGD("aw_filter_api_uninit() handle %d -", mFilterHandle);
    }
    if (mGLContext != NULL) {
        mGLContext->unInitialize();
        mGLContext = NULL;
    }
    FUNCTION_OUT;
}

void Filter::config(const ConfigParam &param)
{
    FUNCTION_IN;
    FUNCTION_OUT;
}

MERROR Filter::negotiate(Selection& sel)
{
    FUNCTION_IN;
    switch (mFilterType) {
        case PREVIEW:
        case CAPTURE:
            sel.mIBufferFull.setRequired(MTRUE)
                            .addAcceptedFormat(eImgFmt_YV12)
                            .addAcceptedSize(eImgSize_Full);
            sel.mOBufferFull.setRequired(MTRUE)
                            .addAcceptedFormat(eImgFmt_YV12)
                            .addAcceptedSize(eImgSize_Full);
            break;
        case MATRIX:
            sel.mIBufferFull.setRequired(MTRUE)
                            .addAcceptedFormat(eImgFmt_YV12)
                            .addAcceptedSize(eImgSize_Full);
            sel.mOBufferFull.setRequired(MTRUE)
                            .addAcceptedFormat(eImgFmt_YV12)
                            .addAcceptedSize(eImgSize_Full);
            sel.mOBufferFull0.setRequired(MTRUE)
                            .addAcceptedFormat(eImgFmt_YV12)
                            .addAcceptedSize(eImgSize_Full);
            sel.mOBufferFull1.setRequired(MTRUE)
                            .addAcceptedFormat(eImgFmt_YV12)
                            .addAcceptedSize(eImgSize_Full);
            sel.mOBufferFull2.setRequired(MTRUE)
                            .addAcceptedFormat(eImgFmt_YV12)
                            .addAcceptedSize(eImgSize_Full);
            sel.mOBufferFull3.setRequired(MTRUE)
                            .addAcceptedFormat(eImgFmt_YV12)
                            .addAcceptedSize(eImgSize_Full);
            sel.mOBufferFull4.setRequired(MTRUE)
                            .addAcceptedFormat(eImgFmt_YV12)
                            .addAcceptedSize(eImgSize_Full);
            sel.mOBufferFull5.setRequired(MTRUE)
                            .addAcceptedFormat(eImgFmt_YV12)
                            .addAcceptedSize(eImgSize_Full);
            sel.mOBufferFull6.setRequired(MTRUE)
                            .addAcceptedFormat(eImgFmt_YV12)
                            .addAcceptedSize(eImgSize_Full);
            sel.mOBufferFull7.setRequired(MTRUE)
                            .addAcceptedFormat(eImgFmt_YV12)
                            .addAcceptedSize(eImgSize_Full);
            break;
    }

    sel.mIMetadataApp.setRequired(MTRUE);
    sel.mOMetadataApp.setRequired(MTRUE);
    FUNCTION_OUT;
    return OK;
}

MERROR Filter::process(RequestPtr pRequest, RequestCallbackPtr pCallback)
{
    FUNCTION_IN;
#ifdef ONE_THREAD_MODE
    mTaskThread->postTaskAndWaitDone(FilterTask(PROCESS, this, pRequest));
#else
    doProcess(pRequest);
#endif
    if (pCallback != nullptr) {
        MY_LOGD("callback request");
        pCallback->onCompleted(pRequest, OK);
    }
    FUNCTION_OUT;
    return OK;
}

MERROR Filter::doProcess(RequestPtr pRequest)
{
    FUNCTION_IN;
    MERROR ret;
    switch (mFilterType) {
        case PREVIEW:
            ret = doProcessPreview(pRequest);
            break;
        case CAPTURE:
            ret = doProcessCapture(pRequest);
            break;
        case MATRIX:
            ret = doProcessMatrix(pRequest);
            break;
        default:
            break;
    }
    FUNCTION_OUT;
    return ret;
}

void Filter::drawFilterFrame(IImageBuffer* in, IImageBuffer* out, aw_filters_type filterType) {
    FUNCTION_IN;

    static float identityMatrix[16] =
        {1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f};

    static const float quality = 1.0f;

    MSize inSize, outSize;
    inSize = in->getImgSize();
    outSize = out->getImgSize();
    MY_LOGD("inFormat = %d, inSize.w = %d, inSize.h = %d, outFormat = %d, outSize.w = %d, outSize.h = %d, filterType = %d",
        in->getImgFormat(), inSize.w, inSize.h,
        out->getImgFormat(),outSize.w, outSize.h,
        (int)filterType);

    if (mFilterType == MATRIX || filterType != NO_FILTER) {
        // create texture in
        GLUTILS_GET_EGLIMAGE_TEXTURE_PARAMS paramsIn;
        memset(&paramsIn, 0, sizeof(GLUTILS_GET_EGLIMAGE_TEXTURE_PARAMS));
        paramsIn.eglDisplay = mGLContext->_display;
        paramsIn.isRenderTarget = false;
        paramsIn.graphicBuffer = BufferUtils::getAHWBuffer(in);

        GLUtils::getEGLImageTexture(&paramsIn);
        MY_LOGD("create input texture ok");

        // create texture out
        GLUTILS_GET_EGLIMAGE_TEXTURE_PARAMS paramsOut;
        memset(&paramsOut, 0, sizeof(GLUTILS_GET_EGLIMAGE_TEXTURE_PARAMS));
        paramsOut.eglDisplay = mGLContext->_display;
        paramsOut.isRenderTarget = true;
        if (out->getImgFormat() != eImgFmt_RGBA8888) {
            if (mRGBAOutBuffer == nullptr) {
                mRGBAOutBuffer = BufferUtils::acquireWorkingBuffer(outSize,
                            eImgFmt_RGBA8888);
            }
            paramsOut.graphicBuffer = BufferUtils::getAHWBuffer(mRGBAOutBuffer);
        } else {
            paramsOut.graphicBuffer = BufferUtils::getAHWBuffer(out);
        }
        GLUtils::getEGLImageTexture(&paramsOut);
        MY_LOGD("create output texture ok");

        // call algo
        aw_filter_api_drawFrame(mFilterHandle, paramsIn.textureID, identityMatrix, true, inSize.w,
        inSize.h, outSize.w, outSize.h, paramsOut.fbo, filterType, quality);
        MY_LOGD("call algo <drawFilterFrame> ok");

        // convert buffer out to YV12 format
        if (out->getImgFormat() != eImgFmt_RGBA8888) {
            BufferUtils::mdpResizeAndConvert(mRGBAOutBuffer, out);
        }

        GLUtils::releaseEGLImageTexture(&paramsIn);
        GLUtils::releaseEGLImageTexture(&paramsOut);
    } else {
        BufferUtils::mdpResizeAndConvert(in, out);
    }

    if (mFilterType == PREVIEW && gDumpBufferPreviewEnabled != 0) {
        char bufferName[128];

        sprintf(bufferName, "preview_in_filter%d", filterType);
        BufferUtils::dumpBuffer(in, bufferName);

        if (out->getImgFormat() != eImgFmt_RGBA8888) {
            sprintf(bufferName, "preview_drawFilterFrame_filter%d", filterType);
            BufferUtils::dumpBuffer(mRGBAOutBuffer, bufferName);
        }

        sprintf(bufferName, "preview_out_filter%d", filterType);
        BufferUtils::dumpBuffer(out, bufferName);
    }

    if (mFilterType == MATRIX && gDumpBufferMatrixEnabled != 0) {
        char bufferName[128];

        sprintf(bufferName, "matrix_in_filter%d", filterType);
        BufferUtils::dumpBuffer(in, bufferName);

        if (out->getImgFormat() != eImgFmt_RGBA8888) {
            sprintf(bufferName, "matrix_drawFilterFrame_filter%d", filterType);
            BufferUtils::dumpBuffer(mRGBAOutBuffer, bufferName);
        }

        sprintf(bufferName, "matrix_out_filter%d", filterType);
        BufferUtils::dumpBuffer(out, bufferName);
    }

    FUNCTION_OUT;
}

MERROR Filter::doProcessPreview(RequestPtr pRequest) {
    FUNCTION_IN;
    FPS;

    if (mHasInit == false) {
        mGLContext = new GLContext();
        mGLContext->initialize(true);
        MY_LOGD("aw_filter_api_init() +");
        mFilterHandle = aw_filter_api_init();
        MY_LOGD("aw_filter_api_init() handle %d -", mFilterHandle);
        mHasInit = true;
    }

    mGLContext->enable();

    drawFilterFrame(pRequest->mIBufferFull->acquire(), pRequest->mOBufferFull->acquire(),
        getFilterType(pRequest));

    FUNCTION_OUT;
    return OK;
}

MERROR Filter::doProcessCapture(RequestPtr pRequest) {
    FUNCTION_IN;
    if (mHasInit == false) {
        mHasInit = true;
    }
    static const float quality = 1.0f;

    IImageBuffer *in = NULL, *out = NULL;
    in = pRequest->mIBufferFull->acquire();
    out = pRequest->mOBufferFull->acquire();

    MSize inSize, outSize;
    inSize = in->getImgSize();
    outSize = out->getImgSize();

    aw_filters_type filterType = getFilterType(pRequest);

    if (filterType != NO_FILTER) {
        // prepare param for algo
        aw_image_info_st param;
        memset(&param, 0, sizeof(aw_image_info_st));
        param.w = inSize.w;
        param.h = inSize.h;
        // convert buffer in to RGBA format
        if (in->getImgFormat() != eImgFmt_RGBA8888) {
            if (mRGBAInBuffer == nullptr) {
                mRGBAInBuffer = BufferUtils::acquireWorkingBuffer(inSize, eImgFmt_RGBA8888);
            }
            BufferUtils::mdpResizeAndConvert(in, mRGBAInBuffer);
            param.size = mRGBAInBuffer->getBufSizeInBytes(0);
            param.data = (unsigned char*)mRGBAInBuffer->getBufVA(0);
        } else {
            param.size = in->getBufSizeInBytes(0);
            param.data = (unsigned char*)in->getBufVA(0);
        }

        // call algo
        MY_LOGD("inFormat = %d, inSize.w = %d, inSize.h = %d, outFormat = %d, outSize.w = %d, outSize.h = %d",
                in->getImgFormat(), inSize.w, inSize.h,
                out->getImgFormat(),outSize.w, outSize.h);
        aw_image_info_st* ret = aw_filter_api_processArgb(&param, filterType, quality);
        MY_LOGD("call algo <processArgb> ok");

        if (out->getImgFormat() != eImgFmt_RGBA8888 || getJpegRotation(pRequest) != 0) {
            // copy algo out buffer to mRGBAOutBuffer
            if (mRGBAOutBuffer == nullptr) {
                mRGBAOutBuffer = BufferUtils::acquireWorkingBuffer(inSize, eImgFmt_RGBA8888);
            }
            memcpy((void *)mRGBAOutBuffer->getBufVA(0), ret->data, ret->size);

            BufferUtils::mdpResizeAndConvert(mRGBAOutBuffer, out, getJpegRotation(pRequest));
        } else {
            memcpy((void *)out->getBufVA(0), ret->data, ret->size);
        }
    } else {
        BufferUtils::mdpResizeAndConvert(in, out, getJpegRotation(pRequest));
    }

    if (gDumpBufferCaptureEnabled) {
        char bufferName[128];
        sprintf(bufferName, "capture_in_filter%d", filterType);
        BufferUtils::dumpBuffer(in, bufferName);

        if (in->getImgFormat() != eImgFmt_RGBA8888 && mRGBAInBuffer != nullptr) {
            sprintf(bufferName, "capture_in_converted_filter%d", filterType);
            BufferUtils::dumpBuffer(mRGBAInBuffer, bufferName);
        }

        if (mRGBAOutBuffer != nullptr) {
            sprintf(bufferName, "capture_processArgb_filter%d", filterType);
            BufferUtils::dumpBuffer(mRGBAOutBuffer, bufferName);
        }

        sprintf(bufferName, "capture_out_filter%d", filterType);
        BufferUtils::dumpBuffer(out, bufferName);
    }

    FUNCTION_OUT;
    return OK;
}

MERROR Filter::doProcessMatrix(RequestPtr pRequest) {
    FUNCTION_IN;
    if (mHasInit == false) {
        mGLContext = new GLContext();
        mGLContext->initialize(true);
        MY_LOGD("aw_filter_api_init() +");
        mFilterHandle = aw_filter_api_init();
        MY_LOGD("aw_filter_api_init() handle %d -", mFilterHandle);
        mHasInit = true;
    }
    FPS;

    IImageBuffer *in = NULL;
    IImageBuffer *out[9];
    in = pRequest->mIBufferFull->acquire();
    out[0] = pRequest->mOBufferFull->acquire();
    out[1] = pRequest->mOBufferFull0->acquire();
    out[2] = pRequest->mOBufferFull1->acquire();
    out[3] = pRequest->mOBufferFull2->acquire();
    out[4] = pRequest->mOBufferFull3->acquire();
    out[5] = pRequest->mOBufferFull4->acquire();
    out[6] = pRequest->mOBufferFull5->acquire();
    out[7] = pRequest->mOBufferFull6->acquire();
    out[8] = pRequest->mOBufferFull7->acquire();

    mGLContext->enable();
    for (int i = 0; i < 9; i++) {
        drawFilterFrame(in, out[i], (aw_filters_type)i);
    }

    FUNCTION_OUT;
    return OK;
}

void Filter::abort(std::vector<RequestPtr> &pRequests)
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

aw_filters_type Filter::getFilterType(RequestPtr pRequest) {
    MINT32 filterType = 0;
    IMetadata* pImetadata = pRequest->mIMetadataApp->acquire();
    if (pImetadata != nullptr && pImetadata->count() > 0) {
        tryGetMetadata<MINT32>(pImetadata, MTK_POSTALGO_FILTER_TYPE, filterType);
    }
    return (aw_filters_type)filterType;
}

int Filter::getJpegRotation(RequestPtr pRequest) {
    MINT32 jpegRotation = 0;
    IMetadata* pImetadata = pRequest->mIMetadataApp->acquire();
    if (pImetadata != nullptr && pImetadata->count() > 0) {
        tryGetMetadata<MINT32>(pImetadata, MTK_POSTALGO_JPEG_ORIENTATION, jpegRotation);
    }
    return jpegRotation;
}