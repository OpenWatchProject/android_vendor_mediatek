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
#define LOG_TAG "MtkADPCMExtractor"
#include <utils/Log.h>

#include "MtkADPCMExtractor.h"
#include "stagefright/MediaDefs_MTK.h"
#include "MetaDataBase_MTK.h"

#include <android/binder_ibinder.h> // for AIBinder_getCallingUid
#include <audio_utils/primitives.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MetaData.h>
#include <private/android_filesystem_config.h> // for AID_MEDIA
#include <system/audio.h>
#include <utils/String8.h>
#include <cutils/bitops.h>

#define CHANNEL_MASK_USE_CHANNEL_ORDER 0

// NOTE: This code assumes the device processor is little endian.

namespace android {
	
// MediaServer is capable of handling float extractor output, but general processes
// may not be able to do so.
// TODO: Improve API to set extractor float output.
// (Note: duplicated with FLACExtractor.cpp)
static inline bool shouldExtractorOutputFloat(int bitsPerSample)
{
    return bitsPerSample > 16 && AIBinder_getCallingUid() == AID_MEDIA;
}

enum {
    WAVE_FORMAT_PCM        = 0x0001,
    WAVE_FORMAT_IEEE_FLOAT = 0x0003,
    WAVE_FORMAT_ALAW       = 0x0006,
    WAVE_FORMAT_MULAW      = 0x0007,
    WAVE_FORMAT_MSGSM      = 0x0031,
    WAVE_FORMAT_EXTENSIBLE = 0xFFFE,
    WAVE_FORMAT_MSADPCM    = 0x0002,
    WAVE_FORMAT_DVI_IMAADCPM = 0x0011
};

static uint32_t U32_LE_AT(const uint8_t *ptr) {
    return ptr[3] << 24 | ptr[2] << 16 | ptr[1] << 8 | ptr[0];
}

static uint16_t U16_LE_AT(const uint8_t *ptr) {
    return ptr[1] << 8 | ptr[0];
}

struct MtkADPCMSource : public MediaTrackHelper {
    MtkADPCMSource(
            DataSourceHelper *dataSource,
            AMediaFormat *meta,
            uint16_t waveFormat,
            bool outputFloat,
            off64_t offset, size_t size);

    virtual media_status_t start();
    virtual media_status_t stop();
    virtual media_status_t getFormat(AMediaFormat *meta);

    virtual media_status_t read(
            MediaBufferHelper **buffer, const ReadOptions *options = NULL);

    virtual bool supportNonblockingRead() { return true; }

protected:
    virtual ~MtkADPCMSource();

private:
    static const size_t kMaxFrameSize;

    DataSourceHelper *mDataSource;
    AMediaFormat *mMeta;
    uint16_t mWaveFormat;
    const bool mOutputFloat;
    int32_t mSampleRate;
    int32_t mNumChannels;
    int32_t mBitsPerSample;
    off64_t mOffset;
    size_t mSize;
    bool mStarted;
    off64_t mCurrentPos;

    int64_t mBlockDurationUs;
    int32_t mBlockAlign;

    MtkADPCMSource(const MtkADPCMSource &);
    MtkADPCMSource &operator=(const MtkADPCMSource &);
};

MtkADPCMExtractor::MtkADPCMExtractor(DataSourceHelper *source)
    : mDataSource(source),
      mValidFormat(false),
      mChannelMask(CHANNEL_MASK_USE_CHANNEL_ORDER) {

      mAvgBytesPerSec = 0;
      mBlockAlign = 0;
      mExtraDataSize = 0;
      mpExtraData = NULL;
      mSamplesPerBlock = 0;
      mSamplesNumberPerChannel = 0;
      mBlockDurationUs = 0;

      mTrackMeta = AMediaFormat_new();
      mInitCheck = init();
}

MtkADPCMExtractor::~MtkADPCMExtractor() {
    if (NULL != mpExtraData) {
        free(mpExtraData);
        mpExtraData = NULL;
    }

    delete mDataSource;
    AMediaFormat_delete(mTrackMeta);
}

media_status_t MtkADPCMExtractor::getMetaData(AMediaFormat *meta) {
    AMediaFormat_clear(meta);
    if (mInitCheck == OK) {
        AMediaFormat_setString(meta, AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_CONTAINER_WAV);
    }

    return AMEDIA_OK;
}

size_t MtkADPCMExtractor::countTracks() {
    return mInitCheck == OK ? 1 : 0;
}

MediaTrackHelper *MtkADPCMExtractor::getTrack(size_t index) {
    if (mInitCheck != OK || index > 0) {
        return NULL;
    }

    return new MtkADPCMSource(
            mDataSource, mTrackMeta,
            mWaveFormat, shouldExtractorOutputFloat(mBitsPerSample), mDataOffset, mDataSize);
}

media_status_t MtkADPCMExtractor::getTrackMetaData(
        AMediaFormat *meta,
        size_t index, uint32_t /* flags */) {
    if (mInitCheck != OK || index > 0) {
        return AMEDIA_ERROR_UNKNOWN;
    }

    const media_status_t status = AMediaFormat_copy(meta, mTrackMeta);
    if (status == OK) {
        AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_PCM_ENCODING,
                kAudioEncodingPcm16bit);
    }
    return status;
}

status_t MtkADPCMExtractor::init() {
    uint8_t header[12];
    if (mDataSource->readAt(
                0, header, sizeof(header)) < (ssize_t)sizeof(header)) {
        return NO_INIT;
    }

    if (memcmp(header, "RIFF", 4) || memcmp(&header[8], "WAVE", 4)) {
        return NO_INIT;
    }

    size_t totalSize = U32_LE_AT(&header[4]);

    off64_t offset = 12;
    size_t remainingSize = totalSize;
    while (remainingSize >= 8) {
        uint8_t chunkHeader[8];
        if (mDataSource->readAt(offset, chunkHeader, 8) < 8) {
            return NO_INIT;
        }

        remainingSize -= 8;
        offset += 8;

        uint32_t chunkSize = U32_LE_AT(&chunkHeader[4]);

        if (chunkSize > remainingSize) {
            return NO_INIT;
        }

        if (!memcmp(chunkHeader, "fmt ", 4)) {
            if (chunkSize < 16) {
                return NO_INIT;
            }

            uint8_t formatSpec[40];
            if (mDataSource->readAt(offset, formatSpec, 2) < 2) {
                return NO_INIT;
            }

            mWaveFormat = U16_LE_AT(formatSpec);
            if (mWaveFormat != WAVE_FORMAT_MSADPCM
                    && mWaveFormat != WAVE_FORMAT_DVI_IMAADCPM) {
                return AMEDIA_ERROR_UNSUPPORTED;
            }

            uint8_t fmtSize = 16;

            if (mDataSource->readAt(offset, formatSpec, fmtSize) < fmtSize) {
                return NO_INIT;
            }

            mNumChannels = U16_LE_AT(&formatSpec[2]);

            if (mNumChannels < 1 || mNumChannels > FCC_8) {
                ALOGE("Unsupported number of channels (%d)", mNumChannels);
                return AMEDIA_ERROR_UNSUPPORTED;
            }

            if (mWaveFormat != WAVE_FORMAT_EXTENSIBLE) {
                if (mNumChannels != 1 && mNumChannels != FCC_2) {
                    ALOGW("More than 2 channels (%d) in non-WAVE_EXT, unknown channel mask",
                            mNumChannels);
                }
            }

            mSampleRate = U32_LE_AT(&formatSpec[4]);

            if (mSampleRate == 0) {
                return ERROR_MALFORMED;
            }

            ALOGV("mNumChannels is %d, mSampleRate is %u", mNumChannels, mSampleRate);

            mAvgBytesPerSec = U32_LE_AT(&formatSpec[8]);
            if (mAvgBytesPerSec <= 0) {
                return ERROR_MALFORMED;
            }

            mBlockAlign = U16_LE_AT(&formatSpec[12]);
            if (mBlockAlign <= 0) {
                return ERROR_MALFORMED;
            }

            if (mWaveFormat == WAVE_FORMAT_MSADPCM ||
                    mWaveFormat == WAVE_FORMAT_DVI_IMAADCPM) {
                ALOGV("mBlockAlign is %u", mBlockAlign);
            }

            mBitsPerSample = U16_LE_AT(&formatSpec[14]);

            if (mWaveFormat == WAVE_FORMAT_MSADPCM ||
                    mWaveFormat == WAVE_FORMAT_DVI_IMAADCPM) {
                uint8_t extraData[2];
                if (mDataSource->readAt(offset+16, extraData, 2) < 2) {
                    return NO_INIT;
                }

                mExtraDataSize = U16_LE_AT(extraData);
                if (mExtraDataSize < 2) {
                    return ERROR_MALFORMED;
                }

                mpExtraData = (uint8_t*)malloc(mExtraDataSize);
                if (NULL == mpExtraData) {
                    ALOGE("ADPCM malloc extraDataSize failed !!!");
                    return ERROR_IO;
                } else {
                    ALOGV("ADPCM mExtraDataSize is %u", mExtraDataSize);
                    uint32_t n = mDataSource->readAt(offset+18, mpExtraData, mExtraDataSize);
                    if (n < mExtraDataSize) {
                        return ERROR_MALFORMED;
                    }
                }
                mSamplesPerBlock = U16_LE_AT(mpExtraData);
            }

            if (mWaveFormat == WAVE_FORMAT_MSADPCM
                || mWaveFormat == WAVE_FORMAT_DVI_IMAADCPM) {
                if (mBitsPerSample != 4) {
                    return ERROR_UNSUPPORTED;
                }
            } else {
                return ERROR_UNSUPPORTED;
            }

            mValidFormat = true;
        } else if (!memcmp(chunkHeader, "fact", 4)) {
            if (chunkSize != 4) {
                ALOGD("fact chunk size is invailed, chunkSize is %u !!!", chunkSize);
            }
            uint8_t factChunkData[4];
            mDataSource->readAt(offset, factChunkData, 4);

            mSamplesNumberPerChannel = U32_LE_AT(factChunkData);
            ALOGV("fact chunk ChannelCount is %d, SamplesNumberPerChannel is %u, SamplesPerBlock is %u",
                          mNumChannels, mSamplesNumberPerChannel, mSamplesPerBlock);
        } else if (!memcmp(chunkHeader, "data", 4)) {
            if (mValidFormat) {
                mDataOffset = offset;
                mDataSize = chunkSize;

                AMediaFormat_clear(mTrackMeta);

                switch (mWaveFormat) {
                    case WAVE_FORMAT_MSADPCM:
                        AMediaFormat_setString(mTrackMeta,
                                AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_AUDIO_MS_ADPCM);
                        break;
                    case WAVE_FORMAT_DVI_IMAADCPM:
                        AMediaFormat_setString(mTrackMeta,
                                AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_AUDIO_DVI_IMA_ADPCM);
                        break;
                    default:
                        CHECK_EQ(mWaveFormat, (uint16_t)WAVE_FORMAT_MULAW);
                        AMediaFormat_setString(mTrackMeta,
                                AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_AUDIO_G711_MLAW);
                        break;
                }

                AMediaFormat_setInt32(mTrackMeta, AMEDIAFORMAT_KEY_CHANNEL_COUNT, mNumChannels);
                AMediaFormat_setInt32(mTrackMeta, AMEDIAFORMAT_KEY_CHANNEL_MASK, mChannelMask);
                AMediaFormat_setInt32(mTrackMeta, AMEDIAFORMAT_KEY_SAMPLE_RATE, mSampleRate);
                AMediaFormat_setInt32(mTrackMeta, AMEDIAFORMAT_KEY_BITS_PER_SAMPLE, mBitsPerSample);
                AMediaFormat_setInt32(mTrackMeta, "block-align", mBlockAlign);

                ALOGV("set value for metaData !!!");

                int64_t durationUs = 0;

                ALOGV("set duration value for metaData !!!");
                ALOGV("mSamplesPerBlock %u, mSampleRate %u, mDataSize %zu, mBlockAlign %u",
                        mSamplesPerBlock, mSampleRate, mDataSize, mBlockAlign);
                        mBlockDurationUs = 1000000LL * mSamplesPerBlock / mSampleRate;
                durationUs = (mDataSize / mBlockAlign) * mBlockDurationUs;
                ALOGV("mBlockDurationUs is %.2f secs, durationUs is %.2f secs",
                                   mBlockDurationUs / 1E6, durationUs / 1E6);

                AMediaFormat_setInt64(mTrackMeta, AMEDIAFORMAT_KEY_DURATION, durationUs);
                AMediaFormat_setInt64(mTrackMeta, "block-duration", mBlockDurationUs);
                uint8_t csdBuffer[4];
                memcpy(csdBuffer, &mBlockAlign, sizeof(mBlockAlign));
                AMediaFormat_setBuffer(mTrackMeta, AMEDIAFORMAT_KEY_CSD_0, csdBuffer, sizeof(csdBuffer));
                return OK;
            }
        }

        offset += chunkSize;
    }

    return NO_INIT;
}

const size_t MtkADPCMSource::kMaxFrameSize = 32768;

MtkADPCMSource::MtkADPCMSource(
        DataSourceHelper *dataSource,
        AMediaFormat *meta,
        uint16_t waveFormat,
        bool outputFloat,
        off64_t offset, size_t size)
    : mDataSource(dataSource),
      mMeta(meta),
      mWaveFormat(waveFormat),
      mOutputFloat(outputFloat),
      mOffset(offset),
      mSize(size),
      mStarted(false) {
    mBlockDurationUs = 0;
    mBlockAlign = 0;
    mCurrentPos = 0;

    CHECK(AMediaFormat_getInt32(mMeta, AMEDIAFORMAT_KEY_SAMPLE_RATE, &mSampleRate));
    CHECK(AMediaFormat_getInt32(mMeta, AMEDIAFORMAT_KEY_CHANNEL_COUNT, &mNumChannels));
    CHECK(AMediaFormat_getInt32(mMeta, AMEDIAFORMAT_KEY_BITS_PER_SAMPLE, &mBitsPerSample));
    CHECK(AMediaFormat_getInt64(mMeta, "block-duration", &mBlockDurationUs));
    CHECK(AMediaFormat_getInt32(mMeta, "block-align", &mBlockAlign));
    ALOGV("mSize is %zu, mBlockDurationUs %lld", mSize, (long long)mBlockDurationUs);
}

MtkADPCMSource::~MtkADPCMSource() {
    if (mStarted) {
        stop();
    }
}

media_status_t MtkADPCMSource::start() {
    ALOGV("MtkADPCMSource::start");

    CHECK(!mStarted);

    // some WAV files may have large audio buffers that use shared memory transfer.
    if (!mBufferGroup->init(4 /* buffers */, kMaxFrameSize)) {
        return AMEDIA_ERROR_UNKNOWN;
    }

    mCurrentPos = mOffset;

    mStarted = true;

    return AMEDIA_OK;
}

media_status_t MtkADPCMSource::stop() {
    ALOGV("MtkADPCMSource::stop");

    CHECK(mStarted);

    mStarted = false;

    return AMEDIA_OK;
}

media_status_t MtkADPCMSource::getFormat(AMediaFormat *meta) {
    ALOGV("MtkADPCMSource::getFormat");

    const media_status_t status = AMediaFormat_copy(meta, mMeta);
    if (status == OK) {
        AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_MAX_INPUT_SIZE, kMaxFrameSize);
        AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_PCM_ENCODING, kAudioEncodingPcm16bit);
    }
    return status;
}

media_status_t MtkADPCMSource::read(
        MediaBufferHelper **out, const ReadOptions *options) {
    *out = NULL;

    if (options != nullptr && options->getNonBlocking() && !mBufferGroup->has_buffers()) {
        return AMEDIA_ERROR_WOULD_BLOCK;
    }

    int64_t seekTimeUs;
    ReadOptions::SeekMode mode;
    if (options != NULL && options->getSeekTo(&seekTimeUs, &mode)) {
        int64_t pos = 0;

        pos = (seekTimeUs - (int64_t)(mBlockDurationUs >> 1)) / mBlockDurationUs * mBlockAlign;
        if (pos < 0) {
            pos = 0;
        }
        ALOGV("ADPCM seekTimeUs is %.2f secs", seekTimeUs / 1E6);
        ALOGV("ADPCM mOffset %llu, pos %lld", (unsigned long long)mOffset, (long long)pos);

        if (pos > (off64_t)mSize) {
            pos = mSize;
        }
        mCurrentPos = pos + mOffset;
    }

    MediaBufferHelper *buffer;
    media_status_t err = mBufferGroup->acquire_buffer(&buffer);
    if (err != OK) {
        return err;
    }

    // maxBytesToRead may be reduced so that in-place data conversion will fit in buffer size.
    const size_t bufferSize = buffer->size();
    size_t maxBytesToRead;
    if (mOutputFloat) { // destination is float at 4 bytes per sample, source may be less.
        maxBytesToRead = (mBitsPerSample / 8) * (bufferSize / 4);
    } else { // destination is int16_t at 2 bytes per sample, only source of 8 bits is less.
        maxBytesToRead = mBitsPerSample == 8 ? bufferSize / 2 : bufferSize;
    }

    if (mWaveFormat == WAVE_FORMAT_MSADPCM ||
        mWaveFormat == WAVE_FORMAT_DVI_IMAADCPM) {
        maxBytesToRead = (bufferSize / 4 / mBlockAlign) * mBlockAlign; // divide 4 to decrease component output buffer size
    }

    size_t maxBytesAvailable =
        (mCurrentPos - mOffset >= (off64_t)mSize)
            ? 0 : mSize - (mCurrentPos - mOffset);

    if (maxBytesToRead > maxBytesAvailable) {
        maxBytesToRead = maxBytesAvailable;
        if (mWaveFormat == WAVE_FORMAT_MSADPCM ||
            mWaveFormat == WAVE_FORMAT_DVI_IMAADCPM) {
            maxBytesToRead = (maxBytesToRead / mBlockAlign) * mBlockAlign;
        }
    }

    // read only integral amounts of audio unit frames.
    const size_t inputUnitFrameSize = mNumChannels * mBitsPerSample / 8;
    if (0 != inputUnitFrameSize) {
        maxBytesToRead -= maxBytesToRead % inputUnitFrameSize;
    }

    ssize_t n = mDataSource->readAt(
            mCurrentPos, buffer->data(),
            maxBytesToRead);

    if (n <= 0) {
        buffer->release();
        buffer = NULL;

        return AMEDIA_ERROR_END_OF_STREAM;
    }

    buffer->set_range(0, n);

    ALOGV("======ADPCM Data pass MTK ADPCM Component !======");

    if (mWaveFormat == WAVE_FORMAT_MSADPCM
           || mWaveFormat == WAVE_FORMAT_DVI_IMAADCPM) {
        ALOGV("ADPCM timestamp of this buffer, mBlockAlign is %d, mBlockDurationUs is %lld +++",
                       mBlockAlign,  (long long)mBlockDurationUs);
        int64_t keyTimeUs = ((mCurrentPos - mOffset) / mBlockAlign) * mBlockDurationUs;
		AMediaFormat_setInt64(buffer->meta_data(), AMEDIAFORMAT_KEY_TIME_US, keyTimeUs);
        ALOGV("ADPCM timestamp of this buffer is %.2f secs, buffer length is %zd", keyTimeUs / 1E6, n);
    }

    AMediaFormat *meta = buffer->meta_data();
//    AMediaFormat_setInt64(meta, AMEDIAFORMAT_KEY_TIME_US, timeStampUs);
    AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_IS_SYNC_FRAME, 1);

    mCurrentPos += n;

    *out = buffer;

    return AMEDIA_OK;
}

////////////////////////////////////////////////////////////////////////////////

static CMediaExtractor* CreateExtractor(
        CDataSource *source,
        void *) {
    return wrap(new MtkADPCMExtractor(new DataSourceHelper(source)));
}

static CreatorFunc Sniff(
        CDataSource *source,
        float *confidence,
        void **,
        FreeMetaFunc *) {
    DataSourceHelper *helper = new DataSourceHelper(source);
    char header[12];
    if (helper->readAt(0, header, sizeof(header)) < (ssize_t)sizeof(header)) {
    	  delete helper;
        return NULL;
    }

    if (memcmp(header, "RIFF", 4) || memcmp(&header[8], "WAVE", 4)) {
    	  delete helper;
        return NULL;
    }

    MtkADPCMExtractor *extractor = new MtkADPCMExtractor(helper);
    int numTracks = extractor->countTracks();
    delete extractor;
    if (numTracks == 0) {
        return NULL;
    }

    *confidence = 0.3f;

    return CreateExtractor;
}

static const char *extensions[] = {
    "wav",
    NULL
};

extern "C" {
// This is the only symbol that needs to be exported
__attribute__ ((visibility ("default")))
ExtractorDef GETEXTRACTORDEF() {
    return {
        EXTRACTORDEF_VERSION,
        UUID("9271997f-a8be-41a0-881b-64561a14a21a"),
        1, // version
        "MtkADPCM Extractor",
        { .v3 = {Sniff, extensions} }
    };
}

} // extern "C"

}  // namespace android
