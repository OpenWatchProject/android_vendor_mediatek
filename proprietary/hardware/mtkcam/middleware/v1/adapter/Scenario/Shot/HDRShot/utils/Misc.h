/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_HDRSHOT_UTILS_MISC_H_
#define _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_HDRSHOT_UTILS_MISC_H_

#include <mtkcam/middleware/v1/IShot.h>

#include <mtkcam/utils/metadata/IMetadataConverter.h>

#include <mtkcam/middleware/v1/camshot/BufferCallbackHandler.h>

#include <mtkcam/middleware/v1/LegacyPipeline/ILegacyPipeline.h>
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>

#include <mtkcam/feature/hdr/utils/Debug.h>

using namespace android;
using namespace NSCam::v1;
using namespace NSCam::v1::NSLegacyPipeline;
using namespace NSShot;

// ---------------------------------------------------------------------------

static sp<CallbackBufferPool> prepareCallbackBufferPool(
        sp<BufferCallbackHandler>& callbackHandler,
        const sp<IImageStreamInfo>& streamInfo,
        bool allocateBuffer)
{
    sp<CallbackBufferPool> cbBufferPool = new CallbackBufferPool(streamInfo);

    if (allocateBuffer)
    {
        cbBufferPool->allocateBuffer(
                streamInfo->getStreamName(),
                streamInfo->getMaxBufNum(),
                streamInfo->getMinInitBufNum());
    }
    callbackHandler->setBufferPool(cbBufferPool);

    return cbBufferPool;
}

static void onSelectedResultReceived(
        const sp<ILegacyPipeline>& pipeline, const IMetadata& result)
{
    if (pipeline == NULL)
    {
        HDR_LOGE("pipeline is NULL");
        return;
    }

    sp<TimestampProcessor> timestampProcessor =
        pipeline->getTimestampProcessor().promote();
    if (timestampProcessor.get())
    {
        timestampProcessor->onResultReceived(
                0,
                eSTREAMID_META_APP_DYNAMIC_P1,
                MFALSE,
                result);
    }
    else
    {
        HDR_LOGE("promote timestamp processor failed");
    }
}

static void onFakeJpegCallback(const sp<IShotCallback>& callback)
{
    if (callback == NULL)
    {
        HDR_LOGE("shot callback is null");
        return;
    }

    HDR_LOGW("send dummy jpeg callback");

    const uint32_t jpegSize = 512;
    const uint8_t  jpegBuffer[jpegSize] {};
    const uint32_t exifHeaderSize = 512;
    const uint8_t  exifHeaderBuffer[exifHeaderSize] {};

    callback->onCB_CompressedImage(
            0, jpegSize, jpegBuffer, exifHeaderSize, exifHeaderBuffer, 0, true);
}

// used to check if applying single or multi-frame capture
static inline bool isSingleCapture(
        const size_t captureFrames, const size_t delayedFrames = 0)
{
    return (captureFrames == (1 + delayedFrames));
}

static void dumpBuffer(
        const sp<IImageBuffer>& buffer,
        const char* fileName, const char* dumpPath = HDR_DUMP_PATH)
{
    if ((buffer == NULL) || (fileName == NULL) || (dumpPath == NULL))
    {
        HDR_LOGW("invalid dump(buffer(%p), fileName(%s) dumpPath(%s)",
                buffer.get(),
                (fileName == NULL) ? "not set" : fileName,
                (dumpPath == NULL) ? "not set" : dumpPath);
        return;
    }

    String8 name(dumpPath);
    name.appendFormat("%s", fileName);

    buffer->lockBuf(DEBUG_LOG_TAG, eBUFFER_USAGE_SW_READ_MASK);
    buffer->saveToFile(name);
    buffer->unlockBuf(DEBUG_LOG_TAG);
}

auto dumpImageStreamInfo = [](const sp<IImageStreamInfo>& info)
{
    HDR_LOGD("streamInfo(%#" PRIxPTR") name(%s) format(%#08x) size(%dx%d) min/max(%zu/%zu)",
            info->getStreamId(), info->getStreamName(),
            info->getImgFormat(), info->getImgSize().w, info->getImgSize().h,
            info->getMinInitBufNum(), info->getMaxBufNum());
};

auto dumpMetadata = [](const IMetadata& metadata, const char* msg = "")
{
    sp<IMetadataConverter> metadataConverter =
        IMetadataConverter::createInstance(
                IDefaultMetadataTagSet::singleton()->getTagSet());

    HDR_LOGD("dump %s metadata(%p)", msg, &metadata);
    // list all settings
    if (metadataConverter.get())
        metadataConverter->dumpAll(metadata);
};

#endif // _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_HDRSHOT_UTILS_MISC_H_
