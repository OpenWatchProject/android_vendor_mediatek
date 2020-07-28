#ifndef ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_VOICE_UL_H
#define ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_VOICE_UL_H

#include "AudioALSACaptureDataProviderBase.h"
namespace android {
class AudioALSACaptureDataProviderVoiceUL : public AudioALSACaptureDataProviderBase {
public:
    virtual ~AudioALSACaptureDataProviderVoiceUL();

    static AudioALSACaptureDataProviderVoiceUL *getInstance();

    static bool hasInstance();

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
    AudioALSACaptureDataProviderVoiceUL();

private:
    /**
     * singleton pattern
     */
    static AudioALSACaptureDataProviderVoiceUL *mAudioALSACaptureDataProviderVoiceUL;

    /**
     * Period buffer related
     */
    RingBuf mRawDataRingBuf;
    uint32_t mPeriodBufferSize;

};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_VOICE_UL_H