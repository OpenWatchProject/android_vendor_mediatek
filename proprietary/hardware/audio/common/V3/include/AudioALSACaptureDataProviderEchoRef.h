#ifndef ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_ECHOREF_H
#define ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_ECHOREF_H

#include "AudioALSACaptureDataProviderBase.h"

namespace android {

class AudioALSACaptureDataProviderEchoRef : public AudioALSACaptureDataProviderBase {
public:
    virtual ~AudioALSACaptureDataProviderEchoRef();

    static AudioALSACaptureDataProviderEchoRef *getInstance();

    /**
     * open/close pcm interface when 1st attach & the last detach
     */
    status_t open();
    status_t close();


protected:
    AudioALSACaptureDataProviderEchoRef();



private:
    /**
     * singleton pattern
     */
    static AudioALSACaptureDataProviderEchoRef *mAudioALSACaptureDataProviderEchoRef;
    struct mixer *mMixer;

    /**
     * pcm read thread
     */
    static void *readThread(void *arg);
    pthread_t hReadThread;

    struct timespec mNewtime, mOldtime; //for calculate latency
    double timerec[3]; //0=>threadloop, 1=>kernel delay, 2=>process delay
};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_ECHOREF_H