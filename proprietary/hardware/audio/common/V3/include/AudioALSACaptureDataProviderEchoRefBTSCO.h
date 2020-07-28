#ifndef ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_ECHOREF__BTSCO_H
#define ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_ECHOREF__BTSCO_H

#include "AudioALSACaptureDataProviderBase.h"

namespace android {

class WCNChipController;

class AudioALSACaptureDataProviderEchoRefBTSCO : public AudioALSACaptureDataProviderBase {
public:
    virtual ~AudioALSACaptureDataProviderEchoRefBTSCO();

    static AudioALSACaptureDataProviderEchoRefBTSCO *getInstance();

    /**
     * open/close pcm interface when 1st attach & the last detach
     */
    status_t open();
    status_t close();



protected:
    AudioALSACaptureDataProviderEchoRefBTSCO();



private:
    /**
     * singleton pattern
     */
    static AudioALSACaptureDataProviderEchoRefBTSCO *mAudioALSACaptureDataProviderEchoRefBTSCO;

    WCNChipController *mWCNChipController;

    /**
     * pcm read thread
     */
    static void *readThread(void *arg);
    pthread_t hReadThread;

    struct timespec mNewtime, mOldtime; //for calculate latency
    double timerec[3]; //0=>threadloop, 1=>kernel delay, 2=>process delay
};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_ECHOREF__BTSCO_H