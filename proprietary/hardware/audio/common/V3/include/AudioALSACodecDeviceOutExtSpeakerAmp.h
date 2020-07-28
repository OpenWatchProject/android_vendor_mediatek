#ifndef ANDROID_AUDIO_ALSA_CODEC_DEVICE_OUT_EXT_SPEAKER_AMP_H
#define ANDROID_AUDIO_ALSA_CODEC_DEVICE_OUT_EXT_SPEAKER_AMP_H

#include "AudioType.h"

#include "AudioALSACodecDeviceBase.h"


namespace android {

class AudioALSACodecDeviceOutExtSpeakerAmp : public AudioALSACodecDeviceBase {
public:
    virtual ~AudioALSACodecDeviceOutExtSpeakerAmp();

    static AudioALSACodecDeviceOutExtSpeakerAmp *getInstance();


    /**
     * open/close codec driver
     */
    status_t open();
    status_t close();



protected:
    AudioALSACodecDeviceOutExtSpeakerAmp();



private:
    /**
     * singleton pattern
     */
    static AudioALSACodecDeviceOutExtSpeakerAmp *mAudioALSACodecDeviceOutExtSpeakerAmp;

};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_CODEC_DEVICE_OUT_EXT_SPEAKER_AMP_H