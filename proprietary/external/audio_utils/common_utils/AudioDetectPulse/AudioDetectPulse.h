#ifndef ANDROID_AUDIO_DETECT_PULSE_H
#define ANDROID_AUDIO_DETECT_PULSE_H

#include <sys/types.h>
#include <pulse.h>

namespace android {

static const int defaultPrecision = 4;

class AudioDetectPulse {
public:
    AudioDetectPulse();
    virtual             ~AudioDetectPulse();

    static AudioDetectPulse *getInstance();

    static void     setDetectPulse(const bool enable);
    static bool     getDetectPulse();

    static void     doDetectPulse(const int TagNum, const int pulseLevel, const int dump, void *ptr,
                                  const size_t desiredFrames, const audio_format_t format,
                                  const int channels, const int sampleRate);

private:
    static AudioDetectPulse *mAudioDetectPulse;
    static bool mIsDetectPulse;
    static int mPrecision;
};


}; // end namespace android

#endif // end of ANDROID_AUDIO_DETECT_PULSE_H
