#ifndef ANDROID_AUDIO_ALSA_SAMPLE_RATE_CONTROLLER_H
#define ANDROID_AUDIO_ALSA_SAMPLE_RATE_CONTROLLER_H

#include "AudioType.h"
#include "AudioAssert.h"
#include <AudioLock.h>

namespace android {

enum playback_scenario_mask_t {
    PLAYBACK_SCENARIO_STREAM_OUT = 0,
    PLAYBACK_SCENARIO_FM,
    PLAYBACK_SCENARIO_ECHO_REF,
    PLAYBACK_SCENARIO_ECHO_REF_EXT,
    PLAYBACK_SCENARIO_VOW_BARGE_IN,
    PLAYBACK_SCENARIO_COUNT
};


class AudioALSASampleRateController {
public:
    virtual ~AudioALSASampleRateController();

    static AudioALSASampleRateController *getInstance();

    virtual status_t setPrimaryStreamOutSampleRate(const uint32_t sample_rate);
    uint32_t         getPrimaryStreamOutSampleRate();


    void             setScenarioStatus(const playback_scenario_mask_t playback_scenario_mask);
    void             resetScenarioStatus(const playback_scenario_mask_t playback_scenario_mask);




protected:
    AudioALSASampleRateController();
    bool hasActiveScenario();

private:
    /**
     * singleton pattern
     */
    static AudioALSASampleRateController *mAudioALSASampleRateController;

    uint32_t mPrimaryStreamOutSampleRate;

    int mScenarioReference[PLAYBACK_SCENARIO_COUNT];

    AudioLock mLock;
};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_SAMPLE_RATE_CONTROLLER_H
