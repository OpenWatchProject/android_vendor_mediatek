#ifndef ANDROID_AUDIO_ALSA_CODEC_DEVICE_OUT_SPEAKER_PMIC_H
#define ANDROID_AUDIO_ALSA_CODEC_DEVICE_OUT_SPEAKER_PMIC_H

#include "AudioType.h"

#include "AudioALSACodecDeviceBase.h"


namespace android {

class AudioALSACodecDeviceOutSpeakerPMIC : public AudioALSACodecDeviceBase {
public:
    virtual ~AudioALSACodecDeviceOutSpeakerPMIC();

    static AudioALSACodecDeviceOutSpeakerPMIC *getInstance();


    /**
     * open/close codec driver
     */
    status_t open();
    status_t close();



protected:
    AudioALSACodecDeviceOutSpeakerPMIC();



private:
    /**
     * singleton pattern
     */
    static AudioALSACodecDeviceOutSpeakerPMIC *mAudioALSACodecDeviceOutSpeakerPMIC;

};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_CODEC_DEVICE_OUT_SPEAKER_PMIC_H
