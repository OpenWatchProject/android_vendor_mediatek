#ifndef ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_NORMAL_H
#define ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_NORMAL_H

#include "AudioALSAPlaybackHandlerBase.h"


namespace android {

class AudioALSAPlaybackHandlerNormal : public AudioALSAPlaybackHandlerBase {
public:
    AudioALSAPlaybackHandlerNormal(const stream_attribute_t *stream_attribute_source);
    virtual ~AudioALSAPlaybackHandlerNormal();


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

    virtual status_t setFilterMng(AudioMTKFilterManager *pFilterMng);


    /**
     * low latency
     */
    virtual status_t setScreenState(bool mode, size_t buffer_size, size_t reduceInterruptSize, bool bforce = false);

private:
    struct timespec mNewtime, mOldtime;
    bool SetLowJitterMode(bool bEnable, uint32_t SampleRate);
    uint32_t ChooseTargetSampleRate(uint32_t SampleRate, audio_devices_t outputdevice);
    bool DeviceSupportHifi(audio_devices_t outputdevice);
    uint32_t GetLowJitterModeSampleRate(void);
#if defined(MTK_HIFIAUDIO_SUPPORT)
    uint32_t UpdateKernelBufferSize(audio_devices_t outputdevice);
#endif
    struct pcm_config mHpImpedanceConfig;
    struct pcm *mHpImpeDancePcm;
    double latencyTime[3];

    //#ifdef MTK_AUDIO_SW_DRE
    bool mForceMute;
    int mCurMuteBytes;
    int mStartMuteBytes;
    char *mAllZeroBlock;
    //#endif

    bool mSupportNLE;
};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_NORMAL_H
