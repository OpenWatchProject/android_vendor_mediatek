#include "AudioALSACodecDeviceOutHeadphone.h"

#include <AudioLock.h>


#define LOG_TAG "AudioALSACodecDeviceOutHeadphone"

namespace android {

AudioALSACodecDeviceOutHeadphone *AudioALSACodecDeviceOutHeadphone::mAudioALSACodecDeviceOutHeadphone = NULL;
AudioALSACodecDeviceOutHeadphone *AudioALSACodecDeviceOutHeadphone::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    if (mAudioALSACodecDeviceOutHeadphone == NULL) {
        mAudioALSACodecDeviceOutHeadphone = new AudioALSACodecDeviceOutHeadphone();
    }
    ASSERT(mAudioALSACodecDeviceOutHeadphone != NULL);
    return mAudioALSACodecDeviceOutHeadphone;
}

AudioALSACodecDeviceOutHeadphone::AudioALSACodecDeviceOutHeadphone() :
    hifi_enable(1),
    mCount(0),
    mHpPcmOut(NULL) {
    memset(&mHpPcmConfig, 0, sizeof(mHpPcmConfig));
    ALOGD("%s()", __FUNCTION__);
}


AudioALSACodecDeviceOutHeadphone::~AudioALSACodecDeviceOutHeadphone() {
    ALOGD("%s()", __FUNCTION__);
}

status_t AudioALSACodecDeviceOutHeadphone::open() {
    open(48000);

    return NO_ERROR;
}

status_t AudioALSACodecDeviceOutHeadphone::open(int SampleRate) {
    ALOGD("+%s(), mCount = %d, hifi_enable %d", __FUNCTION__, mCount, hifi_enable);

    if (mCount == 0) {
        if (hifi_enable) {
            int pcmindex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmExtHpMedia);
            int cardindex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmExtHpMedia);

            memset(&mHpPcmConfig, 0, sizeof(mHpPcmConfig));
            mHpPcmConfig.channels = 2;
            mHpPcmConfig.rate = SampleRate;
            mHpPcmConfig.period_size = 1024;
            mHpPcmConfig.period_count = 2;
            mHpPcmConfig.format = PCM_FORMAT_S32_LE;
            mHpPcmConfig.stop_threshold = ~(0UL);  // TODO: KC: check if needed

            ASSERT(mHpPcmOut == NULL);
            mHpPcmOut = pcm_open(cardindex, pcmindex, PCM_OUT | PCM_MONOTONIC, &mHpPcmConfig);

            if (pcm_is_ready(mHpPcmOut) == false) {
                ALOGE("%s(), pcm_is_ready(%p) == false due to %s, close pcm.", __FUNCTION__, mHpPcmOut, pcm_get_error(mHpPcmOut));
                pcm_close(mHpPcmOut);
                mHpPcmOut = NULL;
            } else {
                if (pcm_start(mHpPcmOut) != 0) {
                    ALOGE("%s(), pcm_start(%p) fail due to %s", __FUNCTION__, mHpPcmOut, pcm_get_error(mHpPcmOut));
                }
            }
            ASSERT(mHpPcmOut != NULL);
            ALOGD("%s(), mHpPcmOut = %p", __FUNCTION__, mHpPcmOut);
            if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "Master Playback Volume"), 0, 255)) {
                ALOGE("Error: Master Playback Volume left channel invalid value");
            }
            if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "Master Playback Volume"), 1, 255)) {
                ALOGE("Error: Master Playback Volume right channel invalid value");
            }
        } else {
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Amp_R_Switch"), "On")) {
                ALOGE("Error: Audio_Amp_R_Switch invalid value");
            }

            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Amp_L_Switch"), "On")) {
                ALOGE("Error: Audio_Amp_L_Switch invalid value");
            }
        }
    }

    mCount++;

    ALOGD("-%s(), mCount = %d", __FUNCTION__, mCount);
    return NO_ERROR;
}


status_t AudioALSACodecDeviceOutHeadphone::close() {
    ALOGD("+%s(), mCount = %d", __FUNCTION__, mCount);

    mCount--;

    if (mCount == 0) {
        if (hifi_enable) {
            pcm_stop(mHpPcmOut);
            pcm_close(mHpPcmOut);
            mHpPcmOut = NULL;
        } else {
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Amp_R_Switch"), "Off")) {
                ALOGE("Error: Audio_Amp_R_Switch invalid value");
            }

            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Amp_L_Switch"), "Off")) {
                ALOGE("Error: Audio_Amp_L_Switch invalid value");
            }
        }
    }

    ALOGD("-%s(), mCount = %d", __FUNCTION__, mCount);
    return NO_ERROR;
}

status_t AudioALSACodecDeviceOutHeadphone::setHeadphoneState(int enable) {
    hifi_enable = enable;

    return NO_ERROR;
}

status_t AudioALSACodecDeviceOutHeadphone::getHeadphoneState() {
    return hifi_enable;

    return NO_ERROR;
}

} // end of namespace android
