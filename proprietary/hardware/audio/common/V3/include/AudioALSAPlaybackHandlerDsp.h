#ifndef ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_DSP_H
#define ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_DSP_H

#include "AudioALSAPlaybackHandlerBase.h"
#include "AudioDspType.h"
struct aurisys_dsp_config_t;


namespace android {

class AudioDspStreamManager;

class AudioALSAPlaybackHandlerDsp : public AudioALSAPlaybackHandlerBase {

public:
    AudioALSAPlaybackHandlerDsp(const stream_attribute_t *stream_attribute_source);
    virtual ~AudioALSAPlaybackHandlerDsp();

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
    virtual int preWriteOperation(const void *buffer __unused, size_t bytes __unused) { return 0; }
    virtual int updateAudioMode(audio_mode_t mode __unused) { return 0; }
    virtual status_t setFilterMng(AudioMTKFilterManager *pFilterMng);

    /**
     * low latency
     */
    virtual status_t setScreenState(bool mode, size_t buffer_size, size_t reduceInterruptSize, bool bforce = false);

    /* dsp pcm API*/
    virtual String8 getPlaybackTurnOnSequence(unsigned int turnOnSeqType,
                                              const char *playbackSeq);
    status_t openDspHwPcm();
    status_t openDspPcmDriverWithFlag(const unsigned int device, unsigned int flag);
    status_t opeDspPcmDriver(const unsigned int device);
    status_t closeDspPcmDriver();

private:
    struct timespec mNewtime, mOldtime;
    bool deviceSupportHifi(audio_devices_t outputdevice);
    uint32_t chooseTargetSampleRate(uint32_t SampleRate, audio_devices_t outputdevice);
    uint32_t getLowJitterModeSampleRate(void);
    int setAfeDspShareMem(unsigned int flag, bool condition);
    int setDspRuntimeEn(bool condition);
    struct pcm_config mDsphwConfig;
    AudioDspStreamManager *mDspStreamManager;

    struct pcm *mDspHwPcm;
    double latencyTime[3];
    bool mForceMute;
    int mCurMuteBytes;
    int mStartMuteBytes;
    bool mSupportNLE;


    uint8_t mTaskScene;
    struct aurisys_dsp_config_t *mAurisysDspConfig;
};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_DSP_H
