#include "AudioALSACodecDeviceBase.h"

#include <AudioLock.h>

#include "AudioALSADriverUtility.h"

#include "AudioALSACodecDeviceOutEarphonePMIC.h"
#include "AudioALSACodecDeviceOutReceiverPMIC.h"
#include "AudioALSACodecDeviceOutSpeakerPMIC.h"
#include "AudioALSACodecDeviceOutSpeakerEarphonePMIC.h"
#include "AudioALSADeviceConfigManager.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioALSACodecDeviceBase"

namespace android {
struct mixer *AudioALSACodecDeviceBase::mMixer = NULL;

AudioALSACodecDeviceBase *AudioALSACodecDeviceBase::createCodecOutputDevices() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    return AudioALSACodecDeviceOutSpeakerPMIC::getInstance();
}

AudioALSACodecDeviceBase::AudioALSACodecDeviceBase() :
    mClientCount(0) {
    ALOGV("%s()", __FUNCTION__);

    if (mMixer == NULL) {
        mMixer = AudioALSADriverUtility::getInstance()->getMixer();
        ASSERT(mMixer != NULL);
    }
}

AudioALSACodecDeviceBase::~AudioALSACodecDeviceBase() {
    ALOGV("%s()", __FUNCTION__);

    if (mMixer != NULL) {
        mixer_close(mMixer);
        mMixer = NULL;
    }
}

} // end of namespace android
