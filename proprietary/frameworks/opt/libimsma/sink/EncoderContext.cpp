
//#define LOG_NDEBUG 0
#define LOG_TAG "[Sink][recorder]EncoderContext"
#include <utils/Log.h>
#include "EncoderContext.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cutils/properties.h>
#include <gui/Surface.h>
#if !(PLATFORM_VERSION == R)
#include <media/ICrypto.h>
#else
#include <mediadrm/ICrypto.h>
#endif
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/MediaBufferBase.h>
#include <media/stagefright/MediaCodec.h>
#include <media/MediaCodecBuffer.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaErrors.h>
#include <arpa/inet.h>

#include <OMX_Video.h>

#include "Recorder.h"
#include "comutils.h"

namespace android
{

EncoderContext::EncoderContext(
    const sp<AMessage> &notify,
    const sp<ALooper> &codecLooper,
    const sp<AMessage> &inputFormat,
    const sp<AMessage> &outputFormat)
    : mNotify(notify),
      mCodecLooper(codecLooper),
      mInputFormat(inputFormat),
      mOutputFormat(outputFormat),
      mIsVideo(false),
      mUsePCM(false),
      mIsFakeEncoder(false),
      mNeedToManuallyPrependSPSPPS(false),
      mDoMoreWorkPending(false)
#if ENABLE_SILENCE_DETECTION
      ,mFirstSilentFrameUs(-1ll)
      ,mInSilentMode(false)
#endif
      ,mPrevVideoBitrate(-1)
{
    AString mime_v;
    CHECK(mInputFormat->findString("mime", &mime_v));

    if(!strncasecmp("video/", mime_v.c_str(), 6)) {
        mIsVideo = true;
    }


    AString  mime_a;
    CHECK(mOutputFormat->findString("mime", &mime_a));

    mIsFakeEncoder = (!strcasecmp(mime_v.c_str(),mime_a.c_str())) ? true: false;

    if(!mIsFakeEncoder) {
        if(mIsVideo) {
            mOutputFormat->setInt32("store-metadata-in-buffers", true);
            mOutputFormat->setInt32(
                "color-format", OMX_COLOR_FormatAndroidOpaque);
        }

        status_t err = initEncoder();
        CHECK(err==OK);

        if(err != OK) {
            releaseEncoder();
        }

    } else {
        mEncoder =NULL;
        mOutputFormat = mInputFormat;
    }
}

static void ReleaseMediaBufferReference(MediaBufferBase *mBuf)
{
    if(mBuf != NULL) {
        VT_LOGV("releasing mBuf %p", mBuf);

        mBuf->release();
        mBuf = NULL;
    }
}

void EncoderContext::releaseEncoder()
{
    if(mEncoder == NULL) {
        return;
    }

    mEncoder->release();
    mEncoder.clear();

    while(!mInputBufferQueue.empty()) {
        sp<ABuffer> accessUnit = *mInputBufferQueue.begin();
        mInputBufferQueue.erase(mInputBufferQueue.begin());

        MediaBufferBase *mBuf = NULL;
        accessUnit->meta()->findPointer("mediaBuffer", (void**) &mBuf);
        accessUnit->meta()->setPointer("mediaBuffer", NULL);

        ReleaseMediaBufferReference(mBuf);
    }

    for(size_t i = 0; i < mEncoderInputBuffers.size(); ++i) {
        sp<MediaCodecBuffer> accessUnit = mEncoderInputBuffers.itemAt(i);
        MediaBufferBase *mBuf = NULL;
        accessUnit->meta()->findPointer("mediaBuffer", (void**) &mBuf);
        accessUnit->meta()->setPointer("mediaBuffer", NULL);

        ReleaseMediaBufferReference(mBuf);
    }

    mEncoderInputBuffers.clear();
    mEncoderOutputBuffers.clear();
}

EncoderContext::~EncoderContext()
{
    CHECK(mEncoder == NULL);
}

void EncoderContext::stopAsync()
{
    VT_LOGV("shutdown");
    (new AMessage(kWhatStop, this))->post();
}

size_t EncoderContext::getInputBufferCount() const
{
    return mEncoderInputBuffers.size();
}

sp<AMessage> EncoderContext::getOutputFormat() const
{
    return mOutputFormat;
}

bool EncoderContext::needToManuallyPrependSPSPPS() const
{
    return mNeedToManuallyPrependSPSPPS;
}

// static
/*int32_t EncoderContext::GetInt32Property(
    const char *propName, int32_t defaultValue)
{
    char val[PROPERTY_VALUE_MAX];

    if(property_get(propName, val, NULL)) {
        char *end;
        unsigned long x = strtoul(val, &end, 10);

        if(*end == '\0' && end > val && x > 0) {
            return x;
        }
    }

    return defaultValue;
}*/

status_t EncoderContext::initEncoder()
{
    AString inputMIME,outputMIME;
    CHECK(mInputFormat->findString("mime", &inputMIME));
    CHECK(mOutputFormat->findString("mime", &outputMIME));

    CHECK(mIsFakeEncoder == false);


    if(!strcasecmp(inputMIME.c_str(), MEDIA_MIMETYPE_AUDIO_RAW)
            && !strcasecmp(outputMIME.c_str(), MEDIA_MIMETYPE_AUDIO_RAW)) {
        mUsePCM=true;
    }

    if(!mIsFakeEncoder) {
        mEncoder = MediaCodec::CreateByType(
                       mCodecLooper, outputMIME.c_str(), true /* encoder */);

        if(mEncoder == NULL) {
            return ERROR_UNSUPPORTED;
        }
    }

    //config encoder parameter
    //fisrt check parameter
    VT_LOGI("output format is '%s'", mOutputFormat->debugString(0).c_str());

    if(mIsVideo) {
        int32_t tmp;
        CHECK(mOutputFormat->findInt32("bitrate",&tmp));
        CHECK(mOutputFormat->findInt32("width",&tmp));
        CHECK(mOutputFormat->findInt32("height",&tmp));
        CHECK(mOutputFormat->findInt32("frame-rate",&tmp));
        CHECK(mOutputFormat->findInt32("i-frame-interval",&tmp));
        //CHECK(mOutputFormat->findInt32("profile",&tmp));
        // CHECK(mOutputFormat->findInt32("level",&tmp));
        mOutputFormat->setInt32("bitrate-mode", OMX_Video_ControlRateConstant);
    } else {
        int32_t tmp;
        CHECK(mOutputFormat->findInt32("bitrate",&tmp));
        CHECK(mOutputFormat->findInt32("channel-count",&tmp));
        CHECK(mOutputFormat->findInt32("sample-rate",&tmp));
    }


    mNeedToManuallyPrependSPSPPS = false;

    status_t err = NO_INIT;

    if(mIsVideo) {
        sp<AMessage> tmp = mOutputFormat->dup();
        tmp->setInt32("prepend-sps-pps-to-idr-frames", 1);

        err = mEncoder->configure(
                  tmp,
                  NULL /* nativeWindow */,
                  NULL /* crypto */,
                  MediaCodec::CONFIGURE_FLAG_ENCODE);

        if(err == OK) {
            // Encoder supported prepending SPS/PPS, we don't need to emulate
            // it.
            mOutputFormat = tmp;
        } else {
            mNeedToManuallyPrependSPSPPS = true;

            VT_LOGI("We going to manually prepend SPS and PPS to IDR frames.");
        }
    }

    if(err != OK) {
        // We'll get here for audio or if we failed to configure the encoder
        // to automatically prepend SPS/PPS in the case of video.

        err = mEncoder->configure(
                  mOutputFormat,
                  NULL /* nativeWindow */,
                  NULL /* crypto */,
                  MediaCodec::CONFIGURE_FLAG_ENCODE);
    }

    if(err != OK) {
        return err;
    }

    err = mEncoder->start();

    if(err != OK) {
        return err;
    }

    err = mEncoder->getInputBuffers(&mEncoderInputBuffers);

    if(err != OK) {
        return err;
    }

    return mEncoder->getOutputBuffers(&mEncoderOutputBuffers);
}

void EncoderContext::notifyError(status_t err)
{
    sp<AMessage> notify = mNotify->dup();
    notify->setInt32("what", kWhatError);
    notify->setInt32("err", err);
    notify->post();
}

// static
bool EncoderContext::IsSilence(const sp<ABuffer> &accessUnit)
{
    const uint8_t *ptr = accessUnit->data();
    const uint8_t *end = ptr + accessUnit->size();

    while(ptr < end) {
        if(*ptr != 0) {
            return false;
        }

        ++ptr;
    }

    return true;
}


void EncoderContext::queueAccessUnit(const sp<ABuffer> &accessUnit)
{
    sp<AMessage> msg = new AMessage(kWhatAccessUnitIn, this);
    msg->setBuffer("accessUnit", accessUnit);
    msg->post();

}
void EncoderContext::onMessageReceived(const sp<AMessage> &msg)
{
    switch(msg->what()) {
    case kWhatAccessUnitIn: {
        VT_LOGV("mEncoder->queueInputBuffer mIsFakeEncoder %d  video %d   ",mIsFakeEncoder,mIsVideo);

        if(!mIsFakeEncoder && mEncoder == NULL) {
            VT_LOGE(" '%s'  encoder got au after encoder shutdown.",mIsVideo?"video":"audio");
            sp<ABuffer> accessUnit;
            CHECK(msg->findBuffer("accessUnit", &accessUnit));
            MediaBufferBase *mBuf = NULL;
            accessUnit->meta()->findPointer("mediaBuffer", (void**) &mBuf);
            accessUnit->meta()->setPointer("mediaBuffer", NULL);
            ReleaseMediaBufferReference(mBuf);
            break;
        } else if(mIsFakeEncoder) {
            sp<ABuffer> accessUnit;
            CHECK(msg->findBuffer("accessUnit", &accessUnit));
            sp<AMessage> notify = mNotify->dup();
            notify->setInt32("what", kWhatAccessUnitOut);
            notify->setBuffer("accessUnit", accessUnit);
            int64_t timeUs;
            CHECK(accessUnit->meta()->findInt64("timeUs", &timeUs));
            VT_LOGD("[%s] time %lld us (%.2f secs)",
                    mIsVideo ? "video" : "audio", (long long) timeUs, timeUs / 1E6);
            notify->post();

            break;
        } else {
            VT_LOGV("normal encoder, feed encoder video %d",mIsVideo);
            sp<ABuffer> accessUnit;
            CHECK(msg->findBuffer("accessUnit", &accessUnit));

#if ENABLE_SILENCE_DETECTION

            if(!mIsVideo) {
                if(IsSilence(accessUnit)) {
                    if(mInSilentMode) {
                        break;
                    }

                    int64_t nowUs = ALooper::GetNowUs();

                    if(mFirstSilentFrameUs < 0ll) {
                        mFirstSilentFrameUs = nowUs;
                    } else if(nowUs >= mFirstSilentFrameUs + 10000000ll) {
                        mInSilentMode = true;
                        VT_LOGD("audio in silent mode now.");
                        break;
                    }
                } else {
                    if(mInSilentMode) {
                        VT_LOGD("audio no longer in silent mode.");
                    }

                    mInSilentMode = false;
                    mFirstSilentFrameUs = -1ll;
                }
            }

#endif

            mInputBufferQueue.push_back(accessUnit);

            feedEncoderInputBuffers();

            scheduleDoMoreWork();
        }

        break;
    }

    case kWhatEncoderActivity: {

        mDoMoreWorkPending = false;

        if(mEncoder == NULL) {
            break;
        }

        status_t err = doMoreWork();

        if(err != OK) {
            notifyError(err);
        } else {
            scheduleDoMoreWork();
        }

        break;
    }

    case kWhatRequestIDRFrame: {
        if(mEncoder == NULL) {
            break;
        }

        if(mIsVideo) {
            VT_LOGD("requesting IDR frame");
            mEncoder->requestIDRFrame();
        }

        break;
    }

    case kWhatStop: {
        VT_LOGI("shutting down %s encoder", mIsVideo ? "video" : "audio");

        releaseEncoder();

        AString mime;
        CHECK(mInputFormat->findString("mime", &mime));
        VT_LOGI("encoder (%s) shut down.", mime.c_str());
        break;
    }

    default:
        TRESPASS();
    }
}

void EncoderContext::scheduleDoMoreWork()
{
    if(mIsFakeEncoder) {
        // There's no encoder involved in this case.
        return;
    }

    if(mDoMoreWorkPending) {
        return;
    }

    mDoMoreWorkPending = true;

#if 1

    if(mEncoderActivityNotify == NULL) {
        mEncoderActivityNotify = new AMessage(kWhatEncoderActivity, this);
    }

    mEncoder->requestActivityNotification(mEncoderActivityNotify->dup());
#else
    sp<AMessage> notify = new AMessage(kWhatEncoderActivity, this);
    notify->setInt64("whenUs", ALooper::GetNowUs());
    mEncoder->requestActivityNotification(notify);
#endif
}

status_t EncoderContext::feedRawAudioInputBuffers()
{
    // Split incoming PCM audio into buffers of 6 AUs of 80 audio frames each
    // and add a 4 byte header according to the wifi display specs.

    while(!mInputBufferQueue.empty()) {
        sp<ABuffer> buffer = *mInputBufferQueue.begin();
        mInputBufferQueue.erase(mInputBufferQueue.begin());

        int16_t *ptr = (int16_t *) buffer->data();
        int16_t *stop = (int16_t *)(buffer->data() + buffer->size());

        while(ptr < stop) {
            *ptr = htons(*ptr);
            ++ptr;
        }

        static const size_t kFrameSize = 2 * sizeof(int16_t);    // stereo
        static const size_t kFramesPerAU = 80;
        static const size_t kNumAUsPerPESPacket = 6;

        if(mPartialAudioAU != NULL) {
            size_t bytesMissingForFullAU =
                kNumAUsPerPESPacket * kFramesPerAU * kFrameSize
                - mPartialAudioAU->size() + 4;

            size_t copy = buffer->size();

            if(copy > bytesMissingForFullAU) {
                copy = bytesMissingForFullAU;
            }

            memcpy(mPartialAudioAU->data() + mPartialAudioAU->size(),
                   buffer->data(),
                   copy);

            mPartialAudioAU->setRange(0, mPartialAudioAU->size() + copy);

            buffer->setRange(buffer->offset() + copy, buffer->size() - copy);

            int64_t timeUs;
            CHECK(buffer->meta()->findInt64("timeUs", &timeUs));

            int64_t copyUs = (int64_t)((copy / kFrameSize) * 1E6 / 48000.0);
            timeUs += copyUs;
            buffer->meta()->setInt64("timeUs", timeUs);

            if(bytesMissingForFullAU == copy) {
                sp<AMessage> notify = mNotify->dup();
                notify->setInt32("what", kWhatAccessUnitOut);
                notify->setBuffer("accessUnit", mPartialAudioAU);
                notify->post();

                mPartialAudioAU.clear();
            }
        }

        while(buffer->size() > 0) {
            sp<ABuffer> partialAudioAU =
                new ABuffer(
                4
                + kNumAUsPerPESPacket * kFrameSize * kFramesPerAU);

            uint8_t *ptr = partialAudioAU->data();
            ptr[0] = 0xa0;  // 10100000b
            ptr[1] = kNumAUsPerPESPacket;
            ptr[2] = 0;  // reserved, audio _emphasis_flag = 0

            static const unsigned kQuantizationWordLength = 0;  // 16-bit
            static const unsigned kAudioSamplingFrequency = 2;  // 48Khz
            static const unsigned kNumberOfAudioChannels = 1;  // stereo

            ptr[3] = (kQuantizationWordLength << 6)
                     | (kAudioSamplingFrequency << 3)
                     | kNumberOfAudioChannels;

            size_t copy = buffer->size();

            if(copy > partialAudioAU->size() - 4) {
                copy = partialAudioAU->size() - 4;
            }

            memcpy(&ptr[4], buffer->data(), copy);

            partialAudioAU->setRange(0, 4 + copy);
            buffer->setRange(buffer->offset() + copy, buffer->size() - copy);

            int64_t timeUs;
            CHECK(buffer->meta()->findInt64("timeUs", &timeUs));

            partialAudioAU->meta()->setInt64("timeUs", timeUs);

            int64_t copyUs = (int64_t)((copy / kFrameSize) * 1E6 / 48000.0);
            timeUs += copyUs;
            buffer->meta()->setInt64("timeUs", timeUs);

            if(copy == partialAudioAU->capacity() - 4) {
                sp<AMessage> notify = mNotify->dup();
                notify->setInt32("what", kWhatAccessUnitOut);
                notify->setBuffer("accessUnit", partialAudioAU);
                notify->post();

                partialAudioAU.clear();
                continue;
            }

            mPartialAudioAU = partialAudioAU;
        }
    }

    return OK;
}

status_t EncoderContext::feedEncoderInputBuffers()
{
    VT_LOGV("feedEncoderInputBuffers +");

    if(mUsePCM) {
        return feedRawAudioInputBuffers();
    }

    VT_LOGV("feedEncoderInputBuffers + mInputBufferQueue size %zu mAvailEncoderInputIndices size %zu",
            mInputBufferQueue.size(),mAvailEncoderInputIndices.size());

    while(!mInputBufferQueue.empty()
            && !mAvailEncoderInputIndices.empty()) {
        sp<ABuffer> buffer = *mInputBufferQueue.begin();
        mInputBufferQueue.erase(mInputBufferQueue.begin());

        size_t bufferIndex = *mAvailEncoderInputIndices.begin();
        mAvailEncoderInputIndices.erase(mAvailEncoderInputIndices.begin());

        int64_t timeUs = 0ll;
        uint32_t flags = 0;

        if(buffer != NULL) {
            CHECK(buffer->meta()->findInt64("timeUs", &timeUs));

            memcpy(mEncoderInputBuffers.itemAt(bufferIndex)->data(),
                   buffer->data(),
                   buffer->size());

            void *mediaBuffer;

            if(buffer->meta()->findPointer("mediaBuffer", (void**) &mediaBuffer)
                    && mediaBuffer != NULL) {
                mEncoderInputBuffers.itemAt(bufferIndex)->meta()
                ->setPointer("mediaBuffer", mediaBuffer);

                buffer->meta()->setPointer("mediaBuffer", NULL);
            }
        } else {
            flags = MediaCodec::BUFFER_FLAG_EOS;
        }

        VT_LOGV("queueInputBuffer timeUs %lld", (long long) timeUs);
        status_t err = mEncoder->queueInputBuffer(
                           bufferIndex, 0, (buffer == NULL) ? 0 : buffer->size(),
                           timeUs, flags);

        if(err != OK) {
            return err;
        }
    }

    return OK;
}

status_t EncoderContext::doMoreWork()
{
    status_t err;

    for(;;) {
        size_t bufferIndex;
        err = mEncoder->dequeueInputBuffer(&bufferIndex);

        if(err != OK) {
            VT_LOGV("fail to dequeueInputBuffer err %d",err);
            break;
        }

        mAvailEncoderInputIndices.push_back(bufferIndex);
    }

    feedEncoderInputBuffers();

    for(;;) {
        size_t bufferIndex;
        size_t offset;
        size_t size;
        int64_t timeUs;
        uint32_t flags;
        err = mEncoder->dequeueOutputBuffer(
                  &bufferIndex, &offset, &size, &timeUs, &flags);

        if(err != OK) {
            if(err == -EAGAIN) {
                err = OK;
            }

            break;
        }

        if(flags & MediaCodec::BUFFER_FLAG_EOS) {
            sp<AMessage> notify = mNotify->dup();
            notify->setInt32("what", kWhatEOS);
            notify->post();
        } else {
            sp<ABuffer> buffer = new ABuffer(size);
            buffer->meta()->setInt64("timeUs", timeUs);

            VT_LOGD("[%s] time %lld us (%.2f secs)",
                    mIsVideo ? "video" : "audio", (long long) timeUs, timeUs / 1E6);

            memcpy(buffer->data(),
                   mEncoderOutputBuffers.itemAt(bufferIndex)->base() + offset,
                   size);

            sp<AMessage> notify = mNotify->dup();

            if(flags & MediaCodec::BUFFER_FLAG_CODECCONFIG) {
                mOutputFormat->setBuffer("csd-0", buffer);
                buffer->meta()->setInt32("csd-0", 1);
            } else if(flags & MediaCodec::BUFFER_FLAG_SYNCFRAME) {
                buffer->meta()->setInt32("is-sync-frame", 1);
            }

            notify->setInt32("what", kWhatAccessUnitOut);
            notify->setBuffer("accessUnit", buffer);

            notify->post();
        }

        mEncoder->releaseOutputBuffer(bufferIndex);

        if(flags & MediaCodec::BUFFER_FLAG_EOS) {
            break;
        }
    }

    return err;
}

void EncoderContext::requestIDRFrame()
{
    (new AMessage(kWhatRequestIDRFrame, this))->post();
}


int32_t EncoderContext::getVideoBitrate() const
{
    return mPrevVideoBitrate;
}

void EncoderContext::setVideoBitrate(int32_t bitRate)
{
    if(mIsVideo && mEncoder != NULL && bitRate != mPrevVideoBitrate) {
        sp<AMessage> params = new AMessage;
        params->setInt32("videoBitrate", bitRate);

        mEncoder->setParameters(params);

        mPrevVideoBitrate = bitRate;
    }
}

}  // namespace android
