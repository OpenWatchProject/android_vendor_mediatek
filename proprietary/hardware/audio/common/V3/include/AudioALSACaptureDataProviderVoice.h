#ifndef ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_VOICE_H
#define ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_VOICE_H

#include "AudioALSACaptureDataProviderBase.h"

namespace android {

class AudioALSACaptureDataProviderVoice : public AudioALSACaptureDataProviderBase {
public:
    virtual ~AudioALSACaptureDataProviderVoice();

    static AudioALSACaptureDataProviderVoice *getInstance();

    /**
     * open/close pcm interface when 1st attach & the last detach
     */
    status_t open();
    status_t close();

    /**
     * provide modem record data to capture data provider
     */
    status_t provideModemRecordDataToProvider(RingBuf modem_record_buf);


protected:
    AudioALSACaptureDataProviderVoice();



private:
    /**
     * singleton pattern
     */
    static AudioALSACaptureDataProviderVoice *mAudioALSACaptureDataProviderVoice;
};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_VOICE_H
