#include "AudioALSACodecDeviceOutReceiverSpeakerSwitch.h"

#include <AudioLock.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioALSACodecDeviceOutReceiverSpeakerSwitch"

namespace android {

AudioALSACodecDeviceOutReceiverSpeakerSwitch *AudioALSACodecDeviceOutReceiverSpeakerSwitch::mAudioALSACodecDeviceOutReceiverSpeakerSwitch = NULL;
AudioALSACodecDeviceOutReceiverSpeakerSwitch *AudioALSACodecDeviceOutReceiverSpeakerSwitch::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    if (mAudioALSACodecDeviceOutReceiverSpeakerSwitch == NULL) {
        mAudioALSACodecDeviceOutReceiverSpeakerSwitch = new AudioALSACodecDeviceOutReceiverSpeakerSwitch();
    }
    ASSERT(mAudioALSACodecDeviceOutReceiverSpeakerSwitch != NULL);
    return mAudioALSACodecDeviceOutReceiverSpeakerSwitch;
}


AudioALSACodecDeviceOutReceiverSpeakerSwitch::AudioALSACodecDeviceOutReceiverSpeakerSwitch() {
    ALOGD("%s()", __FUNCTION__);
}


AudioALSACodecDeviceOutReceiverSpeakerSwitch::~AudioALSACodecDeviceOutReceiverSpeakerSwitch() {
    ALOGD("%s()", __FUNCTION__);
}


status_t AudioALSACodecDeviceOutReceiverSpeakerSwitch::open() {
    ALOGD("+%s(), mClientCount = %d", __FUNCTION__, mClientCount);

    if (mClientCount == 0) {
        if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Receiver_Speaker_Switch"), "On")) {
            ALOGE("Error: Ext_Speaker_Amp_Switch invalid value");
        }
    }

    mClientCount++;

    ALOGD("-%s(), mClientCount = %d", __FUNCTION__, mClientCount);
    return NO_ERROR;
}


status_t AudioALSACodecDeviceOutReceiverSpeakerSwitch::close() {
    ALOGD("+%s(), mClientCount = %d", __FUNCTION__, mClientCount);

    mClientCount--;

    if (mClientCount == 0) {
        if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Receiver_Speaker_Switch"), "Off")) {
            ALOGE("Error: Ext_Speaker_Amp_Switch invalid value");
        }
    }

    ALOGD("-%s(), mClientCount = %d", __FUNCTION__, mClientCount);
    return NO_ERROR;
}


} // end of namespace android
