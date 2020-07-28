#include "AudioALSACaptureHandlerTDM.h"

#include "AudioALSAHardwareResourceManager.h"

#include "AudioALSACaptureDataClient.h"
#include "AudioALSACaptureDataProviderTDM.h"
#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "AudioALSACaptureHandlerTDM"

namespace android {

static FILE *pOutFile = NULL;

AudioALSACaptureHandlerTDM::AudioALSACaptureHandlerTDM(stream_attribute_t *stream_attribute_target) :
    AudioALSACaptureHandlerBase(stream_attribute_target) {
    ALOGD("%s()", __FUNCTION__);

    init();
}


AudioALSACaptureHandlerTDM::~AudioALSACaptureHandlerTDM() {
    ALOGD("+%s()", __FUNCTION__);

    ALOGD("%-s()", __FUNCTION__);
}


status_t AudioALSACaptureHandlerTDM::init() {
    ALOGD("%s()", __FUNCTION__);
    mCaptureHandlerType = CAPTURE_HANDLER_TDM;
    return NO_ERROR;
}


status_t AudioALSACaptureHandlerTDM::open() {
    ALOGD("+%s(), input_device = 0x%x, input_source = 0x%x, sample_rate=%d, num_channels=%d",
          __FUNCTION__, mStreamAttributeTarget->input_device, mStreamAttributeTarget->input_source, mStreamAttributeTarget->sample_rate,
          mStreamAttributeTarget->num_channels);

    ASSERT(mCaptureDataClient == NULL);
    mCaptureDataClient = new AudioALSACaptureDataClient(AudioALSACaptureDataProviderTDM::getInstance(), mStreamAttributeTarget);

#if 1
    pOutFile = fopen("/data/vendor/audiohal/RecTDM.pcm", "wb");
    if (pOutFile == NULL) {
        ALOGD("%s(), fopen fail", __FUNCTION__);
    }
#endif

    mHardwareResourceManager->startInputDevice(mStreamAttributeTarget->input_device);


    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSACaptureHandlerTDM::close() {
    ALOGD("+%s()", __FUNCTION__);

    mHardwareResourceManager->stopInputDevice(mHardwareResourceManager->getInputDevice());

    ASSERT(mCaptureDataClient != NULL);
    delete mCaptureDataClient;

#if 1
    if (pOutFile != NULL) {
        fclose(pOutFile);
    }
#endif
    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSACaptureHandlerTDM::routing(const audio_devices_t input_device __unused) {
    /*mHardwareResourceManager->changeInputDevice(input_device);*/
    return NO_ERROR;
}


ssize_t AudioALSACaptureHandlerTDM::read(void *buffer, ssize_t bytes) {
    ALOGV("%s()", __FUNCTION__);

    mCaptureDataClient->read(buffer, bytes);
#if 1   //remove dump here which might cause process too long due to SD performance
    if (pOutFile != NULL) {
        fwrite(buffer, sizeof(char), bytes, pOutFile);
    }
#endif
    return bytes;
}

} // end of namespace android
