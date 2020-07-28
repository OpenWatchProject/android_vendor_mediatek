/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

/*
 * Copyright (C) 2009 The Android Open Source Project
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
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   MtkAVIExtractor.h
 *
 * Project:
 * --------
 *   MT6573
 *
 * Description:
 * ------------
 *   AVI Extractor interface
 *
 * Author:
 * -------
 *   Demon Deng (mtk80976)
 *
 ****************************************************************************/
#ifndef MTK_AVI_EXTRACTOR_H_
#define MTK_AVI_EXTRACTOR_H_

#include <media/MediaExtractorPluginApi.h>
#include <media/MediaExtractorPluginHelper.h>
#include <utils/Vector.h>
//#include <media/stagefright/SampleTable.h>
#include <media/stagefright/MetaData.h>

typedef off64_t MtkAVIOffT;

namespace android {

struct AMessage;
class DataSource;
class String8;
class MtkAVISource;
struct MtkAVISample;
struct MtkAVIIndexChunk;

enum H264_STYLE {
        SIZE_NAL_TYPE,
        START_CODE_TYPE,
        OTHER_TYPE,
};


class MtkAVIExtractor : public MediaExtractorPluginHelper {
public:
    explicit MtkAVIExtractor(DataSourceHelper *source);
    virtual size_t countTracks();
    virtual MediaTrackHelper *getTrack(size_t index);
    virtual media_status_t getTrackMetaData(AMediaFormat *meta, size_t index, uint32_t flags);
    virtual media_status_t getMetaData(AMediaFormat *meta);
    virtual uint32_t flags() const;
    virtual status_t stopParsing();
    virtual status_t finishParsing();
    virtual const char * name() { return "MtkAVIExtractor"; }

protected:
    virtual ~MtkAVIExtractor();

private:
    struct Track {
        sp<MetaData> meta;
        int scale;
        int rate;
        int sampleSize;
        uint32_t sampleCount;
        int64_t durationUs;
        bool mIsVideo;
        uint32_t mStartTimeOffset;
        int priority;
    };

    DataSourceHelper *mDataSource;
    bool mHasMetadata;
    bool mHasVideo;
    bool mHasAudio;
    bool mHasIndex;
    bool mStopped;
    MtkAVIOffT mFileSize;  // ssize_t -> MtkAVIOffT for build error
    status_t mInitCheck;
    Vector<MtkAVISource *> mTracks;
    AMediaFormat *mFileMetaData;
    uint32_t mIndexOffset;
    uint32_t mMOVIOffset;
    uint32_t mMOVISize;
    uint32_t mEmptyChunks;
    int32_t mNumTracks;

    status_t readMetaData();
    status_t readHeader(MtkAVIOffT *pos, char *h, ssize_t size);
    status_t parseFirstRIFF();

    status_t addSample(size_t id, struct MtkAVISample* s);
    status_t parseHDRL(MtkAVIOffT off, MtkAVIOffT end, MtkAVIOffT &endOffsCRC);
    status_t parseMOVI(MtkAVIOffT off, MtkAVIOffT end);
    status_t parseDataChunk(MtkAVIOffT pos, int ID, uint32_t size, int sync);
    status_t parseMOVIMore(bool full = true);
    status_t parseIDX1(MtkAVIOffT off, MtkAVIOffT end);
    status_t parseINFO(MtkAVIOffT off, MtkAVIOffT end);

    status_t parseAVIH(MtkAVIOffT off, MtkAVIOffT end);
    status_t parseSTRL(MtkAVIOffT off, MtkAVIOffT end, int index);

    status_t parseSTRH(MtkAVIOffT off, MtkAVIOffT end, MtkAVISource* source);
    status_t parseSTRF(MtkAVIOffT off, MtkAVIOffT end, MtkAVISource* source);
    status_t parseSTRD(MtkAVIOffT off, MtkAVIOffT end, MtkAVISource* source);
    status_t parseSTRN(MtkAVIOffT off, MtkAVIOffT end, MtkAVISource* source);
    status_t parseVPRP(MtkAVIOffT off, MtkAVIOffT end, MtkAVISource* source);
    status_t parseINDX(MtkAVIOffT pos, MtkAVIOffT end, MtkAVISource* source);
    status_t parseChunkIndex(MtkAVIOffT pos, MtkAVIOffT end, MtkAVISource* source,
            struct MtkAVIIndexChunk* pHeader);

    status_t checkCapability();

    status_t parseMetaData(MtkAVIOffT offset, size_t size);

    static status_t verifyTrack(Track *track);

    status_t parseTrackHeader(MtkAVIOffT data_offset, MtkAVIOffT data_size);

    MtkAVIExtractor(const MtkAVIExtractor &);
    MtkAVIExtractor &operator=(const MtkAVIExtractor &);
};

int switchAACSampleRateToIndex(int sample_rate);
bool hasDivxLicense();
}  // namespace android

#endif  // MTK_AVI_EXTRACTOR_H_
