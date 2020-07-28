#ifndef ANDROID_AUDIO_ALSA_CODEC_DEVICE_OUT_SPEAKER_EARPHONE_PMIC_H
#define ANDROID_AUDIO_ALSA_CODEC_DEVICE_OUT_SPEAKER_EARPHONE_PMIC_H

#include "AudioType.h"

#include "AudioALSACodecDeviceBase.h"


namespace android {

class AudioALSACodecDeviceOutSpeakerEarphonePMIC : public AudioALSACodecDeviceBase {
public:
    virtual ~AudioALSACodecDeviceOutSpeakerEarphonePMIC();

    static AudioALSACodecDeviceOutSpeakerEarphonePMIC *getInstance();


    /**
     * open/close codec driver
     */
    status_t open();
    status_t close();
    status_t DeviceDoDcCalibrate();


protected:
    AudioALSACodecDeviceOutSpeakerEarphonePMIC();


private:
    /**
     * singleton pattern
     */
    static AudioALSACodecDeviceOutSpeakerEarphonePMIC *mAudioALSACodecDeviceOutSpeakerEarphonePMIC;

};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_CODEC_DEVICE_OUT_SPEAKER_EARPHONE_PMIC_H
