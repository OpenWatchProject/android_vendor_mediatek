#include "AudioALSACodecDeviceOutExtSpeakerAmp.h"

#include <AudioLock.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioALSACodecDeviceOutExtSpeakerAmp"

namespace android {

AudioALSACodecDeviceOutExtSpeakerAmp *AudioALSACodecDeviceOutExtSpeakerAmp::mAudioALSACodecDeviceOutExtSpeakerAmp = NULL;
AudioALSACodecDeviceOutExtSpeakerAmp *AudioALSACodecDeviceOutExtSpeakerAmp::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    if (mAudioALSACodecDeviceOutExtSpeakerAmp == NULL) {
        mAudioALSACodecDeviceOutExtSpeakerAmp = new AudioALSACodecDeviceOutExtSpeakerAmp();
    }
    ASSERT(mAudioALSACodecDeviceOutExtSpeakerAmp != NULL);
    return mAudioALSACodecDeviceOutExtSpeakerAmp;
}


AudioALSACodecDeviceOutExtSpeakerAmp::AudioALSACodecDeviceOutExtSpeakerAmp() {
    ALOGD("%s()", __FUNCTION__);
}


AudioALSACodecDeviceOutExtSpeakerAmp::~AudioALSACodecDeviceOutExtSpeakerAmp() {
    ALOGD("%s()", __FUNCTION__);
}


status_t AudioALSACodecDeviceOutExtSpeakerAmp::open() {
    ALOGD("+%s(), mClientCount = %d", __FUNCTION__, mClientCount);

    if (mClientCount == 0) {
        if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Ext_Speaker_Amp_Switch"), "On")) {
            ALOGE("Error: Ext_Speaker_Amp_Switch invalid value");
        }
    }

    mClientCount++;

    ALOGD("-%s(), mClientCount = %d", __FUNCTION__, mClientCount);
    return NO_ERROR;
}


status_t AudioALSACodecDeviceOutExtSpeakerAmp::close() {
    ALOGD("+%s(), mClientCount = %d", __FUNCTION__, mClientCount);

    mClientCount--;

    if (mClientCount == 0) {
        if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Ext_Speaker_Amp_Switch"), "Off")) {
            ALOGE("Error: Ext_Speaker_Amp_Switch invalid value");
        }
    }

    ALOGD("-%s(), mClientCount = %d", __FUNCTION__, mClientCount);
    return NO_ERROR;
}


} // end of namespace android
