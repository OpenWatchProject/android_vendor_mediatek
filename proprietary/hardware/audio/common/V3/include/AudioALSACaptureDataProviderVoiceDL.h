#ifndef ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_VOICE_DL_H
#define ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_VOICE_DL_H

#include "AudioALSACaptureDataProviderBase.h"

namespace android {

class AudioALSACaptureDataProviderVoiceDL : public AudioALSACaptureDataProviderBase {
public:
    virtual ~AudioALSACaptureDataProviderVoiceDL();

    static AudioALSACaptureDataProviderVoiceDL *getInstance();

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
    AudioALSACaptureDataProviderVoiceDL();



private:
    /**
     * singleton pattern
     */
    static AudioALSACaptureDataProviderVoiceDL *mAudioALSACaptureDataProviderVoiceDL;

    /**
     * Period buffer related
     */
    RingBuf mRawDataRingBuf;
    uint32_t mPeriodBufferSize;

};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_CAPTURE_DATA_PROVIDER_VOICE_DL_H

