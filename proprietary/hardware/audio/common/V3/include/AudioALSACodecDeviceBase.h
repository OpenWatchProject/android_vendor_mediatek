#ifndef ANDROID_AUDIO_ALSA_CODEC_DEVICE_BASE_H
#define ANDROID_AUDIO_ALSA_CODEC_DEVICE_BASE_H

#include <tinyalsa/asoundlib.h>

#include "AudioType.h"

namespace android {

class DeviceCtlDescriptor;
class AudioALSACodecDeviceBase {
public:
    virtual ~AudioALSACodecDeviceBase();

    static AudioALSACodecDeviceBase *createCodecOutputDevices();


    /**
     * open/close codec driver
     */
    virtual status_t open() = 0;
    virtual status_t close() = 0;



protected:
    AudioALSACodecDeviceBase();


    /**
     * mixer controller
     */
    static struct mixer *mMixer;


    /**
     * open count, used to decide open/close codec driver or not
     */
    uint32_t mClientCount;

};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_CODEC_DEVICE_BASE_H
