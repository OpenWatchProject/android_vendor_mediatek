#ifndef ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_VOW_H
#define ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_VOW_H

#include "AudioALSACaptureDataProviderBase.h"

#include <linux/vow.h>

namespace android {

class AudioALSACaptureDataProviderVOW : public AudioALSACaptureDataProviderBase {
public:
    virtual ~AudioALSACaptureDataProviderVOW();

    static AudioALSACaptureDataProviderVOW *getInstance();

    /**
     * open/close pcm interface when 1st attach & the last detach
     */
    status_t open();
    status_t close();



protected:
    AudioALSACaptureDataProviderVOW();



private:
    /**
     * singleton pattern
     */
    static AudioALSACaptureDataProviderVOW *mAudioALSACaptureDataProviderVOW;


    /**
     * pcm read thread
     */
    static void *readThread(void *arg);
    pthread_t hReadThread;

    int mFd;
    vow_model_info_t vow_info_buf;
    void  WriteVOWPcmData(void);

};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_VOW_H