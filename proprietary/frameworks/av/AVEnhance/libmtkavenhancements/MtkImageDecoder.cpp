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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#define LOG_TAG "MtkImageDecoder"
#include <media/stagefright/foundation/AString.h>
#include <media/stagefright/foundation/ABase.h>
#include <media/openmax/OMX_Video.h>
#include <system/graphics-base.h>

#include "MtkImageDecoder.h"
#include <binder/MemoryBase.h>
#include <binder/MemoryHeapBase.h>
#include <media/stagefright/Utils.h>
#include <media/stagefright/MediaBuffer.h>
#include <private/media/VideoFrame.h>
#include <media/MediaCodecBuffer.h>
#include "OMX_IVCommon.h"
#include <gui/Surface.h>
#include <media/ICrypto.h>


#define VDEC_ROUND_N(X, N)   ((X + (N-1)) & (~(N-1)))
static const int64_t kBufferTimeOutUs = 10000ll;
static const size_t kRetryCount = 1000;  // must be >0
static const size_t kGridNum = 8;

namespace android {
sp<IMemory> allocHeicVideoFrame(const sp<MetaData>& trackMeta,
        int32_t width, int32_t height, int32_t tileWidth, int32_t tileHeight,
        int32_t dstBpp, bool metaOnly = false) {
    int32_t rotationAngle;
    if (!trackMeta->findInt32(kKeyRotation, &rotationAngle)) {
        rotationAngle = 0;  // By default, no rotation
    }
    uint32_t type;
    const void *iccData;
    size_t iccSize;
    if (!trackMeta->findData(kKeyIccProfile, &type, &iccData, &iccSize)) {
        iccData = NULL;
        iccSize = 0;
    }

    int32_t sarWidth, sarHeight;
    int32_t displayWidth, displayHeight;
    if (trackMeta->findInt32(kKeySARWidth, &sarWidth)
            && trackMeta->findInt32(kKeySARHeight, &sarHeight)
            && sarHeight != 0) {
        displayWidth = (width * sarWidth) / sarHeight;
        displayHeight = height;
    } else if (trackMeta->findInt32(kKeyDisplayWidth, &displayWidth)
                && trackMeta->findInt32(kKeyDisplayHeight, &displayHeight)
                && displayWidth > 0 && displayHeight > 0
                && width > 0 && height > 0) {
        ALOGV("found display size %dx%d", displayWidth, displayHeight);
    } else {
        displayWidth = width;
        displayHeight = height;
    }

    VideoFrame frame(width, height, displayWidth, displayHeight,
            tileWidth, tileHeight, rotationAngle, dstBpp, !metaOnly, iccSize);

    size_t size = frame.getFlattenedSize();
    sp<MemoryHeapBase> heap = new MemoryHeapBase(size, 0, "MetadataRetrieverClient");
    if (heap == NULL) {
        ALOGE("failed to create MemoryDealer");
        return NULL;
    }
    sp<IMemory> frameMem = new MemoryBase(heap, 0, size);
     if (frameMem == NULL || (frameMem->pointer() == NULL)) {
        ALOGE("not enough memory for VideoFrame size=%zu", size);
        return NULL;
    }
    VideoFrame* frameCopy = static_cast<VideoFrame*>(frameMem->pointer());
    frameCopy->init(frame, iccData, iccSize);

    return frameMem;
}

bool findHeiftestInfo(
        const sp<MetaData> &trackMeta, int32_t *width, int32_t *height,
        uint32_t *type = NULL, const void **data = NULL, size_t *size = NULL) {
    uint32_t dummyType;
    const void *dummyData;
    size_t dummySize;
    return trackMeta->findInt32(kKeyThumbnailWidth, width)
        && trackMeta->findInt32(kKeyThumbnailHeight, height)
        && trackMeta->findData(kKeyThumbnailHVCC,
                type ?: &dummyType, data ?: &dummyData, size ?: &dummySize);
}

bool findHeifGridInfo(const sp<MetaData> &trackMeta,
        int32_t *tileWidth, int32_t *tileHeight, int32_t *gridRows, int32_t *gridCols) {
    return trackMeta->findInt32(kKeyTileWidth, tileWidth) && (*tileWidth > 0)
        && trackMeta->findInt32(kKeyTileHeight, tileHeight) && (*tileHeight > 0)
        && trackMeta->findInt32(kKeyGridRows, gridRows) && (*gridRows > 0)
        && trackMeta->findInt32(kKeyGridCols, gridCols) && (*gridCols > 0);
}

bool getDstColorFormatExt(
        android_pixel_format_t colorFormat,
        OMX_COLOR_FORMATTYPE *dstFormat,
        int32_t *dstBpp) {
    switch (colorFormat) {
        case HAL_PIXEL_FORMAT_RGB_565:
        {
            *dstFormat = OMX_COLOR_Format16bitRGB565;
            *dstBpp = 2;
            return true;
        }
        case HAL_PIXEL_FORMAT_RGBA_8888:
        {
            *dstFormat = OMX_COLOR_Format32BitRGBA8888;
            *dstBpp = 4;
            return true;
        }
        case HAL_PIXEL_FORMAT_BGRA_8888:
        {
            *dstFormat = OMX_COLOR_Format32bitBGRA8888;
            *dstBpp = 4;
            return true;
        }
        default:
        {
            ALOGE("Unsupported color format: %d", colorFormat);
            break;
        }
    }
    return false;
}

MtkImageDecoder::MtkImageDecoder(
        const AString &componentName,
        const sp<MetaData> &trackMeta,
        const sp<IMediaSource> &source)
    : ImageDecoder(componentName, trackMeta, source),
      mComponentName(componentName),
      mTrackMeta(trackMeta),
      mSource(source),
      mDstFormat(OMX_COLOR_Format16bitRGB565),
      mDstBpp(2),
      mHaveMoreInputs(true),
      mFirstSample(true),
      mIsRGBReady(false),
      mIsAllDone(false),
      mIsStop(false),
      mIsEos(false),
      mFrame(NULL),
      mWidth(0),
      mHeight(0),
      mGridRows(1),
      mGridCols(1),
      mInputCount(0),
      mGridNum(kGridNum),
      mTileWidth(0),
      mTileHeight(0),
      mTilesDecoded(0),
      mTargetTiles(0),
      mThread(NULL) {
}
struct MtkImageDecoder::DecodeThread : public Thread {
    explicit DecodeThread(MtkImageDecoder *decoder) : mDecoder(decoder) {}
    private:
        MtkImageDecoder* mDecoder;
        bool threadLoop();
        DISALLOW_EVIL_CONSTRUCTORS(DecodeThread);
};

bool MtkImageDecoder::DecodeThread::threadLoop() {
    return mDecoder->receiveheifouputbuffer();
}
status_t MtkImageDecoder::init_ext(
        int64_t frameTimeUs, size_t numFrames, int option, int colorFormat) {
    if (!getDstColorFormatExt(
            (android_pixel_format_t)colorFormat, &mDstFormat, &mDstBpp)) {
        return ERROR_UNSUPPORTED;
    }

    sp<AMessage> videoFormat = onGetFormatAndSeekOptions(
            frameTimeUs, numFrames, option, &mReadOptions);
    if (videoFormat == NULL) {
        ALOGE("video format or seek mode not supported");
        return ERROR_UNSUPPORTED;
    }
    videoFormat->setInt32("GridNum", mGridNum);

    status_t err;
    sp<ALooper> looper = new ALooper;
    looper->start();
    sp<MediaCodec> decoder = MediaCodec::CreateByComponentName(
            looper, mComponentName, &err);
    if (decoder.get() == NULL || err != OK) {
        ALOGW("Failed to instantiate decoder [%s]", mComponentName.c_str());
        return (decoder.get() == NULL) ? NO_MEMORY : err;
    }

    err = decoder->configure(
         videoFormat, NULL /* surface */, NULL /* crypto */, 0 /* flags */);

    if (err != OK) {
        ALOGW("configure returned error %d (%s)", err, asString(err));
        decoder->release();
        return err;
    }

    err = decoder->start();
    if (err != OK) {
        ALOGW("start returned error %d (%s)", err, asString(err));
        decoder->release();
        return err;
    }

    err = mSource->start();
    if (err != OK) {
        ALOGW("source failed to start: %d (%s)", err, asString(err));
        decoder->release();
        return err;
    }
    mDecoder = decoder;
    return OK;
}

bool MtkImageDecoder::receiveheifouputbuffer() {
    ALOGV("receiveheifouputbuffer()");
    if (mIsEos && (mInputCount != mGridCols*mGridRows)) {
        ALOGE("source err, no need dequeueoutputbuffer");
        return false;
    }
    status_t err = OK;
    size_t index;
    bool done = false;
    mIsRGBReady = false;
    size_t retriesLeft = kRetryCount;
    while (!mIsStop & !mIsAllDone) {
        if (mIsEos && (mInputCount != mGridCols*mGridRows)) {
            ALOGE("source read err, exit outpubuffer looper");
            err = -1;
            break;
        }
        while (err == OK && !mIsRGBReady) {
            if (mIsEos && (mInputCount != mGridCols*mGridRows)) {
                ALOGE("source read err, exit dequeueoutpubuffer");
                break;
            }

            int64_t ptsUs = 0ll;
            uint32_t flags = 0;

            size_t offset, size;
            // wait for a decoded buffer
            err = mDecoder->dequeueOutputBuffer(
                    &index,
                    &offset,
                    &size,
                    &ptsUs,
                    &flags,
                    kBufferTimeOutUs);

            if (err == INFO_FORMAT_CHANGED) {
                ALOGV("Received format change");
                err = mDecoder->getOutputFormat(&mOutputFormat);
            } else if (err == INFO_OUTPUT_BUFFERS_CHANGED) {
                ALOGV("Output buffers changed");
                err = OK;
            } else {
                if (err == -EAGAIN && --retriesLeft > 0) {
                    err = OK;
                } else if (err == OK) {
                    if (mOutputFormat == NULL) {
                        ALOGE("get outputFormat fail");
                        mSource->stop();
                        mDecoder->release();
                        mDecoder.clear();
                        return false;
                    }
                    // If we're seeking with CLOSEST option and obtained a valid targetTimeUs
                    // from the extractor, decode to the specified frame. Otherwise we're done.
                    ALOGV("Received an output buffer, timeUs=%lld", (long long)ptsUs);
                    sp<MediaCodecBuffer> videoFrameBuffer;
                    err = mDecoder->getOutputBuffer(index, &videoFrameBuffer);
                    if (err != OK) {
                        ALOGE("failed to get output buffer %zu", index);
                        break;
                    }
                    err = onOutputReceived(videoFrameBuffer, mOutputFormat, ptsUs, &done);
                    mDecoder->releaseOutputBuffer(index);
                } else {
                    if (err == -EAGAIN) {
                        ALOGW("decode timeout 10s, stop decoding");
                    } else {
                        ALOGW("Received error %d (%s) instead of output", err, asString(err));
                    }
                    done = true;
                }
            }
            if (done == true) {
                mIsRGBReady = true;
                done = false;
            }
            mIsAllDone = mTilesDecoded >= (mGridCols * mGridRows);
            if (mIsEos && (mInputCount != mGridCols*mGridRows)) {
                mIsAllDone = true;
                ALOGE("stop dequeueoutputbuffer");
            }
        }
    }
    return false;

}
sp<AMessage> MtkImageDecoder::onGetFormatAndSeekOptions(
        int64_t frameTimeUs, size_t /*numFrames*/,
        int /*seekMode*/, MediaSource::ReadOptions * options) {
    sp<MetaData> overrideMeta;
    if (frameTimeUs < 0) {
        uint32_t type;
        const void *data;
        size_t size;

        // if we have a stand-alone thumbnail, set up the override meta,
        // and set seekTo time to -1.
        if (!findHeiftestInfo(trackMeta(), &mWidth, &mHeight, &type, &data, &size)) {
            ALOGE("Thumbnail not available");
            return NULL;
        }
        overrideMeta = new MetaData(*(trackMeta()));
        overrideMeta->remove(kKeyDisplayWidth);
        overrideMeta->remove(kKeyDisplayHeight);
        overrideMeta->setInt32(kKeyWidth, mWidth);
        overrideMeta->setInt32(kKeyHeight, mHeight);
        overrideMeta->setData(kKeyHVCC, type, data, size);
        options->setSeekTo(-1);
    } else {
        CHECK(trackMeta()->findInt32(kKeyWidth, &mWidth));
        CHECK(trackMeta()->findInt32(kKeyHeight, &mHeight));

        options->setSeekTo(frameTimeUs);
    }

    mGridRows = mGridCols = 1;
    if (overrideMeta == NULL) {
        // check if we're dealing with a tiled heif
        int32_t tileWidth, tileHeight, gridRows, gridCols;
        if (findHeifGridInfo(trackMeta(), &tileWidth, &tileHeight, &gridRows, &gridCols)) {
            if (mWidth <= tileWidth * gridCols && mHeight <= tileHeight * gridRows) {
                ALOGV("grid: %dx%d, tile size: %dx%d, picture size: %dx%d",
                        gridCols, gridRows, tileWidth, tileHeight, mWidth, mHeight);

                overrideMeta = new MetaData(*(trackMeta()));
                overrideMeta->setInt32(kKeyWidth, tileWidth);
                overrideMeta->setInt32(kKeyHeight, tileHeight);
                mTileWidth = tileWidth;
                mTileHeight = tileHeight;
                mGridCols = gridCols;
                mGridRows = gridRows;
            } else {
                ALOGW("ignore bad grid: %dx%d, tile size: %dx%d, picture size: %dx%d",
                        gridCols, gridRows, tileWidth, tileHeight, mWidth, mHeight);
            }
        }
        if (overrideMeta == NULL) {
            overrideMeta = trackMeta();
        }
    }
    mTargetTiles = mGridCols * mGridRows;

    sp<AMessage> videoFormat;
    if (convertMetaDataToMessage(overrideMeta, &videoFormat) != OK) {
        ALOGE("b/23680780");
        ALOGW("Failed to convert meta data to message");
        return NULL;
    }

    // TODO: Use Flexible color instead
//    videoFormat->setInt32("color-format", OMX_COLOR_FormatYUV420Planar);
    videoFormat->setInt32("color-format", mDstFormat);

    if ((mGridRows == 1) && (mGridCols == 1)) {
        videoFormat->setInt32("android._num-input-buffers", 1);
        videoFormat->setInt32("android._num-output-buffers", 1);
    }

// HEIF task ++
// set Grid num to encoder if mimetype is MEDIA_MIMETYPE_IMAGE_ANDROID_HEIC

    const char *mime;
    CHECK(trackMeta()->findCString(kKeyMIMEType, &mime));

    while (mGridNum >1) {
        if (!(mGridCols % mGridNum))
            break;
        mGridNum--;
    }
    ALOGD("new mGridcols %zu ", mGridNum);
    videoFormat->setInt32("HeifDecoderWidth", mGridNum * mTileWidth);
    videoFormat->setInt32("HeifDecoderHeight", mTileHeight);
    videoFormat->setInt32("GridNum", mGridNum);
// to do: get GridNum from retriever of heifdecoderImpl
// HEIF task --
    return videoFormat;

}
status_t MtkImageDecoder::onExtractRect(FrameRect * rect) {
    // TODO:
    // This callback is for verifying whether we can decode the rect,
    // and if so, set up the internal variables for decoding.
    // Currently, rect decoding is restricted to sequentially decoding one
    // row of tiles at a time. We can't decode arbitrary rects, as the image
    // track doesn't yet support seeking by tiles. So all we do here is to
    // verify the rect against what we expect.
    // When seeking by tile is supported, this code should be updated to
    // set the seek parameters.
    if (rect == NULL) {
        if (mTilesDecoded > 0) {
            return ERROR_UNSUPPORTED;
        }
        mTargetTiles = mGridRows * mGridCols;
        return OK;
    }

    if (mTileWidth <= 0 || mTileHeight <=0) {
        return ERROR_UNSUPPORTED;
    }

    int32_t row = mTilesDecoded / mGridCols;
    int32_t expectedTop = row * mTileHeight;
    int32_t expectedBot = (row + 1) * mTileHeight;
    if (expectedBot > mHeight) {
        expectedBot = mHeight;
    }
    if (rect->left != 0 || rect->top != expectedTop
            || rect->right != mWidth || rect->bottom != expectedBot) {
        ALOGE("currently only support sequential decoding of slices");
        return ERROR_UNSUPPORTED;
    }

    // advance one row
    mTargetTiles = mTilesDecoded + mGridCols;
    return OK;
}
status_t MtkImageDecoder::extractInternal_ext(){
    status_t err = OK;
        bool done = false;
        if (mThread == NULL) {  // new another thread to decode outputbuffer
            mThread = new DecodeThread(this);
            if (mThread->run("FrameDecoderthread", ANDROID_PRIORITY_FOREGROUND) == OK) {
                ALOGV("new decodeThread done");
            } else {
                ALOGE("new decodethread fail");
                return -1;
            }
        }

        mIsRGBReady = false;
        size_t GridNumCount = 0;
        if (mIsEos && (mInputCount != mGridCols*mGridRows)) {
            ALOGE("source EOS return, already send %d input, expect %d", mInputCount, mGridCols*mGridRows);
            return -1;
        }

        do {
            size_t index;
            int64_t ptsUs = 0ll;
            uint32_t flags = 0;

            // Queue as many inputs as we possibly can, then block on dequeuing
            // outputs. After getting each output, come back and queue the inputs
            // again to keep the decoder busy.
            while (err == OK && !mIsEos) {
                err = mDecoder->dequeueInputBuffer(&index, 0);
                if (err != OK) {
                    ALOGV("Timed out waiting for input");
                    err = OK;
                    break;
                }

                sp<MediaCodecBuffer> codecBuffer;
                err = mDecoder->getInputBuffer(index, &codecBuffer);
                if (err != OK) {
                    ALOGE("failed to get input buffer %zu", index);
                    break;
                }
                ALOGV("get one inputbuffer");
                const bool couldReadMultiple = (mSource->supportReadMultiple());

                if (couldReadMultiple) {
                    mReadOptions.setNonBlocking();
                } else {
                    ALOGE("no support readmultiple");
                }

                size_t offset = (mGridNum+2)*4;
                while (GridNumCount< mGridNum) {
                    Vector<MediaBufferBase *> mediaBuffers;
                    status_t err = NO_ERROR;

    //              mLock.unlock();
                    if (couldReadMultiple) {
                        err = mSource->readMultiple(
                                &mediaBuffers, mGridNum - GridNumCount, &mReadOptions);
                    } else {
                        MediaBufferBase *mbuf = NULL;
                        err = mSource->read(&mbuf, &mReadOptions);
                        if (err == OK && mbuf != NULL) {
                            mediaBuffers.push_back(mbuf);
                        }
                    }
    //              mLock.lock();

                    mReadOptions.clearNonPersistent();

                    size_t id = 0;
                    size_t count = mediaBuffers.size();
                    mInputCount += count;
                    ALOGD("HEIF debug, read %zu Grid frames, total %d", count, mInputCount);
                    if ((err == WOULD_BLOCK) & (count== 0)) {
                        ALOGD("read WOULD_BLOCK");
                        continue;
                    }

                    if ((err != WOULD_BLOCK) & (count== 0)) {
                        if (mInputCount != mGridCols*mGridRows) {
                            ALOGE("read err %d,mInputCount %d,expect %d", err, mInputCount, mGridCols*mGridRows);
                            mIsEos = true;
                        } else {
                            ALOGD("read EOS,err %d", err);
                            mIsEos = true;
                        }
                        break;
                    } else {
                        for (; id < count; ++id, ++GridNumCount) {
                            MediaBufferBase *mbuf = mediaBuffers[id];
                            ((uint32_t*)(codecBuffer->data()))[GridNumCount+2] = mbuf->range_length();
                            memcpy(codecBuffer->data() + offset,
                                    (const uint8_t*)mbuf->data() + mbuf->range_offset(),
                                    mbuf->range_length());     // add frame data

                            offset += mbuf->range_length();
                            mbuf->release();
                        }
                        codecBuffer->setRange(0, offset);
                    }
                }

                if (GridNumCount == mGridNum) {
                    ALOGV("get %zu grid frame, queueinputbuffer", GridNumCount);
                    ((char *)(codecBuffer->data()))[0] = 0x48;
                    ((char *)(codecBuffer->data()))[1] = 0x45;
                    ((char *)(codecBuffer->data()))[2] = 0x49;
                    ((char *)(codecBuffer->data()))[3] = 0x46;
                    ((uint32_t*)(codecBuffer->data()))[1] = GridNumCount;

                    err = mDecoder->queueInputBuffer(
                            index,
                            codecBuffer->offset(),
                            codecBuffer->size(),
                            ptsUs,
                            flags);
                } else {
                    ALOGV("reach EOS, send eos to decoder");
                    err = mDecoder->queueInputBuffer(
                            index,
                            0,
                            0,
                            0,
                            MediaCodec::BUFFER_FLAG_EOS);
                }
                if (GridNumCount >= mGridNum) {
                    GridNumCount = 0;
                }
                break;
              }
              if (mIsEos && (mInputCount != mGridCols*mGridRows)) {
                ALOGE("source read err, exit read looper");
                break;
              }
        // query ouputbuffer done or not
              if (mIsRGBReady) {
                done = true;
              }
        } while (err == OK && !done);

        if (err != OK) {
            ALOGE("failed to get video frame (err %d)", err);
        }
        if (mTilesDecoded >= mTargetTiles) {
            ALOGD("heif decode done mTilesDecoded %d mTargetTiles %d mGridCols %d mGridRows %d",
                mTilesDecoded, mTargetTiles, mGridCols, mGridRows);
            return err;
        } else {
            ALOGW("not decode enough gridm :TilesDecoded %d mTargetTiles %d mGridCols %d mGridRows %d",
                mTilesDecoded, mTargetTiles, mGridCols, mGridRows);
            return -1;
        }
}


status_t MtkImageDecoder::onOutputReceived(
        const sp<MediaCodecBuffer> &videoFrameBuffer,
        const sp<AMessage> &outputFormat, int64_t /*timeUs*/, bool *done) {
    if (outputFormat == NULL) {
        return ERROR_MALFORMED;
    }

    int32_t width, height;
    CHECK(outputFormat->findInt32("width", &width));  //  note: width = gridnum * mTileWidth
    CHECK(outputFormat->findInt32("height", &height));

    if (mFrame == NULL) {
        sp<IMemory> frameMem = allocHeicVideoFrame(
                trackMeta(), mWidth, mHeight, mTileWidth, mTileHeight, mDstBpp);
        if (frameMem == NULL) {
            ALOGW("no memory, stop decoding");
            *done = true;
            return -1;
        }
        mFrame = static_cast<VideoFrame*>(frameMem->pointer());
        addFrame(frameMem);
    }
/*
 // count GridNum By outputbuffer size
    int32_t GridNum = videoFrameBuffer->size() *2 /(width * height * 3);
    ALOGD("heif debug outputbuffer has %d Grid, expected num is %zu",GridNum,mGridNum);
*/
    int32_t srcFormat;
    CHECK(outputFormat->findInt32("color-format", &srcFormat));

    int32_t dstLeft, dstTop, dstRight, dstBottom;
    dstLeft = mTilesDecoded % mGridCols * mTileWidth;
    dstTop = mTilesDecoded / mGridCols * mTileHeight;
    dstRight = dstLeft + width - 1;
    dstBottom = dstTop + height - 1;

    int32_t crop_left, crop_top, crop_right, crop_bottom;
    if (!outputFormat->findRect("crop", &crop_left, &crop_top, &crop_right, &crop_bottom)) {
        crop_left = crop_top = 0;
        crop_right = width - 1;
        crop_bottom = height - 1;
    }

    // apply crop on bottom-right
    // TODO: need to move this into the color converter itself.
    if (dstRight >= mWidth) {
        crop_right = mWidth - dstLeft - 1;
        dstRight = dstLeft + crop_right;
    }
    if (dstBottom >= mHeight) {
        crop_bottom = mHeight - dstTop - 1;
        dstBottom = dstTop + crop_bottom;
    }


    status_t err = OK;

    if((OMX_COLOR_FORMATTYPE)srcFormat == (OMX_COLOR_FORMATTYPE)mDstFormat){
        ALOGV("srcFormat == mDstFormat");
        if(width == mWidth){
            memcpy(mFrame->getFlattenedData() + dstTop * mWidth * mDstBpp,
                (const uint8_t *)(videoFrameBuffer->data()),
                (dstBottom - dstTop + 1) * mWidth * mDstBpp);
            mTilesDecoded += mGridNum;
        } else{
            for(int i = 0; i < (crop_bottom - crop_top +1) ; ++i){
                memcpy(mFrame->getFlattenedData() + ((dstTop + i) * mWidth + dstLeft) * mDstBpp,
                    (const uint8_t *)(videoFrameBuffer->data()) + width * i * mDstBpp,
                    (crop_right - crop_left +1) * mDstBpp);
            }
            mTilesDecoded += mGridNum;
        }
    } else {
      ALOGE("not support YUV->RGB");
      *done = true;
      return ERROR_UNSUPPORTED;
    }
    *done = (mTilesDecoded >= mTargetTiles);
    ALOGD("heif debug, mTilesDecoded %d targetTiles %d", mTilesDecoded, mTargetTiles);
    if (err != OK) {
        ALOGE("covert YUV-> RGB fail, stop heif decoding");
        *done = true;
        return ERROR_UNSUPPORTED;
    }

    return OK;
}


MtkImageDecoder::~MtkImageDecoder() {
    mIsStop = true;
    if (mDecoder != NULL) {
        ALOGD("stop decoder +");
        mDecoder->release();
        ALOGD("stop decoder -");
        mSource->stop();
    }

}



}  // namespace android
