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

#include <gui/GLConsumer.h>
#include <vendor/mediatek/hardware/mms/1.0/IMms.h>
#include <hidl/HidlSupport.h>
#include <system/window.h>
#include <android/native_window.h>
#include <utils/gralloc/IGrallocHelper.h>
#include <vndk/hardware_buffer.h>
#include <android_runtime/android_view_Surface.h>
#include <utils/ImageBufferHelper.h>
#include <mtk/mtk_platform_metadata_tag.h>
#include <streammgr/StreamManager.h>


#define LOG_TAG "PostAlgo/RstHdl"

#define ThisNamespace   StreamManager::ResultHandler


using namespace NSPA;
using android::Mutex;
using ::android::hardware::hidl_handle;
using ::vendor::mediatek::hardware::mms::V1_0::HwCopybitParam;
using ::vendor::mediatek::hardware::mms::V1_0::HwJpegEncoderConfig;
using ::NSCam::GrallocStaticInfo;

#define DUMP_BUFFER_CAPTURE     "debug.pas.result.dumpcapture.enabled"
#define DUMP_BUFFER_PREVIEW     "debug.pas.result.dumppreview.enabled"
#define DUMP_BUFFER_JPEG        "debug.pas.result.dumpjpeg.enabled"
static int gDumpBufferCaptureEnabled = ::property_get_int32(DUMP_BUFFER_CAPTURE, 0);
static int gDumpBufferPreviewEnabled = ::property_get_int32(DUMP_BUFFER_PREVIEW, 0);
static int gDumpBufferJpegEnabled = ::property_get_int32(DUMP_BUFFER_JPEG, 0);

/******************************************************************************
 *
 ******************************************************************************/
ThisNamespace ::ResultHandler() {
}

/******************************************************************************
 *
 ******************************************************************************/
auto ThisNamespace::destroy() -> void
{
    ALOGD("%s: ResultHandler destroyed!", __FUNCTION__);

    this->requestExit();
    this->join();
    mMdpService = nullptr;
    delete mTimer;
}

/******************************************************************************
 *
 ******************************************************************************/
// Good place to do one-time initializations
auto ThisNamespace::readyToRun() -> status_t
{
    mMdpService = IMms::tryGetService();

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
auto ThisNamespace::requestExit() -> void
{
    MY_LOGD("+ %s", __FUNCTION__);
    //
    {
        Mutex::Autolock _l(mResultQueueLock);
        Thread::requestExit();
        mResultQueueCond.broadcast();
    }
    //
    MY_LOGD("- %s", __FUNCTION__);
}

status_t ThisNamespace::handleBuffer(sp<android::Surface> sf,
        sp<android::GraphicBuffer> src_gb, android::PixelFormat real_format) {
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

    android::PixelFormat hal_pf = src_gb->getPixelFormat();
    if(real_format != android::PIXEL_FORMAT_UNKNOWN) {
        hal_pf = real_format;
    }
    uint32_t hal_width = src_gb->getWidth();
    uint32_t hal_height = src_gb->getHeight();
    uint32_t hal_stride = src_gb->getStride();

    ALOGD("app buffer format is %d[%d*%d-%d], hal format is %d[%d*%d-%d]",
            ap_pf, ap_width, ap_height, ap_stride, hal_pf, hal_width,
            hal_height, hal_stride);

    if ((ap_pf == hal_pf || ap_pf == HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED)
            && ap_width == hal_width && ap_height == hal_height
            && ap_stride == hal_stride) {
        err = ap_gb->lock(android::GraphicBuffer::USAGE_SW_WRITE_OFTEN, &dest);
        if (err != android::OK) {
            ALOGE("[%s] lock buffer failed (after): %s, return", __func__,
                    strerror(-err));
            goto finish;
        }

        err = src_gb->lock(android::GraphicBuffer::USAGE_SW_READ_OFTEN, &src);
        if (err != android::OK) {
            ALOGE("[%s] lock buffer failed (after): %s, return", __func__,
                    strerror(-err));
            goto finish;
        }

        size_t len = (src_gb->getHeight() * src_gb->getStride())
        * getBytesPerPixel(hal_pf);

        memcpy(dest, src, len);
    }else if (ap_pf == HAL_PIXEL_FORMAT_BLOB) {
        ALOGD("Handle blob for jpeg");
        // need jpeg, and do jpeg handle
        /*
         *  1. resize
         *  2. format
         */

        if(mMdpService == nullptr) {
            ALOGW("doesn't have MDP for JPEG");
            err = android::BAD_VALUE;
            goto finish;
        }
        android::hardware::Return<int32_t> ret = NULL;
        HwJpegEncoderConfig cfg;

        cfg.width = ap_width;
        cfg.height = ap_height;
        //define jpeg quality
        cfg.level = 90;
        cfg.format = hal_pf;// HAL_PIXEL_FORMAT_YCbCr_422_I;
        cfg.rotation = 0;
        cfg.input = hidl_handle(src_gb->getNativeBuffer()->handle);
        cfg.output = hidl_handle(ap_gb->getNativeBuffer()->handle);
        ret = mMdpService->jpegOpenClient(ap_width,ap_height);
        uint64_t outSize = 0;
        int32_t result = 0;
        android::hardware::Return<void> jpeg_ret = mMdpService->jpegencode(cfg,
                [&outSize, &result] (uint64_t _size, int32_t _result) {
                    result = _result;
                    outSize = _size;
                }
        );
        if (jpeg_ret.isOk()) {
            ALOGD("compress jpeg done!");
        } else {
            err = android::BAD_VALUE;
        }
        ret = mMdpService->jpegCloseClient();
    } else {
        //use mdp to copy
        HwCopybitParam hcp = {
            .inputFormat = hal_pf,
            .inputWidth = hal_width,
            .inputHeight = hal_height,
            .inputHandle = hidl_handle(src_gb->getNativeBuffer()->handle),
            .outputFormat = ap_pf,
            .outputWidth = ap_width,
            .outputHeight = ap_height,
            .outputHandle = hidl_handle(ap_gb->getNativeBuffer()->handle)
        };

        /// handle buffer
        ///TODO: how to handle app request 420_888, implemente define?
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
            *getBytesPerPixel(ap_pf);
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

finish:

    if (err == android::OK) {
        aNativeWindow->queueBuffer(aNativeWindow.get(), buffer, -1);
    }
    ap_gb->unlock();
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
auto ThisNamespace::threadLoop() -> bool
{
    status_t err = android::OK;
    uint32_t id = 0;
    int32_t cformat = -1;
    ANativeWindowBuffer *anwb = nullptr;

    ResultQueueT vResult;
    err = dequeResult(vResult);
    if ( OK == err && ! vResult.isEmpty() )
    {
        for(int i = 0; i < vResult.size(); i++)
        {
            ///enqueue buffer and release buffer
            /// if jpeg, we need handle jpeg format
            sp<PA_Request>& pr = vResult[i]->request;
            id = pr->getRequestNo();
            StreamType stype = (StreamType)pr->getStreamType();
            for(const auto& imgInfoPair:pr->mImg) {
                if (imgInfoPair.first == IN_FULL || IN_FULL_YUV == imgInfoPair.first) {
                    continue;
                }
                auto bufInfo = imgInfoPair.second;
                ANativeWindow* aNativeWindow(bufInfo->mSurface.get());
                android::sp<GraphicBuffer> w_gbf(bufInfo->mGbf);
                if (/*stype == StreamType::CAPTURE &&*/ (cformat = ANativeWindow_getFormat(aNativeWindow))
                        == HAL_PIXEL_FORMAT_BLOB) {
                    ALOGD("handle jpeg working buffer %d", pr->getRequestNo());
                    int32_t fenceFd = -1;
                    if (stype == StreamType::PREVIEW) {
                        const auto& metaPack = pr->getMetadataPack(OUT_PAS);
                        MINT32 state = AUTORAMA_UNKNOWN;
                        const auto& entry = metaPack->entryFor(MTK_POSTALGO_AUTORAMA_CMD);
                        if (entry.count() <= 0 || (state = entry.itemAt(0, Type2Type<MINT32>()) != AUTORAMA_COMPLETE)) {
                            ALOGD("r%d Autorama[%d] wait for complete!", pr->getRequestNo(), state);
                            continue;
                        }
                    }
                    if(mMdpService == nullptr) {
                         ALOGW("doesn't have MDP for JPEG");
                         err = android::BAD_VALUE;
                         continue;
                     }
                    err = aNativeWindow->dequeueBuffer(aNativeWindow, &anwb,
                            &fenceFd);
                    ALOGD("capture releaseFenceFd = %d", fenceFd);
                    if (fenceFd > 0) {
                        sp<Fence> releaseFence(new Fence(fenceFd));
                        releaseFence->waitForever("postAlgo capture releaseFence");
                    }
                    if(err != android::OK) {
                        ALOGE("Errot to dequeue buffer from ap %p", bufInfo->mSurface.get());
                        continue;
                    }
                    int32_t width = ANativeWindow_getWidth(aNativeWindow);
                    int32_t height = ANativeWindow_getHeight(aNativeWindow);
                    android::hardware::Return<int32_t> ret = NULL;
                    HwJpegEncoderConfig cfg;

                    cfg.width = w_gbf->getWidth();
                    cfg.height = w_gbf->getHeight();
                    //define jpeg quality
                    cfg.level = 90;
                    android::PixelFormat w_format = w_gbf->getPixelFormat();
                    cfg.format = static_cast<uint32_t>(w_format);// HAL_PIXEL_FORMAT_YCbCr_422_I;
                    cfg.rotation = 0;
                    cfg.input = hidl_handle(w_gbf->getNativeBuffer()->handle);
                    cfg.output = hidl_handle(AHardwareBuffer_getNativeHandle(ANativeWindowBuffer_getHardwareBuffer(anwb)));
                    ///dump jpeg input
                    if (gDumpBufferCaptureEnabled) {
                        dumpBuffer(w_gbf, android::String8::format("rst-t%s-r%d-",getStreamTypeName(stype), id).string());
                    }
                    ///

                    ret = mMdpService->jpegOpenClient(w_gbf->getWidth(),w_gbf->getHeight());
                    uint64_t outSize = 0;
                    int32_t result = 0;
                    android::hardware::Return<void> jpeg_ret = mMdpService->jpegencode(cfg,
                            [&outSize, &result] (uint64_t _size, int32_t _result) {
                                result = _result;
                                outSize = _size;
                            }
                    );
                    ALOGD("after handle jpeg %dx%d -> working buffer %dx%d, outSize = %llu",
                            width, height, w_gbf->getWidth(), w_gbf->getHeight(), (unsigned long long)outSize);

                    status_t res;
                    void *dstBuffer;
                    sp<GraphicBuffer> gb = GraphicBuffer::from(anwb);
                    res = gb->lockAsync(GRALLOC_USAGE_SW_WRITE_OFTEN, &dstBuffer, fenceFd);
                    if (res != OK) {
                            ALOGE("%s: Error trying to lock output buffer fence: %s (%d)", __FUNCTION__,
                                    strerror(-res), res);
                     }

                    uint8_t* header = static_cast<uint8_t *> (dstBuffer) +
                            (gb->getWidth() - sizeof(struct camera3_jpeg_blob));
                    struct camera3_jpeg_blob *blob = reinterpret_cast<struct camera3_jpeg_blob*> (header);
                    blob->jpeg_blob_id = CAMERA3_JPEG_BLOB_ID;
                    blob->jpeg_size = outSize;

                    if (jpeg_ret.isOk()) {
                        if (gDumpBufferJpegEnabled) {
                            dumpJpegBuffer(android::GraphicBuffer::from(anwb), android::String8::format("rst-t%s-r%d-",getStreamTypeName(stype), id).string());
                        }
                        ALOGD("r[%d] compress jpeg done!", pr->getFrameCount());
                    } else {
                        err = android::BAD_VALUE;
                    }
                     ret = mMdpService->jpegCloseClient();
                     aNativeWindow->queueBuffer(aNativeWindow, anwb, -1);
                } else {
                    anwb = bufInfo->mGbf.get();
                    if ((stype == StreamType::CAPTURE && gDumpBufferCaptureEnabled) || (stype == StreamType::PREVIEW && gDumpBufferPreviewEnabled)) {
                        //dump buffer
                        dumpBuffer(android::GraphicBuffer::from(anwb), android::String8::format("rst-t%s-r%d-",getStreamTypeName(stype), id).string());
                    }
                    ALOGD("acquireFenceFd = %d",bufInfo->mImg->acquireFenceFd);
                    if (bufInfo->mImg->acquireFenceFd > 0) {
                        sp<android::Fence> acquireFence = new android::Fence(bufInfo->mImg->acquireFenceFd);
                        acquireFence->waitForever("postAlgo preview acquireFence");
                        bufInfo->mImg->acquireFenceFd = -1;
                    }
                    aNativeWindow->queueBuffer(aNativeWindow, anwb, -1);
                    mFpsCounter->update(mTimer->getTimeSpec());
                    double fps = mFpsCounter->getFPS();
                    ALOGD("PostAlgo FPS = %lf", fps);
                }
            }
            pr->mImg.clear();
#if 0
            sp<PA_Request>& pr = vResult[i]->request;
            id = pr->getRequestNo();
            android_dataspace nativeDataspace;
            const StreamType stream_type = (StreamType) pr->getStreamType();
            const auto& item = pr->mBI;
            auto fp = vResult[i]->pipeConfigure;
            ALOGD("update Result with No: %d", pr->getRequestNo());
            //enqueue buffer and release buffer

            {
                void *ptr = NULL;
                void* src = NULL;

                //check hal buffer format
                android::PixelFormat real_format = android::PIXEL_FORMAT_UNKNOWN;
                const sp<GraphicBuffer>& hal_gb = item.mGraphicBuffer;
                android::PixelFormat pf = hal_gb->getPixelFormat();
                //TODO: handle specific pixel format
                if (pf == HAL_PIXEL_FORMAT_YCBCR_420_888 || pf == HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED) {
                    ANativeWindowBuffer* buffer = hal_gb->getNativeBuffer();
                    GrallocStaticInfo staticInfo;
                    status_t status = IGrallocHelper::singleton()->query(buffer->handle, &staticInfo, NULL);
                    real_format = staticInfo.format;
                    ALOGV("query 420_888:%d real format is %d", HAL_PIXEL_FORMAT_YCBCR_420_888, staticInfo.format);
                }

                KeyedVector<int32_t, sp<android::Surface>> app_sfs;
                {
                    Mutex::Autolock _(fp->mPipeConfigLock);
                    app_sfs = fp->mAppStreams;
                }

                for (int i = 0; i < app_sfs.size(); i++) {
                    //TODO:should we use thread pool to handle buffer?
                    /*
                     * 1. directly copy
                     * 2. MDP to transform
                     * 3. JPEG to compress
                     */
                    err = handleBuffer(app_sfs.valueAt(i), item.mGraphicBuffer, real_format);
                    if (err != android::OK) {
                        ALOGE("%s:handle buffer error! %s:%d", __FUNCTION__,
                                strerror(-err), err);
                        continue;
                    }

                }

            }
#endif
            ///let release buffer and return to queue
            pr->getHalConsumer()->releaseBuffer(pr->mBI);
            pr = nullptr;
            ALOGD("update Request result finished  %d", id);

        }
    }

    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
auto ThisNamespace::dequeResult(ResultQueueT& rvResult) -> status_t
{
    status_t err = OK;
    Mutex::Autolock _l(mResultQueueLock);

    while ( ! exitPending() && mResultQueue.isEmpty() ) {
        err = mResultQueueCond.wait(mResultQueueLock);
        MY_LOGW_IF(
                OK != err,
                "exitPending:%d ResultQueue#:%zu err:%d(%s)",
                exitPending(), mResultQueue.size(), err, ::strerror(-err)
        );
    }

    if ( mResultQueue.isEmpty() ) {
        ALOGD("empty queue");
        rvResult.clear();
        err = NOT_ENOUGH_DATA;
    } else {
        //  If the queue is not empty, deque all items from the queue.
        rvResult = mResultQueue;
        mResultQueue.clear();
        err = OK;
    }
    return err;
}

/******************************************************************************
 *
 ******************************************************************************/
auto ThisNamespace::enqueResult(sp<ResultItem> const& pItem) -> status_t
{
    auto const frameNo = pItem->frameNo;

    //
    if ( exitPending() ) {
        MY_LOGW("Dead ResultQueue");
        return DEAD_OBJECT;
    }
    {
        Mutex::Autolock _l(mResultQueueLock);

        //
        ssize_t const index = mResultQueue.indexOfKey(frameNo);
        if ( 0 <= index ) {
            sp<ResultItem> item = mResultQueue.editValueAt(index);
            MY_LOGW( "frameNo:%u existed in mResultQueue[%zd]", frameNo, index );
            item->pipeConfigure = pItem->pipeConfigure;
            item->request = pItem->request;
        }
        else {
            mResultQueue.add(frameNo, pItem);
        }
        mResultQueueCond.broadcast();
    }
    ALOGD("enque result[%d] done!", pItem->frameNo);
    //
    return OK;
}

