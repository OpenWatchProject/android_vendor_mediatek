#ifndef ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_NORMAL_H
#define ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_NORMAL_H

#include "AudioALSACaptureDataProviderBase.h"

namespace android {

class AudioALSACaptureDataProviderNormal : public AudioALSACaptureDataProviderBase {
public:
    virtual ~AudioALSACaptureDataProviderNormal();

    static AudioALSACaptureDataProviderNormal *getInstance();

    /**
     * open/close pcm interface when 1st attach & the last detach
     */
    status_t open();
    status_t close();



protected:
    AudioALSACaptureDataProviderNormal();



private:
    /**
     * singleton pattern
     */
    static AudioALSACaptureDataProviderNormal *mAudioALSACaptureDataProviderNormal;
    struct mixer *mMixer;

    /**
     * pcm read thread
     */
    static void *readThread(void *arg);
    pthread_t hReadThread;

    uint32_t mCaptureDropSize;

    struct timespec mNewtime, mOldtime; //for calculate latency
    double timerec[3]; //0=>threadloop, 1=>kernel delay, 2=>process delay
    void  adjustSpike();

};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_NORMAL_H