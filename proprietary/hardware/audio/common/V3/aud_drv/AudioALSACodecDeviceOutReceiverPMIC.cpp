#include "AudioALSACodecDeviceOutReceiverPMIC.h"

#include <AudioLock.h>

#include "AudioUtility.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioALSACodecDeviceOutReceiverPMIC"

namespace android {

AudioALSACodecDeviceOutReceiverPMIC *AudioALSACodecDeviceOutReceiverPMIC::mAudioALSACodecDeviceOutReceiverPMIC = NULL;
AudioALSACodecDeviceOutReceiverPMIC *AudioALSACodecDeviceOutReceiverPMIC::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    if (mAudioALSACodecDeviceOutReceiverPMIC == NULL) {
        mAudioALSACodecDeviceOutReceiverPMIC = new AudioALSACodecDeviceOutReceiverPMIC();
    }
    ASSERT(mAudioALSACodecDeviceOutReceiverPMIC != NULL);
    return mAudioALSACodecDeviceOutReceiverPMIC;
}


AudioALSACodecDeviceOutReceiverPMIC::AudioALSACodecDeviceOutReceiverPMIC() {
    ALOGD("%s()", __FUNCTION__);
}


AudioALSACodecDeviceOutReceiverPMIC::~AudioALSACodecDeviceOutReceiverPMIC() {
    ALOGD("%s()", __FUNCTION__);
}


status_t AudioALSACodecDeviceOutReceiverPMIC::open() {
    ALOGD("+%s(), mClientCount = %d", __FUNCTION__, mClientCount);

    if (mClientCount == 0) {
        if (IsAudioSupportFeature(AUDIO_SUPPORT_2IN1_SPEAKER)) {
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Speaker_class_Switch"), "RECEIVER")) {
                ALOGE("Error: Audio_Speaker_class_Switch invalid value");
            }
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Speaker_Amp_Switch"), "On")) {
                ALOGE("Error: Speaker_Amp_Switch invalid value");
            }
        } else {
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Voice_Amp_Switch"), "On")) {
                ALOGE("Error: Voice_Amp_Switch invalid value");
            }
        }

    }

    mClientCount++;

    ALOGD("-%s(), mClientCount = %d", __FUNCTION__, mClientCount);
    return NO_ERROR;
}


status_t AudioALSACodecDeviceOutReceiverPMIC::close() {
    ALOGD("+%s(), mClientCount = %d", __FUNCTION__, mClientCount);

    mClientCount--;

    if (mClientCount == 0) {
        if (IsAudioSupportFeature(AUDIO_SUPPORT_2IN1_SPEAKER)) {
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Speaker_Amp_Switch"), "Off")) {
                ALOGE("Error: Speaker_Amp_Switch invalid value");
            }
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Speaker_class_Switch"), "CLASSAB")) {
                ALOGE("Error: Audio_Speaker_class_Switch invalid value");
            }
        } else {
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Voice_Amp_Switch"), "Off")) {
                ALOGE("Error: Voice_Amp_Switch invalid value");
            }
        }
    }

    ALOGD("-%s(), mClientCount = %d", __FUNCTION__, mClientCount);
    return NO_ERROR;
}


} // end of namespace android
