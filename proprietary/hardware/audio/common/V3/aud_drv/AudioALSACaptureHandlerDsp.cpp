#include "AudioALSACaptureHandlerDsp.h"
#include "AudioALSAHardwareResourceManager.h"
#include "AudioALSACaptureDataClientDsp.h"
#include "AudioALSACaptureDataProviderDsp.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioALSACaptureHandlerDsp"

namespace android {

//static FILE *pOutFile = NULL;

AudioALSACaptureHandlerDsp::AudioALSACaptureHandlerDsp(stream_attribute_t *stream_attribute_target) :
    AudioALSACaptureHandlerBase(stream_attribute_target) {
    ALOGD("%s()", __FUNCTION__);

    init();
}


AudioALSACaptureHandlerDsp::~AudioALSACaptureHandlerDsp() {
    ALOGD("+%s()", __FUNCTION__);

    ALOGD("%-s()", __FUNCTION__);
}


status_t AudioALSACaptureHandlerDsp::init() {
    ALOGD("%s()", __FUNCTION__);
    mCaptureHandlerType = CAPTURE_HANDLER_DSP;
    return NO_ERROR;
}


status_t AudioALSACaptureHandlerDsp::open() {
    ALOGD("+%s(), input_device = 0x%x, input_source = 0x%x, sample_rate=%d, num_channels=%d",
          __FUNCTION__, mStreamAttributeTarget->input_device, mStreamAttributeTarget->input_source, mStreamAttributeTarget->sample_rate,
          mStreamAttributeTarget->num_channels);

    ASSERT(mCaptureDataClient == NULL);

    mCaptureDataClient = new AudioALSACaptureDataClientDsp(AudioALSACaptureDataProviderDsp::getInstance(), mStreamAttributeTarget);

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSACaptureHandlerDsp::close() {
    ALOGD("+%s()", __FUNCTION__);

    ASSERT(mCaptureDataClient != NULL);
    delete mCaptureDataClient;

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSACaptureHandlerDsp::routing(const audio_devices_t input_device) {
    mHardwareResourceManager->changeInputDevice(input_device);
    return NO_ERROR;
}


ssize_t AudioALSACaptureHandlerDsp::read(void *buffer, ssize_t bytes) {
    ALOGV("%s()", __FUNCTION__);

    bytes = mCaptureDataClient->read(buffer, bytes);
#if 0   //remove dump here which might cause process too long due to SD performance
    if (pOutFile != NULL) {
        fwrite(buffer, sizeof(char), bytes, pOutFile);
    }
#endif

    return bytes;
}

} // end of namespace android
