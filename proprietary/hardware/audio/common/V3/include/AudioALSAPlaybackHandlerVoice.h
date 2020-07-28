#ifndef ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_VOICE_H
#define ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_VOICE_H

#include "AudioALSAPlaybackHandlerBase.h"

namespace android {

class SpeechPcmMixerBase;
class SpeechPcmMixerBaseBuffer;
class SpeechDriverInterface;


class AudioALSAPlaybackHandlerVoice : public AudioALSAPlaybackHandlerBase {
public:
    AudioALSAPlaybackHandlerVoice(const stream_attribute_t *stream_attribute_source);
    virtual ~AudioALSAPlaybackHandlerVoice();


    /**
     * open/close audio hardware
     */
    virtual status_t open();
    virtual status_t close();
    virtual status_t routing(const audio_devices_t output_devices);

    /**
     * write data to audio hardware
     */
    virtual ssize_t  write(const void *buffer, size_t bytes);
#if defined(MTK_SPEECH_VOICE_MIXER_SUPPORT)
    virtual status_t setVolume(uint32_t vol __unused);
#endif
    uint32_t ChooseTargetSampleRate(uint32_t SampleRate);


    /**
     * get hardware buffer info (framecount)
     */
    virtual status_t getHardwareBufferInfo(time_info_struct_t *HWBuffer_Time_Info __unused) { return INVALID_OPERATION; }



private:
    SpeechDriverInterface *mSpeechDriver;
    SpeechPcmMixerBase *mPcmMixer;
    SpeechPcmMixerBaseBuffer *mPcmMixerBuffer;

    struct timespec mOpenTime, mCurTime;
    uint64_t mWriteCnt;

    uint64_t mLatencyUs;

    struct timespec mNewtimeLatency, mOldtimeLatency;
    uint64_t mLatencyTimeMs[3];

    bool mBypassPcmMixerSleep;
};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_VOICE_H
