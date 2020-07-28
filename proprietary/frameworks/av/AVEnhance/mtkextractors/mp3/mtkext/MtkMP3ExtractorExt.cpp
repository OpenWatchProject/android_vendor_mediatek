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

//#define LOG_NDEBUG 0
#define LOG_TAG "MtkMP3SourceExt"

#include "MtkMP3ExtractorExt.h"

#include "ID3.h"
#include "VBRISeeker.h"
#include "XINGSeeker.h"


#include <media/DataSourceBase.h>
#include <media/MediaTrack.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/avc_utils.h>
#include <media/stagefright/foundation/ByteUtils.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MetaData.h>
#include <utils/String8.h>

#define MIN_RANDOM_FRAMES_TO_SCAN 4
#define MIN_RANDOM_LOCATION_TO_SCAN 30

namespace android {

static const uint32_t kMask = 0xfffe0c00;

static bool Resync(
        DataSourceHelper *source, uint32_t match_header,
        off64_t *inout_pos, off64_t *post_id3_pos, uint32_t *out_header) {
    if (post_id3_pos != NULL) {
        *post_id3_pos = 0;
    }

    if (*inout_pos == 0) {
        // Skip an optional ID3 header if syncing at the very beginning
        // of the datasource.

        for (;;) {
            uint8_t id3header[10];
            if (source->readAt(*inout_pos, id3header, sizeof(id3header))
                    < (ssize_t)sizeof(id3header)) {
                // If we can't even read these 10 bytes, we might as well bail
                // out, even if there _were_ 10 bytes of valid mp3 audio data...
                return false;
            }

            if (memcmp("ID3", id3header, 3)) {
                break;
            }

            // Skip the ID3v2 header.

            size_t len =
                ((id3header[6] & 0x7f) << 21)
                | ((id3header[7] & 0x7f) << 14)
                | ((id3header[8] & 0x7f) << 7)
                | (id3header[9] & 0x7f);

            len += 10;

            *inout_pos += len;

            ALOGV("skipped ID3 tag, new starting offset is %lld (0x%016llx)",
                    (long long)*inout_pos, (long long)*inout_pos);
        }

        if (post_id3_pos != NULL) {
            *post_id3_pos = *inout_pos;
        }
    }

    off64_t pos = *inout_pos;
    bool valid = false;

    const size_t kMaxReadBytes = 1024;
    const size_t kMaxBytesChecked = 128 * 1024;
    uint8_t buf[kMaxReadBytes];
    ssize_t bytesToRead = kMaxReadBytes;
    ssize_t totalBytesRead = 0;
    ssize_t remainingBytes = 0;
    bool reachEOS = false;
    uint8_t *tmp = buf;

    do {
        if (pos >= (off64_t)(*inout_pos + kMaxBytesChecked)) {
            // Don't scan forever.
            ALOGV("giving up at offset %lld", (long long)pos);
            break;
        }

        if (remainingBytes < 4) {
            if (reachEOS) {
                break;
            } else {
                memcpy(buf, tmp, remainingBytes);
                bytesToRead = kMaxReadBytes - remainingBytes;

                /*
                 * The next read position should start from the end of
                 * the last buffer, and thus should include the remaining
                 * bytes in the buffer.
                 */
                totalBytesRead = source->readAt(pos + remainingBytes,
                                                buf + remainingBytes,
                                                bytesToRead);
                if (totalBytesRead <= 0) {
                    break;
                }
                reachEOS = (totalBytesRead != bytesToRead);
                totalBytesRead += remainingBytes;
                remainingBytes = totalBytesRead;
                tmp = buf;
                continue;
            }
        }

        uint32_t header = U32_AT(tmp);

        if (match_header != 0 && (header & kMask) != (match_header & kMask)) {
            ++pos;
            ++tmp;
            --remainingBytes;
            continue;
        }

        size_t frame_size;
        int sample_rate, num_channels, bitrate;
        if (!GetMPEGAudioFrameSize(
                    header, &frame_size,
                    &sample_rate, &num_channels, &bitrate)) {
            ++pos;
            ++tmp;
            --remainingBytes;
            continue;
        }

        ALOGV("found possible 1st frame at %lld (header = 0x%08x)", (long long)pos, header);

        // We found what looks like a valid frame,
        // now find its successors.

        off64_t test_pos = pos + frame_size;

        valid = true;
        for (int j = 0; j < 3; ++j) {
            uint8_t tmp[4];
            if (source->readAt(test_pos, tmp, 4) < 4) {
                valid = false;
                break;
            }

            uint32_t test_header = U32_AT(tmp);

            ALOGV("subsequent header is %08x", test_header);

            if ((test_header & kMask) != (header & kMask)) {
                valid = false;
                break;
            }

            size_t test_frame_size;
            if (!GetMPEGAudioFrameSize(
                        test_header, &test_frame_size)) {
                valid = false;
                break;
            }

            ALOGV("found subsequent frame #%d at %lld", j + 2, (long long)test_pos);

            test_pos += test_frame_size;
        }

        if (valid) {
            *inout_pos = pos;

            if (out_header != NULL) {
                *out_header = header;
            }
        } else {
            ALOGV("no dice, no valid sequence of frames found.");
        }

        ++pos;
        ++tmp;
        --remainingBytes;
    } while (!valid);

    return valid;
}


////////////////////////////////////////////////////////////////////////////////

MtkMP3SourceExt::MtkMP3SourceExt()
{

}

MtkMP3SourceExt::~MtkMP3SourceExt()
{

}

int MtkMP3SourceExt::getMultiFrameSize(DataSourceHelper *source, off64_t pos,
                uint32_t fixedHeader, size_t *frame_sizes, int *num_samples,
                int *sample_rate, int request_frames)
{
    uint8_t headbuf[4]={0};
    size_t frameSize = 0;
    int numSamples = 0;
    ssize_t frameCount = 0;
    int sampleRate = 0;

    *frame_sizes = 0;
    *num_samples = 0;
    *sample_rate = 0;

    for(int i = 0; i< request_frames; i++) {
        ssize_t n = source->readAt(pos,headbuf,4);

        if (n < 4) {
            ALOGV("Bad header");
            return frameCount;
        }
        uint32_t header = U32_AT((const uint8_t *)headbuf);

        if ((header & kMask) == (fixedHeader & kMask)
                && GetMPEGAudioFrameSize(
                    header, &frameSize, &sampleRate, NULL,
                    NULL, &numSamples) && sampleRate != 0) {
            *sample_rate = sampleRate;
            *frame_sizes += frameSize;//20 frame length
            *num_samples += numSamples;
            frameCount++;
            pos += frameSize; //new position
        } else {
            break;
        }
    }

    return frameCount;
}

status_t ComputeDurationFromNRandomFrames(
      DataSourceHelper *source,
      off64_t FirstFramePos,
      uint32_t FixedHeader,
      int32_t *Averagebr,
      int32_t *isSingle)
{
    const size_t V1_TAG_SIZE = 128;
    off_t audioDataSize = 0;
    off64_t fileSize = 0;
   // off_t fileoffset = 0;
    off64_t audioOffset = 0;
    int32_t totBR = 0;
    int32_t avgBitRate = 0;
    int32_t BitRate = 0;
    int32_t randomByteOffset = 0;
    int32_t framecount = 0;
    size_t frame_size = 0;
    if ((source->getSize(&fileSize) == OK) && (fileSize > FirstFramePos)) {
        audioDataSize = fileSize - FirstFramePos;
        uint8_t *mData;
        mData = NULL;
        if ( fileSize > (off_t)V1_TAG_SIZE) {
            mData = (uint8_t *)malloc(V1_TAG_SIZE);
            if (mData != NULL && source->readAt(fileSize - V1_TAG_SIZE, mData, V1_TAG_SIZE)== (ssize_t)V1_TAG_SIZE)
            {
                if (!memcmp("TAG", mData, 3)) {
                    audioDataSize -= V1_TAG_SIZE;
                    ALOGV("TAG V1_TAG_SIZE 128!");
                }
            }
            free(mData);
            mData = NULL;
        }
    }else{
        ALOGD("ComputeDurationFromNRandomFrames::Read File Size Error!");
        return UNKNOWN_ERROR;
    }
    //ALOGD("audioDataSize=%d,FirstFramePos=%d,fileSize=%d",audioDataSize,FirstFramePos,fileSize);
    randomByteOffset = FirstFramePos;
    uint32_t skipMultiple = audioDataSize / (MIN_RANDOM_LOCATION_TO_SCAN + 1);
    //ALOGD("skipMultiple=%d",skipMultiple);
    int32_t numSearchLoc = 0,currFilePosn = 0;
    //off64_t post_id3_pos = 0;
    audioOffset=FirstFramePos;
    while (numSearchLoc < MIN_RANDOM_LOCATION_TO_SCAN)
    {
        // find random location to which we should seek in order to find
        currFilePosn = audioOffset;
        randomByteOffset = currFilePosn + skipMultiple;
        if (randomByteOffset > fileSize)
        {
            //ALOGD("Duration   finish 1!( pos>file size)");
            break;
        }
        // initialize frame count
        framecount = 0;
        audioOffset = randomByteOffset;
        //ALOGD("audioOffset=%d",audioOffset);
        if (false == Resync(source, FixedHeader, &audioOffset, NULL,NULL) )
        {
            //ALOGD("Resync no success !");
            break;
        }
        // lets check rest of the frames
        while (framecount < MIN_RANDOM_FRAMES_TO_SCAN)
        {
            uint8_t mp3header[4];
            ssize_t n = source->readAt(audioOffset, mp3header, sizeof(mp3header));
            if (n < 4) {
                break;
            }
            uint32_t header = U32_AT((const uint8_t *)mp3header);
            if ((header & kMask) != (FixedHeader & kMask)) {
                //ALOGD("header error!");
                 break;
            }
            if(!GetMPEGAudioFrameSize(
                    header, &frame_size,
                    NULL, NULL, &BitRate)){
                //ALOGD("getmp3framesize error");
                break;
            }
            if(((header>>6) & 3) == 3 )
            {
                (*isSingle)--;  //if channel count is single channel
            }
                        else
                        {
                            (*isSingle)++;
            }
            // ALOGD("framecount=%d,frame_size=%d,BitRate=%d",framecount,frame_size,BitRate);
            audioOffset += frame_size;
            framecount++;
            // initialize avgBitRate first time only
            if (1 == framecount)
            {
                avgBitRate =BitRate;
                //ALOGD("avgBitRate=%d",avgBitRate);
            }
            if (BitRate != avgBitRate)
            {
                avgBitRate += (BitRate - avgBitRate) / framecount;
            }
        }
        //ALOGD("numSearchLoc=%d",numSearchLoc);
        totBR += avgBitRate;
        numSearchLoc++;
    }
     // calculate average bitrate
    *Averagebr = numSearchLoc > 0 ? totBR / numSearchLoc : 0;
    //ALOGD("RandomScan Averagebr=%d",*Averagebr);
    if ( *Averagebr <= 0)
    {
        return BAD_VALUE;
    }
    return OK;
}

void getGeneralDuration(
    int bitrate,
    DataSourceHelper *source,
    MP3Seeker *seeker,
    AMediaFormat *meta,
    off64_t firstFramePos,
    uint32_t fixedHeader)
{
    if(source->flags() & DataSourceBase::kIsCachingDataSource){//streaming using
        int64_t durationUsStream;
        if (seeker == NULL || !seeker->getDuration(&durationUsStream)){
            off64_t fileSize_Stream;
            if (source->getSize(&fileSize_Stream) == OK) {
                if (fileSize_Stream > firstFramePos)
                {
                    durationUsStream = 8000LL * (fileSize_Stream - firstFramePos) / bitrate;
                }
                else
                {
                    durationUsStream = 0;
                }
            } else {
                durationUsStream = -1;
                ALOGD("durationUsStream = -1");
            }
        }
        if (durationUsStream >= 0) {
            AMediaFormat_setInt64(meta, AMEDIAFORMAT_KEY_DURATION, durationUsStream);
        }
        ALOGD("streaming duration = %lld",(long long)durationUsStream);
    }else{
        int64_t durationUs = 0; ;
        int32_t averagebr =0;
        off64_t fileSize_Enh = 0;
        bool specialheader = false;
        int32_t isSingle = 0;//if channel is single channel
        if(seeker != NULL && seeker->getDuration(&durationUs)){
           specialheader = true;
           //ALOGD("Duration %lld from XING&VBRI Header ",durationUs);
        }
        if (!specialheader && source->getSize(&fileSize_Enh) == OK){
            if(ComputeDurationFromNRandomFrames(source,firstFramePos,fixedHeader,&averagebr,&isSingle)==OK)
            {
                if (fileSize_Enh > firstFramePos)
                {
                    durationUs = (fileSize_Enh - firstFramePos) * 8000LL / averagebr; //[byte/(kbit*8)]*1000*1000 us
                }
                else
                {
                    durationUs = 0;
                }
                //ALOGD("RandomScan:AverageBitrate=%d, Duration1=%lld",averagebr,durationUs);
                //ALOGD("DirectCal:Bitrate =%d,Duration=%lld",bitrate,8000LL * (fileSize_Enh - mFirstFramePos) / bitrate);
            }else{
                if (fileSize_Enh > firstFramePos)
                {
                    durationUs = 8000LL * (fileSize_Enh - firstFramePos) / bitrate;
                }
                else
                {
                    durationUs = 0;
                }
                //ALOGD("No use EnhancedDuration ComputeDuration ! duration=%lld",durationUs);
            }
        }
        if (durationUs >= 0) {
            AMediaFormat_setInt64(meta, AMEDIAFORMAT_KEY_DURATION, durationUs);
        }
        if(isSingle < 0)
        {
            //add error handle for those bad split joint mp3 file
            AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_CHANNEL_COUNT, 1);
        }
    }
}


////////////////////////////////////////////////////////////////////////////////
MtkMP3Toc::MtkMP3Toc(DataSourceHelper *source, uint32_t fixedHeader)
    :mDataSource(source),
    mFixedHeader(fixedHeader),
    mDurationUsByTOC(-1) {

    }

MtkMP3Toc::~MtkMP3Toc()
{

}

status_t  MtkMP3Toc::sendDurationUpdateEvent(int64_t duration) {
    Mutex::Autolock autoLock(mDurationLock);
    mDurationUsByTOC = duration;
    return OK;
}

int64_t  MtkMP3Toc::getTocDuration() {
    Mutex::Autolock autoLock(mDurationLock);
    return mDurationUsByTOC;
}

status_t MtkMP3Toc::getNextFramePos(off64_t *curPos, off64_t *pNextPos,int64_t * frameTsUs)
{

    uint8_t mp3header[4];
    size_t frame_size;
    int samplerate=0;
    int num_sample =0;
    for(;;) {
        ssize_t n = mDataSource->readAt(*curPos, mp3header, 4);
        if (n < 4) {
            ALOGD("For Seek Talbe :ERROR_END_OF_STREAM");
            return ERROR_END_OF_STREAM;
        }
       // ALOGD("mp3header[0]=%0x,mp3header[1]=%0x,mp3header[2]=%0x,mp3header[3]=%0x",mp3header[0],mp3header[1],mp3header[2],mp3header[3]);
        uint32_t header = U32_AT((const uint8_t *)mp3header);
        if ((header & kMask) == (mFixedHeader & kMask)
            && GetMPEGAudioFrameSize(header, &frame_size,
                                &samplerate, NULL,NULL,&num_sample)) {
            break;
        }
        off64_t pos = *curPos;
        if (!Resync(mDataSource, mFixedHeader, &pos, NULL,NULL)) {
             //ALOGD("getNextFramePos---Unable to resync. Signalling end of stream.");
             return ERROR_END_OF_STREAM;
        }
        *curPos = pos;
     }
     *pNextPos=*curPos+frame_size;
     *frameTsUs = 1000000ll * num_sample/samplerate;
   return OK;
}


}  // namespace android
