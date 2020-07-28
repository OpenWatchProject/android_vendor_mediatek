#include "AudioALSADriverUtility.h"
#include <cutils/properties.h>
#include <AudioLock.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <AudioALSADeviceParser.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioALSADriverUtility"

namespace android {

AudioALSADriverUtility *AudioALSADriverUtility::mAudioALSADriverUtility = NULL;
AudioALSADriverUtility *AudioALSADriverUtility::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    if (mAudioALSADriverUtility == NULL) {
        mAudioALSADriverUtility = new AudioALSADriverUtility();
    }
    ASSERT(mAudioALSADriverUtility != NULL);
    return mAudioALSADriverUtility;
}

int AudioALSADriverUtility::GetPropertyValue(const char *ProPerty_Key) {
    int result;
    char value[PROPERTY_VALUE_MAX];
    property_get(ProPerty_Key, value, "0");
    result = atoi(value);
    return result;
}

int AudioALSADriverUtility::setPropertyValue(const char *ProPerty_Key, int value) {
    int result;
    char valuestring[PROPERTY_VALUE_MAX];

    sprintf(valuestring, "%d", value);
    property_set(ProPerty_Key, valuestring);
    return 0;
}

AudioALSADriverUtility::AudioALSADriverUtility() :
    mMixer(NULL) {
    ALOGD("%s()", __FUNCTION__);

    mMixer = mixer_open(AudioALSADeviceParser::getInstance()->GetCardIndex());
    ALOGD("mMixer = %p", mMixer);
    ASSERT(mMixer != NULL);
}


AudioALSADriverUtility::~AudioALSADriverUtility() {
    ALOGD("%s()", __FUNCTION__);

    mixer_close(mMixer);
    mMixer = NULL;
}


} // end of namespace android
