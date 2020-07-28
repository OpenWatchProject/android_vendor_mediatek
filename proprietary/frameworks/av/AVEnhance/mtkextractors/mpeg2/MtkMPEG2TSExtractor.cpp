/*
 * Copyright (C) 2010 The Android Open Source Project
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
#define LOG_TAG "MTKMPEG2TSExtractor"

#include <inttypes.h>
#include <utils/Log.h>

#include <android-base/macros.h>

#include "MtkMPEG2TSExtractor.h"

#include <media/DataSourceBase.h>
#include <media/IStreamSource.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AUtils.h>
#include <media/stagefright/foundation/MediaKeys.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/Utils.h>
#include <utils/String8.h>

#include "mpeg2ts/MtkAnotherPacketSource.h"
#include "mpeg2ts/MtkATSParser.h"

#include <hidl/HybridInterface.h>
#include <android/hardware/cas/1.0/ICas.h>

namespace android {

using hardware::cas::V1_0::ICas;

static const size_t kTSPacketSize = 188;
static const int kMaxDurationReadSize = 250000LL;
static const int kMaxDurationRetry = 6;
const static int64_t kMaxPTSTimeOutUs = 3000000LL;

struct MPEG2TSSource : public MediaTrackHelper {
    MPEG2TSSource(
            MPEG2TSExtractor *extractor,
            const sp<AnotherPacketSource> &impl,
            bool doesSeek);
    virtual ~MPEG2TSSource();

    virtual media_status_t start();
    virtual media_status_t stop();
    virtual media_status_t getFormat(AMediaFormat *);

    virtual media_status_t read(
            MediaBufferHelper **buffer, const ReadOptions *options = NULL);

private:
    MPEG2TSExtractor *mExtractor;
    sp<AnotherPacketSource> mImpl;

    // If there are both audio and video streams, only the video stream
    // will signal seek on the extractor; otherwise the single stream will seek.
    bool mDoesSeek;
    // mtk seek patch ++
    bool mIsVideo;
    // mtk seek patch --

    DISALLOW_EVIL_CONSTRUCTORS(MPEG2TSSource);
};

MPEG2TSSource::MPEG2TSSource(
        MPEG2TSExtractor *extractor,
        const sp<AnotherPacketSource> &impl,
        bool doesSeek)
    : mExtractor(extractor),
      mImpl(impl),
      mDoesSeek(doesSeek) {
      mIsVideo = true;
}

MPEG2TSSource::~MPEG2TSSource() {
}

media_status_t MPEG2TSSource::start() {
    // initialize with one small buffer, but allow growth
    mBufferGroup->init(1 /* one buffer */, 256 /* buffer size */, 64 /* max number of buffers */);

    if (!mImpl->start(NULL)) { // AnotherPacketSource::start() doesn't use its argument
        return AMEDIA_OK;
    }
    return AMEDIA_ERROR_UNKNOWN;
}

media_status_t MPEG2TSSource::stop() {
//  mtk seek patch ++
    // add for video short than audio, video stop then audio can do seek
    ALOGD("Stop Video=%d track", mIsVideo);
    if (mIsVideo == true)
        mExtractor->setVideoState(true);
//  mtk seek patc --
    if (!mImpl->stop()) {
        return AMEDIA_OK;
    }
    return AMEDIA_ERROR_UNKNOWN;
}

void copyAMessageToAMediaFormat(AMediaFormat *format, sp<AMessage> msg) {
    size_t numEntries = msg->countEntries();
    for (size_t i = 0; i < numEntries; i++) {
        AMessage::Type type;
        const char *name = msg->getEntryNameAt(i, &type);
        AMessage::ItemData id = msg->getEntryAt(i);

        switch (type) {
            case AMessage::kTypeInt32:
                int32_t val32;
                if (id.find(&val32)) {
                    AMediaFormat_setInt32(format, name, val32);
                }
                break;
            case AMessage::kTypeInt64:
                int64_t val64;
                if (id.find(&val64)) {
                    AMediaFormat_setInt64(format, name, val64);
                }
                break;
            case AMessage::kTypeFloat:
                float valfloat;
                if (id.find(&valfloat)) {
                    AMediaFormat_setFloat(format, name, valfloat);
                }
                break;
            case AMessage::kTypeDouble:
                double valdouble;
                if (id.find(&valdouble)) {
                    AMediaFormat_setDouble(format, name, valdouble);
                }
                break;
            case AMessage::kTypeString:
                if (AString s; id.find(&s)) {
                    AMediaFormat_setString(format, name, s.c_str());
                }
                break;
            case AMessage::kTypeBuffer:
            {
                sp<ABuffer> buffer;
                if (id.find(&buffer)) {
                    AMediaFormat_setBuffer(format, name, buffer->data(), buffer->size());
                }
                break;
            }
            default:
                ALOGW("ignoring unsupported type %d '%s'", type, name);
        }
    }
}

media_status_t MPEG2TSSource::getFormat(AMediaFormat *meta) {
    sp<MetaData> implMeta = mImpl->getFormat();
    int64_t durationUs;
    if (!implMeta->findInt64(kKeyDuration, &durationUs))
        implMeta->setInt64(kKeyDuration, mExtractor->getDurationUs());
//  seek patch ++    mIsVideo
    const char *mime;
    CHECK(implMeta->findCString(kKeyMIMEType, &mime));
    if (strncasecmp("video/", mime, 6))
        mIsVideo = false;
//  seek patch --
    sp<AMessage> msg;
    convertMetaDataToMessage(implMeta, &msg);
    copyAMessageToAMediaFormat(meta, msg);
    return AMEDIA_OK;
}

media_status_t MPEG2TSSource::read(
        MediaBufferHelper **out, const ReadOptions *options) {
    *out = NULL;

    int64_t seekTimeUs;
    ReadOptions::SeekMode seekMode;

//  seek patch ++
    if (options && options->getSeekTo(&seekTimeUs, &seekMode)) {
        if (mExtractor->getVideoState() && mExtractor->IsLocalSource() && !mIsVideo && !mDoesSeek) {
            mDoesSeek = true;
            ALOGE("Audio can seek now");
        }
        if ((mExtractor->countSource() == 1) && !mDoesSeek) {
            mDoesSeek = true;
            ALOGD("Extractor only has one track, enable seek");
        }
        if (mDoesSeek) {
            if (mExtractor->IsLocalSource()) {  // loacal seek use mtk
                mExtractor->seekTo(seekTimeUs, seekMode);
                mImpl->queueDiscontinuity(ATSParser::DISCONTINUITY_NONE, NULL, false);
            } else {
                // http streaming seek use google default
                status_t err = mExtractor->seek(seekTimeUs, seekMode);
                if (err == ERROR_END_OF_STREAM) {
                    return AMEDIA_ERROR_END_OF_STREAM;
                } else if (err != OK) {
                    return AMEDIA_ERROR_UNKNOWN;
                }
            }
        }
    }

/*
    if (mDoesSeek && options && options->getSeekTo(&seekTimeUs, &seekMode)) {
        // seek is needed
        status_t err = mExtractor->seek(seekTimeUs, (ReadOptions::SeekMode)seekMode);
        if (err == ERROR_END_OF_STREAM) {
            return AMEDIA_ERROR_END_OF_STREAM;
        } else if (err != OK) {
            return AMEDIA_ERROR_UNKNOWN;
        }
    }
*/
//  seek patch --


    if (mExtractor->feedUntilBufferAvailable(mImpl) != OK) {
        return AMEDIA_ERROR_END_OF_STREAM;
    }

    MediaBufferBase *mbuf;
    // add for mtk seek method
    // only seek when has both video and audio, need drop audio frames queued in anotherpacketsource before seektime.
    // This seektime is video I frame been found after seek. But audio been queued from seektime set by ap.
    if (options && options->getSeekTo(&seekTimeUs, &seekMode)
        && !(mExtractor->getVideoState()) && mExtractor->IsLocalSource() && !mIsVideo) {
        ALOGV("Audio first frame after seek, time should > %lld", (long long)seekTimeUs);
        status_t err = OK;
        while ((err = mImpl->read(&mbuf, (MediaTrack::ReadOptions*) options)) == OK) {
            int64_t timeUs;
            if (mbuf->meta_data().findInt64(kKeyTime, &timeUs)) {
                if (timeUs == 0) {
                    break;
                }
                if (timeUs < seekTimeUs) {
                    ALOGV("buffer time (%lld) < seektime (%lld)", (long long)timeUs, (long long)seekTimeUs);
                    mbuf->release();
                    mbuf = NULL;
                    if (mExtractor->feedUntilBufferAvailable(mImpl) != OK) {
                        return AMEDIA_ERROR_END_OF_STREAM;
                    }
                } else {
                    ALOGD("find audio first buffer(%lld) after seek time(%lld)",
                            (long long)timeUs, (long long)seekTimeUs);
                    break;
                }
            }
        }
        if (err != OK)
            return AMEDIA_ERROR_UNKNOWN;
    } else {
        mImpl->read(&mbuf, (MediaTrack::ReadOptions*) options);
    }
    //  seek patch --

    size_t length = mbuf->range_length();
    MediaBufferHelper *outbuf;
    mBufferGroup->acquire_buffer(&outbuf, false, length);
    memcpy(outbuf->data(), mbuf->data(), length);
    outbuf->set_range(0, length);
    *out = outbuf;
    MetaDataBase &inMeta = mbuf->meta_data();
    AMediaFormat *outMeta = outbuf->meta_data();
    AMediaFormat_clear(outMeta);
    int64_t val64;
    if (inMeta.findInt64(kKeyTime, &val64)) {
        AMediaFormat_setInt64(outMeta, AMEDIAFORMAT_KEY_TIME_US, val64);
    }
    int32_t val32;
    if (inMeta.findInt32(kKeyIsSyncFrame, &val32)) {
        AMediaFormat_setInt32(outMeta, AMEDIAFORMAT_KEY_IS_SYNC_FRAME, val32);
    }
    if (inMeta.findInt32(kKeyCryptoMode, &val32)) {
        AMediaFormat_setInt32(outMeta, AMEDIAFORMAT_KEY_CRYPTO_MODE, val32);
    }
    uint32_t bufType;
    const void *bufData;
    size_t bufSize;
    if (inMeta.findData(kKeyCryptoIV, &bufType, &bufData, &bufSize)) {
        AMediaFormat_setBuffer(outMeta, AMEDIAFORMAT_KEY_CRYPTO_IV, bufData, bufSize);
    }
    if (inMeta.findData(kKeyCryptoKey, &bufType, &bufData, &bufSize)) {
        AMediaFormat_setBuffer(outMeta, AMEDIAFORMAT_KEY_CRYPTO_KEY, bufData, bufSize);
    }
    if (inMeta.findData(kKeyPlainSizes, &bufType, &bufData, &bufSize)) {
        AMediaFormat_setBuffer(outMeta, AMEDIAFORMAT_KEY_CRYPTO_PLAIN_SIZES, bufData, bufSize);
    }
    if (inMeta.findData(kKeyEncryptedSizes, &bufType, &bufData, &bufSize)) {
        AMediaFormat_setBuffer(outMeta, AMEDIAFORMAT_KEY_CRYPTO_ENCRYPTED_SIZES, bufData, bufSize);
    }
    if (inMeta.findData(kKeySEI, &bufType, &bufData, &bufSize)) {
        AMediaFormat_setBuffer(outMeta, AMEDIAFORMAT_KEY_SEI, bufData, bufSize);
    }
    if (inMeta.findData(kKeyAudioPresentationInfo, &bufType, &bufData, &bufSize)) {
        AMediaFormat_setBuffer(outMeta, AMEDIAFORMAT_KEY_AUDIO_PRESENTATION_INFO, bufData, bufSize);
    }
    mbuf->release();
    return AMEDIA_OK;
}

////////////////////////////////////////////////////////////////////////////////

MPEG2TSExtractor::MPEG2TSExtractor(DataSourceHelper *source)
    : mDataSource(source),
      mParser(IsLocalSource() ?
        new mtk::ATSParser(ATSParser::TS_SOURCE_IS_LOCAL) : new ATSParser),
      mLastSyncEvent(0),
      mOffset(0) {
    char header;
    if (source->readAt(0, &header, 1) == 1 && header == 0x47) {
        mHeaderSkip = 0;
    } else {
        mHeaderSkip = 4;
    }
    init();
}

MPEG2TSExtractor::~MPEG2TSExtractor() {
    delete mDataSource;
}

size_t MPEG2TSExtractor::countTracks() {
    return mSourceImpls.size();
}

MediaTrackHelper *MPEG2TSExtractor::getTrack(size_t index) {
    if (index >= mSourceImpls.size()) {
        return NULL;
    }
//  seek patch ++
    //  add for mtk seek method
    //  http streaming seek use google default method, local use mtk seek
    if (IsLocalSource()) {
        bool doesSeek = true;
        if (mSourceImpls.size() > 1) {
            CHECK_EQ(mSourceImpls.size(), 2u);
            sp<MetaData> meta = mSourceImpls.editItemAt(index)->getFormat();
            const char *mime;
            CHECK(meta->findCString(kKeyMIMEType, &mime));
            if (!strncasecmp("audio/", mime, 6)) {
                haveAudioSource = true;
                doesSeek = false;
            } else if (!strncasecmp("video/", mime, 6)) {
                haveVideoSource = true;
            }
        }
        return new MPEG2TSSource(this, mSourceImpls.editItemAt(index), doesSeek);
    } else

//  seek patch --

    // The seek reference track (video if present; audio otherwise) performs
    // seek requests, while other tracks ignore requests.
    return new MPEG2TSSource(this, mSourceImpls.editItemAt(index),
            (mSeekSyncPoints == &mSyncPoints.editItemAt(index)));
}

media_status_t MPEG2TSExtractor::getTrackMetaData(
        AMediaFormat *meta,
        size_t index, uint32_t /* flags */) {
    sp<MetaData> implMeta = index < mSourceImpls.size()
        ? mSourceImpls.editItemAt(index)->getFormat() : NULL;
    if (implMeta == NULL) {
        return AMEDIA_ERROR_UNKNOWN;
    }
    int64_t durationUs;
    if (!implMeta->findInt64(kKeyDuration, &durationUs))
        implMeta->setInt64(kKeyDuration, mDurationUs);
    sp<AMessage> msg = new AMessage;
    convertMetaDataToMessage(implMeta, &msg);
    copyAMessageToAMediaFormat(meta, msg);
    return AMEDIA_OK;
}

media_status_t MPEG2TSExtractor::getMetaData(AMediaFormat *meta) {
    AMediaFormat_setString(meta, AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_CONTAINER_MPEG2TS);
    return AMEDIA_OK;
}

//static
bool MPEG2TSExtractor::isScrambledFormat(MetaDataBase &format) {
    const char *mime;
    return format.findCString(kKeyMIMEType, &mime)
            && (!strcasecmp(MEDIA_MIMETYPE_VIDEO_SCRAMBLED, mime)
                    || !strcasecmp(MEDIA_MIMETYPE_AUDIO_SCRAMBLED, mime));
}

media_status_t MPEG2TSExtractor::setMediaCas(const uint8_t* casToken, size_t size) {
    HalToken halToken;
    halToken.setToExternal((uint8_t*)casToken, size);
    sp<ICas> cas = ICas::castFrom(retrieveHalInterface(halToken));
    ALOGD("setMediaCas: %p", cas.get());

    status_t err = mParser->setMediaCas(cas);
    if (err == OK) {
        ALOGI("All tracks now have descramblers");
        init();
        return AMEDIA_OK;
    }
    return AMEDIA_ERROR_UNKNOWN;
}

status_t MPEG2TSExtractor::findIndexOfSource(const sp<AnotherPacketSource> &impl, size_t *index) {
    for (size_t i = 0; i < mSourceImpls.size(); i++) {
        if (mSourceImpls[i] == impl) {
            *index = i;
            return OK;
        }
    }
    return NAME_NOT_FOUND;
}

void MPEG2TSExtractor::addSource(const sp<AnotherPacketSource> &impl) {
    size_t index;
    if (findIndexOfSource(impl, &index) != OK) {
        mSourceImpls.push(impl);
        mSyncPoints.push();
    }
}

void MPEG2TSExtractor::init() {
// seek patch ++
// add for mtk seek method
    mSeekTimeUs = 0;
    mSeeking = false;
    mSeekingOffset = 0;
    mMinOffset = 0;
    mMaxOffset = 0;
    mMaxcount = 0;
    mVideoUnSupportedByDecoder = false;
    End_OF_FILE = false;
    mSeekMode = MediaTrackHelper::ReadOptions::SEEK_NEXT_SYNC;
// seek patch --
    bool haveAudio = false;
    bool haveVideo = false;
    haveVideoSource = false;
    haveAudioSource = false;
    mSourceNum = 0;
    size_t index;
    mDurationUs = 0;
    mFileSize = 0;
    mFileDataSize = 0;
    mFindingMaxPTS = false;
    mOffsetPAT = 0;
    // http streaming calculate duration use google default method
    if (!(mDataSource->flags() & DataSourceBase::kIsCachingDataSource)) {
        status_t err = parseMaxPTS();
        if (err != OK) {
            // for ALPS03097043, if get duration fail, return will cause can't play.
            // clear data queue, and setDequeueState true as it was set to false when parseMaxPTS.
            mParser->signalDiscontinuity(ATSParser::DISCONTINUITY_TIME, NULL);
            mParser->setDequeueState(true);
        }
        mOffset = 0;
        End_OF_FILE = false;
    }

    int64_t startTime = ALooper::GetNowUs();
    status_t err;
    while ((err = feedMore(true /* isInit */)) == OK
            || err == ERROR_DRM_DECRYPT_UNIT_NOT_INITIALIZED) {
        if (haveAudio && haveVideo) {
            addSyncPoint_l(mLastSyncEvent);
            mLastSyncEvent.reset();
            break;
        }
        if (!haveVideo) {
            sp<AnotherPacketSource> impl = mParser->getSource(ATSParser::VIDEO);

            if (impl != NULL) {
                sp<MetaData> format = impl->getFormat();
                if (format != NULL) {
                    haveVideo = true;
                    addSource(impl);
                    if (!isScrambledFormat(*(format.get()))) {
                        if (findIndexOfSource(impl, &index) == OK) {
                            mSeekSyncPoints = &mSyncPoints.editItemAt(index);
                        }
                    }
                }
            }
        }

        if (!haveAudio) {
            sp<AnotherPacketSource> impl = mParser->getSource(ATSParser::AUDIO);

            if (impl != NULL) {
                sp<MetaData> format = impl->getFormat();
                if (format != NULL) {
                    haveAudio = true;
                    addSource(impl);
                    if (!isScrambledFormat(*(format.get())) && !haveVideo) {
                        if (findIndexOfSource(impl, &index) == OK) {
                            mSeekSyncPoints = &mSyncPoints.editItemAt(index);
                        }
                    }
                }
            }
        }

        addSyncPoint_l(mLastSyncEvent);
        mLastSyncEvent.reset();

        // ERROR_DRM_DECRYPT_UNIT_NOT_INITIALIZED is returned when the mpeg2ts
        // is scrambled but we don't have a MediaCas object set. The extraction
        // will only continue when setMediaCas() is called successfully.
        if (err == ERROR_DRM_DECRYPT_UNIT_NOT_INITIALIZED) {
            ALOGI("stopped parsing scrambled content, "
                  "haveAudio=%d, haveVideo=%d, elaspedTime=%" PRId64,
                    haveAudio, haveVideo, ALooper::GetNowUs() - startTime);
            return;
        }

        // Wait only for 3 seconds to detect audio/video streams.
        if (ALooper::GetNowUs() - startTime > 3000000LL) {
            break;
        }
    }

    off64_t size;
    if (mDataSource->flags() & DataSourceBase::kIsCachingDataSource) {
        if (mDataSource->getSize(&size) == OK && (haveAudio || haveVideo)) {
            size_t prevSyncSize = 1;
            int64_t durationUs = -1;
            List<int64_t> durations;
            // Estimate duration --- stabilize until you get <500ms deviation.
            while (feedMore() == OK
                    && ALooper::GetNowUs() - startTime <= 2000000LL) {
                if (mSeekSyncPoints->size() > prevSyncSize) {
                    prevSyncSize = mSeekSyncPoints->size();
                    int64_t diffUs = mSeekSyncPoints->keyAt(prevSyncSize - 1)
                            - mSeekSyncPoints->keyAt(0);
                    off64_t diffOffset = mSeekSyncPoints->valueAt(prevSyncSize - 1)
                            - mSeekSyncPoints->valueAt(0);
                    int64_t currentDurationUs = size * diffUs / diffOffset;
                    durations.push_back(currentDurationUs);
                    if (durations.size() > 5) {
                        durations.erase(durations.begin());
                        int64_t min = *durations.begin();
                        int64_t max = *durations.begin();
                        for (auto duration : durations) {
                            if (min > duration) {
                                min = duration;
                            }
                            if (max < duration) {
                                max = duration;
                            }
                        }
                        if (max - min < 500 * 1000) {
                            durationUs = currentDurationUs;
                            break;
                        }
                    }
                }
            }

            bool found = false;
            for (int i = 0; i < ATSParser::NUM_SOURCE_TYPES; ++i) {
                ATSParser::SourceType type = static_cast<ATSParser::SourceType>(i);
                sp<AnotherPacketSource> impl = mParser->getSource(type);
                if (impl == NULL) {
                    continue;
                }

                int64_t trackDurationUs = durationUs;

                status_t err;
                int64_t bufferedDurationUs = impl->getBufferedDurationUs(&err);
                if (err == ERROR_END_OF_STREAM) {
                    trackDurationUs = bufferedDurationUs;
                }
                if (trackDurationUs > 0) {
                    ALOGV("[SourceType%d] durationUs=%" PRId64 "", type, trackDurationUs);
                    const sp<MetaData> meta = impl->getFormat();
                    meta->setInt64(kKeyDuration, trackDurationUs);
                    impl->setFormat(meta);

                    found = true;
                }
            }
            if (!found) {
                estimateDurationsFromTimesUsAtEnd();
            }
        }
    }

    ALOGI("haveAudio=%d, haveVideo=%d, elaspedTime=%" PRId64,
            haveAudio, haveVideo, ALooper::GetNowUs() - startTime);
}

status_t MPEG2TSExtractor::feedMore(bool isInit) {
    Mutex::Autolock autoLock(mLock);
//  seek patch ++
    //  add for mtk seek method
    if (mSeeking) {
        int64_t pts = mParser->getMaxPTS();  // [qian] get the max pts in the had read data
        // to solve the problem that when seek to End_OF_FILE then seek to another place ,instantly. and can not play
        if (End_OF_FILE && pts == 0) {
            ALOGE("seek to End_OF_FILE last time");
            mOffset = (off64_t)((((mMinOffset + mMaxOffset) / 2) / (kTSPacketSize + mHeaderSkip))
                * (kTSPacketSize + mHeaderSkip));

            mSeekingOffset = mOffset;
            End_OF_FILE = false;
        }

        if (pts > 0) {
            mMaxcount++;
            switch (mSeekMode) {
                 case MediaTrackHelper::ReadOptions::SEEK_NEXT_SYNC:
                    if ((pts - mSeekTimeUs < 100000 && pts - mSeekTimeUs > 0)
                        || mMinOffset == mMaxOffset || mMaxcount > 16) {
                        ALOGE("seekdone pts=%lld,mSeekTimeUs=%lld,mMaxcount=%lld,mMinOffset=%lld,"
                             "mMaxOffset=%lld, moffset:%lld",
                            (long long)pts/1000, (long long)mSeekTimeUs/1000, (long long)mMaxcount,
                            (long long)mMinOffset, (long long)mMaxOffset, (long long)mOffset);
                        mSeeking = false;
                        mParser->setDequeueState(true);
                    } else {
                        mParser->signalDiscontinuity(ATSParser::DISCONTINUITY_TIME, NULL);
                        if (pts < mSeekTimeUs) {
                            mMinOffset = mSeekingOffset;    // [qian], 1 enter this will begin with the mid of file

                        } else {
                            mMaxOffset = mSeekingOffset;
                        }
                        mSeekingOffset = (off64_t)((((mMinOffset + mMaxOffset) / 2) / (kTSPacketSize + mHeaderSkip))
                            * (kTSPacketSize + mHeaderSkip));

                        mOffset = mSeekingOffset;
                    }
                    ALOGE("pts=%lld,mSeekTimeUs=%lld,mMaxcount=%lld,mOffset=%lld,mMinOffset=%lld,mMaxOffset=%lld",
                         (long long)pts / 1000, (long long)mSeekTimeUs / 1000, (long long)mMaxcount, (long long)mOffset,
                         (long long)mMinOffset, (long long)mMaxOffset);
                            break;
                 case MediaTrackHelper::ReadOptions::SEEK_CLOSEST_SYNC:
                 case MediaTrackHelper::ReadOptions::SEEK_CLOSEST:
                 case MediaTrackHelper::ReadOptions::SEEK_PREVIOUS_SYNC:
                    if ((pts - mSeekTimeUs < 0 && pts - mSeekTimeUs > -100000)
                        || mMinOffset == mMaxOffset || mMaxcount > 16) {
                        ALOGE("closest seekdone pts %lld SeekTimeUs %lld,Maxcount=%lld,MinOffset %lld,"
                            "MaxOffset %lld offset:%lld",
                            (long long)pts/1000, (long long)mSeekTimeUs/1000, (long long)mMaxcount,
                            (long long)mMinOffset, (long long)mMaxOffset, (long long)mOffset);
                        mSeeking = false;
                        mParser->setDequeueState(true);
                    } else {
                        mParser->signalDiscontinuity(ATSParser::DISCONTINUITY_TIME, NULL);
                        if (pts < mSeekTimeUs) {
                            mMinOffset = mSeekingOffset;    // [qian], 1 enter this will begin with the mid of file

                        } else {
                            mMaxOffset = mSeekingOffset;
                        }
                        mSeekingOffset = (off64_t)((((mMinOffset + mMaxOffset) / 2) / (kTSPacketSize + mHeaderSkip))
                            * (kTSPacketSize + mHeaderSkip));

                        mOffset = mSeekingOffset;
                    }
                    ALOGE("pts=%lld,mSeekTimeUs=%lld,mMaxcount=%lld,mOffset=%lld,mMinOffset=%lld,mMaxOffset=%lld",
                         (long long)pts / 1000, (long long)mSeekTimeUs / 1000, (long long)mMaxcount, (long long)mOffset,
                         (long long)mMinOffset, (long long)mMaxOffset);
                    break;
                 default:
                    ALOGE("should not be here");
                    return ERROR_UNSUPPORTED;
            }
        }
    }
//  seek patch --

    uint8_t packet[kTSPacketSize];
    ssize_t n = mDataSource->readAt(mOffset + mHeaderSkip, packet, kTSPacketSize);

    if (n < (ssize_t)kTSPacketSize) {
//  seek patch ++
        // add for mtk duration calculate method and local seek
        ALOGE(" mOffset=%lld,n =%zd", (long long)mOffset, n);
        End_OF_FILE = true;
        if ((n >= 0) && (!mFindingMaxPTS) && (!mSeeking)) {   //  duration patch & seek patch
            mParser->signalEOS(ERROR_END_OF_STREAM);
        }
        if (mSeeking) {
            mSeeking = false;
            mParser->setDequeueState(true);
            ALOGE("seek to end of file, stop seeking");
        }
//  seek patch --

/*
//  duration patch ++    // why not signal EOS to parser when parsing duration
        if ((n >= 0) && (!mFindingMaxPTS)) {
            mParser->signalEOS(ERROR_END_OF_STREAM);
        }
//  duration patch --
*/
        return (n < 0) ? (status_t)n : ERROR_END_OF_STREAM;
    }

    ATSParser::SyncEvent event(mOffset);
    mOffset += mHeaderSkip + n;
    status_t err = mParser->feedTSPacket(packet, kTSPacketSize, &event);
    if (event.hasReturnedData()) {
        if (isInit) {
            mLastSyncEvent = event;
        } else {
            addSyncPoint_l(event);
        }
    }

//  seek patch ++
    // add to avoid find sync word err when seek.
    if (err == BAD_VALUE) {
        int32_t syncOff = 0;
        syncOff = findSyncCode(packet, kTSPacketSize);
        if (syncOff >= 0) {
            mOffset -= n + mHeaderSkip;
            mOffset += syncOff;
            ALOGV("[TS_ERROR]correction once offset mOffset=%lld", (long long)mOffset);
            return OK;
        }
    }
//  seek patch --

    return err;
}

void MPEG2TSExtractor::addSyncPoint_l(const ATSParser::SyncEvent &event) {
    if (!event.hasReturnedData()) {
        return;
    }

    for (size_t i = 0; i < mSourceImpls.size(); ++i) {
        if (mSourceImpls[i].get() == event.getMediaSource().get()) {
            KeyedVector<int64_t, off64_t> *syncPoints = &mSyncPoints.editItemAt(i);
            syncPoints->add(event.getTimeUs(), event.getOffset());
            // We're keeping the size of the sync points at most 5mb per a track.
            size_t size = syncPoints->size();
            if (size >= 327680) {
                int64_t firstTimeUs = syncPoints->keyAt(0);
                int64_t lastTimeUs = syncPoints->keyAt(size - 1);
                if (event.getTimeUs() - firstTimeUs > lastTimeUs - event.getTimeUs()) {
                    syncPoints->removeItemsAt(0, 4096);
                } else {
                    syncPoints->removeItemsAt(size - 4096, 4096);
                }
            }
            break;
        }
    }
}

status_t MPEG2TSExtractor::estimateDurationsFromTimesUsAtEnd()  {
    if (!(mDataSource->flags() & DataSourceBase::kIsLocalFileSource)) {
        return ERROR_UNSUPPORTED;
    }

    off64_t size = 0;
    status_t err = mDataSource->getSize(&size);
    if (err != OK) {
        return err;
    }

    uint8_t packet[kTSPacketSize];
    const off64_t zero = 0;
    off64_t offset = max(zero, size - kMaxDurationReadSize);
    if (mDataSource->readAt(offset, &packet, 0) < 0) {
        return ERROR_IO;
    }

    int retry = 0;
    bool allDurationsFound = false;
    int64_t timeAnchorUs = mParser->getFirstPTSTimeUs();
    do {
        int bytesRead = 0;
        sp<ATSParser> parser = new mtk::ATSParser(ATSParser::TS_TIMESTAMPS_ARE_ABSOLUTE);
        ATSParser::SyncEvent ev(0);
        offset = max(zero, size - (kMaxDurationReadSize << retry));
        offset = (offset / kTSPacketSize) * kTSPacketSize;
        for (;;) {
            if (bytesRead >= kMaxDurationReadSize << max(0, retry - 1)) {
                break;
            }

            ssize_t n = mDataSource->readAt(offset+mHeaderSkip, packet, kTSPacketSize);
            if (n < 0) {
                return n;
            } else if (n < (ssize_t)kTSPacketSize) {
                break;
            }

            offset += kTSPacketSize + mHeaderSkip;
            bytesRead += kTSPacketSize + mHeaderSkip;
            err = parser->feedTSPacket(packet, kTSPacketSize, &ev);
            if (err != OK) {
                return err;
            }

            if (ev.hasReturnedData()) {
                int64_t durationUs = ev.getTimeUs();
                ATSParser::SourceType type = ev.getType();
                ev.reset();

                int64_t firstTimeUs;
                sp<AnotherPacketSource> src = mParser->getSource(type);
                if (src == NULL || src->nextBufferTime(&firstTimeUs) != OK) {
                    continue;
                }
                durationUs += src->getEstimatedBufferDurationUs();
                durationUs -= timeAnchorUs;
                durationUs -= firstTimeUs;
                if (durationUs > 0) {
                    int64_t origDurationUs, lastDurationUs;
                    const sp<MetaData> meta = src->getFormat();
                    const uint32_t kKeyLastDuration = 'ldur';
                    // Require two consecutive duration calculations to be within 1 sec before
                    // updating; use MetaData to store previous duration estimate in per-stream
                    // context.
                    if (!meta->findInt64(kKeyDuration, &origDurationUs)
                            || !meta->findInt64(kKeyLastDuration, &lastDurationUs)
                            || (origDurationUs < durationUs
                             && abs(durationUs - lastDurationUs) < 60000000)) {
                        meta->setInt64(kKeyDuration, durationUs);
                    }
                    meta->setInt64(kKeyLastDuration, durationUs);
                }
            }
        }

        if (!allDurationsFound) {
            allDurationsFound = true;
            for (auto t: {ATSParser::VIDEO, ATSParser::AUDIO}) {
                sp<AnotherPacketSource> src = mParser->getSource(t);
                if (src == NULL) {
                    continue;
                }
                int64_t durationUs;
                const sp<MetaData> meta = src->getFormat();
                if (!meta->findInt64(kKeyDuration, &durationUs)) {
                    allDurationsFound = false;
                    break;
                }
            }
        }

        ++retry;
    } while(!allDurationsFound && offset > 0 && retry <= kMaxDurationRetry);

    return allDurationsFound? OK : ERROR_UNSUPPORTED;
}

uint32_t MPEG2TSExtractor::flags() const {
    return CAN_PAUSE | CAN_SEEK_BACKWARD | CAN_SEEK_FORWARD;
}

status_t MPEG2TSExtractor::seek(int64_t seekTimeUs,
        const MediaTrackHelper::ReadOptions::SeekMode &seekMode) {
    if (mSeekSyncPoints == NULL || mSeekSyncPoints->isEmpty()) {
        ALOGW("No sync point to seek to.");
        // ... and therefore we have nothing useful to do here.
        return OK;
    }

    // Determine whether we're seeking beyond the known area.
    bool shouldSeekBeyond =
            (seekTimeUs > mSeekSyncPoints->keyAt(mSeekSyncPoints->size() - 1));

    // Determine the sync point to seek.
    size_t index = 0;
    for (; index < mSeekSyncPoints->size(); ++index) {
        int64_t timeUs = mSeekSyncPoints->keyAt(index);
        if (timeUs > seekTimeUs) {
            break;
        }
    }

    switch (seekMode) {
        case MediaTrackHelper::ReadOptions::SEEK_NEXT_SYNC:
            if (index == mSeekSyncPoints->size()) {
                ALOGW("Next sync not found; starting from the latest sync.");
                --index;
            }
            break;
        case MediaTrackHelper::ReadOptions::SEEK_CLOSEST_SYNC:
        case MediaTrackHelper::ReadOptions::SEEK_CLOSEST:
            ALOGW("seekMode not supported: %d; falling back to PREVIOUS_SYNC",
                    seekMode);
            FALLTHROUGH_INTENDED;
        case MediaTrackHelper::ReadOptions::SEEK_PREVIOUS_SYNC:
            if (index == 0) {
                ALOGW("Previous sync not found; starting from the earliest "
                        "sync.");
            } else {
                --index;
            }
            break;
        default:
            return ERROR_UNSUPPORTED;
    }
    if (!shouldSeekBeyond || mOffset <= mSeekSyncPoints->valueAt(index)) {
        int64_t actualSeekTimeUs = mSeekSyncPoints->keyAt(index);
        mOffset = mSeekSyncPoints->valueAt(index);
        status_t err = queueDiscontinuityForSeek(actualSeekTimeUs);
        if (err != OK) {
            return err;
        }
    }

    if (shouldSeekBeyond) {
        status_t err = seekBeyond(seekTimeUs);
        if (err != OK) {
            return err;
        }
    }

    // Fast-forward to sync frame.
    for (size_t i = 0; i < mSourceImpls.size(); ++i) {
        const sp<AnotherPacketSource> &impl = mSourceImpls[i];
        status_t err;
        feedUntilBufferAvailable(impl);
        while (impl->hasBufferAvailable(&err)) {
            sp<AMessage> meta = impl->getMetaAfterLastDequeued(0);
            sp<ABuffer> buffer;
            if (meta == NULL) {
                return UNKNOWN_ERROR;
            }
            int32_t sync;
            if (meta->findInt32("isSync", &sync) && sync) {
                break;
            }
            err = impl->dequeueAccessUnit(&buffer);
            if (err != OK) {
                return err;
            }
            feedUntilBufferAvailable(impl);
        }
    }

    return OK;
}

status_t MPEG2TSExtractor::queueDiscontinuityForSeek(int64_t actualSeekTimeUs) {
    // Signal discontinuity
    sp<AMessage> extra(new AMessage);
    extra->setInt64(kATSParserKeyMediaTimeUs, actualSeekTimeUs);
    mParser->signalDiscontinuity(ATSParser::DISCONTINUITY_TIME, extra);

    // After discontinuity, impl should only have discontinuities
    // with the last being what we queued. Dequeue them all here.
    for (size_t i = 0; i < mSourceImpls.size(); ++i) {
        const sp<AnotherPacketSource> &impl = mSourceImpls.itemAt(i);
        sp<ABuffer> buffer;
        status_t err;
        while (impl->hasBufferAvailable(&err)) {
            if (err != OK) {
                return err;
            }
            err = impl->dequeueAccessUnit(&buffer);
            // If the source contains anything but discontinuity, that's
            // a programming mistake.
            CHECK(err == INFO_DISCONTINUITY);
        }
    }

    // Feed until we have a buffer for each source.
    for (size_t i = 0; i < mSourceImpls.size(); ++i) {
        const sp<AnotherPacketSource> &impl = mSourceImpls.itemAt(i);
        sp<ABuffer> buffer;
        status_t err = feedUntilBufferAvailable(impl);
        if (err != OK) {
            return err;
        }
    }

    return OK;
}

status_t MPEG2TSExtractor::seekBeyond(int64_t seekTimeUs) {
    // If we're seeking beyond where we know --- read until we reach there.
    size_t syncPointsSize = mSeekSyncPoints->size();

    while (seekTimeUs > mSeekSyncPoints->keyAt(
            mSeekSyncPoints->size() - 1)) {
        status_t err;
        if (syncPointsSize < mSeekSyncPoints->size()) {
            syncPointsSize = mSeekSyncPoints->size();
            int64_t syncTimeUs = mSeekSyncPoints->keyAt(syncPointsSize - 1);
            // Dequeue buffers before sync point in order to avoid too much
            // cache building up.
            sp<ABuffer> buffer;
            for (size_t i = 0; i < mSourceImpls.size(); ++i) {
                const sp<AnotherPacketSource> &impl = mSourceImpls[i];
                int64_t timeUs;
                while ((err = impl->nextBufferTime(&timeUs)) == OK) {
                    if (timeUs < syncTimeUs) {
                        impl->dequeueAccessUnit(&buffer);
                    } else {
                        break;
                    }
                }
                if (err != OK && err != -EWOULDBLOCK) {
                    return err;
                }
            }
        }
        if (feedMore() != OK) {
            return ERROR_END_OF_STREAM;
        }
    }

    return OK;
}

status_t MPEG2TSExtractor::feedUntilBufferAvailable(
        const sp<AnotherPacketSource> &impl) {
    status_t finalResult;
//  seek patch ++
    // add for mtk seek method
    while (!impl->hasBufferAvailable(&finalResult) || getSeeking()) {
//    while (!impl->hasBufferAvailable(&finalResult)) {
//  seek patch --
        if (finalResult != OK) {
            return finalResult;
        }

        status_t err = feedMore();
        if (err != OK) {
            impl->signalEOS(err);
        }
    }
    return OK;
}

bool MPEG2TSExtractor::findSyncWord(DataSourceHelper *source, off64_t StartOffset,
                    uint64_t size, size_t PacketSize, off64_t &NewOffset) {
    uint8_t packet[PacketSize];
    off64_t Offset = StartOffset;

    source->readAt(Offset, packet, PacketSize);
    ALOGD("findSyncWord mOffset= %lld  packet=0x%x,0x%x,0x%x,0x%x,0x%x",
        (long long)Offset, packet[0], packet[1], packet[2], packet[3], packet[4]);
    if ((PacketSize == kTSPacketSize) && packet[0] != 0x47) {
        uint8_t packetTempS[(PacketSize + mHeaderSkip)* 3];
        int32_t index = 0;
        for (; Offset < (off64_t)(StartOffset + size - 3 * (PacketSize+mHeaderSkip));) {
            Offset = Offset + PacketSize + mHeaderSkip;
            source->readAt(Offset+ mHeaderSkip, packetTempS, (PacketSize+mHeaderSkip) * 3);

            for (index = 0; index < (int32_t)(PacketSize + mHeaderSkip); index++) {
                if ((packetTempS[index] == 0x47) && (packetTempS[index+ PacketSize +mHeaderSkip] == 0x47)
                    && (packetTempS[index+ (PacketSize+mHeaderSkip) * 2] == 0x47)) {
                    break;
                }
            }

            if (index < (int32_t)(PacketSize+mHeaderSkip)) {
                NewOffset = Offset + index + 2 * (PacketSize+mHeaderSkip);
                ALOGD("findSyncWord mOffset= %lld  kFillPacketSize:%zu packet=0x%x,0x%x,0x%x,0x%x,0x%x",
                    (long long)NewOffset, PacketSize+mHeaderSkip, packetTempS[index], packetTempS[index+1],
                    packetTempS[index+2], packetTempS[index+3], packetTempS[index+4]);
                return true;
            }
        }
        ALOGE("findSyncWord: can not find sync word");
        return false;
    } else {
        return true;
    }
}

status_t MPEG2TSExtractor::parseMaxPTS() {
    mFindingMaxPTS = true;
    status_t err = mDataSource->getSize(&mFileSize);
    mFileDataSize = mFileSize;
    if (err != OK) {
        return err;
    }
    ALOGE("parseMaxPTS mFileSize:%lld", (long long)mFileSize);


    int32_t numPacketsParsed = 0;
    int64_t maxPTSStart = systemTime() / 1000;

    // set false, when parse the ts pakect, will not exec the  main function of onPayloadData
    // only parse the PAT, PMT,PES header, save parse time
    mParser->setDequeueState(false);

    // get first pts(pts in in PES packet)
    bool foundFirstPTS = false;
    off64_t minOffset = 0;
    while (feedMore() == OK) {
        if (mParser->getFirstPTSTimeUs() >= 0) {
            ALOGD("parseMaxPTS:firstPTSIsValid, mOffset %lld", (long long)mOffset);
            foundFirstPTS = true;
            break;
        }
        if (++numPacketsParsed > 30000) {
            break;
        }
    }
    if (!foundFirstPTS) {
        ALOGI("not found first PTS numPacketsParsed %d", numPacketsParsed);
        return UNKNOWN_ERROR;
    }

    mParser->signalDiscontinuity(ATSParser::DISCONTINUITY_TIME, NULL);  // clear
    minOffset = mOffset;
    // get duration
    off64_t maxOffset = mFileSize;
    if (maxOffset > (off64_t)(25000 * kTSPacketSize)) {
        maxOffset = (off64_t)(maxOffset - 25000 * kTSPacketSize);
    } else {
        maxOffset = 0;
    }
    if (maxOffset != 0 &&
        !findSyncWord(mDataSource, maxOffset, 200 * (off64_t)kTSPacketSize, kTSPacketSize, mOffset)) {
        while ((maxOffset - minOffset) > 25000* kTSPacketSize) {
            off64_t midOffset = (minOffset + maxOffset)/2;
            if (findSyncWord(mDataSource, midOffset, 200 * (off64_t)kTSPacketSize, kTSPacketSize, mOffset)) {
                minOffset = midOffset;
            } else {
                maxOffset = midOffset;
            }
            ALOGD("findsyncword, min %lld mid %lld max %lld", (long long)minOffset,
                (long long)midOffset, (long long)maxOffset);
        }
        mOffset = minOffset;
    } else {
        mOffset = maxOffset;
        minOffset = maxOffset;
    }
    if (findSyncWord(mDataSource, minOffset, 200 * (off64_t)kTSPacketSize, kTSPacketSize, mOffset)) {
        ALOGD("find moffset %lld", (long long)mOffset);
        mParser->signalDiscontinuity(ATSParser::DISCONTINUITY_TIME, NULL);
        int64_t duration = 0;
        while (feedMore() == OK) {
            int64_t maxPTSfeedmoreDuration = systemTime() / 1000 - maxPTSStart;
            if (maxPTSfeedmoreDuration > kMaxPTSTimeOutUs) {
                ALOGD("TimeOut find PTS, start time=%lld, maxPTSfeedmoreduration=%lld",
                      (long long)maxPTSStart, (long long)maxPTSfeedmoreDuration);
                return UNKNOWN_ERROR;
            }
            if (mParser->getMaxPTS()!= 0) {
                duration = mParser->getMaxPTS();
                ALOGV("update duration %lld", (long long)duration);
            }
        }
        if (mParser->getMaxPTS()== 0) {
            mFileDataSize = mOffset;
            ALOGV("feedmore err mOffset %lld, mFilesize %lld", (long long)mOffset, (long long)mFileSize);
        } else {
            ALOGV("feedmore err current getMaxPts %lld", (long long)mParser->getMaxPTS());
        }
        if (duration != 0) {
            mDurationUs = duration;
            mFileDataSize = mOffset;
            ALOGD("set duration %lld, filedataSize %lld", (long long)mDurationUs,
            (long long)mFileDataSize);
        }
        if (mDurationUs) {
            mFindingMaxPTS = false;
        }
    }
    // clear data queue
    mParser->signalDiscontinuity(ATSParser::DISCONTINUITY_TIME, NULL);
    mParser->setDequeueState(true);
    ALOGD("getMaxPTS->mDurationUs:%lld", (long long)mDurationUs);

    return OK;
}

uint64_t MPEG2TSExtractor::getDurationUs() {
        ALOGD("getduration %lld", (long long)mDurationUs);
        return mDurationUs;
}

bool MPEG2TSExtractor::IsLocalSource() {
    if (!(mDataSource->flags() & DataSourceBase::kIsCachingDataSource)) {
       return true;
    }
    return false;
}
// seek patch ++
// add for mtk seek method
    // add to avoid find sync word err when seek.
int32_t MPEG2TSExtractor::findSyncCode(const void *data, size_t size) {
    uint32_t i = 0;
    for (i = 0; i < size; i++) {
        if (((uint8_t *) data)[i] == 0x47u)
            return i;
    }
    return -1;
}

void MPEG2TSExtractor::seekTo(int64_t seekTimeUs,
    const MediaTrackHelper::ReadOptions::SeekMode& seekMode) {
    Mutex::Autolock autoLock(mLock);

    ALOGE("seekTo:mDurationMs =%lld,seekTimeMs= %lld, mOffset:%lld, seekMode %d",
          (long long)(mDurationUs / 1000), (long long)(seekTimeUs / 1000)
          , (long long)mOffset, seekMode);
    if (seekTimeUs == 0) {
        mOffset = 0;
        mSeeking = false;
        // clear MaxPTS
        mParser->setDequeueState(false);
        mParser->signalDiscontinuity(ATSParser::DISCONTINUITY_TIME, NULL);
        // clear buffer queue
        mParser->setDequeueState(true);
        mParser->signalDiscontinuity(ATSParser::DISCONTINUITY_TIME, NULL);
    } else if ((mDurationUs - seekTimeUs) < 10000) {  // seek to end
        mOffset = mFileDataSize;
        mSeeking = false;
        // set ATSParser MaxTimeUs to mDurationUs
        mParser->setDequeueState(false);
        sp<AMessage> maxTimeUsMsg = new AMessage;
        maxTimeUsMsg->setInt64("MaxtimeUs", mDurationUs);
        mParser->signalDiscontinuity(ATSParser::DISCONTINUITY_TIME, maxTimeUsMsg);
        // clear buffer queue
        mParser->setDequeueState(true);
        mParser->signalDiscontinuity(ATSParser::DISCONTINUITY_TIME, NULL);
    } else {
        mParser->signalDiscontinuity(ATSParser::DISCONTINUITY_TIME, NULL);
        mSeekingOffset = mOffset;
        mSeekTimeUs = seekTimeUs;
        mMinOffset = 0;
        mMaxOffset = mFileDataSize;
        mMaxcount = 0;
        mSeekMode = seekMode;
        mParser->setDequeueState(false);    // will start search mode, not read data mode
        mSeeking = true;
    }
    return;
}

bool MPEG2TSExtractor::getSeeking() {
    return mSeeking;
}

void MPEG2TSExtractor::setVideoState(bool state) {
    mVideoUnSupportedByDecoder = state;
    ALOGE("setVideoState  mVideoUnSupportedByDecoder=%d",
            mVideoUnSupportedByDecoder);
}

bool MPEG2TSExtractor::getVideoState(void) {
    ALOGE("getVideoState  mVideoUnSupportedByDecoder=%d",
          mVideoUnSupportedByDecoder);
    return mVideoUnSupportedByDecoder;
}

size_t MPEG2TSExtractor::countSource() {
    mSourceNum = 0;
    if (haveAudioSource && haveVideoSource) {
        mSourceNum = 2;
    } else if (haveAudioSource || haveVideoSource) {
        mSourceNum = 1;
    } else {
        mSourceNum = 0;
    }
    return mSourceNum;
}

// seek patch --

////////////////////////////////////////////////////////////////////////////////

bool SniffMPEG2TS(DataSourceHelper *source, float *confidence) {
    for (int i = 0; i < 5; ++i) {
        char header;
        if (source->readAt(kTSPacketSize * i, &header, 1) != 1
                || header != 0x47) {
            // not ts file, check if m2ts file
            for (int j = 0; j < 5; ++j) {
                char headers[5];
                if (source->readAt((kTSPacketSize + 4) * j, &headers, 5) != 5
                    || headers[4] != 0x47) {
                    // not m2ts file too, return
                    return false;
                }
            }
            ALOGV("this is m2ts file\n");
            break;
        }
    }

    *confidence = 0.11f;

    return true;
}

}  // namespace android
