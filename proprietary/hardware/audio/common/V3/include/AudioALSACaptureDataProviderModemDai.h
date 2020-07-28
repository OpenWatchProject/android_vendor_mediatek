#ifndef ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_MODEM_DAI_H
#define ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_MODEM_DAI_H

#include "AudioALSACaptureDataProviderBase.h"

namespace android {

class AudioALSACaptureDataProviderModemDai : public AudioALSACaptureDataProviderBase {
public:
    virtual ~AudioALSACaptureDataProviderModemDai();

    static AudioALSACaptureDataProviderModemDai *getInstance();

    /**
     * open/close pcm interface when 1st attach & the last detach
     */
    status_t open();
    status_t close();


protected:
    AudioALSACaptureDataProviderModemDai();


private:
    /**
     * singleton pattern
     */
    static AudioALSACaptureDataProviderModemDai *mAudioALSACaptureDataProviderModemDai;

    /**
     * pcm read thread
     */
    static void *readThread(void *arg);
    pthread_t hReadThread;

    /*
    *   moddai interface control
    */
    static const uint32_t kModDaiReadBufferSize = 640;
    static const uint32_t kModDaiReadPeriodSize = 160;
    static const uint32_t kModDaiReadBufferCount = 12;
    static const uint32_t kModDaiChanel = 1;
    static const uint32_t kModDaiSampleRate = 16000;

};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_MODEM_DAI_H