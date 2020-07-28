#include "AudioALSASampleRateController.h"

#include <AudioLock.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioALSASampleRateController"

namespace android {

AudioALSASampleRateController *AudioALSASampleRateController::mAudioALSASampleRateController = NULL;
AudioALSASampleRateController *AudioALSASampleRateController::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    if (mAudioALSASampleRateController == NULL) {
        mAudioALSASampleRateController = new AudioALSASampleRateController();
    }
    ASSERT(mAudioALSASampleRateController != NULL);
    return mAudioALSASampleRateController;
}


AudioALSASampleRateController::AudioALSASampleRateController() :
    mPrimaryStreamOutSampleRate(44100) {
    ALOGD("%s()", __FUNCTION__);
    memset(&mScenarioReference, 0, sizeof(mScenarioReference));
}


AudioALSASampleRateController::~AudioALSASampleRateController() {
    ALOGD("%s()", __FUNCTION__);
}


status_t AudioALSASampleRateController::setPrimaryStreamOutSampleRate(const uint32_t sample_rate) {
    AL_AUTOLOCK(mLock);
    uint32_t old_sample_rate = mPrimaryStreamOutSampleRate;

    if (hasActiveScenario()) {
        ALOGW("-%s(), some other scenatio is active", __FUNCTION__);
        return INVALID_OPERATION;
    } else if (sample_rate == mPrimaryStreamOutSampleRate) {
        ALOGW("-%s(), sample_rate(%u) == mPrimaryStreamOutSampleRate(%u), return", __FUNCTION__, sample_rate, old_sample_rate);
        return ALREADY_EXISTS;
    }
    mPrimaryStreamOutSampleRate = sample_rate;

    ALOGD("%s(), update mPrimaryStreamOutSampleRate: %u => %u", __FUNCTION__, old_sample_rate, mPrimaryStreamOutSampleRate);
    return NO_ERROR;
}


uint32_t AudioALSASampleRateController::getPrimaryStreamOutSampleRate() {
    AL_AUTOLOCK(mLock);
    return mPrimaryStreamOutSampleRate;
}


void AudioALSASampleRateController::setScenarioStatus(const playback_scenario_mask_t playback_scenario_mask) {
    AL_AUTOLOCK(mLock);

    mScenarioReference[playback_scenario_mask]++;
}

void AudioALSASampleRateController::resetScenarioStatus(const playback_scenario_mask_t playback_scenario_mask) {
    AL_AUTOLOCK(mLock);

    mScenarioReference[playback_scenario_mask]--;

    if (mScenarioReference[playback_scenario_mask] < 0) {
        ALOGW("%s unexpected operation for scenario %d", __FUNCTION__, playback_scenario_mask);
        mScenarioReference[playback_scenario_mask] = 0;
    }
}

bool AudioALSASampleRateController::hasActiveScenario() {
    for (int i = 0; i < PLAYBACK_SCENARIO_COUNT; i++) {
        if (mScenarioReference[i] > 0) {
            ALOGV("%s scenario %d is active", __FUNCTION__, i);
            return true;
        }
    }

    return false;
}

} // end of namespace android
