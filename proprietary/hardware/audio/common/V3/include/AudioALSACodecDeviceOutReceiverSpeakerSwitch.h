#ifndef ANDROID_AUDIO_ALSA_CODEC_DEVICE_OUT_RECEIVER_SPEAKER_SWITCH_H
#define ANDROID_AUDIO_ALSA_CODEC_DEVICE_OUT_RECEIVER_SPEAKER_SWITCH_H

#include "AudioType.h"

#include "AudioALSACodecDeviceBase.h"


namespace android {

class AudioALSACodecDeviceOutReceiverSpeakerSwitch : public AudioALSACodecDeviceBase {
public:
    virtual ~AudioALSACodecDeviceOutReceiverSpeakerSwitch();

    static AudioALSACodecDeviceOutReceiverSpeakerSwitch *getInstance();


    /**
     * open/close codec driver
     */
    status_t open();
    status_t close();



protected:
    AudioALSACodecDeviceOutReceiverSpeakerSwitch();



private:
    /**
     * singleton pattern
     */
    static AudioALSACodecDeviceOutReceiverSpeakerSwitch *mAudioALSACodecDeviceOutReceiverSpeakerSwitch;

};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_CODEC_DEVICE_OUT_EXT_SPEAKER_AMP_H