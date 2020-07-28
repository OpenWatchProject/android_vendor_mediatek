#include "AudioALSADataProcessor.h"

#include <utils/threads.h>

#include <AudioLock.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioALSADataProcessor"

namespace android {

AudioALSADataProcessor *AudioALSADataProcessor::mAudioALSADataProcessor = NULL;
AudioALSADataProcessor *AudioALSADataProcessor::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    if (mAudioALSADataProcessor == NULL) {
        mAudioALSADataProcessor = new AudioALSADataProcessor();
    }
    ASSERT(mAudioALSADataProcessor != NULL);
    return mAudioALSADataProcessor;
}

AudioALSADataProcessor::AudioALSADataProcessor() {
    ALOGD("%s()", __FUNCTION__);
}

AudioALSADataProcessor::~AudioALSADataProcessor() {
    ALOGD("%s()", __FUNCTION__);
}

} // end of namespace android

