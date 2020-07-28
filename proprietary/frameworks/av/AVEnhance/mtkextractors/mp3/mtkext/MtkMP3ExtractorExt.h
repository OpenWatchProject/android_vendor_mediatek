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

#ifndef MTK_MP3_EXTRACTOR_EXT_H_
#define MTK_MP3_EXTRACTOR_EXT_H_

#include <media/MediaExtractorPluginHelper.h>
//add for toc
#include "TableOfContentThread.h"


namespace android {

struct MP3Seeker;
struct Mp3Meta;

class MtkMP3SourceExt{
    public:
        MtkMP3SourceExt();
        static int getMultiFrameSize(DataSourceHelper *source, off64_t pos,
                uint32_t fixedHeader, size_t *frame_sizes, int *num_samples,
                int *sample_rate, int request_frames);

    protected:
        virtual ~MtkMP3SourceExt();

    private:
        MtkMP3SourceExt(const MtkMP3SourceExt &);
        MtkMP3SourceExt &operator=(const MtkMP3SourceExt &);
};

class MtkMP3Toc : public TableOfContentThread{
    public:
        explicit MtkMP3Toc(DataSourceHelper *source, uint32_t fixedHeader);
        int64_t getTocDuration();

    protected:
        virtual ~MtkMP3Toc();
        virtual status_t getNextFramePos(off64_t *pCurpos, off64_t *pNextPos, int64_t *frameTsUs);
        virtual status_t  sendDurationUpdateEvent(int64_t duration);

    private:
        DataSourceHelper *mDataSource;
        uint32_t mFixedHeader;
        int64_t mDurationUsByTOC;
        Mutex mDurationLock;
        MtkMP3Toc(const MtkMP3Toc &);
        MtkMP3Toc &operator=(const MtkMP3Toc &);
};

status_t ComputeDurationFromNRandomFrames(DataSourceHelper *source, off64_t FirstFramePos,
        uint32_t FixedHeader,int32_t *Averagebr, int32_t *isSingle);

void getGeneralDuration(int bitrate, DataSourceHelper *source, MP3Seeker *seeker,
        AMediaFormat *meta, off64_t firstFramePos, uint32_t fixedHeader);


}
#endif
