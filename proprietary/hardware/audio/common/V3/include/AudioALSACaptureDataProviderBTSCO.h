#ifndef ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_BT_SCO_H
#define ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_BT_SCO_H

#include "AudioALSACaptureDataProviderBase.h"

namespace android {

class WCNChipController;

class AudioALSACaptureDataProviderBTSCO : public AudioALSACaptureDataProviderBase {
public:
    virtual ~AudioALSACaptureDataProviderBTSCO();

    static AudioALSACaptureDataProviderBTSCO *getInstance();

    /**
     * open/close pcm interface when 1st attach & the last detach
     */
    status_t open();
    status_t close();


protected:
    AudioALSACaptureDataProviderBTSCO();



private:
    /**
     * singleton pattern
     */
    static AudioALSACaptureDataProviderBTSCO *mAudioALSACaptureDataProviderBTSCO;

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

#endif // end of ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_BT_SCO_H
