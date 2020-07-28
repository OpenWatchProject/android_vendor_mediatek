#include "AudioDetectPulse.h"

#include <stdlib.h>
#include <utils/Log.h>
#include <cutils/properties.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioDetectPulse"


namespace android {

bool AudioDetectPulse::mIsDetectPulse = false;
int AudioDetectPulse::mPrecision = defaultPrecision;

AudioDetectPulse::AudioDetectPulse() {
    char value[PROPERTY_VALUE_MAX];
    (void) property_get("vendor.audio.detect.pulse", value, "0");
    int debuggable = atoi(value);
    mIsDetectPulse = debuggable ? true : false;

    (void) property_get("vendor.audio.latency.precision", value, "1");  //defaultPrecision = 1
    mPrecision = atoi(value);
}

AudioDetectPulse::~AudioDetectPulse() {
}

AudioDetectPulse *AudioDetectPulse::mAudioDetectPulse = 0;

AudioDetectPulse *AudioDetectPulse::getInstance() {
    if (mAudioDetectPulse == 0) {
        ALOGD("+AudioDetectPulse");
        mAudioDetectPulse = new AudioDetectPulse();
        ALOGD("-AudioDetectPulse %p", mAudioDetectPulse);
    }
    return mAudioDetectPulse;
}

void AudioDetectPulse::setDetectPulse(const bool enable) {
    mIsDetectPulse = enable;
    ALOGD("%s, mIsDetectPulse %d, %p", __FUNCTION__, mIsDetectPulse, &mIsDetectPulse);
}

bool AudioDetectPulse::getDetectPulse() {
    //ALOGD("%s, mIsDetectPulse %d", __FUNCTION__, mIsDetectPulse);
    return mIsDetectPulse;
}

void AudioDetectPulse::doDetectPulse(const int TagNum, const int pulseLevel, const int dump, void *ptr,
                                     const size_t desiredBufferSize, const audio_format_t format,
                                     const int channels, const int sampleRate) {
    if (getDetectPulse()) {
        if (channels == 0) {
            ALOGD("%s, TagNum %d, channels can't be zero!", __FUNCTION__, TagNum);
            return;
        }

        size_t desiredFrames = desiredBufferSize / channels / ((format == AUDIO_FORMAT_PCM_16_BIT) ? 2 : 4);

        if (mPrecision < 1 || mPrecision > (int)desiredFrames) {
            ALOGW("%s, precision (%d) is not support! Use default value (%d)",__FUNCTION__, mPrecision, defaultPrecision);
            mPrecision = defaultPrecision;
        }
        detectPulse(TagNum, pulseLevel, dump, ptr, desiredFrames, format, channels, sampleRate, mPrecision);
    }
}

} // end of namespace android
