#ifndef ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_TDM_RECORD_H
#define ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_TDM_RECORD_H

#include "AudioALSACaptureDataProviderBase.h"

namespace android {

class AudioALSACaptureDataProviderTDM : public AudioALSACaptureDataProviderBase {
public:
    virtual ~AudioALSACaptureDataProviderTDM();

    static AudioALSACaptureDataProviderTDM *getInstance();

    /**
     * open/close pcm interface when 1st attach & the last detach
     */
    status_t open();
    status_t close();



protected:
    AudioALSACaptureDataProviderTDM();



private:
    /**
     * singleton pattern
     */
    static AudioALSACaptureDataProviderTDM *mAudioALSACaptureDataProviderTDM;


    /**
     * pcm read thread
     */
    static void *readThread(void *arg);
    pthread_t hReadThread;

};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_TDM_RECORD_H
