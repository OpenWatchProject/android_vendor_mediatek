#ifndef ANDROID_AUDIO_ALSA_CODEC_DEVICE_OUT_HEADPHONE_H
#define ANDROID_AUDIO_ALSA_CODEC_DEVICE_OUT_HEADPHONE_H

#include "AudioType.h"
#include "AudioALSACodecDeviceBase.h"
#include "AudioCustParam.h"
#include "AudioALSADeviceParser.h"
#include <tinyalsa/asoundlib.h>

namespace android {

class AudioALSACodecDeviceOutHeadphone : public AudioALSACodecDeviceBase {
public:
    virtual ~AudioALSACodecDeviceOutHeadphone();

    static AudioALSACodecDeviceOutHeadphone *getInstance();

    int mCount;
    int hifi_enable;
    struct pcm_config mHpPcmConfig;
    struct pcm *mHpPcmOut;

    /**
     * open/close codec driver
     */
    status_t open();
    status_t open(int SampleRate);
    status_t close();
    status_t setHeadphoneState(int enable);
    status_t getHeadphoneState();

protected:
    AudioALSACodecDeviceOutHeadphone();

private:
    /**
     * singleton pattern
     */
    static AudioALSACodecDeviceOutHeadphone *mAudioALSACodecDeviceOutHeadphone;
};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_CODEC_DEVICE_OUT_HEADPHONE_H
