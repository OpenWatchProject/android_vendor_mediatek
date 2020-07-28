#ifndef ANDROID_AUDIO_ALSA_DATA_PROCESSOR_H
#define ANDROID_AUDIO_ALSA_DATA_PROCESSOR_H

#include "AudioType.h"

namespace android {

class AudioALSADataProcessor {
public:
    virtual ~AudioALSADataProcessor();

    static AudioALSADataProcessor *getInstance();



protected:
    AudioALSADataProcessor();



private:
    /**
     * singleton pattern
     */
    static AudioALSADataProcessor *mAudioALSADataProcessor;
};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_DATA_PROCESSOR_H
