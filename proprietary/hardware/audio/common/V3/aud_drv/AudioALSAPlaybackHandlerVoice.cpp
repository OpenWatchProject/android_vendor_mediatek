#include "AudioALSAPlaybackHandlerVoice.h"

#include "SpeechDriverFactory.h"
#include <SpeechPcmMixerBGSPlayer.h>

#if defined(MTK_COMBO_MODEM_SUPPORT)
#if defined(MTK_SPEECH_VOICE_MIXER_SUPPORT)
#include <SpeechPcmMixerVoipRx.h>
#endif
#include <SpeechPcmMixerTelephonyTx.h>
#endif

#include <SpeechUtility.h>


#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioALSAPlaybackHandlerVoice"

namespace android {

const char *PROPERTY_KEY_PCM_MIXER_NO_SLEEP = "persist.vendor.audiohal.speech.pcm_mixer.no_sleep";



AudioALSAPlaybackHandlerVoice::AudioALSAPlaybackHandlerVoice(const stream_attribute_t *stream_attribute_source) :
    AudioALSAPlaybackHandlerBase(stream_attribute_source) {
    mPcmMixer = NULL;
    mSpeechDriver = NULL;
    mPlaybackHandlerType = PLAYBACK_HANDLER_VOICE;
    mPcmMixerBuffer = NULL;

    memset((void *)&mOpenTime, 0, sizeof(mOpenTime));
    memset((void *)&mCurTime, 0, sizeof(mCurTime));
    mWriteCnt = 0;

    memset((void *)&mNewtimeLatency, 0, sizeof(mNewtimeLatency));
    memset((void *)&mOldtimeLatency, 0, sizeof(mOldtimeLatency));
    memset((void *)&mLatencyTimeMs, 0, sizeof(mLatencyTimeMs));

    mLatencyUs = 0;

    mBypassPcmMixerSleep = 0;
}


AudioALSAPlaybackHandlerVoice::~AudioALSAPlaybackHandlerVoice() {
}

status_t AudioALSAPlaybackHandlerVoice::open() {
    // HW attribute config // TODO(Harvey): query this
    mStreamAttributeTarget.audio_format = AUDIO_FORMAT_PCM_16_BIT;
    mStreamAttributeTarget.audio_channel_mask = AUDIO_CHANNEL_OUT_MONO;
    mStreamAttributeTarget.num_channels = popcount(mStreamAttributeTarget.audio_channel_mask);
    mStreamAttributeTarget.sample_rate = PCM_MIXER_TARGET_SAMPLE_RATE; // same as source stream
    mStreamAttributeTarget.buffer_size = PCM_MIXER_PLAY_BUFFER_LEN;

    mLatencyUs = getBufferLatencyUs(mStreamAttributeSource,
                                    mStreamAttributeSource->buffer_size);

    ALOGD("%s(), audio_mode: %d, audio_format: %d => %d, sample_rate: %u => %u, ch: %u => %u"
          ", buffer_size: (write)%u, (pcm_mixer)%u, flag: 0x%x, mLatencyUs: %u",
          __FUNCTION__, mStreamAttributeSource->audio_mode,
          mStreamAttributeSource->audio_format,
          mStreamAttributeTarget.audio_format,
          mStreamAttributeSource->sample_rate,
          mStreamAttributeTarget.sample_rate,
          mStreamAttributeSource->num_channels,
          mStreamAttributeTarget.num_channels,
          mStreamAttributeSource->buffer_size,
          mStreamAttributeTarget.buffer_size,
          mStreamAttributeSource->mAudioOutputFlags,
          (uint32_t)(mLatencyUs & 0xFFFFFFFF));

    // debug pcm dump
    OpenPCMDump(LOG_TAG);

#if defined(MTK_COMBO_MODEM_SUPPORT)

#if defined(MTK_SPEECH_VOICE_MIXER_SUPPORT)
    if (mStreamAttributeSource->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_VOIP_RX) {
        mPcmMixer = SpeechPcmMixerVoipRx::GetInstance();
    } else
#endif
    {
        if (mStreamAttributeSource->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_INCALL_MUSIC) {
            mPcmMixer = SpeechPcmMixerTelephonyTx::GetInstance();
        } else {
            if (get_uint32_from_property("persist.vendor.audiohal.force_telephonytx") == 1) {
                mPcmMixer = SpeechPcmMixerTelephonyTx::GetInstance();
            } else {
                mPcmMixer = SpeechPcmMixerBGSPlayer::GetInstance();
            }
        }
    }
#else
    if (mStreamAttributeSource->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_INCALL_MUSIC) {
        ALOGW("%s() This platform does not support output flag:0x%x",
              __FUNCTION__, mStreamAttributeSource->mAudioOutputFlags);
        WARNING("AudioOutputFlag not support!!");
        return -EFAULT;
    } else {
        mPcmMixer = SpeechPcmMixerBGSPlayer::GetInstance();
    }
#endif
    // bit conversion
    initBitConverter();

    // open background sound
    if (mStreamAttributeTarget.num_channels > 2) {
        mPcmMixerBuffer = mPcmMixer->CreatePcmMixerBuffer(
                              mStreamAttributeSource->sample_rate,
                              2,
                              mStreamAttributeTarget.audio_format,
                              mPcmMixer->getPcmMixerType());

    } else {
        mPcmMixerBuffer = mPcmMixer->CreatePcmMixerBuffer(
                              mStreamAttributeSource->sample_rate,
                              mStreamAttributeSource->num_channels,
                              mStreamAttributeTarget.audio_format,
                              mPcmMixer->getPcmMixerType());
    }

    mSpeechDriver = SpeechDriverFactory::GetInstance()->GetSpeechDriver();
    mPcmMixer->Open(mSpeechDriver);

    mBypassPcmMixerSleep = get_uint32_from_property(PROPERTY_KEY_PCM_MIXER_NO_SLEEP);

    clock_gettime(CLOCK_MONOTONIC, &mOpenTime);
    mWriteCnt = 0;

    clock_gettime(CLOCK_MONOTONIC, &mNewtimeLatency);
    mOldtimeLatency = mNewtimeLatency;

    return NO_ERROR;
}


status_t AudioALSAPlaybackHandlerVoice::close() {
    ALOGD("%s(), flag: 0x%x", __FUNCTION__, mStreamAttributeSource->mAudioOutputFlags);

    if (mPcmMixer != NULL) {
        // close pcm mixer
        mPcmMixer->Close();
        mPcmMixer->DestroyPcmMixerBuffer(mPcmMixerBuffer);
    }

    // bit conversion
    deinitBitConverter();


    // debug pcm dump
    ClosePCMDump();

    return NO_ERROR;
}


status_t AudioALSAPlaybackHandlerVoice::routing(const audio_devices_t output_devices __unused) {
    return INVALID_OPERATION;
}

uint32_t AudioALSAPlaybackHandlerVoice::ChooseTargetSampleRate(uint32_t SampleRate) {
    ALOGD("ChooseTargetSampleRate SampleRate = %d ", SampleRate);
    uint32_t TargetSampleRate = 44100;
    if ((SampleRate % 8000) == 0) { // 8K base
        TargetSampleRate = 48000;
    }
    return TargetSampleRate;
}

ssize_t AudioALSAPlaybackHandlerVoice::write(const void *buffer, size_t bytes) {
    ALOGV("%s()", __FUNCTION__);

    uint64_t spendTimeUs = 0;
    uint64_t writeTimeUs = 0;
    uint64_t sleepUs = 0;

    mWriteCnt++;

    if (mSpeechDriver == NULL) {
        ALOGW("%s(), mSpeechDriver == NULL!!", __FUNCTION__);
        return bytes;
    }

    if (mPcmMixer == NULL) {
        ALOGW("%s(), mPcmMixer == NULL!!", __FUNCTION__);
        return bytes;
    }

    clock_gettime(CLOCK_MONOTONIC, &mNewtimeLatency);
    mLatencyTimeMs[0] = get_time_diff_ms(&mOldtimeLatency, &mNewtimeLatency);
    mOldtimeLatency = mNewtimeLatency;

    if (mSpeechDriver->CheckModemIsReady() == false) {
        uint32_t sleep_ms = getBufferLatencyMs(mStreamAttributeSource, bytes);
        if (sleep_ms != 0) {
            ALOGW("%s(), modem not ready, sleep %u ms", __FUNCTION__, sleep_ms);
            usleep(sleep_ms * 1000);
        }
        return bytes;
    }

    void *newbuffer[96 * 1024] = {0};
    unsigned char *aaa;
    unsigned char *bbb;
    size_t i = 0;
    size_t j = 0;
    int retval = 0;
    // const -> to non const
    void *pBuffer = const_cast<void *>(buffer);
    ASSERT(pBuffer != NULL);

    aaa = (unsigned char *)newbuffer;
    bbb = (unsigned char *)buffer;


    if (mStreamAttributeSource->audio_format == AUDIO_FORMAT_PCM_16_BIT) {
        if (mStreamAttributeTarget.num_channels == 8) {
            for (i = 0 ; j < bytes; i += 4) {
                memcpy(aaa + i, bbb + j, 4);
                j += 16;
            }
            bytes = (bytes >> 2);
        } else if (mStreamAttributeTarget.num_channels == 6) {
            for (i = 0 ; j < bytes; i += 4) {
                memcpy(aaa + i, bbb + j, 4);
                j += 12;
            }
            bytes = (bytes / 3);
        } else {
            memcpy(aaa, bbb, bytes);
        }
    } else {
        if (mStreamAttributeTarget.num_channels == 8) {
            for (i = 0 ; j < bytes; i += 8) {
                memcpy(aaa + i, bbb + j, 8);
                j += 32;
            }
            bytes = (bytes >> 2);
        } else if (mStreamAttributeTarget.num_channels == 6) {
            for (i = 0 ; j < bytes; i += 8) {
                memcpy(aaa + i, bbb + j, 8);
                j += 24;
            }
            bytes = (bytes / 3);
        } else {
            memcpy(aaa, bbb, bytes);
        }

    }

    // bit conversion
    void *pBufferAfterBitConvertion = NULL;
    uint32_t bytesAfterBitConvertion = 0;
    doBitConversion(newbuffer, bytes, &pBufferAfterBitConvertion, &bytesAfterBitConvertion);


    // write data to background sound
    WritePcmDumpData(pBufferAfterBitConvertion, bytesAfterBitConvertion);

    uint32_t u4WrittenBytes = mPcmMixer->Write(mPcmMixerBuffer, pBufferAfterBitConvertion, bytesAfterBitConvertion);
    if (u4WrittenBytes != bytesAfterBitConvertion) { // TODO: 16/32
        ALOGE("%s(), PcmMixer::GetInstance()->Write() error, u4WrittenBytes(%u) != bytesAfterBitConvertion(%u)", __FUNCTION__, u4WrittenBytes, bytesAfterBitConvertion);
    }
    clock_gettime(CLOCK_MONOTONIC, &mNewtimeLatency);
    mLatencyTimeMs[1] = get_time_diff_ms(&mOldtimeLatency, &mNewtimeLatency);
    mOldtimeLatency = mNewtimeLatency;

    /* HAL sleep latency time to consume data smoothly */
    if (mBypassPcmMixerSleep == false) {
        clock_gettime(CLOCK_MONOTONIC, &mCurTime);
        spendTimeUs = get_time_diff_ns(&mOpenTime, &mCurTime) / 1000;
        writeTimeUs = mWriteCnt * mLatencyUs;
        if (spendTimeUs < writeTimeUs) {
            sleepUs = writeTimeUs - spendTimeUs;
            if (mPcmMixerBuffer->isBufferEnough()) {
                usleep(sleepUs);
            } else {
                if (sleepUs > 1000) {
                    sleepUs -= 1000;
                    usleep(sleepUs);
                } else {
                    sleepUs = 0;
                }
            }
        } else if (spendTimeUs > (writeTimeUs + MODEM_FRAME_MS * 1000)) {
            if (getPcmMixerLogEnableByLevel(PCM_MIXER_LOG_LEVEL_PLAYBACK_HANDLER)) {
                ALOGW("%s(), spendTimeUs %u, writeTimeUs %u", __FUNCTION__,
                      (uint32_t)(spendTimeUs & 0xFFFFFFFF),
                      (uint32_t)(writeTimeUs & 0xFFFFFFFF));
            }
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &mNewtimeLatency);
    mLatencyTimeMs[2] = get_time_diff_ms(&mOldtimeLatency, &mNewtimeLatency);
    mOldtimeLatency = mNewtimeLatency;

    uint64_t logThresholdMs = (mLatencyUs) / 1000;
    if (logThresholdMs < MODEM_FRAME_MS) {
        logThresholdMs = MODEM_FRAME_MS;
    }

    if (mLatencyTimeMs[0] > logThresholdMs ||
        mLatencyTimeMs[1] > logThresholdMs ||
        mLatencyTimeMs[2] > logThresholdMs) {
        ALOGW("latency_in_ms, %3u, %3u, %3u, u4WrittenBytes: %u, mLatencyUs: %u, spendTimeUs: %u, writeTimeUs: %u, sleepUs: %u",
              (uint32_t)(mLatencyTimeMs[0] & 0xFFFFFFFF),
              (uint32_t)(mLatencyTimeMs[1] & 0xFFFFFFFF),
              (uint32_t)(mLatencyTimeMs[2] & 0xFFFFFFFF),
              u4WrittenBytes,
              (uint32_t)(mLatencyUs & 0xFFFFFFFF),
              (uint32_t)(spendTimeUs & 0xFFFFFFFF),
              (uint32_t)(writeTimeUs & 0xFFFFFFFF),
              (uint32_t)(sleepUs & 0xFFFFFFFF));
    } else if (getPcmMixerLogEnableByLevel(PCM_MIXER_LOG_LEVEL_PLAYBACK_HANDLER)) {
        ALOGD("latency_in_ms, %3u, %3u, %3u, u4WrittenBytes: %u, mLatencyUs: %u, spendTimeUs: %u, writeTimeUs: %u, sleepUs: %u",
              (uint32_t)(mLatencyTimeMs[0] & 0xFFFFFFFF),
              (uint32_t)(mLatencyTimeMs[1] & 0xFFFFFFFF),
              (uint32_t)(mLatencyTimeMs[2] & 0xFFFFFFFF),
              u4WrittenBytes,
              (uint32_t)(mLatencyUs & 0xFFFFFFFF),
              (uint32_t)(spendTimeUs & 0xFFFFFFFF),
              (uint32_t)(writeTimeUs & 0xFFFFFFFF),
              (uint32_t)(sleepUs & 0xFFFFFFFF));
    }

    return bytes;
}

#if defined(MTK_SPEECH_VOICE_MIXER_SUPPORT)
status_t AudioALSAPlaybackHandlerVoice::setVolume(uint32_t vol __unused) {
    if (mStreamAttributeSource->mAudioOutputFlags & AUDIO_OUTPUT_FLAG_VOIP_RX) {
        return NO_ERROR;
    } else {
        return INVALID_OPERATION;
    }
}
#endif

} // end of namespace android
