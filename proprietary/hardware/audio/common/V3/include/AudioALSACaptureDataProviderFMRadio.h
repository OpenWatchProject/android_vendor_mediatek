#ifndef ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_FM_FADIO_H
#define ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_FM_FADIO_H

#include "AudioALSACaptureDataProviderBase.h"

namespace android {

class AudioALSACaptureDataProviderFMRadio : public AudioALSACaptureDataProviderBase {
public:
    virtual ~AudioALSACaptureDataProviderFMRadio();

    static AudioALSACaptureDataProviderFMRadio *getInstance();

    /**
     * open/close pcm interface when 1st attach & the last detach
     */
    status_t open();
    status_t close();



protected:
    AudioALSACaptureDataProviderFMRadio();



private:
    /**
     * singleton pattern
     */
    static AudioALSACaptureDataProviderFMRadio *mAudioALSACaptureDataProviderFMRadio;


    /**
     * pcm read thread
     */
    static void *readThread(void *arg);
    pthread_t hReadThread;

};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_FM_FADIO_H
