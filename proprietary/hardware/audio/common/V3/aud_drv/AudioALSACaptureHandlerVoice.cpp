#include "AudioALSACaptureHandlerVoice.h"
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
#include "AudioALSACaptureDataClientAurisysNormal.h"
#else
#include "AudioALSACaptureDataClient.h"
#endif
#include "AudioALSACaptureDataProviderVoice.h"
#include "AudioALSACaptureDataProviderVoiceDL.h"
#include "AudioALSACaptureDataProviderVoiceUL.h"
#include "AudioALSACaptureDataProviderVoiceMix.h"
#include "AudioType.h"
#include "audio_custom_exp.h"
#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "AudioALSACaptureHandlerVoice"

namespace android {

AudioALSACaptureHandlerVoice::AudioALSACaptureHandlerVoice(stream_attribute_t *stream_attribute_target) :
    AudioALSACaptureHandlerBase(stream_attribute_target) {
    ALOGD("%s()", __FUNCTION__);
    //Need to set false because HandlerVoice should close before speechoff
    mSupportConcurrencyInCall = false;
    init();
}


AudioALSACaptureHandlerVoice::~AudioALSACaptureHandlerVoice() {
    ALOGD("%s()", __FUNCTION__);
}


status_t AudioALSACaptureHandlerVoice::init() {
    ALOGD("%s()", __FUNCTION__);
    mCaptureHandlerType = CAPTURE_HANDLER_VOICE;
    return NO_ERROR;
}


status_t AudioALSACaptureHandlerVoice::open() {
    ALOGD("+%s(), input_device = 0x%x, input_source = 0x%x",
          __FUNCTION__, mStreamAttributeTarget->input_device, mStreamAttributeTarget->input_source);

    ASSERT(mCaptureDataClient == NULL);

    AudioALSACaptureDataProviderBase *provider = NULL;

#ifndef LEGACY_VOICE_RECORD
    switch (mStreamAttributeTarget->input_source) {
#ifdef INCALL_DL_RECORD_DISABLED
    default: {
        provider = AudioALSACaptureDataProviderVoiceUL::getInstance();
        break;
    }
#else
    case AUDIO_SOURCE_VOICE_DOWNLINK: {
        provider = AudioALSACaptureDataProviderVoiceDL::getInstance();
        break;
    }
    case AUDIO_SOURCE_VOICE_UPLINK: {
        provider = AudioALSACaptureDataProviderVoiceUL::getInstance();
        break;
    }
    case AUDIO_SOURCE_VOICE_CALL: {
        provider = AudioALSACaptureDataProviderVoiceMix::getInstance();
        break;
    }
    default: {
        provider = AudioALSACaptureDataProviderVoiceUL::getInstance();
        break;
    }
#endif
    }
#else
    // legacy voice record
    provider = AudioALSACaptureDataProviderVoice::getInstance();
#endif


#if defined(MTK_AURISYS_FRAMEWORK_SUPPORT)
    mCaptureDataClient = new AudioALSACaptureDataClientAurisysNormal(provider, mStreamAttributeTarget, NULL);
#else
    mCaptureDataClient = new AudioALSACaptureDataClient(provider, mStreamAttributeTarget);
#endif

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSACaptureHandlerVoice::close() {
    ALOGD("+%s()", __FUNCTION__);

    ASSERT(mCaptureDataClient != NULL);
    delete mCaptureDataClient;

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSACaptureHandlerVoice::routing(const audio_devices_t input_device __unused) {
    WARNING("Not support!!"); // TODO(Harvey): check it
    return INVALID_OPERATION;
}


ssize_t AudioALSACaptureHandlerVoice::read(void *buffer, ssize_t bytes) {
    ALOGV("%s()", __FUNCTION__);

    return mCaptureDataClient->read(buffer, bytes);
}

} // end of namespace android
