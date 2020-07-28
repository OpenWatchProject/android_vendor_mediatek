/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include <streammgr/StreamManager.h>

#include <binder/Status.h>
#include <system/window.h>
#include <system/graphics.h>
#include <HalBufferListener.h>
#include <android_runtime/android_view_Surface.h>
#include <gui/BufferItemConsumer.h>
#include <gui/GLConsumer.h>
#include <vendor/mediatek/hardware/mms/1.0/IMms.h>
#include <hidl/HidlSupport.h>
#include <inttypes.h>
#include <processor/ParamConvert.h>
#include <mtk/mtk_platform_metadata_tag.h>

#define LOG_TAG "PostAlgo/StreamMgr"

#ifndef MAX_LOCK_BUFFER_COUNT
#define MAX_LOCK_BUFFER_COUNT 5
#endif

using ::android::hardware::hidl_handle;
using namespace NSPA;
using android::Mutex;
using ::vendor::mediatek::hardware::mms::V1_0::HwCopybitParam;
using ::com::mediatek::campostalgo::ParamConvert;

sp<IStreamManager> IStreamManager::createInstance(std::string name) {
    sp<IStreamManager> sm = new StreamManager(name);
    if (sm != nullptr && sm->initialize() == android::OK) {
        return sm;
    } else {
        return sp<IStreamManager>();
    }
}

auto StreamManager::initialize() -> status_t {
    status_t status = android::OK;
    Mutex::Autolock _l(mOpsLock);
    mStreamCounter = 0;
    mPipeConfigs.add(StreamType::CAPTURE, new PipelineConfigure());
    mPipeConfigs.add(StreamType::PREVIEW, new PipelineConfigure());
    mResultHandler = new ResultHandler(
            android::String8::format("%s-StreamMgr-RstHdl",
                    mInstanceName.c_str()).string());

    if (mResultHandler == nullptr) {
        ALOGE("Bad mResultHandler");
        return android::UNKNOWN_ERROR;
    } else {
        status = mResultHandler->run(mResultHandler->getInstanceName());
        if (OK != status) {
            MY_LOGE("Fail to run the thread %s - status:%d(%s)",
                    mResultHandler->getInstanceName(), status,
                    ::strerror(-status));
            return status;
        }
    }

    mCallbackHandler = new CallbackHandler(
            android::String8::format("%s-StreamMgr-CbHdl",
                    mInstanceName.c_str()).string());
    if (mCallbackHandler == nullptr) {
        ALOGE("Bad mCallbackHandler");
        return android::UNKNOWN_ERROR;
    } else {
        status = mCallbackHandler->run(mCallbackHandler->getInstanceName());
        if (OK != status) {
            MY_LOGE("Fail to run the thread %s - status:%d(%s)",
                    mCallbackHandler->getInstanceName(), status,
                    ::strerror(-status));
            return status;
        }
    }

    ALOGD("StreamManager-%s initialized done!", mInstanceName.c_str());
    return android::OK;
}

StreamManager::~StreamManager() {
    ALOGD("StreamManager destroyed!");
    Mutex::Autolock _l(mOpsLock);
    mPipeConfigs.clear();
}

auto StreamManager::getConfigureLocked(
        const StreamType type) ->sp<PipelineConfigure> {
    switch (type) {
    case StreamType::CAPTURE:
    case StreamType::PREVIEW:
        return mPipeConfigs.valueFor(type);
    default:
        ALOGE("Not support at present with stream type %d", type);
        return nullptr;
    }
}

auto StreamManager::getConfigure(
        const StreamType type) -> sp<PipelineConfigure> {
    Mutex::Autolock _l(mOpsLock);
    return getConfigureLocked(type);

}

int32_t StreamManager::createStreamUniqueId() {
    return android_atomic_inc(&mStreamCounter);
}

#if 0
status_t StreamManager::handleBuffer(sp<android::Surface> sf,
        android::BufferItem item) {
    status_t err = android::OK;
    // frame no & stream id
    sp<ANativeWindow> aNativeWindow(sf);
    ANativeWindowBuffer *buffer = NULL;
    void *dest = NULL;
    void* src = NULL;
    int fenceFd;

    err = aNativeWindow->dequeueBuffer(aNativeWindow.get(), &buffer, &fenceFd);
    if (err != android::NO_ERROR) {
        ALOGE("[%s] dequeue failed (after): %s, return", __func__,
                strerror(-err));
        return err;
    }

    sp<android::GraphicBuffer> ap_gb = android::GraphicBuffer::from(buffer);
    android::PixelFormat ap_pf = ap_gb->getPixelFormat();
    uint32_t ap_width = ap_gb->getWidth();
    uint32_t ap_height = ap_gb->getHeight();
    uint32_t ap_stride = ap_gb->getStride();
    sp<GraphicBuffer>& hal_gb = item.mGraphicBuffer;

    android::PixelFormat hal_pf = hal_gb->getPixelFormat();
    uint32_t hal_width = hal_gb->getWidth();
    uint32_t hal_height = hal_gb->getHeight();
    uint32_t hal_stride = hal_gb->getStride();

    ALOGD("app buffer format is %d[%d*%d-%d], hal format is %d[%d*%d-%d]",
            ap_pf, ap_width, ap_height, ap_stride, hal_pf, hal_width,
            hal_height, hal_stride);

    //TODO: check if yuv_420_888
    if (ap_pf == hal_pf && ap_width == hal_width && ap_height == hal_height
            && ap_stride == hal_stride) {
        err = ap_gb->lock(android::GraphicBuffer::USAGE_SW_WRITE_OFTEN, &dest);
        if (err != android::OK) {
            ALOGE("[%s] lock buffer failed (after): %s, return", __func__,
                    strerror(-err));
            goto finish;
        }

        err = hal_gb->lock(android::GraphicBuffer::USAGE_SW_READ_OFTEN, &src);
        if (err != android::OK) {
            ALOGE("[%s] lock buffer failed (after): %s, return", __func__,
                    strerror(-err));
            goto finish;
        }

        size_t len = (hal_gb->getHeight() * hal_gb->getStride())
                * getBytesPerPixel(hal_pf);

        memcpy(dest, src, len);
    } else {
        //use mdp to copy
        HwCopybitParam hcp = { };
        hcp.inputFormat = hal_pf;
        hcp.inputWidth = hal_width;
        hcp.inputHeight = hal_height;
        hcp.inputHandle = hidl_handle(hal_gb->getNativeBuffer()->handle);
        hcp.outputFormat = ap_pf;
        hcp.outputWidth = ap_width;
        hcp.outputHeight = ap_height;
        hcp.outputHandle = hidl_handle(ap_gb->getNativeBuffer()->handle);

        /// handle buffer
        ///TODO: use mdp instead
        err = android::OK;
        if(mMdpService == nullptr) {
            ALOGE("doesn't support mMdpService!");
            err = android::BAD_VALUE;
        } else if (mMdpService->isCopybitSupportedOutFormat(ap_pf) == false) {
            ALOGE("copybit doesn't support ap format");
            err = android::BAD_VALUE;
        } else if (mMdpService->isCopybitSupportedOutFormat(hal_pf) == false) {
            ALOGE("copybit doesn't support hal format");
            err = android::BAD_VALUE;
        } else {
            ::android::hardware::Return<int32_t> ret = mMdpService->copybit(
                    hcp);
            if (!ret.isOk()) {
                ALOGE("copybit error!");
                err = android::BAD_VALUE;
            } else if (ret != android::OK) {
                ALOGE("copybit error %d", (int32_t )ret);
                err = android::BAD_VALUE;
            }
        }

        if (err == android::BAD_VALUE) {
            size_t ap_len = (ap_gb->getHeight() * ap_gb->getStride())
                    * getBytesPerPixel(ap_pf);
            err = ap_gb->lock(android::GraphicBuffer::USAGE_SW_WRITE_OFTEN,
                    &dest);
            if (err != android::OK) {
                ap_gb->unlock();
                ALOGE("[%s] lock buffer failed (after): %s, return", __func__,
                        strerror(-err));
            } else {
                ALOGE("%s:format buffer error, and format to zero only!", __FUNCTION__);
                memset(dest, 0, ap_len);
            }
        }
    }

    finish: if (err == android::OK) {
        aNativeWindow->queueBuffer(aNativeWindow.get(), buffer, -1);
    }
    ap_gb->unlock();
    return err;
}
#endif

auto StreamManager::updateResult(sp<PA_Request> pr) -> status_t {
    android::AutoMutex _l(mOpsLock);
    ALOGD("update Result with id %x", pr->getRequestNo());
    const StreamType stream_type = (StreamType) pr->getStreamType();
    auto item = pr->getBufferItem();
    sp<ResultItem> params = new ResultItem;
    params->frameNo = pr->mRequestNo;
    params->request = pr;

    status_t res = android::OK;
    res = mResultHandler->enqueResult(params);
    if (res != android::OK) {
        ALOGW("%s:update result error! %s(%d)", __FUNCTION__, strerror(-res),
                res);
        return res;
    }
    sp<FeatureParam> fp = new FeatureParam;
    sp<MetadataPack> mp = pr->getMetadataPack(OUT_PAS);
    if (mp->getMetaPtr()->isEmpty()) {
        ALOGD("return output metadata is null!");
    } else {
        ParamConvert::getInstance()->convertFromTag(*mp->getMetaPtr(), *fp);
    }
    res = mCallbackHandler->enqueResult(fp);
    if (res != android::OK) {
        ALOGW("%s:update callback error! %s(%d)", __FUNCTION__, strerror(-res),
                res);
        return res;
    }
    return res;
}

auto StreamManager::getCommonMetadata(
        const StreamType& type) const -> const IMetadata* {
    return (type == StreamType::CAPTURE) ?
            &mCommonCaptureParams : &mCommonPreviewParams;
}

auto StreamManager::destroy() ->void {
    Mutex::Autolock _l(mOpsLock);
    sp<PipelineConfigure> pc;
    ALOGD("StreamManger destroy %zu!", mPipeConfigs.size());

    mResultHandler->destroy();
    mCallbackHandler->destroy();

    for (int i = 0; i < mPipeConfigs.size(); i++) {
        sp<PipelineConfigure> pc = mPipeConfigs[i];
        if (pc != nullptr) {
            pc->destroy();
        }
    }
    mPipeConfigs.clear();

    ALOGD("PipelineConfigure destroyed!");
}

Status StreamManager::configure(const FeatureConfig& config,
        FeatureResult* result) {
    Status stat = Status::ok();
    status_t res = android::OK;
    Mutex::Autolock _l(mOpsLock);
    const Vector<sp<android::Surface>>& surfaceList = config.getSurfaceList();
    StreamType type = config.getFeaturePipeConfig()->getStreamType();

    const sp<FeatureParam>& sfp = config.getInterfaceParams();
    if (sfp != nullptr) {
        ParamConvert::getInstance()->convertFromParam(*sfp,
                type == StreamType::CAPTURE ?
                        mCommonCaptureParams : mCommonPreviewParams);
    }

    res = configureStreams(type, surfaceList);
    if (res != android::OK) {
        ALOGE("Error to confiureStreams");
        return Status::fromExceptionCode(Status::EX_ILLEGAL_ARGUMENT);
    }

    res = configureHalStreams(type, config.getStreamInfos());
    if (res != android::OK) {
        ALOGE("Error to confiureHalStreams");
        return Status::fromExceptionCode(Status::EX_ILLEGAL_ARGUMENT);
    }

    //return hal streames
    sp<PipelineConfigure> pc = getConfigureLocked(type);
    {
        Mutex::Autolock _l(pc->mPipeConfigLock);
        KeyedVector<int32_t, sp<android::Surface>>& halStreams = pc->mHalStreams;
        for (int i = 0; i < (int) halStreams.size(); i++) {
            result->addStream(
                    new Stream(halStreams.valueAt(i), StreamType::CAPTURE));
        }
    }
    return Status::ok();

}

auto StreamManager::configureHalStreams(const StreamType type,
        const Vector<sp<StreamInfo>> & infos) ->status_t {
    sp<PipelineConfigure> pipeConf = getConfigureLocked(type);
    int width = 480;
    int height = 640;
    int halFormat = HAL_PIXEL_FORMAT_YCBCR_420_888;
    constexpr int maxLockedBuffers = MAX_LOCK_BUFFER_COUNT;
    sp<android::IGraphicBufferProducer> producer;
    sp<android::IGraphicBufferConsumer> consumer;
    status_t res = android::OK;
    android::String8 consumerName;
    Mutex::Autolock _l(pipeConf->mPipeConfigLock);
    if(infos.size() <= 0) {
        ALOGE("configure Hal Streams without any info !");
        return android::BAD_VALUE;
    }
    int usage = 0;
//    if (type == StreamType::CAPTURE) {
//      usage = GraphicBuffer::USAGE_HW_TEXTURE;
//    } else {
//      usage = GraphicBuffer::USAGE_HW_TEXTURE;
////                                        | GraphicBuffer::USAGE_SW_READ_OFTEN;
//    }
    usage = GraphicBuffer::USAGE_HW_TEXTURE;
    android::PublicFormat publicFormat;
    android_dataspace nativeDataspace;
    int32_t id;
    sp<HalBufferListener> bl;
    sp<PABufferItem> bi;

    for (auto si : infos) {
        width = si->mWidth;
        height = si->mHeight;
        publicFormat = static_cast<android::PublicFormat>(si->mFormat);
        halFormat = android::android_view_Surface_mapPublicFormatToHalFormat(
                publicFormat);
        nativeDataspace = android::android_view_Surface_mapPublicFormatToHalDataspace(
                publicFormat);
        consumerName = android::String8::format("CampostAlgo-%dx%df%xm%d-%d",
                width, height, halFormat, maxLockedBuffers, getpid());

        android::BufferQueue::createBufferQueue(&producer, &consumer);
//        consumer->setConsumerUsageBits(GraphicBuffer::USAGE_HW_TEXTURE);
        sp<android::BufferItemConsumer> bic = new android::BufferItemConsumer(
                consumer, usage, maxLockedBuffers, true);
        bic->setName(consumerName);
        res = bic->setDefaultBufferSize(width, height);
        if (res != android::OK) {
            ALOGE("sefaultDefault size error %d/%d, (%d, %s)", width, height,
                    res, strerror(-res));
            return res;
        }
        res = bic->setDefaultBufferFormat(halFormat);
        if (res != android::OK) {
            ALOGE("unable set default buffer format %d, (%d, %s)", halFormat,
                    res, strerror(-res));
            return res;
        }
        res = bic->setDefaultBufferDataSpace(nativeDataspace);
        if (res != android::OK) {
            ALOGE("unable set default buffer database %d, (%d, %s)",
                    (int32_t )nativeDataspace, res, strerror(-res));
            return res;
        }

        id = createStreamUniqueId();
        pipeConf->mHalStreamMap.add(
                android::IInterface::asBinder(producer.get()), id);
        auto hal_sf = new android::Surface(producer, true);
        pipeConf->mHalStreams.add(id, hal_sf);
        bl = new HalBufferListener(hal_sf, bic, type);
        bic->setFrameAvailableListener(bl);
        bi = new PABufferItem(consumer, producer, bic, bl);
        pipeConf->mPABuffers.add(id, bi);
    }

    return android::OK;
}

void StreamManager::addInterfaceCallback(
        const ::android::sp<ICamPostAlgoCallback>& callback) {
    Mutex::Autolock _l(mOpsLock);
    mCallbackHandler->addInterfaceCallback(callback);
}

auto StreamManager::configureStreams(const StreamType type,
        const Vector<sp<android::Surface>>& appSurfaces) -> status_t {
    sp<PipelineConfigure> pipeConf;
    pipeConf = getConfigureLocked(type);
    //check ORIENTATION and mirror
    const IMetadata& metadataPtr =
            (type == StreamType::CAPTURE) ?
                    mCommonCaptureParams : mCommonPreviewParams;
    const IMetadata::IEntry& ieSensorOrientation = metadataPtr.entryFor(
            MTK_POSTALGO_SENSOR_ORIENTATION);
    const IMetadata::IEntry& ieLensFacing = metadataPtr.entryFor(
            MTK_POSTALGO_LENS_FACING);
    const IMetadata::IEntry& ieJpegOrientation = metadataPtr.entryFor(
            MTK_POSTALGO_JPEG_ORIENTATION);
    int32_t orientation = 0;
    int32_t lensFacing = 0;
    int32_t transform = 0;
    int32_t jpegOrientation = 0;

    if (!ieSensorOrientation.isEmpty()) {
        orientation = ieSensorOrientation.itemAt(0, Type2Type< MINT32 >());
        ALOGD("check sensor orientation %d", orientation);
    }

    if (! ieLensFacing.isEmpty()) {
        lensFacing = ieLensFacing.itemAt(0, Type2Type< MINT32 >());
        ALOGD("check facing orientation %d", lensFacing);
    }

    if (!ieJpegOrientation.isEmpty()) {
        jpegOrientation = ieJpegOrientation.itemAt(0, Type2Type<MINT32>());
        ALOGD("check jpeg orientation %d", jpegOrientation);
    }
    int32_t flags = 0;
    if (lensFacing == PAS_LENS_FACING_FRONT) {
        // Front camera needs to be horizontally flipped for mirror-like behavior.
        // Note: Flips are applied before rotates; using XOR here as some of these flags are
        // composed in terms of other flip/rotation flags, and are not bitwise-ORable.
        switch (orientation) {
            case 0:
                flags = NATIVE_WINDOW_TRANSFORM_FLIP_H;
                break;
            case 90:
                flags = NATIVE_WINDOW_TRANSFORM_FLIP_H ^
                        NATIVE_WINDOW_TRANSFORM_ROT_270;
                break;
            case 180:
                flags = NATIVE_WINDOW_TRANSFORM_FLIP_H ^
                        NATIVE_WINDOW_TRANSFORM_ROT_180;
                break;
            case 270:
                flags = NATIVE_WINDOW_TRANSFORM_FLIP_H ^
                        NATIVE_WINDOW_TRANSFORM_ROT_90;

                break;
            default:
                ALOGE("%s: Invalid pas.sensor.orientation value: %d",
                      __FUNCTION__, orientation);
                break;
        }
    } else {
        switch (orientation) {
            case 0:
                flags = 0;
                break;
            case 90:
                flags = NATIVE_WINDOW_TRANSFORM_ROT_90;
                break;
            case 180:
                flags = NATIVE_WINDOW_TRANSFORM_ROT_180;
                break;
            case 270:
                flags = NATIVE_WINDOW_TRANSFORM_ROT_270;
                break;
            default:
                ALOGE("%s: Invalid pas.sensor.orientation value: %d",
                      __FUNCTION__, orientation);
                break;
        }
    }

    {
        Mutex::Autolock _ll(pipeConf->mPipeConfigLock);

        for (auto surface : appSurfaces) {
            if (pipeConf->mAppStreamMap.indexOfKey(
                    android::IInterface::asBinder(
                            surface->getIGraphicBufferProducer())) >= 0) {
                ALOGE("Surface has been added!");
                return android::BAD_VALUE;
            }
            if (!android::Surface::isValid(surface)) {
                ALOGE("Surface is not valid!");
                return android::BAD_VALUE;
            }

            ANativeWindow* anw = (ANativeWindow*) surface.get();
            //remember to disconnect
            //TODO: check this, if purpose
            int err = native_window_api_connect(anw,
                    NATIVE_WINDOW_API_CPU);

            if (err != android::NO_ERROR) {
                ALOGE("[%s] connect failed: %s, return", __func__,
                        strerror(-err));
                return android::BAD_VALUE;
            }

            int32_t ap_format = ANativeWindow_getFormat(anw);
            if (type == StreamType::PREVIEW) {
                //set app preview surface buffer count is 3.
                err = native_window_set_buffer_count(anw, MAX_LOCK_BUFFER_COUNT);
                if (err != android::NO_ERROR) {
                    ALOGE(
                            "[%s] Unable to native_window_set_buffer_count: %s (%d)",
                            __func__, strerror(-err), err);
                }
                if (ap_format == HAL_PIXEL_FORMAT_RGB_565) {
                    //set app preview surface buffer format is RGBA8888.
                    ALOGD("set app preview surface buffer format is RGBA8888");
                    err = native_window_set_buffers_format(anw, HAL_PIXEL_FORMAT_RGBA_8888);
                    if (err != android::NO_ERROR) {
                        ALOGE(
                                "[%s] Unable to native_window_set_buffers_format: %s (%d)",
                                __func__, strerror(-err), err);
                        status_t ret_mode =
                                native_window_set_scaling_mode(anw,
                                                               NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
                        if (ret_mode != android::OK) {
                            ALOGE("[%s] Unable to native_window_set_scaling_mode: %s (%d)",
                                  __func__, strerror(-err), err);
                        }
                    }
                }
            }

            int32_t width = -1;
            int32_t height = -1;
            if (ap_format == HAL_PIXEL_FORMAT_BLOB) {
                ALOGD("change w*h for blob");
                width = ANativeWindow_getWidth(anw);
                height = ANativeWindow_getHeight(anw);
                int32_t maxSize = height * width;
                err = native_window_set_buffers_dimensions(anw,
                        maxSize, 1);
            }

            ///TODO: if front lens, need do mirror
            ///int32_t transform = NATIVE_WINDOW_TRANSFORM_FLIP_H ^ NATIVE_WINDOW_TRANSFORM_ROT_90 || NATIVE_WINDOW_TRANSFORM_INVERSE_DISPLAY;
            if (flags != 0) {
                transform = flags | NATIVE_WINDOW_TRANSFORM_INVERSE_DISPLAY;
                err = native_window_set_buffers_transform(anw, transform);
                if (err != android::NO_ERROR) {
                    ALOGW("%d: Change transform error!", type);
                } else {
                    ALOGD("%d: change buffer transform %d", type, transform);
                }
            }

            uint64_t u = 0;
            status_t res;
            res =  native_window_get_consumer_usage(anw, &u);
            if (res == android::OK) {
                uint64_t usage = u | GRALLOC_USAGE_SW_READ_OFTEN
                        | GRALLOC_USAGE_SW_WRITE_OFTEN;
                native_window_set_usage(anw, usage);
                ALOGD("change usage from %" PRIu64 " to %" PRIu64, u, usage);
            }


            int32_t id = createStreamUniqueId();
            pipeConf->mAppStreamMap.add(
                    android::IInterface::asBinder(surface->getIGraphicBufferProducer()),
                    id);
            pipeConf->mAppStreamInfoMap.add(id,
                    new ImageStreamInfo(
                            android::String8::format("%s-i%d-w%dxh%d-f%d-t%d",
                                    getStreamTypeName(type), id, width, height,
                                    ap_format, transform).string(), type, id, width,
                            height, surface, transform));
        }

    }
    return android::OK;

}
