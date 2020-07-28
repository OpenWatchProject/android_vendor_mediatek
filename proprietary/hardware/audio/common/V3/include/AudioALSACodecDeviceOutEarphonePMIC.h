#ifndef ANDROID_AUDIO_ALSA_CODEC_DEVICE_OUT_EARPHONE_PMIC_H
#define ANDROID_AUDIO_ALSA_CODEC_DEVICE_OUT_EARPHONE_PMIC_H

#include "AudioType.h"
#include "AudioALSACodecDeviceBase.h"
#include "AudioCustParamClient.h"


namespace android {

class AudioALSACodecDeviceOutEarphonePMIC : public AudioALSACodecDeviceBase {
public:
    virtual ~AudioALSACodecDeviceOutEarphonePMIC();

    static AudioALSACodecDeviceOutEarphonePMIC *getInstance();


    /**
     * open/close codec driver
     */
    status_t open();
    status_t close();
    status_t DeviceDoDcCalibrate();


protected:
    AudioALSACodecDeviceOutEarphonePMIC();

private:
    /**
     * singleton pattern
     */
    static AudioALSACodecDeviceOutEarphonePMIC *mAudioALSACodecDeviceOutEarphonePMIC;

};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_CODEC_DEVICE_OUT_EARPHONE_PMIC_H
