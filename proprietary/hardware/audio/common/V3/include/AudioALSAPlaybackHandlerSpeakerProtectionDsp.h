#ifndef ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_SPEAKERPROTECTION_DSP_H
#define ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_SPEAKERPROTECTION_DSP_H

#include "AudioALSAPlaybackHandlerBase.h"


namespace android {
class AudioALSAPlaybackHandlerSpeakerProtectionDsp : public AudioALSAPlaybackHandlerBase {
public:
    AudioALSAPlaybackHandlerSpeakerProtectionDsp(const stream_attribute_t *stream_attribute_source);
    virtual ~AudioALSAPlaybackHandlerSpeakerProtectionDsp();

    status_t setParameters(const String8 &keyValuePairs);
    String8  getParameters(const String8 &keys);

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


    unsigned int GetSampleSize(unsigned int Format);
    unsigned int GetFrameSize(unsigned int channels, unsigned int Format);
    int initSmartPaConfig();

    /* scp spk pcm API */
    status_t openScpSpkPcmDriverWithFlag(const unsigned int device, unsigned int flag);
    status_t openScpSpkPcmDriver(const unsigned int dlDevice, const unsigned int ivDevice);
    status_t openScpSpkHwPcm();
    status_t closeScpSpkHwPcm();

private:
    struct timespec mNewtime, mOldtime;
    uint32_t ChooseTargetSampleRate(uint32_t SampleRate, audio_devices_t outputdevice);
    int setPcmDump(bool benable);
    double latencyTime[3];
    struct pcm_config mScpSpkHwOutConfig;
    struct pcm_config mScpSpkHwInConfig;
    struct pcm *mScpSpkDlHwPcm;
    struct pcm *mScpSpkIvHwPcm;

};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_SPEAKERPROTECTION_DSP_H
