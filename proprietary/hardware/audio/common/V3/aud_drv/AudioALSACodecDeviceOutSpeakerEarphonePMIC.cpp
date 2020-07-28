#include "AudioALSACodecDeviceOutSpeakerEarphonePMIC.h"

#include <AudioLock.h>


#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioALSACodecDeviceOutSpeakerEarphonePMIC"

namespace android {

AudioALSACodecDeviceOutSpeakerEarphonePMIC *AudioALSACodecDeviceOutSpeakerEarphonePMIC::mAudioALSACodecDeviceOutSpeakerEarphonePMIC = NULL;
AudioALSACodecDeviceOutSpeakerEarphonePMIC *AudioALSACodecDeviceOutSpeakerEarphonePMIC::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    if (mAudioALSACodecDeviceOutSpeakerEarphonePMIC == NULL) {
        mAudioALSACodecDeviceOutSpeakerEarphonePMIC = new AudioALSACodecDeviceOutSpeakerEarphonePMIC();
    }
    ASSERT(mAudioALSACodecDeviceOutSpeakerEarphonePMIC != NULL);
    return mAudioALSACodecDeviceOutSpeakerEarphonePMIC;
}

status_t AudioALSACodecDeviceOutSpeakerEarphonePMIC::DeviceDoDcCalibrate() {
#define MAX_HP_GAIN_LEVEL (19)
#define DC_TRIM_OFFSET_HPL_SPK_NAME "Audio HPL_SPK Offset"
#define DC_TRIM_OFFSET_HPR_SPK_NAME "Audio HPR_SPK Offset"


    int HplOffset[MAX_HP_GAIN_LEVEL * 2], HprOffset, i, num_values;
    struct mixer_ctl *ctl;

    ALOGD("%s()", __FUNCTION__);

    // do dc calibrate
    ctl = mixer_get_ctl_by_name(mMixer, DC_TRIM_OFFSET_HPL_SPK_NAME);
    if (ctl) {
        mixer_ctl_get_array(ctl, HplOffset, MAX_HP_GAIN_LEVEL);
        for (i = 0; i < MAX_HP_GAIN_LEVEL; i++) {
            ALOGV("%s(), HplOffset[%d] = 0x%x ", __FUNCTION__, i,
                  HplOffset[i]);
        }
    } else {
        ALOGD("%s(), can't get mixer control(%s)", __FUNCTION__,
              DC_TRIM_OFFSET_HPL_SPK_NAME);
    }

    ctl = mixer_get_ctl_by_name(mMixer, DC_TRIM_OFFSET_HPR_SPK_NAME);
    if (ctl) {
        HprOffset = mixer_ctl_get_value(ctl, 0);
        ALOGD("%s(), HprOffset = 0x%x ", __FUNCTION__, HprOffset);
    } else {
        ALOGD("%s(), can't get mixer control(%s)", __FUNCTION__,
              DC_TRIM_OFFSET_HPR_SPK_NAME);
    }

    return NO_ERROR;
}

AudioALSACodecDeviceOutSpeakerEarphonePMIC::AudioALSACodecDeviceOutSpeakerEarphonePMIC() {
    ALOGD("%s()", __FUNCTION__);
#if !defined(MTK_AUDIO_KS)
    DeviceDoDcCalibrate();
#endif
}


AudioALSACodecDeviceOutSpeakerEarphonePMIC::~AudioALSACodecDeviceOutSpeakerEarphonePMIC() {
    ALOGD("%s()", __FUNCTION__);
}


status_t AudioALSACodecDeviceOutSpeakerEarphonePMIC::open() {
    ALOGD("+%s(), mClientCount = %d", __FUNCTION__, mClientCount);

    if (mClientCount == 0) {
        if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Headset_Speaker_Amp_Switch"), "On")) {
            ALOGE("Error: Headset_Speaker_Amp_Switch invalid value");
        }
    }

    mClientCount++;

    ALOGD("-%s(), mClientCount = %d", __FUNCTION__, mClientCount);
    return NO_ERROR;
}


status_t AudioALSACodecDeviceOutSpeakerEarphonePMIC::close() {
    ALOGD("+%s(), mClientCount = %d", __FUNCTION__, mClientCount);

    mClientCount--;

    if (mClientCount == 0) {
        if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Headset_Speaker_Amp_Switch"), "Off")) {
            ALOGE("Error: Headset_Speaker_Amp_Switch invalid value");
        }
    }

    ALOGD("-%s(), mClientCount = %d", __FUNCTION__, mClientCount);
    return NO_ERROR;
}


} // end of namespace android
