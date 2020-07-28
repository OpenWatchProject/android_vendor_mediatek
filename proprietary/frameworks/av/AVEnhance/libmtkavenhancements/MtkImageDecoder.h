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

#ifndef MTK_IMAGE_DECODER_H_
#define MTK_IMAGE_DECODER_H_

#include <FrameDecoder.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/MediaCodec.h>
#include <media/MediaSource.h>
#include <media/IMediaSource.h>
namespace android {

struct AMessage;
class MediaCodecBuffer;
class IMediaSource;
class VideoFrame;
struct MediaCodec;

struct MtkImageDecoder : public ImageDecoder {
    MtkImageDecoder(
            const AString &componentName,
            const sp<MetaData> &trackMeta,
            const sp<IMediaSource> &source);
protected:
    virtual ~MtkImageDecoder();
    virtual status_t extractInternal_ext() override;
    virtual sp<AMessage> onGetFormatAndSeekOptions(
            int64_t frameTimeUs,
            size_t numFrames,
            int seekMode,
            MediaSource::ReadOptions *options)override;

    virtual status_t onExtractRect(FrameRect *rect)override;

    virtual status_t onInputReceived(
            const sp<MediaCodecBuffer> &codecBuffer __unused,
            MetaDataBase &sampleMeta __unused,
            bool firstSample __unused,
            uint32_t *flags __unused)override { return OK; }

    virtual status_t onOutputReceived(
            const sp<MediaCodecBuffer> &videoFrameBuffer,
            const sp<AMessage> &outputFormat,
            int64_t timeUs,
            bool *done)override;
    virtual status_t init_ext(
            int64_t frameTimeUs, size_t numFrames, int option,
            int colorFormat ) override;    
private:
    AString mComponentName;
    sp<MetaData> mTrackMeta;
    sp<IMediaSource> mSource;
    OMX_COLOR_FORMATTYPE mDstFormat;
    int32_t mDstBpp;
    std::vector<sp<IMemory> > mFrames;
    MediaSource::ReadOptions mReadOptions;
    sp<MediaCodec> mDecoder;
    sp<AMessage> mOutputFormat;
    bool mHaveMoreInputs;
    bool mFirstSample;
    bool mIsRGBReady;
    bool mIsAllDone;
    bool mIsStop;
    bool mIsEos;
    VideoFrame *mFrame;
    int32_t mWidth;
    int32_t mHeight;
    int32_t mGridRows;
    int32_t mGridCols;
    int32_t mInputCount;
    size_t mGridNum;
    int32_t mTileWidth;
    int32_t mTileHeight;
    int32_t mTilesDecoded;
    int32_t mTargetTiles;
    struct DecodeThread;
    sp<DecodeThread> mThread;

    bool receiveheifouputbuffer();

    DISALLOW_EVIL_CONSTRUCTORS(MtkImageDecoder);
};

}  // namespace android

#endif  // MTK_IMAGE_DECODER_H_
