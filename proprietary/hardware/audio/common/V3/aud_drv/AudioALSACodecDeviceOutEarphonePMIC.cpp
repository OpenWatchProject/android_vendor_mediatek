#include "AudioALSACodecDeviceOutEarphonePMIC.h"

#include <AudioLock.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioALSACodecDeviceOutEarphonePMIC"

namespace android {

AudioALSACodecDeviceOutEarphonePMIC *AudioALSACodecDeviceOutEarphonePMIC::mAudioALSACodecDeviceOutEarphonePMIC = NULL;
AudioALSACodecDeviceOutEarphonePMIC *AudioALSACodecDeviceOutEarphonePMIC::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    if (mAudioALSACodecDeviceOutEarphonePMIC == NULL) {
        mAudioALSACodecDeviceOutEarphonePMIC = new AudioALSACodecDeviceOutEarphonePMIC();
    }
    ASSERT(mAudioALSACodecDeviceOutEarphonePMIC != NULL);
    return mAudioALSACodecDeviceOutEarphonePMIC;
}

status_t AudioALSACodecDeviceOutEarphonePMIC::DeviceDoDcCalibrate() {
    int hplOffset = 0, hprOffset = 0;
    int hplTrimCode = 0, hprTrimCode = 0;
    int retval = 0;

#ifdef NEW_PMIC_DCTRIM_FLOW
#define DC_TRIM_CONTROL_SWITCH_NAME "Dctrim_Control_Switch"
#define DC_TRIM_OFFSET_NAME "DcTrim_DC_Offset"
#define DC_TRIM_CODE_NAME "DcTrim_Hp_Trimcode"

#define DC_TRIM_CALIBRATED_STRING "Calibrated"
#define DC_TRIM_CALIBRATING_STRING "Calibrating"

    int dcTrimState = mixer_ctl_get_value(mixer_get_ctl_by_name(mMixer, DC_TRIM_CONTROL_SWITCH_NAME), 0);

    ALOGD("%s(), ctl %s = %d", __FUNCTION__, DC_TRIM_CONTROL_SWITCH_NAME, dcTrimState);

    // check if calibrated
    if (strcmp(mixer_ctl_get_enum_string(mixer_get_ctl_by_name(mMixer, DC_TRIM_CONTROL_SWITCH_NAME), dcTrimState), DC_TRIM_CALIBRATED_STRING) == 0) {
        ALOGD("%s(), dc trimmed", __FUNCTION__);
        return 0;
    }

    // do dc calibrate
    retval = mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, DC_TRIM_CONTROL_SWITCH_NAME), DC_TRIM_CALIBRATING_STRING);
    hplOffset = mixer_ctl_get_value(mixer_get_ctl_by_name(mMixer, DC_TRIM_OFFSET_NAME), 0);
    hprOffset = mixer_ctl_get_value(mixer_get_ctl_by_name(mMixer, DC_TRIM_OFFSET_NAME), 1);
    hplTrimCode = mixer_ctl_get_value(mixer_get_ctl_by_name(mMixer, DC_TRIM_CODE_NAME), 0);
    hprTrimCode = mixer_ctl_get_value(mixer_get_ctl_by_name(mMixer, DC_TRIM_CODE_NAME), 1);
#else
    hplOffset = mixer_ctl_get_value(mixer_get_ctl_by_name(mMixer, "Audio HPL Offset"), 0);
    hprOffset = mixer_ctl_get_value(mixer_get_ctl_by_name(mMixer, "Audio HPR Offset"), 0);
#endif

    ALOGD("-%s(), hp_dc_offset = [%d, %d], hp_trim_code = [%d, %d]", __FUNCTION__, hplOffset, hprOffset, hplTrimCode, hprTrimCode);

    return NO_ERROR;
}

AudioALSACodecDeviceOutEarphonePMIC::AudioALSACodecDeviceOutEarphonePMIC() {
    ALOGD("%s()", __FUNCTION__);
#if !defined(MTK_AUDIO_KS)
    DeviceDoDcCalibrate();
#endif
}


AudioALSACodecDeviceOutEarphonePMIC::~AudioALSACodecDeviceOutEarphonePMIC() {
    ALOGD("%s()", __FUNCTION__);
}


status_t AudioALSACodecDeviceOutEarphonePMIC::open() {
    ALOGD("+%s(), mClientCount = %d", __FUNCTION__, mClientCount);

    if (mClientCount == 0) {
        if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Amp_R_Switch"), "On")) {
            ALOGE("Error: Audio_Amp_R_Switch invalid value");
        }

        if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Amp_L_Switch"), "On")) {
            ALOGE("Error: Audio_Amp_L_Switch invalid value");
        }
    }

    mClientCount++;

    ALOGD("-%s(), mClientCount = %d", __FUNCTION__, mClientCount);
    return NO_ERROR;
}


status_t AudioALSACodecDeviceOutEarphonePMIC::close() {
    ALOGD("+%s(), mClientCount = %d", __FUNCTION__, mClientCount);

    mClientCount--;

    if (mClientCount == 0) {
        if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Amp_R_Switch"), "Off")) {
            ALOGE("Error: Audio_Amp_R_Switch invalid value");
        }

        if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Amp_L_Switch"), "Off")) {
            ALOGE("Error: Audio_Amp_L_Switch invalid value");
        }
    }

    ALOGD("-%s(), mClientCount = %d", __FUNCTION__, mClientCount);
    return NO_ERROR;
}


} // end of namespace android
