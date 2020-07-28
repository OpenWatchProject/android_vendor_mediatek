#ifndef ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_DSP_H
#define ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_DSP_H

#include "AudioALSACaptureDataProviderBase.h"
#include "AudioDspType.h"

struct aurisys_dsp_config_t;
struct aurisys_lib_manager_t;
class AudioVolumeInterface;

namespace android {

class AudioALSACaptureDataProviderDsp : public AudioALSACaptureDataProviderBase {
public:
    virtual ~AudioALSACaptureDataProviderDsp();

    static AudioALSACaptureDataProviderDsp *getInstance();

    /**
     * open/close pcm interface when 1st attach & the last detach
     */
    status_t open();
    status_t close();



protected:
    AudioALSACaptureDataProviderDsp();


private:
    /**
     * singleton pattern
     */
    static AudioALSACaptureDataProviderDsp *mAudioALSACaptureDataProviderDsp;
    struct mixer *mMixer;

    int setAfeDspShareMem(bool condition);
    int setDspRuntimeEn(bool condition);
    status_t openDspHwPcm();
    status_t closeDspHwPcm();
    status_t openDspHwRefPcm();
    status_t closeDspHwRefPcm();
    struct pcm *mDspHwPcm;
    struct pcm *mDspHwRefPcm;
    /**
     * pcm read thread
     */
    static void *readThread(void *arg);
    pthread_t hReadThread;

    uint32_t mCaptureDropSize;

    struct timespec mNewtime, mOldtime; //for calculate latency
    double timerec[3]; //0=>threadloop, 1=>kernel delay, 2=>process delay
    void  adjustSpike();

    struct pcm_config mDsphwConfig;
    struct pcm_config mDsphwRefConfig;

    struct aurisys_lib_manager_t *mAurisysLibManager;
    struct aurisys_dsp_config_t *mAurisysDspConfig;
    String8 mDspRefTurnOnSequence;

    /* Volume Controller */
    AudioVolumeInterface *mAudioALSAVolumeController;
};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_DSP_H
