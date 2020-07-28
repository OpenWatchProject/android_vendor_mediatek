#ifndef ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_ANC_H
#define ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_ANC_H

#include "AudioALSACaptureDataProviderBase.h"

namespace android {

class AudioALSACaptureDataProviderANC : public AudioALSACaptureDataProviderBase {
public:
    virtual ~AudioALSACaptureDataProviderANC();

    static AudioALSACaptureDataProviderANC *getInstance();

    /**
     * open/close pcm interface when 1st attach & the last detach
     */
    status_t open();
    status_t close();



protected:
    AudioALSACaptureDataProviderANC();



private:
    /**
     * singleton pattern
     */
    static AudioALSACaptureDataProviderANC *mAudioALSACaptureDataProviderANC;


    /**
     * pcm read thread
     */
    static void *readThread(void *arg);
    pthread_t hReadThread;

};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_ANC_H
