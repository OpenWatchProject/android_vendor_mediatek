#include "AudioALSACaptureHandlerVOW.h"

#include "AudioALSACaptureDataClient.h"
#include "AudioALSACaptureDataProviderVOW.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif


#define LOG_TAG "AudioALSACaptureHandlerVOW"

namespace android {

AudioALSACaptureHandlerVOW::AudioALSACaptureHandlerVOW(stream_attribute_t *stream_attribute_target) :
    AudioALSACaptureHandlerBase(stream_attribute_target) {
    ALOGD("%s()", __FUNCTION__);

    init();
}


AudioALSACaptureHandlerVOW::~AudioALSACaptureHandlerVOW() {
    ALOGD("%s()", __FUNCTION__);
}


status_t AudioALSACaptureHandlerVOW::init() {
    ALOGD("%s()", __FUNCTION__);


    return NO_ERROR;
}


status_t AudioALSACaptureHandlerVOW::open() {
    ALOGD("+%s(), input_device = 0x%x, input_source = 0x%x",
          __FUNCTION__, mStreamAttributeTarget->input_device, mStreamAttributeTarget->input_source);

    //ASSERT(mCaptureDataClient == NULL);
    mCaptureDataClient = new AudioALSACaptureDataClient(AudioALSACaptureDataProviderVOW::getInstance(), mStreamAttributeTarget);
    //mVOWCaptureDataProvider = AudioALSACaptureDataProviderVOW::getInstance();

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSACaptureHandlerVOW::close() {
    ALOGD("+%s()", __FUNCTION__);

    ASSERT(mCaptureDataClient != NULL);
    delete mCaptureDataClient;

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSACaptureHandlerVOW::routing(const audio_devices_t input_device __unused) {
    WARNING("Not support!!");
    return INVALID_OPERATION;
}


ssize_t AudioALSACaptureHandlerVOW::read(void *buffer, ssize_t bytes) {
    ALOGV("%s()", __FUNCTION__);

    return mCaptureDataClient->read(buffer, bytes);
}

} // end of namespace android
