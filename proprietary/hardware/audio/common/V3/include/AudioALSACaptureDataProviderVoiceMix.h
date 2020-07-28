#ifndef ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_VOICE_MIX_H
#define ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_VOICE_MIX_H

#include "AudioALSACaptureDataProviderBase.h"

namespace android {

class AudioALSACaptureDataProviderVoiceMix : public AudioALSACaptureDataProviderBase {
public:
    virtual ~AudioALSACaptureDataProviderVoiceMix();

    static AudioALSACaptureDataProviderVoiceMix *getInstance();

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
    AudioALSACaptureDataProviderVoiceMix();



private:
    /**
     * singleton pattern
     */
    static AudioALSACaptureDataProviderVoiceMix *mAudioALSACaptureDataProviderVoiceMix;

    /**
     * Period buffer related
     */
    RingBuf mRawDataRingBuf;
    uint32_t mPeriodBufferSize;

};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_VOICE_MIX_H

